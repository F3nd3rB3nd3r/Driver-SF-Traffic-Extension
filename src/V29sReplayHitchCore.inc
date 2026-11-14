// Passive observers only. No engine calls, native writes or file I/O here.
#define DSF_REPLAY_HITCH_RUNTIME 1
enum RHKind {RHGate20,RHGate24,RHStep,RHSeek,RHStream,RHUpdate,RHImport,RHRestore,RHRequest,RHKinds};
struct RHView {
    U32 valid,owner,recording,mode,phase,transition,countdown,loading,lifecycle;
    U32 clock,requested,applied,paused,traffic,retained,spool,selected,eventType,eventKind;
};
struct RHStats {
    U32 calls[RHKinds],ticks[RHKinds],maximum[RHKinds];
    U32 busy20,busy24,subscriber,snapshot,traffic,unknown,advanced,unchanged,backwards,maxStepGap,maxUpdateGap;
};
struct RHFrame {U32 kind,self,caller,arg0,arg1,tick;__int64 start;RHView before;};
struct RHSlot {
    volatile long thread;U32 depth,overflow;RHFrame stack[24];RHStats stats;
    U32 windowTick,lastStep,lastUpdate,tracking,clockTick,holdPrinted,lastHold;
    RHView previous;__int64 overhead;
    U32 gateOpen[2];__int64 gateStart[2];RHView gateBefore[2];
};
struct RHRecord {U32 kind,op,thread,tick,duration,result,caller,arg0,arg1,span,overhead;RHView before,after;RHStats stats;};
static RHSlot g_rhSlots[16]={};
static RHRecord g_rhRing[512]={};
static U32 g_rhHead=0,g_rhTail=0;
static volatile long g_rhLock=0,g_rhDropped=0,g_rhThreadDrops=0,g_rhDepthDrops=0,g_rhMismatches=0,g_rhClockFailures=0;
static long g_rhStatus=0;
static __int64 g_rhWaitMinTicks=1;
static U32 RHThread();static U32 RHTick();static __int64 RHNow();
static RHView RHReadView(U32,UPtr);
static U32 RHDuration(__int64 delta){return delta<=0?0u:(delta>0xFFFFFFFFLL?0xFFFFFFFFu:static_cast<U32>(delta));}
static void RHAdd(U32& to,U32 n){to=n>0xFFFFFFFFu-to?0xFFFFFFFFu:to+n;}
static void RHMax(U32& to,U32 n){if(n>to)to=n;}
static RHSlot* RHSlotForThread() {
    const U32 id=RHThread();
    for(U32 i=0;i<16u;++i)if(static_cast<U32>(g_rhSlots[i].thread)==id)return &g_rhSlots[i];
    for(U32 i=0;i<16u;++i)if(!InterlockedCompareExchange(&g_rhSlots[i].thread,static_cast<long>(id),0))return &g_rhSlots[i];
    InterlockedIncrement(&g_rhThreadDrops);return nullptr;
}
static void RHQueue(const RHRecord& record) {
    if(InterlockedCompareExchange(&g_rhLock,1,0)){InterlockedIncrement(&g_rhDropped);return;}
    if(g_rhHead-g_rhTail>=512u)InterlockedIncrement(&g_rhDropped);
    else{g_rhRing[g_rhHead&511u]=record;++g_rhHead;}
    InterlockedExchange(&g_rhLock,0);
}
static bool RHPop(RHRecord& record) {
    if(InterlockedCompareExchange(&g_rhLock,1,0))return false;
    const bool found=g_rhTail!=g_rhHead;
    if(found){record=g_rhRing[g_rhTail&511u];++g_rhTail;}
    InterlockedExchange(&g_rhLock,0);return found;
}
static void RHEmit(RHSlot& slot,U32 event,U32 op,const RHView& before,const RHView& after,U32 duration=0u) {
    RHRecord r={};r.kind=event;r.op=op;r.thread=static_cast<U32>(slot.thread);r.tick=RHTick();
    r.duration=duration;r.before=before;r.after=after;r.stats=slot.stats;RHQueue(r);
}
static bool RHStateChanged(const RHView& a,const RHView& b) {
    return a.valid!=b.valid || a.owner!=b.owner || a.recording!=b.recording || a.mode!=b.mode ||
        a.phase!=b.phase || a.transition!=b.transition || a.loading!=b.loading ||
        a.requested!=b.requested || a.applied!=b.applied || a.paused!=b.paused;
}
static void RHTrack(RHSlot& slot,const RHView& v,U32 now) {
    const RHView old=slot.previous;
    const bool same=slot.tracking && v.valid && old.valid && v.owner==old.owner && v.recording==old.recording;
    if(!same){slot.lastStep=0u;slot.lastUpdate=0u;slot.gateOpen[0]=slot.gateOpen[1]=0u;}
    if(!same || RHStateChanged(old,v))RHEmit(slot,1u,RHUpdate,old,v);
    if(!same || v.clock!=old.clock || !(v.requested&0x7FFFFFFFu)) {
        if(same && slot.holdPrinted)RHEmit(slot,4u,RHUpdate,old,v,now-slot.clockTick);
        slot.clockTick=now;slot.holdPrinted=0u;
    }else if(now-slot.clockTick>=50u && (!slot.holdPrinted || now-slot.lastHold>=1000u)) {
        RHEmit(slot,3u,RHUpdate,old,v,now-slot.clockTick);slot.holdPrinted=1u;slot.lastHold=now;
    }
    slot.previous=v;slot.tracking=v.valid;
}
static void RHBegin(U32 kind,UPtr self,UPtr caller,U32 arg0=0u,U32 arg1=0u) {
    if(g_rhStatus!=1)return;
    const __int64 overhead=RHNow();RHSlot* slot=RHSlotForThread();if(!slot)return;
    if(slot->depth>=24u || slot->overflow){++slot->overflow;InterlockedIncrement(&g_rhDepthDrops);return;}
    RHFrame& f=slot->stack[slot->depth++];f.kind=kind;f.self=self;f.caller=caller;f.arg0=arg0;f.arg1=arg1;
    f.tick=RHTick();f.before=RHReadView(kind,self);
    if(kind==RHUpdate && f.before.valid && (!slot->tracking || slot->previous.owner!=f.before.owner || slot->previous.recording!=f.before.recording)) {
        // Do not count time spent outside replay as a scheduler gap/window.
        slot->stats={};slot->windowTick=f.tick;slot->lastStep=slot->lastUpdate=0u;slot->overhead=0;
    }
    f.start=RHNow();slot->overhead+=f.start-overhead;
}
static void RHFinish(U32 kind,U32 result) {
    if(g_rhStatus!=1)return;
    const __int64 end=RHNow();RHSlot* slot=RHSlotForThread();if(!slot)return;
    if(slot->overflow){--slot->overflow;return;}
    if(!slot->depth || slot->stack[slot->depth-1u].kind!=kind){InterlockedIncrement(&g_rhMismatches);return;}
    const RHFrame f=slot->stack[--slot->depth];const RHView v=RHReadView(kind,f.self);
    const U32 duration=RHDuration(end-f.start),now=RHTick();
    if(f.before.valid || v.valid) {
        ++slot->stats.calls[kind];RHAdd(slot->stats.ticks[kind],duration);RHMax(slot->stats.maximum[kind],duration);
        if(kind==RHGate20 || kind==RHGate24) {
            if(slot->gateOpen[kind] && (!v.valid || !slot->gateBefore[kind].valid ||
                slot->gateBefore[kind].owner!=v.owner || slot->gateBefore[kind].recording!=v.recording))slot->gateOpen[kind]=0u;
            if(result&255u) {
                if(!slot->gateOpen[kind]){slot->gateOpen[kind]=1u;slot->gateStart[kind]=f.start;slot->gateBefore[kind]=f.before;}
            }else if(slot->gateOpen[kind]) {
                const __int64 span=end-slot->gateStart[kind];
                if(span>=g_rhWaitMinTicks)RHEmit(*slot,5u,kind,slot->gateBefore[kind],v,RHDuration(span));
                slot->gateOpen[kind]=0u;
            }
            if(result&255u) {
                if(kind==RHGate20)++slot->stats.busy20;else ++slot->stats.busy24;
                if(!v.valid)++slot->stats.unknown;
                else if(v.phase)++slot->stats.snapshot;
                else if(v.spool==0xFFFFFFFFu || v.traffic==0xFFFFFFFFu)++slot->stats.unknown;
                else if(v.spool==1u || v.traffic)++slot->stats.traffic;
                else ++slot->stats.subscriber; // Gate returned true, fallback is idle. No extra virtual calls.
            }
        }else if(kind==RHStep) {
            if(f.before.valid && v.valid && f.before.owner==v.owner && f.before.recording==v.recording) {
                if(v.clock>f.before.clock)++slot->stats.advanced;
                else if(v.clock==f.before.clock)++slot->stats.unchanged;else ++slot->stats.backwards;
            }
            if(slot->lastStep)RHMax(slot->stats.maxStepGap,now-slot->lastStep);slot->lastStep=now;
        }else if(kind==RHUpdate) {
            if(slot->lastUpdate)RHMax(slot->stats.maxUpdateGap,now-slot->lastUpdate);slot->lastUpdate=now;
            RHTrack(*slot,v,now);
        }else {
            RHRecord r={};r.kind=2u;r.op=kind;r.thread=static_cast<U32>(slot->thread);r.tick=now;
            r.duration=duration;r.result=result;r.caller=f.caller;r.arg0=f.arg0;r.arg1=f.arg1;r.before=f.before;r.after=v;RHQueue(r);
        }
        if(v.valid && !slot->windowTick)slot->windowTick=now;
        if(v.valid && now-slot->windowTick>=1000u) {
            RHRecord r={};r.kind=0u;r.op=kind;r.thread=static_cast<U32>(slot->thread);r.tick=now;
            r.span=now-slot->windowTick;r.before=f.before;r.after=v;r.stats=slot->stats;r.overhead=RHDuration(slot->overhead);
            RHQueue(r);slot->stats={};slot->overhead=0;slot->windowTick=now;
        }
    }
    else if(kind==RHUpdate && slot->tracking){RHTrack(*slot,v,now);slot->gateOpen[0]=slot->gateOpen[1]=0u;}
    if(kind==RHUpdate && !v.valid){slot->windowTick=0u;slot->stats={};slot->lastStep=slot->lastUpdate=0u;}
    slot->overhead+=RHNow()-end;
}
// pushfd/pushad frame: EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX,flags,return,args.
extern "C" __declspec(noinline) void __cdecl RHEnter(U32* frame,U32 kind) {
    RHBegin(kind,frame[6],frame[9],kind==RHSeek || kind==RHStream?frame[10]:0u,kind==RHSeek?frame[11]:0u);
}
extern "C" __declspec(noinline) void __cdecl RHLeave(U32* frame,U32 kind){RHFinish(kind,frame[7]);}
struct RHScope {
    U32 kind;
    RHScope(U32 k,UPtr self,UPtr caller=0u,U32 arg=0u):kind(k){RHBegin(k,self,caller,arg);}
    ~RHScope(){RHFinish(kind,0u);}
};

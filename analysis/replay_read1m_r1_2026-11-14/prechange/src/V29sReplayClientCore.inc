// Boundary-only replay client observer. No engine calls or native writes.
struct SCClient {U32 id,offset,vtable,method;};
static const SCClient kSCClients[]={
    {1u,0x4E3A8u,0xE2B0CCu,1u},{2u,0x4E3E0u,0xE2AB20u,2u},
    {3u,0x4E388u,0xE2ADACu,0u},{7u,0x4E428u,0xE2A8D0u,0u}
};
struct SCNode {U32 pointer,valid,typeFlags,timeBits,bytes,next;};
struct SCView {
    U32 owner,self,recording,vtable,mode,clockBits,requestedBits,phase,transition,loading,countdown,vehiclePending;
    SCNode nodes[3];
    U32 recordingValid,backend,recordingVtable,ioMode,capacity,base,retained,filled,parse;
    U32 ringValid,freeBytes,usedBytes,unparsedBytes,filePosition,inFlightBytes,endOfData,active,eventHead;
};
struct SCRecord {U32 kind,index,sequence,thread,tick,caller,elapsedTicks,captureTicks;SCView view;};
struct SCState {U32 open,owner,self,recording,lastClock,sequence,lastTick;__int64 start;SCView lastView;};
struct SCThread {volatile long id;SCState states[4];};
static SCThread g_scThreads[8]={};
static SCRecord g_scRing[512]={};static U32 g_scHead=0,g_scTail=0;
static volatile long g_scStatus=0,g_scLock=0,g_scOpenCount=0,g_scSequence=0;
static volatile long g_scFastClear=0,g_scObserved=0,g_scBusy=0,g_scBegin=0,g_scEnd=0,g_scAbandoned=0,g_scHeld=0;
static volatile long g_scDropped=0,g_scThreadDrops=0,g_scUnknown=0,g_scReadFailures=0,g_scReadChecks=0,g_scClockFailures=0;
static volatile long g_scClientBusy[4]={};
static UPtr SCAddress(U32);static U32 SCThreadId();static U32 SCTick();static __int64 SCNow();
static SCView SCCapture(UPtr,UPtr,U32);
static U32 SCDuration(__int64 delta){return delta<=0?0u:(delta>0xFFFFFFFFLL?0xFFFFFFFFu:static_cast<U32>(delta));}
static bool SCQueue(const SCRecord& r) {
    if(InterlockedCompareExchange(&g_scLock,1,0)){InterlockedIncrement(&g_scDropped);return false;}
    const bool room=g_scHead-g_scTail<512u;
    if(room){g_scRing[g_scHead&511u]=r;++g_scHead;}else InterlockedIncrement(&g_scDropped);
    InterlockedExchange(&g_scLock,0);return room;
}
static bool SCPop(SCRecord& r) {
    if(InterlockedCompareExchange(&g_scLock,1,0))return false;
    const bool found=g_scHead!=g_scTail;if(found){r=g_scRing[g_scTail&511u];++g_scTail;}
    InterlockedExchange(&g_scLock,0);return found;
}
static SCThread* SCFindThread() {
    const U32 thread=SCThreadId();
    for(U32 i=0;i<8u;++i)if(static_cast<U32>(g_scThreads[i].id)==thread)return &g_scThreads[i];
    for(U32 i=0;i<8u;++i)if(!InterlockedCompareExchange(&g_scThreads[i].id,static_cast<long>(thread),0))return &g_scThreads[i];
    InterlockedIncrement(&g_scThreadDrops);return nullptr;
}
static int SCIdentify(U32 method,UPtr owner,UPtr self) {
    if(!owner || !self)return -1;
    // EDI is the manager in the original 7379D0 subscriber loop. The bridge
    // separately saves input ECX: the vehicle predicate can return a scratch
    // timestamp in ECX. EDI is callee-saved and preserved by these predicates.
    for(U32 i=0;i<4u;++i)if(method==kSCClients[i].method && owner+kSCClients[i].offset==self &&
        Read32(self)==SCAddress(kSCClients[i].vtable))return static_cast<int>(i);
    return -1;
}
static void SCEmit(SCState& state,U32 kind,U32 index,UPtr caller,U32 now,__int64 stamp,bool capture) {
    SCRecord r={};r.kind=kind;r.index=index;r.sequence=state.sequence;r.thread=SCThreadId();r.tick=now;r.caller=caller;
    r.elapsedTicks=SCDuration(stamp-state.start);
    if(capture){const __int64 before=SCNow();state.lastView=SCCapture(state.owner,state.self,index);r.captureTicks=SCDuration(SCNow()-before);}
    r.view=state.lastView;SCQueue(r);state.lastTick=now;
}
extern "C" __declspec(noinline) void __cdecl SCObserve(U32* frame,U32 method) {
    // Frame: pushad[0..7], native flags[8], saved input self[9], caller[10].
    if(g_scStatus!=1 || frame[10]!=SCAddress(0x7379F1u))return;
    const UPtr owner=frame[0],self=frame[9];const int identified=SCIdentify(method,owner,self);
    if(identified<0){InterlockedIncrement(&g_scUnknown);return;}
    SCThread* thread=SCFindThread();if(!thread)return;const U32 index=static_cast<U32>(identified);
    SCState& state=thread->states[index];const bool busy=(frame[7]&255u)!=0u;InterlockedIncrement(&g_scObserved);
    if(!busy && !state.open)return; // No clock/readability calls for unrelated clear clients.
    const U32 recording=Read32(owner+0x4E58Cu),clock=Read32(owner+0x4E9A0u),now=SCTick();const __int64 stamp=SCNow();
    if(state.open && (state.owner!=owner || state.self!=self || state.recording!=recording || clock<state.lastClock)) {
        SCEmit(state,3u,index,frame[10],now,stamp,false);state.open=0u;InterlockedDecrement(&g_scOpenCount);InterlockedIncrement(&g_scAbandoned);
    }
    if(busy) {
        InterlockedIncrement(&g_scBusy);InterlockedIncrement(&g_scClientBusy[index]);
        if(!state.open) {
            state.open=1u;state.owner=owner;state.self=self;state.recording=recording;state.start=stamp;
            state.sequence=static_cast<U32>(InterlockedIncrement(&g_scSequence));InterlockedIncrement(&g_scOpenCount);
            InterlockedIncrement(&g_scBegin);SCEmit(state,0u,index,frame[10],now,stamp,true);
        }else if(now-state.lastTick>=1000u){InterlockedIncrement(&g_scHeld);SCEmit(state,2u,index,frame[10],now,stamp,true);}
    }else if(state.open) {
        InterlockedIncrement(&g_scEnd);SCEmit(state,1u,index,frame[10],now,stamp,true);
        state.open=0u;InterlockedDecrement(&g_scOpenCount);
    }
    state.lastClock=clock;
}

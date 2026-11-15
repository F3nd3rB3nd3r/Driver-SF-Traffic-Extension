// The native object and both caller frames remain 0x264-byte legacy views.
// Only these seven audited boundaries access the complete, separate payload.
#if defined(DSF_V29S_GENERIC_LIMITS)
#define kReplayTraffic (g_limits.requests)
#define kReplayRetained (g_limits.retained)
#else
static const U32 kReplayTraffic=150u,kReplayRetained=256u;
#endif
static const U32 kReplayGuard=0x29150ACDu;
struct ReplayPair {U32 id,refs;};
#if defined(DSF_V29S_GENERIC_LIMITS)
struct ReplayData {U32 count;U32* ids;U32 retained;ReplayPair* pairs;U32 extra;U32* guard;};
static U32 ReplayStorageBytes(){return kReplayTraffic*4u+kReplayRetained*8u+16u;}
static void ReplayStorageInit(ReplayData& d,void* storage){d.ids=static_cast<U32*>(storage);
    d.pairs=reinterpret_cast<ReplayPair*>(d.ids+kReplayTraffic);d.guard=reinterpret_cast<U32*>(d.pairs+kReplayRetained);
    for(U32 i=0;i<4u;++i)d.guard[i]=kReplayGuard;}
#else
struct ReplayData {U32 count,ids[150],retained;ReplayPair pairs[256];U32 extra;};
static_assert(sizeof(ReplayData)==2660u,"snapshot payload layout");
#endif
static void ReplayCopy(ReplayData& to,const ReplayData& from){
#if defined(DSF_V29S_GENERIC_LIMITS)
    to.count=from.count;to.retained=from.retained;to.extra=from.extra;
    for(U32 i=0;i<from.count;++i)to.ids[i]=from.ids[i];for(U32 i=0;i<from.retained;++i)to.pairs[i]=from.pairs[i];
#else
    to=from;
#endif
}
struct ReplaySlot {ReplaySlot* next;UPtr owner;U32 guardA,valid;ReplayData data;U32 guardB;};
struct ReplayTemp {ReplayTemp* next;UPtr base;U32 thread,stage,guardA;ReplayData data;U32 guardB;};
static ReplaySlot* g_replaySlots=nullptr;
static ReplayTemp* g_replayTemps=nullptr;
static volatile long g_replayLock=0,g_replayOwners=0,g_replayTempsAllocated=0,g_replayActive=0;
static volatile long g_replayCaptures=0,g_replayCached=0,g_replaySaves=0,g_replayLoads=0;
static volatile long g_replayImports=0,g_replayRestores=0,g_replayDeferred=0,g_replayInvalid=0;
static volatile long g_replayPeakTraffic=0,g_replayPeakRetained=0;
static void ReplayFatal(const char*,UPtr,U32);
static void* ReplayAllocate(U32);
static U32 ReplayThread();
static UPtr ReplaySpooler();
static bool ReplayIdle(UPtr);
static U32 ReplayRefs(UPtr,U32);
static void ReplayRequest(UPtr,const U32*,U32);
static void ReplayReference(UPtr,U32,bool);
static void ReplayExtra(UPtr,U32);
static void ReplayAppend(UPtr,const void*,U32);
static void ReplayEvent(const char*,UPtr,const ReplayData&);
static void ReplayLock(){while(InterlockedExchange(&g_replayLock,1)){} }
static void ReplayUnlock(){InterlockedExchange(&g_replayLock,0);}
static void ReplayFail(const char* reason,UPtr p,U32 n){InterlockedIncrement(&g_replayInvalid);ReplayFatal(reason,p,n);}
static bool ReplayValid(const ReplayData& d) {
#if defined(DSF_V29S_GENERIC_LIMITS)
    if(!d.ids || !d.pairs || !d.guard)return false;for(U32 i=0;i<4u;++i)if(d.guard[i]!=kReplayGuard)return false;
#endif
    if(d.count>kReplayTraffic || d.retained>kReplayRetained)return false;
    for(U32 i=0;i<d.count;++i)if(!d.ids[i] || d.ids[i]>65535u)return false;
    for(U32 i=0;i<d.retained;++i)if(!d.pairs[i].id || d.pairs[i].id>65535u)return false;
    return true;
}
static U32 ReplayHash(const ReplayData& d){U32 h=2166136261u;for(U32 i=0;i<d.count;++i)h=(h^d.ids[i])*16777619u;return h;}
static void ReplayPeak(const ReplayData& d){ParkPeak(&g_replayPeakTraffic,d.count);ParkPeak(&g_replayPeakRetained,d.retained);}
static ReplaySlot* ReplayFindSlot(UPtr owner,bool create) {
    for(ReplaySlot* s=g_replaySlots;s;s=s->next)if(s->owner==owner)return s;
    if(!create)return nullptr;
    U32 bytes=sizeof(ReplaySlot);
#if defined(DSF_V29S_GENERIC_LIMITS)
    bytes+=ReplayStorageBytes();
#endif
    ReplaySlot* s=static_cast<ReplaySlot*>(ReplayAllocate(bytes));if(!s)return nullptr;
#if defined(DSF_V29S_GENERIC_LIMITS)
    ReplayStorageInit(s->data,s+1);
#endif
    s->owner=owner;s->guardA=s->guardB=kReplayGuard;s->next=g_replaySlots;g_replaySlots=s;
    InterlockedIncrement(&g_replayOwners);return s;
}
static bool ReplaySlotOkay(ReplaySlot* s){return s && s->guardA==kReplayGuard && s->guardB==kReplayGuard && s->valid && ReplayValid(s->data);}
// A temp is scoped to one synchronous native save or load. Recycle completed
// temps, including across different stack addresses. No fixed arena registry.
static ReplayTemp* ReplayBegin(UPtr base,U32 stage) {
    const U32 thread=ReplayThread();ReplayLock();ReplayTemp* free=nullptr;
    for(ReplayTemp* t=g_replayTemps;t;t=t->next) {
        bool collision=t->stage && t->thread==thread && t->base==base;
#if defined(DSF_V29S_GENERIC_LIMITS)
        if((t->stage==5u && stage==6u) || (t->stage==6u && stage==5u))collision=false;
#endif
        if(collision){ReplayUnlock();ReplayFail("tempAlreadyActive",base,t->stage);return nullptr;}
        if(!t->stage)free=t;
    }
    if(!free) {
        U32 bytes=sizeof(ReplayTemp);
#if defined(DSF_V29S_GENERIC_LIMITS)
        bytes+=ReplayStorageBytes();
#endif
        free=static_cast<ReplayTemp*>(ReplayAllocate(bytes));
#if defined(DSF_V29S_GENERIC_LIMITS)
        if(free)ReplayStorageInit(free->data,free+1);
#endif
        if(free){free->next=g_replayTemps;g_replayTemps=free;InterlockedIncrement(&g_replayTempsAllocated);}
    }
    if(!free){ReplayUnlock();ReplayFail("tempAllocation",base,stage);return nullptr;}
    free->base=base;free->thread=thread;free->stage=stage;free->guardA=free->guardB=kReplayGuard;
    ReplayUnlock();InterlockedIncrement(&g_replayActive);return free;
}
static ReplayTemp* ReplayTempAt(UPtr base,U32 stage) {
    ReplayTemp* result=nullptr;const U32 thread=ReplayThread();ReplayLock();
    for(ReplayTemp* t=g_replayTemps;t;t=t->next)if(t->stage && t->base==base && t->thread==thread){result=t;break;}
    const bool okay=result && result->stage==stage && result->guardA==kReplayGuard && result->guardB==kReplayGuard;
    ReplayUnlock();if(!okay){ReplayFail("tempMissingOrGuard",base,stage);return nullptr;}return result;
}
static void ReplayFinish(ReplayTemp* t){ReplayLock();t->stage=0;ReplayUnlock();InterlockedDecrement(&g_replayActive);}
static void ReplayMirror(UPtr out,const ReplayData& d) {
    // Preserve real counts, but mirror only the original inline capacities.
    // All full-array consumers below are redirected; no 150-DWORD native read.
    Write32(out,d.count);for(U32 i=0;i<50u;++i)Write32(out+4u+i*4u,i<d.count?d.ids[i]:0u);
    Write32(out+0xCCu,d.retained);
    for(U32 i=0;i<50u;++i){Write32(out+0xD0u+i*8u,i<d.retained?d.pairs[i].id:0u);Write32(out+0xD4u+i*8u,i<d.retained?d.pairs[i].refs:0u);}
    Write32(out+0x260u,d.extra);
}
static bool ReplayMirrorMatches(UPtr base,const ReplayData& d) {
    if(!Readable(base,0x264u) || Read32(base)!=d.count || Read32(base+0xCCu)!=d.retained || Read32(base+0x260u)!=d.extra)return false;
    for(U32 i=0;i<d.count && i<50u;++i)if(Read32(base+4u+i*4u)!=d.ids[i])return false;
    for(U32 i=0;i<d.retained && i<50u;++i)if(Read32(base+0xD0u+i*8u)!=d.pairs[i].id || Read32(base+0xD4u+i*8u)!=d.pairs[i].refs)return false;
    return true;
}
static U32 WideReplayWalkBound(){
#if defined(DSF_V29S_WIDE_LIMITS)
return WideWorkNodes();
#else
return 4096u;
#endif
}
static bool ReplayNode(UPtr node,U32 bytes,U32 walks,UPtr head) {
    if(walks>=WideReplayWalkBound() || !Readable(node,bytes)){ReplayFail("listNodeOrCycle",head,walks);return false;}return true;
}
static bool ReplayAddRetained(ReplayData& d,U32 id,UPtr owner) {
    if(d.retained>=kReplayRetained){ReplayFail("retainedCapacity",owner,d.retained+1u);return false;}
    d.pairs[d.retained].id=id;d.pairs[d.retained].refs=0u;++d.retained;return true;
}
static bool ReplayCollectRetained(UPtr spooler,ReplayData& d) {
    d.retained=0u;UPtr head=spooler+0x998u,node=Read32(head+4u);U32 walks=0;
    while(node!=head) {
        if(!ReplayNode(node,0x14u,walks++,head))return false;
        if(static_cast<long>(Read32(node+0x10u))>0 && !ReplayAddRetained(d,Read32(node+8u),spooler))return false;
        node=Read32(node+4u);
    }
    head=spooler+0xD94u;node=Read32(head+4u);walks=0;
    while(node!=head) {
        if(!ReplayNode(node,0x14u,walks++,head))return false;
        if(!Read32(node+0xCu) && !ReplayAddRetained(d,Read32(node+8u),spooler))return false;
        node=Read32(node+4u);
    }
    // Third source in native 005A69B0, including its pending special model.
    if(Read32(spooler+0xDACu)==2u && !ReplayAddRetained(d,Read32(spooler+0xD9Cu),spooler))return false;
    return true;
}
static bool ReplayCollect(UPtr spooler,ReplayData& d) {
    d.count=0u;const UPtr head=spooler+0x330u;UPtr node=Read32(head+4u);U32 walks=0;
    while(node!=head) {
        if(!ReplayNode(node,10u,walks++,head))return false;
        if(d.count>=kReplayTraffic){ReplayFail("trafficCapacity",spooler,d.count+1u);return false;}
        d.ids[d.count++]=Read8(node+8u)|(static_cast<U32>(Read8(node+9u))<<8u);node=Read32(node+4u);
    }
    if(!ReplayCollectRetained(spooler,d))return false;
    for(U32 i=0;i<d.retained;++i)d.pairs[i].refs=ReplayRefs(spooler,d.pairs[i].id);
    d.extra=Read32(spooler+0xDB0u);return true;
}
extern "C" __declspec(noinline) void __fastcall ReplayCapture(void* self,void*,void* output) {
    const UPtr owner=reinterpret_cast<UPtr>(self),out=reinterpret_cast<UPtr>(output);
    if(!Readable(owner,0x270u) || !Readable(out,0x264u)){ReplayFail("capturePointers",owner,out);return;}
    ReplayTemp* t=ReplayBegin(out,1u);if(!t)return;
    if(Read32(owner+8u)) {
        ReplayLock();ReplaySlot* s=ReplayFindSlot(owner,false);
        const bool ok=ReplaySlotOkay(s) && ReplayMirrorMatches(owner+0xCu,s->data);
        if(ok)ReplayCopy(t->data,s->data);ReplayUnlock();
        if(!ok){ReplayFinish(t);ReplayFail("captureCache",owner,0u);return;}InterlockedIncrement(&g_replayCached);
    }else if(!ReplayCollect(ReplaySpooler(),t->data)){ReplayFinish(t);return;}
    if(!ReplayValid(t->data)){ReplayFinish(t);ReplayFail("captureData",owner,0u);return;}
    ReplayMirror(out,t->data);ReplayPeak(t->data);InterlockedIncrement(&g_replayCaptures);ReplayEvent("capture",owner,t->data);
}
extern "C" __declspec(noinline) void __fastcall ReplaySaveTraffic(void* stream,void*,const void* source,U32 bytes) {
    const UPtr base=reinterpret_cast<UPtr>(source)-4u;ReplayTemp* t=ReplayTempAt(base,1u);if(!t)return;
    if(!ReplayValid(t->data) || bytes!=t->data.count*4u || !ReplayMirrorMatches(base,t->data)){ReplayFail("saveTraffic",base,bytes);return;}
    ReplayLock();t->stage=2u;ReplayUnlock();ReplayAppend(reinterpret_cast<UPtr>(stream),t->data.ids,bytes);
}
extern "C" __declspec(noinline) void __fastcall ReplaySaveRetained(void* stream,void*,const void* source,U32 bytes) {
    const UPtr base=reinterpret_cast<UPtr>(source)-0xD0u;ReplayTemp* t=ReplayTempAt(base,2u);if(!t)return;
    if(bytes!=t->data.retained*8u || !ReplayMirrorMatches(base,t->data)){ReplayFail("saveRetained",base,bytes);return;}
    ReplayAppend(reinterpret_cast<UPtr>(stream),t->data.pairs,bytes);ReplayEvent("save",base,t->data);
    InterlockedIncrement(&g_replaySaves);ReplayFinish(t);
}
extern "C" __declspec(noinline) void* __cdecl ReplayLoadTraffic(void* destination,const void* source,U32 bytes) {
    const UPtr base=reinterpret_cast<UPtr>(destination)-4u;
    if(bytes>kReplayTraffic*4u || (bytes&3u) || !Readable(base,0x264u) || Read32(base)!=bytes/4u || (bytes && !Readable(reinterpret_cast<UPtr>(source),bytes))) {
        ReplayFail("loadTrafficBounds",base,bytes);return destination;
    }
    ReplayTemp* t=ReplayBegin(base,3u);if(!t)return destination;
    t->data.count=bytes/4u;t->data.retained=0u;t->data.extra=0xFFFFFFFFu;
    const U32* ids=static_cast<const U32*>(source);for(U32 i=0;i<t->data.count;++i){t->data.ids[i]=ids[i];if(i<50u)Write32(base+4u+i*4u,ids[i]);}
    return destination;
}
extern "C" __declspec(noinline) void* __cdecl ReplayLoadRetained(void* destination,const void* source,U32 bytes) {
    const UPtr base=reinterpret_cast<UPtr>(destination)-0xD0u;ReplayTemp* t=ReplayTempAt(base,3u);if(!t)return destination;
    if(bytes>kReplayRetained*8u || (bytes&7u) || Read32(base+0xCCu)!=bytes/8u || (bytes && !Readable(reinterpret_cast<UPtr>(source),bytes))) {
        ReplayFail("loadRetainedBounds",base,bytes);return destination;
    }
    t->data.retained=bytes/8u;const ReplayPair* pairs=static_cast<const ReplayPair*>(source);
    for(U32 i=0;i<t->data.retained;++i){t->data.pairs[i]=pairs[i];if(i<50u){Write32(base+0xD0u+i*8u,pairs[i].id);Write32(base+0xD4u+i*8u,pairs[i].refs);}}
    ReplayLock();t->stage=4u;ReplayUnlock();InterlockedIncrement(&g_replayLoads);return destination;
}
extern "C" __declspec(noinline) void __fastcall ReplayRestore(void* self,void*) {
#if defined(DSF_REPLAY_HITCH_RUNTIME)
    RHScope timing(RHRestore,reinterpret_cast<UPtr>(self));
#endif
    const UPtr owner=reinterpret_cast<UPtr>(self);
#if defined(DSF_V29S_GENERIC_LIMITS)
    ReplayTemp* saved=ReplayBegin(owner,6u);if(!saved)return;ReplayData& d=saved->data;
#else
    ReplayData d={};
#endif
    ReplayLock();ReplaySlot* s=ReplayFindSlot(owner,false);
    const bool ok=ReplaySlotOkay(s) && ReplayMirrorMatches(owner+0xCu,s->data);if(ok)ReplayCopy(d,s->data);ReplayUnlock();
    if(!ok){
#if defined(DSF_V29S_GENERIC_LIMITS)
        ReplayFinish(saved);
#endif
        ReplayFail("restoreCache",owner,0u);return;}const UPtr spooler=ReplaySpooler();
    // Match native order: request traffic, enumerate current retained IDs,
    // release current references, restore recorded references, extra state.
    ReplayTemp* scratch=ReplayBegin(owner,5u);if(!scratch){
#if defined(DSF_V29S_GENERIC_LIMITS)
        ReplayFinish(saved);
#endif
        return;}
    if(d.count)ReplayRequest(spooler,d.ids,d.count);
    if(!ReplayCollectRetained(spooler,scratch->data)){ReplayFinish(scratch);
#if defined(DSF_V29S_GENERIC_LIMITS)
        ReplayFinish(saved);
#endif
        return;}
    for(U32 i=0;i<scratch->data.retained;++i){const U32 id=scratch->data.pairs[i].id,n=ReplayRefs(spooler,id);for(U32 j=0;j<n;++j)ReplayReference(spooler,id,false);}
    ReplayFinish(scratch);
    for(U32 i=0;i<d.retained;++i)for(U32 j=0;j<d.pairs[i].refs;++j)ReplayReference(spooler,d.pairs[i].id,true);
    if(d.extra!=0xFFFFFFFFu)ReplayExtra(spooler,d.extra);
    Write32(owner+8u,2u);InterlockedIncrement(&g_replayRestores);ReplayEvent("restore",owner,d);
#if defined(DSF_V29S_GENERIC_LIMITS)
    ReplayFinish(saved);
#endif
}
extern "C" __declspec(noinline) void __fastcall ReplayImport(void* self,void*,const void* input) {
#if defined(DSF_REPLAY_HITCH_RUNTIME)
    RHScope timing(RHImport,reinterpret_cast<UPtr>(self));
#endif
    const UPtr owner=reinterpret_cast<UPtr>(self),base=reinterpret_cast<UPtr>(input);ReplayTemp* t=ReplayTempAt(base,4u);if(!t)return;
    t->data.extra=Read32(base+0x260u);
    if(!ReplayValid(t->data) || !ReplayMirrorMatches(base,t->data) || !Readable(owner,0x270u)){ReplayFail("importData",base,0u);return;}
    ReplayLock();ReplaySlot* s=ReplayFindSlot(owner,true);const bool okay=s && s->guardA==kReplayGuard && s->guardB==kReplayGuard;
    if(okay){ReplayCopy(s->data,t->data);s->valid=1u;}ReplayUnlock();
    if(!okay){ReplayFail("cacheAllocationOrGuard",owner,0u);return;}
    ReplayMirror(owner+0xCu,t->data);ReplayPeak(t->data);ReplayEvent("import",owner,t->data);
    InterlockedIncrement(&g_replayImports);ReplayFinish(t);
    if(ReplayIdle(ReplaySpooler()))ReplayRestore(self,nullptr);
    else{Write32(owner+8u,1u);InterlockedIncrement(&g_replayDeferred);}
#if defined(DSF_V29S_REPLAY_PAUSE)
    PauseOnImport(owner);
#endif
}

// Reuse the current native-image fixture support, not the game's entry point.
#define WideTestMain UnusedWideTestMain
#include "WideLimitsTest.cpp"
#undef WideTestMain

static UPtr SourceIds(U32 count,U32 seed){
    const UPtr h=Alloc(0x300u),end=h+0x264u,nodes=Alloc((count?count:1u)*12u);EmptyList(h,0x264u);
    Write32(h+8u,count);Write32(end,count?nodes+(count-1u)*12u:end);Write32(end+4u,count?nodes:end);
    for(U32 i=0;i<count;++i){const UPtr at=nodes+i*12u;Write32(at,i?at-12u:end);Write32(at+4u,i+1u<count?at+12u:end);Write32(at+8u,seed+i);}
    return h;
}
static void NativeRetire(UPtr owner,U32 count,U32 seed){
    typedef void (__thiscall* Merge)(void*,UPtr);
    reinterpret_cast<Merge>(Address(0x5A7D50u))(reinterpret_cast<void*>(owner+0x338u),SourceIds(count,seed));
}
static void NativeAppend20(U32 kind,UPtr owner,U32 value){
    const UPtr h=owner+kTrafficQueueOffsets[kind];U32 payload[3]={value,0x12340000u+value,0xABCD0000u+value};
    // The fixture maps the image at20000000; the production bridge's caller
    // filter uses retail400000 addresses. Exercise its helper explicitly, then
    // run the actual20-byte allocator and append instructions in the image.
    CapacityTrafficEntry(h,kind==1u?0x5A7E3Du:0x5A7E6Du);
    typedef UPtr (__thiscall* Append)(void*,const U32*);
    const UPtr node=reinterpret_cast<Append>(Address(kind==1u?0x5A7E30u:0x5A7E60u))(reinterpret_cast<void*>(h),payload);
    CHECK(node && Read32(node+8u)==payload[0] && Read32(node+12u)==payload[1] && Read32(node+16u)==payload[2]);
}
static UPtr Owner(){
    const UPtr o=Alloc(0x4000u);
    for(U32 k=0;k<3u;++k){const UPtr h=o+kTrafficQueueOffsets[k];EmptyList(h,12u+50u*kTrafficQueueStrides[k]);}
    const UPtr root=Alloc(0x1000u);Write32(Address(kRootPtr),root);Write32(root+0x214u,o);return o;
}
static void VerifyQueue(U32 kind,UPtr owner,U32 active){
    const char* why=nullptr;TrafficQueueState state={};const TrafficQueueArena* a=TrafficQueueFind(kind,owner);
    CHECK(TrafficQueueValidate(kind,owner,a,state,why));CHECK(state.active==active && TrafficQueueGuards(a));
}
static void Drain(U32 kind,UPtr owner){
    const UPtr h=owner+kTrafficQueueOffsets[kind],end=h+12u+50u*kTrafficQueueStrides[kind];
    while(Read32(h+8u))Recycle(h,Read32(end+4u),12u+50u*kTrafficQueueStrides[kind]);
}
static U32 CountRecords(U32 kind,UPtr owner){U32 count=0;for(auto* a=g_trafficQueueArenas;a;a=a->older)if(a->kind==kind && a->owner==owner)++count;return count;}
static UPtr g_trafficFatalCanary=0;static U32 g_trafficFatalExpected=0;
static void TrafficFailure(const char* text){CHECK(WideContains(text,"traffic queue"));if(g_trafficFatalCanary)CHECK(Read32(g_trafficFatalCanary)==g_trafficFatalExpected);Say("PASS traffic fail-before-write\n");}
static void RunTrafficQueues(U32 n){
    // Isolate the original bug: the51st native12-byte node writes into the
    // neighbouring header. Do this before the hooks, in disposable memory.
    UPtr old=Owner();const U32 before=Read32(old+0x5A4u);NativeRetire(old,51u,1000u);CHECK(Read32(old+0x5A4u)!=before);
    if(WideContains(GetCommandLineA()," signature")){
        const UPtr site=Address(0x83B170u);*reinterpret_cast<U8*>(site)=0xCCu;
        const U32 retire=Read32(Address(0x5A7DD1u)),audio=Read32(Address(0x849870u));
        CHECK(!CapacityInstall());CHECK(Read32(Address(0x5A7DD1u))==retire && Read32(Address(0x849870u))==audio);
        Say("PASS traffic hook signature refuses before partial installation\n");return;
    }
    CHECK(CapacityInstall());g_capacityHooks=3;g_capacityFile=GetStdHandle(0xFFFFFFF5u);CapacityStartLine();
    const UPtr owner=Owner();U32 neighbours[9]={};for(U32 i=0;i<9u;++i)neighbours[i]=Read32(owner+0x5A4u+i*4u);
    // Reproduce the supplied transition's146 retirements, then shrink/grow/drain.
    const U32 transitions[]={146u,18u,146u,0u,152u,0u,146u};
    for(U32 count:transitions){NativeRetire(owner,count,1000u);VerifyQueue(0u,owner,count);for(U32 i=0;i<9u;++i)CHECK(Read32(owner+0x5A4u+i*4u)==neighbours[i]);}
    TrafficQueueArena* first=TrafficQueueFind(0u,owner);CHECK(first && first->nodes && CountRecords(0u,owner)==1u);
    CHECK(g_trafficQueueStats[0].recycles>0 && !g_trafficQueueStats[0].failures && !g_trafficQueueStats[0].guardBad);
    for(U32 kind=0;kind<3u;++kind){
        const UPtr o=Owner(),h=o+kTrafficQueueOffsets[kind];const U32 stride=kTrafficQueueStrides[kind],endOffset=12u+50u*stride;
        const UPtr after=h+endOffset+8u;Write32(after,0x12345678u);
        const U32 capacity=TrafficQueueCapacity();
        if(!kind)NativeRetire(o,capacity,2000u);else for(U32 i=0;i<capacity;++i)NativeAppend20(kind,o,2000u+i);
        VerifyQueue(kind,o,capacity);CHECK(Read32(after)==0x12345678u);
        const char* why=nullptr;CHECK(!TrafficQueuePrepare(kind,o,why) && WideContains(why,"configuredCapacity"));CHECK(Read32(after)==0x12345678u);
        TrafficQueueArena* arena=TrafficQueueFind(kind,o);const UPtr savedNodes=arena->nodes;
        Drain(kind,o);VerifyQueue(kind,o,0u);
        if(!kind)NativeRetire(o,capacity,3000u);else for(U32 i=0;i<capacity;++i)NativeAppend20(kind,o,3000u+i);
        CHECK(TrafficQueueFind(kind,o)==arena && arena->nodes==savedNodes);VerifyQueue(kind,o,capacity);
        // Same address, new constructor: never reuse a previous generation's
        // externally referenced nodes, and retain its old allocation/guards.
        EmptyList(h,endOffset);if(!kind)NativeRetire(o,51u,5000u);else for(U32 i=0;i<51u;++i)NativeAppend20(kind,o,5000u+i);
        TrafficQueueArena* next=TrafficQueueFind(kind,o);CHECK(next!=arena && next->generation==arena->generation+1u && next->nodes!=savedNodes);
        CHECK(TrafficQueueGuards(arena));VerifyQueue(kind,o,51u);CHECK(Read32(after)==0x12345678u);
        // Corrupt inputs must fail before changing the adjacent native memory.
        const UPtr saved=Read32(h);Write32(h,next->nodes+1u);CHECK(!TrafficQueuePrepare(kind,o,why));Write32(h,saved);
        Write32(next->storage,0u);CHECK(!TrafficQueuePrepare(kind,o,why) && WideContains(why,"arenaGuard"));Write32(next->storage,kTrafficQueueGuard);
        VerifyQueue(kind,o,51u);CHECK(Read32(after)==0x12345678u);
    }
    // More than the historical eight owner slots, and a recycled/active alias.
    for(U32 i=0;i<12u;++i){UPtr o=Owner();NativeRetire(o,60u,1000u);VerifyQueue(0u,o,60u);}
    UPtr broken=Owner();NativeRetire(broken,60u,1000u);UPtr h=broken+0x338u;Write32(h,Read32(h+0x268u));Write32(h+4u,1u);
    const char* why=nullptr;CHECK(!TrafficQueuePrepare(0u,broken,why));
    // Actual native guard adapter/fatal path, not just the predicate.
    if(WideContains(GetCommandLineA()," fatal")){
        UPtr o=Owner();NativeRetire(o,TrafficQueueCapacity(),1000u);g_wideFailureSink=&TrafficFailure;
        g_trafficFatalCanary=o+0x5A4u;g_trafficFatalExpected=Read32(g_trafficFatalCanary);
        NativeRetire(o,TrafficQueueCapacity()+1u,1000u);CHECK(false);
    }
    Say("PASS traffic queues requested=");Number(n);Say(" capacity=");Number(TrafficQueueCapacity());Say(" checks=");Number(g_checks);Say("\n");
}
extern "C" void TrafficTestMain(){
    SetErrorMode(3u);U32 n=Argument();g_limits={n,WideMax(192u,n+32u),2048,64,1,WideMax(256u,n),1,true,"configured",0,0,0,0,0,0,0,0,0};
    g_limits=GenericValidate(g_limits);CHECK(g_limits.valid);MapNativeImage();g_wideFailureSink=&TrafficFailure;
    RunTrafficQueues(n);ExitProcess(0u);
}

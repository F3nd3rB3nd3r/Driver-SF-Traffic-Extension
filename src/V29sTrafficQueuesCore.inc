// The native sentinels stay in the owner. Only the fresh-node cursor moves;
// ordinary native unlink/recycle/merge code keeps owning all links and payloads.
// Keep old generation storage alive: pending engine pointers must never dangle.
struct TrafficQueueArena {
    TrafficQueueArena* older;UPtr owner,storage,nodes;U32 kind,generation,used,nativeIssued,externalIssued;
};
struct TrafficQueueState {UPtr fresh;U32 active,recycled,nativeIssued,externalIssued;};
struct TrafficQueueStats {
    volatile long checks,generations,buffers,redirects,recycles,resets,failures,guardBad,peak;
    UPtr owner;U32 active,recycled,issued;
};
static TrafficQueueArena* g_trafficQueueArenas=nullptr;
static TrafficQueueStats g_trafficQueueStats[3]={};
static volatile long g_trafficQueueLock=0;
static const U32 kTrafficQueueGuard=0x54513447u;
static const U32 kTrafficQueueOffsets[3]={0x338u,0x5A4u,0x9A0u};
static const U32 kTrafficQueueStrides[3]={12u,20u,20u};
static U32 TrafficQueueCapacity(){return WideWorkNodes();}
static bool TrafficQueueBad(const char*& why,const char* text){why=text;return false;}
static TrafficQueueArena* TrafficQueueFind(U32 kind,UPtr owner){
    for(TrafficQueueArena* a=g_trafficQueueArenas;a;a=a->older)if(a->owner==owner && a->kind==kind)return a;
    return nullptr;
}
static bool TrafficQueueGuards(const TrafficQueueArena* a){
    if(!a || !a->storage)return true;
    const U32 bytes=(TrafficQueueCapacity()-50u)*kTrafficQueueStrides[a->kind];
    for(U32 n=0;n<4u;++n)if(Read32(a->storage+n*4u)!=kTrafficQueueGuard || Read32(a->nodes+bytes+n*4u)!=kTrafficQueueGuard)return false;
    return true;
}
static bool TrafficQueueIndex(UPtr first,U32 stride,const TrafficQueueArena* a,UPtr node,U32& index){
    if(node>=first && node-first<50u*stride && (node-first)%stride==0u){index=(node-first)/stride;return true;}
    const U32 extra=TrafficQueueCapacity()-50u;
    if(a && a->nodes && node>=a->nodes && node-a->nodes<extra*stride && (node-a->nodes)%stride==0u){index=50u+(node-a->nodes)/stride;return true;}
    return false;
}
static bool TrafficQueueValidate(U32 kind,UPtr owner,const TrafficQueueArena* a,TrafficQueueState& s,const char*& why){
    const U32 stride=kTrafficQueueStrides[kind],capacity=TrafficQueueCapacity();
    const UPtr h=owner+kTrafficQueueOffsets[kind],first=h+12u,end=first+50u*stride;
    if(!owner || !Readable(h,12u+50u*stride+8u))return TrafficQueueBad(why,"headerUnreadable");
    if(!TrafficQueueGuards(a)){InterlockedIncrement(&g_trafficQueueStats[kind].guardBad);return TrafficQueueBad(why,"arenaGuard");}
    s.active=Read32(h+8u);s.recycled=Read32(h+4u);
    if(s.active>capacity || s.recycled>capacity-s.active)return TrafficQueueBad(why,"counts");
    WideScratchLease lease(((capacity+31u)/32u)*4u);U32* seen=static_cast<U32*>(lease.data);
    U32 index=0,count=0;UPtr previous=end,node=Read32(end+4u);
    while(node!=end){
        if(count>=s.active || !TrafficQueueIndex(first,stride,a,node,index) || (seen[index/32u]&(1u<<(index%32u))))return TrafficQueueBad(why,"activeChain");
        seen[index/32u]|=1u<<(index%32u);
        if(Read32(node)!=previous)return TrafficQueueBad(why,"backLink");
        previous=node;node=Read32(node+4u);++count;
    }
    if(count!=s.active || Read32(end)!=previous)return TrafficQueueBad(why,"sentinel");
    node=Read32(h);
    for(U32 n=0;n<s.recycled;++n){
        if(!TrafficQueueIndex(first,stride,a,node,index) || (seen[index/32u]&(1u<<(index%32u))))return TrafficQueueBad(why,"recycledChain");
        seen[index/32u]|=1u<<(index%32u);node=Read32(node);
    }
    s.fresh=node;s.nativeIssued=s.externalIssued=0u;
    if(node>=first && node<=end && (node-first)%stride==0u)s.nativeIssued=(node-first)/stride;
    else if(a && a->nodes && node>=a->nodes && node-a->nodes<=(capacity-50u)*stride && (node-a->nodes)%stride==0u){
        s.nativeIssued=50u;s.externalIssued=(node-a->nodes)/stride;
    }else return TrafficQueueBad(why,"freshCursor");
    if(s.active+s.recycled!=s.nativeIssued+s.externalIssued)return TrafficQueueBad(why,"issuedCount");
    for(U32 n=0;n<capacity;++n)if(seen[n/32u]&(1u<<(n%32u)))
        if((n<50u && n>=s.nativeIssued) || (n>=50u && n-50u>=s.externalIssued))return TrafficQueueBad(why,"unissuedNode");
    return true;
}
static bool TrafficQueuePrepareLocked(U32 kind,UPtr owner,const char*& why){
    TrafficQueueStats& stats=g_trafficQueueStats[kind];InterlockedIncrement(&stats.checks);
    const U32 stride=kTrafficQueueStrides[kind],capacity=TrafficQueueCapacity();
    const UPtr h=owner+kTrafficQueueOffsets[kind],first=h+12u,end=first+50u*stride;
    if(!owner || !Readable(h,12u+50u*stride+8u))return TrafficQueueBad(why,"headerUnreadable");
    TrafficQueueArena* a=TrafficQueueFind(kind,owner);
    const bool cold=Read32(h)==first && !Read32(h+4u) && !Read32(h+8u) && Read32(end)==end && Read32(end+4u)==end;
    const U32 generation=a?a->generation:0u;
    if(a && cold && a->used){a=nullptr;InterlockedIncrement(&stats.resets);}
    if(!a){
        if(generation==0x7FFFFFFFu)return TrafficQueueBad(why,"generationOverflow");
        a=static_cast<TrafficQueueArena*>(WideAllocate(sizeof(TrafficQueueArena)));
        a->owner=owner;a->kind=kind;a->generation=generation+1u;a->older=g_trafficQueueArenas;g_trafficQueueArenas=a;
        InterlockedIncrement(&stats.generations);
    }
    TrafficQueueState state={};if(!TrafficQueueValidate(kind,owner,a,state,why))return false;
    if(state.active>=capacity)return TrafficQueueBad(why,"configuredCapacity");
    if(!state.recycled && state.fresh==end){
        if(!a->storage){
            const U32 bytes=(capacity-50u)*stride;a->storage=reinterpret_cast<UPtr>(WideAllocate(bytes+32u));a->nodes=a->storage+16u;
            for(U32 n=0;n<4u;++n){Write32(a->storage+n*4u,kTrafficQueueGuard);Write32(a->nodes+bytes+n*4u,kTrafficQueueGuard);}
            InterlockedIncrement(&stats.buffers);
        }
        // There are no recycled nodes: replacing the header's fresh cursor
        // preserves every active node and the native sentinel.
        Write32(h,a->nodes);state.fresh=a->nodes;InterlockedIncrement(&stats.redirects);
    }
    if(!state.recycled && state.externalIssued==capacity-50u && state.fresh!=end)return TrafficQueueBad(why,"externalEnd");
    a->nativeIssued=state.nativeIssued;a->externalIssued=state.externalIssued;a->used=1u;
    stats.owner=owner;stats.active=state.active;stats.recycled=state.recycled;stats.issued=state.nativeIssued+state.externalIssued;
    if(state.active+1u>static_cast<U32>(stats.peak))stats.peak=static_cast<long>(state.active+1u);
    if(state.recycled)InterlockedIncrement(&stats.recycles);
    return true;
}
static __declspec(noinline) bool TrafficQueuePrepare(U32 kind,UPtr owner,const char*& why){
    if(kind>=3u)return TrafficQueueBad(why,"queueKind");
    while(InterlockedExchange(&g_trafficQueueLock,1)){}
    const bool okay=TrafficQueuePrepareLocked(kind,owner,why);
    if(!okay)InterlockedIncrement(&g_trafficQueueStats[kind].failures);
    InterlockedExchange(&g_trafficQueueLock,0);return okay;
}

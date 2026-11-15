static U32 WideSpoolNodes(){
#if defined(DSF_V29S_WIDE_LIMITS)
return WideMax(256u,g_limits.requests);
#else
return 256u;
#endif
}
// The request frame already contains150 inline nodes (sentinel ESP+728).
// Persistent spooler storage is keyed by owner, retained for process lifetime.
struct ReuseNode {UPtr previous,next;U32 value;};
#if defined(DSF_V29S_WIDE_LIMITS)
struct SpoolReuseArena {UPtr older,owner;U32 inlineIssued;U32 before[4];ReuseNode* nodes;U32* after;};
#else
struct SpoolReuseArena {UPtr older,owner;U32 inlineIssued;U32 before[4];ReuseNode nodes[256];U32 after[4];};
#endif
static SpoolReuseArena* g_spoolReuseHead=nullptr;
static const U32 kReuseGuard=0x52555331u;
static volatile long g_reuseLock=0,g_reuseAllocations=0,g_reuseSpoolChecks=0,g_reuseScratchChecks=0;
static volatile long g_reuseRedirects=0,g_reuseResets=0,g_reuseInvalid=0,g_reuseGuardBad=0,g_reuseAllocFailed=0;
static volatile long g_reuseLastOwner=0,g_reuseLastScratch=0,g_reuseSpoolIssuedPeak=0,g_reuseScratchPeak=0;
static bool ReuseBad(const char*& why,const char* text){why=text;InterlockedIncrement(&g_reuseInvalid);return false;}
static SpoolReuseArena* ReuseFind(UPtr owner){for(SpoolReuseArena* a=g_spoolReuseHead;a;a=reinterpret_cast<SpoolReuseArena*>(a->older))if(a->owner==owner)return a;return nullptr;}
struct ReuseListState {UPtr fresh,tailSlot;U32 nativeIssued,extraIssued,active,recycled;};
static bool ReuseIndex(UPtr first,U32 inlineCount,const SpoolReuseArena* a,UPtr node,U32& index) {
    if(node>=first && node-first<inlineCount*12u && !((node-first)%12u)){index=(node-first)/12u;return true;}
    const UPtr extra=a?reinterpret_cast<UPtr>(&a->nodes[0]):0u;
    if(a && node>=extra && node-extra<WideSpoolNodes()*12u && !((node-extra)%12u)){index=inlineCount+(node-extra)/12u;return true;}
    return false;
}
static bool ReuseValidate(UPtr h,U32 inlineCount,const SpoolReuseArena* a,ReuseListState& state,const char*& why) {
    const UPtr first=h+12u,end=first+inlineCount*12u;
    if(!Readable(h,20u+inlineCount*12u))return ReuseBad(why,"headerUnreadable");
    if(a)for(U32 n=0;n<4u;++n)if(a->before[n]!=kReuseGuard || a->after[n]!=kReuseGuard){
        why="arenaGuard";InterlockedIncrement(&g_reuseGuardBad);return false;}
    state.active=Read32(h+8u);state.recycled=Read32(h+4u);
    if(state.active>kWideRequestNodes || state.recycled>kWideRequestNodes || state.active+state.recycled>kWideRequestNodes)return ReuseBad(why,"counts");

#if defined(DSF_V29S_WIDE_LIMITS)
WideScratchLease seenLease(((inlineCount+WideSpoolNodes()+31u)/32u)*4u);U32* seen=static_cast<U32*>(seenLease.data);U32 count=0,index=0;
#else
U32 seen[13]={},count=0,index=0;
#endif
UPtr previous=end,node=Read32(end+4u);
    while(node!=end) {
        if(count>=state.active || !ReuseIndex(first,inlineCount,a,node,index) || (seen[index/32u]&(1u<<(index%32u))))return ReuseBad(why,"activeChain");
        seen[index/32u]|=1u<<(index%32u);if(Read32(node)!=previous)return ReuseBad(why,"backLink");
        previous=node;node=Read32(node+4u);++count;
    }
    if(count!=state.active || Read32(end)!=previous)return ReuseBad(why,"sentinel");
    node=Read32(h);state.tailSlot=h;
    for(U32 n=0;n<state.recycled;++n) {
        if(!ReuseIndex(first,inlineCount,a,node,index) || (seen[index/32u]&(1u<<(index%32u))))return ReuseBad(why,"recycledChain");
        seen[index/32u]|=1u<<(index%32u);state.tailSlot=node;node=Read32(node);
    }
    state.fresh=node;state.nativeIssued=state.extraIssued=0u;
    const UPtr extra=a?reinterpret_cast<UPtr>(&a->nodes[0]):0u;
    if(node>=first && node<=end && !((node-first)%12u))state.nativeIssued=(node-first)/12u;
    else if(a && a->inlineIssued<=inlineCount && node>=extra && node<=extra+WideSpoolNodes()*12u && !((node-extra)%12u)) {
        state.nativeIssued=a->inlineIssued;state.extraIssued=(node-extra)/12u;
    }else return ReuseBad(why,"freshCursor");
    if(state.active+state.recycled!=state.nativeIssued+state.extraIssued)return ReuseBad(why,"issuedCount");
    for(U32 n=0;n<inlineCount+WideSpoolNodes();++n)if(seen[n/32u]&(1u<<(n%32u))) {
        if((n<inlineCount && n>=state.nativeIssued) || (n>=inlineCount && n-inlineCount>=state.extraIssued))return ReuseBad(why,"unissuedNode");
    }
    return true;
}
static bool ReusePrepareSpool(UPtr owner,const char*& why) {
    InterlockedIncrement(&g_reuseSpoolChecks);g_reuseLastOwner=static_cast<long>(owner);
    if(g_sScratchHook!=2 || !owner || !Readable(owner,0x338u))return ReuseBad(why,"spoolContext");
    const U32 requests=Read32(owner+0xC8u);if(requests>kWideRequestNodes)return ReuseBad(why,"requestBound");
    const UPtr h=owner+0xCCu;SpoolReuseArena* a=ReuseFind(owner);ReuseListState state={};
    if(!ReuseValidate(h,50u,a,state,why))return false;
    if(state.active>static_cast<U32>(g_sSpool0High))g_sSpool0High=static_cast<long>(state.active);
    // Native merge copies payload values, never source-node ownership. Attach
    // only when this header's active/free chains contain native inline nodes.
    const bool inlineTail=state.fresh>=h+12u && state.fresh<=h+0x264u;
    if(inlineTail && requests>50u) {
        if(!a) {
            a=reinterpret_cast<SpoolReuseArena*>(VirtualAlloc(nullptr,sizeof(SpoolReuseArena),kMemCommitReserve,kPageReadWrite));
            if(!a){why="arenaAllocation";InterlockedIncrement(&g_reuseAllocFailed);return false;}
#if defined(DSF_V29S_WIDE_LIMITS)
            a->nodes=static_cast<ReuseNode*>(WideAllocate(WideSpoolNodes()*12u+16u));a->after=reinterpret_cast<U32*>(a->nodes+WideSpoolNodes());
#endif
            a->owner=owner;a->older=reinterpret_cast<UPtr>(g_spoolReuseHead);
            for(U32 n=0;n<4u;++n)a->before[n]=a->after[n]=kReuseGuard;
            g_spoolReuseHead=a;InterlockedIncrement(&g_reuseAllocations);InterlockedIncrement(&g_sSpool0Used);
        }else InterlockedIncrement(&g_reuseResets);
        a->inlineIssued=state.nativeIssued;
        // Preserve recycled nodes: replace the fresh tail, not the free head.
        Write32(state.tailSlot,reinterpret_cast<UPtr>(&a->nodes[0]));
        InterlockedIncrement(&g_reuseRedirects);InterlockedIncrement(&g_sSpool0Redirects);
        if(!ReuseValidate(h,50u,a,state,why))return false;
    }
    const U32 issued=state.nativeIssued+state.extraIssued;
    if(issued>static_cast<U32>(g_reuseSpoolIssuedPeak))g_reuseSpoolIssuedPeak=static_cast<long>(issued);
    return true;
}
static bool ReusePrepareScratch(U32 site,UPtr frame,const char*& why) {
    InterlockedIncrement(&g_reuseScratchChecks);InterlockedIncrement(&g_sScratchCalls);
    if(site>1u || !Readable(frame,36u))return ReuseBad(why,"scratchFrame");
    const UPtr esp=Read32(frame+12u)+4u,h=esp+0x14u,first=esp+0x20u,end=esp+12u*kWideRequestNodes+32u;
    g_reuseLastScratch=static_cast<long>(end);
    ReuseListState state={};if(!ReuseValidate(h,kWideRequestNodes,nullptr,state,why))return false;
    if(state.active>static_cast<U32>(g_reuseScratchPeak))g_reuseScratchPeak=static_cast<long>(state.active);
    if(state.recycled)return true; // Native recycled path has no fresh write.
    const UPtr cursor=Read32(frame+28u);
    if(cursor!=state.fresh || cursor<first || cursor>=end || (cursor-first)%12u || state.active>=kWideRequestNodes)
        return ReuseBad(why,"scratchCapacityOrCursor");
    return true; // All150 slots are already in the expanded stack frame.
}

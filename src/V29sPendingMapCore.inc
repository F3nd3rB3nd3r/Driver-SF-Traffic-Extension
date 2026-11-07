// Pending map:64 native16-byte nodes +448 private nodes per constructor lifetime.
static const U32 kPendingCapacity=512u,kPendingExtra=448u,kPendingGuard=0x504D3531u;
struct PendingArena {
    UPtr older,owner;U32 generation,inlineUsed,extraUsed,reserved[3];
    U32 before[4];Node16 nodes[kPendingExtra];U32 after[4];
};
static_assert(sizeof(PendingArena)==7232u,"Pending arena geometry");
static PendingArena* volatile g_pendingHead=nullptr;
static volatile long g_pendingLock=0,g_pendingGeneration=0,g_pendingAttempts=0,g_pendingStores=0;
static volatile long g_pendingRedirects=0,g_pendingReuses=0,g_pendingPeak=0,g_pendingIssuedPeak=0;
static volatile long g_pendingInvalid=0,g_pendingFull=0,g_pendingAllocFailed=0,g_pendingGuardBad=0;
static volatile long g_pendingWalkStarts=0,g_pendingWalkEnds=0,g_pendingEmpty=0,g_pendingDepth=0;
static volatile long g_pendingCurrent=0,g_pendingFree=0,g_pendingLastMap=0,g_pendingResets=0;
static long g_pendingStatus=0;static const char* g_pendingReason="notAttempted";
static void PendingEvent(const char* kind,UPtr map,U32 active,U32 issued);
static void PendingFatal(const char* reason,UPtr map);
static PendingArena* PendingFind(UPtr map) {
    for(PendingArena* p=g_pendingHead;p;p=reinterpret_cast<PendingArena*>(p->older))if(p->owner==map)return p;
    return nullptr;
}
static bool PendingOwns(UPtr map){return (g_pkg0 && map==g_pkg0+0x5F5Cu) || PendingFind(map)!=nullptr;}
static U32 PendingGuards(const PendingArena* p) {
    U32 bad=0;for(U32 n=0;n<4u;++n){if(p->before[n]!=kPendingGuard)++bad;if(p->after[n]!=kPendingGuard)++bad;}return bad;
}
static bool PendingPristine(UPtr map) {
    const UPtr end=map+0x40Cu;
    return Read32(map)==map+12u && !Read32(map+4u) && !Read32(map+8u) && Read32(end)==end && Read32(end+4u)==end;
}
static bool PendingReject(U32 code){InterlockedIncrement(&g_pendingInvalid);g_pendingReason=code==1u?"countsOrCursor":code==2u?"listOrFreeChain":"argumentsOrNativeResult";return false;}
static bool PendingIndex(const PendingArena* p,UPtr node,U32& index) {
    const UPtr first=p->owner+12u,extra=reinterpret_cast<UPtr>(&p->nodes[0]);
    if(node>=first && node-first<p->inlineUsed*16u && !((node-first)&15u)){index=(node-first)/16u;return true;}
    if(node>=extra && node-extra<p->extraUsed*16u && !((node-extra)&15u)){index=64u+(node-extra)/16u;return true;}
    return false;
}
static UPtr PendingFresh(const PendingArena* p){return p->extraUsed?reinterpret_cast<UPtr>(&p->nodes[p->extraUsed]):p->owner+12u+p->inlineUsed*16u;}
static bool PendingValidate(const PendingArena* p) {
    if(PendingGuards(p)){InterlockedIncrement(&g_pendingGuardBad);g_pendingReason="arenaGuard";return false;}
    if(p->inlineUsed>64u || p->extraUsed>kPendingExtra || !Readable(p->owner,0x41Cu))return PendingReject(1u);
    const UPtr map=p->owner,end=map+0x40Cu;const U32 active=Read32(map+8u),free=Read32(map+4u),issued=p->inlineUsed+p->extraUsed;
    if(active>kPendingCapacity || free>kPendingCapacity || active+free!=issued)return PendingReject(1u);
    U32 seen[16]={},count=0,priorKey=0,index=0;UPtr previous=end,node=Read32(end+4u);
    while(node!=end) {
        if(count>=active || !PendingIndex(p,node,index) || (seen[index/32u]&(1u<<(index%32u))))return PendingReject(2u);
        seen[index/32u]|=1u<<(index%32u);
        const U32 key=Read32(node+8u)&0xFFFFu;
        if(Read32(node)!=previous || (count && key<priorKey))return PendingReject(2u);
        previous=node;node=Read32(node+4u);priorKey=key;++count;
    }
    if(count!=active || Read32(end)!=previous)return PendingReject(2u);
    node=Read32(map);
    for(U32 n=0;n<free;++n) {
        if(!PendingIndex(p,node,index) || (seen[index/32u]&(1u<<(index%32u))))return PendingReject(2u);
        seen[index/32u]|=1u<<(index%32u);node=Read32(node);
    }
    if(node!=PendingFresh(p))return PendingReject(1u);
    InterlockedExchange(&g_pendingCurrent,static_cast<long>(active));InterlockedExchange(&g_pendingFree,static_cast<long>(free));
    InterlockedExchange(&g_pendingLastMap,static_cast<long>(map));return true;
}
static PendingArena* PendingPrepare(UPtr map) {
    if(!Readable(map,0x41Cu)){PendingReject(1u);return nullptr;}
    PendingArena* old=PendingFind(map);
    if(PendingPristine(map)) {
        if(old && PendingGuards(old)){InterlockedIncrement(&g_pendingGuardBad);g_pendingReason="arenaGuard";return nullptr;}
        PendingArena* p=reinterpret_cast<PendingArena*>(VirtualAlloc(nullptr,sizeof(PendingArena),kMemCommitReserve,kPageReadWrite));
        if(!p){InterlockedIncrement(&g_pendingAllocFailed);g_pendingReason="arenaAllocation";return nullptr;}
        p->owner=map;p->generation=static_cast<U32>(InterlockedIncrement(&g_pendingGeneration));
        p->older=reinterpret_cast<UPtr>(g_pendingHead);
        for(U32 n=0;n<4u;++n)p->before[n]=p->after[n]=kPendingGuard;
        InterlockedExchange(reinterpret_cast<volatile long*>(&g_pendingHead),static_cast<long>(reinterpret_cast<UPtr>(p)));
        if(old)InterlockedIncrement(&g_pendingResets);
        PendingEvent("generation",map,0u,0u);return p;
    }
    if(!old){g_pendingReason="nonemptyUnownedMap";InterlockedIncrement(&g_pendingInvalid);return nullptr;}
    return old;
}
static UPtr PendingInsert(UPtr map,UPtr a,UPtr b,UPtr c) {
    if(g_pendingStatus!=1) {PendingFatal("notArmed",map);return 0;}
    while(InterlockedExchange(&g_pendingLock,1)){}
    InterlockedIncrement(&g_pendingAttempts);PendingArena* p=PendingPrepare(map);
    if(!p || !PendingValidate(p)) {InterlockedExchange(&g_pendingLock,0);PendingFatal(g_pendingReason,map);return 0;}
    const U32 active=Read32(map+8u),free=Read32(map+4u);U32 index=0;
    if(active>=kPendingCapacity) {InterlockedIncrement(&g_pendingFull);InterlockedExchange(&g_pendingLock,0);PendingFatal("capacity",map);return 0;}
    bool position=b==map+0x40Cu;
    if(!position && PendingIndex(p,b,index))for(UPtr at=Read32(map+0x410u);at!=map+0x40Cu;at=Read32(at+4u))if(at==b){position=true;break;}
    if(!Readable(a,4u) || !Readable(c,8u) || !position) {
        PendingReject(3u);InterlockedExchange(&g_pendingLock,0);PendingFatal(g_pendingReason,map);return 0;
    }
    const bool fresh=free==0u,extra=fresh && p->inlineUsed==64u;
    if(extra && !p->extraUsed){Write32(map,reinterpret_cast<UPtr>(&p->nodes[0]));InterlockedIncrement(&g_pendingRedirects);PendingEvent("redirect",map,active,64u);}
    const UPtr slot=Read32(map),next=free?Read32(slot):slot+16u;
    const UPtr result=g_origMapInsert(reinterpret_cast<void*>(map),a,b,c);
    if(fresh){if(extra)++p->extraUsed;else ++p->inlineUsed;}else InterlockedIncrement(&g_pendingReuses);
    if(result!=a || Read32(a)!=slot || Read32(map)!=next || Read32(map+8u)!=active+1u || Read32(map+4u)!=(free?free-1u:0u) || !PendingValidate(p)) {
        PendingReject(3u);InterlockedExchange(&g_pendingLock,0);PendingFatal("nativeInsertionResult",map);return 0;
    }
    InterlockedIncrement(&g_pendingStores);CapacityMax(g_pendingPeak,active+1u);CapacityMax(g_pendingIssuedPeak,p->inlineUsed+p->extraUsed);
    InterlockedExchange(&g_pendingLock,0);return result;
}
static bool PendingIncludeValid(UPtr section) {
    const UPtr end=section+0x6998u;const U32 active=Read32(section+0x6694u);
    if(active>256u)return PendingReject(2u);
    UPtr previous=end,node=Read32(end+4u);U32 count=0,priorKey=0;
    while(node!=end) {
        if(count>=active || !Readable(node,12u) || Read32(node)!=previous)return PendingReject(2u);
        const U32 key=Read32(node+8u)&0xFFFFu;if(count && key<priorKey)return PendingReject(2u);
        previous=node;node=Read32(node+4u);priorKey=key;++count;
    }
    return count==active && Read32(end)==previous?true:PendingReject(2u);
}
static bool PendingTraversal(U32 kind,UPtr section) {
    const UPtr map=section+0x5F5Cu;if(!PendingOwns(map))return true;
    if(!Readable(map,0x41Cu) || !Readable(section+0x6694u,0x310u))return PendingReject(1u);
    while(InterlockedExchange(&g_pendingLock,1)){}
    PendingArena* p=PendingFind(map);
    const bool pristine=PendingPristine(map);
    if(pristine && p && PendingGuards(p)){InterlockedIncrement(&g_pendingGuardBad);InterlockedExchange(&g_pendingLock,0);return false;}
    const U32 active=Read32(map+8u);
    const bool valid=(pristine || (p && PendingValidate(p))) &&
        ((kind==0u && !active) || PendingIncludeValid(section));
    if(!valid){InterlockedExchange(&g_pendingLock,0);return false;}
    InterlockedExchange(&g_pendingCurrent,static_cast<long>(active));InterlockedExchange(&g_pendingFree,static_cast<long>(Read32(map+4u)));
    if(kind==0u) {
        if(!active)InterlockedIncrement(&g_pendingEmpty);
        else {if(g_pendingDepth){InterlockedExchange(&g_pendingLock,0);return PendingReject(2u);}
            InterlockedIncrement(&g_pendingWalkStarts);InterlockedExchange(&g_pendingDepth,1);PendingEvent("walkBegin",map,active,p?p->inlineUsed+p->extraUsed:0u);}
    } else {
        if(g_pendingDepth!=1){InterlockedExchange(&g_pendingLock,0);return PendingReject(2u);}
        InterlockedIncrement(&g_pendingWalkEnds);InterlockedExchange(&g_pendingDepth,0);PendingEvent("walkEnd",map,active,p?p->inlineUsed+p->extraUsed:0u);
    }
    InterlockedExchange(&g_pendingLock,0);return true;
}

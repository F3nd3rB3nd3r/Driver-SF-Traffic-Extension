// Native map nodes stay stable for the process lifetime. No fixed owner directory.
#if defined(DSF_V29S_WIDE_LIMITS)
static const U32 kRegMapInline=45u,kRegMapGuard=0x29DE51ACu;
#define kRegMapNodes (g_limits.definitions)
#else
static const U32 kRegMapInline=45u,kRegMapNodes=512u,kRegMapGuard=0x29DE51ACu;
#endif
struct RegMapArena {
    RegMapArena* next;UPtr map;U32 generation,inlineIssued,externalIssued,attached;
    U32 guard0;
#if defined(DSF_V29S_WIDE_LIMITS)
U32 (*nodes)[6];U32* guard1;
#else
U32 nodes[kRegMapNodes][6];U32 guard1;
#endif
};
struct RegMapDetail {const char* reason;UPtr map,cursor,start,end;U32 active,recycled,generation,inlineIssued,externalIssued,attached,redirected;};
static RegMapArena* g_regMapHead=nullptr;
static volatile long g_regMapLock=0,g_regMapOwners=0,g_regMapConstructs=0,g_regMapReconstructs=0;
static volatile long g_regMapChecks=0,g_regMapRedirects=0,g_regMapReattaches=0,g_regMapRecycled=0;
static volatile long g_regMapFaults=0,g_regMapAllocFailed=0,g_regMapExternalHigh=0;
static UPtr RegMapAddress(U32);
static void* RegMapAllocate(U32);
static RegMapArena* RegMapFind(UPtr map){for(RegMapArena* a=g_regMapHead;a;a=a->next)if(a->map==map)return a;return nullptr;}
static UPtr RegMapStart(const RegMapArena* a){return reinterpret_cast<UPtr>(&a->nodes[0]);}
static UPtr RegMapFrontier(const RegMapArena* a){return a->attached?RegMapStart(a)+a->externalIssued*24u:a->map+12u+a->inlineIssued*24u;}
static U32 WideRegMapTail(const RegMapArena* a){
#if defined(DSF_V29S_WIDE_LIMITS)
return *a->guard1;
#else
return a->guard1;
#endif
}
static bool RegMapGuards(const RegMapArena* a){return a && a->guard0==kRegMapGuard && WideRegMapTail(a)==kRegMapGuard;}
static RegMapArena* RegMapCreate(UPtr map){
    RegMapArena* a=reinterpret_cast<RegMapArena*>(RegMapAllocate(sizeof(RegMapArena)));
    if(!a){InterlockedIncrement(&g_regMapAllocFailed);return nullptr;}
    // VirtualAlloc supplies zeroed pages; explicitly initialize every metadata field.
    a->map=map;a->generation=1u;a->inlineIssued=a->externalIssued=a->attached=0u;

#if defined(DSF_V29S_WIDE_LIMITS)
a->nodes=static_cast<U32(*)[6]>(WideAllocate(kRegMapNodes*24u+4u));a->guard1=reinterpret_cast<U32*>(a->nodes+kRegMapNodes);a->guard0=*a->guard1=kRegMapGuard;
#else
a->guard0=a->guard1=kRegMapGuard;
#endif
a->next=g_regMapHead;g_regMapHead=a;
    InterlockedIncrement(&g_regMapOwners);return a;
}
static bool RegMapView(UPtr map){return map>=0x254u && Readable(map-0x254u,0x6A4u) && Read32(map-0x254u)==RegMapAddress(0xE395D0u);}
static bool RegMapFresh(UPtr map){return RegMapView(map) && Read32(map)==map+12u && !Read32(map+4u) && !Read32(map+8u) && Read32(map+0x444u)==map+0x444u && Read32(map+0x448u)==map+0x444u;}
static void RegMapDescribe(RegMapDetail& d,UPtr map,RegMapArena* a,const char* reason){
    d.reason=reason;d.map=map;
    if(Readable(map,12u)){d.cursor=Read32(map);d.recycled=Read32(map+4u);d.active=Read32(map+8u);}
    if(a){d.start=RegMapStart(a);d.end=d.start+kRegMapNodes*24u;d.generation=a->generation;d.inlineIssued=a->inlineIssued;d.externalIssued=a->externalIssued;d.attached=a->attached;}
}
static bool RegMapFinish(RegMapDetail& d,UPtr map,RegMapArena* a,const char* reason){
    RegMapDescribe(d,map,a,reason);if(reason)InterlockedIncrement(&g_regMapFaults);
    InterlockedExchange(&g_regMapLock,0);return reason==nullptr;
}
static bool RegMapConstructed(UPtr owner,RegMapDetail& d){
    const UPtr map=owner+0x254u;while(InterlockedExchange(&g_regMapLock,1)){}
    RegMapArena* a=RegMapFind(map);
    if(!RegMapFresh(map))return RegMapFinish(d,map,a,"constructorState");
    if(a){
        if(!RegMapGuards(a))return RegMapFinish(d,map,a,"guard");
        if(a->generation==0xFFFFFFFFu)return RegMapFinish(d,map,a,"generationOverflow");
        ++a->generation;a->inlineIssued=a->externalIssued=a->attached=0u;
        InterlockedIncrement(&g_regMapReconstructs);
    }else {a=RegMapCreate(map);if(!a)return RegMapFinish(d,map,a,"allocationFailed");}
    InterlockedIncrement(&g_regMapConstructs);return RegMapFinish(d,map,a,nullptr);
}
static bool RegMapIssuedNode(const RegMapArena* a,UPtr p){
    const UPtr lo=a->map+12u,hi=lo+a->inlineIssued*24u;
    if(p>=lo && p<hi && (p-lo)%24u==0u)return true;
    const UPtr ext=RegMapStart(a);
    return a->attached && p>=ext && p<ext+a->externalIssued*24u && (p-ext)%24u==0u;
}
static bool RegMapPrepare(UPtr map,RegMapDetail& d){
    InterlockedIncrement(&g_regMapChecks);while(InterlockedExchange(&g_regMapLock,1)){}
    RegMapArena* a=RegMapFind(map);
    if(!RegMapView(map))return RegMapFinish(d,map,a,"mapIdentity");
    // Cold attach only at an entirely fresh map; never infer a reset of a known owner.
    if(!a){if(!RegMapFresh(map))return RegMapFinish(d,map,a,"unobservedGeneration");a=RegMapCreate(map);if(!a)return RegMapFinish(d,map,a,"allocationFailed");}
    if(!RegMapGuards(a))return RegMapFinish(d,map,a,"guard");
    if(a->inlineIssued>kRegMapInline || a->externalIssued>kRegMapNodes)return RegMapFinish(d,map,a,"issuedBounds");
    const U32 active=Read32(map+8u),free=Read32(map+4u),issued=a->inlineIssued+a->externalIssued;
    if(active>issued || free!=issued-active)return RegMapFinish(d,map,a,"countMismatch");
    const UPtr cursor=Read32(map),frontier=RegMapFrontier(a);
    if(free){
        if(!RegMapIssuedNode(a,cursor) || !Readable(cursor,24u))return RegMapFinish(d,map,a,"badRecycledNode");
        const UPtr next=Read32(cursor);
        if((free==1u && next!=frontier) || (free>1u && (next==cursor || !RegMapIssuedNode(a,next))))return RegMapFinish(d,map,a,"badRecycledLink");
        InterlockedIncrement(&g_regMapRecycled);return RegMapFinish(d,map,a,nullptr);
    }
    if(cursor!=frontier)return RegMapFinish(d,map,a,"unexpectedCursor");
    if(!a->attached && a->inlineIssued<kRegMapInline){++a->inlineIssued;return RegMapFinish(d,map,a,nullptr);}
    if(!a->attached){
        a->attached=1u;d.redirected=1u;Write32(map,RegMapStart(a));InterlockedIncrement(&g_regMapRedirects);
        if(a->generation>1u)InterlockedIncrement(&g_regMapReattaches);
    }
    if(a->externalIssued==kRegMapNodes)return RegMapFinish(d,map,a,"externalEnd");
    ++a->externalIssued;if(static_cast<long>(a->externalIssued)>g_regMapExternalHigh)g_regMapExternalHigh=static_cast<long>(a->externalIssued);
    return RegMapFinish(d,map,a,nullptr);
}
static UPtr RegMapRunConstructor(UPtr);
static void RegMapFatal(const RegMapDetail&);
static void RegMapEvent(const char*,const RegMapDetail&);
extern "C" __declspec(noinline) UPtr __fastcall RegMapConstruct(void* self,void*){
    const UPtr owner=reinterpret_cast<UPtr>(self),result=RegMapRunConstructor(owner);RegMapDetail d={};
    if(result!=owner){d.map=owner+0x254u;d.reason="constructorReturn";InterlockedIncrement(&g_regMapFaults);RegMapFatal(d);return result;}
    if(!RegMapConstructed(owner,d)){RegMapFatal(d);return result;}
    RegMapEvent("constructed",d);return result;
}
extern "C" __declspec(noinline) void __cdecl DiagSPrepareRegMap(UPtr map){
    RegMapDetail d={};
    if(!RegMapPrepare(map,d)){RegMapFatal(d);return;}
    if(d.redirected)RegMapEvent("redirect",d);
}

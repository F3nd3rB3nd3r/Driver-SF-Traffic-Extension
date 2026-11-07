static const U32 kParkCapacity=150u,kParkGuard=0x29FA150Du;
struct ParkSlot {
    UPtr owner;ParkSlot* next;U32 guardA,count,generation,busy,unique;
    U32 ids[150],picks[150],guardB;
};
static ParkSlot* g_parkSlots=nullptr;
static volatile long g_parkLock=0,g_parkOwners=0,g_parkBuilds=0,g_parkPeak=0;
static volatile long g_parkBucketPeak=0,g_parkPicks=0,g_parkSelected=0,g_parkEmpty=0;
static volatile long g_parkInvalid=0,g_parkGuardBad=0,g_parkConcurrent=0,g_parkUniquePeak=0,g_parkLastId=-1;
static volatile long g_parkExtraSelections=0;
static void ParkFatal(const char*,UPtr,U32);
static void* ParkAllocate(U32);
static UPtr ParkNativeDescribe(UPtr,U32);
static void ParkRunBuilder(UPtr);
static void ParkBuildEvent(UPtr,U32,U32,U32,const U32*);
static void ParkLock(){while(InterlockedExchange(&g_parkLock,1)){} }
static void ParkUnlock(){InterlockedExchange(&g_parkLock,0);}
static void ParkPeak(volatile long* value,U32 n) {
    long old=*value;while(n>static_cast<U32>(old)) {
        const long seen=InterlockedCompareExchange(value,static_cast<long>(n),old);
        if(seen==old)break;old=seen;
    }
}
static ParkSlot* ParkFind(UPtr owner,bool create) {
    for(ParkSlot* s=g_parkSlots;s;s=s->next)if(s->owner==owner)return s;
    if(!create)return nullptr;
    ParkSlot* s=static_cast<ParkSlot*>(ParkAllocate(sizeof(ParkSlot)));if(!s)return nullptr;
    // Allocator supplies zeroed storage; entries persist until process exit.
    s->owner=owner;s->guardA=s->guardB=kParkGuard;s->next=g_parkSlots;g_parkSlots=s;
    InterlockedIncrement(&g_parkOwners);return s;
}
static bool ParkSlotGuard(ParkSlot* s) {
    return s && s->guardA==kParkGuard && s->guardB==kParkGuard && s->count<=kParkCapacity;
}
extern "C" __declspec(noinline) UPtr __fastcall ParkDescribe(void* self,void*,U32 id) {
    const UPtr descriptor=ParkNativeDescribe(reinterpret_cast<UPtr>(self),id);
    if(!Readable(descriptor,0x1Cu) || Read8(descriptor+0x1Bu)>=25u) {
        InterlockedIncrement(&g_parkInvalid);ParkFatal("descriptorCategory",descriptor,id);return 0;
    }
    return descriptor;
}
extern "C" __declspec(noinline) void __cdecl ParkBucketCheck(CpuRegs* r) {
    const UPtr first=r->savedEsp+4u+0x288u,base=r->eax;
    if(base<first || base-first>=25u*0x25Cu || (base-first)%0x25Cu || r->ecx>=kParkCapacity) {
        InterlockedIncrement(&g_parkInvalid);ParkFatal("categoryWriteBounds",base,r->ecx);return;
    }
    ParkPeak(&g_parkBucketPeak,r->ecx+1u);
}
extern "C" __declspec(noinline) void* __cdecl ParkAppend(void* result,const U32* first,const U32* last,
    UPtr destination,U32,U32,U32) {
    const UPtr lo=reinterpret_cast<UPtr>(first),hi=reinterpret_cast<UPtr>(last),owner=destination-0x2408u;
    if(destination<0x2408u || hi<lo || ((hi-lo)&3u) || hi-lo>600u) {
        InterlockedIncrement(&g_parkInvalid);ParkFatal("appendRange",owner,hi-lo);return nullptr;
    }
    const U32 n=(hi-lo)/4u;ParkLock();ParkSlot* s=ParkFind(owner,false);
    const U32 count=Read32(owner+0x24D0u);
    if(!ParkSlotGuard(s) || !s->busy || count!=s->count || n>kParkCapacity-count) {
        ParkUnlock();InterlockedIncrement(&g_parkInvalid);ParkFatal("appendCapacityOrOwner",owner,count+n);return nullptr;
    }
    for(U32 i=0;i<n;++i){s->ids[count+i]=first[i];if(count+i<50u)Write32(destination+(count+i)*4u,first[i]);}
    s->count=count+n;Write32(owner+0x24D0u,s->count);ParkPeak(&g_parkPeak,s->count);ParkUnlock();
    Write32(reinterpret_cast<UPtr>(result),destination);return result;
}
extern "C" __declspec(noinline) void __fastcall ParkRebuild(void* self,void*) {
    const UPtr owner=reinterpret_cast<UPtr>(self);ParkLock();ParkSlot* s=ParkFind(owner,true);
    if(!ParkSlotGuard(s) || s->busy) {
        ParkUnlock();InterlockedIncrement(&g_parkInvalid);ParkFatal("buildOwnerOrAllocation",owner,0);return;
    }
    s->busy=1u;s->count=s->unique=0u;++s->generation;
    for(U32 i=0;i<150u;++i)s->ids[i]=s->picks[i]=0u;
    ParkUnlock();ParkRunBuilder(owner);
    U32 ids[150]={},count=0,generation=0,hash=2166136261u;ParkLock();
    bool ok=ParkSlotGuard(s) && Read32(owner+0x24D0u)==s->count;U32 end=0u;
    for(U32 c=0;c<25u && ok;++c) {
        const U32 start=Read32(owner+0x24D4u+c*8u),n=Read32(owner+0x24D8u+c*8u);
        ok=start==end && n<=s->count-end;if(ok)end+=n;
    }
    ok=ok && end==s->count;
    if(!ok){ParkUnlock();InterlockedIncrement(&g_parkGuardBad);ParkFatal("buildLayout",owner,end);return;}
    count=s->count;generation=s->generation;
    for(U32 i=0;i<count;++i){ids[i]=s->ids[i];hash=(hash^ids[i])*16777619u;}
    s->busy=0;ParkUnlock();InterlockedIncrement(&g_parkBuilds);ParkBuildEvent(owner,count,generation,hash,ids);
}
extern "C" __declspec(noinline) U32 __fastcall ParkPick(void* self,void*,U32 type,U32 seed) {
    const UPtr owner=reinterpret_cast<UPtr>(self);InterlockedIncrement(&g_parkPicks);ParkLock();ParkSlot* s=ParkFind(owner,false);
    if(s && s->busy){ParkUnlock();InterlockedIncrement(&g_parkConcurrent);return 0xFFFFFFFFu;}
    // A native constructor can reuse a previous manager address before refresh.
    // Its empty native count is authoritative until ParkRebuild resets private state.
    if(!Read32(owner+0x24D0u)){ParkUnlock();InterlockedIncrement(&g_parkEmpty);return 0xFFFFFFFFu;}
    if(!ParkSlotGuard(s) || Read32(owner+0x24D0u)!=s->count) {
        ParkUnlock();InterlockedIncrement(&g_parkGuardBad);ParkFatal("pickCacheOrOwner",owner,0);return 0xFFFFFFFFu;
    }
    U32 start=0u,n=s->count;
    if(type!=0x709u) {
        const U32 category=type-0x70Au;
        if(category>=25u){ParkUnlock();InterlockedIncrement(&g_parkEmpty);return 0xFFFFFFFFu;}
        start=Read32(owner+0x24D4u+category*8u);n=Read32(owner+0x24D8u+category*8u);
    }
    if(start>s->count || n>s->count-start){ParkUnlock();InterlockedIncrement(&g_parkGuardBad);ParkFatal("pickCategoryBounds",owner,n);return 0xFFFFFFFFu;}
    if(!n){ParkUnlock();InterlockedIncrement(&g_parkEmpty);return 0xFFFFFFFFu;}
    const U32 at=start+seed%n,id=s->ids[at];
    if(at>=50u)InterlockedIncrement(&g_parkExtraSelections);
    if(!s->picks[at]++)ParkPeak(&g_parkUniquePeak,++s->unique);
    ParkUnlock();g_parkLastId=static_cast<long>(id);InterlockedIncrement(&g_parkSelected);return id;
}

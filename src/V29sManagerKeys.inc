// R8: fourteen manager-owned sorted key/refcount lists. Preserve the native
// 0xC18 stride and sentinel at header+0xC0C; extend only fresh node storage.
#include "V29sManagerKeyContracts.inc"
struct ManagerKeyList {UPtr storage,nodes;U32 used,peak;};
struct ManagerKeyOwner {ManagerKeyOwner* next;UPtr owner;U32 generation,alive;ManagerKeyList lists[14];};
static ManagerKeyOwner* g_managerKeyOwners=nullptr;
static volatile long g_managerKeyLock=0,g_managerKeyGenerations=0,g_managerKeyDestructions=0;
static volatile long g_managerKeyInserts=0,g_managerKeyLookups=0,g_managerKeyNative=0,g_managerKeyRedirects=0;
static volatile long g_managerKeyBuffers=0,g_managerKeyFailures=0,g_managerKeyGuardBad=0,g_managerKeyPeak=0;
static long g_managerKeyStatus=0;
static const U32 kManagerKeyGuard=0x4B385147u;
static DiagFDetour g_managerKeyDetours[3]={};
static UPtr g_managerKeyTramps[3]={};
static U32 ManagerKeyCapacity(){return WideWorkNodes();}
static UPtr ManagerKeyHeader(UPtr owner,U32 kind){return owner+12u+kind*0xC18u;}
static ManagerKeyOwner* ManagerKeyFindOwner(UPtr owner){for(auto* a=g_managerKeyOwners;a;a=a->next)if(a->owner==owner)return a;return nullptr;}
static ManagerKeyOwner* ManagerKeyFindHeader(UPtr header,U32& kind){
    for(auto* a=g_managerKeyOwners;a;a=a->next)if(a->alive && header>=a->owner+12u){
        const U32 delta=header-a->owner-12u;
        if(delta<14u*0xC18u && delta%0xC18u==0u){kind=delta/0xC18u;return a;}
    }return nullptr;
}
static bool ManagerKeyGuards(const ManagerKeyList& a){
    if(!a.storage)return true;
    for(U32 i=0;i<4u;++i)if(Read32(a.storage+i*4u)!=kManagerKeyGuard || Read32(a.nodes+(ManagerKeyCapacity()-256u)*12u+i*4u)!=kManagerKeyGuard)return false;
    return true;
}
static void ManagerKeyFatal(const char* why,UPtr header){
    InterlockedIncrement(&g_managerKeyFailures);char line[600]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_MANAGER_KEY_FATAL reason=",end);AppendText(p,why,end);
    DiagFAppendKeyHex(p," header=",header,end);DiagFAppendKeyU(p," capacity=",ManagerKeyCapacity(),end);
    AppendText(p," beforeNativeSearchOrWrite=1\n",end);*p=0;CapacityWrite(line);
    WideAllocationFailure("Unlimiter: manager key list invalid or configured capacity exhausted; see MANAGER_KEY_FATAL.\n");
}
static void ManagerKeyConstruct(UPtr owner){
    if(g_managerKeyStatus!=1)return;
    while(InterlockedExchange(&g_managerKeyLock,1)){}
    auto* old=ManagerKeyFindOwner(owner);if(old && old->alive)ManagerKeyFatal("reconstructBeforeDestruction",owner);
    auto* a=static_cast<ManagerKeyOwner*>(WideAllocate(sizeof(ManagerKeyOwner)));
    a->owner=owner;a->alive=1u;a->generation=old?old->generation+1u:1u;
    a->next=g_managerKeyOwners;g_managerKeyOwners=a;InterlockedIncrement(&g_managerKeyGenerations);
    InterlockedExchange(&g_managerKeyLock,0);
}
static void ManagerKeyDestroy(UPtr owner){
    while(InterlockedExchange(&g_managerKeyLock,1)){}
    auto* a=ManagerKeyFindOwner(owner);if(!a || !a->alive)ManagerKeyFatal("destructionState",owner);
    for(const auto& list:a->lists)if(!ManagerKeyGuards(list)){InterlockedIncrement(&g_managerKeyGuardBad);ManagerKeyFatal("destructionGuard",owner);}
    a->alive=0u;InterlockedIncrement(&g_managerKeyDestructions);InterlockedExchange(&g_managerKeyLock,0);
}
static bool ManagerKeyIndex(UPtr header,const ManagerKeyList& a,UPtr node,U32& index){
    const UPtr first=header+12u;
    if(node>=first && node-first<256u*12u && (node-first)%12u==0u){index=(node-first)/12u;return true;}
    if(a.nodes && node>=a.nodes && node-a.nodes<(ManagerKeyCapacity()-256u)*12u && (node-a.nodes)%12u==0u){index=256u+(node-a.nodes)/12u;return true;}
    return false;
}
struct ManagerKeyState {U32 active,recycled,issued;UPtr fresh;};
static bool ManagerKeyValidate(UPtr h,const ManagerKeyList& a,ManagerKeyState& s,const char*& why){
    if(!Readable(h,0xC18u)){why="headerUnreadable";return false;}
    if(!ManagerKeyGuards(a)){why="arenaGuard";return false;}
    const U32 cap=ManagerKeyCapacity();const UPtr first=h+12u,end=h+0xC0Cu;
    s.active=Read32(h+8u);s.recycled=Read32(h+4u);
    if(s.active>cap || s.recycled>cap-s.active){why="counts";return false;}
    WideScratchLease lease(((cap+31u)/32u)*4u);auto* seen=static_cast<U32*>(lease.data);
    UPtr node=Read32(end+4u),previous=end;U32 index=0,key=0;
    for(U32 i=0;i<s.active;++i){
        if(!ManagerKeyIndex(h,a,node,index) || (seen[index/32u]&(1u<<(index%32u)))){why="activeCycleOrForeignNode";return false;}
        seen[index/32u]|=1u<<(index%32u);
        if(Read32(node)!=previous || (i && Read16(node+8u)<key)){why="activeBacklinkOrOrder";return false;}
        key=Read16(node+8u);previous=node;node=Read32(node+4u);
    }
    if(node!=end || Read32(end)!=previous){why="sentinelOrActiveCount";return false;}
    node=Read32(h);
    for(U32 i=0;i<s.recycled;++i){
        if(!ManagerKeyIndex(h,a,node,index) || (seen[index/32u]&(1u<<(index%32u)))){why="recycledCycleOrAlias";return false;}
        seen[index/32u]|=1u<<(index%32u);node=Read32(node);
    }
    s.fresh=node;
    if(node>=first && node<=end && (node-first)%12u==0u)s.issued=(node-first)/12u;
    else if(a.nodes && node>=a.nodes && node<=a.nodes+(cap-256u)*12u && (node-a.nodes)%12u==0u)s.issued=256u+(node-a.nodes)/12u;
    else {why="freshCursor";return false;}
    if(s.active+s.recycled!=s.issued){why="issuedCount";return false;}
    for(U32 i=s.issued;i<cap;++i)if(seen[i/32u]&(1u<<(i%32u))){why="unissuedNode";return false;}
    return true;
}
static void ManagerKeyRequire(UPtr header,bool insertion,UPtr position){
    while(InterlockedExchange(&g_managerKeyLock,1)){}
    U32 kind=0;auto* owner=ManagerKeyFindHeader(header,kind);
    if(!owner){InterlockedIncrement(&g_managerKeyNative);InterlockedExchange(&g_managerKeyLock,0);return;}
    auto& a=owner->lists[kind];const char* why=nullptr;ManagerKeyState s={};
    if(!ManagerKeyValidate(header,a,s,why)){
        if(!ManagerKeyGuards(a))InterlockedIncrement(&g_managerKeyGuardBad);
        ManagerKeyFatal(why?why:"integrity",header);
    }
    a.used=1u;
    if(insertion){
        if(s.active>=ManagerKeyCapacity())ManagerKeyFatal("configuredCapacity",header);
        U32 positionIndex=0;
        if(position!=header+0xC0Cu && !ManagerKeyIndex(header,a,position,positionIndex))ManagerKeyFatal("insertPosition",header);
        if(!s.recycled && s.fresh==header+0xC0Cu){
            if(!a.storage){
                a.storage=reinterpret_cast<UPtr>(WideAllocate((ManagerKeyCapacity()-256u)*12u+32u));a.nodes=a.storage+16u;
                for(U32 i=0;i<4u;++i){Write32(a.storage+i*4u,kManagerKeyGuard);Write32(a.nodes+(ManagerKeyCapacity()-256u)*12u+i*4u,kManagerKeyGuard);}
                InterlockedIncrement(&g_managerKeyBuffers);
            }
            Write32(header,a.nodes);InterlockedIncrement(&g_managerKeyRedirects);
        }
        if(s.active+1u>a.peak)a.peak=s.active+1u;CapacityMax(g_managerKeyPeak,a.peak);InterlockedIncrement(&g_managerKeyInserts);
    }else InterlockedIncrement(&g_managerKeyLookups);
    InterlockedExchange(&g_managerKeyLock,0);
}
extern "C" void __cdecl ManagerKeyInsertEntry(CpuRegs* r){ManagerKeyRequire(r->ecx,true,Read32(r->savedEsp+12u));}
extern "C" void __cdecl ManagerKeyLookupEntry(CpuRegs* r){ManagerKeyRequire(r->ecx,false,0u);}
extern "C" void __cdecl ManagerKeyDestroyEntry(CpuRegs* r){ManagerKeyDestroy(r->ebx);}
#define MANAGER_KEY_BRIDGE(name,entry,index) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} __asm {pushad} __asm {mov ebp,esp} \
    __asm {sub esp,528} __asm {and esp,0FFFFFFF0h} __asm {fxsave [esp]} __asm {cld} \
    __asm {push ebp} __asm {call entry} __asm {add esp,4} \
    __asm {fxrstor [esp]} __asm {mov esp,ebp} __asm {popad} __asm {popfd} \
    __asm {jmp dword ptr [g_managerKeyTramps+index*4]} \
}
MANAGER_KEY_BRIDGE(ManagerKeyInsertBridge,ManagerKeyInsertEntry,0)
MANAGER_KEY_BRIDGE(ManagerKeyLookupBridge,ManagerKeyLookupEntry,1)
MANAGER_KEY_BRIDGE(ManagerKeyDestroyBridge,ManagerKeyDestroyEntry,2)
#undef MANAGER_KEY_BRIDGE
static bool ManagerKeyInstall(){
    const U8 insert[]={0x83,0x79,4,0,0x8B,0x11},lookup[]={0x8B,0x91,0x10,0x0C,0,0},destroy[]={0xC7,0x43,4,0xC0,0x64,0xDD,0};
    struct Hook {UPtr at;const U8* sig;U32 size;UPtr bridge;};
    const Hook hooks[]={{0x59E120u,insert,sizeof(insert),reinterpret_cast<UPtr>(&ManagerKeyInsertBridge)},
      {0x59E0D0u,lookup,sizeof(lookup),reinterpret_cast<UPtr>(&ManagerKeyLookupBridge)},
      {0x59D566u,destroy,sizeof(destroy),reinterpret_cast<UPtr>(&ManagerKeyDestroyBridge)}};
    // Validate geometry and retained lookup/refcount/removal semantics before
    // any mutation. This runs after the inherited cold hooks, before objects.
    for(const auto& v:kManagerKeyContracts){
        if(!Readable(Address(v.at),v.bytes))return false;
        U32 hash=2166136261u;for(U32 i=0;i<v.bytes;++i)hash=(hash^Read8(Address(v.at)+i))*16777619u;
        if(hash!=v.fnv)return false;
    }
    // All three complete instruction spans are checked before any mutation.
    for(const auto& h:hooks){if(!Readable(Address(h.at),h.size))return false;for(U32 i=0;i<h.size;++i)if(Read8(Address(h.at)+i)!=h.sig[i])return false;}
    for(U32 i=0;i<3u;++i)if(!DiagFInstallDetour(g_managerKeyDetours[i],hooks[i].at,hooks[i].sig,hooks[i].size,hooks[i].bridge,g_managerKeyTramps[i])){
        for(U32 j=i+1u;j>0u;--j)DiagFRestoreDetour(g_managerKeyDetours[j-1u],g_managerKeyTramps[j-1u]);g_managerKeyStatus=-1;return false;
    }
    g_managerKeyStatus=1;return true;
}
static void ManagerKeyLog(bool startup,U32 tick){
    while(InterlockedExchange(&g_managerKeyLock,1)){}
    U32 alive=0,active=0;for(auto* a=g_managerKeyOwners;a;a=a->next){
        if(a->alive)++alive;
        for(U32 i=0;i<14u;++i){auto& list=a->lists[i];if(!ManagerKeyGuards(list)){InterlockedIncrement(&g_managerKeyGuardBad);ManagerKeyFatal("sampledArenaGuard",a->owner);}
            if(a->alive && list.used)active+=Read32(ManagerKeyHeader(a->owner,i)+8u);}
    }
    char line[1100]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,startup?"DSFV29s_MANAGER_KEY_START revision=1":"DSFV29s_MANAGER_KEY_STATUS",end);
    DiagFAppendKeyU(p," tick=",tick,end);DiagFAppendKeyU(p," status=",static_cast<U32>(g_managerKeyStatus),end);DiagFAppendKeyU(p," capacity=",ManagerKeyCapacity(),end);
    DiagFAppendKeyU(p," generations=",static_cast<U32>(g_managerKeyGenerations),end);DiagFAppendKeyU(p," destructions=",static_cast<U32>(g_managerKeyDestructions),end);
    DiagFAppendKeyU(p," alive=",alive,end);DiagFAppendKeyU(p," buffers=",static_cast<U32>(g_managerKeyBuffers),end);
    DiagFAppendKeyU(p," inserts=",static_cast<U32>(g_managerKeyInserts),end);DiagFAppendKeyU(p," lookups=",static_cast<U32>(g_managerKeyLookups),end);
    DiagFAppendKeyU(p," nativeFallbacks=",static_cast<U32>(g_managerKeyNative),end);DiagFAppendKeyU(p," redirects=",static_cast<U32>(g_managerKeyRedirects),end);
    DiagFAppendKeyU(p," activeSampled=",active,end);DiagFAppendKeyU(p," peakPerListProjected=",static_cast<U32>(g_managerKeyPeak),end);
    DiagFAppendKeyU(p," failures=",static_cast<U32>(g_managerKeyFailures),end);DiagFAppendKeyU(p," guardBad=",static_cast<U32>(g_managerKeyGuardBad),end);
    AppendText(p," lists=14 nativeCapacity=256 stride=12 hooks=3 preserveFX=1 nativeSentinels=1 stableGenerationNodes=1 beforeNativeSearchOrWrite=1\n",end);*p=0;
    InterlockedExchange(&g_managerKeyLock,0);CapacityWrite(line);
}

// R5: the shared dispatch records and their scheduler must grow together.
// Native record pointers never move within a manager generation. Old slabs
// remain allocated until process exit; initialization never migrates live data.
#include "V29sStreamHandlerLayout.inc"
static U32 StreamCapacity(){return WideWorkNodes();}
static U32 StreamVectorStride(){return StreamCapacity()*4u+4u;}
static U32 StreamMemberStride(){return StreamCapacity()*12u+24u;}
static U32 StreamFourthBase(){return StreamVectorStride()*3u+StreamMemberStride()*14u;}
static U32 StreamVectorBase(U32 kind){return kind<3u?kind*StreamVectorStride():StreamFourthBase();}
// Original layout: three256-pointer vectors, fourteen512-node lists,
// then a fourth512-pointer vector at15D5C with its count at1655C.
static U32 StreamTail(){return StreamFourthBase()+StreamCapacity()*4u;}
static U32 StreamSchedulerBytes(){return StreamTail()+196u;}
static U32 StreamLayout(U32 value){
    if(value>=0x1655Cu && value<=0x16620u)return StreamTail()+value-0x1655Cu;
    switch(value){
    case 0x800u:return StreamCapacity()*4u;
    case 0x15D5Cu:return StreamFourthBase();
    case 0x400u:return StreamCapacity()*4u;
    case 0x404u:return StreamVectorStride();
    case 0x804u:return StreamVectorStride()*2u-4u;
    case 0x808u:return StreamVectorStride()*2u;
    case 0xC08u:return StreamVectorStride()*3u-4u;
    case 0xC0Cu:return StreamVectorStride()*3u;
    case 0x1818u:return StreamMemberStride();
    case 0x180Cu:return StreamMemberStride()-12u;
    case 0x1810u:return StreamMemberStride()-8u;
    default:return value;
    }
}
struct StreamArena {
    StreamArena* next;UPtr owner,storage,nodes;U32 generation,constructed,alive;
};
struct StreamScheduler {
    StreamScheduler* next;UPtr owner;U32 generation,alive;
};
static StreamArena* g_streamArenas=nullptr;
static StreamScheduler* g_streamSchedulers=nullptr;
static volatile long g_streamLock=0;
static long g_streamStatus=0;
static volatile long g_streamGenerations=0,g_streamInitializations=0,g_streamDestructions=0;
static volatile long g_streamSchedulerGenerations=0,g_streamSchedulerDestructions=0;
static volatile long g_streamFailures=0,g_streamGuardBad=0,g_streamPeak=0,g_streamQueuePeaks[4]={};
static volatile long g_streamMemberPeak=0;
static UPtr g_streamLastOwner=0;
static const U32 kStreamGuard=0x53523547u;
static DiagFDetour g_streamDetours[8]={};
static UPtr g_streamTramps[8]={};
static StreamArena* StreamFind(UPtr owner){for(auto* a=g_streamArenas;a;a=a->next)if(a->owner==owner)return a;return nullptr;}
static StreamScheduler* StreamSchedulerFind(UPtr owner){for(auto* a=g_streamSchedulers;a;a=a->next)if(a->owner==owner)return a;return nullptr;}
static bool StreamGuards(const StreamArena* a){
    for(U32 i=0;i<4u;++i)if(Read32(a->storage+i*4u)!=kStreamGuard || Read32(a->nodes+StreamCapacity()*80u+i*4u)!=kStreamGuard)return false;
    return true;
}
static bool StreamSchedulerGuards(UPtr owner){for(U32 i=0;i<4u;++i)if(Read32(owner+StreamSchedulerBytes()+i*4u)!=kStreamGuard)return false;return true;}
static U32 StreamBusy(const StreamArena* a){U32 n=0;for(U32 i=0;i<a->constructed;++i)n+=Read8(a->nodes+i*80u+0x4Du)?1u:0u;return n;}
static void StreamFatal(const char* reason,UPtr owner,U32 model=0u){
    InterlockedIncrement(&g_streamFailures);
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_STREAM_HANDLER_FATAL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," model=",model,end);
    DiagFAppendKeyU(p," capacity=",StreamCapacity(),end);StreamArena* a=StreamFind(owner);
    DiagFAppendKeyU(p," constructed=",a?a->constructed:0u,end);DiagFAppendKeyU(p," busy=",a?StreamBusy(a):0u,end);
    AppendText(p," beforeNullDereferenceOrWrite=1\n",end);*p=0;CapacityWrite(line);
    WideAllocationFailure("Unlimiter: streaming handler storage exhausted or invalid; see STREAM_HANDLER_FATAL.\n");
}
static void StreamConstruct(UPtr owner){
    while(InterlockedExchange(&g_streamLock,1)){}
    StreamArena* old=StreamFind(owner);
    if(old && !StreamGuards(old))StreamFatal("oldGenerationGuard",owner);
    if(old && old->alive)StreamFatal("reconstructBeforeDestruction",owner);
    auto* a=static_cast<StreamArena*>(WideAllocate(sizeof(StreamArena)));
    a->owner=owner;a->generation=old?old->generation+1u:1u;a->alive=1u;
    a->storage=reinterpret_cast<UPtr>(WideAllocate(StreamCapacity()*80u+32u));a->nodes=a->storage+16u;
    for(U32 i=0;i<4u;++i){Write32(a->storage+i*4u,kStreamGuard);Write32(a->nodes+StreamCapacity()*80u+i*4u,kStreamGuard);}
    // Only the obsolete inline array's first word holds the external base.
    // Count and subsequent manager fields remain at their original addresses.
    Write32(owner+0x6FDA0u,a->nodes);a->next=g_streamArenas;g_streamArenas=a;
    InterlockedIncrement(&g_streamGenerations);g_streamLastOwner=owner;
    InterlockedExchange(&g_streamLock,0);
}
static void StreamResize(UPtr inlinePool,U32 originalCount,UPtr source){
    const UPtr owner=inlinePool-0x6FDA0u;
    while(InterlockedExchange(&g_streamLock,1)){}
    StreamArena* a=StreamFind(owner);
    if(!a || !a->alive || originalCount!=256u || !Readable(source,80u))StreamFatal("initializationState",owner);
    if(!StreamGuards(a)){InterlockedIncrement(&g_streamGuardBad);StreamFatal("arenaGuard",owner);}
    if(Read32(owner+0x74DA0u)!=a->constructed || Read32(inlinePool)!=a->nodes)StreamFatal("countOrBaseMismatch",owner);
    // This is the retail copy constructor, including its native vtable and
    // per-handler flags. No engine resource/loading routine is called here.
    typedef UPtr (__thiscall* Copy)(void*,UPtr);
    for(U32 i=a->constructed;i<StreamCapacity();++i)
        reinterpret_cast<Copy>(Address(0x0059E1D0u))(reinterpret_cast<void*>(a->nodes+i*80u),source);
    a->constructed=StreamCapacity();Write32(owner+0x74DA0u,a->constructed);
    InterlockedIncrement(&g_streamInitializations);InterlockedExchange(&g_streamLock,0);
}
static void StreamDestroyed(UPtr owner){
    while(InterlockedExchange(&g_streamLock,1)){}
    StreamArena* a=StreamFind(owner);
    if(!a || !a->alive || !StreamGuards(a))StreamFatal("destructionState",owner);
    a->alive=0u;InterlockedIncrement(&g_streamDestructions);InterlockedExchange(&g_streamLock,0);
}
static void StreamSchedulerConstructed(UPtr owner){
    while(InterlockedExchange(&g_streamLock,1)){}
    StreamScheduler* old=StreamSchedulerFind(owner);
    if(old && old->alive)StreamFatal("schedulerReconstructBeforeDestruction",0u);
    auto* a=static_cast<StreamScheduler*>(WideAllocate(sizeof(StreamScheduler)));
    a->owner=owner;a->generation=old?old->generation+1u:1u;a->alive=1u;
    for(U32 i=0;i<4u;++i)Write32(owner+StreamSchedulerBytes()+i*4u,kStreamGuard);
    a->next=g_streamSchedulers;g_streamSchedulers=a;InterlockedIncrement(&g_streamSchedulerGenerations);
    InterlockedExchange(&g_streamLock,0);
}
static void StreamSchedulerDestroyed(UPtr owner){
    while(InterlockedExchange(&g_streamLock,1)){}
    auto* a=StreamSchedulerFind(owner);
    if(!a || !a->alive || !StreamSchedulerGuards(owner))StreamFatal("schedulerDestructionState",0u);
    a->alive=0u;InterlockedIncrement(&g_streamSchedulerDestructions);InterlockedExchange(&g_streamLock,0);
}
static void StreamVectorBeforeInsert(UPtr vector){
    while(InterlockedExchange(&g_streamLock,1)){}
    StreamScheduler* found=nullptr;U32 kind=0;
    for(auto* a=g_streamSchedulers;a && !found;a=a->next)if(a->alive)for(U32 i=0;i<4u;++i)
        if(vector==a->owner+StreamVectorBase(i)){found=a;kind=i;break;}
    if(!found)StreamFatal("unregisteredSchedulerVector",0u);
    const U32 count=Read32(vector+StreamCapacity()*4u);
    if(count>=StreamCapacity() || !StreamSchedulerGuards(found->owner))StreamFatal("schedulerVectorFullOrGuard",0u);
    CapacityMax(g_streamQueuePeaks[kind],count+1u);InterlockedExchange(&g_streamLock,0);
}
static void StreamMemberBeforeInsert(UPtr header){
    while(InterlockedExchange(&g_streamLock,1)){}
    bool found=false;
    for(auto* a=g_streamSchedulers;a && !found;a=a->next)if(a->alive)for(U32 i=0;i<14u;++i)
        if(header==a->owner+StreamVectorStride()*3u+i*StreamMemberStride()){found=true;break;}
    if(!found)StreamFatal("unregisteredMembershipList",0u);
    const U32 active=Read32(header+8u),free=Read32(header+4u);const UPtr cursor=Read32(header),first=header+12u,end=header+StreamMemberStride()-12u;
    if(active>=StreamCapacity() || free>StreamCapacity()-active || cursor<first || cursor>=end || (cursor-first)%12u)
        StreamFatal("membershipFullOrCursor",0u);
    CapacityMax(g_streamMemberPeak,active+1u);InterlockedExchange(&g_streamLock,0);
}
extern "C" void __cdecl StreamConstructEntry(CpuRegs* r){StreamConstruct(r->esi);}
extern "C" void __cdecl StreamDestroyEntry(CpuRegs* r){StreamDestroyed(r->ebx);}
extern "C" void __cdecl StreamResizeEntry(CpuRegs* r){StreamResize(r->ecx,Read32(r->savedEsp+8u),Read32(r->savedEsp+12u));}
extern "C" void __cdecl StreamNullEntry(CpuRegs* r){if(!r->esi)StreamFatal("noFreeDispatchHandler",Read32(r->savedEsp+4u+0x14u),r->edi);}
extern "C" void __cdecl StreamSchedulerConstructEntry(CpuRegs* r){StreamSchedulerConstructed(r->esi);}
extern "C" void __cdecl StreamSchedulerDestroyEntry(CpuRegs* r){StreamSchedulerDestroyed(r->ecx);}
extern "C" void __cdecl StreamVectorEntry(CpuRegs* r){StreamVectorBeforeInsert(r->ecx);}
extern "C" void __cdecl StreamFourthVectorEntry(CpuRegs* r){StreamVectorBeforeInsert(r->ecx);}
extern "C" void __cdecl StreamMemberEntry(CpuRegs* r){StreamMemberBeforeInsert(r->esi);}
#define STREAM_BRIDGE(name,entry,index) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} __asm {pushad} __asm {mov ebp,esp} \
    __asm {sub esp,528} __asm {and esp,0FFFFFFF0h} __asm {fxsave [esp]} __asm {cld} \
    __asm {push ebp} __asm {call entry} __asm {add esp,4} \
    __asm {fxrstor [esp]} __asm {mov esp,ebp} __asm {popad} __asm {popfd} \
    __asm {jmp dword ptr [g_streamTramps+index*4]} \
}
STREAM_BRIDGE(StreamConstructBridge,StreamConstructEntry,0)
STREAM_BRIDGE(StreamDestroyBridge,StreamDestroyEntry,1)
STREAM_BRIDGE(StreamNullBridge,StreamNullEntry,2)
STREAM_BRIDGE(StreamSchedulerConstructBridge,StreamSchedulerConstructEntry,3)
STREAM_BRIDGE(StreamSchedulerDestroyBridge,StreamSchedulerDestroyEntry,4)
STREAM_BRIDGE(StreamVectorBridge,StreamVectorEntry,5)
STREAM_BRIDGE(StreamMemberBridge,StreamMemberEntry,6)
STREAM_BRIDGE(StreamFourthVectorBridge,StreamFourthVectorEntry,7)
#undef STREAM_BRIDGE
extern "C" __declspec(naked) void StreamResizeBridge(){
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push ebp
        call StreamResizeEntry
        add esp,4
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        ret 8
    }
}
struct StreamBasePatch {UPtr site;U8 oldOp,oldMod,newMod;};
static const StreamBasePatch kStreamBasePatches[]={
    {0x59C707u,0x8D,0xB1,0xB1},{0x59C759u,0x8D,0xB1,0xB1},{0x59C7D9u,0x8D,0xB1,0xB1},
    {0x59C85Au,0x8D,0xB1,0xB1},{0x59C969u,0x8D,0xB1,0xB1},{0x59C9F9u,0x8D,0xB1,0xB1},
    {0x59CA79u,0x8D,0xB1,0xB1},{0x59D524u,0x8D,0xB3,0xB3},
    {0x59D7D9u,0x81,0xC6,0xB6},{0x59D91Eu,0x81,0xC6,0xB6},{0x59D9DCu,0x81,0xC6,0xB6}
};
static bool StreamWrite(UPtr site,const void* data,U32 bytes){
    DWORD old=0,unused=0;void* p=reinterpret_cast<void*>(Address(site));
    if(!VirtualProtect(p,bytes,kPageExecuteReadWrite,&old))return false;
    memcpy(p,data,bytes);const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),p,bytes);
    const BOOL restored=VirtualProtect(p,bytes,old,&unused);return flushed && restored;
}
static bool StreamOperandMatches(const StreamLayoutOperand& v){
    if(!Readable(Address(v.instruction),v.length))return false;
    U32 hash=2166136261u;for(U32 i=0;i<v.length;++i)hash=(hash^Read8(Address(v.instruction)+i))*16777619u;
    return hash==v.signature && Read32(Address(v.site))==v.original;
}
static bool StreamInstallRefuse(const char* stage,UPtr site){
    g_streamStatus=-1;char line[350]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_STREAM_HANDLER_INSTALL_FAIL stage=",end);AppendText(p,stage,end);
    DiagFAppendKeyHex(p," site=",site,end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);return false;
}
static bool StreamInstall(){
    U8 ctor[]={0x89,0x9E,0xA0,0x4D,7,0},destroy[]={0x8D,0xB3,0x90,0xFD,6,0};
    U8 nullSite[]={0x8B,6,0x8B,0x50,0x44},schedulerCtor[]={0x8B,0xC6,0x5E,0x5B,0xC3};
    U8 schedulerDtor[]={0x56,0x8B,0xF1,0x8B,0x8E,0x68,0x65,1,0},vector[]={0x8B,0x81,0,4,0,0};
    U8 member[]={0x83,0x7E,4,0,0x66,0x89,0x54,0x24,0x3E};
    U8 fourth[]={0x8B,0x81,0,8,0,0};
    struct Hook {UPtr site;U8* bytes;U32 length;UPtr bridge;};
    Hook hooks[]={
        {0x59D253u,ctor,sizeof(ctor),reinterpret_cast<UPtr>(&StreamConstructBridge)},
        {0x59D547u,destroy,sizeof(destroy),reinterpret_cast<UPtr>(&StreamDestroyBridge)},
        {0x59DA4Au,nullSite,sizeof(nullSite),reinterpret_cast<UPtr>(&StreamNullBridge)},
        {0x5A3482u,schedulerCtor,sizeof(schedulerCtor),reinterpret_cast<UPtr>(&StreamSchedulerConstructBridge)},
        {0x5A2F00u,schedulerDtor,sizeof(schedulerDtor),reinterpret_cast<UPtr>(&StreamSchedulerDestroyBridge)},
        {0x5A5520u,vector,sizeof(vector),reinterpret_cast<UPtr>(&StreamVectorBridge)},
        {0x5A3F08u,member,sizeof(member),reinterpret_cast<UPtr>(&StreamMemberBridge)},
        {0x5A55A0u,fourth,sizeof(fourth),reinterpret_cast<UPtr>(&StreamFourthVectorBridge)}
    };
    // All original signatures and every coupled operand are checked before
    // any mutation. This runs at cold ASI initialization, before game objects.
    for(const auto& h:hooks){if(!Readable(Address(h.site),h.length))return StreamInstallRefuse("hookUnreadable",h.site);for(U32 i=0;i<h.length;++i)if(Read8(Address(h.site)+i)!=h.bytes[i])return StreamInstallRefuse("hookSignature",h.site);}
    for(const auto& b:kStreamBasePatches)if(Read8(Address(b.site))!=b.oldOp || Read8(Address(b.site)+1u)!=b.oldMod || Read32(Address(b.site)+2u)!=0x6FDA0u)return StreamInstallRefuse("baseSignature",b.site);
    for(const auto& v:kStreamLayoutOperands)if(!StreamOperandMatches(v))return StreamInstallRefuse("layoutSignature",v.instruction);
    const UPtr calls[]={0x59DCF1u,0x59DE57u};
    for(UPtr c:calls)if(Read8(Address(c))!=0xE8u || Address(c)+5u+Read32(Address(c)+1u)!=Address(0x59E5B0u))return StreamInstallRefuse("resizeSignature",c);
    for(const auto& v:kStreamLayoutOperands){U32 n=StreamLayout(v.original);if(v.site==0x5A5C61u)n+=16u;if(!StreamWrite(v.site,&n,4u))return StreamInstallRefuse("layoutWrite",v.site);}
    for(const auto& b:kStreamBasePatches){U8 n[]={0x8B,b.newMod};if(!StreamWrite(b.site,n,2u))return StreamInstallRefuse("baseWrite",b.site);}
    for(UPtr c:calls){U8 n[5]={0xE8};Write32(reinterpret_cast<UPtr>(n)+1u,reinterpret_cast<UPtr>(&StreamResizeBridge)-(Address(c)+5u));if(!StreamWrite(c,n,5u))return StreamInstallRefuse("resizeWrite",c);}
    Write32(reinterpret_cast<UPtr>(schedulerDtor)+5u,StreamLayout(0x16568u));Write32(reinterpret_cast<UPtr>(vector)+2u,StreamCapacity()*4u);
    Write32(reinterpret_cast<UPtr>(fourth)+2u,StreamCapacity()*4u);
    for(U32 i=0;i<8u;++i)if(!DiagFInstallDetour(g_streamDetours[i],hooks[i].site,hooks[i].bytes,hooks[i].length,hooks[i].bridge,g_streamTramps[i]))return StreamInstallRefuse("installHook",hooks[i].site);
    g_streamStatus=1;return true;
}
static void StreamLog(bool startup,U32 tick){
    while(InterlockedExchange(&g_streamLock,1)){}
    U32 current=0,alive=0;
    for(auto* a=g_streamArenas;a;a=a->next){if(a->alive){++alive;current+=StreamBusy(a);}if(!StreamGuards(a)){InterlockedIncrement(&g_streamGuardBad);StreamFatal("sampledArenaGuard",a->owner);}}
    CapacityMax(g_streamPeak,current);
    U32 queues[4]={},schedulerAlive=0;for(auto* a=g_streamSchedulers;a;a=a->next)if(a->alive){
        ++schedulerAlive;
        if(!StreamSchedulerGuards(a->owner)){InterlockedIncrement(&g_streamGuardBad);StreamFatal("sampledSchedulerGuard",0u);}
        for(U32 i=0;i<4u;++i){const U32 n=Read32(a->owner+StreamVectorBase(i)+StreamCapacity()*4u);if(n>StreamCapacity())StreamFatal("sampledSchedulerCount",0u);queues[i]+=n;}
    }
    char line[1100]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,startup?"DSFV29s_STREAM_HANDLER_START revision=2":"DSFV29s_STREAM_HANDLER_STATUS",end);
    DiagFAppendKeyU(p," tick=",tick,end);DiagFAppendKeyU(p," status=",static_cast<U32>(g_streamStatus),end);
    DiagFAppendKeyU(p," capacity=",StreamCapacity(),end);DiagFAppendKeyU(p," schedulerQueueCapacity=",StreamCapacity(),end);
    DiagFAppendKeyU(p," membershipCapacityPerKind=",StreamCapacity(),end);DiagFAppendKeyU(p," schedulerBytes=",StreamSchedulerBytes(),end);
    DiagFAppendKeyU(p," generations=",static_cast<U32>(g_streamGenerations),end);DiagFAppendKeyU(p," initializations=",static_cast<U32>(g_streamInitializations),end);
    DiagFAppendKeyU(p," destructions=",static_cast<U32>(g_streamDestructions),end);DiagFAppendKeyU(p," alive=",alive,end);
    DiagFAppendKeyU(p," currentSampled=",current,end);DiagFAppendKeyU(p," peakSampled=",static_cast<U32>(g_streamPeak),end);
    DiagFAppendKeyU(p," schedulerGenerations=",static_cast<U32>(g_streamSchedulerGenerations),end);DiagFAppendKeyU(p," schedulerDestructions=",static_cast<U32>(g_streamSchedulerDestructions),end);
    DiagFAppendKeyU(p," schedulerAlive=",schedulerAlive,end);DiagFAppendKeyHex(p," lastOwner=",g_streamLastOwner,end);
    for(U32 i=0;i<4u;++i){const char* c[]={" queue0="," queue1="," queue2="," queue3="};const char* h[]={" queue0Peak="," queue1Peak="," queue2Peak="," queue3Peak="};DiagFAppendKeyU(p,c[i],queues[i],end);DiagFAppendKeyU(p,h[i],static_cast<U32>(g_streamQueuePeaks[i]),end);}
    DiagFAppendKeyU(p," failures=",static_cast<U32>(g_streamFailures),end);DiagFAppendKeyU(p," guardBad=",static_cast<U32>(g_streamGuardBad),end);
    DiagFAppendKeyU(p," membershipPeak=",static_cast<U32>(g_streamMemberPeak),end);
    DiagFAppendKeyU(p," layoutOperands=",sizeof(kStreamLayoutOperands)/sizeof(kStreamLayoutOperands[0]),end);
    AppendText(p," hooks=8 queueCount=4 nativeFourthQueueCapacity=512 basePatches=11 initializationCalls=2 nativeCapacity=256 stride=80 coldOnly=1 activeMigration=0 stableGenerationSlabs=1\n",end);*p=0;
    InterlockedExchange(&g_streamLock,0);CapacityWrite(line);
}

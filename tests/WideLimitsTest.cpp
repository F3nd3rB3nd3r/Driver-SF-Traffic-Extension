// Runs selected native routines in an isolated test process. The game entry
// point, rendering, audio middleware and game loop are never called.
#define DSF_V29S_NATIVE_FIXTURE 1
#define DllMain WideFixtureDllMain
#include "../src/DriverSF_V29b_NativeFastPath.cpp"
#undef DllMain
extern "C" {
__declspec(dllimport) HANDLE __stdcall GetStdHandle(U32);
__declspec(dllimport) U32 __stdcall GetFileSize(HANDLE,U32*);
__declspec(dllimport) const char* __stdcall GetCommandLineA();
__declspec(dllimport) U32 __stdcall SetErrorMode(U32);
}
static U32 g_checks=0;
static U32 g_expectedFailure=0;static U32* g_collectionCanary=nullptr;
static U32* g_completedCanary=nullptr;
static void Say(const char* s){U32 n=0;while(s[n])++n;U32 written=0;WriteFile(GetStdHandle(0xFFFFFFF5u),s,n,&written,nullptr);}
static void Number(U32 n){char b[16]={};U32 k=0;do{b[k++]=static_cast<char>('0'+n%10u);n/=10u;}while(n);char out[16]={};for(U32 i=0;i<k;++i)out[i]=b[k-i-1u];Say(out);}
static void CheckAt(bool okay,U32 line){++g_checks;if(!okay){Say("FAIL line=");Number(line);Say("\n");ExitProcess(line);}}
#define CHECK(x) CheckAt((x),__LINE__)
static void FixtureFailure(const char* s){
    if(g_expectedFailure==1u){CHECK(g_collectionCanary && *g_collectionCanary==0xA1234567u);CHECK(WideContains(s,"ResourceCollectionEntries exhausted"));}
    if(g_expectedFailure==2u)CHECK(WideContains(s,"allocation failed"));
    if(g_expectedFailure==3u)CHECK(WideContains(s,"byte overflow"));
    if(g_expectedFailure==4u){CHECK(g_completedCanary && *g_completedCanary==0xC011CAFEu);CHECK(WideContains(s,"completed-map"));}
    Say(s);
}
static U32 Argument(){const char* s=GetCommandLineA();if(*s=='"'){++s;while(*s && *s!='"')++s;if(*s)++s;}else while(*s && *s!=' ')++s;while(*s==' ')++s;U32 n=0;while(*s>='0' && *s<='9')n=n*10u+*s++-'0';return n?n:150u;}
static UPtr Alloc(U32 bytes){return reinterpret_cast<UPtr>(WideAllocate(bytes));}
static void MapNativeImage(){
    HANDLE f=CreateFileA("E:\\Games\\Driver San Francisco Save Bug Test\\Driver.exe",0x80000000u,1u,nullptr,3u,0u,nullptr);
    CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));U32 size=GetFileSize(f,nullptr);UPtr raw=reinterpret_cast<UPtr>(VirtualAlloc(reinterpret_cast<void*>(0x70000000u),size,0x3000u,4u));CHECK(raw);U32 read=0;
    CHECK(ReadFile(f,reinterpret_cast<void*>(raw),size,&read,nullptr) && read==size);CloseHandle(f);
    UPtr pe=raw+Read32(raw+0x3Cu);const U32 imageBytes=Read32(pe+24u+56u),headers=Read32(pe+24u+60u);
    // The fixture process has its own loader heaps in the low address range.
    // Relocate only the two absolute root operands in the initializer below;
    // all other fixture-executed instructions use relative/package pointers.
    g_base=reinterpret_cast<UPtr>(VirtualAlloc(reinterpret_cast<void*>(0x20000000u),imageBytes,0x3000u,0x40u));CHECK(g_base==0x20000000u);
    memcpy(reinterpret_cast<void*>(g_base),reinterpret_cast<void*>(raw),headers);
    const UPtr sections=pe+24u+Read16(pe+20u);for(U32 i=0;i<Read16(pe+6u);++i){const UPtr section=sections+i*40u;
        memcpy(reinterpret_cast<void*>(g_base+Read32(section+12u)),reinterpret_cast<void*>(raw+Read32(section+20u)),Read32(section+16u));}
    CHECK(VerifyExecutable());
}
static UPtr g_handlers=0;static U32 g_created=0,g_expectedModels=0;
static UPtr __fastcall FixtureFactory(void*,void*,UPtr out,U32,U32 kind,U32,U32 ordinal){
    const UPtr h=g_handlers+g_created++*0x570u;Write32(h,0xE37118u);
    typedef void (__thiscall* Setter)(void*,U32,U32);reinterpret_cast<Setter>(Address(0x858860u))(reinterpret_cast<void*>(h),0u,ordinal);
    CHECK(Read32(h+0x564u)==ordinal);CHECK(kind<3u);if(kind==0u)CHECK(ordinal<g_expectedModels);
    Write32(out,h);return out;
}
static void NativeInitializer(U32 models){
    CHECK(Read32(Address(0x5A1421u))==0xFE7228u && Read32(Address(0x5A1438u))==0xFE7228u);
    Write32(Address(0x5A1421u),Address(kRootPtr));Write32(Address(0x5A1438u),Address(kRootPtr));
    const UPtr root=Alloc(0x1000u),manager=Alloc(0x80000u),meta=Alloc(0x1000u);Write32(Address(kRootPtr),root);Write32(root+0x15Cu,Alloc(0x1000u));Write32(root+0x204u,manager);
    g_pkg0=manager+0xAF68u;Write32(g_pkg0+0x72F8u,meta);*reinterpret_cast<U8*>(meta+0x18u)=static_cast<U8>(models>255u?255u:models);*reinterpret_cast<U8*>(meta+0x19u)=2;*reinterpret_cast<U8*>(meta+0x1Au)=2;
    g_handlers=Alloc((models+4u)*0x570u);g_created=0;g_expectedModels=models;
    Write32(Address(0x5A1492u),reinterpret_cast<UPtr>(&FixtureFactory)-Address(0x5A1496u));
    typedef void (__thiscall* Init)(void*);reinterpret_cast<Init>(Address(0x5A1420u))(reinterpret_cast<void*>(g_pkg0));
    CHECK(g_created==models+4u);CHECK(Read32(g_pkg0+0x5730u)==models+4u);CHECK(Read32(g_pkg0+0x5834u)==models);
    for(U32 i=0;i<models+4u;++i)CHECK(g_globalHandlers[i]==g_handlers+i*0x570u);
    for(U32 i=64u;i<models;++i)CHECK(g_kind0Handlers[i]==g_handlers+i*0x570u);
    CHECK(Read32(g_handlers+(models-1u)*0x570u+0x564u)==models-1u);
    // Downstream graphics selection reads the ordinal as a DWORD and clamps
    // against the runtime descriptor count, rather than an8-bit constant.
    const UPtr graphics=Alloc(0x1000u),records=Alloc(models*0x60u);const U32 kind=9u;
    Write32(graphics+kind*0x5Cu+0x0Cu,records);Write32(graphics+kind*0x5Cu+0x10u,models);
    typedef void (__thiscall* Select)(void*,U32,U32,U32,U32);
    reinterpret_cast<Select>(Address(0xBB5690u))(reinterpret_cast<void*>(graphics),kind,1u,models-1u,0u);
    CHECK(Read32(graphics+4u)==models-1u && Read32(graphics+0x798u+kind*4u)==models-1u);
    reinterpret_cast<Select>(Address(0xBB5690u))(reinterpret_cast<void*>(graphics),kind,1u,models+10u,0u);
    CHECK(Read32(graphics+4u)==models-1u);
}
static void EmptyList(UPtr,U32);
static void RequestsAndSnapshots(U32 n){
    // Unknown caller preserves the supplied arbitrary list; no engine dispatch
    // is performed while native spool state is nonzero.
    const UPtr owner=Alloc(0x1000u);Write32(owner+0xDACu,1u);U32* ids=static_cast<U32*>(WideAllocate(n*4u));for(U32 i=0;i<n;++i)ids[i]=65000u-i;
    RequestWriteBridge(reinterpret_cast<void*>(owner),nullptr,ids,static_cast<int>(n));RequestSidecar* r=RequestSlot(owner,false);CHECK(r && r->count==n);
    for(U32 i=0;i<n;++i){CpuRegs cpu={};cpu.esi=owner;cpu.ebp=i;RequestReadBridge(&cpu);CHECK(cpu.edi==ids[i]);}
    U32* notified=static_cast<U32*>(WideAllocate(n*4u+4u));notified[n]=0x29FA1234u;
    CHECK(NotifyCopy(notified,reinterpret_cast<void*>(owner),n*4u)==notified);
    for(U32 i=0;i<n;++i)CHECK(notified[i]==ids[i]);CHECK(notified[n]==0x29FA1234u);
    ReplayData a={},b={};ReplayStorageInit(a,WideAllocate(ReplayStorageBytes()));ReplayStorageInit(b,WideAllocate(ReplayStorageBytes()));a.count=n;a.retained=g_limits.retained;a.extra=212u;
    for(U32 i=0;i<n;++i)a.ids[i]=ids[i];for(U32 i=0;i<a.retained;++i){a.pairs[i].id=63000u-i;a.pairs[i].refs=i+1u;}
    CHECK(ReplayValid(a));ReplayCopy(b,a);CHECK(ReplayValid(b) && b.count==n && b.retained==a.retained && b.extra==212u);
    a.ids[n-1u]=1;CHECK(b.ids[n-1u]==ids[n-1u]);a.pairs[a.retained-1u].refs=0;CHECK(b.pairs[b.retained-1u].refs==b.retained);
    for(U32 i=0;i<4u;++i){CHECK(a.guard[i]==kReplayGuard);CHECK(b.guard[i]==kReplayGuard);}
    // Large baseline snapshots retain their own arrays across resets/copies.
    WideBaselineReset(g_baseline);g_baseline.count=n;g_baseline.originalCount=static_cast<int>(n);g_baseline.originalValid=1;
    for(U32 i=0;i<n;++i){g_baseline.original[i]=ids[i];g_baseline.models[i].id=ids[i];}
    WideBaselineCopy(g_baselinePrint,g_baseline);WideBaselineReset(g_baseline);CHECK(g_baselinePrint.models[n-1u].id==ids[n-1u]);CHECK(g_baselinePrint.original[n-1u]==ids[n-1u]);
}
static void AssetRoster(U32 n){
    const UPtr root=Alloc(0x800u),context=Alloc(0x5CB000u),owner=Alloc(0x1000u),ai=Alloc(0x200u);
    const UPtr settings=Alloc(28u),lists=Alloc(6u),groups=Alloc(12u),routes=Alloc(20u),metadata=Alloc(12u),weights=Alloc(n*8u);
    Write32(Address(kRootPtr),root);Write32(root+0x758u,context);Write32(root+0x214u,owner);Write32(context+0x5CA840u,0x12345678u);Write32(Address(0xFE7AC0u),ai);
    const U32 counts[]={1u,3u,1u,1u,1u,n},offsets[]={0x118u,0x120u,0xE8u,0x128u,0x158u,0x160u};
    const UPtr pointers[]={settings,lists,groups,routes,metadata,weights};for(U32 i=0;i<6u;++i){Write32(ai+offsets[i],counts[i]);Write32(ai+offsets[i]+4u,pointers[i]);}
    Write32(settings,0x12345678u);*reinterpret_cast<U16*>(settings+14u)=3u;Write32(metadata+4u,n);
    U32 native[3]={65000u,64999u,64998u};for(U32 i=0;i<3u;++i)*reinterpret_cast<U16*>(lists+i*2u)=static_cast<U16>(native[i]);
    for(U32 i=0;i<n;++i){Write32(weights+i*8u,65000u-i);Write32(weights+i*8u+4u,0x3F800000u);}
    const U32* list=native;int count=3;g_chapterStatus=1;ChapterSelectRequest(Address(0x76FE9Au),owner,list,count);
    CHECK(count==static_cast<int>(n));for(U32 i=0;i<n;++i)CHECK(list[i]==65001u-n+i);GenericRelease(owner);
    list=native;count=3;g_limits.assetWeights=0;ChapterSelectRequest(Address(0x76FE9Au),owner,list,count);CHECK(list==native && count==3);g_limits.assetWeights=1;
}
static void AppendFixtureNode(UPtr h,UPtr end){const UPtr node=Read32(h),previous=Read32(end);Write32(h,node+12u);Write32(node,previous);Write32(node+4u,end);Write32(previous+4u,node);Write32(end,node);Write32(h+8u,Read32(h+8u)+1u);}
static void ReuseStorage(U32 n){
    const UPtr owner=Alloc(0x1000u),h=owner+0xCCu;EmptyList(h,0x264u);Write32(owner+0xC8u,n);g_sScratchHook=2;const char* why=nullptr;
    CHECK(ReusePrepareSpool(owner,why));SpoolReuseArena* a=ReuseFind(owner);
    for(U32 i=0;i<n;++i)AppendFixtureNode(h,h+0x264u);
    ReuseListState state={};CHECK(ReuseValidate(h,50u,a,state,why) && state.active==n);
    const U32 physical=GenericRequestStorage();const UPtr frameBase=Alloc(physical*12u+128u),scratch=frameBase+0x14u,end=frameBase+physical*12u+32u;
    EmptyList(scratch,12u+physical*12u);for(U32 i=0;i+1u<physical;++i)AppendFixtureNode(scratch,end);
    CpuRegs cpu={};cpu.savedEsp=frameBase-4u;cpu.eax=Read32(scratch);CHECK(ReusePrepareScratch(0u,reinterpret_cast<UPtr>(&cpu),why));
}
static void AudioDirectories(U32 n){
    // More than eight manager lifetimes and more than512 distinct diagnostic
    // keys, with stable pointers and per-manager tail guards.
    for(U32 i=0;i<12u;++i){AudioModelInstance* r=AudioModelFindInstance(0x10000000u+i*0x1000u,true);CHECK(r && r->storage);for(U32 k=0;k<4u;++k)CHECK(Read32(r->storage+g_limits.audioModels*16u+k*4u)==kAudioModelGuard);}
    CHECK(kAudioModelInstances==12u);for(U32 i=1;i<=n+600u;++i){AudioModelKey* k=AudioModelFindKey(i);CHECK(k && static_cast<U32>(k->key)==i);}
    AudioModelKey* a=AudioModelFindKey(1u);AudioModelKey* b=AudioModelFindKey(65537u);CHECK(a!=b && AudioModelFindKey(1u)==a && AudioModelFindKey(65537u)==b);
}
static void Recycle(UPtr h,UPtr node,U32 endOffset){
    const UPtr previous=Read32(node),next=Read32(node+4u);Write32(previous+4u,next);Write32(next,previous);
    Write32(node,Read32(h));Write32(h,node);Write32(h+4u,Read32(h+4u)+1u);Write32(h+8u,Read32(h+8u)-1u);
    CHECK(Read32(h+endOffset+4u)!=node);
}
static void EmptyList(UPtr h,U32 endOffset){Write32(h,h+12u);Write32(h+4u,0);Write32(h+8u,0);Write32(h+endOffset,h+endOffset);Write32(h+endOffset+4u,h+endOffset);}
static void CompletedMaps(){
    const UPtr savedPackage=g_pkg0;Node16* savedExtra=g_mapExtra;
    const bool full=WideContains(GetCommandLineA()," completed-full"),misaligned=WideContains(GetCommandLineA()," completed-misaligned"),foreign=WideContains(GetCommandLineA()," completed-foreign");
    for(U32 order=0;order<2u;++order){
        const UPtr packageAddress=order?0x10000000u:0x35000000u,extraAddress=order?0x35000000u:0x10000000u;
        const UPtr package=reinterpret_cast<UPtr>(VirtualAlloc(reinterpret_cast<void*>(packageAddress),0x10000u,0x3000u,4u));
        const UPtr external=reinterpret_cast<UPtr>(VirtualAlloc(reinterpret_cast<void*>(extraAddress),kMapExtraNodes*16u+16u,0x3000u,4u));CHECK(package==packageAddress && external==extraAddress);
        g_pkg0=package;g_mapExtra=reinterpret_cast<Node16*>(external);const UPtr map=package+0x5B44u,end=map+0x40Cu;EmptyList(map,0x40Cu);
        g_completedCanary=reinterpret_cast<U32*>(external+kMapExtraNodes*16u);*g_completedCanary=0xC011CAFEu;
        for(U32 i=0;i<64u+kMapExtraNodes;++i){U32 out=0,data[2]={298u+i,0xCA000000u+i};
            const UPtr result=MapInsertBridge(reinterpret_cast<void*>(map),nullptr,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data));
            if(!result){Say("completed map rejected order=");Number(order);Say(" index=");Number(i);Say(" boundary=");Number(DSFV27B_UnsafeBoundary);Say("\n");}
            CHECK(result==reinterpret_cast<UPtr>(&out));CHECK(out==(i<64u?map+12u+i*16u:external+(i-64u)*16u));CHECK(Read32(out+12u)==data[1]);
        }
        CHECK(*g_completedCanary==0xC011CAFEu && Read32(map+8u)==64u+kMapExtraNodes);
        if(full || misaligned || foreign){g_expectedFailure=4u;if(misaligned)Write32(map,external+1u);if(foreign)Write32(map,package+0x7000u);
            U32 out=0,data[2]={65000u,123u};MapInsertBridge(reinterpret_cast<void*>(map),nullptr,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data));CHECK(false);}
        for(U32 i=0;i<32u;++i)Recycle(map,Read32(end+4u),0x40Cu);
        for(U32 i=0;i<32u;++i){U32 out=0,data[2]={32000u+i,i};CHECK(MapInsertBridge(reinterpret_cast<void*>(map),nullptr,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data))==reinterpret_cast<UPtr>(&out));}
        CHECK(*g_completedCanary==0xC011CAFEu && !Read32(map+4u));
        // A reconstructed package starts inline again even after prior overflow.
        EmptyList(map,0x40Cu);U32 out=0,data[2]={298u,0x1234u};CHECK(MapInsertBridge(reinterpret_cast<void*>(map),nullptr,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data))==reinterpret_cast<UPtr>(&out));CHECK(out==map+12u && Read32(map+8u)==1u);
        CHECK(VirtualFree(reinterpret_cast<void*>(package),0u,0x8000u) && VirtualFree(reinterpret_cast<void*>(external),0u,0x8000u));
    }
    g_pkg0=savedPackage;g_mapExtra=savedExtra;
}
static void Queues(){
    const UPtr section=Alloc(0x7400u),h=section+0x5F5Cu,end=h+0x40Cu;EmptyList(h,0x40Cu);
    g_pkg0=section;g_pendingStatus=1;CHECK(g_origMapInsert);
    for(U32 i=0;i<g_limits.pending;++i){U32 out=0,data[2]={i,0xABC00000u+i};CHECK(PendingInsert(h,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data)));CHECK(out && Read32(out+12u)==data[1]);}
    PendingArena* p=PendingFind(h);CHECK(p && PendingValidate(p) && !PendingGuards(p));
    for(U32 i=0;i<32u;++i)Recycle(h,Read32(end+4u),0x40Cu);
    CHECK(PendingValidate(p));
    for(U32 i=0;i<32u;++i){U32 out=0,data[2]={g_limits.pending+i,0xDD000000u+i};CHECK(PendingInsert(h,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data)));}
    CHECK(PendingValidate(p) && g_pendingReuses==32);
    // Same address, new constructor generation retains old nodes and allocates
    // separate backing rather than reinterpreting stale native cursors.
    EmptyList(h,0x40Cu);PendingArena* newer=PendingPrepare(h);CHECK(newer!=p && newer->generation>p->generation && PendingValidate(newer));
    g_callbackGroups=static_cast<CallbackGroup*>(WideAllocate(sizeof(CallbackGroup)*kCallbackGroups));g_callbackStatus=1;
    g_callbackNativeInsert=reinterpret_cast<CallbackNativeInsert>(Address(0x5567D0u));
    for(U32 slot=0;slot<kCallbackGroups;++slot){CallbackGroup& g=g_callbackGroups[slot];g.nodes=static_cast<CallbackNode*>(WideAllocate(kCallbackExtra*12u+16u));g.after=reinterpret_cast<U32*>(g.nodes+kCallbackExtra);
        for(U32 j=0;j<4u;++j)g.before[j]=g.after[j]=kCallbackGuard;EmptyList(CallbackHeader(slot),0x30Cu);}
    const UPtr cb=CallbackHeader(20u);
    for(U32 i=0;i<g_limits.callbacks;++i){U32 value=0xD0000000u+i;CHECK(CallbackInsertBridge(reinterpret_cast<void*>(cb),nullptr,reinterpret_cast<UPtr>(&value)));}
    CHECK(CallbackValidate(20u));for(U32 i=0;i<32u;++i)Recycle(cb,Read32(cb+0x310u),0x30Cu);CHECK(CallbackValidate(20u));
    for(U32 i=0;i<32u;++i){U32 value=i;CHECK(CallbackInsertBridge(reinterpret_cast<void*>(cb),nullptr,reinterpret_cast<UPtr>(&value)));}
    CHECK(CallbackValidate(20u) && g_callbackGroups[20].reuses==32);
    // Reconstruct definition-map owner in place, then exhaust all configured
    // external nodes using the actual admission helper and native node stride.
    const UPtr owner=Alloc(0x1000u),map=owner+0x254u;Write32(owner,RegMapAddress(0xE395D0u));EmptyList(map,0x444u);RegMapDetail d={};CHECK(RegMapConstructed(owner,d));
    for(U32 i=0;i<45u+g_limits.definitions;++i){d={};CHECK(RegMapPrepare(map,d));UPtr node=Read32(map);Write32(map,node+24u);Write32(map+8u,i+1u);}
    d={};CHECK(!RegMapPrepare(map,d) && WideContains(d.reason,"externalEnd"));RegMapArena* a=RegMapFind(map);CHECK(RegMapGuards(a));
    EmptyList(map,0x444u);d={};CHECK(RegMapConstructed(owner,d) && a==RegMapFind(map) && a->generation==2u && !a->externalIssued);
}
static UPtr g_fixtureParkRoot=0,g_fixtureParkNodes=0,g_fixtureDescriptor=0;static U32 g_fixtureParkCount=0,g_fixtureParkConcentrated=0;
static UPtr __cdecl FixtureParkFactory(){return g_fixtureParkRoot;}
static void __fastcall FixtureParkClone(void*,void*,UPtr h){const UPtr end=h+0x264u;Write32(h+0x268u,g_fixtureParkCount?g_fixtureParkNodes:end);
    for(U32 i=0;i<g_fixtureParkCount;++i){const UPtr node=g_fixtureParkNodes+i*12u;Write32(node+4u,i+1u<g_fixtureParkCount?node+12u:end);Write32(node+8u,1000u+i);}}
static UPtr __fastcall FixtureParkDescribe(void*,void*,U32 id){*reinterpret_cast<U8*>(g_fixtureDescriptor+0x1Bu)=static_cast<U8>(g_fixtureParkConcentrated?0u:(id-1000u)%25u);return g_fixtureDescriptor;}
static void Parked(U32 n){
    const UPtr code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,0x40u)),owner=Alloc(0x3000u);CHECK(code);
    g_fixtureParkRoot=Alloc(0x800u);Write32(g_fixtureParkRoot+0x214u,1u);Write32(g_fixtureParkRoot+0x780u,1u);g_fixtureDescriptor=Alloc(0x20u);g_fixtureParkNodes=Alloc(n*12u);g_fixtureParkCount=n;
    ParkFillImage(code,code,reinterpret_cast<UPtr>(&g_fixtureParkRoot),Address(0xC287F0u),reinterpret_cast<UPtr>(&FixtureParkFactory),reinterpret_cast<UPtr>(&FixtureParkClone),reinterpret_cast<UPtr>(&FixtureParkDescribe),reinterpret_cast<UPtr>(&ParkAppend),reinterpret_cast<UPtr>(&ParkBucketBridge),true);
    g_parkBucketTrampoline=code+0x300u;g_parkBuilder=code;CHECK(FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(code),0x1000u));
    for(U32 concentrated=0;concentrated<2u;++concentrated){g_fixtureParkConcentrated=concentrated;ParkRebuild(reinterpret_cast<void*>(owner),nullptr);ParkSlot* slot=ParkFind(owner,false);CHECK(ParkSlotGuard(slot) && slot->count==n);
        for(U32 i=0;i<n;++i){U32 id=ParkPick(reinterpret_cast<void*>(owner),nullptr,0x709u,i);CHECK(id>=1000u && id<1000u+n);CHECK(!concentrated || id==1000u+i);}}
    CHECK(!g_parkInvalid && !g_parkGuardBad && !g_parkConcurrent);
}
extern "C" __declspec(naked) void FixtureStackProbe(){__asm {
    push ebp
    mov ebp,esp
    mov eax,18000h
    call WideProbeStack
    sub esp,eax
    mov dword ptr [esp],12345678h
    mov esp,ebp
    pop ebp
    ret
}}
extern "C" __declspec(naked) void FixtureReturn(){__asm {ret}}
static void CollectionStore(U32 index){
    U32 before=0,after=0,eaxAfter=0;const UPtr bridge=Address(0x6A593Fu);
    __asm {
        pushad
        mov edi,index
        mov edx,87654321h
        mov eax,12345678h
        cmp edi,edx
        pushfd
        pop before
        call bridge
        pushfd
        pop after
        mov eaxAfter,eax
        popad
    }
    CHECK(before==after && eaxAfter==0x12345678u);
}
static void Collection(){
    CHECK(g_wideCollectionGuardInstalled && IndexedPendingRedirectIntact());
    const UPtr saved=g_wideCollectionResume;g_wideCollectionResume=reinterpret_cast<UPtr>(&FixtureReturn);
    CollectionStore(0u);CollectionStore(g_limits.collection-1u);
    CHECK(g_sResourcePairs[0]==0x87654321u && g_sResourcePairs[(g_limits.collection-1u)*2u]==0x87654321u);
    CHECK(*g_collectionCanary==0xA1234567u);
    if(g_expectedFailure==1u){CollectionStore(g_limits.collection);CHECK(false);}
    g_wideCollectionResume=saved;CHECK(RestoreResourceCollection());CHECK(!g_wideCollectionGuardInstalled && !g_sResourcePatchCount);
    CHECK(Read32(Address(0x6A5942u))==0x186A6B8u);
}
static void FullStartup(){
    // Execute the complete cold installer sequence against the copied image,
    // without starting Monitor, invoking game entry or constructing game data.
    const U32 methods[]={0x73FB40u,0x73FB20u,0x8203B0u,0x73FB30u,0x73F910u,0x73F930u,0x73F950u,0x73FA00u,0x73F9F0u,0x73F970u};
    CHECK(Read32(Address(0xE2CA74u))==0x53AA20u);Write32(Address(0xE2CA74u),Address(0x53AA20u));
    for(const auto& c:kSCClients){const UPtr at=Address(c.vtable)+0x20u;CHECK(Read32(at)==kSCPatches[c.method].at);Write32(at,Address(kSCPatches[c.method].at));}
    for(U32 i=0;i<10u;++i){const UPtr at=Address(0xE2A088u)+i*4u;CHECK(Read32(at)==methods[i]);Write32(at,Address(methods[i]));}
    g_capacityFile=GetStdHandle(0xFFFFFFF5u);CHECK(ChapterPreflight());ApplyPreinitGraphicsExpansion();CHECK(DSF150_Status==1);
    CHECK(InstallPool512Patches() && InstallRequestFramePatches() && InstallMerge150Fix() && InstallExtendedSites() && InstallHashArenaHook());
    g_sRegMapHook=DiagSRegMapInstall()?1:-1;CHECK(g_sRegMapHook==1 && InstallModelPoolCapacity() && InstallResourceCollection() && InstallHandlerHook());
    DSFV29s_Hooks=DiagSInstall()?5:-1;g_sMutationHooks=DiagSMutationInstall()?3:-1;g_sGuardHook=DiagSGuardInstall()?1:-1;g_sScratchHook=DiagSScratchInstall()?2:-1;g_sCloneHook=DiagSCloneInstall()?1:-1;
    CHECK(DSFV29s_Hooks==5 && g_sMutationHooks==3 && g_sGuardHook==1 && g_sScratchHook==2 && g_sCloneHook==1);
    g_capacityHooks=CapacityInstall()?3:-1;CHECK(g_capacityHooks==3 && IndexedInstall());IndexedStartLine();
#if defined(DSF_V29S_STREAM_HANDLERS)
    CHECK(StreamInstall());
#if defined(DSF_V29S_MANAGER_KEYS)
    CHECK(ManagerKeyInstall());
#endif
#endif
#if defined(DSF_V29S_TRAFFIC_QUEUES)
    CapacityStartLine();
#endif
    CHECK(AudioProbeInstall());AudioDefaultPoolInstall();AudioDefaultPoolStartLine();CHECK(g_audioDefaultStatus==1);
    AudioSlotsInstall();AudioSlotsStartLine();CHECK(g_audioSlotsStatus==1);AudioReasonInstall();AudioReasonStartLine();CHECK(g_audioReasonStatus==1);
    AudioModelInstall();AudioModelStartLine();CHECK(g_audioModelStatus==1);CleanupInstall();CleanupStartLine();CHECK(g_cleanupStatus==1);
    CHECK(PendingInstall());PendingStartLine();ChapterStartLine();CHECK(CallbackInstall());CallbackStartLine();
    ArenaReuseStartLine();BaselineInstall();CHECK(g_baselineInstalled);NotifyInstall();CHECK(g_notifyStatus==1);ParkInstall();CHECK(g_parkStatus==1);
    ReplayInstall();CHECK(g_replayStatus==1);PauseInstall();CHECK(g_pauseStatus==1);SCInstall();CHECK(g_scStatus==1);RFInstall(true);CHECK(g_rfStatus==1);RegMapStartLine();AudioProbeStartLine();
    CHECK(IndexedPendingRedirectIntact() && ArenaReuseStartup());
    Say("PASS full cold startup capacity=");Number(g_limits.requests);Say("\n");ExitProcess(0u);
}
extern "C" void WideTestMain(){
    SetErrorMode(3u);g_wideFailureSink=&FixtureFailure;const U32 n=Argument();CHECK(n<=10000u);
    HANDLE iniFile=CreateFileA("dist\\USERCAP_R3\\DriverSF_Unlimiter.ini",0x80000000u,1u,nullptr,3u,0u,nullptr);CHECK(iniFile!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    static char ini[4097]={};U32 iniSize=0;CHECK(ReadFile(iniFile,ini,sizeof(ini),&iniSize,nullptr));CloseHandle(iniFile);GenericLimits defaults=GenericParse(ini,iniSize);
    CHECK(defaults.valid && defaults.requests==150u && defaults.models==160u && defaults.objects==512u && defaults.audioModels==192u && defaults.audioMiB==64u && defaults.replayMiB==1u && defaults.retained==256u);
    const char config[]="[Limits]\nRequestCapacity=700\nAudioModelEntries=800\nReplayRetainedReferences=900\nModelSlots=720\nObjectEntries=1024\nDefinitionEntries=1500\nPendingEntries=2048\nCallbackEntriesPerGroup=4096\nCleanupEntries=1048576\nResourceCollectionEntries=300000\nIndexedReferenceEntries=524288\nHashExtraEntries=90000\nGlobalAudioEntries=16384\nDefaultAudioMiB=512\nReplayReaderMiB=32\n[Traffic]\nRosterSource=AssetWeights\n";
    GenericLimits parsed=GenericParse(config,sizeof(config)-1u);CHECK(parsed.valid && parsed.requests==700u && parsed.models==720u && parsed.replayMiB==32u);
    const char bad[]="[Limits]\nRequestCapacity=4294967295\n";CHECK(!GenericParse(bad,sizeof(bad)-1u).valid);
    const char badBytes[]="[Limits]\nDefaultAudioMiB=4096\n";CHECK(!GenericParse(badBytes,sizeof(badBytes)-1u).valid);
    const char badModels[]="[Limits]\nModelSlots=100\n";CHECK(!GenericParse(badModels,sizeof(badModels)-1u).valid);
    g_limits={n,WideMax(192u,n+32u),2048,64,1,WideMax(256u,n),1,true,"configured",0,0,0,0,0,0,0,0,0};g_limits=GenericValidate(g_limits);CHECK(g_limits.valid);
    if(WideContains(GetCommandLineA()," customized"))g_limits=parsed;
    MapNativeImage();CHECK(WideStartupStorage());g_wideFailureSink=&FixtureFailure;
    if(WideContains(GetCommandLineA()," startup"))FullStartup();
    if(WideContains(GetCommandLineA()," allocation-failure")){g_expectedFailure=2;WideAllocate(0x7FFFFFFFu);CHECK(false);}
    if(WideContains(GetCommandLineA()," byte-overflow")){g_expectedFailure=3;WideAllocate(0u);CHECK(false);}
    if(WideContains(GetCommandLineA()," collection-full"))g_expectedFailure=1;
    g_sResourcePairs=static_cast<U32*>(WideAllocate(g_limits.collection*8u+4u));g_collectionCanary=g_sResourcePairs+g_limits.collection*2u;*g_collectionCanary=0xA1234567u;
    CHECK(InstallResourceCollection());Collection();
    CHECK(InstallPool512Patches());CHECK(InstallRequestFramePatches());CHECK(InstallMerge150Fix());CHECK(InstallExtendedSites());
    CHECK(Read32(reinterpret_cast<UPtr>(g_merge150Clone)+0x10u)==12u*GenericRequestStorage()+16u);
    CHECK(Read32(reinterpret_cast<UPtr>(g_merge150Clone)+0x16u)==12u*GenericRequestStorage()+12u);
    CHECK(g_wideRequestProbeInstalled);NativeInitializer(g_limits.models);
    g_wideAudioBuckets=static_cast<AudioModelKey**>(WideAllocate(65536u*4u));g_audioModelStorage=reinterpret_cast<UPtr>(g_wideAudioBuckets);
    CompletedMaps();RequestsAndSnapshots(n);AudioDirectories(n);AssetRoster(n);ReuseStorage(n);Say("queues\n");Queues();Say("parked\n");Parked(n);Say("stack\n");FixtureStackProbe();
    Say("PASS capacity=");Number(n);Say(" models=");Number(g_limits.models);Say(" checks=");Number(g_checks);Say("\n");ExitProcess(0u);
}


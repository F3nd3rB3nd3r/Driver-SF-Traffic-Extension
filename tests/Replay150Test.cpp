typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" int _fltused=0;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("replay fixture failure at line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedDecrement(volatile long* p){return --*p;}
static long InterlockedExchange(volatile long* p,long n){long old=*p;*p=n;return old;}
static void ParkPeak(volatile long* p,U32 n){if(n>static_cast<U32>(*p))*p=static_cast<long>(n);}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool Readable(UPtr p,U32){return p>=0x10000u;}
static UPtr testSpooler=0,testOwner=0,code=0,frame=0,stackTop=0,entry=0,hostEsp=0,afterEsp=0,afterEbp=0;
static U32 threadId=1,fatals=0,events=0,wireBytes=0,savedCount=0,savedRetained=0,savedExtra=0;
static U32 afterRegs[3]={},requested[150]={},requestCount=0,releases=0,retains=0,lastExtra=0,callOrder=0;
static bool idle=true,allocationFailure=false;static U8 root[0x220]={};static UPtr rootPointer=0;
static U32 trafficNodes[151][3]={},retainedNodes[257][5]={},pendingNodes[257][5]={};
static U32 cloneNodes[260][3]={},sourceCount=0,sourceIds[150]={};static U8 wire[3000]={};
static void ReplayFatal(const char*,UPtr,U32){++fatals;}
static void* ReplayAllocate(U32 n){return allocationFailure?nullptr:VirtualAlloc(nullptr,n,0x3000u,4u);}
static U32 ReplayThread(){return threadId;}static UPtr ReplaySpooler(){return testSpooler;}static bool ReplayIdle(UPtr p){Check(p==testSpooler);return idle;}
static U32 ReplayRefs(UPtr p,U32 id){typedef U32 (__thiscall* Fn)(void*,U32);return reinterpret_cast<Fn>(code+0x2400u)(reinterpret_cast<void*>(p),id);}
static void ReplayRequest(UPtr p,const U32* ids,U32 n){Check(p==testSpooler && n<=150u);requestCount=n;for(U32 i=0;i<n;++i)requested[i]=ids[i];callOrder=1u;}
static void ReplayReference(UPtr p,U32,bool retain){Check(p==testSpooler);if(retain){++retains;callOrder=3u;}else{Check(callOrder<=2u);++releases;callOrder=2u;}}
static void ReplayExtra(UPtr p,U32 value){Check(p==testSpooler);lastExtra=value;callOrder=4u;}
static void ReplayAppend(UPtr,const void* source,U32 n){Check(wireBytes+n<=sizeof(wire));memcpy(wire+wireBytes,source,n);wireBytes+=n;}
#include "../src/V29sReplay150Native.inc"
#if defined(DSF_V29S_REPLAY_PAUSE)
static U32 observedPauseImports=0;
extern "C" void __cdecl PauseOnImport(UPtr snapshot){Check(snapshot==testOwner);++observedPauseImports;}
#endif
#include "../src/V29sReplay150Core.inc"
#include "../src/Chapter7Preload.h"
static UPtr patchPage=0;static UPtr Address(U32 p){return patchPage+p-0x74B000u;}
#include "../src/V29sReplay150Layout.inc"
static void ReplayEvent(const char*,UPtr,const ReplayData& d){Check(ReplayValid(d));Check(ReplayHash(d)!=0u);++events;}
static void Call(UPtr at,UPtr target){reinterpret_cast<U8*>(at)[0]=0xE8u;Write32(at+1u,target-at-5u);}
static void Jump(UPtr at,UPtr target){reinterpret_cast<U8*>(at)[0]=0xE9u;Write32(at+1u,target-at-5u);reinterpret_cast<U8*>(at)[5]=0x90u;}
static UPtr __fastcall Accessor(void* self,void*){return reinterpret_cast<UPtr>(self)+0x438E4u;}
static UPtr __cdecl Factory(){return rootPointer;}
static bool __fastcall Ready(void*,void*){return idle;}
static void __fastcall NativeClone(void*,void*,UPtr header){
    const UPtr sentinel=header+0x264u;Write32(header+0x268u,sourceCount?reinterpret_cast<UPtr>(cloneNodes):sentinel);
    for(U32 i=0;i<sourceCount;++i){cloneNodes[i][2]=sourceIds[i];cloneNodes[i][1]=i+1u<sourceCount?reinterpret_cast<UPtr>(&cloneNodes[i+1u]):sentinel;}
}
static void __fastcall EmptyClone(void*,void*,UPtr header){Write32(header+0x268u,header+0x264u);}
static U32 __fastcall NativeRefs(void* p,void*,U32 id){return ReplayRefs(reinterpret_cast<UPtr>(p),id);}
static U32 argPointer=0;static bool pushArg=false;
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov hostEsp,esp
    fninit
    mov esp,stackTop
    mov ebp,frame
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,testOwner
    cmp pushArg,0
    je no_arg
    push argPointer
no_arg:
    call dword ptr [entry]
    mov afterRegs,ebx
    mov afterRegs+4,esi
    mov afterRegs+8,edi
    mov afterEsp,esp
    mov afterEbp,ebp
    mov esp,hostEsp
    popad
    ret
}}
static void Abi(){Check(afterEsp==stackTop && afterEbp==frame);Check(afterRegs[0]==0x44444444u && afterRegs[1]==0x55555555u && afterRegs[2]==0x66666666u);}
static void EmptyLists(){for(U32 off=0x330u;off;off=off==0x330u?0x998u:off==0x998u?0xD94u:0u){Write32(testSpooler+off,testSpooler+off);Write32(testSpooler+off+4u,testSpooler+off);}Write32(testSpooler+0xDACu,0u);Write32(testSpooler+0xDB0u,0xFFFFFFFFu);}
static void Source(U32 n,U32 retained=0u){
    EmptyLists();sourceCount=n;Check(n<=150u);
    for(U32 i=0;i<150u;++i)sourceIds[i]=kChapter7AllPositiveIds[i];
    for(U32 i=1;i<150u;++i){const U32 id=sourceIds[i];U32 j=i;while(j && sourceIds[j-1u]>id){sourceIds[j]=sourceIds[j-1u];--j;}sourceIds[j]=id;}
    for(U32 i=0;i<n;++i){trafficNodes[i][2]=sourceIds[i];trafficNodes[i][1]=i+1u<n?reinterpret_cast<UPtr>(&trafficNodes[i+1u]):testSpooler+0x330u;}
    Write32(testSpooler+0x334u,n?reinterpret_cast<UPtr>(trafficNodes):testSpooler+0x330u);
    for(U32 i=0;i<retained;++i){retainedNodes[i][2]=1000u+i;retainedNodes[i][4]=i%3u+1u;retainedNodes[i][1]=i+1u<retained?reinterpret_cast<UPtr>(&retainedNodes[i+1u]):testSpooler+0x998u;}
    Write32(testSpooler+0x99Cu,retained?reinterpret_cast<UPtr>(retainedNodes):testSpooler+0x998u);
    Write32(testOwner+8u,0u);wireBytes=0u;releases=retains=requestCount=callOrder=0u;lastExtra=0xFFFFFFFFu;
}
static void SetupSlices(){
    memcpy(reinterpret_cast<void*>(code),kReplaySaveSliceNative,sizeof(kReplaySaveSliceNative));
    memcpy(reinterpret_cast<void*>(code+0x400u),kReplayLoadSliceNative,sizeof(kReplayLoadSliceNative));
    for(U32 i=0;i<2u;++i){const UPtr base=code+(i?0x400u:0u);const U32 old=i?0x74DEDCu:0x74C53Eu;
        const U32 sites[2][4]={{0x74C54Bu,0x74C552u,0x74C57Du,0x74C59Fu},{0x74DF09u,0x74DF4Au,0x74DF7Du,0x74DF84u}};
        const UPtr targets[2][4]={{reinterpret_cast<UPtr>(&Accessor),code+0x1000u,reinterpret_cast<UPtr>(&ReplaySaveTraffic),reinterpret_cast<UPtr>(&ReplaySaveRetained)},
            {reinterpret_cast<UPtr>(&ReplayLoadTraffic),reinterpret_cast<UPtr>(&ReplayLoadRetained),reinterpret_cast<UPtr>(&Accessor),code+0x1100u}};
        for(U32 j=0;j<4u;++j)Call(base+sites[i][j]-old,targets[i][j]);
        reinterpret_cast<U8*>(base)[i?sizeof(kReplayLoadSliceNative):sizeof(kReplaySaveSliceNative)]=0xC3u;
    }
    Jump(code+0x1000u,reinterpret_cast<UPtr>(&ReplayCapture));Jump(code+0x1100u,reinterpret_cast<UPtr>(&ReplayImport));Jump(code+0x1200u,reinterpret_cast<UPtr>(&ReplayRestore));
}
static void Save(){
    wireBytes=0u;Write32(frame-0xACu,testOwner-0x438E4u);Write32(frame-0x1Cu,1u);Write32(frame-0xB4u,0xA5A5A5A5u);
    entry=code;pushArg=false;Invoke();Abi();Check(!g_replayActive && Read32(frame-0xB4u)==0xA5A5A5A5u);
    savedExtra=Read32(frame-0x54u);savedCount=Read32(frame-0x50u);savedRetained=Read32(frame-0x4Cu);
    Check(wireBytes==savedCount*4u+savedRetained*8u);
}
static void Load(bool immediate){
    U32 header[13]={};header[5]=savedExtra;header[6]=savedCount;header[7]=savedRetained;
    Write32(frame-0x28u,reinterpret_cast<UPtr>(header));Write32(frame-0x24u,reinterpret_cast<UPtr>(wire));Write32(frame-0xCu,testOwner-0x438E4u);
    Write32(frame-0x34u,0x5A5A5A5Au);idle=immediate;entry=code+0x400u;pushArg=false;Invoke();Abi();Check(!g_replayActive && Read32(frame-0x34u)==0x5A5A5A5Au);
}
static void RoundTrip(U32 n,U32 retained){
    Source(n,retained);Save();Check(savedCount==n && savedRetained==retained);for(U32 i=0;i<n;++i)Check(Read32(reinterpret_cast<UPtr>(wire)+i*4u)==sourceIds[i]);
    if(n>50u)Check(Read32(reinterpret_cast<UPtr>(wire)+200u)==181u);
    const UPtr neighbor=testOwner+0x270u;Write32(neighbor,0xA5C35A3Cu);Load(true);Check(Read32(neighbor)==0xA5C35A3Cu && Read32(testOwner+8u)==2u);
    Check(requestCount==n);for(U32 i=0;i<n;++i)Check(requested[i]==sourceIds[i]);U32 refs=0;for(U32 i=0;i<retained;++i)refs+=i%3u+1u;Check(releases==refs && retains==refs);
    // Capture during native phase2 must use the saved full cache, not live lists.
    Write32(testSpooler+0x334u,testSpooler+0x330u);Save();Check(savedCount==n && savedRetained==retained);for(U32 i=0;i<n;++i)Check(Read32(reinterpret_cast<UPtr>(wire)+i*4u)==sourceIds[i]);
}
static void NativeCollectionCheck(){
    Source(18,3u);retainedNodes[1][4]=0u;retainedNodes[2][4]=0xFFFFFFFFu;
    for(U32 i=0;i<2u;++i){pendingNodes[i][2]=2000u+i;pendingNodes[i][3]=i;pendingNodes[i][4]=4u+i;pendingNodes[i][1]=i?testSpooler+0xD94u:reinterpret_cast<UPtr>(&pendingNodes[1]);}
    Write32(testSpooler+0xD98u,reinterpret_cast<UPtr>(pendingNodes));Write32(testSpooler+0xDACu,2u);Write32(testSpooler+0xD9Cu,3000u);Write32(testSpooler+0xDA4u,7u);
    U32 list[160]={};typedef void (__thiscall* Fn)(void*,void*);reinterpret_cast<Fn>(code+0x2000u)(reinterpret_cast<void*>(testSpooler),list);
    ReplayData d={};Check(ReplayCollectRetained(testSpooler,d));Check(d.retained==3u);UPtr node=list[0x268u/4u];U32 i=0;
    while(node!=reinterpret_cast<UPtr>(list)+0x264u){Check(i<d.retained && d.pairs[i].id==Read32(node+8u));node=Read32(node+4u);++i;}
    Check(i==d.retained);Check(ReplayRefs(testSpooler,1000u)==1u && ReplayRefs(testSpooler,2000u)==4u && ReplayRefs(testSpooler,3000u)==7u);
    Say("native retained-list filters, order and special model match\n");
}
extern "C" void __stdcall ReplayTestMain(){
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x4000u,0x3000u,0x40u));Check(code);
    testSpooler=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x2000u,0x3000u,4u));Check(testSpooler);
    testOwner=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x50000u,0x3000u,4u))+0x438E4u;Check(testOwner>0x438E4u);
    frame=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x5000u,0x3000u,4u))+0x3000u;Check(frame>0x3000u);
    stackTop=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x30000u,0x3000u,4u))+0x28000u;Check(stackTop>0x28000u);
    memcpy(reinterpret_cast<void*>(code+0x2000u),kReplayRetainedCloneNative,sizeof(kReplayRetainedCloneNative));
    memcpy(reinterpret_cast<void*>(code+0x2400u),kReplayNativeRefsNative,sizeof(kReplayNativeRefsNative));
    SetupSlices();NativeCollectionCheck();
    patchPage=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x4000u,0x3000u,4u));Check(patchPage);
    for(U32 i=0;i<7u;++i)memcpy(reinterpret_cast<void*>(Address(kReplayPatches[i].at)),kReplayPatches[i].native,kReplayPatches[i].nativeLength);
    Check(ReplayBytesMatch(false));reinterpret_cast<U8*>(patchPage)[0xC57D-0xB000]^=1u;Check(!ReplayBytesMatch(false));reinterpret_cast<U8*>(patchPage)[0xC57D-0xB000]^=1u;
    ReplayApply(true);Check(ReplayBytesMatch(true));ReplayApply(false);Check(ReplayBytesMatch(false));Say("all seven patch boundaries, signature rejection and exact rollback passed\n");
    // Execute the complete old producer to demonstrate the exact 181->0 defect.
    memcpy(reinterpret_cast<void*>(code+0x1800u),kReplayCaptureNative,sizeof(kReplayCaptureNative));rootPointer=reinterpret_cast<UPtr>(root);Write32(rootPointer+0x214u,testSpooler);
    for(U32 offset=0;offset+4u<sizeof(kReplayCaptureNative);++offset)if(Read32(code+0x1800u+offset)==0xFE7228u)Write32(code+0x1800u+offset,reinterpret_cast<UPtr>(&rootPointer));
    const U32 oldSites[]={0x74B7BCu,0x74B7FBu,0x74B876u,0x74B8B6u};const UPtr oldTargets[]={reinterpret_cast<UPtr>(&Factory),reinterpret_cast<UPtr>(&NativeClone),reinterpret_cast<UPtr>(&EmptyClone),reinterpret_cast<UPtr>(&NativeRefs)};
    for(U32 i=0;i<4u;++i)Call(code+0x1800u+oldSites[i]-0x74B7A0u,oldTargets[i]);
    Source(150);Check(sourceIds[50]==181u);U32 legacy[155]={};entry=code+0x1800u;argPointer=reinterpret_cast<UPtr>(legacy);pushArg=true;Invoke();Abi();Check(legacy[0]==150u && legacy[51]==0u);
    for(U32 i=0;i<150u;++i)Check(legacy[1u+i]==(i==50u?0u:sourceIds[i]));Say("complete native producer reproduces ID181 overwrite\n");
    for(U32 n=0;n<=150u;++n)RoundTrip(n,n%2u?0u:3u);
    for(U32 n=50;n<=256u;n+=1u)if(n==50u || n==51u || n==150u || n==256u)RoundTrip(150,n);
    Say("native save/load instruction slices: counts0..150, retained0/3/50/51/150/256, cached snapshots, guards and ABI passed\n");
    Source(150,2u);Write32(testSpooler+0xDB0u,62u);Save();Load(false);Check(Read32(testOwner+8u)==1u && !requestCount);
    // Execute the native phase tick unchanged except its two service targets.
    memcpy(reinterpret_cast<void*>(code+0x2800u),kReplayTickNative,sizeof(kReplayTickNative));
    Call(code+0x2800u+0x10u,reinterpret_cast<UPtr>(&Ready));Call(code+0x2800u+0x22u,reinterpret_cast<UPtr>(&Ready));
    Jump(code+0x2800u+0x2Eu,code+0x1200u);
    entry=code+0x2800u;pushArg=false;Invoke();Abi();Check(Read32(testOwner+8u)==1u && !requestCount);
    idle=true;Invoke();Abi();Check(Read32(testOwner+8u)==2u && lastExtra==62u && callOrder==4u);
    Invoke();Abi();Check(Read32(testOwner+8u)==0u);
    Say("native phase tick: deferred1, restore2, completed0 and extra state passed\n");
    const U32 slots=static_cast<U32>(g_replayTempsAllocated);for(U32 i=0;i<100u;++i){threadId=1u+i%3u;RoundTrip(150,0u);}Check(static_cast<U32>(g_replayTempsAllocated)==slots);
    Say("temporary storage recycled across threads, stack reuse and repeated transitions\n");
    // Refuse invalid lengths and stale stages before touching native views.
    Source(150);Save();U32 guarded[157]={};guarded[0]=151u;const U32 oldFatals=fatals;
    ReplayLoadTraffic(guarded+1,sourceIds,604u);Check(fatals==oldFatals+1u && !g_replayActive);
    allocationFailure=true;const U32 oldThread=threadId;threadId=99u;
    ReplayTemp* held=ReplayBegin(0x123456u,1u);Check(held);ReplayCapture(reinterpret_cast<void*>(testOwner),nullptr,guarded);Check(fatals==oldFatals+2u);ReplayFinish(held);allocationFailure=false;threadId=oldThread;
    ReplayLock();ReplaySlot* slot=ReplayFindSlot(testOwner,false);slot->guardB^=1u;ReplayUnlock();Write32(testOwner+8u,2u);ReplayCapture(reinterpret_cast<void*>(testOwner),nullptr,guarded);Check(fatals==oldFatals+3u && !g_replayActive);slot->guardB^=1u;
    Say("oversize input, allocation failure and stale cache guard refused\n");
    Check(!g_replayActive);
#if defined(DSF_V29S_REPLAY_PAUSE)
    Check(observedPauseImports==static_cast<U32>(g_replayImports));Say("additive pause import observer matches all imports\n");
#endif
    Say("replay150 checks passed\n");ExitProcess(0);
}

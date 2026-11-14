typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" int _fltused=0;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("pause fixture failure at line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long n,long expected){long old=*p;if(old==expected)*p=n;return old;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static void Write32(UPtr p,U32 n){*reinterpret_cast<volatile U32*>(p)=n;}
static bool Readable(UPtr p,U32){return p>=0x10000u;}
static UPtr testRoot=0,testOwner=0,testSim=0,testDebug=0,testNotify=0,testWorld=0,code=0,testTrampoline=0;
static U32 threadId=7,originalCalls=0,notifyCalls=0,lifecycleCalls=0,eventCalls=0,readyCalls=0,completedEvents=0;
static bool snapshotOkay=true,busy=false,debugFlag=false,importInside=false,replaceInside=false,reenter=false;
static U32 worldResult=2u;static UPtr rootPointer=0;
static UPtr PauseRoot(){return rootPointer;}static UPtr PauseAddress(U32 p){return p;}
static U32 PauseThread(){return threadId;}static bool PauseSnapshotOkay(UPtr p){Check(p==testOwner);return snapshotOkay;}
static void PauseRunOriginal(UPtr);
static void PauseRunCompletion(UPtr);
#include "../src/V29sReplayPauseNative.inc"
#include "../src/V29sReplayPauseCore.inc"
#include "../src/V29sReplayPauseBridge.inc"
#include "../src/V29sReplayPauseLayout.inc"
static void PauseEvent(const char* kind,const PauseView& before,const PauseView* after,U32){
    ++eventCalls;Check(before.paused==1u && !(before.rateBits&0x7FFFFFFFu));
    if(kind[0]=='c'){++completedEvents;Check(after && !after->loading);}
}
static void Call(UPtr at,UPtr target){*reinterpret_cast<U8*>(at)=0xE8u;Write32(at+1u,target-at-5u);}
static void PatchRoot(UPtr at,U32 bytes){for(U32 i=0;i+4u<=bytes;++i)if(Read32(at+i)==0xFE7228u)Write32(at+i,reinterpret_cast<UPtr>(&rootPointer));}
static UPtr __cdecl Factory(){return rootPointer;}
static bool __fastcall ReplayBusy(void* p,void*){Check(reinterpret_cast<UPtr>(p)==testOwner);++readyCalls;return busy;}
static U32 __fastcall WorldStatus(void* p,void*){Check(reinterpret_cast<UPtr>(p)==testWorld);return worldResult;}
static bool __fastcall DebugFlag(void*,void*){return debugFlag;}
static void __fastcall Notify(void* p,void*){
    Check(reinterpret_cast<UPtr>(p)==testNotify);Check(!Read8(testOwner+0x4E9ECu));++notifyCalls;
    if(reenter){reenter=false;PauseUpdate(reinterpret_cast<void*>(testOwner),nullptr);}
}
static void __fastcall Lifecycle(void* p,void*,U32 n){Check(reinterpret_cast<UPtr>(p)==testOwner+0x4EA10u && n==1u);++lifecycleCalls;}
static void __fastcall Original(void* p,void*) {
    Check(reinterpret_cast<UPtr>(p)==testOwner);++originalCalls;
    if(importInside){importInside=false;PauseOnImport(testOwner+0x438E4u);}
    if(replaceInside){replaceInside=false;Write32(testRoot+0x22Cu,testOwner+4u);}
}
static void PauseRunOriginal(UPtr p){typedef void (__thiscall* Fn)(void*);reinterpret_cast<Fn>(testTrampoline)(reinterpret_cast<void*>(p));}
static void PauseRunCompletion(UPtr p){PauseCompleteNative(reinterpret_cast<void*>(p),nullptr);}
static UPtr entry=0,hostEsp=0,stackTop=0,afterEsp=0;static U32 afterRegs[4]={};static unsigned short fpStatus=0;
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov hostEsp,esp
    mov esp,stackTop
    fninit
    mov ebp,77777777h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,testOwner
    call dword ptr [entry]
    mov afterRegs,ebx
    mov afterRegs+4,esi
    mov afterRegs+8,edi
    mov afterRegs+12,ebp
    mov afterEsp,esp
    fnstsw fpStatus
    fninit
    mov esp,hostEsp
    popad
    ret
}}
static void Abi(){Check(afterEsp==stackTop && afterRegs[0]==0x44444444u && afterRegs[1]==0x55555555u && afterRegs[2]==0x66666666u && afterRegs[3]==0x77777777u && !(fpStatus&0x3800u));}
static void Update(){entry=reinterpret_cast<UPtr>(&PauseUpdate);Invoke();Abi();}
static void Setup(bool mark=true) {
    Write32(testRoot+0x22Cu,testOwner);Write32(testRoot+0xCCu,testSim);Write32(testRoot+0x204u,testWorld);
    Write32(testRoot+0x368u,testNotify);Write32(testRoot+0x1Cu,testDebug);
    Write32(testSim,0xE2CA94u);Write32(testSim+0x168u,0xE2CA70u);Write32(testSim+0x188u,1u);Write32(testSim+0x190u,0u);
    Write32(testOwner+0x4E58Cu,testOwner);Write32(testOwner+0x4E588u,2u);Write32(testOwner+0x4E9ECu,1u);
    Write32(testOwner+0x4EA14u,5u);Write32(testOwner+0x4E9F4u,0u);Write32(testOwner+0x438ECu,0u);
    Write32(testOwner+0x4E9F0u,0u);Write32(testOwner+0x4E99Cu,0u);Write32(testOwner+0x4E9A0u,0x3F644445u);
    Write32(testOwner+0x8584u,0u);Write32(testOwner+0x858Cu,0u);
    threadId=7u;snapshotOkay=true;busy=false;debugFlag=false;worldResult=2u;
    importInside=replaceInside=reenter=false;PauseForget(testOwner);if(mark)PauseOnImport(testOwner+0x438E4u);
}
static void KeptPause(){Check(Read8(testSim+0x188u)==1u && Read32(testSim+0x190u)==0u && Read32(testOwner+0x4E99Cu)==0u && Read32(testOwner+0x4E9A0u)==0x3F644445u);}
static void ExpectNoService(UPtr at,U32 value) {
    Setup();Write32(at,value);const long n=g_pauseAttempts;const U32 orig=originalCalls;Update();Check(g_pauseAttempts==n && originalCalls==orig+1u);
}
extern "C" void __stdcall PauseTestMain(){
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x5000u,0x3000u,0x40u));Check(code);
    testRoot=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));rootPointer=testRoot;
    testOwner=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x51000u,0x3000u,4u));testSim=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));
    testWorld=testSim+0x300u;testNotify=testSim+0x400u;testDebug=testSim+0x500u;Write32(testDebug,testSim+0x600u);Write32(testSim+0x610u,reinterpret_cast<UPtr>(&DebugFlag));
    stackTop=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x20000u,0x3000u,4u))+0x18000u;
    memcpy(reinterpret_cast<void*>(code),kPauseUpdateNative,sizeof(kPauseUpdateNative));Check(PauseUpdateMatches(code,reinterpret_cast<UPtr>(&PauseUpdate),false));
    reinterpret_cast<U8*>(code)[100]^=1u;Check(!PauseUpdateMatches(code,reinterpret_cast<UPtr>(&PauseUpdate),false));reinterpret_cast<U8*>(code)[100]^=1u;
    PausePatch(code,reinterpret_cast<UPtr>(&PauseUpdate),true);Check(PauseUpdateMatches(code,reinterpret_cast<UPtr>(&PauseUpdate),true));PausePatch(code,0u,false);Check(PauseUpdateMatches(code,0u,false));
    Say("full update signature, one-site patch and exact rollback passed\n");
    // Real six-byte prologue and production trampoline; external update work is stubbed.
    Call(code+6u,reinterpret_cast<UPtr>(&Original));const U8 epilogue[]={0x5E,0x83,0xC4,0x44,0xC3};memcpy(reinterpret_cast<void*>(code+11u),epilogue,sizeof(epilogue));
    testTrampoline=code+0x1000u;PauseFillTrampoline(testTrampoline,testTrampoline,code+6u);
    memcpy(reinterpret_cast<void*>(code+0x2000u),kPauseCompleteNative,sizeof(kPauseCompleteNative));PatchRoot(code+0x2000u,sizeof(kPauseCompleteNative));
    const U32 completeSites[]={0x7390C9u,0x7390E1u,0x7390F1u,0x7390FFu,0x739121u};
    const UPtr completeTargets[]={code+0x2200u,reinterpret_cast<UPtr>(&Factory),reinterpret_cast<UPtr>(&Notify),reinterpret_cast<UPtr>(&Factory),reinterpret_cast<UPtr>(&Lifecycle)};
    for(U32 i=0;i<5u;++i)Call(code+0x2000u+completeSites[i]-0x7390BEu,completeTargets[i]);g_pauseCompletionTarget=code+0x2000u;
    memcpy(reinterpret_cast<void*>(code+0x2200u),kPauseReadyNative,sizeof(kPauseReadyNative));PatchRoot(code+0x2200u,sizeof(kPauseReadyNative));
    const U32 readySites[]={0x738D35u,0x738D5Fu,0x738D6Fu};const UPtr readyTargets[]={reinterpret_cast<UPtr>(&ReplayBusy),reinterpret_cast<UPtr>(&Factory),reinterpret_cast<UPtr>(&WorldStatus)};
    for(U32 i=0;i<3u;++i)Call(code+0x2200u+readySites[i]-0x738D20u,readyTargets[i]);
    Setup();Check(Read8(testSim+0x188u)==1u);Check(Read8(testOwner+0x4E9ECu)==1u);
    // A paused clock does not dispatch its step. Invoke the new updater instead.
    Update();Check(!Read8(testOwner+0x4E9ECu) && Read32(testOwner+0x4EA14u)==2u && notifyCalls==1u && lifecycleCalls==1u && g_pauseCompleted==1);KeptPause();
    Update();Check(notifyCalls==1u && lifecycleCalls==1u);Say("captured paused/loading state completes once; native notifications, lifecycle, pause and clock preserved\n");
    Setup();busy=true;U32 n=notifyCalls;long held=g_pauseHeld;Update();Check(Read8(testOwner+0x4E9ECu)==1u && notifyCalls==n && g_pauseHeld==held+1);KeptPause();busy=false;Update();Check(!Read8(testOwner+0x4E9ECu) && notifyCalls==n+1u);
    Setup();worldResult=0u;n=notifyCalls;Update();Check(Read8(testOwner+0x4E9ECu)==1u && notifyCalls==n);worldResult=2u;Update();Check(!Read8(testOwner+0x4E9ECu));
    Say("actual native readiness code holds loading for busy subscribers/world and completes on retry\n");
    const U32 offsets[]={0x4E588u,0x4E9ECu,0x4EA14u,0x4E9F4u,0x438ECu,0x438ECu,0x4E9F0u,0x4E99Cu,0x4E99Cu};
    const U32 values[]={1u,0u,4u,3u,1u,2u,1u,0x3F800000u,0x7FC00000u};
    for(U32 i=0;i<9u;++i)ExpectNoService(testOwner+offsets[i],values[i]);
    ExpectNoService(testSim+0x188u,0u);ExpectNoService(testSim+0x190u,0x3F800000u);ExpectNoService(testSim+0x190u,0x7FC00000u);
    Setup(false);n=notifyCalls;Update();Check(notifyCalls==n);
    Setup();threadId=8u;n=notifyCalls;Update();Check(notifyCalls==n);threadId=7u;Update();Check(notifyCalls==n+1u);
    Setup();snapshotOkay=false;n=notifyCalls;Update();Check(notifyCalls==n);snapshotOkay=true;Update();Check(notifyCalls==n+1u);
    Setup();Write32(testRoot+0xCCu,0u);n=notifyCalls;Update();Check(notifyCalls==n);
    Setup();Write32(testOwner+0x4E58Cu,testOwner+4u);n=notifyCalls;Update();Check(notifyCalls==n);
    Setup();replaceInside=true;n=notifyCalls;Update();Check(notifyCalls==n);
    Say("mode/loading/lifecycle/transition/phase/countdown/rate/import/thread/cache/lifetime guards passed\n");
    Setup(false);importInside=true;n=notifyCalls;Update();Check(notifyCalls==n+1u);
    Setup();debugFlag=true;n=lifecycleCalls;Update();Check(!Read8(testOwner+0x4E9ECu) && Read32(testOwner+0x4EA14u)==5u && lifecycleCalls==n);KeptPause();
    Setup();reenter=true;const long reentrant=g_pauseReentrant;n=notifyCalls;Update();Check(notifyCalls==n+1u && g_pauseReentrant==reentrant+1);
    for(U32 i=0;i<100u;++i){Setup();Update();Check(!Read8(testOwner+0x4E9ECu));KeptPause();}
    Check(!g_pauseBusy && !g_pauseMarkLock && completedEvents==static_cast<U32>(g_pauseCompleted));
    Say("import-during-update, native debug branch, reentrancy, repeated loads and x86/x87 ABI passed\n");
    Say("replay pause checks passed\n");ExitProcess(0);
}

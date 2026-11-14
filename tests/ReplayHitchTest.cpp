typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall VirtualProtect(void*,U32,DWORD,DWORD*);
extern "C" __declspec(dllimport) U32 __stdcall VirtualQuery(const void*,void*,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentProcess();
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("hitch fixture failure line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long n,long expected){long old=*p;if(old==expected)*p=n;return old;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static void Write32(UPtr p,U32 n){*reinterpret_cast<volatile U32*>(p)=n;}
struct MemoryBasicInformation32 {void* base;void* allocationBase;U32 allocationProtect,regionSize,state,protect,type;};
static const U32 kCommitted=0x1000u,kPageGuard=0x100u,kPageNoAccess=1u;
bool Readable(UPtr address, U32 bytes) {
    if (!address || bytes > 0x100000u || address + bytes < address) return false;
    MemoryBasicInformation32 m = {};
    if (VirtualQuery(reinterpret_cast<void*>(address), &m, sizeof(m)) < sizeof(m)) return false;
    if (m.state != kCommitted || (m.protect & kPageGuard) ||
        (m.protect & 0xffu) == kPageNoAccess) return false;
    const UPtr end = reinterpret_cast<UPtr>(m.base) + m.regionSize;
    return end >= address && address + bytes <= end;
}
static bool RegMapReadableSpan(UPtr at,U32 n){
    if(!at || n>0x100000u || at+n<at)return false;
    while(n){U32 chunk=0x1000u-(at&0xFFFu);if(chunk>n)chunk=n;if(!Readable(at,chunk))return false;at+=chunk;n-=chunk;}
    return true;
}
static U32 fakeThread=1u,fakeTick=1u;static __int64 fakeQpc=0;
#include "../src/V29sReplayHitchCore.inc"
static RHView fakeView={};
static U32 RHThread(){return fakeThread;}static U32 RHTick(){return fakeTick;}static __int64 RHNow(){return ++fakeQpc;}
static RHView RHReadView(U32,UPtr){return fakeView;}
#include "../src/V29sReplayHitchNative.inc"
#include "../src/V29sReplayHitchBridges.inc"
static UPtr code=0,g_pkg0=0;static long g_pauseStatus=1,g_sRegMapHook=1;
static const U32 kPageExecuteReadWrite=0x40u;
static __int64 g_rhFrequency=0;
static BOOL QueryPerformanceFrequency(__int64* n){*n=10000000;return 1;}
static UPtr Address(U32 at){return code+at-0x737000u;}
static bool VerifyExecutableIdentity(){return true;}
static char lastLog[650]={};
static void CapacityWrite(const char* s){U32 i=0;for(;s[i] && i<649u;++i)lastLog[i]=s[i];lastLog[i]=0;}
static void AppendText(char*& p,const char* s,const char* end){while(*s && p<end)*p++=*s++;}
static void AppendNumber(char*& p,long value,const char* end){if(value<0){if(p<end)*p++='-';value=-value;}char s[12]={};U32 n=0,v=static_cast<U32>(value);do{s[n++]=static_cast<char>('0'+v%10u);v/=10u;}while(v);while(n && p<end)*p++=s[--n];}
static void DiagFAppendKeyU(char*& p,const char* key,U32 v,const char* end){AppendText(p,key,end);AppendNumber(p,static_cast<long>(v),end);}
static void PauseFatal(const char* why){Say(why);ExitProcess(2);}
static bool failPage=false,failCommitFlush=false,corruptAtRecheck=false;
static U32 pageUnlocks=0;
static BOOL Protect(void* p,U32 n,DWORD mode,DWORD* old) {
    if(mode==0x40u && reinterpret_cast<UPtr>(p)>=code && reinterpret_cast<UPtr>(p)<code+0x10000u) {
        if(failPage && ++pageUnlocks==3u){failPage=false;return 0;}
        if(corruptAtRecheck && reinterpret_cast<UPtr>(p)==Address(0x746000u)) {corruptAtRecheck=false;Check(VirtualProtect(p,n,mode,old));*reinterpret_cast<U8*>(Address(0x746000u))^=1u;return 1;}
    }
    return VirtualProtect(p,n,mode,old);
}
static BOOL Flush(void* p,const void* at,U32 n){if(failCommitFlush && reinterpret_cast<UPtr>(at)==Address(0x738000u)){failCommitFlush=false;return 0;}return FlushInstructionCache(p,at,n);}
#define VirtualProtect Protect
#define FlushInstructionCache Flush
#include "../src/V29sReplayHitchLayout.inc"
#undef VirtualProtect
#undef FlushInstructionCache
static U32 PageMode(U32 at){MemoryBasicInformation32 m={};Check(VirtualQuery(reinterpret_cast<void*>(Address(at)),&m,sizeof(m))==sizeof(m));return m.protect;}
static void Stock() {
    DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x10000u,0x40u,&old));
    for(U32 i=0;i<5u;++i)memcpy(reinterpret_cast<void*>(Address(kRHPatches[i].at)),kRHPatches[i].bytes,kRHPatches[i].size);
    Check(VirtualProtect(reinterpret_cast<void*>(code),0x10000u,0x20u,&old));g_rhStatus=0;
}
static void Clear(){memset(g_rhSlots,0,sizeof(g_rhSlots));g_rhHead=g_rhTail=0;g_rhLock=g_rhDropped=g_rhThreadDrops=g_rhDepthDrops=g_rhMismatches=0;fakeTick=1u;fakeThread=1u;fakeView={};fakeView.valid=1u;fakeView.owner=0x10000u;fakeView.recording=0x20000u;fakeView.mode=2u;fakeView.requested=fakeView.applied=0x3F800000u;g_rhStatus=1;}
static U32 Count(U32 kind){RHRecord r={};U32 n=0;while(RHPop(r))if(r.kind==kind)++n;return n;}
static UPtr entry=0,hostEsp=0,stackTop=0,afterEsp=0;static U32 argCount=0,nativeCalls=0,seen0=0,seen1=0;
__declspec(align(16)) static U8 fpAfter[512]={};
static U32 regsAfter[8]={},flagsAfter=0;
extern "C" __declspec(naked) void Native0(){__asm {
    inc nativeCalls
    mov eax,12345601h
    mov ecx,34567890h
    mov edx,45678901h
    pcmpeqd xmm0,xmm0
    std
    stc
    ret
}}
extern "C" __declspec(naked) void Native1(){__asm {
    mov eax,[esp+4]
    mov seen0,eax
    call Native0
    ret 4
}}
extern "C" __declspec(naked) void Native2(){__asm {
    mov eax,[esp+4]
    mov seen0,eax
    mov eax,[esp+8]
    mov seen1,eax
    call Native0
    ret 8
}}
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov hostEsp,esp
    mov esp,stackTop
    fninit
    fld1
    fldpi
    mov eax,argCount
    cmp eax,2
    jne one
    push 22222222h
one:
    cmp eax,0
    je invoke
    push 11111111h
invoke:
    mov eax,10101010h
    mov ecx,20202020h
    mov edx,30303030h
    mov ebx,40404040h
    mov ebp,50505050h
    mov esi,60606060h
    mov edi,70707070h
    std
    stc
    call dword ptr [entry]
    mov afterEsp,esp
    pushfd
    pop flagsAfter
    pushad
    mov esi,esp
    mov edi,offset regsAfter
    cld
    mov ecx,8
    rep movsd
    popad
    fxsave fpAfter
    fninit
    mov esp,hostEsp
    popad
    ret
}}
static U32 expectedRegs[8]={},expectedFlags=0;static U8 expectedFp[512]={};
static void Abi(U32 index,U32 args,UPtr original,UPtr bridge) {
    argCount=args;entry=original;nativeCalls=0;Invoke();Check(afterEsp==stackTop && nativeCalls==1u);
    memcpy(expectedRegs,regsAfter,32u);expectedFlags=flagsAfter;memcpy(expectedFp,fpAfter,512u);
    g_rhTargets[index]=original;entry=bridge;nativeCalls=0;Invoke();Check(afterEsp==stackTop && nativeCalls==1u);
    for(U32 i=0;i<8u;++i)if(i!=3u)Check(expectedRegs[i]==regsAfter[i]);
    Check((expectedFlags&0xCD5u)==(flagsAfter&0xCD5u));
    for(U32 i=0;i<512u;++i)if(i<8u || (i>=24u && i<288u))Check(expectedFp[i]==fpAfter[i]);
    if(args)Check(seen0==0x11111111u);if(args==2u)Check(seen1==0x22222222u);
    Check(g_rhSlots[0].depth==0u && g_rhSlots[0].overflow==0u && !g_rhMismatches);
}
extern "C" void __stdcall HitchTestMain() {
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,0x40u));Check(code);
    stackTop=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,4u))+0xC000u;Check(stackTop>0xC000u);
    Stock();Check(RHMatches(false));failPage=true;pageUnlocks=0;RHInstall();Check(g_rhStatus==-1 && RHMatches(false));
    Check(PageMode(0x737000u)==0x20u && PageMode(0x738000u)==0x20u);
    failCommitFlush=true;RHInstall();Check(g_rhStatus==-1 && RHMatches(false));
    corruptAtRecheck=true;RHInstall();Check(g_rhStatus==-1);Stock();RHInstall();Check(g_rhStatus==1 && RHMatches(true));
    for(U32 i=0;i<5u;++i){Check(PageMode(kRHPatches[i].at)==0x20u);for(U32 j=0;j<kRHPatches[i].len;++j)Check(Read8(g_rhTargets[i]+j)==kRHPatches[i].bytes[j]);}
    Say("real RX installation, adjacent RX/RWX signature recheck and rollback checks passed\n");
    Clear();Abi(0u,0u,reinterpret_cast<UPtr>(&Native0),reinterpret_cast<UPtr>(&RHBridge20));
    Abi(1u,0u,reinterpret_cast<UPtr>(&Native0),reinterpret_cast<UPtr>(&RHBridge24));
    Abi(2u,0u,reinterpret_cast<UPtr>(&Native0),reinterpret_cast<UPtr>(&RHBridgeStep));
    Abi(3u,2u,reinterpret_cast<UPtr>(&Native2),reinterpret_cast<UPtr>(&RHBridgeSeek));
    Abi(4u,1u,reinterpret_cast<UPtr>(&Native1),reinterpret_cast<UPtr>(&RHBridgeStream));
    Say("all five bridges preserve native calls, stack arguments, GPRs, flags/DF, x87 and SSE state\n");
    Clear();RHBegin(RHUpdate,1u,0u);RHFinish(RHUpdate,0u);fakeTick=70u;RHBegin(RHUpdate,1u,0u);RHFinish(RHUpdate,0u);Check(Count(3u)==1u);
    ++fakeView.clock;fakeTick=85u;RHBegin(RHUpdate,1u,0u);RHFinish(RHUpdate,0u);Check(Count(4u)==1u);
    RHBegin(RHGate24,1u,0u);RHFinish(RHGate24,1u);Check(g_rhSlots[0].stats.subscriber==1u);
    fakeView.phase=2u;RHBegin(RHGate24,1u,0u);RHFinish(RHGate24,1u);Check(g_rhSlots[0].stats.snapshot==1u);
    fakeView.phase=0u;fakeView.traffic=3u;RHBegin(RHGate20,1u,0u);RHFinish(RHGate20,1u);Check(g_rhSlots[0].stats.traffic==1u);
    g_rhWaitMinTicks=1;fakeQpc+=100000;RHBegin(RHGate20,1u,0u);RHFinish(RHGate20,0u);Check(Count(5u)==1u);
    fakeView.valid=0u;RHBegin(RHUpdate,1u,0u);RHFinish(RHUpdate,0u);Check(!g_rhSlots[0].tracking);
    fakeTick+=60000u;fakeView.valid=1u;RHBegin(RHUpdate,1u,0u);RHFinish(RHUpdate,0u);
    Check(!g_rhSlots[0].stats.maxStepGap && !g_rhSlots[0].stats.maxUpdateGap && g_rhSlots[0].windowTick==fakeTick);
    Say("clock hold/recovery, independent busy predicates and replay exit tracking checks passed\n");
    Clear();for(U32 i=0;i<30u;++i)RHBegin(RHStep,1u,0u);for(U32 i=0;i<30u;++i)RHFinish(RHStep,0u);
    Check(g_rhDepthDrops==6 && !g_rhSlots[0].depth && !g_rhSlots[0].overflow);
    for(U32 i=2u;i<=17u;++i){fakeThread=i;RHBegin(RHStep,1u,0u);RHFinish(RHStep,0u);}Check(g_rhThreadDrops==2);
    RHRecord r={};for(U32 i=0;i<520u;++i)RHQueue(r);Check(g_rhDropped==8 && Count(0u)==512u);
    g_rhLock=1;RHQueue(r);g_rhLock=0;Check(g_rhDropped==9);
    Check(!g_rhMismatches);Say("bounded depth/thread/ring pressure drops telemetry without blocking native work\n");
    Say("replay hitch checks passed\n");ExitProcess(0);
}

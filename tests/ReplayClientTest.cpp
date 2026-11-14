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
static void CheckAt(bool v,U32 line){if(v)return;Say("client fixture failure line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
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
static long InterlockedDecrement(volatile long* p){return --*p;}
static UPtr code=0,data=0,owner=0,recording=0,nodes=0,ring=0,root=0,rootPointer=0;
static U32 fakeThread=1u,fakeTick=1u,nowCalls=0,clockCalls=0,clockBits=0x40000000u;
static __int64 fakeQpc=0;
#include "../src/V29sReplayClientCore.inc"
static UPtr SCAddress(U32 at){return at>=0xD00000u?data+at-0xE29000u:code+at-0x737000u;}
static U32 SCThreadId(){return fakeThread;}static U32 SCTick(){return fakeTick;}static __int64 SCNow(){++nowCalls;return ++fakeQpc;}
#include "../src/V29sReplayClientData.inc"
#include "../src/V29sReplayClientNative.inc"
#include "../src/V29sReplayClientBridges.inc"
static UPtr g_pkg0=0;static long g_pauseStatus=1,g_sRegMapHook=1;static __int64 g_scFrequency=0;
static const U32 kPageExecuteReadWrite=0x40u;
static bool VerifyExecutableIdentity(){return true;}static BOOL QueryPerformanceFrequency(__int64* n){*n=10000000;return 1;}
static bool RegMapEqual(UPtr at,const U8* b,U32 n){if(!RegMapReadableSpan(at,n))return false;for(U32 i=0;i<n;++i)if(Read8(at+i)!=b[i])return false;return true;}
static char lastLog[800]={};
static void CapacityWrite(const char* s){U32 i=0;for(;s[i] && i<799u;++i)lastLog[i]=s[i];lastLog[i]=0;}
static void AppendText(char*& p,const char* s,const char* end){while(*s && p<end)*p++=*s++;}
static void AppendNumber(char*& p,long value,const char* end){if(value<0){if(p<end)*p++='-';value=-value;}char s[12]={};U32 n=0,v=static_cast<U32>(value);do{s[n++]=static_cast<char>('0'+v%10u);v/=10u;}while(v);while(n && p<end)*p++=s[--n];}
static void DiagFAppendKeyU(char*& p,const char* key,U32 v,const char* end){AppendText(p,key,end);AppendNumber(p,static_cast<long>(v),end);}
static void DiagFAppendKeyHex(char*& p,const char* key,U32 v,const char* end){AppendText(p,key,end);const char* d="0123456789ABCDEF";for(U32 i=0;i<8u && p<end;++i)*p++=d[(v>>(28u-i*4u))&15u];}
static void PauseFatal(const char* why){Say(why);ExitProcess(2);}
static bool failPage=false,failFlush=false,corruptRecheck=false;static U32 unlocked=0;
static BOOL Protect(void* p,U32 n,DWORD mode,DWORD* old) {
    if(mode==0x40u && reinterpret_cast<UPtr>(p)>=code && reinterpret_cast<UPtr>(p)<code+0x20000u) {
        if(failPage && ++unlocked==3u){failPage=false;return 0;}
        if(corruptRecheck && reinterpret_cast<UPtr>(p)==SCAddress(0x752000u)){corruptRecheck=false;Check(VirtualProtect(p,n,mode,old));*reinterpret_cast<U8*>(SCAddress(0x7523B0u))^=1u;return 1;}
    }return VirtualProtect(p,n,mode,old);
}
static BOOL Flush(void* p,const void* at,U32 n){if(failFlush && reinterpret_cast<UPtr>(at)==SCAddress(0x754000u)){failFlush=false;return 0;}return FlushInstructionCache(p,at,n);}
#define VirtualProtect Protect
#define FlushInstructionCache Flush
#include "../src/V29sReplayClientLayout.inc"
#undef VirtualProtect
#undef FlushInstructionCache
static void Stock() {
    DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x20000u,0x40u,&old));
    for(U32 i=0;i<3u;++i)memcpy(reinterpret_cast<void*>(SCAddress(kSCPatches[i].at)),kSCPatches[i].bytes,kSCPatches[i].size);
    memcpy(reinterpret_cast<void*>(SCAddress(0x7379D0u)),kSCGateNative,sizeof(kSCGateNative));
    for(U32 i=0;i<4u;++i)Write32(SCAddress(kSCClients[i].vtable)+0x20u,SCAddress(kSCPatches[kSCClients[i].method].at));
    Check(VirtualProtect(reinterpret_cast<void*>(code),0x20000u,0x20u,&old));g_scStatus=0;
}
static U32 PageMode(U32 at){MemoryBasicInformation32 m={};Check(VirtualQuery(reinterpret_cast<void*>(SCAddress(at)),&m,sizeof(m))==sizeof(m));return m.protect;}
static void Reset() {
    memset(g_scThreads,0,sizeof(g_scThreads));g_scHead=g_scTail=0;
    g_scLock=g_scOpenCount=g_scSequence=g_scFastClear=g_scObserved=g_scBusy=g_scBegin=g_scEnd=g_scAbandoned=g_scHeld=0;
    g_scDropped=g_scThreadDrops=g_scUnknown=g_scReadFailures=g_scReadChecks=g_scClockFailures=0;memset((void*)g_scClientBusy,0,sizeof(g_scClientBusy));
    fakeTick=1u;fakeThread=1u;nowCalls=0;fakeQpc=0;g_scStatus=1;
    Write32(owner+0x4E588u,2u);Write32(owner+0x4E58Cu,recording);Write32(owner+0x4E9A0u,clockBits);Write32(owner+0x4E99Cu,0x3F800000u);
    for(U32 i=0;i<4u;++i){Write32(owner+kSCClients[i].offset,SCAddress(kSCClients[i].vtable));Write32(owner+kSCClients[i].offset+4u,0u);}
    Write32(owner+0x4E3A8u+0x34u,0u);
    Write32(recording,SCAddress(0xE2A088u));Write32(recording+4u,0u);Write32(recording+0x855Cu,0x40000u);Write32(recording+0x8560u,ring);
    Write32(recording+0x8564u,ring+0x10000u);Write32(recording+0x8568u,ring+0x20000u);Write32(recording+0x856Cu,ring+0x18000u);
    Write32(nodes,3u);Write32(nodes+4u,0x3F800000u);Write32(nodes+8u,24u);Write32(nodes+12u,0u);
    Write32(nodes+0x44u,0x40400000u);Write32(nodes+0x48u,24u);Write32(nodes+0x4Cu,0u);
    Write32(nodes+0x84u,0x40800000u);Write32(nodes+0x88u,24u);Write32(nodes+0x8Cu,0u);
}
static UPtr callTarget=0,callSite=0,hostEsp=0,stackTop=0,afterEsp=0,self=0;static U32 regsAfter[8]={},flagsAfter=0;
__declspec(align(16)) static U8 fpAfter[512]={};static U32 expectedRegs[8]={},expectedFlags=0;static U8 expectedFp[512]={};
extern "C" __declspec(naked) void Clock(){__asm { inc clockCalls } __asm { fld dword ptr [clockBits] } __asm { ret } }
static void CallSite(UPtr target,bool correct=true) {
    const UPtr at=SCAddress(correct?0x7379ECu:0x737A20u);DWORD old=0,unused=0;Check(VirtualProtect(reinterpret_cast<void*>(at),6u,0x40u,&old));
    *reinterpret_cast<U8*>(at)=0xE8u;Write32(at+1u,target-at-5u);*reinterpret_cast<U8*>(at+5u)=0xC3u;
    Check(FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),6u));Check(VirtualProtect(reinterpret_cast<void*>(at),6u,old,&unused));callSite=at;
}
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov hostEsp,esp
    mov esp,stackTop
    fninit
    fld1
    fldpi
    mov eax,10101010h
    mov ecx,self
    mov edx,30303030h
    mov ebx,40404040h
    mov ebp,50505050h
    mov esi,60606060h
    mov edi,owner
    std
    stc
    call dword ptr [callSite]
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
static UPtr Bridge(U32 method){return method==0u?reinterpret_cast<UPtr>(&SCBridgeSamples):(method==1u?reinterpret_cast<UPtr>(&SCBridgeVehicle):reinterpret_cast<UPtr>(&SCBridgePlayer));}
static void Abi(U32 index,bool expectedBusy) {
    self=owner+kSCClients[index].offset;const U32 method=kSCClients[index].method;
    CallSite(g_scTargets[method]);Invoke();Check(afterEsp==stackTop && (regsAfter[7]&255u)==static_cast<U32>(expectedBusy));
    memcpy(expectedRegs,regsAfter,32u);expectedFlags=flagsAfter;memcpy(expectedFp,fpAfter,512u);
    CallSite(Bridge(method));Invoke();Check(afterEsp==stackTop);
    for(U32 i=0;i<8u;++i)if(i!=3u)Check(expectedRegs[i]==regsAfter[i]);
    Check((expectedFlags&0xCD5u)==(flagsAfter&0xCD5u));
    for(U32 i=0;i<512u;++i)if(i<8u || (i>=24u && i<288u))Check(expectedFp[i]==fpAfter[i]);
    Check(regsAfter[0]==owner);
}
static U32 Drain(U32 kind){SCRecord r={};U32 n=0;while(SCPop(r))if(r.kind==kind)++n;return n;}
extern "C" void __stdcall ClientTestMain() {
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x20000u,0x3000u,0x40u));data=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x4000u,0x3000u,4u));
    owner=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x54000u,0x3000u,4u));recording=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x9000u,0x3000u,4u));
    nodes=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));ring=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x40000u,0x3000u,4u));
    root=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));rootPointer=root;Write32(root+0x22Cu,owner);
    stackTop=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,4u))+0xC000u;Check(code && data && owner && recording && nodes && ring && root && stackTop>0xC000u);
    Stock();Check(SCMatches(false) && SCDependencies());failPage=true;unlocked=0;SCInstall();Check(g_scStatus==-1 && SCMatches(false));
    Check(PageMode(0x754000u)==0x20u && PageMode(0x755000u)==0x20u);
    failFlush=true;SCInstall();Check(g_scStatus==-1 && SCMatches(false));corruptRecheck=true;SCInstall();Check(g_scStatus==-1);
    Stock();SCInstall();Check(g_scStatus==1 && SCMatches(true));for(U32 i=0;i<3u;++i)Check(PageMode(kSCPatches[i].at)==0x20u);
    Say("real RX installation, exact native/vtable/gate dependencies, failure rollback and protection checks passed\n");
    // Relocate only external root/time services in the real copied vehicle predicate.
    DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(SCAddress(0x755000u)),0x1000u,0x40u,&old));
    const UPtr vehicle=SCAddress(0x755DF0u);for(U32 i=7u;i+4u<=sizeof(kSCVehicleNative);++i)if(Read32(vehicle+i)==0xFE7228u)Write32(vehicle+i,reinterpret_cast<UPtr>(&rootPointer));
    const UPtr c=SCAddress(0x755E2Cu);Check(Read8(c)==0xE8u);Write32(c+1u,reinterpret_cast<UPtr>(&Clock)-c-5u);
    DWORD unused=0;Check(FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(vehicle),sizeof(kSCVehicleNative)));Check(VirtualProtect(reinterpret_cast<void*>(SCAddress(0x755000u)),0x1000u,old,&unused));
    for(U32 i=0;i<4u;++i){Reset();Abi(i,true);Check(g_scBegin==1 && !g_scUnknown);}
    for(U32 i=0;i<4u;++i){Reset();self=owner+kSCClients[i].offset;Write32(self+4u,nodes);Write32(nodes+12u,nodes+0x40u);Write32(nodes+0x4Cu,nodes+0x80u);Abi(i,false);Check(g_scFastClear==1 && !nowCalls && !g_scReadChecks && !g_scObserved);}
    Reset();Write32(owner+0x4E3A8u+4u,nodes);Write32(nodes+12u,nodes+0x40u);Write32(nodes+0x44u,0x3F800000u);clockCalls=0;Abi(0u,true);Check(clockCalls==2u);
    Reset();Write32(owner+0x4E3A8u+4u,nodes);Write32(nodes+12u,nodes+0x40u);Write32(nodes+0x4Cu,nodes+0x80u);Write32(owner+0x4E3A8u+0x34u,1u);Abi(0u,true);
    Reset();self=owner+0x4E388u;CallSite(Bridge(0u),false);Invoke();Check((regsAfter[7]&255u)==1u && !g_scObserved && !g_scReadChecks && !nowCalls);
    Say("real predicate branches preserve native output GPRs/ECX scratch, EDI identity, flags/DF, x87/SSE and stack; normal clear has no QPC/readability/C++ observer\n");
    Reset();U32 frame[11]={};frame[0]=owner;frame[9]=owner+0x4E388u;frame[7]=1u;frame[10]=SCAddress(0x7379F1u);SCObserve(frame,0u);Check(g_scOpenCount==1 && Drain(0u)==1u);
    Write32(frame[9]+4u,nodes);Write32(nodes+12u,nodes+0x40u);fakeTick=30u;frame[7]=0u;SCObserve(frame,0u);Check(!g_scOpenCount && Drain(1u)==1u);
    SCView v=SCCapture(owner,frame[9],2u);Check(v.ringValid && v.freeBytes==0x30000u && v.usedBytes==0x10000u && v.unparsedBytes==0x8000u);
    Write32(recording+0x8564u,ring+0x30000u);Write32(recording+0x8568u,ring+0x10000u);Write32(recording+0x856Cu,ring+0x38000u);v=SCCapture(owner,frame[9],2u);Check(v.freeBytes==0x20000u && v.unparsedBytes==0x18000u);
    Write32(recording+0x8564u,ring+0x10000u);v=SCCapture(owner,frame[9],2u);Check(v.freeBytes==v.capacity && !v.usedBytes);
    Write32(recording+0x855Cu,0xFFFFFFFFu);v=SCCapture(owner,frame[9],2u);Check(!v.ringValid);
    Write32(frame[9]+4u,16u);v=SCCapture(owner,frame[9],2u);Check(!v.nodes[0].valid && g_scReadFailures==1);
    Say("client identity, busy/clear data, chained samples and wrapped/empty/invalid ring calculations passed\n");
    Reset();frame[7]=1u;SCObserve(frame,0u);fakeTick=1200u;SCObserve(frame,0u);Check(g_scHeld==1);
    Write32(owner+0x4E9A0u,0x3F000000u);SCObserve(frame,0u);Check(g_scAbandoned==1 && g_scBegin==2 && g_scOpenCount==1);
    Reset();for(U32 i=1u;i<=9u;++i){fakeThread=i;SCObserve(frame,0u);}Check(g_scThreadDrops==1);
    Reset();SCRecord r={};for(U32 i=0;i<520u;++i)SCQueue(r);Check(g_scDropped==8 && Drain(0u)==512u);g_scLock=1;SCQueue(r);g_scLock=0;Check(g_scDropped==9);
    Say("held/context changes and bounded thread/queue contention drop telemetry without blocking game work\n");
    Say("replay client checks passed\n");ExitProcess(0);
}

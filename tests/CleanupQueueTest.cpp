// Executes selected native instructions in isolated fake storage. Never loads Driver.exe/ASI.
typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
static void* Allocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x40u);}
extern "C" __declspec(noinline) void* __cdecl memset(void* out,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(out)[i]=static_cast<U8>(v);return out;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* out,const void* in,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(out)[i]=reinterpret_cast<const U8*>(in)[i];return out;}
static U32 checks=0;static void Check(bool v){++checks;if(!v)ExitProcess(checks);}
static UPtr codePage=0,owner=0,clockPage=0,iatPage=0,storage=0,tramps=0;
static UPtr Address(UPtr p){if(p>=0xBAE000u && p<0xBAF000u)return codePage+p-0xBAE000u;
    if(p>=0x16F4000u && p<0x16F5000u)return clockPage+p-0x16F4000u;
    if(p>=0xDC9000u && p<0xDCA000u)return iatPage+p-0xDC9000u;
    if(p>=0x1732000u && p<0x19C0000u)return owner+p-0x1732BF0u;
    ExitProcess(240u);return 0;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static void CapacityMax(volatile long& p,U32 v){if(v>static_cast<U32>(p))p=static_cast<long>(v);}
static bool cold=true,identity=true,parent=true,readable=true;
static long g_audioModelStatus=1,g_indexedStatus=1;static UPtr g_indexedBase=1;
static bool Readable(UPtr,U32){return readable;}static bool IndexedCold(){return cold;}
static bool VerifyExecutableIdentity(){return identity;}static bool AudioModelSignature(bool){return parent;}
static bool IndexedInstructionsMatch(bool,UPtr){return parent;}static bool IndexedPendingRedirectIntact(){return parent;}
static const U32 kMemCommitReserve=0x3000u,kPageExecuteReadWrite=0x40u;
#include "../src/V29sCleanupQueueCore.inc"
static U32 events=0;static void CleanupEvent(const char*,U32,U32,U32){++events;}
static bool nativeMode=true;static U32 helperCalls=0,helperKind=0,helperDf=0,captured[12]={};
extern "C" __declspec(noinline) void __cdecl CleanupEntry(U32 kind,U32* frame){
    if(nativeMode){Check(CleanupObserve(kind,frame));return;}
    ++helperCalls;helperKind=kind;for(U32 i=0;i<12u;++i)captured[i]=frame[i];
    U32 flags=0;__asm {pushfd
        pop flags
        fninit
        fldz
        pxor xmm0,xmm0
        pxor xmm7,xmm7}helperDf=flags&0x400u;
}
extern "C" UPtr g_cleanupTramps[6];
#include "../src/V29sCleanupQueueBridges.inc"
static U32 allocations=0,protects=0,flushes=0,failAllocation=0,failProtect=0,failFlush=0;
static bool changeCold=false;
static void* MockAlloc(void*,U32 bytes,U32,U32){++allocations;if(allocations==failAllocation)return nullptr;
    return reinterpret_cast<void*>(bytes>4096u?storage:tramps+allocations*64u);}
static BOOL MockProtect(void*,U32,U32,DWORD* old){++protects;*old=0x20u;if(changeCold && protects==1)cold=false;return protects!=failProtect;}
static BOOL MockFlush(void*,const void*,U32){++flushes;return flushes!=failFlush;}static void* GetCurrentProcess(){return nullptr;}
#define VirtualAlloc MockAlloc
#define VirtualProtect MockProtect
#define FlushInstructionCache MockFlush
#include "../src/V29sCleanupQueuePatch.inc"
#undef FlushInstructionCache
#undef VirtualProtect
#undef VirtualAlloc
#include "CleanupNativeBytes.inc"
static U32 record[3]={},unlinks=0,destroyed=0,freed=0;
static UPtr expectedWrapper=0,expectedObject=0;
struct Resource {U32 vtable,unused,refs,dead,keylo,keyhi,a,b;};
static Resource objects[262144]={};static U32 wrappers[262144]={};static U32 vtable[1]={};
extern "C" U32 __fastcall Destroy(Resource* r,void*,U32 flag){Check(!flag && !r->dead);if(expectedObject)Check(reinterpret_cast<UPtr>(r)==expectedObject);
    r->dead=1;++destroyed;return 0;}
extern "C" void __cdecl FreeWrapper(U32 p){Check(Read32(p+0u)!=0u);Resource* r=reinterpret_cast<Resource*>(Read32(p));Check(r->dead==1);
    if(expectedWrapper)Check(p==expectedWrapper);++freed;}
extern "C" __declspec(naked) void NativeFree(){__asm {push dword ptr [esp+4]
    call FreeWrapper
    add esp,4
    ret 4}}
extern "C" __declspec(naked) void NativeFind(){__asm {mov eax,offset record
    ret 8}}
extern "C" __declspec(naked) void NativeUnlink(){__asm {inc unlinks
    ret 8}}
static long __stdcall Decrement(volatile long* p){return --*p;}
static long __stdcall Exchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static void Jump(UPtr p,UPtr to){*reinterpret_cast<U8*>(p)=0xE9u;Write32(p+1u,to-p-5u);}
static U8 snapshot[4096]={};
static void Reset(){
    memset(reinterpret_cast<void*>(codePage),0xCC,4096u);memset(reinterpret_cast<void*>(owner-128u),0,0x280200u);
    for(const NativeBlock& b:nativeBlocks)memcpy(reinterpret_cast<void*>(Address(b.at)),b.bytes,b.length);
    Jump(Address(0xBAEE00u),reinterpret_cast<UPtr>(&NativeFind));Jump(Address(0xBAEEA0u),reinterpret_cast<UPtr>(&NativeUnlink));
    allocations=protects=flushes=failAllocation=failProtect=failFlush=0;changeCold=false;cold=identity=parent=readable=true;g_audioModelStatus=g_indexedStatus=1;
    g_cleanupAllocation=g_cleanupBase=0;g_cleanupInstalled=0;g_cleanupStatus=0;memset(g_cleanupTramps,0,sizeof(g_cleanupTramps));
    memset(const_cast<long*>(g_cleanupAttempts),0,sizeof(g_cleanupAttempts));g_cleanupPeak=g_cleanupCurrent=g_cleanupBeyondStock=g_cleanupOldCrossings=0;
    g_cleanupFull=g_cleanupGuardBad=g_cleanupInvalid=g_cleanupDrainStarts=g_cleanupDrainEnds=g_cleanupNonempty=g_cleanupDrained=0;
    g_cleanupDepth=g_cleanupDuringDrain=g_cleanupDrainMismatch=0;g_cleanupExpected=g_cleanupStartAttempts=events=0;
    expectedWrapper=expectedObject=0;destroyed=freed=unlinks=0;nativeMode=true;memset(record,0,sizeof(record));
    memcpy(snapshot,reinterpret_cast<void*>(codePage),4096u);
}
static void Unchanged(){for(U32 i=0;i<4096u;++i)Check(Read8(codePage+i)==snapshot[i]);}
static void InstallerTests(){
    Reset();cold=false;Check(!CleanupInstall());Unchanged();Reset();identity=false;Check(!CleanupInstall());Unchanged();
    Reset();parent=false;Check(!CleanupInstall());Unchanged();Reset();g_audioModelStatus=-1;Check(!CleanupInstall());Unchanged();
    Reset();g_indexedStatus=2;Check(!CleanupInstall());Unchanged();Reset();Write32(owner+0x17E090u,1u);Check(!CleanupInstall());Unchanged();
    for(U32 i=1;i<=7u;++i){Reset();failAllocation=i;Check(!CleanupInstall());Unchanged();}
    Reset();failProtect=1;Check(!CleanupInstall());Unchanged();Reset();changeCold=true;Check(!CleanupInstall());Unchanged();
    Reset();changeCold=true;failProtect=2;Check(!CleanupInstall() && g_cleanupStatus==-2);Unchanged();
    Reset();failFlush=1;Check(!CleanupInstall() && g_cleanupStatus==-1);Unchanged();
    Reset();failProtect=2;Check(!CleanupInstall() && g_cleanupStatus==-1);Unchanged();
    Reset();failFlush=1;failProtect=3;Check(!CleanupInstall() && g_cleanupStatus==-2);
    Reset();failFlush=1;failProtect=4;Check(!CleanupInstall() && g_cleanupStatus==-2);
    Reset();*reinterpret_cast<U8*>(Address(kCleanupSites[0].address))=0x90u;Check(!CleanupInstall());
    Reset();Check(CleanupInstall() && CleanupSignature(true) && g_cleanupInstalled==6u);
    for(U32 n=0;n<4096u;++n){bool edited=false;for(const CleanupSite& s:kCleanupSites)if(n>=s.address-0xBAE000u && n<s.address-0xBAE000u+s.length)edited=true;
        for(const CleanupOperand& s:kCleanupOperands)if(n>=s.address-0xBAE000u+s.offset && n<s.address-0xBAE000u+6u)edited=true;
        if(!edited)Check(Read8(codePage+n)==snapshot[n]);}
}
static UPtr appendFn=0,drainFn=0,replaceFn=0,releaseFn=0;
extern "C" __declspec(naked) void __cdecl Append(U32){__asm {mov ecx,owner
    push dword ptr [esp+4]
    call dword ptr [appendFn]
    ret}}
extern "C" __declspec(naked) void __cdecl Drain(){__asm {mov ecx,owner
    call dword ptr [drainFn]
    ret}}
extern "C" __declspec(naked) void __cdecl Replace(U32,U32){__asm {mov ecx,owner
    push dword ptr [esp+8]
    push dword ptr [esp+8]
    call dword ptr [replaceFn]
    ret}}
extern "C" __declspec(naked) void __cdecl Release(U32,U32){__asm {mov ecx,owner
    push dword ptr [esp+8]
    push dword ptr [esp+8]
    call dword ptr [releaseFn]
    ret}}
static void ArmNative(){Reset();Check(CleanupInstall());
    for(const NativeFixup& f:nativeFixups){bool hooked=false;for(const CleanupSite& s:kCleanupSites)if(f.at>=s.address && f.at<s.address+s.length)hooked=true;
        if(!hooked)Write32(Address(f.at),Address(f.value));}
    Write32(g_cleanupTramps[3]+2u,Address(0x16F49A4u));
    Write32(Address(0xBAE745u)+1u,reinterpret_cast<UPtr>(&NativeFree)-Address(0xBAE745u)-5u);
    Write32(Address(0xDC915Cu),reinterpret_cast<UPtr>(&Decrement));Write32(Address(0xDC9168u),reinterpret_cast<UPtr>(&Exchange));
    appendFn=Address(0xBAEA60u);drainFn=Address(0xBAE6B0u);replaceFn=Address(0xBAEBE0u);releaseFn=Address(0xBAECA0u);
    vtable[0]=reinterpret_cast<U32>(&Destroy);
}
static void InitObject(U32 i,U32 refs){objects[i]={reinterpret_cast<U32>(vtable),0u,refs,0u,0x100000u+i,0u,0u,0u};wrappers[i]=reinterpret_cast<U32>(&objects[i]);}
static void NativeTests(){
    ArmNative();for(U32 i=0;i<kCleanupCapacity;++i){InitObject(i,0u);Append(reinterpret_cast<U32>(&wrappers[i]));Check(Read32(owner+0x17E090u)==i+1u);Check(Read32(g_cleanupBase+i*4u)==reinterpret_cast<U32>(&wrappers[i]));}
    Check(g_cleanupPeak==262144 && g_cleanupOldCrossings==1 && g_cleanupBeyondStock==229376 && !CleanupGuards());
    Check(Read32(owner+0x15E090u)==0u && Read32(owner+0x17E094u)==0u);
    Drain();Check(destroyed==kCleanupCapacity && freed==kCleanupCapacity && !Read32(owner+0x17E090u));
    Check(g_cleanupDrained==262144 && !g_cleanupDepth && !g_cleanupDrainMismatch && !g_cleanupDuringDrain && !CleanupGuards());
    Drain();Check(g_cleanupDrainStarts==2 && g_cleanupDrainEnds==2 && g_cleanupNonempty==1);
    InitObject(0u,0u);Append(reinterpret_cast<U32>(&wrappers[0]));Drain();Check(destroyed==262145u && freed==262145u);
    ArmNative();InitObject(0u,3u);InitObject(1u,0u);record[0]=reinterpret_cast<U32>(&wrappers[0]);record[1]=123u;
    Replace(123u,reinterpret_cast<U32>(&wrappers[1]));Check(record[0]==reinterpret_cast<U32>(&wrappers[1]) && objects[1].refs==4u);
    Check(g_cleanupAttempts[1]==1 && !objects[0].dead && !objects[1].dead && Read32(owner+0x17E090u)==1u);
    expectedWrapper=reinterpret_cast<U32>(&wrappers[0]);expectedObject=reinterpret_cast<U32>(&objects[0]);Drain();Check(objects[0].dead && !objects[1].dead);
    for(U32 n=0;n<3u;++n)Release(0x100001u,0u);Check(!Read32(owner+0x17E090u) && !objects[1].dead);
    Release(0x100001u,0u);Check(Read32(owner+0x17E090u)==1u && unlinks==1u && !objects[1].dead && g_cleanupAttempts[2]==1);
    expectedWrapper=reinterpret_cast<U32>(&wrappers[1]);expectedObject=reinterpret_cast<U32>(&objects[1]);Drain();Check(objects[1].dead && destroyed==2u && freed==2u);
}
static void BoundsTests(){ArmNative();U32 frame[20]={};frame[6]=owner;frame[7]=kCleanupCapacity;Check(!CleanupObserve(0u,frame) && g_cleanupFull==1 && !CleanupGuards());
    frame[7]=0;Write32(g_cleanupBase-4u,0u);Check(!CleanupObserve(0u,frame) && g_cleanupGuardBad==1);
    ArmNative();memset(frame,0,sizeof(frame));frame[6]=owner+4u;Check(!CleanupObserve(0u,frame) && g_cleanupInvalid==1);
    ArmNative();memset(frame,0,sizeof(frame));frame[6]=owner;Check(CleanupObserve(3u,frame));Check(CleanupObserve(0u,frame) && g_cleanupDuringDrain==1);
    frame[2]=owner;Check(CleanupObserve(5u,frame) && g_cleanupDrainMismatch==1);
    ArmNative();memset(frame,0,sizeof(frame));frame[6]=owner;Check(CleanupObserve(3u,frame));Check(!CleanupObserve(3u,frame) && g_cleanupInvalid==1);
}
static __declspec(align(16)) U8 beforeFx[512]={},afterFx[512]={};static U32 afterRegs[7]={},afterFlags=0,bridgeAddress=0;
extern "C" __declspec(naked) void Capture(){__asm {mov afterRegs[0],eax
    mov afterRegs[4],ecx
    mov afterRegs[8],edx
    mov afterRegs[12],ebx
    mov afterRegs[16],esi
    mov afterRegs[20],edi
    mov afterRegs[24],ebp
    pushfd
    pop afterFlags
    fxsave [afterFx]
    ret}}
extern "C" __declspec(naked) void Invoke(){__asm {pushad
    fninit
    fld1
    pcmpeqd xmm0,xmm0
    pcmpeqd xmm7,xmm7
    mov eax,11111111h
    mov ecx,22222222h
    mov edx,33333333h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ebp,77777777h
    fxsave [beforeFx]
    push 0ABCDEF01h
    push 12345678h
    std
    stc
    call dword ptr [bridgeAddress]
    cld
    add esp,8
    fninit
    popad
    ret}}
static void BridgeTests(){nativeMode=false;for(U32 i=0;i<6u;++i){g_cleanupTramps[i]=reinterpret_cast<UPtr>(&Capture);bridgeAddress=kCleanupSites[i].bridge;Invoke();
    Check(helperCalls==i+1u && helperKind==i && !helperDf && (afterFlags&0x401u)==0x401u);
    Check(captured[10]==0x12345678u && captured[11]==0xABCDEF01u);
    for(U32 n=0;n<7u;++n)Check(afterRegs[n]==0x11111111u*(n+1u));
    for(U32 n=0;n<24u;++n)Check(beforeFx[n]==afterFx[n]);
    for(U32 n=0;n<8u;++n)for(U32 j=0;j<10u;++j)Check(beforeFx[32u+n*16u+j]==afterFx[32u+n*16u+j]);
    for(U32 n=160u;n<288u;++n)Check(beforeFx[n]==afterFx[n]);}}
extern "C" void __stdcall CleanupTestMain(){codePage=reinterpret_cast<UPtr>(Allocate(4096u));owner=reinterpret_cast<UPtr>(Allocate(0x290000u))+128u;
    clockPage=reinterpret_cast<UPtr>(Allocate(4096u));iatPage=reinterpret_cast<UPtr>(Allocate(4096u));storage=reinterpret_cast<UPtr>(Allocate(0x101000u));tramps=reinterpret_cast<UPtr>(Allocate(4096u));
    Check(codePage && owner && clockPage && iatPage && storage && tramps);InstallerTests();NativeTests();BoundsTests();BridgeTests();
    const char message[]="cleanup queue checks passed\n";DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);ExitProcess(0u);}

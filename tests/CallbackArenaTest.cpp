// Exact production core/installer plus selected native instructions in fake memory.
typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) void* __stdcall AddVectoredExceptionHandler(U32,void*);
static void* Allocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x40u);}
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static U32 checks=0;
static void Say(const char* text){U32 n=0;while(text[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),text,n,&written,nullptr);}
static void CheckAt(bool value,U32 line){++checks;if(value)return;char text[64]="fixture failure at source line ";U32 n=31,div=10000;bool begun=false;
    for(;div;div/=10){U32 d=line/div%10;if(d || begun || div==1){text[n++]=static_cast<char>('0'+d);begun=true;}}
    text[n++]='\n';DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),text,n,&written,nullptr);ExitProcess(line);}
#define Check(v) CheckAt((v),__LINE__)
static UPtr pages[4]={},globals=0;static U32 vas[4]={0x556000u,0x858000u,0x859000u,0xDB4000u};
static void Hex(U32 value){char text[10]={};const char* digits="0123456789ABCDEF";for(U32 i=0;i<8;++i)text[i]=digits[(value>>(28-i*4))&15];text[8]='\n';Say(text);}
static long __stdcall FixtureException(UPtr* pointers){U32* context=reinterpret_cast<U32*>(pointers[1]);Say("exception EIP/native equivalent/ECX/EDI/ESP\n");
    Hex(context[46]);for(U32 i=0;i<4;++i)if(context[46]>=pages[i] && context[46]<pages[i]+4096)Hex(vas[i]+context[46]-pages[i]);
    Hex(context[43]);Hex(context[39]);Hex(context[49]);Say("pages\n");for(U32 i=0;i<4;++i)Hex(pages[i]);
    U32* stack=reinterpret_cast<U32*>(context[49]);Say("stack\n");for(U32 i=0;i<6;++i)Hex(stack[i]);ExitProcess(222);return 0;}
static UPtr Address(UPtr p){if(p>=0x1442000u && p<0x1448000u)return globals+p-0x1442000u;
    for(U32 n=0;n<4;++n)if(p>=vas[n] && p<vas[n]+4096u)return pages[n]+p-vas[n];ExitProcess(999);return 0;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool readable=true,cold=true,parent=true,changeCold=false;static U32 tid=1;
static bool Readable(UPtr p,U32){return readable && p!=0u;}static U32 GetCurrentThreadId(){return tid;}
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long v,long expected){long old=*p;if(old==expected)*p=v;return old;}
static const U32 kMemCommitReserve=0x3000u,kPageReadWrite=4u,kPageExecuteReadWrite=0x40u;static UPtr g_pkg0=0;
static long g_pendingStatus=1,g_chapterStatus=1;static bool PendingParent(){return parent;}static bool PendingSignature(bool){return parent;}
static bool ChapterSignature(){return parent;}static bool IndexedCold(){return cold;}
static U32 allocations=0,protects=0,flushes=0,failAllocation=0,failProtect=0,failFlush=0;
static void* MockAlloc(void*,U32 n,U32,U32){++allocations;return allocations==failAllocation?nullptr:Allocate(n);}
static BOOL MockProtect(void*,U32,U32,DWORD* old){++protects;*old=0x20u;if(changeCold && protects==1)cold=false;return protects!=failProtect;}
static BOOL MockFlush(void*,const void*,U32){return ++flushes!=failFlush;}static void* GetCurrentProcess(){return nullptr;}
#define VirtualAlloc MockAlloc
#define VirtualProtect MockProtect
#define FlushInstructionCache MockFlush
#include "../src/V29sCallbackCore.inc"
static U32 fatals=0,events=0,writerChecks=0;static bool writerClobber=false;
static void CallbackFatal(const char*,UPtr){++fatals;}static void CallbackEvent(U32,const char*){++events;}
extern "C" __declspec(noinline) void __cdecl CallbackWriterCheck(UPtr handler){U32 slot=0;Check(CallbackHandlerSlot(handler,slot));++writerChecks;
    if(writerClobber){__asm {fninit
        fldz
        pxor xmm0,xmm0
        pxor xmm7,xmm7}}}
#include "../src/V29sCallbackBridges.inc"
#include "../src/V29sCallbackPatch.inc"
#undef FlushInstructionCache
#undef VirtualProtect
#undef VirtualAlloc
#include "CallbackNativeBytes.inc"
struct Object {UPtr vtable;U32 ready;};static Object objects[1024]={};static UPtr vtable[4]={};
static U8 handlers[21][0x570]={};static UPtr ctor=0,writer=0;static U32 spawn=0,spawnSlot=0;
static U8 __fastcall Done(Object* object,void*) {if(spawn){--spawn;UPtr payload=reinterpret_cast<UPtr>(&objects[900]);
        Check(CallbackInsertBridge(reinterpret_cast<void*>(CallbackHeader(spawnSlot)),nullptr,reinterpret_cast<UPtr>(&payload))!=0u);}return static_cast<U8>(object->ready);}
extern "C" __declspec(naked) void __cdecl Submit(UPtr,UPtr){__asm {push esi
    push edi
    mov esi,[esp+12]
    mov edi,[esp+16]
    call dword ptr [writer]
    pop edi
    pop esi
    ret}}
static void Reset(){
    for(U32 i=0;i<4;++i)memset(reinterpret_cast<void*>(pages[i]),0xCC,4096);
    for(const NativeBlock& b:nativeBlocks)memcpy(reinterpret_cast<void*>(Address(b.at)),b.bytes,b.length);
    memset(reinterpret_cast<void*>(globals),0xA5,0x6000);
    // Rebase four polling globals, registration's base, and the constructor.
    // Relative branches inside each native block remain unchanged.
    const U32 operands[6]={0x859015u,0x85901Fu,0x859025u,0x859054u,0x858FC4u,0xDB40A6u};
    for(U32 at:operands)Write32(Address(at),Address(Read32(Address(at))));
    Write32(Address(0x858FC9u),Address(0x5567D0u)-Address(0x858FCDu));
    *reinterpret_cast<U8*>(Address(0x858FD4u))=0xC3;
    const U8 tail[]={0xB0,0x01,0x5F,0x5E,0x5B,0x59,0xC3};memcpy(reinterpret_cast<void*>(Address(0x859073u)),tail,sizeof(tail));
    ctor=Address(0xDB40A0u);writer=Address(0x858FB5u);
    typedef void (__cdecl* Constructor)();reinterpret_cast<Constructor>(ctor)();
    memset(handlers,0,sizeof(handlers));for(U32 i=0;i<21;++i)Write32(reinterpret_cast<UPtr>(handlers[i])+0x560u,i);
    vtable[3]=reinterpret_cast<UPtr>(&Done);for(U32 i=0;i<1024;++i){objects[i].vtable=reinterpret_cast<UPtr>(vtable);objects[i].ready=1;}
    g_callbackGroups=nullptr;g_callbackStatus=0;g_callbackInvalid=g_callbackFull=g_callbackGuardBad=g_callbackConcurrent=0;
    g_callbackNativeInsert=reinterpret_cast<CallbackNativeInsert>(Address(0x5567D0u));
    g_callbackNativePoll=reinterpret_cast<CallbackNativePoll>(Address(0x859000u));
    fatals=events=allocations=protects=flushes=failAllocation=failProtect=failFlush=writerChecks=spawn=0;
    parent=cold=readable=true;changeCold=writerClobber=false;tid=1;g_pkg0=0;g_pendingStatus=g_chapterStatus=1;
}
static void Arm(){Reset();Check(CallbackInstall());}
static UPtr Insert(U32 slot,U32 index){UPtr payload=reinterpret_cast<UPtr>(&objects[index]);
    return CallbackInsertBridge(reinterpret_cast<void*>(CallbackHeader(slot)),nullptr,reinterpret_cast<UPtr>(&payload));}
static U8 Poll(U32 slot){return CallbackPollBridge(handlers[slot],nullptr);}
static void UnsafeRegression(){Reset();UPtr h=CallbackHeader(9),payload=0x12345678;
    for(U32 i=0;i<65;++i)g_callbackNativeInsert(reinterpret_cast<void*>(h),reinterpret_cast<UPtr>(&payload));
    Check(Read32(h+8)==65 && Read32(CallbackHeader(10))==payload);}
static void NativeTests(){Arm();
    // Install can precede native initialization: no early cursor publication.
    reinterpret_cast<void (__cdecl*)()>(ctor)();
    for(U32 g=0;g<21;++g){for(U32 i=0;i<512;++i)Check(Insert(g,i)!=0u);
        Check(CallbackValidate(g) && g_callbackGroups[g].peak==512 && g_callbackGroups[g].redirects==1);
        Check(!Insert(g,512) && g_callbackFull==static_cast<long>(g+1));}
    Check(fatals==21);fatals=0;g_callbackFull=0;
    Say("filled\n");for(U32 i=0;i<512;++i)objects[i].ready=i&1;
    for(U32 g=0;g<21;++g){Check(Poll(g)==1 && Read32(CallbackHeader(g)+8)==256);
        for(U32 i=0;i<256;++i)Check(Insert(g,600+i)!=0u);
        Check(g_callbackGroups[g].reuses==256 && CallbackValidate(g));}
    Say("partial recycled\n");for(U32 i=0;i<1024;++i)objects[i].ready=1;
    for(U32 g=0;g<21;++g){Check(Poll(g)==1 && !Read32(CallbackHeader(g)+8));
        for(U32 i=0;i<150;++i)Check(Insert(g,i)!=0u);
        Check(Poll(g)==1 && !Read32(CallbackHeader(g)+8) && g_callbackGroups[g].redirects==1 && CallbackValidate(g));}
    // Native poll visits a newly appended callback in the same traversal.
    Say("drained\n");Check(Insert(3,0)!=0u);spawn=1;spawnSlot=3;Check(Poll(3)==1 && !spawn && CallbackValidate(3));
    Check(!Read32(CallbackHeader(3)+8) && !g_callbackConcurrent && !fatals);
    // Exact registration fragment, including production mid-function guard.
    UPtr handler=reinterpret_cast<UPtr>(handlers[9]);Write32(handler+0x55Cu,reinterpret_cast<UPtr>(&objects[0]));
    Say("submit\n");Submit(handler,handler+0x55Cu);Check(writerChecks==1 && Read32(CallbackHeader(9)+8)==1);
    Check(Poll(9)==1 && !fatals);
    // Non-target helper caller keeps the original64-node ABI/geometry.
    UPtr other=reinterpret_cast<UPtr>(Allocate(0x400));Write32(other,other+12);Write32(other+0x30C,other+0x30C);Write32(other+0x310,other+0x30C);
    UPtr payload=0x11223344;Check(CallbackInsertBridge(reinterpret_cast<void*>(other),nullptr,reinterpret_cast<UPtr>(&payload))==other+12);
    Check(Read32(other+20)==payload && Read32(other+8)==1);
}
static void IntegrityTests(){Arm();Check(Insert(9,0)!=0u);g_callbackGroups[9].after[0]=0;Check(!Insert(9,1) && g_callbackGuardBad);
    Arm();Check(Insert(9,0)!=0u);UPtr h=CallbackHeader(9),n=Read32(h+0x310);Write32(n+4,n);Check(!Poll(9) && g_callbackInvalid);
    Arm();Check(Insert(9,0)!=0u);Check(Poll(9)==1);h=CallbackHeader(9);Write32(Read32(h),Read32(h));Check(!Insert(9,1) && g_callbackInvalid);
    Arm();Check(Insert(9,0)!=0u);reinterpret_cast<void (__cdecl*)()>(ctor)();Check(!Poll(9) && g_callbackInvalid);
    Arm();U32 slot=0;Write32(reinterpret_cast<UPtr>(handlers[9])+0x560,21);Check(!CallbackHandlerSlot(reinterpret_cast<UPtr>(handlers[9]),slot));Check(!Poll(9));
    Arm();g_callbackGroups[9].thread=2;Check(!Insert(9,0) && g_callbackConcurrent==1 && !Read32(CallbackHeader(9)+8));
    Arm();g_callbackGroups[9].pollDepth=1;Check(!Poll(9) && g_callbackInvalid);
    Arm();readable=false;Check(!Poll(9));
}
static U8 snapshots[3][4096];
static void Save(){for(U32 i=0;i<3;++i)memcpy(snapshots[i],reinterpret_cast<void*>(pages[i]),4096);}
static void Unchanged(){for(U32 i=0;i<3;++i)for(U32 j=0;j<4096;++j)Check(Read8(pages[i]+j)==snapshots[i][j]);}
static void InstallerTests(){
    for(U32 test=0;test<11;++test){Reset();Save();if(test==0)parent=false;else if(test==1)cold=false;else if(test==2)g_pkg0=1;
        else if(test==3)g_pendingStatus=-1;else if(test==4)g_chapterStatus=-1;else if(test<9)failAllocation=test-4;else failProtect=test-8;
        Check(!CallbackInstall());Unchanged();}
    Reset();Save();failProtect=3;Check(!CallbackInstall());Unchanged();
    Reset();Save();changeCold=true;Check(!CallbackInstall());Unchanged();
    for(U32 f=1;f<=3;++f){Reset();Save();failFlush=f;Check(!CallbackInstall() && g_callbackStatus==-1);Unchanged();}
    for(U32 f=4;f<=6;++f){Reset();Save();failProtect=f;Check(!CallbackInstall() && g_callbackStatus==-1);Unchanged();}
    Reset();failFlush=1;failProtect=7;Check(!CallbackInstall() && g_callbackStatus==-2);
    Reset();changeCold=true;failProtect=4;Check(!CallbackInstall() && g_callbackStatus==-2);
    Reset();*reinterpret_cast<U8*>(Address(0x5567D0))=0x90;Check(!CallbackInstall());
}
static __declspec(align(16)) U8 beforeFx[512]={},afterFx[512]={};static U32 afterRegs[7]={},afterFlags=0,bridgeAddress=0,abiHandler=0;
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
    mov esi,abiHandler
    mov edi,66666666h
    mov ebp,77777777h
    fxsave [beforeFx]
    std
    stc
    call dword ptr [bridgeAddress]
    cld
    fninit
    popad
    ret}}
static void BridgeTests(){Arm();writerClobber=true;abiHandler=reinterpret_cast<UPtr>(handlers[0]);
    g_callbackTramps[1]=reinterpret_cast<UPtr>(&Capture);bridgeAddress=reinterpret_cast<UPtr>(&CallbackWriterBridge);Invoke();
    Check(writerChecks==1 && (afterFlags&0x401u)==0x401u);
    for(U32 n=0;n<7;++n)Check(afterRegs[n]==(n==4?abiHandler:0x11111111u*(n+1)));
    for(U32 n=0;n<24;++n)Check(beforeFx[n]==afterFx[n]);
    for(U32 n=0;n<8;++n)for(U32 j=0;j<10;++j)Check(beforeFx[32+n*16+j]==afterFx[32+n*16+j]);
    for(U32 n=160;n<288;++n)Check(beforeFx[n]==afterFx[n]);}
extern "C" void __stdcall CallbackTestMain(){globals=reinterpret_cast<UPtr>(Allocate(0x6000));Check(globals!=0);
    AddVectoredExceptionHandler(1,reinterpret_cast<void*>(&FixtureException));
    for(U32 i=0;i<4;++i){pages[i]=reinterpret_cast<UPtr>(Allocate(4096));Check(pages[i]!=0);}
    Say("unsafe\n");UnsafeRegression();Say("installer\n");InstallerTests();Say("native\n");NativeTests();Say("integrity\n");IntegrityTests();Say("bridge\n");BridgeTests();
    const char text[]="callback arena checks passed\n";DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),text,sizeof(text)-1,&written,nullptr);ExitProcess(0);}

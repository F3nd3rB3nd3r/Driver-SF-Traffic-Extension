// Production pending-map logic + selected native code, in fake packages only.
typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
static void* Allocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x40u);}
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static U32 checks=0;static void CheckAt(bool v,U32 line){++checks;if(!v){char text[64]="fixture failure at source line ";U32 n=31u,div=10000u;bool begun=false;
    for(;div;div/=10u){U32 digit=line/div%10u;if(digit || begun || div==1u){text[n++]=static_cast<char>('0'+digit);begun=true;}}text[n++]='\n';DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),text,n,&written,nullptr);ExitProcess(checks);}}
#define Check(v) CheckAt((v),__LINE__)
static UPtr pages[3]={};static U32 vas[3]={0x5A1000u,0x5A2000u,0x7D7000u};
static UPtr Address(UPtr p){for(U32 i=0;i<3u;++i)if(p>=vas[i] && p<vas[i]+4096u)return pages[i]+p-vas[i];ExitProcess(240);return 0;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool readable=true,cold=true,identity=true,parent=true;
static bool Readable(UPtr p,U32){return readable && p!=0u;}
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static void CapacityMax(volatile long& p,U32 v){if(v>static_cast<U32>(p))p=static_cast<long>(v);}
static UPtr g_pkg0=0;static const U32 kMemCommitReserve=0x3000u,kPageReadWrite=4u,kPageExecuteReadWrite=0x40u;
typedef UPtr (__thiscall* NativeMapInsert)(void*,UPtr,UPtr,UPtr);static NativeMapInsert g_origMapInsert=nullptr;
struct Node16{U32 a,b,key,value;};
static U32 allocations=0,protects=0,flushes=0,failAllocation=0,failProtect=0,failFlush=0;
static bool changeCold=false;
static void* MockAlloc(void*,U32 n,U32,U32){++allocations;return allocations==failAllocation?nullptr:Allocate(n);}
static BOOL MockProtect(void*,U32,U32,DWORD* old){++protects;*old=0x20u;if(changeCold && protects==1)cold=false;return protects!=failProtect;}
static BOOL MockFlush(void*,const void*,U32){++flushes;return flushes!=failFlush;}static void* GetCurrentProcess(){return nullptr;}
static long g_cleanupStatus=1;static bool CleanupSignature(bool){return parent;}static bool VerifyExecutableIdentity(){return identity;}static bool IndexedCold(){return cold;}
#define VirtualAlloc MockAlloc
#define VirtualProtect MockProtect
#define FlushInstructionCache MockFlush
#include "../src/V29sPendingMapCore.inc"
static U32 fatals=0,events=0;static void PendingEvent(const char*,UPtr,U32,U32){++events;}
static void PendingFatal(const char*,UPtr){++fatals;}
static bool nativeMode=true;static U32 helperCalls=0,helperKind=0,helperDf=0,captured[12]={};
extern "C" __declspec(noinline) void __cdecl PendingEntry(U32 kind,U32* f){
    if(nativeMode){Check(PendingTraversal(kind,f[kind==0u?6u:1u]));return;}
    ++helperCalls;helperKind=kind;for(U32 i=0;i<12u;++i)captured[i]=f[i];U32 flags=0;
    __asm {pushfd
        pop flags
        fninit
        fldz
        pxor xmm0,xmm0
        pxor xmm7,xmm7}helperDf=flags&0x400u;
}
extern "C" UPtr g_pendingTramps[2];
#include "../src/V29sPendingMapBridges.inc"
#include "../src/V29sPendingMapPatch.inc"
#undef FlushInstructionCache
#undef VirtualProtect
#undef VirtualAlloc
#include "PendingNativeBytes.inc"
static U8 packages[2][0x7400]={},oldExternal[448*16]={};static Node16 completedExtra[256]={};
static UPtr ctorFn=0,removeFn=0,walkFn=0;static bool retainPending=false;
extern "C" __declspec(naked) void __cdecl Construct(UPtr){__asm {push esi
    push ebx
    mov esi,[esp+12]
    xor ebx,ebx
    call dword ptr [ctorFn]
    pop ebx
    pop esi
    ret}}
extern "C" __declspec(naked) void __cdecl Remove(UPtr,UPtr){__asm {push edi
    push ebx
    mov edi,[esp+12]
    mov edx,[esp+16]
    or ebx,0FFFFFFFFh
    call dword ptr [removeFn]
    pop ebx
    pop edi
    ret}}
extern "C" __declspec(naked) U32 __cdecl Walk(UPtr){__asm {mov ecx,[esp+4]
    call dword ptr [walkFn]
    movzx eax,al
    ret}}
extern "C" void __cdecl FakeDrain(UPtr section){if(retainPending)return;const UPtr map=section+0x5F5Cu,end=section+0x6368u;
    while(Read32(end+4u)!=end)Remove(map,Read32(end+4u));}
static void AppendRet(UPtr p,U32 n){*reinterpret_cast<U8*>(p+n)=0xC3u;}
static void Initialize(UPtr section){Construct(section);
    const U32 maps[2]={0x5B44u,0x668Cu};for(U32 i=0;i<2u;++i){const UPtr map=section+maps[i],end=section+(i==0u?0x5F50u:0x6998u);
        Write32(map,map+12u);Write32(map+4u,0u);Write32(map+8u,0u);Write32(end,end);Write32(end+4u,end);}
}
static void Reset(){
    for(U32 i=0;i<3u;++i)memset(reinterpret_cast<void*>(pages[i]),0xCC,4096u);
    for(const NativeBlock& b:nativeBlocks)memcpy(reinterpret_cast<void*>(Address(b.at)),b.bytes,b.length);
    ctorFn=Address(0x5A1BD0u);removeFn=Address(0x5A242Cu);walkFn=Address(0x5A2230u);
    AppendRet(ctorFn,0x26u);AppendRet(removeFn,0x16u);
    // Keep real entry/early-return/end instructions; substitute only the work
    // between them, using the exact native pending-node removal instructions.
    UPtr p=Address(0x5A2246u);*reinterpret_cast<U8*>(p++)=0x57u; // PUSH EDI
    const U8 flag[]={0xC6,0x44,0x24,0x0B,0x01,0x56,0xE8};memcpy(reinterpret_cast<void*>(p),flag,sizeof(flag));p+=sizeof(flag);
    Write32(p,reinterpret_cast<UPtr>(&FakeDrain)-p-4u);p+=4u;
    const U8 cleanup[]={0x83,0xC4,0x04,0xE9};memcpy(reinterpret_cast<void*>(p),cleanup,sizeof(cleanup));p+=sizeof(cleanup);
    Write32(p,Address(0x5A2466u)-p-4u);
    memset(packages,0xA5,sizeof(packages));for(U32 i=0;i<2u;++i)Initialize(reinterpret_cast<UPtr>(packages[i]));
    g_origMapInsert=reinterpret_cast<NativeMapInsert>(Address(0x7D7270u));g_pkg0=0;
    g_pendingHead=nullptr;g_pendingLock=g_pendingGeneration=g_pendingAttempts=g_pendingStores=g_pendingRedirects=g_pendingReuses=0;
    g_pendingPeak=g_pendingIssuedPeak=g_pendingInvalid=g_pendingFull=g_pendingAllocFailed=g_pendingGuardBad=0;
    g_pendingWalkStarts=g_pendingWalkEnds=g_pendingEmpty=g_pendingDepth=g_pendingCurrent=g_pendingFree=g_pendingLastMap=g_pendingResets=0;
    g_pendingStatus=0;fatals=events=allocations=protects=flushes=failAllocation=failProtect=failFlush=0;
    cold=identity=parent=readable=true;changeCold=retainPending=false;g_cleanupStatus=1;nativeMode=true;
}
static void Arm(){Reset();Check(PendingInstall());g_pkg0=reinterpret_cast<UPtr>(packages[0]);}
static UPtr Position(UPtr map,U32 key){UPtr p=Read32(map+0x410u);const UPtr end=map+0x40Cu;U32 limit=0;
    while(p!=end && (Read32(p+8u)&0xFFFFu)<key){Check(++limit<=512u);p=Read32(p+4u);}return p;}
static UPtr Insert(UPtr map,U32 key,U32 value){U32 out=0,data[2]={key,value};UPtr at=Position(map,key);
    UPtr ret=PendingInsert(map,reinterpret_cast<UPtr>(&out),at,reinterpret_cast<UPtr>(data));if(ret)Check(ret==reinterpret_cast<UPtr>(&out));return out;}
static void NativeUnsafeRegression(){Reset();const UPtr map=reinterpret_cast<UPtr>(packages[0])+0x5F5Cu,end=map+0x40Cu;U32 out=0;
    for(U32 n=0;n<65u;++n){U32 data[2]={n,0xA0000000u+n};g_origMapInsert(reinterpret_cast<void*>(map),reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data));}
    Check(out==end && Read32(map+8u)==65u && Read32(map+0x418u)==0xA0000040u); // Neighbor header overwritten.
}
static void NativeMapTests(){Arm();const UPtr section=g_pkg0,map=section+0x5F5Cu,end=section+0x6368u;
    U8 neighbor[0x400];memcpy(neighbor,reinterpret_cast<void*>(section+0x6374u),sizeof(neighbor));
    Check(Walk(section)==1u && g_pendingEmpty==1 && !g_pendingHead);
    for(U32 n=0;n<512u;++n){UPtr at=Insert(map,n*3u+10u,0xA0000000u+n);Check(at && Read32(at+8u)==n*3u+10u);}
    PendingArena* first=PendingFind(map);Check(first && first->inlineUsed==64u && first->extraUsed==448u && !PendingGuards(first));
    Check(g_pendingPeak==512 && g_pendingIssuedPeak==512 && g_pendingRedirects==1 && PendingValidate(first));
    for(U32 i=0;i<sizeof(neighbor);++i)Check(Read8(section+0x6374u+i)==neighbor[i]);
    U32 out=0,data[2]={2000u,0u};Check(!PendingInsert(map,reinterpret_cast<UPtr>(&out),end,reinterpret_cast<UPtr>(data)) && fatals==1 && g_pendingFull==1 && !out);fatals=0;
    for(U32 n=0;n<256u;++n)Remove(map,Position(map,n*6u+10u));Check(PendingValidate(first) && Read32(map+4u)==256u);
    for(U32 n=0;n<256u;++n)Check(Insert(map,3000u+n,0xB0000000u+n)!=0u);
    Check(g_pendingReuses==256 && first->extraUsed==448u && !PendingGuards(first));
    retainPending=true;Check(Walk(section)==1u && g_pendingWalkStarts==1 && g_pendingWalkEnds==1 && Read32(map+8u)==512u);
    retainPending=false;Check(Walk(section)==1u && !Read32(map+8u) && g_pendingWalkStarts==2 && g_pendingWalkEnds==2 && !g_pendingDepth);
    for(U32 n=0;n<150u;++n)Check(Insert(map,n+100u,0xC0000000u+n)!=0u);
    Check(PendingValidate(first) && first->extraUsed==448u);Walk(section);
    // A genuine constructor reset gets NEW storage. Old external nodes remain.
    memcpy(oldExternal,first->nodes,sizeof(oldExternal));Construct(section);Check(PendingTraversal(0u,section));
    for(U32 n=0;n<154u;++n)Check(Insert(map,2000u+n,0xD0000000u+n)!=0u);
    PendingArena* second=PendingFind(map);Check(second!=first && second->generation!=first->generation && g_pendingResets==1);
    for(U32 i=0;i<sizeof(oldExternal);++i)Check(Read8(reinterpret_cast<UPtr>(first->nodes)+i)==oldExternal[i]);
    // Completed and pending maps use distinct backing at the same time.
    const UPtr completed=section+0x5B44u,completedEnd=section+0x5F50u;Check(!PendingOwns(completed));
    for(U32 n=0;n<154u;++n){if(n==64u)Write32(completed,reinterpret_cast<UPtr>(completedExtra));U32 pair[2]={n,0xE0000000u+n},result=0;
        g_origMapInsert(reinterpret_cast<void*>(completed),reinterpret_cast<UPtr>(&result),completedEnd,reinterpret_cast<UPtr>(pair));}
    Check(Read32(completed+8u)==154u && PendingValidate(second));Walk(section);Check(Read32(completed+8u)==154u);
    // Keep a second owner live; neither owner steals or resets the other's arena.
    const UPtr next=reinterpret_cast<UPtr>(packages[1]);g_pkg0=next;for(U32 n=0;n<154u;++n)Check(Insert(next+0x5F5Cu,n+50u,0xF0000000u+n)!=0u);
    Check(PendingFind(next+0x5F5Cu)!=second && PendingOwns(map));Check(Insert(map,42u,0x12345678u)!=0u);Walk(section);Walk(next);
    Check(!fatals && !g_pendingInvalid && !g_pendingGuardBad);
}
static U8 snapshot[4096];static void Unchanged(){for(U32 i=0;i<4096u;++i)Check(Read8(pages[1]+i)==snapshot[i]);}
static void InstallerTests(){
    for(U32 test=0;test<8u;++test){Reset();memcpy(snapshot,reinterpret_cast<void*>(pages[1]),4096u);
        if(test==0)identity=false;else if(test==1)parent=false;else if(test==2)cold=false;else if(test==3)g_pkg0=1u;
        else if(test==4)g_origMapInsert=nullptr;else if(test==5)g_cleanupStatus=-1;else failAllocation=test-5u;
        Check(!PendingInstall());Unchanged();}
    Reset();memcpy(snapshot,reinterpret_cast<void*>(pages[1]),4096u);failProtect=1;Check(!PendingInstall());Unchanged();
    Reset();memcpy(snapshot,reinterpret_cast<void*>(pages[1]),4096u);changeCold=true;Check(!PendingInstall());Unchanged();
    Reset();changeCold=true;failProtect=2;Check(!PendingInstall() && g_pendingStatus==-2);
    Reset();memcpy(snapshot,reinterpret_cast<void*>(pages[1]),4096u);failFlush=1;Check(!PendingInstall() && g_pendingStatus==-1);Unchanged();
    Reset();memcpy(snapshot,reinterpret_cast<void*>(pages[1]),4096u);failProtect=2;Check(!PendingInstall() && g_pendingStatus==-1);Unchanged();
    Reset();failFlush=1;failProtect=3;Check(!PendingInstall() && g_pendingStatus==-2);
    Reset();failFlush=1;failProtect=4;Check(!PendingInstall() && g_pendingStatus==-2);
    Reset();*reinterpret_cast<U8*>(Address(0x5A2230u))=0x90;Check(!PendingInstall());
}
static void IntegrityTests(){
    Arm();const UPtr map=g_pkg0+0x5F5Cu;failAllocation=allocations+1u;U32 out=0,pair[2]={12u,34u};
    Check(!PendingInsert(map,reinterpret_cast<UPtr>(&out),map+0x40Cu,reinterpret_cast<UPtr>(pair)) && g_pendingAllocFailed==1 && !Read32(map+8u));
    Arm();Write32(g_pkg0+0x5F64u,1u);Check(!PendingPrepare(g_pkg0+0x5F5Cu) && g_pendingInvalid==1);
    Arm();UPtr p=Insert(g_pkg0+0x5F5Cu,10u,22u);PendingArena* a=PendingFind(g_pkg0+0x5F5Cu);a->after[0]=0;Check(!PendingValidate(a) && g_pendingGuardBad==1);
    Arm();p=Insert(g_pkg0+0x5F5Cu,10u,22u);Write32(p+4u,p);Check(!PendingTraversal(0u,g_pkg0) && g_pendingInvalid);
    Arm();for(U32 n=0;n<3u;++n)Insert(g_pkg0+0x5F5Cu,n+10u,n);UPtr m=g_pkg0+0x5F5Cu;Remove(m,Read32(m+0x410u));
    Write32(Read32(m),Read32(m));Check(!PendingValidate(PendingFind(m)) && g_pendingInvalid);
    Arm();Insert(g_pkg0+0x5F5Cu,10u,22u);Write32(g_pkg0+0x6694u,0x30318BF3u);Check(!PendingTraversal(0u,g_pkg0) && g_pendingInvalid);
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
static void BridgeTests(){nativeMode=false;for(U32 i=0;i<2u;++i){g_pendingTramps[i]=reinterpret_cast<UPtr>(&Capture);bridgeAddress=kPendingSites[i].bridge;Invoke();
    Check(helperCalls==i+1u && helperKind==i && !helperDf && (afterFlags&0x401u)==0x401u);Check(captured[10]==0x12345678u && captured[11]==0xABCDEF01u);
    for(U32 n=0;n<7u;++n)Check(afterRegs[n]==0x11111111u*(n+1u));for(U32 n=0;n<24u;++n)Check(beforeFx[n]==afterFx[n]);
    for(U32 n=0;n<8u;++n)for(U32 j=0;j<10u;++j)Check(beforeFx[32u+n*16u+j]==afterFx[32u+n*16u+j]);for(U32 n=160u;n<288u;++n)Check(beforeFx[n]==afterFx[n]);}}
extern "C" void __stdcall PendingTestMain(){for(U32 i=0;i<3u;++i){pages[i]=reinterpret_cast<UPtr>(Allocate(4096u));Check(pages[i]!=0u);}
    NativeUnsafeRegression();InstallerTests();NativeMapTests();IntegrityTests();BridgeTests();
    const char message[]="pending map checks passed\n";DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);ExitProcess(0u);}

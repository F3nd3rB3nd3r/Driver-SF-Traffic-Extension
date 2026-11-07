typedef unsigned char U8;typedef unsigned short U16;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
static void* Allocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x40u);}
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;char text[64]="fixture failure at source line ";U32 n=31;bool begun=false;
    for(U32 div=10000;div;div/=10){U32 digit=line/div%10;if(digit || begun || div==1){text[n++]=static_cast<char>('0'+digit);begun=true;}}
    text[n++]='\n';text[n]=0;Say(text);ExitProcess(line);}
#define Check(v) CheckAt((v),__LINE__)
static UPtr Address(UPtr p){return p;}static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool readable=true,clobber=false;static bool Readable(UPtr p,U32){if(clobber){U32 flags=0;__asm {pushfd
    pop flags
    fninit
    fldz
    pxor xmm0,xmm0
    pxor xmm7,xmm7}Check(!(flags&0x400));}return readable && p!=0;}
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static const U32 kMemCommitReserve=0x3000,kPageReadWrite=4;static bool failAllocation=false;
static void* MockAlloc(void*,U32 n,U32,U32){return failAllocation?nullptr:Allocate(n);}
static long g_sScratchHook=2,g_sSpool0Used=0,g_sScratchUsed=0,g_sSpool0High=0,g_sSpool0Redirects=0,g_sSpool0Fault=0,g_sScratchCalls=0,g_sScratchFault=0,g_sCloneUsed=0;
static UPtr g_sScratchTramps[2]={},g_sTramp[5]={};
static void LegacyExit(U32,const char*,UPtr=0,UPtr=0,U32=0,U32=0);
#define VirtualAlloc MockAlloc
#define DSF_V29S_ARENA_REUSE 1
#include "../src/V29sArenaReuseCore.inc"
#include "ArenaReuseAdapters.inc"
#include "../src/V29sArenaReuseBridges.inc"
#undef VirtualAlloc
static U32 g_capacityStart=0,lastExit=0,exitCalls=0;static long g_callbackStatus=1,DSFV27B_RequestFramePatchCount=13,DSFV27B_Merge150Fix=1;
struct RequestFramePatch {UPtr at;U8 length,immOffset;U8 stock[7];U32 replacement;bool done;};
static RequestFramePatch g_requestFramePatches[13]={};static U32 installedFrame[13]={};static void* g_merge150Clone=nullptr;
static char output[16000]={};static U32 outputUsed=0;
static void CapacityWrite(const char* s){for(U32 n=0;s[n] && outputUsed+1<sizeof(output);++n)output[outputUsed++]=s[n];output[outputUsed]=0;}
static void OutputDebugStringA(const char*){}static U32 GetTickCount(){return 1234;}
static void AppendText(char*& p,const char* text,const char* end){for(U32 n=0;text[n] && p<end;++n)*p++=text[n];}
static void DiagFAppendKeyU(char*& p,const char* key,U32 value,const char* end){AppendText(p,key,end);char digits[11]={};U32 n=0;
    do{digits[n++]=static_cast<char>('0'+value%10);value/=10;}while(value);while(n && p<end)*p++=digits[--n];}
static void DiagFAppendKeyHex(char*& p,const char* key,U32 value,const char* end){AppendText(p,key,end);const char* digits="0123456789ABCDEF";
    for(U32 n=0;n<8 && p<end;++n)*p++=digits[(value>>(28-n*4))&15];}
static void MockExit(U32 code){lastExit=code;++exitCalls;}
#define ExitProcess MockExit
#include "../src/V29sArenaReuse.inc"
#undef ExitProcess
namespace LegacyScratch {
static const U32 kSScratchNodes=256u,kSScratchArenas=8u;
struct DiagSScratchArena {UPtr sentinel;U32 nodes[kSScratchNodes][3];};
static DiagSScratchArena g_sScratchArenas[kSScratchArenas]={};
static volatile long g_sScratchCalls=0,g_sScratchUsed=0,g_sScratchLock=0,g_sScratchRedirects=0,g_sScratchFault=0;
#undef DSF_V29S_ARENA_REUSE
#include "LegacyScratchAdapter.inc"
#define DSF_V29S_ARENA_REUSE 1
}
#include "ArenaReuseNativeBytes.inc"
static bool Contains(const char* text){for(U32 i=0;i<outputUsed;++i){U32 n=0;while(text[n] && output[i+n]==text[n])++n;if(!text[n])return true;}return false;}
static UPtr code=0,scratch=0,owners=0,nativeEsp=0,nativeTarget=0,savedEsp=0,nativeId=0;static bool spy=false;static U32 abiOwner=0;
static __declspec(align(16)) U8 beforeFx[512]={},afterFx[512]={};static U32 afterRegs[7]={},afterFlags=0;
extern "C" __declspec(naked) void NativeReturn(){__asm {cld
    fninit
    mov esp,savedEsp
    popad
    ret}}
extern "C" __declspec(naked) void Invoke(){__asm {pushad
    mov savedEsp,esp
    fninit
    fld1
    pcmpeqd xmm0,xmm0
    pcmpeqd xmm7,xmm7
    fxsave [beforeFx]
    mov esp,nativeEsp
    mov dword ptr [esp],offset NativeReturn
    mov eax,[esp+14h]
    mov ecx,abiOwner
    mov edx,eax
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,nativeId
    mov ebp,77777777h
    cmp spy,0
    je normal
    std
    stc
normal:
    jmp dword ptr [nativeTarget]}}
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
    jmp NativeReturn}}
static void InitList(UPtr h,U32 count){const UPtr end=h+12+count*12;Write32(h,h+12);Write32(h+4,0);Write32(h+8,0);Write32(end,end);Write32(end+4,end);}
static void InitScratch(){memset(reinterpret_cast<void*>(scratch),0xA5,0x2000);nativeEsp=scratch+0x800;InitList(nativeEsp+0x14,150);}
static void Append(U32 site,U32 id){nativeId=id;nativeTarget=site?reinterpret_cast<UPtr>(&DiagSScratchSecondBridge):reinterpret_cast<UPtr>(&DiagSScratchBridge);
    // Site1 begins after MOV ECX,EAX. Invoke loads ECX via abiOwner.
    abiOwner=Read32(nativeEsp+0x14);Invoke();}
typedef void (__thiscall* Merge)(void*,UPtr);
static void Reset(){g_spoolReuseHead=nullptr;g_sSpool0Used=g_sScratchUsed=g_sSpool0High=g_sSpool0Redirects=g_sSpool0Fault=g_sScratchCalls=g_sScratchFault=0;
    g_reuseLock=g_reuseAllocations=g_reuseSpoolChecks=g_reuseScratchChecks=g_reuseRedirects=g_reuseResets=g_reuseInvalid=g_reuseGuardBad=g_reuseAllocFailed=0;
    g_reuseSpoolIssuedPeak=g_reuseScratchPeak=0;outputUsed=lastExit=exitCalls=0;output[0]=0;readable=true;clobber=spy=failAllocation=false;g_sScratchHook=2;
    memset(reinterpret_cast<void*>(owners),0,16*0x1000);for(U32 n=0;n<16;++n)InitList(owners+n*0x1000+0xCC,50);InitScratch();}
static void CheckListValues(UPtr h,U32 first,U32 count){UPtr p=Read32(h+0x268);for(U32 n=0;n<count;++n){Check((Read32(p+8)&0xFFFF)==first+n);p=Read32(p+4);}Check(p==h+0x264);}
static void LegacyRegression(){Reset();
    for(U32 generation=0;generation<9;++generation){InitScratch();for(U32 n=0;n<50;++n)Append(n&1,n+1);
        U32 frame[9]={};frame[3]=nativeEsp-4;frame[7]=Read32(nativeEsp+0x14);
        LegacyScratch::LegacyPrepareScratch(0,reinterpret_cast<UPtr>(frame));
        Check(LegacyScratch::g_sScratchUsed==static_cast<long>(generation<8?generation+1:8));
        Check(generation<8?!exitCalls:lastExit==0x2D && exitCalls==1);
    }
}
static void NativeTests(){Reset();const UPtr owner=owners,h=owner+0xCC;Write32(owner+0xC8,150);
    for(U32 generation=0;generation<64;++generation){InitScratch();for(U32 n=0;n<150;++n)Append(n&1,generation*150+n+1);
        Check(!exitCalls && Read32(nativeEsp+0x1C)==150 && Read32(nativeEsp+0x14)==nativeEsp+0x728);
        DiagSPrepareSpool0(owner);Check(!exitCalls);reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);
        const char* why="none";ReuseListState state={};Check(ReuseValidate(h,50,ReuseFind(owner),state,why));
        CheckListValues(h,generation*150+1,150);Check(g_sSpool0Used==1 && !g_sScratchUsed);
        // Destroying the local frame must not change persistent list values.
        InitScratch();CheckListValues(h,generation*150+1,150);
        if(generation%4==3)InitList(h,50); // Genuine native owner-header reset.
    }
    Check(g_reuseResets==15 && !g_reuseInvalid && !g_reuseGuardBad);
    // Distinct owners get distinct process-lived backing with no8-owner cap.
    for(U32 n=1;n<16;++n){UPtr o=owners+n*0x1000;Write32(o+0xC8,150);DiagSPrepareSpool0(o);Check(!exitCalls && ReuseFind(o)!=ReuseFind(owner));}
    Check(g_sSpool0Used==16);
    // Native destination shrink recycles nodes; later refill reuses them.
    Reset();Write32(owner+0xC8,150);DiagSPrepareSpool0(owner);for(U32 n=0;n<150;++n)Append(n&1,n+1);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);
    InitScratch();for(U32 n=0;n<30;++n)Append(n&1,n+20);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);Check(Read32(h+8)==30 && Read32(h+4)==120);
    DiagSPrepareSpool0(owner);Check(!exitCalls);InitScratch();for(U32 n=0;n<150;++n)Append(n&1,n+1);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);CheckListValues(h,1,150);Check(!Read32(h+4));
    // Attach with an existing native-only recycled chain: redirect its tail.
    Reset();for(U32 n=0;n<30;++n)Append(n&1,n+1);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);
    InitScratch();for(U32 n=0;n<10;++n)Append(n&1,n+1);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);Check(Read32(h+4)==20);
    Write32(owner+0xC8,150);DiagSPrepareSpool0(owner);Check(!exitCalls && Read32(h+4)==20);
    InitScratch();for(U32 n=0;n<150;++n)Append(n&1,n+1);
    reinterpret_cast<Merge>(g_merge150Clone)(reinterpret_cast<void*>(h),nativeEsp+0x14);CheckListValues(h,1,150);
    const char* why="none";ReuseListState state={};Check(ReuseValidate(h,50,ReuseFind(owner),state,why) && state.nativeIssued==30 && state.extraIssued==120);
}
static void FailureTests(){Reset();const char* why="none";U32 frame[9]={};frame[3]=nativeEsp-4;frame[7]=Read32(nativeEsp+0x14);
    for(U32 n=0;n<150;++n)Append(n&1,n+1);frame[7]=Read32(nativeEsp+0x14);Check(!ReusePrepareScratch(0,reinterpret_cast<UPtr>(frame),why));
    Reset();frame[3]=nativeEsp-4;frame[7]=nativeEsp+0x278;Check(!ReusePrepareScratch(0,reinterpret_cast<UPtr>(frame),why));
    Reset();failAllocation=true;Write32(owners+0xC8,150);DiagSPrepareSpool0(owners);Check(lastExit==0x2C && Contains("LEGACY_FATAL") && Contains("reason=arenaAllocation") && Contains("exitCode=0000002C"));
    Reset();Write32(owners+0xC8,150);DiagSPrepareSpool0(owners);ReuseFind(owners)->after[0]=0;DiagSPrepareSpool0(owners);Check(lastExit==0x2C && Contains("reason=arenaGuard"));
    Reset();Write32(owners+0xC8,151);DiagSPrepareSpool0(owners);Check(lastExit==0x2C && Contains("reason=requestBound"));
    Reset();Write32(owners+0xD4,1);DiagSPrepareSpool0(owners);Check(lastExit==0x2C);
    Reset();LegacyExit(0x2E,"spoolClone",owners,123,45,6);Check(Contains("reason=spoolClone") && Contains("exitCode=0000002E") && Contains("active=45"));
}
static void BridgeTests(){for(U32 i=0;i<3;++i){Reset();InitScratch();abiOwner=i==2?owners:Read32(nativeEsp+0x14);nativeId=0x66666666;
        g_sScratchTramps[0]=g_sScratchTramps[1]=g_sTramp[0]=reinterpret_cast<UPtr>(&Capture);
        nativeTarget=i==0?reinterpret_cast<UPtr>(&DiagSScratchBridge):i==1?reinterpret_cast<UPtr>(&DiagSScratchSecondBridge):reinterpret_cast<UPtr>(&DiagSReconcileBridge);
        const UPtr cursor=Read32(nativeEsp+0x14);clobber=spy=true;Invoke();clobber=spy=false;
        Check((afterFlags&0x401)==0x401 && !exitCalls);Check(afterRegs[0]==cursor && afterRegs[1]==abiOwner && afterRegs[2]==cursor);
        Check(afterRegs[3]==0x44444444 && afterRegs[4]==0x55555555 && afterRegs[5]==0x66666666 && afterRegs[6]==0x77777777);
        for(U32 n=0;n<24;++n)Check(beforeFx[n]==afterFx[n]);for(U32 n=0;n<8;++n)for(U32 b=0;b<10;++b)Check(beforeFx[32+n*16+b]==afterFx[32+n*16+b]);
        for(U32 n=160;n<288;++n)Check(beforeFx[n]==afterFx[n]);}
}
extern "C" void __stdcall ArenaReuseTestMain(){code=reinterpret_cast<UPtr>(Allocate(0x2000));scratch=reinterpret_cast<UPtr>(Allocate(0x2000));owners=reinterpret_cast<UPtr>(Allocate(16*0x1000));Check(code && scratch && owners);
    for(U32 i=0;i<2;++i){const UPtr fn=code+i*0x100;memcpy(reinterpret_cast<void*>(fn),nativeBlocks[i].bytes,nativeBlocks[i].length);
        for(U32 n=0;n<4;++n)Write32(fn+scratchOperands[i][n],0x728);*reinterpret_cast<U8*>(fn+nativeBlocks[i].length)=0xC3;
        UPtr tramp=code+0x400+i*0x100;memcpy(reinterpret_cast<void*>(tramp),reinterpret_cast<void*>(fn),5);*reinterpret_cast<U8*>(tramp+5)=0xE9;Write32(tramp+6,fn+5-tramp-10);g_sScratchTramps[i]=tramp;}
    g_merge150Clone=reinterpret_cast<void*>(code+0x1000);memcpy(g_merge150Clone,nativeBlocks[2].bytes,nativeBlocks[2].length);Write32(code+0x1010,0x718);Write32(code+0x1016,0x714);
    for(U32 n=0;n<13;++n){installedFrame[n]=0x728;g_requestFramePatches[n].at=reinterpret_cast<UPtr>(&installedFrame[n]);g_requestFramePatches[n].replacement=0x728;g_requestFramePatches[n].done=true;}
    Check(ArenaReuseStartup());installedFrame[3]=0;Check(!ArenaReuseStartup());installedFrame[3]=0x728;
    LegacyRegression();NativeTests();FailureTests();BridgeTests();Say("arena reuse checks passed\n");ExitProcess(0);}

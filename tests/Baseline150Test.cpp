typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("baseline fixture failed\n");ExitProcess(line);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long n){long old=*p;*p=n;return old;}
static long InterlockedCompareExchange(volatile long* p,long n,long old){long seen=*p;if(seen==old)*p=n;return seen;}
static U32 now=10000u;static U32 GetTickCount(){return now;}
static char output[900000]={};static U32 outputUsed=0;
static void CapacityWrite(const char* s){for(U32 i=0;s[i] && outputUsed+1<sizeof(output);++i)output[outputUsed++]=s[i];output[outputUsed]=0;}
static void AppendText(char*& p,const char* s,const char* end){for(U32 i=0;s[i] && p<end;++i)*p++=s[i];}
static void DiagFAppendU(char*& p,U32 n,const char* end){char digits[11]={};U32 count=0;do{digits[count++]=static_cast<char>('0'+n%10);n/=10;}while(n);while(count && p<end)*p++=digits[--count];}
static void DiagFAppendKeyU(char*& p,const char* s,U32 n,const char* end){AppendText(p,s,end);DiagFAppendU(p,n,end);}
static void AppendNumber(char*& p,int n,const char* end){if(n<0){AppendText(p,"-",end);n=-n;}DiagFAppendU(p,static_cast<U32>(n),end);}
static void DiagFAppendKeyHex(char*& p,const char* s,U32 n,const char* end){AppendText(p,s,end);const char* hex="0123456789ABCDEF";for(U32 i=0;i<8 && p<end;++i)*p++=hex[(n>>(28-i*4))&15];}
struct AudioModelKey {long key,stores,removes,hits,misses,bankReady,bankNotReady,setupOk,setupFail;};
static AudioModelKey g_audioModelKeys[512]={};
struct CallbackGroup {long active;};static CallbackGroup callbackGroups[21]={};static CallbackGroup* g_callbackGroups=callbackGroups;
struct AudioSlotsList {long updateCount;};static AudioSlotsList g_audioSlotsLists[3]={};
static long g_cleanupCurrent=0,g_pendingCurrent=0,g_audioProbeEmitterPeak=0,DSFV29_HashExtraGranted=0,g_reuseAllocations=0;
struct DiagFProcessMemoryCounters32 {U32 cb;U32 unused[8];U32 workingSetSize,privateUsage;};
static BOOL GetProcessMemoryInfo(void*,void* value,DWORD){auto* m=static_cast<DiagFProcessMemoryCounters32*>(value);m->privateUsage=104857600;m->workingSetSize=52428800;return 1;}
static void* GetCurrentProcess(){return nullptr;}
static BOOL FlushInstructionCache(void*,const void*,U32){return 1;}
static BOOL VirtualProtect(void*,U32,DWORD,DWORD*){return 0;}
static const U32 kPageExecuteReadWrite=0x40u,kRootPtr=0x00FE7228u;static U32 g_capacityStart=0;
struct DiagFDetour {U32 unused;};
static bool DiagFInstallDetour(DiagFDetour&,UPtr,const U8*,U32,UPtr,UPtr&){return false;}
static void DiagFRestoreDetour(DiagFDetour&,UPtr){}
static bool ArenaReuseStartup(){return false;}
static U8 rootBytes[0x758]={},ownerBytes[0xDB0]={},managerBytes[0x70000]={},vehicleBytes[0x600]={},modelBytes[0x4900]={},definitionBytes[32]={};
static UPtr rootPtr=reinterpret_cast<UPtr>(rootBytes),g_pkg0=reinterpret_cast<UPtr>(managerBytes)+0xAF68;
static U32 missing=0,readyCalls=0;static bool clobber=false;
static U8 __fastcall ReadyMock(void* manager,void*,U32 kind,U32 id){Check(manager==managerBytes && kind==0);++readyCalls;
    if(clobber){U32 flags=0;__asm {pushfd
        pop flags
        fninit
        fldz
        pxor xmm0,xmm0
        pxor xmm7,xmm7}Check(!(flags&0x400));}
    return id==missing?0u:1u;
}
static UPtr Address(UPtr p){if(p==kRootPtr)return reinterpret_cast<UPtr>(&rootPtr);if(p==0x59CFC0)return reinterpret_cast<UPtr>(&ReadyMock);return p;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 n){*reinterpret_cast<volatile U32*>(p)=n;}
static U8 Read8(UPtr p){if(p>=0x5A7B80u && p<0x5A7B8Au)return 0;if(p>=0x8D590Cu && p<0x8D5911u)return 0;return *reinterpret_cast<volatile U8*>(p);}
static bool Readable(UPtr p,U32){return p!=0;}
#include "../src/Chapter7Preload.h"
#include "../src/V29sBaseline150.inc"
static U32 regs[7]={},flags=0,bridgeKind=0,callOwner=0,savedStack=0;
static __declspec(align(16)) U8 fxBefore[512]={},fxAfter[512]={};
extern "C" __declspec(naked) void Continuation(){__asm {
    mov regs,eax
    mov regs+4,ecx
    mov regs+8,edx
    mov regs+12,ebx
    mov regs+16,esi
    mov regs+20,edi
    mov regs+24,ebp
    pushfd
    pop flags
    fxsave [fxAfter]
    cld
    ret}}
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov savedStack,esp
    fninit
    fld1
    pcmpeqd xmm0,xmm0
    pcmpeqd xmm7,xmm7
    fxsave [fxBefore]
    mov eax,11111111h
    mov ecx,callOwner
    mov edx,33333333h
    mov ebx,offset vehicleBytes
    mov esi,55555555h
    mov edi,66666666h
    mov ebp,77777777h
    cmp bridgeKind,0
    jne draw
    std
    stc
    call BaselineUpdateBridge
    jmp done
draw:
    std
    stc
    call BaselineDrawBridge
done:
    cld
    fninit
    mov esp,savedStack
    popad
    ret}}
static void CheckAbi(){Check(regs[0]==0x11111111 && regs[1]==callOwner && regs[2]==0x33333333 && regs[3]==reinterpret_cast<UPtr>(vehicleBytes));
    Check(regs[4]==0x55555555 && regs[5]==0x66666666 && regs[6]==0x77777777 && (flags&0x401)==0x401);
    for(U32 i=0;i<512;++i)if(i!=6 && i!=7)Check(fxBefore[i]==fxAfter[i]);}
extern "C" void __stdcall BaselineTestMain(){
    Write32(rootPtr+0x214,reinterpret_cast<UPtr>(ownerBytes));Write32(rootPtr+0x204,reinterpret_cast<UPtr>(managerBytes));
    Write32(reinterpret_cast<UPtr>(ownerBytes)+0xDA8,reinterpret_cast<UPtr>(managerBytes));Write32(reinterpret_cast<UPtr>(managerBytes)+0x6FD94,1);
    g_baselineInstalled=true;callOwner=reinterpret_cast<UPtr>(ownerBytes);
    BaselineRequest(callOwner,0x66E5F8,kChapter7AllPositiveIds,150,kChapter7AllPositiveIds,150,"accepted");
    Check(g_baseline.count==150 && g_baseline.duplicates==0 && g_baseline.originalValid);
    g_baselineUpdateTrampoline=reinterpret_cast<UPtr>(&Continuation);g_baselineDrawTarget=g_baselineUpdateTrampoline;
    clobber=true;Invoke();CheckAbi();Check(readyCalls==150 && g_baseline.readyCount==150 && g_baseline.checkpoints==1 && g_baseline.allReadyTick==now);
    missing=303;now+=1000;BaselineReadyEntry(callOwner);Check(g_baseline.readyCount==149);
    Check(g_baseline.models[149].id==303 && g_baseline.models[149].readyLosses==1);
    missing=0;now+=1000;BaselineReadyEntry(callOwner);Check(g_baseline.readyCount==150 && g_baseline.models[149].readyChecks==3);
    Write32(reinterpret_cast<UPtr>(vehicleBytes)+0x5AC,187);Write32(reinterpret_cast<UPtr>(vehicleBytes)+0x5B4,reinterpret_cast<UPtr>(modelBytes));
    Write32(reinterpret_cast<UPtr>(modelBytes),0xE39EAC);Write32(reinterpret_cast<UPtr>(modelBytes)+0x4850,reinterpret_cast<UPtr>(definitionBytes));Write32(reinterpret_cast<UPtr>(definitionBytes)+0x14,187);
    bridgeKind=1;Invoke();CheckAbi();Check(g_baselineDrawCalls==1 && !g_baselineDrawUnmapped && !g_baseline.drawMismatch);
    Write32(reinterpret_cast<UPtr>(definitionBytes)+0x14,62);Invoke();Check(g_baseline.drawMismatch==1);
    for(U32 i=0;i<150;++i)if(g_baseline.models[i].id==187)Check(g_baseline.models[i].drawFallback==1 && g_baseline.models[i].lastResolved==62);
    g_cleanupCurrent=52157;BaselinePoll();Check(g_baseline.metricPeak[0]==52157);
    const U32 seq=g_baseline.seq;BaselineRequest(callOwner,1,nullptr,151,nullptr,0,"invalidInput");Check(g_baseline.seq==seq && g_baseline.count==150);
    BaselinePoll();g_audioModelKeys[0].key=187;g_audioModelKeys[0].setupFail=7;
    U32 shortList[3]={62,123,130};BaselineRequest(callOwner,0x66E5F8,shortList,3,shortList,3,"accepted");
    const BaselineState& previous=g_baselineEvents[(g_baselineWrite-2)%8u].state;
    for(U32 i=0;i<150;++i)if(previous.models[i].id==187)Check(previous.models[i].audioSample[7]==7);
    g_audioModelKeys[0].setupFail=99;
    for(U32 i=0;i<150;++i)if(previous.models[i].id==187)Check(previous.models[i].audioSample[7]==7);
    Check(g_baseline.count==3 && g_baseline.models[0].id==62);BaselinePoll();
    U32 duplicateList[3]={62,62,130};BaselineRequest(callOwner,1,duplicateList,3,duplicateList,3,"accepted");Check(g_baseline.duplicates==1);BaselinePoll();
    InterlockedExchange(&g_baselineLock,1);BaselineRequest(callOwner,1,shortList,3,shortList,3,"accepted");Check(g_baseline.count==3);InterlockedExchange(&g_baselineLock,0);Check(g_baselineLockSkips>0);
    for(U32 n=0;n<30;++n)BaselineRequest(callOwner,1,shortList,3,shortList,3,"accepted");Check(g_baselineDrops>0 && g_baseline.count==3);
    now+=1000;BaselinePoll();Say(output);Say("baseline150 checks passed\n");ExitProcess(0);
}

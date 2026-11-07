typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" int _fltused=0;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentProcess();
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("notify fixture failure at line ");char s[12]={};U32 n=0;
    do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
struct RequestSidecar {UPtr owner;U32 count,ids[150];};static RequestSidecar request={};static bool haveSidecar=true,locked=false;
static void RequestLock(){Check(!locked);locked=true;}static void RequestUnlock(){Check(locked);locked=false;}
static RequestSidecar* RequestSlot(UPtr owner,bool create){Check(locked && !create);return haveSidecar && request.owner==owner?&request:nullptr;}
static U32 fatals=0,eventCount=0,lastHash=0,lastCount=0;static bool lastSidecar=false;
static void NotifyFatal(const char*,UPtr,U32){++fatals;}
static void NotifyEvent(UPtr,U32 count,U32 hash,U32,U32,bool sidecar){++eventCount;lastHash=hash;lastCount=count;lastSidecar=sidecar;}
#include "../src/V29sNotify150Native.inc"
#include "../src/V29sNotify150Layout.inc"
#include "../src/V29sNotify150Core.inc"
#include "../src/Chapter7Preload.h"
static U8 root[0x240]={},parent[0x4EA00]={},recorder[0x8600]={},owner[0x2000]={};
static UPtr rootPointer=reinterpret_cast<UPtr>(root),code=0,entry=0,stackTop=0,hostEsp=0;
static UPtr returnSlot=0,expectedReturn=0;static U32 countArg=0,observedBadReturn=0;
static bool repairOldReturn=false,replayCanApply=true;static U32 replayCount=0,replayCalls=0;
static U32 afterRegisters[4]={},afterEsp=0;static unsigned short afterFp=0;
static U8 ringStorage[16416]={},message[640]={};static U32 ringOffset=0,packetBytes=0;
static const U32 ringCapacity=16384u;
extern "C" __declspec(noinline) void* __cdecl RawCopy(void* d,const void* s,U32 n){
    memcpy(d,s,n);
    if(repairOldReturn && s==owner && n>200u){observedBadReturn=Read32(returnSlot);Check(observedBadReturn==countArg);Write32(returnSlot,expectedReturn);}
    return d;
}
static __declspec(noreturn) void __cdecl Unexpected(){Say("unexpected native service\n");ExitProcess(2);}
static UPtr __cdecl Factory(){return rootPointer;}
static float __cdecl Timestamp(){return 1.25f;}
static U8 __fastcall ReplayReady(void*,void*){return replayCanApply?1u:0u;}
static void __fastcall ReplaySubmit(void*,void*,const U32* ids,U32 count){
    ++replayCalls;replayCount=count;Check(count<=150u);
    for(U32 i=0;i<count;++i)Check(ids[i]==request.ids[i]);
}
static UPtr Resolve(U32 address){
    if(address==0x40A690u)return reinterpret_cast<UPtr>(&Factory);
    if(address==0xC28820u)return reinterpret_cast<UPtr>(&RawCopy);
    if(address==0x738CE0u)return reinterpret_cast<UPtr>(&Timestamp);
    if(address==0x7467D0u)return code+0x1000u;
    if(address==0x746650u)return code+0x2000u;
    if(address==0x74B9D0u)return reinterpret_cast<UPtr>(&ReplayReady);
    if(address==0x74B950u)return reinterpret_cast<UPtr>(&ReplaySubmit);
    return reinterpret_cast<UPtr>(&Unexpected);
}
static void Relocate(UPtr base,const U32 (*calls)[2],U32 callCount,const U32* roots,U32 rootCount){
    for(U32 i=0;i<callCount;++i)Write32(base+calls[i][0]+1u,Resolve(calls[i][1])-(base+calls[i][0]+5u));
    for(U32 i=0;i<rootCount;++i)if(roots[i])Write32(base+roots[i],reinterpret_cast<UPtr>(&rootPointer));
}
extern "C" __declspec(naked) void Invoke(){__asm {
    pushad
    mov hostEsp,esp
    fninit
    mov esp,stackTop
    mov expectedReturn,offset capture
    mov eax,esp
    sub eax,12
    mov returnSlot,eax
    mov ebp,77777777h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,offset parent
    push countArg
    push offset owner
    call dword ptr [entry]
capture:
    mov afterRegisters,ebx
    mov afterRegisters+4,esi
    mov afterRegisters+8,edi
    mov afterRegisters+12,ebp
    mov afterEsp,esp
    fnstsw afterFp
    fninit
    mov esp,hostEsp
    popad
    ret}}
static void Prepare(U32 n,bool frame,bool sidecar,bool wrap=false){
    memcpy(reinterpret_cast<void*>(code),kNotifyNative,sizeof(kNotifyNative));
    Check(NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),false));
    if(frame){NotifyApply(code,reinterpret_cast<UPtr>(&NotifyCopy),true);Check(NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),true));}
    Relocate(code,kNotifyCalls,sizeof(kNotifyCalls)/sizeof(kNotifyCalls[0]),kNotifyRoots,sizeof(kNotifyRoots)/sizeof(kNotifyRoots[0]));
    // Relocation changes the one CALL which is intentionally different in production.
    if(sidecar)Write32(code+97u,reinterpret_cast<UPtr>(&NotifyCopy)-(code+101u));
    memset(owner,0xCC,sizeof(owner));request.owner=reinterpret_cast<UPtr>(owner);request.count=n;haveSidecar=true;
    for(U32 i=0;i<150u;++i)request.ids[i]=kChapter7AllPositiveIds[i];
    for(U32 i=0;i<50u;++i)Write32(request.owner+i*4u,request.ids[i]);Write32(request.owner+0xC8u,n);
    memset(ringStorage,0xA5,sizeof(ringStorage));memset(message,0,sizeof(message));
    const UPtr ring=reinterpret_cast<UPtr>(ringStorage)+16u;ringOffset=wrap?ringCapacity-64u:0u;
    Write32(reinterpret_cast<UPtr>(recorder)+0x855Cu,ringCapacity);
    Write32(reinterpret_cast<UPtr>(recorder)+0x8560u,ring);
    Write32(reinterpret_cast<UPtr>(recorder)+0x8564u,ring+(wrap?2000u:0u));
    Write32(reinterpret_cast<UPtr>(recorder)+0x8568u,ring+ringOffset);
    memset(reinterpret_cast<void*>(stackTop-4096u),0x5A,8192u);
    Write32(reinterpret_cast<UPtr>(parent)+0x4E588u,1u);Write32(reinterpret_cast<UPtr>(parent)+0x4E9E0u,0u);
    countArg=n;observedBadReturn=0;repairOldReturn=!frame;entry=code;
    Check(FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(code),0x4000u));
}
static void CheckPacket(U32 n,bool validIds,bool oldOverflow){
    Invoke();Check(afterEsp==stackTop);
    Check(afterRegisters[0]==0x44444444 && afterRegisters[1]==0x55555555 && afterRegisters[2]==0x66666666 && afterRegisters[3]==0x77777777);
    Check(!(afterFp&0x3800u));
    packetBytes=24u+n*4u;Check(packetBytes<=sizeof(message));
    for(U32 i=0;i<packetBytes;++i)message[i]=ringStorage[16u+(ringOffset+i)%ringCapacity];
    Check(Read32(reinterpret_cast<UPtr>(message))==0x00010005u);
    Check(Read32(reinterpret_cast<UPtr>(message)+4u)==0x3FA00000u);
    Check(Read32(reinterpret_cast<UPtr>(message)+8u)==8u+n*4u);
    Check(Read32(reinterpret_cast<UPtr>(message)+16u)==0u && Read32(reinterpret_cast<UPtr>(message)+20u)==n);
    if(validIds)for(U32 i=0;i<n;++i)Check(Read32(reinterpret_cast<UPtr>(message)+24u+i*4u)==request.ids[i]);
    if(!validIds && !oldOverflow)Check(Read32(reinterpret_cast<UPtr>(message)+24u+50u*4u)==n);
    if(oldOverflow)Check(observedBadReturn==n);else Check(!observedBadReturn && Read32(stackTop+16u)==0x5A5A5A5Au);
    for(U32 i=0;i<16u;++i)Check(ringStorage[i]==0xA5 && ringStorage[16u+ringCapacity+i]==0xA5);
    const UPtr ring=reinterpret_cast<UPtr>(ringStorage)+16u;
    Check(Read32(reinterpret_cast<UPtr>(recorder)+0x8568u)==ring+(ringOffset+packetBytes)%ringCapacity);
}
extern "C" void __stdcall NotifyTestMain(){
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x4000u,0x3000u,0x40u));Check(code);
    memcpy(reinterpret_cast<void*>(code),kNotifyNative,sizeof(kNotifyNative));
    Check(NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),false));
    NotifyApply(code,reinterpret_cast<UPtr>(&NotifyCopy),true);Check(NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),true));
    reinterpret_cast<U8*>(code)[30]^=1;Check(!NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),true));reinterpret_cast<U8*>(code)[30]^=1;
    NotifyApply(code,reinterpret_cast<UPtr>(&NotifyCopy),false);Check(NotifyBytesMatch(code,reinterpret_cast<UPtr>(&NotifyCopy),false));
    const UPtr stack=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,0x04u));Check(stack);stackTop=stack+0x8000u;
    Write32(rootPointer+0x22Cu,reinterpret_cast<UPtr>(parent));Write32(reinterpret_cast<UPtr>(parent)+0x4E58Cu,reinterpret_cast<UPtr>(recorder));
    memcpy(reinterpret_cast<void*>(code+0x1000u),kRecordNative,sizeof(kRecordNative));
    memcpy(reinterpret_cast<void*>(code+0x2000u),kRingNative,sizeof(kRingNative));
    memcpy(reinterpret_cast<void*>(code+0x3000u),kReplayNative,sizeof(kReplayNative));
    Relocate(code+0x1000u,kRecordCalls,sizeof(kRecordCalls)/sizeof(kRecordCalls[0]),kRecordRoots,sizeof(kRecordRoots)/sizeof(kRecordRoots[0]));
    Relocate(code+0x2000u,kRingCalls,sizeof(kRingCalls)/sizeof(kRingCalls[0]),kRingRoots,sizeof(kRingRoots)/sizeof(kRingRoots[0]));
    Relocate(code+0x3000u,kReplayCalls,sizeof(kReplayCalls)/sizeof(kReplayCalls[0]),kReplayRoots,sizeof(kReplayRoots)/sizeof(kReplayRoots[0]));
    Write32(code+0x3030u,code+0x30D4u);for(U32 i=0;i<4u;++i){U32 native=Read32(code+0x30D4u+i*4u);Write32(code+0x30D4u+i*4u,code+0x3000u+native-0x754AC0u);}
    Prepare(18,false,false);CheckPacket(18,true,false);
    Prepare(50,false,false);CheckPacket(50,true,false);
    Prepare(51,false,false);CheckPacket(51,false,true);
    Prepare(150,false,false);CheckPacket(150,false,true);Say("native stock return overwrite reproduced:150\n");
    Prepare(150,true,false);CheckPacket(150,false,false);Say("frame-only repair still copies count as ID51\n");
    for(U32 n=0;n<=150u;++n){Prepare(n,true,true);CheckPacket(n,true,false);Check(!fatals && !locked);}
    Check(g_notifySidecar==100 && g_notifyPeak==150);Say("all counts0..150 native callback and writer passed\n");
    Prepare(150,true,true,true);CheckPacket(150,true,false);Check(lastSidecar && lastCount==150);
    U32 hash=2166136261u;for(U32 i=0;i<150u;++i)hash=(hash^request.ids[i])*16777619u;Check(hash==lastHash);
    typedef U8 (__thiscall* ReplayFn)(void*,const void*);
    replayCanApply=true;Check(reinterpret_cast<ReplayFn>(code+0x3000u)(parent,message)==1u);Check(replayCalls==1 && replayCount==150);
    replayCanApply=false;Check(reinterpret_cast<ReplayFn>(code+0x3000u)(parent,message)==0u);Check(replayCalls==1);
    Say("native ring wrap and replay150 ID round trip passed\n");
    for(U32 mode=0;mode<3u;++mode){Prepare(150,true,true);Write32(reinterpret_cast<UPtr>(parent)+0x4E588u,mode);if(mode==1)Write32(reinterpret_cast<UPtr>(parent)+0x4E9E0u,1);
        const long copies=g_notifyCopies;Invoke();Check(g_notifyCopies==copies);Check(Read32(reinterpret_cast<UPtr>(recorder)+0x8568u)==reinterpret_cast<UPtr>(ringStorage)+16u);}
    Prepare(150,true,true);U32 destination[151]={};memset(destination,0xCC,sizeof(destination));
    Check(!NotifyCopy(destination,owner,604u));Check(fatals==1 && destination[0]==0xCCCCCCCC);
    haveSidecar=false;Check(!NotifyCopy(destination,owner,600u));Check(fatals==2 && !locked);
    haveSidecar=true;request.count=149;Check(!NotifyCopy(destination,owner,600u));Check(fatals==3 && !locked);
    request.count=150;Write32(request.owner,0xDEAD);Check(!NotifyCopy(destination,owner,600u));Check(fatals==4 && !locked);
    Check(!NotifyCopy(destination,owner,601u));Check(fatals==5 && destination[0]==0xCCCCCCCC);
    Say("notify150 native checks passed\n");ExitProcess(0);
}

// Production reason core/bridges/installer in fake memory, never the game/ASI.
typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;
typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memcpy(void* out,const void* in,U32 size){for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=reinterpret_cast<const volatile U8*>(in)[i];return out;}
extern "C" __declspec(noinline) void* __cdecl memset(void* out,int v,U32 size){for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=static_cast<U8>(v);return out;}
static U32 checks=0;static void Check(bool v){++checks;if(!v)ExitProcess(checks);}
static U8 page0[4096]={},page1[4096]={},before0[4096]={},before1[4096]={};
static bool cold=true,identity=true,readable=true,slotsSignature=true;
static long g_audioSlotsStatus=1;static bool g_audioSlotsPatched=true;static U32 g_audioSlotsInstalled=9;
static UPtr Address(UPtr p) {
    if(p>=0x84C000u && p<0x84D000u)return reinterpret_cast<UPtr>(page0)+p-0x84C000u;
    if(p>=0xC54000u && p<0xC55000u)return reinterpret_cast<UPtr>(page1)+p-0xC54000u;
    ExitProcess(230u);return 0;
}
static bool Readable(UPtr,U32){return readable;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long v){const long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long v,long expected){const long old=*p;if(old==expected)*p=v;return old;}
static void CapacityMax(volatile long& p,U32 v){if(v>static_cast<U32>(p))p=static_cast<long>(v);}
static U32 g_capacityStart=0;static U32 GetTickCount(){return 12345u;}
static bool AudioSlotsCold(){return cold;}static bool AudioSlotsSignature(bool){return slotsSignature;}
static bool VerifyExecutableIdentity(){return identity;}
static const U32 kPageExecuteReadWrite=0x40u,kMemCommitReserve=0x3000u;
#define AudioReasonEntry AudioReasonMeasuredEntry
#include "../src/V29sAudioReasonCore.inc"
#undef AudioReasonEntry
extern "C" UPtr g_audioReasonTramps[10];
static U32 helperCalls=0,helperKind=0,helperDf=0;static U32 helperFrame[12]={};
extern "C" __declspec(noinline) void __cdecl AudioReasonEntry(U32 kind,const U32* frame) {
    ++helperCalls;helperKind=kind;
    for(U32 i=0;i<12u;++i)helperFrame[i]=frame[i];
    U32 flags=0;
    __asm {pushfd
        pop flags
        fninit
        fldz
        pxor xmm0,xmm0
        mov ecx,11223344h
        mov edx,55667788h
        clc}
    helperDf|=flags&0x400u;
}
#include "../src/V29sAudioReasonBridges.inc"
static U8 trampMemory[10][32]={};
static U32 allocCalls=0,protectCalls=0,flushCalls=0,failAlloc=0,failProtect=0,failProtect2=0,failFlush=0,mutate=0;
static void* VirtualAlloc(void*,U32,U32,U32){++allocCalls;return allocCalls==failAlloc?nullptr:trampMemory[(allocCalls-1u)%10u];}
static BOOL VirtualProtect(void*,U32,DWORD,DWORD* old){++protectCalls;*old=0x20u;if(protectCalls==1u && mutate==1u)cold=false;if(protectCalls==1u && mutate==2u)page0[0xC98u]=0x90u;return protectCalls!=failProtect && protectCalls!=failProtect2;}
static BOOL FlushInstructionCache(void*,const void*,U32){++flushCalls;return flushCalls!=failFlush;}
static void* GetCurrentProcess(){return nullptr;}
#include "../src/V29sAudioReasonPatch.inc"
static void Reset() {
    for(U32 i=0;i<4096u;++i)page0[i]=page1[i]=0xCCu;
    for(U32 i=0;i<10u;++i)for(U32 j=0;j<kAudioReasonSites[i].length;++j)*reinterpret_cast<U8*>(Address(kAudioReasonSites[i].address)+j)=kAudioReasonSites[i].bytes[j];
    for(U32 i=0;i<4096u;++i){before0[i]=page0[i];before1[i]=page1[i];}
    cold=identity=readable=slotsSignature=true;g_audioSlotsStatus=1;g_audioSlotsPatched=true;g_audioSlotsInstalled=9u;
    allocCalls=protectCalls=flushCalls=failAlloc=failProtect=failProtect2=failFlush=mutate=0;g_audioReasonStatus=0;g_audioReasonInstalled=0;
}
static bool Unchanged(){for(U32 i=0;i<4096u;++i)if(page0[i]!=before0[i] || page1[i]!=before1[i])return false;return true;}
static void Refused(){Check(!AudioReasonInstall());Check(g_audioReasonStatus==-1);}
static void InstallerTests() {
    Reset();Check(AudioReasonInstall());Check(g_audioReasonStatus==1 && g_audioReasonInstalled==10u);Check(AudioReasonSignature(true));Check(allocCalls==10u && protectCalls==4u && flushCalls==1u);
    for(U32 i=0;i<10u;++i) {
        const AudioReasonSite& s=kAudioReasonSites[i];const UPtr t=g_audioReasonTramps[i];
        for(U32 j=0;j<s.length;++j)Check(Read8(t+j)==s.bytes[j]);
        Check(Read8(t+s.length)==0xE9u && t+s.length+5u+Read32(t+s.length+1u)==Address(s.address)+s.length);
    }
    for(U32 offset=0;offset<4096u;++offset) {
        bool hook0=false,hook1=false;
        for(U32 i=0;i<10u;++i) {
            const AudioReasonSite& s=kAudioReasonSites[i];const U32 base=s.address&~0xFFFu;
            if(offset>=s.address-base && offset<s.address-base+s.length){if(base==0x84C000u)hook0=true;else hook1=true;}
        }
        if(!hook0)Check(page0[offset]==before0[offset]);if(!hook1)Check(page1[offset]==before1[offset]);
    }
    Check(Read8(g_audioReasonTramps[2]+6u)==16u); // Stolen compare retains16.
    Reset();g_audioSlotsStatus=-1;Refused();Check(Unchanged());
    Reset();g_audioSlotsPatched=false;Refused();Check(Unchanged());
    Reset();g_audioSlotsInstalled=8;Refused();Check(Unchanged());
    Reset();slotsSignature=false;Refused();Check(Unchanged());
    Reset();identity=false;Refused();Check(Unchanged());
    Reset();cold=false;Refused();Check(Unchanged());
    Reset();readable=false;Refused();Check(Unchanged());
    for(U32 i=0;i<10u;++i){Reset();*reinterpret_cast<U8*>(Address(kAudioReasonSites[i].address))=0x90u;Refused();Check(protectCalls==0u);}
    for(U32 i=1;i<=10u;++i){Reset();failAlloc=i;Refused();Check(Unchanged());}
    for(U32 i=1;i<=2u;++i){Reset();failProtect=i;Refused();Check(Unchanged());}
    Reset();mutate=1;Refused();Check(Unchanged());
    Reset();mutate=2;Refused();Check(Read8(Address(0xC5400Bu))==0x3Bu);
    Reset();failFlush=1;Refused();Check(Unchanged());
    Reset();failProtect=3;Refused();Check(Unchanged());
    Reset();failProtect=4;Refused();Check(Unchanged());
    Reset();failFlush=1;failProtect=5;Check(!AudioReasonInstall());Check(g_audioReasonStatus==-2);
    Reset();failFlush=1;failProtect=7;Check(!AudioReasonInstall());Check(g_audioReasonStatus==-2);
}
static U32 frame[64]={},object[92]={},info[3]={};
static void Stack(U32 offset,U32 value){frame[9u+offset/4u]=value;}
static void CoreTests() {
    Reset();const UPtr owner=reinterpret_cast<UPtr>(object);frame[6]=owner;
    AudioReasonMeasuredEntry(9u,frame);AudioReasonOwner* s=AudioReasonFindOwner(owner,false);Check(s && s->registrations==1);
    frame[1]=owner;Stack(0x24u,0x2CA33BDBu);Stack(0x20u,0x72DF8Fu);
    AudioReasonMeasuredEntry(2u,frame);Check(s->requests==1 && s->noneRequests==1 && !s->refused);
    frame[2]=0x2CA33BDBu;frame[7]=42u;AudioReasonMeasuredEntry(3u,frame);
    Check(s->accepted==1 && s->acceptedPeakProjected==1 && object[91]==0u);
    object[91]=16u;AudioReasonMeasuredEntry(2u,frame);Check(s->refused==1 && g_audioReasonFull==1 && s->peakObserved==16);
    object[91]=1u;frame[7]=0u;AudioReasonMeasuredEntry(3u,frame);Check(s->zero==1);
    Stack(0x34u,0x2CA33BDBu);Stack(0x38u,owner);Stack(0x30u,0xC54E03u);
    AudioReasonMeasuredEntry(0u,frame);Check(g_audioReasonLookupMiss==1 && s->lookupMiss==1);
    Stack(0x34u,0x38C61846u);AudioReasonMeasuredEntry(0u,frame);
    Check(g_audioReasonDetailCounts[0]==1 && g_audioReasonDetailCounts[1]==1);
    frame[7]=0x12345000u;AudioReasonMeasuredEntry(0u,frame);Check(g_audioReasonLookupHit==1 && s->lookupHit==1);
    Stack(0x30u,0x38C61846u);Stack(0x34u,owner);frame[7]=2u;AudioReasonMeasuredEntry(1u,frame);
    Check(AudioReasonFindEvent(0x38C61846u)->createFailed==1 && g_audioReasonDetails[2][0].a==0x38C61846u && g_audioReasonDetails[2][0].c==2u);
    Stack(4u,1u);Stack(8u,reinterpret_cast<UPtr>(info));info[0]=owner;info[2]=42u;object[12]=42u;
    AudioReasonMeasuredEntry(4u,frame);Check(g_audioReasonEndsDelivered==1);
    frame[6]=owner;AudioReasonMeasuredEntry(5u,frame);Check(s->ends==1 && !s->unmatched);
    info[2]=99u;AudioReasonMeasuredEntry(5u,frame);Check(s->unmatched==1 && g_audioReasonDetailCounts[5]==1);
    frame[6]=owner+0x2Cu;AudioReasonMeasuredEntry(6u,frame);Check(s->removes==1 && object[91]==1u);
    frame[6]=owner;AudioReasonMeasuredEntry(9u,frame);Check(s->registrations==2);
    const char menu[]="Menu_Move",none[]="none";
    Stack(0u,0x53BE90u);Stack(4u,reinterpret_cast<UPtr>(menu));AudioReasonMeasuredEntry(7u,frame);
    Check(g_audioReasonDetails[6][0].a==0x72DCBB24u && g_audioReasonDetails[6][0].e==1u);
    Stack(4u,reinterpret_cast<UPtr>(none));AudioReasonMeasuredEntry(8u,frame);Check(g_audioReasonDetails[6][1].a==0x2CA33BDBu);
    Stack(4u,0u);AudioReasonMeasuredEntry(7u,frame);Check(g_audioReasonDetails[6][2].e==4u);
    Stack(4u,reinterpret_cast<UPtr>(none));readable=false;AudioReasonMeasuredEntry(7u,frame);Check(g_audioReasonDetails[6][3].e==2u);readable=true;
    frame[7]=0;Stack(0x34u,0x2CA33BDBu);Stack(0x38u,owner);
    for(U32 i=0;i<100u;++i)AudioReasonMeasuredEntry(0u,frame);
    Check(g_audioReasonDetailCounts[0]==101 && g_audioReasonDetails[0][63].ready && !g_audioReasonDetails[0][64].ready);
    Stack(0x34u,0x38C61846u);for(U32 i=0;i<300u;++i)AudioReasonMeasuredEntry(0u,frame);
    Check(g_audioReasonDetailCounts[1]==301 && g_audioReasonDetails[1][255].ready);
    for(U32 i=1;i<700u;++i)AudioReasonFindEvent(i);Check(g_audioReasonEventOverflow>0);
    for(U32 i=1;i<150u;++i)AudioReasonFindOwner(0x800000u+i*16u,true);Check(g_audioReasonOwnerOverflow>0);
    object[91]=17u;frame[1]=owner;Stack(0x24u,0x38C61846u);AudioReasonMeasuredEntry(2u,frame);Check(g_audioReasonInvalid>0);
}
static __declspec(align(16)) U8 beforeFx[512]={},afterFx[512]={};
static U32 afterRegs[7]={},afterFlags=0,bridgeAddress=0;
extern "C" __declspec(naked) void CaptureTramp() {
    __asm {mov afterRegs[0],eax
        mov afterRegs[4],ecx
        mov afterRegs[8],edx
        mov afterRegs[12],ebx
        mov afterRegs[16],esi
        mov afterRegs[20],edi
        mov afterRegs[24],ebp
        pushfd
        pop afterFlags
        fxsave [afterFx]
        ret}
}
extern "C" __declspec(naked) void InvokeBridge() {
    __asm {pushad
        fninit
        fld1
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
        ret}
}
static void BridgeTests() {
    for(U32 i=0;i<10u;++i) {
        g_audioReasonTramps[i]=reinterpret_cast<UPtr>(&CaptureTramp);bridgeAddress=kAudioReasonSites[i].bridge;
        InvokeBridge();Check(helperCalls==i+1u && helperKind==i && !helperDf && (afterFlags&0x401u)==0x401u);
        Check(helperFrame[0]==0x66666666u && helperFrame[1]==0x55555555u && helperFrame[2]==0x77777777u);
        Check(helperFrame[4]==0x44444444u && helperFrame[5]==0x33333333u && helperFrame[6]==0x22222222u && helperFrame[7]==0x11111111u);
        Check((helperFrame[8]&0x401u)==0x401u && helperFrame[10]==0x12345678u && helperFrame[11]==0xABCDEF01u);
        for(U32 n=0;n<7u;++n)Check(afterRegs[n]==0x11111111u*(n+1u));
        for(U32 n=0;n<24u;++n)Check(beforeFx[n]==afterFx[n]);
        for(U32 n=0;n<8u;++n)for(U32 j=0;j<10u;++j)Check(beforeFx[32u+n*16u+j]==afterFx[32u+n*16u+j]);
        for(U32 n=160u;n<288u;++n)Check(beforeFx[n]==afterFx[n]);
    }
}
extern "C" void __stdcall AudioReasonTestMain() {
    InstallerTests();CoreTests();BridgeTests();
    const char message[]="audio reason checks passed\n";DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);ExitProcess(0u);
}

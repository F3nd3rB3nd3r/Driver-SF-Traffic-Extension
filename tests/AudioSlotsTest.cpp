// Exact production core, bridges and installer, with fake game memory/OS calls.
typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned int UPtr;
typedef unsigned long DWORD;
typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memcpy(void* out,const void* in,U32 size) {
    for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=reinterpret_cast<const volatile U8*>(in)[i];return out;
}
extern "C" __declspec(noinline) void* __cdecl memset(void* out,int value,U32 size) {
    for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=static_cast<U8>(value);return out;
}
static U32 checks=0;
static void Check(bool value){++checks;if(!value)ExitProcess(checks);}
static U8 page0[4096]={},page1[4096]={},backup0[4096]={},backup1[4096]={};
static U32 managers[2]={},coldIds[2]={};
static bool cold=true,identity=true,readable=true;
static UPtr Address(UPtr p) {
    if(p>=0xC50000u && p<0xC51000u)return reinterpret_cast<UPtr>(page0)+p-0xC50000u;
    if(p>=0xC6B000u && p<0xC6C000u)return reinterpret_cast<UPtr>(page1)+p-0xC6B000u;
    if(p==0x1D7184Cu)return reinterpret_cast<UPtr>(managers);
    if(p==0x1D71850u)return reinterpret_cast<UPtr>(managers+1);
    ExitProcess(230u);return 0;
}
static bool Readable(UPtr,U32){return readable;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long v){const long previous=*p;*p=v;return previous;}
static void CapacityMax(volatile long& p,U32 v){if(v>static_cast<U32>(p))p=static_cast<long>(v);}
static bool AudioDefaultCold(){return cold;}
static bool VerifyExecutableIdentity(){return identity;}
static long g_audioDefaultStatus=1,g_audioProbeStatus=1,g_capacityHooks=3,g_indexedStatus=1;
static bool g_indexedStartupVerified=true;
static const U32 kPageExecuteReadWrite=0x40u,kMemCommitReserve=0x3000u;
#define AudioSlotsEntry AudioSlotsMeasuredEntry
#include "../src/V29sAudioSlotsCore.inc"
#undef AudioSlotsEntry
extern "C" UPtr g_audioSlotsTramps[9];
static U32 helperCalls=0,helperKind=0,helperPointer=0,helperDf=0;
extern "C" __declspec(noinline) void __cdecl AudioSlotsEntry(U32 kind,UPtr pointer) {
    ++helperCalls;helperKind=kind;helperPointer=pointer;
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
#include "../src/V29sAudioSlotsBridges.inc"
static U8 trampMemory[9][32]={};
static U32 allocCalls=0,protectCalls=0,flushCalls=0,failAlloc=0,failProtect=0,failProtect2=0,failFlush=0,mutate=0;
static void* VirtualAlloc(void*,U32,U32,U32) {
    ++allocCalls;if(allocCalls==failAlloc)return nullptr;
    return trampMemory[(allocCalls-1u)%9u];
}
static BOOL VirtualProtect(void*,U32,DWORD,DWORD* old) {
    ++protectCalls;*old=0x20u;
    if(protectCalls==1u && mutate==1u)cold=false;
    if(protectCalls==1u && mutate==2u)page1[0x42Fu]=0x90u;
    return protectCalls!=failProtect && protectCalls!=failProtect2;
}
static BOOL FlushInstructionCache(void*,const void*,U32){++flushCalls;return flushCalls!=failFlush;}
static void* GetCurrentProcess(){return nullptr;}
#include "../src/V29sAudioSlotsPatch.inc"
static void Reset() {
    for(U32 i=0;i<4096u;++i)page0[i]=page1[i]=0xCCu;
    const U8 settings[5]={0xB9,0xFF,0,0,0},stores[6]={0x89,0x48,4,0x89,0x48,8};
    for(U32 i=0;i<5u;++i)page0[0xF0Cu+i]=settings[i];
    for(U32 i=0;i<6u;++i)page0[0xF17u+i]=stores[i];
    for(U32 i=0;i<9u;++i)for(U32 j=0;j<kAudioSlotsSites[i].length;++j)
        page1[kAudioSlotsSites[i].address-0xC6B000u+j]=kAudioSlotsSites[i].bytes[j];
    for(U32 i=0;i<4096u;++i){backup0[i]=page0[i];backup1[i]=page1[i];}
    managers[0]=managers[1]=0;cold=identity=readable=true;
    g_audioDefaultStatus=g_audioProbeStatus=g_indexedStatus=1;g_capacityHooks=3;g_indexedStartupVerified=true;
    allocCalls=protectCalls=flushCalls=failAlloc=failProtect=failProtect2=failFlush=mutate=0;
    g_audioSlotsStatus=0;g_audioSlotsInstalled=0;g_audioSlotsPatched=false;
}
static bool Unchanged() {
    for(U32 i=0;i<4096u;++i)if(page0[i]!=backup0[i] || page1[i]!=backup1[i])return false;
    return true;
}
static void Refused(){Check(!AudioSlotsInstall());Check(g_audioSlotsStatus==-1);Check(!g_audioSlotsPatched);}
static void InstallerTests() {
    Reset();Check(AudioSlotsInstall());Check(g_audioSlotsStatus==1 && g_audioSlotsPatched && g_audioSlotsInstalled==9u);
    Check(AudioSlotsSignature(true));Check(protectCalls==4u && flushCalls==1u && allocCalls==9u);
    for(U32 i=0;i<4096u;++i)if(i<0xF0Du || i>=0xF11u)Check(page0[i]==backup0[i]);
    for(U32 i=0;i<9u;++i) {
        const AudioSlotsSite& s=kAudioSlotsSites[i];const UPtr t=g_audioSlotsTramps[i];
        for(U32 j=0;j<s.length;++j)Check(Read8(t+j)==s.bytes[j]);
        Check(Read8(t+s.length)==0xE9u);
        Check(t+s.length+5u+Read32(t+s.length+1u)==Address(s.address)+s.length);
    }
    Reset();g_audioDefaultStatus=-1;Refused();Check(Unchanged());
    Reset();g_audioProbeStatus=-1;Refused();Check(Unchanged());
    Reset();g_capacityHooks=-1;Refused();Check(Unchanged());
    Reset();g_indexedStatus=2;Refused();Check(Unchanged());
    Reset();g_indexedStartupVerified=false;Refused();Check(Unchanged());
    Reset();identity=false;Refused();Check(Unchanged());
    Reset();cold=false;Refused();Check(Unchanged());
    Reset();managers[0]=1;Refused();Check(Unchanged());
    Reset();managers[1]=1;Refused();Check(Unchanged());
    Reset();readable=false;Refused();Check(Unchanged());
    Reset();page0[0xF0Du]=0;Refused();Check(protectCalls==0u);
    Reset();page0[0xF1Cu]=7;Refused();Check(protectCalls==0u);
    for(U32 i=0;i<9u;++i){Reset();page1[kAudioSlotsSites[i].address-0xC6B000u]=0x90u;Refused();Check(protectCalls==0u);}
    for(U32 i=1;i<=9u;++i){Reset();failAlloc=i;Refused();Check(Unchanged());}
    for(U32 i=1;i<=2u;++i){Reset();failProtect=i;Refused();Check(Unchanged());}
    Reset();mutate=1;Refused();Check(Unchanged());
    Reset();mutate=2;Refused();Check(Read32(Address(0xC50F0Du))==255u);
    Reset();failFlush=1;Refused();Check(Unchanged());Check(AudioSlotsSignature(false));
    Reset();failProtect=3;Refused();Check(Unchanged());Check(AudioSlotsSignature(false));
    Reset();failProtect=4;Refused();Check(Unchanged());Check(AudioSlotsSignature(false));
    Reset();failFlush=1;failProtect=5;Check(!AudioSlotsInstall());Check(g_audioSlotsStatus==-2);
    Reset();failFlush=1;failProtect=7;Check(!AudioSlotsInstall());Check(g_audioSlotsStatus==-2);
}
static U32 ma[8]={},mb[5]={},record[32]={};
static void CoreTests() {
    managers[0]=reinterpret_cast<UPtr>(ma);ma[1]=2048u;
    ma[2]=0x10000u;ma[3]=ma[2]+300u*4u;ma[4]=2048u;
    ma[5]=0x20000u;ma[6]=ma[5]+2048u*4u;ma[7]=2048u;
    mb[1]=0x30000u;mb[2]=mb[1]+2048u*4u;mb[3]=mb[4]=2048u;
    AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma+2));
    Check(g_audioSlotsLists[0].attempts==1 && g_audioSlotsLists[0].peakBefore==300 && !g_audioSlotsLists[0].ceiling);
    AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma+5));
    Check(g_audioSlotsLists[1].ceiling==1 && g_audioSlotsLists[1].peakBefore==2048);
    AudioSlotsMeasuredEntry(1u,reinterpret_cast<UPtr>(mb));Check(g_audioSlotsLists[2].ceiling==1);
    AudioSlotsMeasuredEntry(2u,reinterpret_cast<UPtr>(ma+2));Check(g_audioSlotsLists[0].peakInserted==300);
    AudioSlotsMeasuredEntry(3u,reinterpret_cast<UPtr>(mb));Check(g_audioSlotsLists[2].peakInserted==2048);
    AudioSlotsMeasuredEntry(4u,reinterpret_cast<UPtr>(ma+5));Check(g_audioSlotsLists[1].reuses==1);
    AudioSlotsMeasuredEntry(7u,reinterpret_cast<UPtr>(ma+2));Check(g_audioSlotsLists[0].updateCount==300);
    ma[3]=ma[2];AudioSlotsMeasuredEntry(7u,reinterpret_cast<UPtr>(ma+2));
    Check(g_audioSlotsLists[0].updateCount==0 && g_audioSlotsLists[0].updatePeak==300);
    AudioSlotsMeasuredEntry(8u,reinterpret_cast<UPtr>(mb));Check(g_audioSlotsLists[2].updateCount==2048);
    record[9]=0x40000u;record[10]=record[9]+7u*4u;
    AudioSlotsMeasuredEntry(5u,reinterpret_cast<UPtr>(record));Check(!g_audioSlotsUserFull[0]);
    record[10]+=4u;AudioSlotsMeasuredEntry(5u,reinterpret_cast<UPtr>(record));
    Check(g_audioSlotsUserFull[0]==1 && g_audioSlotsUserPeak[0]==8);
    reinterpret_cast<U8*>(record)[0x39u]=254u;AudioSlotsMeasuredEntry(6u,reinterpret_cast<UPtr>(record));
    Check(!g_audioSlotsUserFull[1]);reinterpret_cast<U8*>(record)[0x39u]=255u;
    AudioSlotsMeasuredEntry(6u,reinterpret_cast<UPtr>(record));Check(g_audioSlotsUserFull[1]==1 && g_audioSlotsUserPeak[1]==255);
    ma[3]=ma[2]-4u;AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma+2));Check(g_audioSlotsLists[0].invalid==1);
    ma[3]=ma[2]+1u;AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma+2));Check(g_audioSlotsLists[0].invalid==2);
    ma[3]=ma[2]+2049u*4u;AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma+2));Check(g_audioSlotsLists[0].invalid==3);
    AudioSlotsMeasuredEntry(0u,reinterpret_cast<UPtr>(ma));Check(g_audioSlotsInvalid==1);
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
        std
        stc
        call dword ptr [bridgeAddress]
        cld
        fninit
        popad
        ret}
}
static void BridgeTests() {
    for(U32 i=0;i<9u;++i) {
        g_audioSlotsTramps[i]=reinterpret_cast<UPtr>(&CaptureTramp);bridgeAddress=kAudioSlotsSites[i].bridge;
        InvokeBridge();Check(helperCalls==i+1u && helperKind==i);
        Check(helperPointer==(i==5u?0x44444444u:0x66666666u));
        Check(!helperDf && (afterFlags&0x401u)==0x401u);
        for(U32 n=0;n<7u;++n)Check(afterRegs[n]==0x11111111u*(n+1u));
        // FXSAVE reserved bytes are undefined. Compare defined header, ten-byte
        // x87 values, and the eight XMM registers in this 32-bit process.
        for(U32 n=0;n<24u;++n)Check(beforeFx[n]==afterFx[n]);
        for(U32 n=0;n<8u;++n)for(U32 j=0;j<10u;++j)Check(beforeFx[32u+n*16u+j]==afterFx[32u+n*16u+j]);
        for(U32 n=160u;n<288u;++n)Check(beforeFx[n]==afterFx[n]);
    }
}
extern "C" void __stdcall AudioSlotsTestMain() {
    InstallerTests();CoreTests();BridgeTests();
    const char message[]="audio slots checks passed\n";DWORD written=0;
    WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);ExitProcess(0);
}

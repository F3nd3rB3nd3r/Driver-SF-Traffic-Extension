// Production operand installer against fake game memory and fault-injected OS
// calls, plus an isolated copy of the native pressure predicate with mocked stats.
typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned int UPtr;
typedef unsigned long DWORD;
typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) int __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" int _fltused=0;
extern "C" __declspec(noinline) void* __cdecl memcpy(void* destination,const void* source,U32 count) {
    volatile U8* out=reinterpret_cast<volatile U8*>(destination);
    const volatile U8* in=reinterpret_cast<const volatile U8*>(source);
    for(U32 i=0;i<count;++i)out[i]=in[i];return destination;
}
static const U32 kPageExecuteReadWrite=0x40u;
static U8 g_init[256]={},g_before[256]={};
static U32 g_defaultId=0xFFFFFFFFu,g_constant=0x3F666666u,g_state[68]={};
static bool g_identity=true,g_readable=true;
static long g_capacityHooks=3,g_indexedStatus=1;
static bool g_indexedStockVerified=true,g_indexedStartupVerified=true;
static U32 g_protectCalls=0,g_flushCalls=0,g_failProtect=0,g_failProtect2=0,g_failFlush=0,g_mutateCold=0,g_mutateCode=0;
static U32 g_checks=0;
static void Check(bool ok) {++g_checks;if(!ok)ExitProcess(g_checks);}
static UPtr Address(UPtr p) {return p;}
static U8* Translate(UPtr p) {
    if(p>=0x84A600u && p<0x84A700u)return g_init+p-0x84A600u;
    if(p>=0x1D7183Cu && p<0x1D7194Cu)return reinterpret_cast<U8*>(g_state)+p-0x1D7183Cu;
    if(p>=0xFDD3B8u && p<0xFDD3BCu)return reinterpret_cast<U8*>(&g_defaultId)+p-0xFDD3B8u;
    if(p>=0xDD6CD4u && p<0xDD6CD8u)return reinterpret_cast<U8*>(&g_constant)+p-0xDD6CD4u;
    ExitProcess(240u);return nullptr;
}
static bool Readable(UPtr,U32) {return g_readable;}
static U8 Read8(UPtr p) {return *Translate(p);}
static U32 Read32(UPtr p) {return *reinterpret_cast<U32*>(Translate(p));}
static void Write32(UPtr p,U32 v) {*reinterpret_cast<U32*>(Translate(p))=v;}
static bool VerifyExecutableIdentity() {return g_identity;}
static void* GetCurrentProcess() {return nullptr;}
static BOOL FakeProtect(void*,U32,DWORD,DWORD* old) {
    ++g_protectCalls;*old=0x20u;
    if(g_protectCalls==1u && g_mutateCold)g_defaultId=5u;
    if(g_protectCalls==1u && g_mutateCode)g_init[0x4Du]=0x90u;
    return g_protectCalls!=g_failProtect && g_protectCalls!=g_failProtect2;
}
static BOOL FakeFlush(void*,const void*,U32) {++g_flushCalls;return g_flushCalls!=g_failFlush;}
#define VirtualProtect FakeProtect
#define FlushInstructionCache FakeFlush
#include "../src/V29sAudioDefaultPoolPatch.inc"
#undef VirtualProtect
#undef FlushInstructionCache
static void Reset() {
    for(U32 i=0;i<256u;++i)*reinterpret_cast<volatile U8*>(g_init+i)=0xCCu;
    for(U32 i=0;i<68u;++i)*reinterpret_cast<volatile U32*>(g_state+i)=0;
    const U8 load[8]={0xF3,0x0F,0x10,0x05,0xD4,0x6C,0xDD,0};
    const U8 stores[10]={0xF3,0x0F,0x11,0x45,0xE8,0xF3,0x0F,0x11,0x45,0x44};
    for(U32 i=0;i<7u;++i)g_init[0x4Du+i]=kAudioDefaultStockInstruction[i];
    for(U32 i=0;i<8u;++i)g_init[0x2Eu+i]=load[i];
    for(U32 i=0;i<10u;++i)g_init[0x54u+i]=stores[i];
    g_init[0x83u]=0x68u;Write32(0x0084A684u,0x0042B800u);
    for(U32 i=0;i<256u;++i)g_before[i]=g_init[i];
    g_defaultId=0xFFFFFFFFu;g_constant=0x3F666666u;g_identity=true;g_readable=true;
    g_capacityHooks=3;g_indexedStatus=1;g_indexedStockVerified=true;g_indexedStartupVerified=true;
    g_protectCalls=g_flushCalls=g_failProtect=g_failProtect2=g_failFlush=g_mutateCold=g_mutateCode=0;
    g_audioDefaultStatus=0;g_audioDefaultPatched=false;
}
static void Refused() {Check(!AudioDefaultPoolInstall());Check(g_audioDefaultStatus==-1);Check(!g_audioDefaultPatched);Check(AudioDefaultInstructionMatches(kAudioDefaultStockBytes));}
static U32 g_used=0,g_total=0;
extern "C" void __cdecl FakeStats(U32,U32* out) {out[0]=g_total;out[1]=g_used;}
static UPtr g_predicate=0;
static const float kOne=1.0f,kUnsigned=4294967296.0f;
#include "AudioPressurePredicateBytes.inc"
extern "C" __declspec(naked) U32 __cdecl NativePressure(U32,float) {
    __asm {
        mov ecx,[esp+4]
        push dword ptr [esp+8]
        call dword ptr [g_predicate]
        add esp,4
        and eax,0FFh
        ret
    }
}
extern "C" void __stdcall AudioDefaultTestMain() {
    Reset();Check(AudioDefaultPoolInstall());Check(g_audioDefaultStatus==1 && g_audioDefaultPatched);
    Check(AudioDefaultInstructionMatches(kAudioDefaultTestBytes));Check(g_protectCalls==2u && g_flushCalls==1u);
    bool unchanged=true;for(U32 i=0;i<256u;++i)if((i<0x50u || i>=0x54u) && g_before[i]!=g_init[i])unchanged=false;
    Check(unchanged);
    Reset();g_capacityHooks=-1;Refused();
    Reset();g_indexedStatus=2;Refused();
    Reset();g_indexedStockVerified=false;Refused();
    Reset();g_indexedStartupVerified=false;Refused();
    Reset();g_identity=false;Refused();
    Reset();g_defaultId=5u;Refused();
    Reset();g_state[0]=1u;Refused();
    Reset();g_state[3]=1u;Refused();
    Reset();g_state[(0x1D71940u-0x1D7183Cu)/4u]=1u;Refused();
    Reset();g_constant=0x3F800000u;Refused();
    Reset();g_init[0x54u]=0x90u;Refused();
    Reset();g_init[0x4Du]=0x90u;Check(!AudioDefaultPoolInstall());Check(!g_audioDefaultPatched && g_protectCalls==0u);
    Reset();g_readable=false;Check(!AudioDefaultPoolInstall());Check(g_protectCalls==0u);
    Reset();g_failProtect=1u;Refused();
    Reset();g_failFlush=1u;Refused();Check(g_protectCalls==4u && g_flushCalls==2u);
    Reset();g_failProtect=2u;Refused();Check(g_protectCalls==4u);
    Reset();g_mutateCold=1u;Refused();Check(g_flushCalls==0u);
    Reset();g_mutateCode=1u;Check(!AudioDefaultPoolInstall());Check(!g_audioDefaultPatched && g_init[0x4Du]==0x90u && g_flushCalls==0u);
    Reset();g_failFlush=1u;g_failProtect=3u;
    Check(!AudioDefaultPoolInstall());Check(g_audioDefaultStatus==-2 && g_audioDefaultPatched);
    Check(AudioDefaultInstructionMatches(kAudioDefaultTestBytes));
#if defined(DSF_V29S_AUDIO_DEFAULT_64M)
    Reset();Write32(0x0084A684u,0x04000000u);Refused();
    Reset();g_init[0x83u]=0x90u;Refused();
#endif

    // Execute only the 108-byte native predicate relocated into isolated memory.
    // All native absolute data/call references are rebound to test-owned objects.
    U8* code=reinterpret_cast<U8*>(VirtualAlloc(nullptr,sizeof(kNativePressureBytes),0x3000u,0x40u));Check(code!=nullptr);
    for(U32 i=0;i<sizeof(kNativePressureBytes);++i)code[i]=kNativePressureBytes[i];
    *reinterpret_cast<U32*>(code+4u)=reinterpret_cast<UPtr>(&kOne);
    *reinterpret_cast<U32*>(code+0x18u)=reinterpret_cast<UPtr>(&FakeStats)-(reinterpret_cast<UPtr>(code)+0x1Cu);
    *reinterpret_cast<U32*>(code+0x35u)=reinterpret_cast<UPtr>(&kUnsigned);
    *reinterpret_cast<U32*>(code+0x45u)=reinterpret_cast<UPtr>(&kUnsigned);
    Check(FlushInstructionCache(reinterpret_cast<void*>(0xFFFFFFFFu),code,sizeof(kNativePressureBytes))!=0);
    g_predicate=reinterpret_cast<UPtr>(code);
    const U32 cases[][2]={{132448u,5868820u},{3142676u,5868820u},{5325936u,5868820u},
        {5361532u,5868820u},{5361532u,kAudioDefaultTestBytes-3180u},{8u,10u},{9u,10u},{10u,10u},
        {0u,0u},{0u,100u},{0xFFFFFFFFu,0xFFFFFFFFu},{0xE6666600u,0xFFFFFFFFu}};
    for(U32 i=0;i<sizeof(cases)/sizeof(cases[0]);++i) {
        g_used=cases[i][0];g_total=cases[i][1];
        Check((NativePressure(5u,0.9f)!=0u)==AudioDefaultAbove90(g_used,g_total));
    }
    g_total=5868820u;g_used=5325936u;Check(NativePressure(5u,0.9f)==1u);
    Check(NativePressure(5u,1.0f)==0u);
    g_total=kAudioDefaultTestBytes-3180u;Check(NativePressure(5u,0.9f)==0u);
    const char* message="audio default-pool checks passed\n";DWORD written=0;
    WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,33u,&written,nullptr);
    ExitProcess(0u);
}

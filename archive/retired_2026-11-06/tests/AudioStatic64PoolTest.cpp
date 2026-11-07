// Execute production installers against fake image/OS state; no game or ASI load.
typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned int UPtr;
typedef unsigned long DWORD;
typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) int __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memcpy(void* destination,const void* source,U32 count) {
    volatile U8* out=reinterpret_cast<volatile U8*>(destination);
    const volatile U8* in=reinterpret_cast<const volatile U8*>(source);
    for(U32 i=0;i<count;++i)out[i]=in[i];return destination;
}
static const U32 kPageExecuteReadWrite=0x40u,kRootPtr=0x00FE7228u;
static U8 g_init[256]={},g_before[256]={};
static U32 g_defaultId=0xFFFFFFFFu,g_constant=0x3F666666u,g_state[68]={},g_meta[3]={},g_root=0;
static bool g_identity=true,g_readable=true;
static long g_capacityHooks=3,g_indexedStatus=1,g_audioProbeStatus=1;
static bool g_indexedStockVerified=true,g_indexedStartupVerified=true;
static U32 g_protectCalls=0,g_flushCalls=0,g_failProtect=0,g_failFlush=0,g_mutateCold=0,g_mutateCode=0;
static U32 g_checks=0,g_scenarios=0;
static void Check(bool ok) {++g_checks;if(!ok)ExitProcess(g_checks);}
static UPtr Address(UPtr p) {return p;}
static U8* Translate(UPtr p) {
    if(p>=0x84A600u && p<0x84A700u)return g_init+p-0x84A600u;
    if(p>=0x1D7183Cu && p<0x1D7194Cu)return reinterpret_cast<U8*>(g_state)+p-0x1D7183Cu;
    if(p>=0x1D72388u && p<0x1D72394u)return reinterpret_cast<U8*>(g_meta)+p-0x1D72388u;
    if(p>=kRootPtr && p<kRootPtr+4u)return reinterpret_cast<U8*>(&g_root)+p-kRootPtr;
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
    if(g_protectCalls==1u && g_mutateCold)g_root=1u;
    if(g_protectCalls==1u && g_mutateCode)g_init[0x83u]=0x90u;
    return g_protectCalls!=g_failProtect;
}
static BOOL FakeFlush(void*,const void*,U32) {++g_flushCalls;return g_flushCalls!=g_failFlush;}
#define VirtualProtect FakeProtect
#define FlushInstructionCache FakeFlush
#include "../src/V29sAudioDefaultPoolPatch.inc"
#include "../src/V29sAudioStatic64Patch.inc"
#undef VirtualProtect
#undef FlushInstructionCache
static void Reset() {
    ++g_scenarios;
    for(U32 i=0;i<256u;++i)*reinterpret_cast<volatile U8*>(g_init+i)=0xCCu;
    for(U32 i=0;i<68u;++i)*reinterpret_cast<volatile U32*>(g_state+i)=0u;
    for(U32 i=0;i<3u;++i)g_meta[i]=0u;
    const U8 load[8]={0xF3,0x0F,0x10,0x05,0xD4,0x6C,0xDD,0};
    const U8 stores[10]={0xF3,0x0F,0x11,0x45,0xE8,0xF3,0x0F,0x11,0x45,0x44};
    const U8 context[8]={0x68,0x00,0x04,0x00,0x00,0x83,0xCB,0xFF};
    const U8 create[5]={0xE8,0xF5,0xF6,0x44,0};
    for(U32 i=0;i<7u;++i)g_init[0x4Du+i]=kAudioDefaultStockInstruction[i];
    for(U32 i=0;i<8u;++i)g_init[0x2Eu+i]=load[i];
    for(U32 i=0;i<10u;++i)g_init[0x54u+i]=stores[i];
    for(U32 i=0;i<8u;++i)g_init[0x7Bu+i]=context[i];
    for(U32 i=0;i<5u;++i)g_init[0x96u+i]=create[i];
    g_init[0x83u]=0x68u;Write32(kAudioStaticSizeSite+1u,kAudioStaticStockBytes);
    g_defaultId=0xFFFFFFFFu;g_constant=0x3F666666u;g_identity=true;g_readable=true;g_root=0;
    g_capacityHooks=3;g_indexedStatus=1;g_audioProbeStatus=1;g_indexedStockVerified=true;g_indexedStartupVerified=true;
    g_protectCalls=g_flushCalls=g_failProtect=g_failFlush=g_mutateCold=g_mutateCode=0;
    g_audioDefaultStatus=0;g_audioDefaultPatched=false;g_audioStaticStatus=0;g_audioStaticPatched=false;
    Check(AudioDefaultPoolInstall()); // Real production default16 baseline first.
    g_protectCalls=g_flushCalls=0;
    for(U32 i=0;i<256u;++i)g_before[i]=g_init[i];
}
static void Refused() {
    Check(!AudioStaticPoolInstall());Check(g_audioStaticStatus==-1 && !g_audioStaticPatched);
    Check(AudioStaticInstructionMatches(kAudioStaticStockBytes));
}
extern "C" void __stdcall AudioStaticTestMain() {
    Reset();Check(AudioStaticPoolInstall());Check(g_audioStaticStatus==1 && g_audioStaticPatched);
    Check(AudioStaticInstructionMatches(0x4000000u));Check(AudioDefaultInstructionMatches(0x1000000u));
    Check(g_protectCalls==2u && g_flushCalls==1u);
    bool unchanged=true;for(U32 i=0;i<256u;++i)if((i<0x84u || i>=0x88u) && g_before[i]!=g_init[i])unchanged=false;
    Check(unchanged);
    Reset();g_audioDefaultStatus=-1;Refused();
    Reset();g_audioDefaultPatched=false;Refused();
    Reset();g_audioProbeStatus=-1;Refused();
    Reset();g_capacityHooks=-1;Refused();
    Reset();g_indexedStatus=2;Refused();
    Reset();g_indexedStockVerified=false;Refused();
    Reset();g_indexedStartupVerified=false;Refused();
    Reset();g_identity=false;Refused();
    Reset();Write32(kAudioDefaultSizeSite+3u,kAudioDefaultStockBytes);Refused();
    Reset();g_root=1u;Refused();
    Reset();g_meta[0]=20u;Refused();
    Reset();g_meta[1]=1u;Refused();
    Reset();g_meta[2]=0x12340000u;Refused();
    Reset();g_defaultId=5u;Refused();
    Reset();g_state[0]=1u;Refused();
    Reset();g_state[3]=1u;Refused();
    Reset();g_state[(0x1D71940u-0x1D7183Cu)/4u]=1u;Refused();
    Reset();g_init[0x83u]=0x90u;Check(!AudioStaticPoolInstall());Check(!g_audioStaticPatched && g_protectCalls==0u);
    Reset();Write32(kAudioStaticSizeSite+1u,0x1000000u);Check(!AudioStaticPoolInstall());Check(g_protectCalls==0u);
    Reset();g_init[0x7Bu]=0x90u;Refused();
    Reset();g_init[0x96u]=0x90u;Refused();
    Reset();g_failProtect=1u;Refused();
    Reset();g_failFlush=1u;Refused();Check(g_protectCalls==4u && g_flushCalls==2u);
    Reset();g_failProtect=2u;Refused();Check(g_protectCalls==4u);
    Reset();g_mutateCold=1u;Refused();Check(g_flushCalls==0u);
    Reset();g_mutateCode=1u;Check(!AudioStaticPoolInstall());Check(!g_audioStaticPatched && g_init[0x83u]==0x90u && g_flushCalls==0u);
    Reset();g_readable=false;Check(!AudioStaticPoolInstall());Check(!g_audioStaticPatched && g_protectCalls==0u);
    Reset();g_failFlush=1u;g_failProtect=3u;
    Check(!AudioStaticPoolInstall());Check(g_audioStaticStatus==-2 && g_audioStaticPatched);
    Check(AudioStaticInstructionMatches(kAudioStaticTestBytes));Check(AudioDefaultInstructionMatches(kAudioDefaultTestBytes));
    Check(g_scenarios==29u);
    const char message[]="static64 scenarios passed: 29\n";DWORD written=0;
    WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);
    ExitProcess(0u);
}

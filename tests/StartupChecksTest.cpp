// Isolated fake-image test. Never loads an ASI or opens/executes Driver.exe.
#define DSF_V29S_INDEXED_262144 1
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned int UPtr;
typedef unsigned long DWORD;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) int __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
constexpr UPtr kPreferredBase=0x00400000u,kGraphicsCtor=0x00BAD960u,kHandlerInit=0x005A1420u;
constexpr U32 kTimestamp=0x4EC0D92Bu,kSizeOfImage=0x019C0000u;
static UPtr g_base=kPreferredBase,g_unreadable=0u;
static const UPtr kWindows[]={kPreferredBase,0x005A0000u,0x00BA0000u,0x00C00000u};
static U8 g_memory[4][0x10000u]={};
static UPtr Address(UPtr preferred) {return g_base+(preferred-kPreferredBase);}
static U8* Translate(UPtr address) {
    for(U32 i=0u;i<4u;++i)if(address>=kWindows[i] && address-kWindows[i]<0x10000u)
        return g_memory[i]+(address-kWindows[i]);
    ExitProcess(249u);return nullptr;
}
static U8 Read8(UPtr address) {return *Translate(address);}
static U16 Read16(UPtr address) {return *reinterpret_cast<U16*>(Translate(address));}
static U32 Read32(UPtr address) {return *reinterpret_cast<U32*>(Translate(address));}
static void Write32(UPtr address,U32 value) {*reinterpret_cast<U32*>(Translate(address))=value;}
static void Write16(UPtr address,U16 value) {*reinterpret_cast<U16*>(Translate(address))=value;}
static void Write8(UPtr address,U8 value) {*Translate(address)=value;}
static bool Readable(UPtr address,U32 size) {
    if(address+size<address || (g_unreadable>=address && g_unreadable<address+size))return false;
    for(U32 i=0u;i<4u;++i)if(address>=kWindows[i] && address-kWindows[i]+size<=0x10000u)return true;
    return false;
}
#include "../src/V29sStartupChecks.inc"

static U32 g_checks=0u;
static void Check(bool result) {++g_checks;if(!result)ExitProcess(g_checks);}
extern "C" void __stdcall StartupGuardTestMain() {
    // Synthetic addresses map into local arrays, independent of host ASLR/DLLs.
    const UPtr nt=g_base+0x100u,bridge=0x00C00100u,trampoline=0x00C00000u;
    Write8(g_base,'M');Write8(g_base+1u,'Z');Write32(g_base+0x3Cu,0x100u);
    Write32(nt,0x4550u);Write16(nt+4u,0x14Cu);Write32(nt+8u,kTimestamp);
    Write16(nt+24u,0x10Bu);Write32(nt+0x50u,kSizeOfImage);
    const U8 ctor[]={0x81,0xEC,0x5C,0x02,0x00,0x00};
    const U8 stock[]={0xA1,0x28,0x72,0xFE,0x00};
    for(U32 i=0u;i<6u;++i)Write8(kGraphicsCtor+i,ctor[i]);
    for(U32 i=0u;i<5u;++i) {Write8(kHandlerInit+i,stock[i]);Write8(trampoline+i,stock[i]);}
    Write8(trampoline+5u,0xE9u);Write32(trampoline+6u,(kHandlerInit+5u)-(trampoline+10u));
    // Real installation order: stock preflight -> owned hook -> late validation.
    const bool stockVerified=VerifyExecutable();Check(stockVerified);
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,0));
    Write8(kHandlerInit,0xE9u);Write32(kHandlerInit+1u,bridge-(kHandlerInit+5u));
    Check(!VerifyExecutable()); // Reproduces why the old installer rejected itself.
    Check(VerifyExecutableIdentity());
    Check(VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Check(!VerifyIndexedStartup(false,bridge,trampoline,trampoline,1));
    Write32(kHandlerInit+1u,bridge+1u-(kHandlerInit+5u));
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write32(kHandlerInit+1u,bridge-(kHandlerInit+5u));
    Write8(kHandlerInit,0xE8u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write8(kHandlerInit,0xE9u);
    Write8(trampoline,0x90u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write8(trampoline,stock[0]);
    Write8(trampoline+5u,0xE8u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write8(trampoline+5u,0xE9u);
    Write32(trampoline+6u,(kHandlerInit+6u)-(trampoline+10u));
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write32(trampoline+6u,(kHandlerInit+5u)-(trampoline+10u));
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline+1u,1));
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,0));
    Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,2));
    Check(!VerifyIndexedStartup(stockVerified,0u,trampoline,trampoline,1));
    Check(!VerifyIndexedStartup(stockVerified,bridge,0u,0u,1));
    g_unreadable=kHandlerInit;Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    g_unreadable=trampoline;Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    g_unreadable=bridge;Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    g_unreadable=0u;
    Write32(nt+8u,kTimestamp+1u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    Write32(nt+8u,kTimestamp);
    Write16(nt+4u,0x8664u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write16(nt+4u,0x14Cu);
    Write8(g_base,'X');Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write8(g_base,'M');
    Write32(nt,0u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write32(nt,0x4550u);
    Write16(nt+24u,0x20Bu);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write16(nt+24u,0x10Bu);
    Write32(nt+0x50u,kSizeOfImage+1u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write32(nt+0x50u,kSizeOfImage);
    Write8(kGraphicsCtor,0x90u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write8(kGraphicsCtor,ctor[0]);
    g_base=kPreferredBase+0x10000u;Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));g_base=kPreferredBase;
    Write32(g_base+0x3Cu,0x900u);Check(!VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));Write32(g_base+0x3Cu,0x100u);
    Check(VerifyIndexedStartup(stockVerified,bridge,trampoline,trampoline,1));
    char message[]="startup guard checks passed: 00\r\n";
    message[29]=static_cast<char>('0'+g_checks/10u);message[30]=static_cast<char>('0'+g_checks%10u);
    DWORD written=0u;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);
    ExitProcess(0u);
}

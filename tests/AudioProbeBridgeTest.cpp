// Exercises the exact production naked bridges against synthetic native frames.
// Never loads an ASI or executes/opens Driver.exe.
typedef unsigned int U32;
typedef unsigned int UPtr;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long);
extern "C" __declspec(dllimport) int __stdcall WriteFile(void*,const void*,unsigned long,unsigned long*,void*);
extern "C" UPtr g_audioProbeTramps[6]={};
extern "C" volatile long g_audioProbePostNormal=0;
static U32 g_args[5]={},g_calls=0,g_kind=0,g_flags=0,g_ecx=0,g_edx=0,g_helperDf=0;
static __declspec(align(16)) unsigned char g_before[512]={},g_after[512]={};
static U32 job[8]={},owner[1024]={};
static void Clobber() {
    U32 flags=0;
    __asm {pushfd
        pop flags
        fninit
        fldz
        pxor xmm0,xmm0
        mov ecx,11223344h
        mov edx,55667788h
        clc
    }
    g_helperDf|=flags&0x400u;
}
extern "C" __declspec(noinline) void __cdecl AudioProbeBankEntry(U32 bank,U32 result,U32 pool,UPtr cookie) {
    ++g_calls;g_kind=0;g_args[0]=bank;g_args[1]=result;g_args[2]=pool;g_args[3]=cookie;Clobber();
}
extern "C" __declspec(noinline) void __cdecl AudioProbeEmitterEntry(UPtr owner) {
    ++g_calls;g_kind=1;g_args[0]=owner;g_args[1]=*reinterpret_cast<U32*>(owner+0xEA0u);Clobber();
}
extern "C" __declspec(noinline) void __cdecl AudioProbeAllocEntry(U32 kind,U32 pool,U32 bytes,U32 caller,U32 alignment) {
    ++g_calls;g_kind=2+kind;g_args[0]=pool;g_args[1]=bytes;g_args[2]=caller;g_args[3]=alignment;Clobber();
}
extern "C" __declspec(noinline) void __cdecl AudioProbePostEntry(U32 early,U32 event,U32 object,U32 caller) {
    ++g_calls;g_kind=4+early;g_args[0]=event;g_args[1]=object;g_args[2]=caller;Clobber();
}
#include "../src/V29sAudioProbeBridges.inc"

// Record machine state at trampoline entry before the stolen native epilogue.
#define CAPTURE_STATE __asm {pushfd} __asm {pop g_flags} __asm {mov g_ecx,ecx} __asm {mov g_edx,edx} __asm {fxsave [g_after]}
#define SET_STATE __asm {mov ecx,0A1B2C3D4h} __asm {mov edx,0B2C3D4E5h} __asm {std} __asm {stc}
extern "C" __declspec(naked) U32 BankNative(U32,U32,U32,UPtr) {
    __asm {mov eax,[esp+16]}
    SET_STATE
    __asm {jmp AudioProbeBankBridge}
}
extern "C" __declspec(naked) void BankTramp() {
    CAPTURE_STATE
    __asm {mov dword ptr [eax+14h],4
        ret}
}
extern "C" __declspec(naked) U32 EmitterNative(UPtr) {
    __asm {sub esp,170h
        push esi
        mov esi,[esp+178h]}
    SET_STATE
    __asm {jmp AudioProbeEmitterBridge}
}
extern "C" __declspec(naked) void EmitterTramp() {
    CAPTURE_STATE
    __asm {cmp dword ptr [esi+0EA0h],5Ah
        setb al
        movzx eax,al
        pop esi
        add esp,170h
        ret}
}
extern "C" __declspec(naked) U32 MallocNative(U32,U32,U32,U32) {
    __asm {push edi
        push ebx
        push esi
        mov edi,[esp+28]}
    SET_STATE
    __asm {jmp AudioProbeMallocBridge}
}
extern "C" __declspec(naked) U32 MalignNative(U32,U32,U32,U32) {
    __asm {push edi
        push ebx
        push esi
        mov edi,[esp+28]}
    SET_STATE
    __asm {jmp AudioProbeMalignBridge}
}
extern "C" __declspec(naked) void AllocTramp() {
    CAPTURE_STATE
    __asm {pop esi
        pop ebx
        mov eax,edi
        pop edi
        ret}
}
extern "C" __declspec(naked) U32 PostNative(U32,U32,U32) {
    __asm {sub esp,10h
        push ebx
        push edi
        push esi
        mov esi,[esp+40]}
    SET_STATE
    __asm {jmp AudioProbePostBridge}
}
extern "C" __declspec(naked) void PostTramp() {
    CAPTURE_STATE
    __asm {mov eax,esi
        pop esi
        pop edi
        pop ebx
        add esp,10h
        ret}
}
extern "C" __declspec(naked) U32 EarlyNative(U32,U32) {
    __asm {sub esp,10h
        push ebx
        push edi
        xor eax,eax}
    SET_STATE
    __asm {jmp AudioProbePostEarlyBridge}
}
extern "C" __declspec(naked) void EarlyTramp() {
    CAPTURE_STATE
    __asm {pop edi
        pop ebx
        add esp,10h
        ret}
}
static U32 g_checks=0;
static void Check(bool ok) {++g_checks;if(!ok)ExitProcess(g_checks);}
static void Begin() {
    g_calls=0;g_flags=0;g_ecx=0;g_edx=0;
    __asm {fninit
        fld1
        pcmpeqd xmm0,xmm0
        fxsave [g_before]}
}
static void End(U32 calls,U32 kind) {
    // Native DF deliberately starts set; each helper must see it cleared, and
    // trampoline must see original DF/CF restored, including fast paths.
    __asm {cld}
    Check(g_calls==calls);if(calls)Check(g_kind==kind);
    Check((g_flags&0x401u)==0x401u && g_helperDf==0u);
    Check(g_ecx==0xA1B2C3D4u && g_edx==0xB2C3D4E5u);
    bool same=true;for(U32 i=0;i<512u;++i)if(g_before[i]!=g_after[i])same=false;
    Check(same);
}
extern "C" void __stdcall AudioBridgeTestMain() {
    g_audioProbeTramps[0]=reinterpret_cast<UPtr>(&BankTramp);
    g_audioProbeTramps[1]=reinterpret_cast<UPtr>(&EmitterTramp);
    g_audioProbeTramps[2]=reinterpret_cast<UPtr>(&AllocTramp);
    g_audioProbeTramps[3]=reinterpret_cast<UPtr>(&AllocTramp);
    g_audioProbeTramps[4]=reinterpret_cast<UPtr>(&PostTramp);
    g_audioProbeTramps[5]=reinterpret_cast<UPtr>(&EarlyTramp);
    const UPtr cookie=reinterpret_cast<UPtr>(job),a=reinterpret_cast<UPtr>(owner);
    Begin();BankNative(0x1234u,0x34u,7u,cookie);End(1u,0u);
    Check(g_args[0]==0x1234u && g_args[1]==0x34u && g_args[2]==7u && g_args[3]==cookie && job[5]==4u);
    owner[0xEA0u/4u]=89u;Begin();U32 result=EmitterNative(a);End(1u,1u);
    Check(result==1u && g_args[0]==a && g_args[1]==89u);
    owner[0xEA0u/4u]=90u;Begin();result=EmitterNative(a);End(1u,1u);Check(result==0u && g_args[1]==90u);
    Begin();result=MallocNative(3u,0x3456u,0x999u,0u);End(1u,2u);
    Check(result==0u && g_args[0]==3u && g_args[1]==0x3456u && g_args[2]!=0u && g_args[3]==0u);
    Begin();result=MallocNative(3u,0x3456u,0x999u,0x12345678u);End(0u,2u);Check(result==0x12345678u);
    Begin();result=MalignNative(4u,0x4567u,64u,0u);End(1u,3u);
    Check(result==0u && g_args[0]==4u && g_args[1]==0x4567u && g_args[2]!=0u && g_args[3]==64u);
    Begin();result=MalignNative(4u,0x4567u,64u,0x22334455u);End(0u,3u);Check(result==0x22334455u);
    Begin();result=PostNative(0x556677u,0x778899u,0u);End(1u,4u);
    Check(result==0u && g_args[0]==0x556677u && g_args[1]==0x778899u && g_args[2]!=0u);
    Begin();result=PostNative(0x556677u,0x778899u,0x112233u);End(0u,4u);Check(result==0x112233u && g_audioProbePostNormal==2);
    Begin();result=EarlyNative(0x667788u,0x8899AAu);End(1u,5u);
    Check(result==0u && g_args[0]==0x667788u && g_args[1]==0x8899AAu && g_args[2]!=0u);
    const char message[]="audio bridge checks passed\n";unsigned long written=0;
    WriteFile(GetStdHandle(static_cast<unsigned long>(-11)),message,sizeof(message)-1u,&written,nullptr);
    ExitProcess(0u);
}

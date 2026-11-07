// Isolated native map code + production core, detours and installer. No ASI/game.
typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;
typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
static void* RealAllocate(U32 bytes){return VirtualAlloc(nullptr,bytes,0x3000u,0x40u);}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* out,const void* in,U32 size){for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=reinterpret_cast<const volatile U8*>(in)[i];return out;}
extern "C" __declspec(noinline) void* __cdecl memset(void* out,int v,U32 size){for(U32 i=0;i<size;++i)reinterpret_cast<volatile U8*>(out)[i]=static_cast<U8>(v);return out;}
static U32 checks=0;static void Check(bool v){++checks;if(!v)ExitProcess(checks);}
static UPtr pages[5]={},trampArea=0;
static const U32 pageVas[5]={0x553000u,0x556000u,0x557000u,0x7D7000u,0xDE3000u};
static UPtr Address(UPtr p){for(U32 i=0;i<5u;++i)if(p>=pageVas[i] && p<pageVas[i]+4096u)return pages[i]+p-pageVas[i];ExitProcess(240u);return 0;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool readable=true,cold=true,identity=true,parent=true;
static bool Readable(UPtr,U32){return readable;}
static bool AudioSlotsCold(){return cold;}static bool AudioReasonSignature(bool){return parent;}
static bool VerifyExecutableIdentity(){return identity;}static long g_audioReasonStatus=1;
static const U32 kPageExecuteReadWrite=0x40u,kMemCommitReserve=0x3000u;
static U32 g_capacityStart=0;static U32 GetTickCount(){return 12345u;}
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long v){const long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long v,long expected){const long old=*p;if(old==expected)*p=v;return old;}
static void CapacityMax(volatile long& p,U32 v){if(v>static_cast<U32>(p))p=static_cast<long>(v);}
#define AudioModelEntry AudioModelMeasuredEntry
#include "../src/V29sAudioModelRegistryCore.inc"
#undef AudioModelEntry
extern "C" UPtr g_audioModelTramps[8];
static bool nativeMode=false;
static U32 helperCalls=0,helperKind=0,helperDf=0,frameValues[12]={},captureTarget=0;
extern "C" __declspec(noinline) void __cdecl AudioModelEntry(U32 kind,U32* frame) {
    if(nativeMode){AudioModelMeasuredEntry(kind,frame);return;}
    ++helperCalls;helperKind=kind;for(U32 i=0;i<12u;++i)frameValues[i]=frame[i];
    if(kind==0u){frame[3]=captureTarget;frame[8]=AudioModelCompareFlags(192u,192u,frame[8]);}
    if(kind==5u){frame[3]=captureTarget;frame[8]=AudioModelTestFlags(0u,frame[8]);}
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
#include "../src/V29sAudioModelRegistryBridges.inc"
static __declspec(align(16)) U8 nodeStorage[8u*193u*16u]={};
static U32 allocCalls=0,protectCalls=0,flushCalls=0,failAlloc=0,failProtect=0,failProtect2=0,failFlush=0,mutate=0;
static void* TestAlloc(void*,U32,U32,U32){++allocCalls;if(allocCalls==failAlloc)return nullptr;return reinterpret_cast<void*>(allocCalls==1u?reinterpret_cast<UPtr>(nodeStorage):trampArea+(allocCalls-2u)*32u);}
static BOOL VirtualProtect(void*,U32,DWORD,DWORD* old){++protectCalls;*old=0x20u;if(protectCalls==1u && mutate==1u)cold=false;if(protectCalls==1u && mutate==2u)*reinterpret_cast<U8*>(Address(0x556E30u))=0x90u;return protectCalls!=failProtect && protectCalls!=failProtect2;}
static BOOL FlushInstructionCache(void*,const void*,U32){++flushCalls;return flushCalls!=failFlush;}
static void* GetCurrentProcess(){return nullptr;}
#define VirtualAlloc TestAlloc
#include "../src/V29sAudioModelRegistryPatch.inc"
#undef VirtualAlloc
#include "AudioModelNativeBytes.inc"
static U8 before[2][4096]={};
static void Reset() {
    for(U32 p=0;p<5u;++p)memset(reinterpret_cast<void*>(pages[p]),0xCC,4096u);
    for(U32 i=0;i<kNativeBlockCount;++i)memcpy(reinterpret_cast<void*>(Address(kNativeBlocks[i].address)),kNativeBlocks[i].bytes,kNativeBlocks[i].size);
    // Rebase only the isolated constructor's vtable immediate and the one call
    // crossing from the map's code page to the generic insertion code page.
    Write32(Address(0x556EE4u),Address(0xDE3824u));
    Write32(Address(0x556F9Cu),Address(0x7D7270u)-(Address(0x556F9Bu)+5u));
    for(U32 p=0;p<2u;++p)memcpy(before[p],reinterpret_cast<void*>(pages[p]),4096u);
    readable=cold=identity=parent=true;g_audioReasonStatus=1;
    allocCalls=protectCalls=flushCalls=failAlloc=failProtect=failProtect2=failFlush=mutate=0;
    g_audioModelStorage=0;g_audioModelStatus=0;g_audioModelInstalled=0;
}
static bool Unchanged(){for(U32 p=0;p<2u;++p)for(U32 i=0;i<4096u;++i)if(Read8(pages[p]+i)!=before[p][i])return false;return true;}
static void Refused(){Check(!AudioModelInstall());Check(g_audioModelStatus==-1);}
static void InstallerTests() {
    Reset();Check(AudioModelInstall());Check(g_audioModelStatus==1 && g_audioModelInstalled==8u);Check(AudioModelSignature(true));Check(allocCalls==7u && protectCalls==4u && flushCalls==1u);
    for(U32 i=0;i<8u;++i)if(i!=0u && i!=5u) {
        const AudioModelSite& s=kAudioModelSites[i];UPtr t=g_audioModelTramps[i];
        for(U32 j=0;j<s.length;++j)Check(Read8(t+j)==s.bytes[j]);
        Check(Read8(t+s.length)==0xE9u && t+s.length+5u+Read32(t+s.length+1u)==Address(s.address)+s.length);
    }
    for(U32 p=0;p<2u;++p)for(U32 offset=0;offset<4096u;++offset) {
        bool hook=false;for(U32 i=0;i<8u;++i)if(kAudioModelSites[i].address>=pageVas[p] && kAudioModelSites[i].address<pageVas[p]+4096u) {
            const U32 at=kAudioModelSites[i].address-pageVas[p];if(offset>=at && offset<at+kAudioModelSites[i].length)hook=true;
        }
        if(!hook)Check(Read8(pages[p]+offset)==before[p][offset]);
    }
    Reset();g_audioReasonStatus=-1;Refused();Check(Unchanged());
    Reset();parent=false;Refused();Check(Unchanged());
    Reset();identity=false;Refused();Check(Unchanged());
    Reset();cold=false;Refused();Check(Unchanged());
    Reset();readable=false;Refused();Check(Unchanged());
    Reset();Write32(Address(0x557001u),0x1000u);Refused();Check(Unchanged());
    for(U32 i=0;i<8u;++i){Reset();*reinterpret_cast<U8*>(Address(kAudioModelSites[i].address))=0x90u;Refused();Check(protectCalls==0u);}
    for(U32 i=1;i<=7u;++i){Reset();failAlloc=i;Refused();Check(Unchanged());}
    for(U32 i=1;i<=2u;++i){Reset();failProtect=i;Refused();Check(Unchanged());}
    Reset();mutate=1;Refused();Check(Unchanged());
    Reset();mutate=2;Refused();Check(Read8(Address(0x553860u))==0x56u);
    Reset();failFlush=1;Refused();Check(Unchanged());
    Reset();failProtect=3;Refused();Check(Unchanged());
    Reset();failProtect=4;Refused();Check(Unchanged());
    Reset();failFlush=1;failProtect=5;Check(!AudioModelInstall());Check(g_audioModelStatus==-2);
    Reset();failFlush=1;failProtect=7;Check(!AudioModelInstall());Check(g_audioModelStatus==-2);
}
static UPtr nativeCtor=0,nativeRegister=0,nativeLookup=0,nativeRemove=0;
extern "C" __declspec(naked) void __cdecl Construct(UPtr){__asm {mov ecx,[esp+4]
    call dword ptr [nativeCtor]
    ret}}
extern "C" __declspec(naked) void __cdecl Register(UPtr,U32,U32){__asm {mov ecx,[esp+4]
    push dword ptr [esp+12]
    push dword ptr [esp+12]
    call dword ptr [nativeRegister]
    ret}}
extern "C" __declspec(naked) U32 __cdecl Lookup(UPtr,U32){__asm {mov ecx,[esp+4]
    push dword ptr [esp+8]
    call dword ptr [nativeLookup]
    ret}}
extern "C" __declspec(naked) void __cdecl Remove(UPtr,U32){__asm {mov ecx,[esp+4]
    push dword ptr [esp+8]
    call dword ptr [nativeRemove]
    ret}}
struct GuardedProvider{U32 before[4],object[188],after[4];};
static GuardedProvider providers[2]={};
static void NativeMapTests() {
    Reset();Check(AudioModelInstall());nativeMode=true;
    nativeCtor=Address(0x556EE0u);nativeRegister=Address(0x556E30u);nativeLookup=Address(0x556E90u);nativeRemove=Address(0x556E50u);
    memset(providers,0xA5,sizeof(providers));UPtr owner=reinterpret_cast<UPtr>(providers[0].object);Construct(owner);Write32(owner+0x2ECu,0xA11CE001u);
    for(U32 i=0;i<192u;++i){Register(owner,0x10000000u+i*64u,i*7u+3u);Check(Read32(owner+12u)==i+1u);}
    AudioModelInstance* r=AudioModelFindInstance(owner,false);Check(r && r->state==1 && r->stores==192 && r->peak==192);
    Check(g_audioModelAttachments==1 && r->wouldStockFull==147 && !r->full);
    for(U32 i=0;i<192u;++i)Check(Lookup(owner,i*7u+3u)==0x10000000u+i*64u);
    Register(owner,0xDEAD0001u,99999u);Check(Read32(owner+12u)==192u && Lookup(owner,99999u)==0 && r->full==1);
    Register(owner,0xDEAD0002u,3u);Check(Lookup(owner,3u)==0x10000000u && r->full==2);
    for(U32 i=0;i<3u;++i)Remove(owner,i*7u+3u);Check(Read32(owner+12u)==189u);
    Register(owner,0xABCDEF01u,24u);Check(Read32(owner+12u)==189u && Lookup(owner,24u)==0xABCDEF01u); // Existing-key update below full.
    for(U32 i=0;i<3u;++i)Register(owner,0x20000000u+i,10001u+i);Check(Read32(owner+12u)==192u);
    for(U32 i=0;i<3u;++i)Check(Lookup(owner,10001u+i)==0x20000000u+i);
    for(U32 i=3;i<192u;++i)Remove(owner,i*7u+3u);
    for(U32 i=0;i<3u;++i)Remove(owner,10001u+i);
    Check(Read32(owner+12u)==0 && Read32(owner+0x2E0u)==owner+0x2E0u && Read32(owner+0x2E4u)==owner+0x2E0u);
    Check(Read32(owner+0x2ECu)==0xA11CE001u && !g_audioModelInvalid);
    for(U32 i=0;i<4u;++i){Check(providers[0].before[i]==0xA5A5A5A5u && providers[0].after[i]==0xA5A5A5A5u);Check(Read32(AudioModelBuffer(r)+192u*16u+i*4u)==kAudioModelGuard);}
    Construct(owner);Register(owner,0x11111111u,7u);Check(r->generations==2 && Lookup(owner,7u)==0x11111111u);
    // Deliberately fill a second native object before external storage exists.
    // Later arming must retain stock45 and refuse live migration.
    const UPtr second=reinterpret_cast<UPtr>(providers[1].object);Construct(second);const UPtr savedStorage=g_audioModelStorage;g_audioModelStorage=0;
    for(U32 i=0;i<45u;++i)Register(second,0x30000000u+i,i+1u);
    Check(Read32(second+12u)==45u);g_audioModelStorage=savedStorage;
    Register(second,0x44444444u,46u);Check(Read32(second+12u)==45u && Lookup(second,46u)==0);
    AudioModelInstance* late=AudioModelFindInstance(second,false);Check(late && late->state==-1 && g_audioModelAttachRefused==1);
    Check(!g_audioModelInvalid);nativeMode=false;
}
static U32 fakeFrame[64]={},vehicle[400]={},controller[200]={};
static void Stack(U32 offset,U32 v){fakeFrame[9u+offset/4u]=v;}
static void SetupTests() {
    const UPtr vp=reinterpret_cast<UPtr>(vehicle),cp=reinterpret_cast<UPtr>(controller);vehicle[0x5ACu/4u]=4001u;
    *reinterpret_cast<U8*>(vp+0x5E7u)=1;controller[0xB4u/4u]=vp;controller[4]=1u;
    fakeFrame[6]=cp;Stack(4u,vp);Stack(8u,0u);AudioModelMeasuredEntry(4u,fakeFrame);Check(AudioModelFindKey(4001u)->setup==1);
    fakeFrame[1]=cp;Stack(8u,vp);fakeFrame[7]=0;fakeFrame[8]=0x401u;AudioModelMeasuredEntry(5u,fakeFrame);
    Check(fakeFrame[3]==Address(0x553AD5u) && (fakeFrame[8]&0x40u));
    fakeFrame[7]=1;AudioModelMeasuredEntry(5u,fakeFrame);Check(fakeFrame[3]==Address(0x553920u) && !(fakeFrame[8]&0x40u));
    AudioModelMeasuredEntry(6u,fakeFrame);AudioModelMeasuredEntry(7u,fakeFrame);Check(AudioModelFindKey(4001u)->setupOk==1 && AudioModelFindKey(4001u)->setupFail==1);
    const UPtr owner=reinterpret_cast<UPtr>(providers[0].object);fakeFrame[0]=owner;fakeFrame[7]=owner+0x2E0u;
    Stack(0x14u,1u);Stack(0x10u,Address(0x5538EFu));Stack(4u,cp);AudioModelMeasuredEntry(3u,fakeFrame);
    Check(AudioModelFindKey(4001u)->fallbackMiss==1);
    fakeFrame[7]=AudioModelBuffer(AudioModelFindInstance(owner,false));Write32(fakeFrame[7]+12u,0x12345678u);AudioModelMeasuredEntry(3u,fakeFrame);
    Check(AudioModelFindKey(4001u)->fallbackHit==1);
}
static __declspec(align(16)) U8 beforeFx[512]={},afterFx[512]={};
static U32 afterRegs[7]={},afterFlags=0,bridgeAddress=0;
extern "C" __declspec(naked) void CaptureTramp(){__asm {mov afterRegs[0],eax
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
extern "C" __declspec(naked) void InvokeBridge(){__asm {pushad
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
    ret}}
static void BridgeTests(){captureTarget=reinterpret_cast<UPtr>(&CaptureTramp);
    for(U32 i=0;i<8u;++i){g_audioModelTramps[i]=captureTarget;bridgeAddress=kAudioModelSites[i].bridge;InvokeBridge();
        Check(helperCalls==i+1u && helperKind==i && !helperDf);
        Check(frameValues[10]==0x12345678u && frameValues[11]==0xABCDEF01u);
        Check((afterFlags&0x400u)!=0u);
        if(i==0u)Check((afterFlags&0x8D5u)==(AudioModelCompareFlags(192u,192u,0)&0x8D5u));
        else if(i==5u)Check((afterFlags&0x8C5u)==(AudioModelTestFlags(0u,0)&0x8C5u));
        else Check((afterFlags&1u)==1u);
        for(U32 n=0;n<7u;++n)Check(afterRegs[n]==0x11111111u*(n+1u));
        for(U32 n=0;n<24u;++n)Check(beforeFx[n]==afterFx[n]);
        for(U32 n=0;n<8u;++n)for(U32 j=0;j<10u;++j)Check(beforeFx[32u+n*16u+j]==afterFx[32u+n*16u+j]);
        for(U32 n=160u;n<288u;++n)Check(beforeFx[n]==afterFx[n]);
    }
}
extern "C" void __stdcall AudioModelTestMain(){
    for(U32 i=0;i<5u;++i){pages[i]=reinterpret_cast<UPtr>(RealAllocate(4096u));Check(pages[i]!=0u);}trampArea=reinterpret_cast<UPtr>(RealAllocate(4096u));Check(trampArea!=0u);
    InstallerTests();NativeMapTests();SetupTests();BridgeTests();
    const char message[]="audio model registry checks passed\n";DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),message,sizeof(message)-1u,&written,nullptr);ExitProcess(0u);
}

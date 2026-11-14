typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" int _fltused=0;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall VirtualProtect(void*,U32,DWORD,DWORD*);
extern "C" __declspec(dllimport) U32 __stdcall VirtualQuery(const void*,void*,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentProcess();
extern "C" __declspec(dllimport) DWORD __stdcall GetTickCount();
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("regmap fixture failure line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
struct MemoryBasicInformation32 {void* base;void* allocationBase;U32 allocationProtect,regionSize,state,protect,type;};
static const U32 kCommitted=0x1000u,kPageGuard=0x100u,kPageNoAccess=1u;
bool Readable(UPtr address, U32 bytes) {
    if (!address || bytes > 0x100000u || address + bytes < address) return false;
    MemoryBasicInformation32 m = {};
    if (VirtualQuery(reinterpret_cast<void*>(address), &m, sizeof(m)) < sizeof(m)) return false;
    if (m.state != kCommitted || (m.protect & kPageGuard) ||
        (m.protect & 0xffu) == kPageNoAccess) return false;
    const UPtr end = reinterpret_cast<UPtr>(m.base) + m.regionSize;
    return end >= address && address + bytes <= end;
}
static const U32 kMemCommitReserve=0x3000u,kPageReadWrite=4u,kPageExecuteReadWrite=0x40u;
static UPtr nativeCode=0,g_pkg0=0;static long DSF150_Status=1;static U32 singleton=0,g_capacityStart=0;
static bool failAllocation=false,failProtect=false,failFlush=false,failCommitFlush=false,allowFatal=false;static U32 legacyCode=0;
static UPtr Address(U32 p){if(p==0x144BCFCu)return reinterpret_cast<UPtr>(&singleton);if(p==0xE395D0u)return p;return nativeCode+p-0x860000u;}
static bool VerifyExecutableIdentity(){return true;}
// Inject failure only into the real production install/allocation calls.
static void* Allocate(void* p,U32 n,U32 type,U32 protection){if(failAllocation)return nullptr;return VirtualAlloc(p,n,type,protection);}
static BOOL Protect(void* p,U32 n,DWORD protection,DWORD* old){if(failProtect){failProtect=false;return 0;}return VirtualProtect(p,n,protection,old);}
static BOOL Flush(void* p,const void* at,U32 n){if(failFlush || (failCommitFlush && reinterpret_cast<UPtr>(at)==Address(0x86B000u))){failFlush=failCommitFlush=false;return 0;}return FlushInstructionCache(p,at,n);}
static char lastLog[1000]={};
static void CapacityWrite(const char* s){U32 i=0;for(;s[i] && i<999u;++i)lastLog[i]=s[i];lastLog[i]=0;}
static void AppendText(char*& p,const char* s,const char* end){while(*s && p<end)*p++=*s++;}
static void AppendNumber(char*& p,long value,const char* end){char s[12]={};U32 n=0,v=static_cast<U32>(value);do{s[n++]=static_cast<char>('0'+v%10u);v/=10u;}while(v);while(n && p<end)*p++=s[--n];}
static void DiagFAppendKeyU(char*& p,const char* key,U32 v,const char* end){AppendText(p,key,end);AppendNumber(p,static_cast<long>(v),end);}
static void DiagFAppendKeyHex(char*& p,const char* key,U32 v,const char* end){AppendText(p,key,end);const char* digits="0123456789ABCDEF";for(U32 i=0;i<8u && p<end;++i)*p++=digits[(v>>(28u-i*4u))&15u];}
static void LegacyExit(U32 code,const char*,UPtr,UPtr,U32,U32){legacyCode=code;if(!allowFatal){Say(lastLog);ExitProcess(2);}}
#define VirtualAlloc Allocate
#define VirtualProtect Protect
#define FlushInstructionCache Flush
#include "../src/V29sRegMap.inc"
#include "../src/V29sRegMapRuntime.inc"
#undef VirtualAlloc
#undef VirtualProtect
#undef FlushInstructionCache
static bool Contains(const char* s,const char* needle){for(;*s;++s){U32 n=0;while(needle[n] && needle[n]==s[n])++n;if(!needle[n])return true;}return false;}
static UPtr owner=0,payload=0;static U32 key=0,constructorServices=0,destructorServices=0,deleteServices=0;
static void __cdecl CtorService(){++constructorServices;}
static void __cdecl DtorService(){++destructorServices;}
static void __cdecl DeleteService(void*){++deleteServices;}
static void Jump(UPtr at,UPtr target){*reinterpret_cast<U8*>(at)=0xE9u;Write32(at+1u,target-at-5u);}
static void Copy(U32 at,const U8* bytes,U32 n){memcpy(reinterpret_cast<void*>(Address(at)),bytes,n);}
static void FlipCode(U32 at){DWORD old=0,unused=0;Check(VirtualProtect(reinterpret_cast<void*>(Address(at)),1u,0x40u,&old));*reinterpret_cast<U8*>(Address(at))^=1u;Check(VirtualProtect(reinterpret_cast<void*>(Address(at)),1u,old,&unused));}
static U32 PageProtection(U32 at){MemoryBasicInformation32 m={};Check(VirtualQuery(reinterpret_cast<void*>(Address(at)),&m,sizeof(m))==sizeof(m));return m.protect;}
static void Construct(){typedef UPtr (__thiscall* Fn)(void*);Check(reinterpret_cast<Fn>(Address(0x86BFA0u))(reinterpret_cast<void*>(owner))==owner);}
static void Destroy(){typedef UPtr (__thiscall* Fn)(void*,U32);Check(reinterpret_cast<Fn>(Address(0x86C040u))(reinterpret_cast<void*>(owner),1u)==owner);}
static UPtr Map(){return owner+0x254u;}
static UPtr Insert(U32 id){typedef UPtr (__thiscall* Fn)(void*,const U32*);return reinterpret_cast<Fn>(Address(0x86BE10u))(reinterpret_cast<void*>(Map()),&id);}
static U32 Remove(U32 id){typedef U32 (__thiscall* Fn)(void*,const U32*);return reinterpret_cast<Fn>(Address(0x86BE90u))(reinterpret_cast<void*>(Map()),&id);}
static void Query(U32 id,U32 count){typedef void (__thiscall* Fn)(void*,UPtr*,const U32*);UPtr range[2]={};reinterpret_cast<Fn>(Address(0x86BDC0u))(reinterpret_cast<void*>(Map()),range,&id);Check((range[0]!=range[1])==(count!=0u));if(count)Check(Read32(range[0]+8u)==id);}
static void CheckList(U32 count){
    UPtr sentinel=Map()+0x444u,p=Read32(sentinel+4u),previous=sentinel;U32 n=0,last=0;
    while(p!=sentinel){Check(++n<=count);Check(Read32(p)==previous);Check(n==1u || Read32(p+8u)>last);last=Read32(p+8u);previous=p;p=Read32(p+4u);}
    Check(n==count && Read32(sentinel)==previous && Read32(Map()+8u)==count);
}
static void Fill(U32 n){for(U32 i=0;i<n;++i){UPtr p=Insert(i+1u);Check(p);Write32(p,0x12340000u+i);}CheckList(n);}
static void Fault(UPtr map,const char* expected){RegMapDetail d={};Check(!RegMapPrepare(map,d) && Contains(d.reason,expected));Check(!g_regMapLock);}
static UPtr hostEsp=0,stackTop=0,afterEsp=0;static U32 afterRegs[4]={};static unsigned short fpStatus=0;static U32 xmmAfter[4]={},abiMode=0;
extern "C" __declspec(naked) void AbiInsert(){__asm {
    pushad
    mov hostEsp,esp
    mov esp,stackTop
    fninit
    fld1
    pcmpeqd xmm0,xmm0
    mov ebp,77777777h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,owner
    mov eax,nativeCode
    cmp abiMode,0
    jne abi_constructor
    add ecx,254h
    push offset key
    add eax,0BE10h
    jmp abi_call
abi_constructor:
    add eax,0BFA0h
    cmp abiMode,1
    je abi_call
    sub eax,0BFA0h
    add eax,0C040h
    push 1
abi_call:
    call eax
    mov payload,eax
    mov afterRegs,ebx
    mov afterRegs+4,esi
    mov afterRegs+8,edi
    mov afterRegs+12,ebp
    mov afterEsp,esp
    fnstsw fpStatus
    movups xmmAfter,xmm0
    fninit
    mov esp,hostEsp
    popad
    ret
}}
static void Abi(){Check(afterEsp==stackTop && afterRegs[0]==0x44444444u && afterRegs[1]==0x55555555u && afterRegs[2]==0x66666666u && afterRegs[3]==0x77777777u && (fpStatus&0x3800u)==0x3800u);if(!abiMode)for(U32 i=0;i<4u;++i)Check(xmmAfter[i]==0xFFFFFFFFu);}
extern "C" void __stdcall RegMapTestMain(){
    nativeCode=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x400000u,0x3000u,0x40u));Check(nativeCode);
    Copy(0x86BFA0u,kRegMapCtorNative,sizeof(kRegMapCtorNative));Copy(0x86BE10u,kRegMapInsertNative,sizeof(kRegMapInsertNative));
    Copy(0x86BDC0u,kRegMapLookupNative,sizeof(kRegMapLookupNative));Copy(0x86BE90u,kRegMapRemoveNative,sizeof(kRegMapRemoveNative));
    Copy(0x8CCAA0u,kRegMapAllocateNative,sizeof(kRegMapAllocateNative));Copy(0x86C040u,kRegMapDestructorNative,sizeof(kRegMapDestructorNative));
    Jump(Address(0x888A10u),reinterpret_cast<UPtr>(&CtorService));Jump(Address(0x888D50u),reinterpret_cast<UPtr>(&DtorService));Jump(Address(0xC2875Au),reinterpret_cast<UPtr>(&DeleteService));
    DWORD codeOld=0;Check(VirtualProtect(reinterpret_cast<void*>(nativeCode),0x400000u,0x20u,&codeOld));
    DWORD prior=0;Check(VirtualProtect(reinterpret_cast<void*>(Address(0x86B000u)),0x1000u,0x40u,&prior));Check(prior==0x20u);
    Check(!Readable(Address(0x86BFA0u),sizeof(kRegMapCtorNative)) && RegMapSitesMatch(false));
    Check(VirtualProtect(reinterpret_cast<void*>(Address(0x86C000u)),0x1000u,1u,&prior));
    Check(!RegMapReadableSpan(Address(0x86BFA0u),sizeof(kRegMapCtorNative)) && !DiagSRegMapInstall());Check(Contains(lastLog,"reason=stockSignatures"));
    Check(VirtualProtect(reinterpret_cast<void*>(Address(0x86C000u)),0x1000u,0x120u,&prior));
    Check(!RegMapReadableSpan(Address(0x86BFA0u),sizeof(kRegMapCtorNative)) && !DiagSRegMapInstall());
    Check(VirtualProtect(reinterpret_cast<void*>(Address(0x86C000u)),0x1000u,0x20u,&prior));
    FlipCode(0x86C00Cu);Check(!DiagSRegMapInstall());FlipCode(0x86C00Cu);
    Check(VirtualProtect(reinterpret_cast<void*>(Address(0x86B000u)),0x1000u,0x20u,&prior));
    Say("real VirtualQuery reproduces R1 split-region rejection; R2 accepts RX/RWX signatures and rejects no-access/guard/second-page corruption\n");
    Check(RegMapSitesMatch(false));failAllocation=true;Check(!DiagSRegMapInstall() && RegMapSitesMatch(false));failAllocation=false;
    failProtect=true;Check(!DiagSRegMapInstall() && RegMapSitesMatch(false));failFlush=true;Check(!DiagSRegMapInstall() && RegMapSitesMatch(false));
    FlipCode(0x86BFE0u);Check(!DiagSRegMapInstall());FlipCode(0x86BFE0u);
    failCommitFlush=true;Check(!DiagSRegMapInstall() && RegMapSitesMatch(false));
    Check(DiagSRegMapInstall() && RegMapSitesMatch(true));DiagSRegMapRestore();Check(RegMapSitesMatch(false));Check(DiagSRegMapInstall());g_sRegMapHook=1;
    Check(PageProtection(0x86BFA0u)==0x20u && PageProtection(0x86C000u)==0x20u);
    Say("real two-site installer signatures, allocation/protection/flush rejection and exact restore passed\n");
    UPtr owners[8]={};for(U32 i=0;i<8u;++i){owner=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));owners[i]=owner;Construct();Fill(150u);}
    Check(g_regMapOwners==8 && g_regMapRedirects==8 && g_regMapExternalHigh==105);Say("actual native constructor/insert/lookup/allocator execute for eight distinct150-ID owners\n");
    owner=owners[0];RegMapArena* a=RegMapFind(Map());Check(a && a->generation==1u);UPtr stable=RegMapStart(a);long checks=g_regMapChecks;
    Check(Read32(Insert(20u))==0x12340013u);Check(g_regMapChecks==checks);Query(20u,1u);Query(800u,0u);CheckList(150u);
    for(U32 i=1;i<=150u;i+=2u)Check(Remove(i)==1u);Check(Remove(1000u)==0u);CheckList(75u);
    for(U32 i=1;i<=150u;i+=2u)Check(Insert(i)!=0u);CheckList(150u);Check(a->externalIssued==105u && !Read32(Map()+4u));
    for(U32 cycle=0;cycle<100u;++cycle){Destroy();Construct();Check(a->generation==cycle+2u && !a->attached && !a->inlineIssued);Fill(150u);Check(RegMapStart(a)==stable);}
    Check(g_regMapReconstructs==100 && g_regMapReattaches==100);Check(constructorServices==108u && destructorServices==100u && deleteServices==100u);
    Say("duplicates, mixed inline/external recycling and100 actual destroy/reconstruct generations passed\n");
    Destroy();Construct();Fill(557u);Check(a->externalIssued==512u);Fault(Map(),"externalEnd");Check(Remove(550u)==1u);Check(Insert(900u)!=0u);CheckList(557u);Fault(Map(),"externalEnd");
    Destroy();Construct();Fill(2u);Write32(Map(),Map()+13u);Fault(Map(),"unexpectedCursor");
    Destroy();Construct();Fill(1u);Check(Remove(1u));Write32(Read32(Map()),0x12345678u);Fault(Map(),"badRecycledLink");
    Destroy();Construct();Fill(1u);Write32(Map()+8u,2u);Fault(Map(),"countMismatch");
    Destroy();Construct();a->guard1^=1u;Fault(Map(),"guard");a->guard1^=1u;
    Say("real external-end bound, recycling at capacity and corruption diagnostics passed\n");
    owner=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));
    // Execute the original native constructor without observer to test cold attach and allocation failure.
    typedef UPtr (__thiscall* Ctor)(void*);reinterpret_cast<Ctor>(g_regMapPatches[0].trampoline)(reinterpret_cast<void*>(owner));
    failAllocation=true;Fault(Map(),"allocationFailed");failAllocation=false;Fill(150u);
    RegMapDetail d={};d.reason="fixtureFatal";d.map=Map();d.cursor=0x12345678u;d.active=17u;d.recycled=2u;allowFatal=true;RegMapFatal(d);allowFatal=false;
    Check(legacyCode==0x31u && Contains(lastLog,"reason=fixtureFatal") && Contains(lastLog,"cursor=12345678") && Contains(lastLog,"active=17"));
    stackTop=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,4u))+0xC000u;
    abiMode=2u;AbiInsert();Abi();Check(payload==owner);abiMode=1u;AbiInsert();Abi();Check(payload==owner);Fill(150u);
    abiMode=0u;key=4000u;AbiInsert();Abi();Check(payload);CheckList(151u);
    RegMapStartLine();Check(Contains(lastLog,"status=1 hooks=2") && Contains(lastLog,"startupCheckRevision=2"));g_capacityStart=GetTickCount()-2000u;RegMapPoll();Check(Contains(lastLog,"REGMAP_STATUS"));
    Say("cold attach, allocation fault, actual diagnostic fields, GP/stack/x87/SSE preservation passed\n");
    DiagSRegMapRestore();Check(RegMapSitesMatch(false) && PageProtection(0x86BFA0u)==0x20u && PageProtection(0x86C000u)==0x20u);Say("regmap reuse checks passed\n");ExitProcess(0);
}

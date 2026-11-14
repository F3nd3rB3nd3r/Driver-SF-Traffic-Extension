typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall VirtualFree(void*,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall VirtualProtect(void*,U32,DWORD,DWORD*);
extern "C" __declspec(dllimport) U32 __stdcall VirtualQuery(const void*,void*,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentProcess();
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("read fixture failure line ");char s[12]={};U32 n=0;do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}static long InterlockedExchange(volatile long* p,long v){long old=*p;*p=v;return old;}
static long InterlockedCompareExchange(volatile long* p,long n,long expected){long old=*p;if(old==expected)*p=n;return old;}
static long InterlockedExchangeAdd(volatile long* p,long v){long old=*p;*p+=v;return old;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}static void Write32(UPtr p,U32 n){*reinterpret_cast<volatile U32*>(p)=n;}
struct MemoryBasicInformation32 {void* base;void* allocationBase;U32 allocationProtect,regionSize,state,protect,type;};
static bool Readable(UPtr address,U32 bytes){if(!address || address+bytes<address)return false;MemoryBasicInformation32 m={};if(VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))!=sizeof(m))return false;return m.state==0x1000u && !(m.protect&0x100u) && (m.protect&0xFFu)!=1u && address+bytes<=reinterpret_cast<UPtr>(m.base)+m.regionSize;}
static bool RegMapReadableSpan(UPtr at,U32 n){if(!at || n>0x100000u || at+n<at)return false;while(n){U32 chunk=0x1000u-(at&0xFFFu);if(chunk>n)chunk=n;if(!Readable(at,chunk))return false;at+=chunk;n-=chunk;}return true;}
static bool RegMapEqual(UPtr at,const U8* b,U32 n){if(!RegMapReadableSpan(at,n))return false;for(U32 i=0;i<n;++i)if(Read8(at+i)!=b[i])return false;return true;}
static UPtr code=0,data=0;static U32 failAllocation=0,allocationCalls=0;
#include "../src/V29sReplayReadCore.inc"
static UPtr RFAddress(U32 at){return at>=0xE00000u?data+at-0xE2A000u:code+at-0x73F000u;}
static U32 RFRead32(UPtr p){return Read32(p);}static U8 RFRead8(UPtr p){return Read8(p);}
static void* RFAllocate(U32 n,U32 reserve){if(++allocationCalls==failAllocation)return nullptr;return VirtualAlloc(nullptr,n,reserve?0x2000u:0x3000u,reserve?1u:4u);}
static bool RFCommitMemory(UPtr p,U32 n){if(++allocationCalls==failAllocation)return false;return VirtualAlloc(reinterpret_cast<void*>(p),n,0x1000u,4u)==reinterpret_cast<void*>(p);}
static void RFFree(UPtr p){Check(VirtualFree(reinterpret_cast<void*>(p),0,0x8000u));}
#include "../src/V29sReplayReadNative.inc"
#include "../src/V29sReplayReadBridges.inc"
static UPtr g_pkg0=0;static long g_pauseStatus=1;static const U32 kPageExecuteReadWrite=0x40u;
static bool VerifyExecutableIdentity(){return true;}
static void PauseFatal(const char* why){Say(why);Say("\n");ExitProcess(2);}
static void AppendText(char*& p,const char* s,const char* end){while(*s && p<end)*p++=*s++;}
static void AppendNumber(char*& p,long v,const char* end){if(v<0){if(p<end)*p++='-';v=-v;}char s[12]={};U32 n=0,u=static_cast<U32>(v);do{s[n++]=static_cast<char>('0'+u%10);u/=10;}while(u);while(n && p<end)*p++=s[--n];}
static char lastLog[900]={};static void CapacityWrite(const char* s){U32 i=0;for(;s[i] && i<899u;++i)lastLog[i]=s[i];lastLog[i]=0;}
static bool failPage=false,failFlush=false,corruptRecheck=false;static U32 unlocked=0;
static BOOL Protect(void* p,U32 n,DWORD mode,DWORD* old){
    if(mode==0x40u && reinterpret_cast<UPtr>(p)>=code && reinterpret_cast<UPtr>(p)<code+0x8000u){
        if(failPage && ++unlocked==2u){failPage=false;return 0;}
        if(corruptRecheck && reinterpret_cast<UPtr>(p)==RFAddress(0x746000u)){corruptRecheck=false;Check(VirtualProtect(p,n,mode,old));*reinterpret_cast<U8*>(RFAddress(0x746940u))^=1u;return 1;}
    }return VirtualProtect(p,n,mode,old);
}
static BOOL Flush(void* p,const void* at,U32 n){if(failFlush && reinterpret_cast<UPtr>(at)==RFAddress(0x73F000u)){failFlush=false;return 0;}return FlushInstructionCache(p,at,n);}
#define VirtualProtect Protect
#define FlushInstructionCache Flush
#include "../src/V29sReplayReadLayout.inc"
#undef VirtualProtect
#undef FlushInstructionCache
static U32 PageMode(U32 at){MemoryBasicInformation32 m={};Check(VirtualQuery(reinterpret_cast<void*>(at),&m,sizeof(m))==sizeof(m));return m.protect;}
static void Stock(){DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x40u,&old));
    memcpy(reinterpret_cast<void*>(RFAddress(0x73FAB0u)),kRFCtorNative,sizeof(kRFCtorNative));memcpy(reinterpret_cast<void*>(RFAddress(0x73FB40u)),kRFDtorNative,sizeof(kRFDtorNative));
    memcpy(reinterpret_cast<void*>(RFAddress(0x746940u)),kRFReaderNative,sizeof(kRFReaderNative));memcpy(reinterpret_cast<void*>(RFAddress(0x746CA0u)),kRFDispatchNative,sizeof(kRFDispatchNative));
    const U32 methods[]={0x73FB40u,0x73FB20u,0x8203B0u,0x73FB30u,0x73F910u,0x73F930u,0x73F950u,0x73FA00u,0x73F9F0u,0x73F970u};
    for(U32 i=0;i<10u;++i)Write32(RFAddress(0xE2A088u)+i*4u,RFAddress(methods[i]));
    Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x20u,&old));g_rfStatus=0;
}
static U32 ctorCalls=0,dtorCalls=0;static UPtr lastSelf=0,lastBuffer=0;static U32 lastCapacity=0,lastMode=0,lastFlags=0;
static void* __fastcall FakeCtor(void* self,void*,const char*,void* buffer,U32 capacity,U32 mode){
    ++ctorCalls;lastSelf=reinterpret_cast<UPtr>(self);lastBuffer=reinterpret_cast<UPtr>(buffer);lastCapacity=capacity;lastMode=mode;
    Write32(lastSelf,RFAddress(0xE2A088u));Write32(lastSelf+4u,mode);Write32(lastSelf+0x855Cu,capacity);Write32(lastSelf+0x8560u,lastBuffer);
    Write32(lastSelf+0x8564u,lastBuffer);Write32(lastSelf+0x8568u,lastBuffer);Write32(lastSelf+0x856Cu,lastBuffer);return self;
}
static void* __fastcall FakeDtor(void* self,void*,U32 flags){++dtorCalls;lastFlags=flags;if(flags&1u)Check(VirtualFree(self,0,0x8000u));return self;}
__declspec(naked) static void DtorTail(){__asm{push dword ptr [esp+8]} __asm{call FakeDtor} __asm{pop esi} __asm{ret 4}}
static void Jump(UPtr at,UPtr target){*reinterpret_cast<U8*>(at)=0xE9;Write32(at+1,target-at-5);}
static void EngineTails(){DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x40u,&old));Jump(RFAddress(0x73FAB8u),reinterpret_cast<UPtr>(&FakeCtor));Jump(RFAddress(0x73FB49u),reinterpret_cast<UPtr>(&DtorTail));Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x20u,&old));}
static UPtr NewObject(){const UPtr p=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x9000u,0x3000u,4u));Check(p!=0);return p;}
static void Construct(UPtr self,UPtr original,U32 capacity=0x40000u,U32 mode=0){Check(reinterpret_cast<RFCtorFn>(RFAddress(0x73FAB0u))(reinterpret_cast<void*>(self),"fixture",reinterpret_cast<void*>(original),capacity,mode)==reinterpret_cast<void*>(self));}
static void Destruct(UPtr self,U32 flags=0){Check(reinterpret_cast<RFDtorFn>(RFAddress(0x73FB40u))(reinterpret_cast<void*>(self),flags)==reinterpret_cast<void*>(self));}
static UPtr site=0,inputSelf=0,hostEsp=0,afterEsp=0;static U32 inputValue=0,inputRequired=0,regs[8]={},flags=0;
__declspec(align(16)) static U8 fp[512]={};
__declspec(naked) static void Invoke(){
    __asm {pushfd} __asm {pushad} __asm {mov hostEsp,esp}
    __asm {fninit} __asm {fld1} __asm {mov eax,3F800000h} __asm {movd xmm0,eax}
    __asm {mov esi,inputSelf} __asm {mov eax,inputValue} __asm {mov ebp,inputRequired}
    __asm {mov ebx,11223344h} __asm {mov ecx,22334455h} __asm {mov edx,33445566h} __asm {mov edi,44556677h}
    __asm {std} __asm {stc} __asm {call dword ptr [site]}
    __asm {pushfd} __asm {pop flags} __asm {mov afterEsp,esp}
    __asm {mov dword ptr [regs],eax} __asm {mov dword ptr [regs+4],ecx} __asm {mov dword ptr [regs+8],edx} __asm {mov dword ptr [regs+12],ebx}
    __asm {mov dword ptr [regs+16],esp} __asm {mov dword ptr [regs+20],ebp} __asm {mov dword ptr [regs+24],esi} __asm {mov dword ptr [regs+28],edi}
    __asm {fxsave fp} __asm {cld} __asm {mov esp,hostEsp} __asm {popad} __asm {popfd} __asm {ret}
}
static void BridgeParity(UPtr self){
    DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x40u,&old));
    const U32 endings[]={0x746952u,0x746992u,0x746AE1u,0x746B69u,0x746BDDu,0x746B38u,0x746BE3u,0x746BE5u};
    for(U32 i=0;i<8u;++i)*reinterpret_cast<U8*>(RFAddress(endings[i]))=0xC3;
    Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x20u,&old));
    inputSelf=self;
    for(U32 i=2u;i<8u;++i)for(U32 n=0;n<3u;++n){const RFPatch& p=kRFPatches[i];site=RFAddress(p.at);inputValue=n==0u?0u:(n==1u?0x8000u:0x8001u);inputRequired=0x8000u;
        Write32(self+0x8584u,0x8000u);*reinterpret_cast<U8*>(self+0x858Cu)=static_cast<U8>(n&1u);
        Check(VirtualProtect(reinterpret_cast<void*>(site),p.len,0x40u,&old));memcpy(reinterpret_cast<void*>(site),p.stock,p.len);Check(VirtualProtect(reinterpret_cast<void*>(site),p.len,old,&old));
        Invoke();U32 expected[8]={};memcpy(expected,regs,sizeof(regs));U32 expectedFlags=flags;U8 expectedFp[512]={};memcpy(expectedFp,fp,512u);Check(afterEsp==hostEsp);
        Check(VirtualProtect(reinterpret_cast<void*>(site),p.len,0x40u,&old));for(U32 j=0;j<p.len;++j)*reinterpret_cast<U8*>(site+j)=RFPatchedByte(p,site,j);Check(VirtualProtect(reinterpret_cast<void*>(site),p.len,old,&old));
        Invoke();Check(afterEsp==hostEsp);for(U32 j=0;j<8u;++j)Check(regs[j]==expected[j]);Check(((flags^expectedFlags)&0xCD5u)==0u);
        // FXSAVE instruction/data pointers are historical; compare control,
        // status/tag, x87 register bytes, MXCSR and all XMM payloads.
        for(U32 j=0;j<8u;++j)Check(fp[j]==expectedFp[j]);for(U32 j=24;j<288u;++j)Check(fp[j]==expectedFp[j]);
    }
    // Commit parity intentionally wrote filled=EAX. Restore valid ring fields.
    Write32(self+0x8568u,RFFind(self)->buffer);*reinterpret_cast<U8*>(self+0x858Cu)=0;
}
static UPtr input=0;static U32 inputPos=0,polls=0,reads=0,links=0;static UPtr firstLink=0,lastLink=0;static bool backendBusy=false,acceptRead=true;
static U8 __fastcall FakePoll(void*,void*){++polls;return backendBusy?1u:0u;}
static U32 __fastcall FakeLength(void*,void*){return 0x200000u;}
static void __fastcall FakeSeek(void*,void*,U32 pos){inputPos=pos;}
static U8 __fastcall FakeRead(void*,void*,void* dst,U32 bytes){++reads;Check(bytes==0x8000u);memcpy(dst,reinterpret_cast<void*>(input+inputPos),bytes);inputPos+=bytes;return acceptRead?1u:0u;}
static void __fastcall FakeLink(void*,void*,UPtr p){++links;if(lastLink)Write32(lastLink+12u,p);else firstLink=p;lastLink=p;Write32(p+12u,0u);}
__declspec(naked) static void NoOp(){__asm{ret}}
static void* __cdecl Move(void* d,const void* s,U32 n){return memcpy(d,s,n);}
static void FullReader(UPtr self){
    Stock();RFInstall(true);Check(g_rfStatus==1);EngineTails();Construct(self,0x12340000u);RFOwner* r=RFFind(self);Check(r && r->state==1);
    DWORD old=0;Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x40u,&old));
    Jump(RFAddress(0x746400u),reinterpret_cast<UPtr>(&FakeLink));Jump(RFAddress(0x7468C0u),reinterpret_cast<UPtr>(&NoOp));
    const U8 cursor[]={0x8B,0x91,0x5C,0x85,0,0,0x8B,0x44,0x24,4,0x8B,0x89,0x60,0x85,0,0,3,0x44,0x24,8,3,0xCA,0x3B,0xC1,0x72,2,0x2B,0xC2,0xC2,8,0};
    const U8 freeSpan[]={0x8B,0x91,0x64,0x85,0,0,0x56,0x8B,0xB1,0x68,0x85,0,0,0x3B,0xD6,0x75,8,0x8B,0x81,0x5C,0x85,0,0,0x5E,0xC3,0x72,6,0x2B,0xD6,0x8B,0xC2,0x5E,0xC3,0x8B,0x81,0x5C,0x85,0,0,0x2B,0xC6,3,0xC2,0x5E,0xC3};
    memcpy(reinterpret_cast<void*>(RFAddress(0x745EA0u)),cursor,sizeof(cursor));memcpy(reinterpret_cast<void*>(RFAddress(0x745F20u)),freeSpan,sizeof(freeSpan));
    Write32(RFAddress(0x746B07u)+1u,reinterpret_cast<UPtr>(&Move)-(RFAddress(0x746B07u)+5u));
    Check(VirtualProtect(reinterpret_cast<void*>(code),0x9000u,0x20u,&old));
    Write32(RFAddress(0xE2A088u)+0xCu,reinterpret_cast<UPtr>(&FakePoll));Write32(RFAddress(0xE2A088u)+0x10u,reinterpret_cast<UPtr>(&FakeSeek));Write32(RFAddress(0xE2A088u)+0x14u,reinterpret_cast<UPtr>(&FakeRead));Write32(RFAddress(0xE2A088u)+0x24u,reinterpret_cast<UPtr>(&FakeLength));
    Write32(self+0xCu,0x200000u);Write32(self+0x8580u,0x8000u);Write32(self+0x8584u,0u);Write32(self+0x8588u,0u);*reinterpret_cast<U8*>(self+0x8594u)=0;*reinterpret_cast<U8*>(self+0x858Cu)=0;
    // Header at the very end, payload requires three I/O blocks. Real native
    // reader/assembly arithmetic is executed, with isolated backend/link stubs.
    const UPtr header=r->buffer+kRFBytes-16u;Write32(header,3u);Write32(header+4u,0x3F800000u);Write32(header+8u,0x10020u);Write32(header+12u,0);
    Write32(self+0x8564u,r->buffer+0x80000u);Write32(self+0x8568u,r->buffer);Write32(self+0x856Cu,header);
    inputPos=0;memset(reinterpret_cast<void*>(input),0,0x40000u);memset(reinterpret_cast<void*>(input),0x5A,0x10020u);polls=reads=links=0;firstLink=lastLink=0;acceptRead=true;
    typedef void (__thiscall* Reader)(void*);Reader read=reinterpret_cast<Reader>(RFAddress(0x746940u));
    backendBusy=true;read(reinterpret_cast<void*>(self));Check(polls==1 && reads==0 && r->counts.backendBusy>0);backendBusy=false;
    for(U32 i=0;i<4u;++i)read(reinterpret_cast<void*>(self));
    Check(polls==5 && reads==4 && firstLink && links>0);Check(Read32(firstLink)==3 && Read32(firstLink+8u)==0x10020u);
    Check(Read8(firstLink+16u)==0x5A && Read8(firstLink+16u+0x1001Fu)==0x5A);Check(r->counts.wrapChecks>0 && r->counts.commits>0);
    Check(Read32(self+0x856Cu)>=r->buffer && Read32(self+0x856Cu)<r->buffer+kRFBytes);
    // Native ordinary-space stop, then wrapped-record-space stop, then end stop.
    Write32(self+0x8584u,0);Write32(self+0x856Cu,r->buffer);Write32(self+0x8568u,r->buffer);Write32(self+0x8564u,r->buffer+0x8000u);
    U32 before=reads;read(reinterpret_cast<void*>(self));Check(reads==before && r->counts.spaceStops>0);
    Write32(self+0x856Cu,header);Write32(self+0x8568u,r->buffer);Write32(self+0x8564u,r->buffer+0x8000u);Write32(header+8u,0x10020u);
    read(reinterpret_cast<void*>(self));Check(reads==before && r->counts.wrapStops>0 && static_cast<U32>(r->lastStopRequired)==0x18000u);
    Write32(self+0x856Cu,r->buffer);Write32(self+0x8568u,r->buffer);Write32(self+0x8564u,r->buffer+0x80000u);*reinterpret_cast<U8*>(self+0x858Cu)=1;
    read(reinterpret_cast<void*>(self));Check(reads==before && r->counts.endStops>0);
    RFView v=RFCapture(self);Check(v.present && v.capacity==kRFBytes && v.issues==static_cast<U32>(r->counts.issues));Destruct(self);
}
extern "C" void __stdcall ReadTestMain(){
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x9000u,0x3000u,4u));data=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));input=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x200000u,0x3000u,4u));Check(code && data && input);
    Stock();Check(RFMatches(false) && RFDependencies());RFInstall(false);Check(g_rfStatus==-1 && RFMatches(false));
    Stock();failPage=true;unlocked=0;RFInstall(true);Check(g_rfStatus==-1 && RFMatches(false) && PageMode(code)==0x20u);
    Stock();failFlush=true;RFInstall(true);Check(g_rfStatus==-1 && RFMatches(false) && PageMode(code)==0x20u && PageMode(code+0x7000u)==0x20u);
    Stock();corruptRecheck=true;RFInstall(true);Check(g_rfStatus==-1);Stock();
    RFInstall(true);Check(g_rfStatus==1 && RFMatches(true));EngineTails();Say("installer verifies native bytes/vtables and restores RX pages after refusal/partial unlock/flush rollback\n");
    UPtr self=NewObject(),original=NewObject();Construct(self,original);RFOwner* r=RFFind(self);Check(r && lastBuffer==r->buffer && lastCapacity==kRFBytes && lastMode==0);
    Check((r->buffer&0xFFFFu)==0 && !Readable(r->buffer-1u,1u) && !Readable(r->buffer+kRFBytes,1u) && Readable(r->buffer,kRFBytes));
    UPtr backing=r->buffer;Destruct(self);Check(r->state==0);
    for(U32 i=0;i<100u;++i){Construct(self,original);Check(r->buffer==backing && r->state==1);Destruct(self);}
    Check(r->generation==101 && g_rfOwnerCount==1);Construct(self,original,0x80000u,1u);Check(lastBuffer==original && lastCapacity==0x80000u && lastMode==1 && g_rfOwnerCount==1);Destruct(self);
    for(U32 i=0;i<9u;++i){UPtr other=NewObject();Construct(other,original);Check(RFFind(other) && RFFind(other)->buffer!=backing);Destruct(other);}
    Say("1MiB aligned private backing/guards, 100 same-address generations, writer passthrough and more than4 owners passed\n");
    for(U32 stage=1;stage<=3;++stage){UPtr other=NewObject();failAllocation=allocationCalls+stage;U32 prior=ctorCalls;Construct(other,original);Check(ctorCalls==prior+1 && lastBuffer==original && lastCapacity==0x40000u && !RFFind(other));failAllocation=0;Destruct(other);}
    Check(g_rfAllocationFailed==3 && g_rfFallbacks==3);g_rfAllocationFailed=g_rfFallbacks=0;
    Construct(self,original);BridgeParity(self);Destruct(self);Say("six reader bridges preserve stack/GPRs/flags/DF/x87/SSE and native branch outcomes\n");
    FullReader(self);Say("actual native reader with 1MiB ring passes async-busy, 3-block wrapped payload, commit, space/wrap/end stops and single32KiB issue cadence\n");
    Stock();RFInstall(true);EngineTails();UPtr deleted=NewObject();Construct(deleted,original);RFOwner* deletedRecord=RFFind(deleted);Destruct(deleted,1u);Check(deletedRecord->state==0 && lastFlags==1);
    Check(g_rfFaults==0);Say("allocation fallback remains native, and deleting destructor never dereferences the freed object\n");
    Say("replay read checks passed\n");ExitProcess(0);
}

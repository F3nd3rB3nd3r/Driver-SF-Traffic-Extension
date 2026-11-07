// Production request bridge + actual native setter bytes in an isolated process.
// Only native lookup/rebuild helpers and request processing are fixture stubs.
typedef unsigned char U8;typedef unsigned short U16;typedef unsigned int U32;typedef unsigned int UPtr;
typedef unsigned long DWORD;typedef int BOOL;
extern "C" {
__declspec(dllimport) void __stdcall ExitProcess(U32);
__declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
__declspec(dllimport) U32 __stdcall VirtualQuery(const void*,void*,U32);
__declspec(dllimport) void* __stdcall CreateFileA(const char*,U32,U32,void*,U32,U32,void*);
__declspec(dllimport) BOOL __stdcall ReadFile(void*,void*,U32,U32*,void*);
__declspec(dllimport) BOOL __stdcall CloseHandle(void*);
__declspec(dllimport) void* __stdcall GetStdHandle(U32);
__declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,U32,U32*,void*);
__declspec(dllimport) long __stdcall InterlockedIncrement(volatile long*);
__declspec(dllimport) long __stdcall InterlockedExchange(volatile long*,long);
void* _ReturnAddress();
}
#pragma intrinsic(_ReturnAddress)
extern "C" __declspec(noinline) void* __cdecl memset(void* d,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(d)[i]=static_cast<U8>(v);return d;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* d,const void* s,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(d)[i]=reinterpret_cast<const U8*>(s)[i];return d;}
static void CheckAt(bool ok,U32 line){if(ok)return;char text[48]="chapter fixture failed line ";U32 n=28u,div=10000u;bool begun=false;
    for(;div;div/=10u){const U32 digit=line/div%10u;if(digit || begun || div==1u){text[n++]=static_cast<char>('0'+digit);begun=true;}}
    text[n++]='\n';U32 written=0;WriteFile(GetStdHandle(static_cast<U32>(-11)),text,n,&written,nullptr);ExitProcess(line);}
#define Check(v) CheckAt((v),__LINE__)
static const U32 kPreferredBase=0x400000u,kRootPtr=0xFE7228u,kReqCapacity=150u,kReqInline=50u;
static UPtr g_base=kPreferredBase;static UPtr Address(UPtr p){return g_base+p-kPreferredBase;}
static U8 Read8(UPtr p){return *reinterpret_cast<volatile const U8*>(p);}static U16 Read16(UPtr p){return *reinterpret_cast<volatile const U16*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile const U32*>(p);}static void Write32(UPtr p,U32 n){*reinterpret_cast<volatile U32*>(p)=n;}
struct MemoryBasicInformation32{void* base;void* allocationBase;U32 allocationProtect,regionSize,state,protect,type;};
static bool Readable(UPtr address,U32 bytes){if(!address || bytes>0x100000u || address+bytes<address)return false;
    MemoryBasicInformation32 m={};if(VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))<sizeof(m))return false;
    const UPtr end=reinterpret_cast<UPtr>(m.base)+m.regionSize;
    return m.state==0x1000u && !(m.protect&0x100u) && (m.protect&0xFFu)!=1u && end>=address && address+bytes<=end;}
static bool identity=true;static bool VerifyExecutable(){return identity;}
static volatile long DSF150_Status=2,DSFV27B_AdmissionAfter=160;
static volatile long DSFV27B_RequestRejected=0,DSFV27B_RequestLastCount=0,DSFV27B_RequestHighWater=0;
static long eventReason=-1;static UPtr eventCaller=0;static U32 processed=0;
static const U32 kChapter7ActiveIds[]={
#include "ChapterRosterValues.inc"
};
static const U32* kChapter7Active150Ids=kChapter7ActiveIds;
#include "../src/V29sChapterProfiles.h"
static const U8* nativeSetterBytes=kChapterSetterBytes;
static U8 rebasedSetter[198]={};
#define kChapterSetterBytes rebasedSetter
#include "../src/V29sChapterCore.inc"
#undef kChapterSetterBytes
static void ChapterEvent(ChapterReason reason,UPtr caller,UPtr,U32,U32,int,U32){eventReason=reason;eventCaller=caller;}
struct RequestSidecar{UPtr owner;U32 count;U32 ids[150];};static RequestSidecar slots[16]={};
static void RequestLock(){}static void RequestUnlock(){}
static RequestSidecar* RequestSlot(UPtr owner,bool create){RequestSidecar* empty=nullptr;for(auto& s:slots){if(s.owner==owner)return &s;if(!s.owner && !empty)empty=&s;}
    if(create && empty){empty->owner=owner;return empty;}return nullptr;}
static void Log(const char*){}
#define DSF_V29S_CHAPTERS 1
#include "ChapterRequestBody.inc"
static UPtr aiCurrent=0,aiOriginal=0,root=0,context=0,owner=0;
static void* Alloc(U32 n,UPtr fixed=0){void* p=VirtualAlloc(reinterpret_cast<void*>(fixed),n,0x3000u,0x40u);Check(p!=nullptr);if(fixed)Check(reinterpret_cast<UPtr>(p)==fixed);return p;}
static void Jump(UPtr at,UPtr to){*reinterpret_cast<U8*>(at)=0xE9u;Write32(at+1u,to-at-5u);}
extern "C" UPtr __fastcall FakeLookup(void* ai,void*,U32 hash){const UPtr records=Read32(reinterpret_cast<UPtr>(ai)+0x11Cu);
    for(U32 i=0;i<59u;++i)if(Read32(records+i*28u)==hash)return records+i*28u;return 0;}
extern "C" void __fastcall FakeProperty(void*,void*,U32){}
extern "C" void __cdecl FakeRebuild(){}
extern "C" void __fastcall FakeProcess(void*,void*){++processed;}
static UPtr LoadAi(const char* path){void* f=CreateFileA(path,0x80000000u,3u,nullptr,3u,0x80u,nullptr);Check(reinterpret_cast<UPtr>(f)!=0xFFFFFFFFu);
    U8* bytes=reinterpret_cast<U8*>(Alloc(4403200u));U32 read=0;Check(ReadFile(f,bytes,4403200u,&read,nullptr)!=0 && read==4403200u);CloseHandle(f);
    const UPtr ai=reinterpret_cast<UPtr>(bytes)+0x1080u;const U32 offsets[]={0xECu,0x11Cu,0x124u,0x12Cu,0x15Cu,0x164u};
    for(U32 o:offsets)Write32(ai+o,ai+Read32(ai+o));return ai;}
typedef U8 (__thiscall* Setter)(void*,U32,U32);
static void SetAi(UPtr ai){Write32(Address(0xFE7AC0u),ai);}
static void Request(U32 hash,U32 frequency){eventReason=-1;Check(reinterpret_cast<Setter>(Address(0x76FDF0u))(reinterpret_cast<void*>(context),hash,frequency)==1u);
    Check(Read32(context+0x5CA840u)==hash && Read32(context+0x5CA844u)==frequency);}
static void ExpectCount(U32 count){Check(Read32(owner+0xC8u)==count);RequestSidecar* s=RequestSlot(owner,false);Check(s && s->count==count);}
static void ExpectExpanded(){ExpectCount(150u);const auto* s=RequestSlot(owner,false);for(U32 i=0;i<150u;++i)Check(s->ids[i]==kChapter7ActiveIds[i]);
    for(U32 i=0;i<50u;++i)Check(Read32(owner+i*4u)==kChapter7ActiveIds[i]);Check(eventReason==ChapterExpand && eventCaller==Address(0x76FE9Au));}
static const ChapterProfile& Find(U32 hash,U32 frequency){for(const auto& p:kChapterProfiles)if(p.hash==hash && p.frequency==frequency)return p;ExitProcess(200);return kChapterProfiles[0];}
static void Reject(ChapterReason reason,const ChapterProfile& p){Request(p.hash,p.frequency);ExpectCount(p.count);Check(eventReason==reason);}
static void OtherCaller(UPtr preferred){const UPtr call=Address(preferred),entry=call-8u;const U8 pushes[]={0xFF,0x74,0x24,0x08,0xFF,0x74,0x24,0x08};memcpy(reinterpret_cast<void*>(entry),pushes,8u);
    *reinterpret_cast<U8*>(call)=0xE8;Write32(call+1u,Address(0x5A79F0u)-call-5u);const U8 tail[]={0xC2,0x08,0x00};memcpy(reinterpret_cast<void*>(call+5u),tail,3u);
    typedef void (__thiscall* Writer)(void*,const U32*,int);U32 ids[18]={};const auto& p=Find(0x47EB4D0Eu,0u);for(U32 i=0;i<18u;++i)ids[i]=p.ids[i];
    reinterpret_cast<Writer>(entry)(reinterpret_cast<void*>(owner),ids,18);ExpectCount(18);Check(eventReason==ChapterOtherCaller && eventCaller==call+5u);}
extern "C" void __stdcall ChapterTestMain(){
    g_base=reinterpret_cast<UPtr>(Alloc(0xC00000u));
    memcpy(rebasedSetter,nativeSetterBytes,sizeof(rebasedSetter));U32 relocations=0;
    for(U32 i=0;i+4u<=sizeof(rebasedSetter);++i){const U32 value=Read32(reinterpret_cast<UPtr>(rebasedSetter)+i);
        if(value==0xFE7AC0u || value==0xFE7228u){Write32(reinterpret_cast<UPtr>(rebasedSetter)+i,Address(value));i+=3u;++relocations;}}
    Check(relocations==4u);memcpy(reinterpret_cast<void*>(Address(0x76FDF0u)),rebasedSetter,sizeof(rebasedSetter));
    Jump(Address(0x40A780u),reinterpret_cast<UPtr>(&FakeLookup));Jump(Address(0x76FB60u),reinterpret_cast<UPtr>(&FakeProperty));Jump(Address(0x769240u),reinterpret_cast<UPtr>(&FakeRebuild));
    Jump(Address(0x5A79F0u),reinterpret_cast<UPtr>(&RequestWriteBridge));Jump(Address(0x5A7490u),reinterpret_cast<UPtr>(&FakeProcess));
    root=reinterpret_cast<UPtr>(Alloc(0x1000u));context=reinterpret_cast<UPtr>(Alloc(0x5CB000u));owner=reinterpret_cast<UPtr>(Alloc(0x1000u));
    Write32(Address(0xFE7228u),root);Write32(root+0x758u,context);Write32(root+0x214u,owner);Write32(owner+0xDACu,1u);
    aiCurrent=LoadAi("E:\\Games\\Driver San Francisco Save Bug Test\\media\\san_franciscoAI.fchunk");
    aiOriginal=LoadAi("E:\\Games\\Driver San Francisco Save Bug Test\\media\\san_franciscoAI_original.fchunk");
    Check(ChapterPreflight());identity=false;Check(!ChapterPreflight());identity=true;
    *reinterpret_cast<U8*>(Address(0x76FDF0u))^=1u;Check(!ChapterPreflight());*reinterpret_cast<U8*>(Address(0x76FDF0u))^=1u;Check(ChapterPreflight());
    SetAi(aiCurrent);U32 expanded=0,native=0;
    for(const auto& p:kChapterProfiles){Request(p.hash,p.frequency);if(p.expand){ExpectExpanded();++expanded;}else{ExpectCount(p.count);Check(eventReason==ChapterExcluded);++native;}}
    Check(expanded==55u && native==25u && DSFV27B_RequestRejected==0);
    SetAi(aiOriginal);for(const auto& p:kChapterProfiles){if(p.expand)Reject(ChapterData,p);else{Request(p.hash,p.frequency);ExpectCount(p.count);Check(eventReason==ChapterExcluded);}}
    SetAi(aiCurrent);const auto& p=Find(0x47EB4D0Eu,0u);Request(p.hash,0);ExpectExpanded();
    OtherCaller(0x66E5F3u);OtherCaller(0x74BA58u);
    Request(p.hash,4u);ExpectCount(0u);Check(eventReason==ChapterEmpty);
    DSF150_Status=1;Reject(ChapterGraphics,p);DSF150_Status=2;DSFV27B_AdmissionAfter=150;Reject(ChapterAdmission,p);DSFV27B_AdmissionAfter=160;
    UPtr record=Read32(aiCurrent+0x11Cu)+28u*p.slot,list=Read32(aiCurrent+0x124u)+2u*Read16(record+4u);
    const U16 first=Read16(list);*reinterpret_cast<U16*>(list)=62u;Reject(ChapterSource,p);*reinterpret_cast<U16*>(list)=first;
    const UPtr meta=Read32(aiCurrent+0x15Cu)+12u*p.tables[0];U32 old=Read32(meta+8u);Write32(meta+8u,0);Reject(ChapterData,p);Write32(meta+8u,old);
    UPtr weights=Read32(aiCurrent+0x164u)+p.tables[0]*191u*8u;old=Read32(weights);Write32(weights,9999u);Reject(ChapterData,p);Write32(weights,old);
    old=Read32(weights+4u);Write32(weights+4u,0x40400000u);Reject(ChapterData,p);Write32(weights+4u,old);
    const UPtr groups=Read32(aiCurrent+0xECu),routes=Read32(aiCurrent+0x12Cu);
    const UPtr route=routes+(Read16(groups+8u)+p.slot)*20u+13u;const U8 table=Read8(route);*reinterpret_cast<U8*>(route)=255u;Reject(ChapterData,p);*reinterpret_cast<U8*>(route)=table;
    old=Read32(aiCurrent+0x164u);Write32(aiCurrent+0x164u,0);Reject(ChapterData,p);Write32(aiCurrent+0x164u,old);
    old=Read32(aiCurrent+0x118u);Write32(aiCurrent+0x118u,60u);Reject(ChapterData,p);Write32(aiCurrent+0x118u,old);
    U32 ids[27]={};for(U32 i=0;i<p.count;++i)ids[i]=p.ids[i];U32 hash=0,freq=0,index=0;
    Write32(context+0x5CA840u,p.hash);Write32(context+0x5CA844u,0);
    Check(ChapterDecide(Address(0x76FE9Au),owner+16u,ids,p.count,hash,freq,index)==ChapterOwner);
    ids[0]|=0x10000u;Check(ChapterDecide(Address(0x76FE9Au),owner,ids,p.count,hash,freq,index)==ChapterSource);ids[0]=p.ids[0];
    Write32(context+0x5CA844u,5u);Check(ChapterDecide(Address(0x76FE9Au),owner,ids,p.count,hash,freq,index)==ChapterUnknown);
    Write32(context+0x5CA840u,0xDEADBEEFu);Write32(context+0x5CA844u,0);Check(ChapterDecide(Address(0x76FE9Au),owner,ids,p.count,hash,freq,index)==ChapterUnknown);
    Write32(context+0x5CA840u,p.hash);g_chapterStatus=-1;Check(ChapterDecide(Address(0x76FE9Au),owner,ids,p.count,hash,freq,index)==ChapterContext);Check(ChapterPreflight());
    Write32(owner+0xDACu,0);Request(p.hash,0);ExpectExpanded();Check(processed==1u);Write32(owner+0xDACu,1u);
    for(U32 loop=0;loop<3u;++loop)for(const auto& profile:kChapterProfiles)if(profile.expand){Request(profile.hash,profile.frequency);ExpectExpanded();}
    Check(DSFV27B_RequestRejected==0 && slots[0].owner==owner && !slots[1].owner);
    const char result[]="chapter request checks passed\n";U32 written=0;WriteFile(GetStdHandle(static_cast<U32>(-11)),result,sizeof(result)-1u,&written,nullptr);ExitProcess(0);
}

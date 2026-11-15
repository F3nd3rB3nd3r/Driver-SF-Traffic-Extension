// Isolated production config/parser, chapter policy and request storage fixture.
// Native services and return-address identity are stubbed; no Driver.exe/assets.
typedef unsigned char U8;typedef unsigned short U16;typedef unsigned int U32;typedef unsigned int UPtr;
typedef unsigned long DWORD;typedef int BOOL;typedef void* HANDLE;typedef void* HMODULE;typedef const wchar_t* LPCWSTR;
extern "C" {
__declspec(dllimport) void __stdcall ExitProcess(U32);
__declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
__declspec(dllimport) U32 __stdcall VirtualQuery(const void*,void*,U32);
__declspec(dllimport) void* __stdcall GetStdHandle(U32);
__declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,U32,U32*,void*);
__declspec(dllimport) long __stdcall InterlockedIncrement(volatile long*);
__declspec(dllimport) long __stdcall InterlockedExchange(volatile long*,long);
}
extern "C" __declspec(noinline) void* __cdecl memset(void* d,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(d)[i]=static_cast<U8>(v);return d;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* d,const void* s,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(d)[i]=reinterpret_cast<const U8*>(s)[i];return d;}
static void CheckAt(bool ok,U32 line){if(ok)return;ExitProcess(line);}
#define Check(value) CheckAt((value),__LINE__)
static U32 TextSize(const char* s){U32 n=0;while(s[n])++n;return n;}
static U32 ioError=2,ioLength=19,ioReadOk=1,ioClosed=0;static const char* ioBytes="";
static U32 GetModuleFileNameW(HMODULE,wchar_t* out,U32){const wchar_t name[]=L"E:\\mods\\traffic.asi";for(U32 i=0;i<sizeof(name)/2u;++i)out[i]=name[i];return ioLength;}
static U32 GetLastError(){return ioError;}
static HANDLE CreateFileW(LPCWSTR path,U32,U32,void*,U32,U32,HANDLE){
    const wchar_t expected[]=L"E:\\mods\\DriverSF_Traffic.ini";for(U32 i=0;i<sizeof(expected)/2u;++i)Check(path[i]==expected[i]);
    return ioError?reinterpret_cast<HANDLE>(0xFFFFFFFFu):reinterpret_cast<HANDLE>(1u);}
static BOOL ReadFile(HANDLE,void* out,U32 capacity,U32* read,void*){*read=TextSize(ioBytes);if(*read>capacity)*read=capacity;memcpy(out,ioBytes,*read);return ioReadOk;}
static BOOL CloseHandle(HANDLE){++ioClosed;return 1;}
#include "../src/V29sTrafficPolicyConfig.inc"
static void Parse(const char* text,TrafficRosterMode mode,const char* reason){g_trafficPolicy=TrafficParse(text,TextSize(text));Check(g_trafficPolicy.mode==mode);Check(TrafficEqual(g_trafficPolicy.reason,g_trafficPolicy.reason+TextSize(g_trafficPolicy.reason),reason));}
static void ConfigTests(){
    Check(g_trafficPolicy.mode==TrafficNative);
    Parse("",TrafficNative,"noSetting");Parse("[Other]\nRosterMode=Preset150",TrafficNative,"noSetting");
    Parse("[Traffic]\nRosterMode=Native",TrafficNative,"configured");
    Parse("\xEF\xBB\xBF[ traffic ]\r\n RosterMode = pReSeT150 ; note\r\n",TrafficPreset150,"configured");
    Parse("[Traffic]\nRosterMode=700",TrafficNative,"invalidMode");
    Parse("[Traffic]\nRosterMode=",TrafficNative,"invalidMode");
    Parse("[Traffic]\nRosterMode",TrafficNative,"invalidMode");
    Parse("[Traffic]\nRosterMode=Preset150\nRosterMode=Native",TrafficNative,"duplicateSetting");
    Parse("[Traffic]\nRosterMode=Preset150\n[Traffic]\nRosterMode=Preset150",TrafficNative,"duplicateSetting");
    Parse("[Traffic\nRosterMode=Preset150",TrafficNative,"invalidSection");
    const char utf16[]={'[',0,'T',0};Check(TrafficParse(utf16,sizeof(utf16)).mode==TrafficNative);
    static char large[4097];for(U32 i=0;i<sizeof(large);++i)large[i]=' ';
    Check(TrafficParse(large,sizeof(large)).mode==TrafficNative);
    TrafficLoadPolicy(nullptr);Check(g_trafficPolicy.mode==TrafficNative && ioClosed==0);
    ioLength=1024;TrafficLoadPolicy(nullptr);Check(g_trafficPolicy.mode==TrafficNative);ioLength=19;
    ioError=0;ioBytes="[Traffic]\nRosterMode=Preset150";TrafficLoadPolicy(nullptr);Check(g_trafficPolicy.mode==TrafficPreset150 && ioClosed==1);
    ioReadOk=0;TrafficLoadPolicy(nullptr);Check(g_trafficPolicy.mode==TrafficNative && ioClosed==2);ioReadOk=1;
}
static const U32 kPreferredBase=0x400000u,kRootPtr=0xFE7228u,kReqCapacity=150u,kReqInline=50u;
static UPtr g_base=0;static UPtr Address(UPtr p){return g_base+p-kPreferredBase;}
static U8 Read8(UPtr p){return *reinterpret_cast<const U8*>(p);}static U16 Read16(UPtr p){return *reinterpret_cast<const U16*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<const U32*>(p);}static void Write32(UPtr p,U32 n){*reinterpret_cast<U32*>(p)=n;}
struct MBI{void* base;void* allocation;U32 allocationProtect,size,state,protect,type;};
static bool Readable(UPtr p,U32 size){MBI m={};return p && size<=0x100000u && p+size>=p && VirtualQuery(reinterpret_cast<void*>(p),&m,sizeof(m))==sizeof(m) && m.state==0x1000u && !(m.protect&0x100u) && (m.protect&0xFFu)!=1u && p+size<=reinterpret_cast<UPtr>(m.base)+m.size;}
static UPtr Allocate(U32 size){const UPtr p=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,size,0x3000u,0x40u));Check(p!=0);return p;}
static bool identity=true;static bool VerifyExecutable(){return identity;}
static volatile long DSF150_Status=2,DSFV27B_AdmissionAfter=160;
static volatile long DSFV27B_RequestRejected=0,DSFV27B_RequestLastCount=0,DSFV27B_RequestHighWater=0;
#include "../src/Chapter7Preload.h"
static U32 ordered[150];static const U32* kChapter7Active150Ids=ordered;
#define DSF_V29S_TRAFFIC_POLICY 1
#define DSF_V29S_CHAPTERS 1
#include "../src/V29sChapterCore.inc"
static ChapterReason lastReason=ChapterContext;
static void ChapterEvent(ChapterReason reason,UPtr,UPtr,U32,U32,int,U32){lastReason=reason;}
struct RequestSidecar{UPtr owner;U32 count;U32 ids[150];};static RequestSidecar slots[16]={};
static void RequestLock(){}static void RequestUnlock(){}
static RequestSidecar* RequestSlot(UPtr owner,bool create){RequestSidecar* empty=nullptr;for(auto& s:slots){if(s.owner==owner)return &s;if(!s.owner && !empty)empty=&s;}if(create && empty){empty->owner=owner;return empty;}return nullptr;}
static void Log(const char*){}
static UPtr caller=0;static void* TrafficFixtureCaller(){return reinterpret_cast<void*>(caller);}
#include "TrafficRequestBody.inc"
static const U32 kRequestProbeIndex[10]={0,44,49,50,51,52,100,101,102,149};
static volatile long g_requestProbeCalls[10]={},g_requestProbeId[10]={},g_requestProbeExpected[10]={},g_requestProbeFound[10]={},g_requestProbeBad[10]={};
static volatile long DSFV29s_RequestBoundaryBad=0;
#include "TrafficRequestProbe.inc"
static U32 processed=0;static void __fastcall FakeProcess(void*,void*){++processed;}
static void Submit(UPtr owner,const U32* ids,int count){RequestWriteBridge(reinterpret_cast<void*>(owner),nullptr,ids,count);}
static void Expect(UPtr owner,const U32* ids,U32 count){const auto* s=RequestSlot(owner,false);Check(s && s->count==count && Read32(owner+0xC8u)==count);for(U32 i=0;i<count;++i)Check(s->ids[i]==ids[i]);for(U32 i=0;i<count && i<50;++i)Check(Read32(owner+i*4u)==ids[i]);}
static UPtr ai,settings,lists,groups,routes,metadata,weights,root,context;
static void PrepareAi(const ChapterProfile& p){
    Write32(Address(0xFE7AC0u),ai);Write32(root+0x758u,context);Write32(context+0x5CA840u,p.hash);Write32(context+0x5CA844u,p.frequency);
    const UPtr record=settings+p.slot*28u;Write32(record,p.hash);*reinterpret_cast<U8*>(record+24u)=static_cast<U8>(p.slot);
    *reinterpret_cast<U16*>(record+14u+p.frequency*2u)=static_cast<U16>(p.count);*reinterpret_cast<U16*>(record+4u+p.frequency*2u)=0;
    for(U32 i=0;i<p.count;++i)*reinterpret_cast<U16*>(lists+2u*i)=p.ids[i];
    for(U32 g=0;g<319;++g){*reinterpret_cast<U16*>(groups+g*12u+8u)=static_cast<U16>(g*59u);*reinterpret_cast<U8*>(routes+(g*59u+p.slot)*20u+13u+p.frequency)=p.tables[g%p.tableCount];}
}
extern "C" void __stdcall TrafficPolicyTestMain(){
    ConfigTests();g_base=Allocate(0xC00000u);caller=Address(0x76FE9Au);
    memcpy(reinterpret_cast<void*>(Address(0x76FDF0u)),kChapterSetterBytes,sizeof(kChapterSetterBytes));
    Check(ChapterPreflight());identity=false;Check(!ChapterPreflight());identity=true;Check(ChapterPreflight());
    for(U32 i=0;i<150;++i)ordered[i]=kChapter7AllPositiveIds[i];for(U32 i=1;i<150;++i)for(U32 j=i;j && ordered[j-1]>ordered[j];--j){const U32 n=ordered[j];ordered[j]=ordered[j-1];ordered[j-1]=n;}
    const UPtr owner=Allocate(0x1000u);Write32(owner+0xDACu,1);
    g_trafficPolicy={TrafficNative,"configured"};
    // Every named source profile passes unchanged even with no AI/root pointer.
    for(const auto& p:kChapterProfiles){U32 ids[27]={};for(U32 i=0;i<p.count;++i)ids[i]=p.ids[i];Submit(owner,ids,p.count);Expect(owner,ids,p.count);Check(lastReason==ChapterNative);}
    U32 custom[150];for(U32 i=0;i<150;++i)custom[i]=65000u+i;
    Submit(owner,custom,150);Expect(owner,custom,150);
    for(U32 i=0;i<10;++i){const U32 index=kRequestProbeIndex[i];RecordRequestProbe(owner,index,custom[index],true);}Check(DSFV29s_RequestBoundaryBad==0);
    RecordRequestProbe(owner,149,custom[149]-1u,true);Check(DSFV29s_RequestBoundaryBad==1);
    const long rejected=DSFV27B_RequestRejected;Submit(owner,custom,151);Expect(owner,custom,150);Check(DSFV27B_RequestRejected==rejected+1);
    U32 invalid[1]={65536};Submit(owner,invalid,1);Expect(owner,custom,150);Check(DSFV27B_RequestRejected==rejected+2);
    Submit(owner,nullptr,1);Expect(owner,custom,150);Check(DSFV27B_RequestRejected==rejected+3);
    Submit(owner,nullptr,0);Expect(owner,nullptr,0);Submit(owner,nullptr,-1);Expect(owner,nullptr,0);
    // Minimal synthetic AI with the production shape and all expected weights.
    ai=Allocate(0x1000);settings=Allocate(59*28);lists=Allocate(1369*2);groups=Allocate(319*12);routes=Allocate(18821*20);metadata=Allocate(208*12);weights=Allocate(39728*8);
    root=Allocate(0x1000);context=Allocate(0x5CB000);Write32(Address(kRootPtr),root);Write32(root+0x214,owner);
    const U32 offsets[]={0x118,0x11C,0x120,0x124,0xE8,0xEC,0x128,0x12C,0x158,0x15C,0x160,0x164};
    const U32 values[]={59,settings,1369,lists,319,groups,18821,routes,208,metadata,39728,weights};for(U32 i=0;i<12;++i)Write32(ai+offsets[i],values[i]);
    for(U32 t=0;t<208;++t){Write32(metadata+t*12,t*191);Write32(metadata+t*12+4,191);Write32(metadata+t*12+8,0x43160000);
        for(U32 r=0;r<191;++r){Write32(weights+(t*191+r)*8,kChapterRowIds[r]);Write32(weights+(t*191+r)*8+4,kChapterRowWeights[r]);}}
    g_trafficPolicy={TrafficPreset150,"configured"};U32 expanded=0;
    for(const auto& p:kChapterProfiles){PrepareAi(p);U32 ids[27]={};for(U32 i=0;i<p.count;++i)ids[i]=p.ids[i];Submit(owner,ids,p.count);
        if(p.expand){Expect(owner,ordered,150);Check(lastReason==ChapterExpand);++expanded;}else{Expect(owner,ids,p.count);Check(lastReason==ChapterExcluded);}}
    Check(expanded==55);
    const ChapterProfile* chosen=nullptr;for(const auto& p:kChapterProfiles)if(p.expand){chosen=&p;break;}Check(chosen!=nullptr);
    const auto& p=*chosen;PrepareAi(p);U32 source[27]={};for(U32 i=0;i<p.count;++i)source[i]=p.ids[i];
    Write32(ai+0x118,58);Submit(owner,source,p.count);Expect(owner,source,p.count);Check(lastReason==ChapterData);Write32(ai+0x118,59);
    DSFV27B_AdmissionAfter=150;Submit(owner,source,p.count);Expect(owner,source,p.count);Check(lastReason==ChapterAdmission);DSFV27B_AdmissionAfter=160;
    caller=Address(0x66E5F8);Submit(owner,source,p.count);Expect(owner,source,p.count);Check(lastReason==ChapterOtherCaller);caller=Address(0x76FE9A);
    g_trafficPolicy={TrafficNative,"configured"};Submit(owner,source,p.count);Expect(owner,source,p.count);Check(lastReason==ChapterNative);
    // Native processing is still called once for an idle owner.
    const UPtr process=Address(0x5A7490);*reinterpret_cast<U8*>(process)=0xE9;Write32(process+1,reinterpret_cast<UPtr>(&FakeProcess)-process-5);
    Write32(owner+0xDAC,0);Submit(owner,source,p.count);Check(processed==1);
    const char text[]="traffic policy checks passed\n";U32 written=0;WriteFile(GetStdHandle(static_cast<U32>(-11)),text,sizeof(text)-1,&written,nullptr);ExitProcess(0);
}

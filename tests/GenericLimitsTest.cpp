#include "GenericTestSupport.inc"
#define DSF_V29S_GENERIC_LIMITS 1
#define DSF_V29S_CHAPTERS 1
#include "../src/V29sGenericConfig.inc"
static const U32 kPreferredBase=0x400000u,kRootPtr=0xFE7228u,kMemCommitReserve=0x3000u,kPageReadWrite=4u,kReqInline=50u;
static UPtr g_base=0;static UPtr Address(UPtr at){return g_base+at-kPreferredBase;}
static U8 Read8(UPtr p){return *reinterpret_cast<U8*>(p);}static U16 Read16(UPtr p){return *reinterpret_cast<U16*>(p);}
static U32 Read32(UPtr p){return *reinterpret_cast<U32*>(p);}static void Write32(UPtr p,U32 v){*reinterpret_cast<U32*>(p)=v;}
struct MBI{void* base;void* allocation;U32 allocationProtect,size,state,protect,type;};
static bool Readable(UPtr p,U32 n){MBI m={};return p && p+n>=p && VirtualQuery(reinterpret_cast<void*>(p),&m,sizeof(m))==sizeof(m) && m.state==0x1000u && (m.protect&0xFFu)!=1u && !(m.protect&0x100u) && p+n<=reinterpret_cast<UPtr>(m.base)+m.size;}
static UPtr Allocate(U32 n){const UPtr p=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,n,0x3000u,4u));Check(p!=0);return p;}
static bool VerifyExecutable(){return true;}static U32 GetCurrentThreadId(){return 1;}
static U32 fatalCount=0,fatalRequired=0;static const char* fatalReason="";static const char* lastReason="";
#include "../src/V29sGenericRosterCore.inc"
static void GenericFatal(const char* reason,UPtr,U32 required){++fatalCount;fatalReason=reason;fatalRequired=required;}
static void GenericEvent(const char* reason,UPtr,UPtr,U32,U32,int,U32){lastReason=reason;}
#include "GenericRequestTypes.inc"
static void RequestLock(){}static void RequestUnlock(){}
#include "GenericRequestSlot.inc"
static volatile long DSFV27B_RequestRejected=0,DSFV27B_RequestLastCount=0,DSFV27B_RequestHighWater=0,DSFV27B_RequestFault=0,DSFV27B_RequestExtraReads=0;
static void Log(const char*){}static UPtr caller=0;static void* FixtureCaller(){return reinterpret_cast<void*>(caller);}
#include "GenericRequestBody.inc"
struct CpuRegs{U32 edi,esi,ebp,savedEsp,ebx,edx,ecx,eax,eflags;};
#include "GenericRequestRead.inc"
static UPtr root,context,ai,settings,lists,groups,routes,metadata,weights;
static U32 nativeIds[3]={65000,64000,63000};
static void Configure(U32 capacity){g_limits={capacity,192,2048,64,1,256,1,true,"configured"};Check(GenericValidate(g_limits).valid);}
static void Expect(UPtr owner,const U32* ids,U32 count){const RequestSidecar* s=RequestSlot(owner,false);Check(s && s->count==count && Read32(owner+0xC8u)==count);
    for(U32 i=0;i<count;++i){Check(s->ids[i]==ids[i]);CpuRegs r={};r.esi=owner;r.ebp=i;RequestReadBridge(&r);Check(r.edi==ids[i]);}}
static UPtr Owner(){UPtr p=Allocate(0x1000u);Write32(p+0xDACu,1);Write32(root+0x214u,p);return p;}
static void Submit(UPtr owner,const U32* ids,int count){RequestWriteBridge(reinterpret_cast<void*>(owner),nullptr,ids,count);}
static void Asset(U32 capacity){
    Write32(Address(0xFE7AC0),ai);
    const U32 n=capacity-3u;Write32(ai+0x118,1);Write32(ai+0x11C,settings);Write32(ai+0x120,3);Write32(ai+0x124,lists);
    Write32(ai+0xE8,2);Write32(ai+0xEC,groups);Write32(ai+0x128,2);Write32(ai+0x12C,routes);
    Write32(ai+0x158,2);Write32(ai+0x15C,metadata);Write32(ai+0x160,n+2);Write32(ai+0x164,weights);
    Write32(settings,0xDEADBEEF);*reinterpret_cast<U16*>(settings+4)=0;*reinterpret_cast<U16*>(settings+14)=3;*reinterpret_cast<U8*>(settings+24)=0;
    for(U32 i=0;i<3;++i)*reinterpret_cast<U16*>(lists+i*2)=static_cast<U16>(nativeIds[i]);
    *reinterpret_cast<U16*>(groups+8)=0;*reinterpret_cast<U16*>(groups+20)=1;
    *reinterpret_cast<U8*>(routes+13)=0;*reinterpret_cast<U8*>(routes+33)=1;
    for(U32 table=0;table<2;++table){Write32(metadata+table*12,0);Write32(metadata+table*12+4,n+2);Write32(metadata+table*12+8,0);}
    for(U32 i=0;i<n;++i){Write32(weights+i*8,45000u+i);Write32(weights+i*8+4,0x3F800000u);}
    Write32(weights+n*8,62000);Write32(weights+n*8+4,0);Write32(weights+(n+1)*8,61000);Write32(weights+(n+1)*8+4,0xBF800000u);
    Write32(context+0x5CA840,0xDEADBEEF);Write32(context+0x5CA844,0);
}
extern "C" void __stdcall GenericTestMain(){
    Check(GenericParse("",0).valid);const char good[]="[Limits]\nRequestCapacity=100\nAudioModelEntries=256\nGlobalAudioEntries=4096\nDefaultAudioMiB=128\nReplayReaderMiB=2\nReplayRetainedReferences=512\n[Traffic]\nRosterSource=Native\n";
    GenericLimits parsed=GenericParse(good,sizeof(good)-1);Check(parsed.valid && parsed.requests==100 && parsed.audioModels==256 && parsed.audioSlots==4096 && parsed.audioMiB==128 && parsed.replayMiB==2 && parsed.retained==512 && !parsed.assetWeights);
    const char big[]="[Limits]\nRequestCapacity=700";Check(!GenericParse(big,sizeof(big)-1).valid);
    const char overflow[]="[Limits]\nDefaultAudioMiB=42949672960";Check(!GenericParse(overflow,sizeof(overflow)-1).valid);
    const char duplicate[]="[Limits]\nRequestCapacity=18\nRequestCapacity=100";Check(!GenericParse(duplicate,sizeof(duplicate)-1).valid);
    const char unknown[]="[Limits]\nVehicleSlots=700";Check(!GenericParse(unknown,sizeof(unknown)-1).valid);
    const char badRing[]="[Limits]\nReplayReaderMiB=3";Check(!GenericParse(badRing,sizeof(badRing)-1).valid);
    GenericLoadConfig(nullptr);Check(g_limits.valid && g_limits.requests==150);
    ioError=5;GenericLoadConfig(nullptr);Check(!g_limits.valid);Configure(150);
    ioError=0;ioBytes=good;GenericLoadConfig(nullptr);Check(g_limits.valid && g_limits.requests==100);
    ioReadOk=0;GenericLoadConfig(nullptr);Check(!g_limits.valid);ioReadOk=1;
    Configure(150);Check(GenericParkStride()==604 && GenericParkFrame()==15732 && GenericNotifyFrame()==608 && GenericReplayBytes()==2660);
    Configure(18);Check(GenericRequestStorage()==50 && GenericParkFrame()==5732 && GenericNotifyFrame()==208);
    g_base=Allocate(0xC00000);memcpy(reinterpret_cast<void*>(Address(0x76FDF0)),kGenericSetterBytes,sizeof(kGenericSetterBytes));Check(ChapterPreflight());
    root=Allocate(0x1000);context=Allocate(0x5CB000);ai=Allocate(0x1000);settings=Allocate(0x1000);lists=Allocate(0x1000);groups=Allocate(0x1000);routes=Allocate(0x1000);metadata=Allocate(0x1000);weights=Allocate(0x1000);
    Write32(Address(kRootPtr),root);Write32(root+0x758,context);Write32(Address(0xFE7AC0),ai);
    const U32 capacities[]={18,50,100,150};
    for(U32 capacity:capacities){Configure(capacity);Asset(capacity);UPtr owner=Owner();caller=Address(0x76FE9A);Submit(owner,nativeIds,3);
        RequestSidecar* s=RequestSlot(owner,false);Check(s && s->count==capacity);Check(s->ids[capacity-1]==65000 && s->ids[capacity-2]==64000 && s->ids[capacity-3]==63000);
        for(U32 i=0;i<capacity-3;++i)Check(s->ids[i]==45000u+i);Expect(owner,s->ids,capacity);
        GenericRoster* lease=g_genericRosters;while(lease && lease->owner!=owner)lease=lease->next;Check(lease && !lease->busy);
        // Native selection retains exact order even with unrelated/zero weights.
        g_limits.assetWeights=0;Write32(Address(0xFE7AC0),0);Submit(owner,nativeIds,3);Expect(owner,nativeIds,3);
        caller=Address(0x123456);U32 arbitrary[150];for(U32 i=0;i<capacity;++i)arbitrary[i]=65535u-i;
        Submit(owner,arbitrary,capacity);Expect(owner,arbitrary,capacity);const long rejected=DSFV27B_RequestRejected;
        Submit(owner,arbitrary,capacity+1);Expect(owner,arbitrary,capacity);Check(DSFV27B_RequestRejected==rejected+1);
        Submit(owner,nullptr,0);Expect(owner,nullptr,0);Submit(owner,nullptr,-1);Expect(owner,nullptr,0);
    }
    Configure(150);caller=Address(0x123456);for(U32 n=0;n<24;++n){const UPtr owner=Owner();Submit(owner,nativeIds,3);Expect(owner,nativeIds,3);}
    // A malformed asset cannot publish a partial union. Fatal is stubbed here.
    Configure(18);Asset(50);UPtr owner=Owner();caller=Address(0x76FE9A);const U32* input=nativeIds;int n=3;const U32 before=fatalCount;
    ChapterSelectRequest(caller,owner,input,n);Check(fatalCount==before+1 && fatalRequired==50 && input==nativeIds && n==3);GenericRelease(owner);
    Asset(18);Write32(weights+4,0x7FC00000);input=nativeIds;n=3;ChapterSelectRequest(caller,owner,input,n);
    Check(fatalCount==before+2 && input==nativeIds && n==3);GenericRelease(owner);
    Check(DSFV27B_RequestFault==0);
    const char result[]="generic limits checks passed\n";U32 written=0;WriteFile(GetStdHandle(static_cast<U32>(-11)),result,sizeof(result)-1,&written,nullptr);ExitProcess(0);
}

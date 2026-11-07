// Runs on the existing request thread. No new hooks, native state mutation,
// allocation, release suppression or monitor traversal of live game arrays.
#include "V29sChapterProfiles.h"
enum ChapterReason {ChapterExpand,ChapterOtherCaller,ChapterEmpty,ChapterContext,
    ChapterUnknown,ChapterExcluded,ChapterOwner,ChapterSource,ChapterData,ChapterGraphics,ChapterAdmission};
static volatile long g_chapterStatus=0,g_chapterCalls=0,g_chapterExpanded=0,g_chapterOther=0;
static volatile long g_chapterEmpty=0,g_chapterUnknown=0,g_chapterExcluded=0,g_chapterRefused=0;
struct ChapterCounter {volatile long calls,expanded,refused;};
static ChapterCounter g_chapterCounters[80]={};
static void ChapterEvent(ChapterReason reason,UPtr caller,UPtr owner,U32 hash,U32 frequency,int nativeCount,U32 profile);
static bool ChapterSignature() {
    const UPtr at=Address(0x0076FDF0u);
    if(!Readable(at,sizeof(kChapterSetterBytes)))return false;
    for(U32 i=0;i<sizeof(kChapterSetterBytes);++i)if(Read8(at+i)!=kChapterSetterBytes[i])return false;
    return true;
}
static __declspec(noinline) bool ChapterPreflight() {
    const bool ok=VerifyExecutable() && ChapterSignature();
    InterlockedExchange(&g_chapterStatus,ok?1:-1);return ok;
}
static bool ChapterDataMatches(UPtr ai,const ChapterProfile& p) {
    if(!Readable(ai,0x168u) || Read32(ai+0x118u)!=59u || Read32(ai+0x120u)!=1369u ||
       Read32(ai+0xE8u)!=319u || Read32(ai+0x128u)!=18821u ||
       Read32(ai+0x158u)!=208u || Read32(ai+0x160u)!=39728u)return false;
    const UPtr settings=Read32(ai+0x11Cu),lists=Read32(ai+0x124u);
    const UPtr groups=Read32(ai+0xECu),routes=Read32(ai+0x12Cu);
    const UPtr metadata=Read32(ai+0x15Cu),weights=Read32(ai+0x164u);
    if(!Readable(settings,59u*28u) || !Readable(lists,1369u*2u) ||
       !Readable(groups,319u*12u) || !Readable(routes,18821u*20u) ||
       !Readable(metadata,208u*12u) || !Readable(weights,39728u*8u))return false;
    const UPtr record=settings+p.slot*28u;
    if(Read32(record)!=p.hash || Read8(record+24u)!=p.slot ||
       Read16(record+14u+p.frequency*2u)!=p.count)return false;
    const U32 start=Read16(record+4u+p.frequency*2u);
    if(start>1369u || p.count>1369u-start)return false;
    for(U32 i=0;i<p.count;++i)if(Read16(lists+(start+i)*2u)!=p.ids[i])return false;
    U32 seen[7]={};
    for(U32 group=0;group<319u;++group) {
        const U32 route=Read16(groups+group*12u+8u)+p.slot;
        if(route>=18821u)return false;
        const U32 table=Read8(routes+route*20u+13u+p.frequency);
        bool listed=false;for(U32 j=0;j<p.tableCount;++j)if(table==p.tables[j])listed=true;
        if(!listed)return false;seen[table/32u]|=1u<<(table%32u);
    }
    for(U32 t=0;t<p.tableCount;++t) {
        const U32 table=p.tables[t];
        if(!(seen[table/32u]&(1u<<(table%32u))))return false;
        const UPtr m=metadata+table*12u;
        if(Read32(m)!=table*191u || Read32(m+4u)!=191u || Read32(m+8u)!=0x43160000u)return false;
        const UPtr w=weights+table*191u*8u;
        for(U32 row=0;row<191u;++row)
            if(Read32(w+row*8u)!=kChapterRowIds[row] || Read32(w+row*8u+4u)!=kChapterRowWeights[row])return false;
    }
    return true;
}
static __declspec(noinline) ChapterReason ChapterDecide(UPtr caller,UPtr owner,const U32* list,int count,U32& hash,U32& frequency,U32& index) {
    hash=0u;frequency=0u;index=kChapterProfileCount;
    if(caller!=Address(0x0076FE9Au))return ChapterOtherCaller;
    if(count<=0)return ChapterEmpty;
    if(g_chapterStatus!=1 || !Readable(Address(kRootPtr),4u))return ChapterContext;
    const UPtr root=Read32(Address(kRootPtr));
    if(!Readable(root+0x758u,4u) || !Readable(root+0x214u,4u))return ChapterContext;
    if(Read32(root+0x214u)!=owner)return ChapterOwner;
    const UPtr context=Read32(root+0x758u);
    if(!Readable(context+0x5CA840u,8u))return ChapterContext;
    hash=Read32(context+0x5CA840u);frequency=Read32(context+0x5CA844u);
    for(U32 i=0;i<kChapterProfileCount;++i)
        if(kChapterProfiles[i].hash==hash && kChapterProfiles[i].frequency==frequency){index=i;break;}
    if(index==kChapterProfileCount)return ChapterUnknown;
    const ChapterProfile& p=kChapterProfiles[index];
    if(!p.expand)return ChapterExcluded;
    if(count!=static_cast<int>(p.count) || !Readable(reinterpret_cast<UPtr>(list),p.count*4u))return ChapterSource;
    for(U32 i=0;i<p.count;++i)if(list[i]!=p.ids[i])return ChapterSource;
    if(DSF150_Status!=2)return ChapterGraphics;
    if(DSFV27B_AdmissionAfter!=160)return ChapterAdmission;
    if(!Readable(Address(0x00FE7AC0u),4u) || !ChapterDataMatches(Read32(Address(0x00FE7AC0u)),p))return ChapterData;
    return ChapterExpand;
}
static void ChapterSelectRequest(UPtr caller,UPtr owner,const U32*& list,int& count) {
    U32 hash=0u,frequency=0u,index=kChapterProfileCount;
    const int original=count;
    const ChapterReason reason=ChapterDecide(caller,owner,list,count,hash,frequency,index);
    InterlockedIncrement(&g_chapterCalls);
    if(index<kChapterProfileCount)InterlockedIncrement(&g_chapterCounters[index].calls);
    if(reason==ChapterExpand) {
        list=kChapter7Active150Ids;count=150;
        InterlockedIncrement(&g_chapterExpanded);InterlockedIncrement(&g_chapterCounters[index].expanded);
    } else if(reason==ChapterOtherCaller)InterlockedIncrement(&g_chapterOther);
    else if(reason==ChapterEmpty)InterlockedIncrement(&g_chapterEmpty);
    else if(reason==ChapterUnknown)InterlockedIncrement(&g_chapterUnknown);
    else if(reason==ChapterExcluded)InterlockedIncrement(&g_chapterExcluded);
    else {
        InterlockedIncrement(&g_chapterRefused);
        if(index<kChapterProfileCount)InterlockedIncrement(&g_chapterCounters[index].refused);
    }
    ChapterEvent(reason,caller,owner,hash,frequency,original,index);
}

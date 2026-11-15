#include "V29sGenericSetterNative.inc"
static volatile long g_chapterStatus=0;
static volatile long g_genericCalls=0,g_genericAugmented=0,g_genericPreserved=0,g_genericFailures=0;
struct GenericRoster {GenericRoster* next;UPtr owner;U32 thread,busy,count;U32* ids;U32 seen[2048];};
static GenericRoster* g_genericRosters=nullptr;static volatile long g_genericLock=0;
static void GenericEvent(const char*,UPtr,UPtr,U32,U32,int,U32);
static void GenericFatal(const char*,UPtr,U32);
static void GenericLock(){while(InterlockedExchange(&g_genericLock,1)){} }
static void GenericUnlock(){InterlockedExchange(&g_genericLock,0);}
static void GenericRelease(UPtr owner){const U32 thread=GetCurrentThreadId();GenericLock();
    for(GenericRoster* r=g_genericRosters;r;r=r->next)if(r->owner==owner && r->thread==thread){r->busy=0;break;}GenericUnlock();}
static GenericRoster* GenericLease(UPtr owner){const U32 thread=GetCurrentThreadId();GenericLock();
    GenericRoster* r=g_genericRosters;while(r && (r->owner!=owner || r->thread!=thread))r=r->next;
    if(!r){r=static_cast<GenericRoster*>(VirtualAlloc(nullptr,sizeof(GenericRoster)+g_limits.requests*4u,kMemCommitReserve,kPageReadWrite));
        if(r){r->owner=owner;r->thread=thread;r->ids=reinterpret_cast<U32*>(r+1);r->next=g_genericRosters;g_genericRosters=r;}}
    if(!r || r->busy){GenericUnlock();GenericFatal(r?"rosterReentrant":"rosterAllocation",owner,0);return nullptr;}
    r->busy=1;r->count=0;for(U32 i=0;i<2048;++i)r->seen[i]=0;GenericUnlock();return r;
}
static bool ChapterSignature(){bool okay=true;const UPtr at=Address(0x76FDF0u);
    if(!Readable(at,sizeof(kGenericSetterBytes)))okay=false;
    if(okay)for(U32 i=0;i<sizeof(kGenericSetterBytes);++i)if(Read8(at+i)!=kGenericSetterBytes[i])okay=false;
    return okay;}
static bool ChapterPreflight(){const bool okay=VerifyExecutable() && ChapterSignature();
    InterlockedExchange(&g_chapterStatus,okay?1:-1);return okay;}
static bool GenericAdd(GenericRoster* r,U32 id,bool native){
    if(!id || id>65535u){GenericFatal("assetModelId",r->owner,id);return false;}
    const U32 bit=1u<<(id&31u);if(!native && (r->seen[id/32u]&bit))return true;
    r->seen[id/32u]|=bit;
    // Count the full requirement even when storage is too small; never write
    // beyond it or publish the partially collected private list.
    if(r->count<g_limits.requests)r->ids[r->count]=id;
    ++r->count;return true;
}
static bool GenericSpan(UPtr p,U32 count,U32 stride,U32 maximum){
#if defined(DSF_V29S_WIDE_LIMITS)
    // Setting/probability-table selectors are bytes. Other array counts are
    // DWORDs; validate their byte geometry instead of a test-sized ceiling.
    if((maximum==256u && count>256u) || !WideSize(count,stride))return false;
    const U32 bytes=count*stride;return !count || (p && p<=0xFFFFFFFFu-bytes && Readable(p,bytes));
#else
    return count<=maximum && (!count || (p && Readable(p,count*stride)));
#endif
}
static void ChapterSelectRequest(UPtr caller,UPtr owner,const U32*& list,int& count){
    InterlockedIncrement(&g_genericCalls);U32 hash=0,frequency=0;
    const char* nativeReason=count<=0?"empty":!g_limits.assetWeights?"nativeRoster":"otherCaller";
    if(count<=0 || !g_limits.assetWeights || caller!=Address(0x76FE9Au)){
        InterlockedIncrement(&g_genericPreserved);GenericEvent(nativeReason,caller,owner,0,0,count,count>0?count:0);return;}
    const UPtr root=Read32(Address(kRootPtr));
    if(g_chapterStatus!=1 || !root || !Readable(root+0x758u,4u) || !Readable(root+0x214u,4u) || Read32(root+0x214u)!=owner){GenericFatal("requestContext",owner,0);return;}
    const UPtr context=Read32(root+0x758u);if(!Readable(context+0x5CA840u,8u)){GenericFatal("settingContext",owner,0);return;}
    hash=Read32(context+0x5CA840u);frequency=Read32(context+0x5CA844u);
    const UPtr ai=Read32(Address(0xFE7AC0u));if(frequency>=5u || !Readable(ai,0x168u)){GenericFatal("assetHeader",owner,frequency);return;}
    const U32 settingsN=Read32(ai+0x118u),listsN=Read32(ai+0x120u),groupsN=Read32(ai+0xE8u),routesN=Read32(ai+0x128u),tablesN=Read32(ai+0x158u),weightsN=Read32(ai+0x160u);
    const UPtr settings=Read32(ai+0x11Cu),lists=Read32(ai+0x124u),groups=Read32(ai+0xECu),routes=Read32(ai+0x12Cu),metadata=Read32(ai+0x15Cu),weights=Read32(ai+0x164u);
    if(!settingsN || !groupsN || !tablesN || !GenericSpan(settings,settingsN,28u,256u) || !GenericSpan(lists,listsN,2u,1048576u) ||
       !GenericSpan(groups,groupsN,12u,65536u) || !GenericSpan(routes,routesN,20u,1048576u) || !GenericSpan(metadata,tablesN,12u,256u) || !GenericSpan(weights,weightsN,8u,1048576u)){
        GenericFatal("assetArrays",owner,settingsN);return;}
    U32 slot=settingsN;
    for(U32 i=0;i<settingsN;++i)if(Read32(settings+i*28u)==hash){if(slot!=settingsN){GenericFatal("duplicateSettingHash",owner,hash);return;}slot=i;}
    if(slot==settingsN){InterlockedIncrement(&g_genericPreserved);GenericEvent("nativeUnmappedSetting",caller,owner,hash,frequency,count,count);return;}
    const UPtr record=settings+slot*28u;const U32 start=Read16(record+4u+frequency*2u),nativeCount=Read16(record+14u+frequency*2u);
    if(Read8(record+24u)!=slot || nativeCount!=static_cast<U32>(count) || start>listsN || nativeCount>listsN-start){GenericFatal("nativeAssetRoster",owner,count);return;}
    for(U32 i=0;i<nativeCount;++i)if(Read16(lists+(start+i)*2u)!=list[i]){GenericFatal("nativeAssetIds",owner,i);return;}
    GenericRoster* r=GenericLease(owner);if(!r)return;
    for(U32 i=0;i<nativeCount;++i)if(!GenericAdd(r,list[i],true))return;
    U32 visited[8]={};
    for(U32 group=0;group<groupsN;++group){const U32 route=Read16(groups+group*12u+8u)+slot;
        if(route>=routesN){GenericFatal("assetRoute",owner,route);return;}
        const U32 table=Read8(routes+route*20u+13u+frequency);if(table>=tablesN){GenericFatal("assetProbabilityTable",owner,table);return;}
        const U32 bit=1u<<(table&31u);if(visited[table/32u]&bit)continue;visited[table/32u]|=bit;
        const U32 first=Read32(metadata+table*12u),rows=Read32(metadata+table*12u+4u);
        if(first>weightsN || rows>weightsN-first){GenericFatal("assetWeightRange",owner,table);return;}
        for(U32 j=0;j<rows;++j){const UPtr row=weights+(first+j)*8u;const U32 weight=Read32(row+4u);
            if((weight&0x7F800000u)==0x7F800000u){GenericFatal("nonfiniteWeight",owner,table);return;}
            if(!(weight&0x80000000u) && (weight&0x7FFFFFFFu) && !GenericAdd(r,Read32(row),false))return;}
    }
    if(r->count>g_limits.requests){GenericFatal("rosterExceedsConfiguredCapacity",owner,r->count);return;}
    const int native=count;
    // Stable generic order for newly augmented lists; no compiled model roster.
    if(r->count>nativeCount)for(U32 i=1;i<r->count;++i){const U32 value=r->ids[i];U32 j=i;while(j && r->ids[j-1]>value){r->ids[j]=r->ids[j-1];--j;}r->ids[j]=value;}
    list=r->ids;count=static_cast<int>(r->count);
    if(count!=native)InterlockedIncrement(&g_genericAugmented);else InterlockedIncrement(&g_genericPreserved);
    GenericEvent(count!=native?"assetRoster":"nativeRoster",caller,owner,hash,frequency,native,r->count);
}

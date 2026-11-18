//192 external nodes, retaining the native provider/sentinel/default layout.
#if defined(DSF_V29S_GENERIC_LIMITS)
#define kAudioModelCapacity (g_limits.audioModels)
#define kAudioModelStride ((g_limits.audioModels+1u)*16u)
#else
static const U32 kAudioModelCapacity=192u,kAudioModelStride=193u*16u;
#endif
#if defined(DSF_V29S_WIDE_LIMITS)
static const U32 kAudioModelNodeBytes=16u,kAudioModelGuard=0x4D313932u;
#define kAudioModelInstances (g_audioModelInstances.count)
#else
static const U32 kAudioModelInstances=8u,kAudioModelNodeBytes=16u,kAudioModelGuard=0x4D313932u;
#endif
static UPtr g_audioModelStorage=0;
struct AudioModelInstance {
#if defined(DSF_V29S_WIDE_LIMITS)
    UPtr storage;
#endif
    volatile long owner,state,generations,attempts,stores,full,wouldStockFull,removes,hits,misses,peak,current;
};
struct AudioModelKey {
#if defined(DSF_V29S_WIDE_LIMITS)
    AudioModelKey* hashNext;U32 reportedVersion;
#endif
    volatile long key,attempts,stores,full,wouldStockFull,removes,hits,misses,fallbackHit,fallbackMiss;
    volatile long setup,disabled,bankReady,bankNotReady,setupOk,setupFail,lastProfile,lastSelected;
};
struct AudioModelDetail { volatile long ready;U32 tick,kind,owner,model,a,b,c,d; };
#if defined(DSF_V29S_WIDE_LIMITS)
static void WideInitialize(AudioModelInstance& r){r.storage=reinterpret_cast<UPtr>(WideAllocate(kAudioModelStride));for(U32 n=0;n<4u;++n)*reinterpret_cast<U32*>(r.storage+kAudioModelCapacity*16u+n*4u)=kAudioModelGuard;}
static WideDirectory<AudioModelInstance> g_audioModelInstances={};
static volatile long g_wideAudioInstanceLock=0;
#else
static AudioModelInstance g_audioModelInstances[8]={};
#endif
#if defined(DSF_V29S_WIDE_LIMITS)
static void WideInitialize(AudioModelKey&){}
static WideDirectory<AudioModelKey> g_audioModelKeys={};
static AudioModelKey g_audioModelZeroKey={};
static AudioModelKey** g_wideAudioBuckets=nullptr;
static volatile long g_wideAudioKeyLock=0;
#else
static AudioModelKey g_audioModelKeys[512]={},g_audioModelZeroKey={};
#endif
static AudioModelDetail g_audioModelDetails[7][256]={};
static const U32 kAudioModelBounds[7]={256u,256u,64u,128u,128u,128u,128u};
static volatile long g_audioModelDetailCounts[7]={},g_audioModelCalls[8]={};
static volatile long g_audioModelInvalid=0,g_audioModelInstanceOverflow=0,g_audioModelKeyOverflow=0;
static volatile long g_audioModelAttachments=0,g_audioModelAttachRefused=0,g_audioModelFallbackHit=0,g_audioModelFallbackMiss=0;
#if defined(DSF_V29S_WIDE_LIMITS)
static AudioModelKey* AudioModelFindKey(U32 key){
    if(!key)return &g_audioModelZeroKey;while(InterlockedExchange(&g_wideAudioKeyLock,1)){}
    const U32 bucket=key&65535u;AudioModelKey* p=g_wideAudioBuckets[bucket];while(p && static_cast<U32>(p->key)!=key)p=p->hashNext;
    if(!p){p=&g_audioModelKeys.Append();p->key=static_cast<long>(key);p->hashNext=g_wideAudioBuckets[bucket];g_wideAudioBuckets[bucket]=p;}
    InterlockedExchange(&g_wideAudioKeyLock,0);return p;
}
#else
static AudioModelKey* AudioModelFindKey(U32 key) {
    if(!key)return &g_audioModelZeroKey;
    for(U32 n=0;n<512u;++n) {
        AudioModelKey& r=g_audioModelKeys[(key+n)&511u];
        long seen=InterlockedCompareExchange(&r.key,0,0);
        if(static_cast<U32>(seen)==key)return &r;
        if(!seen){seen=InterlockedCompareExchange(&r.key,static_cast<long>(key),0);if(!seen || static_cast<U32>(seen)==key)return &r;}
    }
    InterlockedIncrement(&g_audioModelKeyOverflow);return nullptr;
}
#endif
#if defined(DSF_V29S_WIDE_LIMITS)
static AudioModelInstance* AudioModelFindInstance(UPtr owner,bool create){
    while(InterlockedExchange(&g_wideAudioInstanceLock,1)){}AudioModelInstance* result=nullptr;
    for(auto& r:g_audioModelInstances)if(static_cast<UPtr>(r.owner)==owner){result=&r;break;}
    if(!result && create){result=&g_audioModelInstances.Append();result->owner=static_cast<long>(owner);}
    InterlockedExchange(&g_wideAudioInstanceLock,0);return result;
}
#else
static AudioModelInstance* AudioModelFindInstance(UPtr owner,bool create) {
    for(U32 i=0;i<8u;++i) {
        AudioModelInstance& r=g_audioModelInstances[i];long seen=InterlockedCompareExchange(&r.owner,0,0);
        if(static_cast<U32>(seen)==owner)return &r;
        if(!seen && create){seen=InterlockedCompareExchange(&r.owner,static_cast<long>(owner),0);if(!seen || static_cast<U32>(seen)==owner)return &r;}
    }
    if(create)InterlockedIncrement(&g_audioModelInstanceOverflow);return nullptr;
}
#endif
#if defined(DSF_V29S_WIDE_LIMITS)
static UPtr AudioModelBuffer(const AudioModelInstance* r){return r->storage;}
#else
static UPtr AudioModelBuffer(const AudioModelInstance* r) {
    return g_audioModelStorage+static_cast<U32>(r-g_audioModelInstances)*kAudioModelStride;
}
#endif
static void AudioModelRecord(U32 kind,UPtr owner,U32 model,U32 a,U32 b,U32 c=0u,U32 d=0u) {
    const long n=InterlockedIncrement(&g_audioModelDetailCounts[kind]);
    if(n<=0 || static_cast<U32>(n)>kAudioModelBounds[kind])return;
    AudioModelDetail& r=g_audioModelDetails[kind][n-1];r.tick=GetTickCount()-g_capacityStart;
    r.kind=kind;r.owner=owner;r.model=model;r.a=a;r.b=b;r.c=c;r.d=d;InterlockedExchange(&r.ready,1);
}
static bool AudioModelEmptyNative(UPtr owner) {
    return !Read32(owner+8u) && !Read32(owner+12u) && Read32(owner+4u)==owner+0x10u &&
        Read32(owner+0x2E0u)==owner+0x2E0u && Read32(owner+0x2E4u)==owner+0x2E0u;
}
static U32 AudioModelPrepare(AudioModelInstance* r,UPtr owner) {
    if(!r || !g_audioModelStorage)return 45u;
    if(Read32(owner)!=Address(0x00DE3824u)){InterlockedIncrement(&g_audioModelInvalid);return 45u;}
    // A constructor/reset can reuse the same owner address. Reattach only the
    // pristine empty native header; never migrate live entries.
    if(AudioModelEmptyNative(owner)) {
        Write32(owner+4u,AudioModelBuffer(r));
        InterlockedIncrement(&r->generations);InterlockedExchange(&r->state,1);
        InterlockedIncrement(&g_audioModelAttachments);
        AudioModelRecord(2u,owner,0u,kAudioModelCapacity,AudioModelBuffer(r),Read32(owner+0x2ECu));
    } else if(!r->state) {
        InterlockedExchange(&r->state,-1);InterlockedIncrement(&g_audioModelAttachRefused);
        AudioModelRecord(2u,owner,0u,45u,Read32(owner+4u),Read32(owner+12u));
    }
    return r->state==1?kAudioModelCapacity:45u;
}
static U32 AudioModelCompareFlags(U32 count,U32 cap,U32 previous) {
    U32 flags=0;
    __asm {mov eax,count
        cmp eax,cap
        pushfd
        pop flags}
    return (previous&~0x8D5u)|(flags&0x8D5u);
}
static U32 AudioModelTestFlags(U32 value,U32 previous) {
    U32 flags=0;
    __asm {mov eax,value
        test al,al
        pushfd
        pop flags}
    return (previous&~0x8C5u)|(flags&0x8C5u);
}
static void AudioModelObserve(AudioModelInstance* r,UPtr owner,U32 count) {
    if(!r)return;InterlockedExchange(&r->current,static_cast<long>(count));CapacityMax(r->peak,count);
    if(count>(r->state==1?kAudioModelCapacity:45u))InterlockedIncrement(&g_audioModelInvalid);
    if(r->state==1) {
        const U32 cursor=Read32(owner+4u),base=AudioModelBuffer(r);
        if(cursor<base || cursor>base+kAudioModelCapacity*16u || ((cursor-base)&15u))InterlockedIncrement(&g_audioModelInvalid);
        for(U32 i=0;i<4u;++i)if(Read32(base+kAudioModelCapacity*16u+i*4u)!=kAudioModelGuard)InterlockedIncrement(&g_audioModelInvalid);
    }
}
#if defined(DSF_V29S_AUDIO_SETUP_TRACE)
static void AudioSetupBankFailure(U32* frame);
#endif
extern "C" __declspec(noinline) void __cdecl AudioModelEntry(U32 kind,U32* frame) {
    if(kind>=8u)return;InterlockedIncrement(&g_audioModelCalls[kind]);
    const UPtr f=reinterpret_cast<UPtr>(frame),sp=f+36u;
    if(kind==0u) {
        const UPtr owner=Read32(f+24u);const U32 model=Read32(sp+8u),data=Read32(sp+4u),count=Read32(owner+12u);
        AudioModelInstance* r=AudioModelFindInstance(owner,true);const U32 cap=AudioModelPrepare(r,owner);
        AudioModelObserve(r,owner,count);AudioModelKey* k=AudioModelFindKey(model);
        if(r)InterlockedIncrement(&r->attempts);if(k){InterlockedIncrement(&k->attempts);InterlockedExchange(&k->lastProfile,static_cast<long>(data));}
        if(count>=45u){if(r)InterlockedIncrement(&r->wouldStockFull);if(k)InterlockedIncrement(&k->wouldStockFull);}
        const bool full=count>=cap;
        if(full){if(r)InterlockedIncrement(&r->full);if(k)InterlockedIncrement(&k->full);}
        AudioModelRecord(0u,owner,model,data,count,cap,full?1u:0u);
        // POPAD deliberately ignores its saved-ESP word. Use it as a per-call
        // continuation slot; the naked bridge reads it at native ESP-24.
        frame[3]=Address(full?0x00556E49u:0x00556E36u);
        frame[8]=AudioModelCompareFlags(count,cap,frame[8]);return;
    }
    if(kind==1u) {
        const UPtr value=Read32(f+28u);AudioModelInstance* r=nullptr;
        for(U32 i=0;i<8u;++i) {
            AudioModelInstance& s=g_audioModelInstances[i];if(!s.owner)continue;
            const U32 base=s.state==1?AudioModelBuffer(&s):static_cast<U32>(s.owner)+0x10u;
            const U32 cap=s.state==1?kAudioModelCapacity:45u;
            if(value>=base+12u && value<base+cap*16u && ((value-base)&15u)==12u){r=&s;break;}
        }
        if(!r){InterlockedIncrement(&g_audioModelInvalid);return;}
        const UPtr owner=static_cast<U32>(r->owner);const U32 model=Read32(sp+8u),data=Read32(sp+4u),count=Read32(owner+12u);
        if(Read32(value-4u)!=model)InterlockedIncrement(&g_audioModelInvalid);
        InterlockedIncrement(&r->stores);AudioModelObserve(r,owner,count);
        AudioModelKey* k=AudioModelFindKey(model);if(k)InterlockedIncrement(&k->stores);
        AudioModelRecord(1u,owner,model,data,count,r->state==1?kAudioModelCapacity:45u);return;
    }
    if(kind==2u) {
        const UPtr owner=Read32(f+4u)-4u,node=Read32(f+28u);const U32 model=Read32(node+8u),count=Read32(owner+12u);
        AudioModelInstance* r=AudioModelFindInstance(owner,false);if(r)InterlockedIncrement(&r->removes);
        AudioModelObserve(r,owner,count);if(!count)InterlockedIncrement(&g_audioModelInvalid);
        AudioModelKey* k=AudioModelFindKey(model);if(k)InterlockedIncrement(&k->removes);return;
    }
    if(kind==3u) {
        const UPtr owner=Read32(f),node=Read32(f+28u);const U32 model=Read32(sp+0x14u),caller=Read32(sp+0x10u);
        const U32 value=node==owner+0x2E0u?0u:Read32(node+12u);
        AudioModelInstance* r=AudioModelFindInstance(owner,true);AudioModelObserve(r,owner,Read32(owner+12u));
        AudioModelKey* k=AudioModelFindKey(model);
        if(value){if(r)InterlockedIncrement(&r->hits);if(k)InterlockedIncrement(&k->hits);}
        else {if(r)InterlockedIncrement(&r->misses);if(k)InterlockedIncrement(&k->misses);AudioModelRecord(3u,owner,model,caller,Read32(owner+12u));}
        if(caller==Address(0x005538EFu)) {
            const UPtr controller=Read32(sp+4u),vehicle=Read32(controller+0xB4u);const U32 original=Read32(vehicle+0x5ACu);
            AudioModelKey* originalKey=AudioModelFindKey(original);
            if(value){InterlockedIncrement(&g_audioModelFallbackHit);if(originalKey)InterlockedIncrement(&originalKey->fallbackHit);}
            else {InterlockedIncrement(&g_audioModelFallbackMiss);if(originalKey)InterlockedIncrement(&originalKey->fallbackMiss);}
            AudioModelRecord(4u,owner,original,model,value,caller);
        }
        return;
    }
    const UPtr controller=kind==4u?Read32(f+24u):Read32(f+4u);
    const UPtr vehicle=Read32(sp+(kind==4u?4u:8u));const U32 original=Read32(vehicle+0x5ACu);
    AudioModelKey* k=AudioModelFindKey(original);
    if(kind==4u) {
        if(k){InterlockedIncrement(&k->setup);if(!Read8(vehicle+0x5E7u))InterlockedIncrement(&k->disabled);}
        return;
    }
    if(kind==5u) {
        const bool ready=(Read32(f+28u)&255u)!=0u;const U32 selected=Read32(controller+0x10u);
#if defined(DSF_V29S_AUDIO_SETUP_TRACE)
        if(!ready)AudioSetupBankFailure(frame);
#endif
        if(k){if(ready)InterlockedIncrement(&k->bankReady);else InterlockedIncrement(&k->bankNotReady);InterlockedExchange(&k->lastSelected,static_cast<long>(selected));}
        AudioModelRecord(5u,controller,original,selected,ready?1u:0u,Read32(controller+0xBCu),Read32(controller+0xC0u));
        frame[3]=Address(ready?0x00553920u:0x00553AD5u);
        frame[8]=AudioModelTestFlags(Read32(f+28u),frame[8]);return;
    }
    if(k){if(kind==6u)InterlockedIncrement(&k->setupOk);else InterlockedIncrement(&k->setupFail);}
    AudioModelRecord(6u,controller,original,kind==6u?1u:0u,Read8(vehicle+0x5E7u));
}

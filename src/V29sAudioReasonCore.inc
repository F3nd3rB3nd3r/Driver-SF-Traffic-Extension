// Passive native-thread observations. No live object walks on the monitor.
struct AudioReasonOwner {
    volatile long key,registrations,requests,refused,accepted,zero,noneRequests;
    volatile long peakObserved,acceptedPeakProjected,lastCount,callbacks,ends,unmatched,removes,lookupHit,lookupMiss;
};
struct AudioReasonEvent {
    volatile long key,lookupHit,lookupMiss,createFailed,requests,refused,accepted,zero,firstOwner,lastOwner,firstCaller,lastCaller;
};
struct AudioReasonDetail { volatile long ready;U32 tick,a,b,c,d,e;char name[64]; };
static AudioReasonOwner g_audioReasonOwners[128]={};
static AudioReasonEvent g_audioReasonEvents[512]={},g_audioReasonZeroEvent={};
static AudioReasonDetail g_audioReasonDetails[7][256]={};
static const U32 kAudioReasonBounds[7]={64u,256u,64u,64u,128u,64u,128u};
static volatile long g_audioReasonDetailCounts[7]={};
static volatile long g_audioReasonCalls[10]={},g_audioReasonLookupHit=0,g_audioReasonLookupMiss=0;
static volatile long g_audioReasonFull=0,g_audioReasonAccepted=0,g_audioReasonZero=0,g_audioReasonEndsDelivered=0;
static volatile long g_audioReasonEndsHandled=0,g_audioReasonRemoves=0,g_audioReasonInvalid=0;
static volatile long g_audioReasonOwnerOverflow=0,g_audioReasonEventOverflow=0,g_audioReasonPeak=0;
static AudioReasonOwner* AudioReasonFindOwner(U32 key,bool create) {
    if(!key)return nullptr;
    for(U32 n=0;n<128u;++n) {
        AudioReasonOwner& s=g_audioReasonOwners[((key>>3u)+n)&127u];
        long seen=InterlockedCompareExchange(&s.key,0,0);
        if(static_cast<U32>(seen)==key)return &s;
        if(!seen && create) {
            seen=InterlockedCompareExchange(&s.key,static_cast<long>(key),0);
            if(!seen || static_cast<U32>(seen)==key)return &s;
        }
        if(!seen && !create)return nullptr;
    }
    if(create)InterlockedIncrement(&g_audioReasonOwnerOverflow);return nullptr;
}
static AudioReasonEvent* AudioReasonFindEvent(U32 key) {
    if(!key)return &g_audioReasonZeroEvent;
    for(U32 n=0;n<512u;++n) {
        AudioReasonEvent& s=g_audioReasonEvents[(key+n)&511u];
        long seen=InterlockedCompareExchange(&s.key,0,0);
        if(static_cast<U32>(seen)==key)return &s;
        if(!seen) {
            seen=InterlockedCompareExchange(&s.key,static_cast<long>(key),0);
            if(!seen || static_cast<U32>(seen)==key)return &s;
        }
    }
    InterlockedIncrement(&g_audioReasonEventOverflow);return nullptr;
}
static void AudioReasonEventContext(AudioReasonEvent* s,U32 owner,U32 caller) {
    if(!s)return;
    if(owner){InterlockedCompareExchange(&s->firstOwner,static_cast<long>(owner),0);InterlockedExchange(&s->lastOwner,static_cast<long>(owner));}
    if(caller){InterlockedCompareExchange(&s->firstCaller,static_cast<long>(caller),0);InterlockedExchange(&s->lastCaller,static_cast<long>(caller));}
}
static AudioReasonDetail* AudioReasonRecord(U32 kind,U32 a,U32 b,U32 c,U32 d,U32 e) {
    const long ordinal=InterlockedIncrement(&g_audioReasonDetailCounts[kind]);
    if(ordinal<=0 || static_cast<U32>(ordinal)>kAudioReasonBounds[kind])return nullptr;
    AudioReasonDetail& r=g_audioReasonDetails[kind][ordinal-1];
    r.tick=GetTickCount()-g_capacityStart;r.a=a;r.b=b;r.c=c;r.d=d;r.e=e;return &r;
}
static void AudioReasonPublish(AudioReasonDetail* r){if(r)InterlockedExchange(&r->ready,1);}
static void AudioReasonCount(AudioReasonOwner* s,U32 count) {
    if(count>16u)InterlockedIncrement(&g_audioReasonInvalid);
    if(!s)return;
    InterlockedExchange(&s->lastCount,static_cast<long>(count));
    CapacityMax(s->peakObserved,count);CapacityMax(g_audioReasonPeak,count);
}
extern "C" __declspec(noinline) void __cdecl AudioReasonEntry(U32 kind,const U32* frame) {
    if(kind>=10u)return;InterlockedIncrement(&g_audioReasonCalls[kind]);
    const UPtr f=reinterpret_cast<UPtr>(frame),sp=f+36u;
    if(kind==0u || kind==1u) {
        const U32 event=Read32(sp+(kind?0x30u:0x34u)),object=Read32(sp+(kind?0x34u:0x38u));
        const U32 value=Read32(f+28u);AudioReasonEvent* e=AudioReasonFindEvent(event);
        AudioReasonEventContext(e,object,0u);
        if(kind==1u) {
            if(e)InterlockedIncrement(&e->createFailed);
            AudioReasonPublish(AudioReasonRecord(2u,event,object,value,0u,0u));return;
        }
        AudioReasonOwner* s=AudioReasonFindOwner(object,false); // scalar table only; never dereference a gameObject ID
        if(value) {
            InterlockedIncrement(&g_audioReasonLookupHit);if(e)InterlockedIncrement(&e->lookupHit);
            if(s)InterlockedIncrement(&s->lookupHit);
        } else {
            InterlockedIncrement(&g_audioReasonLookupMiss);if(e)InterlockedIncrement(&e->lookupMiss);
            if(s)InterlockedIncrement(&s->lookupMiss);
            AudioReasonPublish(AudioReasonRecord(event==0x2CA33BDBu?0u:1u,event,object,Read32(sp+(kind?0x2Cu:0x30u)),0u,0u));
        }
        return;
    }
    if(kind==4u) { // callback entry: cookie may be stale, so do not dereference it here
        if(Read32(sp+4u)==1u)InterlockedIncrement(&g_audioReasonEndsDelivered);return;
    }
    if(kind==7u || kind==8u) {
        const U32 object=Read32(f+24u),name=Read32(sp+4u),caller=Read32(sp);
        AudioReasonDetail* r=AudioReasonRecord(6u,0u,object,caller,name,0u);if(!r)return;
        U32 hash=0x811C9DC5u;
        if(!name){r->a=0u;r->e=4u;AudioReasonPublish(r);return;}
        for(U32 i=0;i<63u;++i) {
            if(!Readable(name+i,1u)){r->e=2u;break;}
            const U8 byte=Read8(name+i);r->name[i]=static_cast<char>(byte);
            if(!byte){r->e=1u;break;}
            const U32 lower=byte>='A' && byte<='Z'?byte+32u:byte;
            hash=hash*0x01000193u ^ lower;
        }
        r->a=r->e==1u?hash:0u;AudioReasonPublish(r);return;
    }
    const U32 object=kind==2u || kind==3u?Read32(f+4u):kind==6u?Read32(f+24u)-0x2Cu:Read32(f+24u);
    // Only wrapper/validated callback/erase/registration paths reach here.
    const U32 count=Read32(object+0x16Cu);
    AudioReasonOwner* s=AudioReasonFindOwner(object,true);AudioReasonCount(s,count);
    if(kind==2u || kind==3u) {
        const U32 event=kind==2u?Read32(sp+0x24u):Read32(f+8u);
        AudioReasonEvent* e=AudioReasonFindEvent(event);
        const U32 caller=kind==2u?Read32(sp+0x20u):0u;AudioReasonEventContext(e,object,caller);
        if(kind==2u) {
            if(s){InterlockedIncrement(&s->requests);if(event==0x2CA33BDBu)InterlockedIncrement(&s->noneRequests);}
            if(e)InterlockedIncrement(&e->requests);
            if(count>=16u) {
                InterlockedIncrement(&g_audioReasonFull);if(s)InterlockedIncrement(&s->refused);if(e)InterlockedIncrement(&e->refused);
                AudioReasonPublish(AudioReasonRecord(3u,event,object,caller,count,0u));
            }
        } else if(Read32(f+28u)) {
            InterlockedIncrement(&g_audioReasonAccepted);if(e)InterlockedIncrement(&e->accepted);
            if(s){InterlockedIncrement(&s->accepted);if(count<=16u)CapacityMax(s->acceptedPeakProjected,count+1u);}
        } else {
            InterlockedIncrement(&g_audioReasonZero);if(s)InterlockedIncrement(&s->zero);if(e)InterlockedIncrement(&e->zero);
            AudioReasonPublish(AudioReasonRecord(4u,event,object,0u,count,0u));
        }
    } else if(kind==5u) {
        const U32 type=Read32(sp+4u),info=Read32(sp+8u),playing=Read32(info+8u);
        if(s)InterlockedIncrement(&s->callbacks);
        if(type==1u) {
            InterlockedIncrement(&g_audioReasonEndsHandled);if(s)InterlockedIncrement(&s->ends);
            bool found=false;
            if(count<=16u)for(U32 i=0;i<count;++i)if(Read32(object+0x30u+i*20u)==playing){found=true;break;}
            if(!found){if(s)InterlockedIncrement(&s->unmatched);AudioReasonPublish(AudioReasonRecord(5u,playing,object,type,count,0u));}
        }
    } else if(kind==6u) {
        InterlockedIncrement(&g_audioReasonRemoves);if(s)InterlockedIncrement(&s->removes);
        if(!count)InterlockedIncrement(&g_audioReasonInvalid);
        // The original decrement runs immediately on return; do not write native state.
    } else if(kind==9u) {if(s)InterlockedIncrement(&s->registrations);}
}

#if !defined(DSF_V29S_AUDIO_SLOTS_2048)
#error Audio reason probe requires the existing2048/64MiB comparison baseline
#endif
#include "V29sAudioReasonCore.inc"
extern "C" UPtr g_audioReasonTramps[10];
#include "V29sAudioReasonBridges.inc"
#include "V29sAudioReasonPatch.inc"
static void AudioReasonStartLine() {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_REASON_START revision=1 status=",end);AppendNumber(p,g_audioReasonStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioReasonReason,end);
    DiagFAppendKeyU(p," hooks=",g_audioReasonInstalled,end);
    AppendText(p," expectedHooks=10 changedLimits=0 objectEventLimit=16 globalMax=2048 defaultBytes=67108864 ownerBound=128 eventBound=512 detailBounds=64,256,64,64,128,64,128\n",end);
    *p=0;CapacityWrite(line);
}
static void AudioReasonDrain() {
    static U32 printed[7]={};
    const char* kinds[7]={"noneLookupMissing","lookupMissing","createFailed","objectFull","wrapperPostZero","callbackUnmatched","inputName"};
    for(U32 kind=0;kind<7u;++kind)while(printed[kind]<kAudioReasonBounds[kind]) {
        const AudioReasonDetail& r=g_audioReasonDetails[kind][printed[kind]];if(!r.ready)break;
        char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_REASON_DETAIL kind=",end);AppendText(p,kinds[kind],end);
        DiagFAppendKeyU(p," ordinal=",++printed[kind],end);DiagFAppendKeyU(p," tick=",r.tick,end);
        DiagFAppendKeyHex(p," a=",r.a,end);DiagFAppendKeyHex(p," owner=",r.b,end);
        DiagFAppendKeyHex(p," c=",r.c,end);DiagFAppendKeyU(p," d=",r.d,end);DiagFAppendKeyU(p," flags=",r.e,end);
        if(kind==6u) {
            AppendText(p," nameHex=",end);const char* hex="0123456789ABCDEF";
            for(U32 i=0;i<63u && r.name[i] && p+2<end;++i){const U8 b=static_cast<U8>(r.name[i]);*p++=hex[b>>4u];*p++=hex[b&15u];}
        }
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}
static void AudioReasonPoll() {
    if(g_audioReasonStatus!=1)return;AudioReasonDrain();
    static U32 last=0,tables=0;const U32 now=GetTickCount()-g_capacityStart;
    if(last && now-last<1000u)return;last=now;
    char line[1600]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_REASON_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    const char* callKeys[10]={" lookupCalls="," createFailureCalls="," wrapperRequests="," wrapperReturns="," deliveredCallbacks="," handledCallbacks="," removes="," nameCalls="," nameDefaultCalls="," registrations="};
    for(U32 i=0;i<10u;++i)DiagFAppendKeyU(p,callKeys[i],static_cast<U32>(g_audioReasonCalls[i]),end);
    DiagFAppendKeyU(p," lookupHit=",static_cast<U32>(g_audioReasonLookupHit),end);
    DiagFAppendKeyU(p," lookupMiss=",static_cast<U32>(g_audioReasonLookupMiss),end);
    DiagFAppendKeyU(p," full16=",static_cast<U32>(g_audioReasonFull),end);
    DiagFAppendKeyU(p," wrapperAccepted=",static_cast<U32>(g_audioReasonAccepted),end);
    DiagFAppendKeyU(p," wrapperZero=",static_cast<U32>(g_audioReasonZero),end);
    DiagFAppendKeyU(p," endsDelivered=",static_cast<U32>(g_audioReasonEndsDelivered),end);
    DiagFAppendKeyU(p," endsHandled=",static_cast<U32>(g_audioReasonEndsHandled),end);
    DiagFAppendKeyU(p," peakObserved=",static_cast<U32>(g_audioReasonPeak),end);
    DiagFAppendKeyU(p," ownerOverflow=",static_cast<U32>(g_audioReasonOwnerOverflow),end);
    DiagFAppendKeyU(p," eventOverflow=",static_cast<U32>(g_audioReasonEventOverflow),end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_audioReasonInvalid),end);
    const char* detailKeys[7]={" noneMissing="," otherMissing="," createDetails="," fullDetails="," zeroDetails="," unmatchedEnds="," nameDetails="};
    for(U32 i=0;i<7u;++i)DiagFAppendKeyU(p,detailKeys[i],static_cast<U32>(g_audioReasonDetailCounts[i]),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    if(tables && now-tables<5000u)return;tables=now;
    static U32 ownerPrinted[128]={},eventPrinted[513]={};
    for(U32 i=0;i<128u;++i) {
        const AudioReasonOwner& s=g_audioReasonOwners[i];if(!s.key)continue;
        const U32 version=static_cast<U32>(s.requests+s.accepted+s.zero+s.callbacks+s.removes+s.registrations+s.lookupHit+s.lookupMiss);
        if(version==ownerPrinted[i])continue;ownerPrinted[i]=version;
        p=line;AppendText(p,"DSFV29s_AUDIO_REASON_OWNER",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyHex(p," owner=",static_cast<U32>(s.key),end);
        const char* keys[16]={" registrations="," requests="," refused="," accepted="," zero="," noneRequests="," peakObserved="," acceptedPeakProjected="," lastCount="," callbacks="," ends="," unmatched="," removes="," lookupHit="," lookupMiss=",nullptr};
        const volatile long* values=&s.registrations;
        for(U32 n=0;n<15u;++n)DiagFAppendKeyU(p,keys[n],static_cast<U32>(values[n]),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    for(U32 i=0;i<513u;++i) {
        const AudioReasonEvent& s=i==512u?g_audioReasonZeroEvent:g_audioReasonEvents[i];
        if(i<512u && !s.key)continue;
        const U32 version=static_cast<U32>(s.lookupHit+s.lookupMiss+s.createFailed+s.requests+s.refused+s.accepted+s.zero);
        if(version==eventPrinted[i])continue;eventPrinted[i]=version;
        p=line;AppendText(p,"DSFV29s_AUDIO_REASON_EVENT",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyHex(p," event=",static_cast<U32>(s.key),end);
        const char* keys[7]={" lookupHit="," lookupMiss="," createFailed="," requests="," refused="," accepted="," zero="};
        const volatile long* values=&s.lookupHit;
        for(U32 n=0;n<7u;++n)DiagFAppendKeyU(p,keys[n],static_cast<U32>(values[n]),end);
        DiagFAppendKeyHex(p," firstOwner=",static_cast<U32>(s.firstOwner),end);DiagFAppendKeyHex(p," lastOwner=",static_cast<U32>(s.lastOwner),end);
        DiagFAppendKeyHex(p," firstCaller=",static_cast<U32>(s.firstCaller),end);DiagFAppendKeyHex(p," lastCaller=",static_cast<U32>(s.lastCaller),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

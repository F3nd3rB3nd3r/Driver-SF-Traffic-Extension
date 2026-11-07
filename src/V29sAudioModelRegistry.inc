#if !defined(DSF_V29S_AUDIO_REASON_PROBE)
#error Model registry test requires the verified audio reason baseline
#endif
#include "V29sAudioModelRegistryCore.inc"
extern "C" UPtr g_audioModelTramps[8];
#include "V29sAudioModelRegistryBridges.inc"
#include "V29sAudioModelRegistryPatch.inc"
static void AudioModelStartLine() {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_MODEL_START revision=1 status=",end);AppendNumber(p,g_audioModelStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioModelReason,end);
    DiagFAppendKeyU(p," hooks=",g_audioModelInstalled,end);
    AppendText(p," expectedHooks=8 stockCapacity=45 testCapacity=192 nativeLayoutPreserved=1 emptyOnly=1 activeMigration=0 instanceBound=8 keyBound=512\n",end);
    *p=0;CapacityWrite(line);
}
static void AudioModelPoll() {
    if(g_audioModelStatus!=1)return;
    const char* kinds[7]={"registerAttempt","registeredStore","storage","lookupMissing","fallback","bankCheck","setupResult"};
    static U32 printed[7]={};
    for(U32 kind=0;kind<7u;++kind)while(printed[kind]<kAudioModelBounds[kind]) {
        const AudioModelDetail& r=g_audioModelDetails[kind][printed[kind]];if(!r.ready)break;
        char line[750]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_MODEL_DETAIL kind=",end);AppendText(p,kinds[kind],end);
        DiagFAppendKeyU(p," ordinal=",++printed[kind],end);DiagFAppendKeyU(p," tick=",r.tick,end);
        DiagFAppendKeyHex(p," owner=",r.owner,end);DiagFAppendKeyU(p," model=",r.model,end);
        DiagFAppendKeyHex(p," a=",r.a,end);DiagFAppendKeyU(p," b=",r.b,end);DiagFAppendKeyU(p," c=",r.c,end);DiagFAppendKeyU(p," d=",r.d,end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    char line[1500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_MODEL_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    const char* keys[8]={" attempts="," stores="," removes="," lookups="," setupAttempts="," bankChecks="," setupOk="," setupFail="};
    for(U32 i=0;i<8u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(g_audioModelCalls[i]),end);
    DiagFAppendKeyU(p," attachments=",static_cast<U32>(g_audioModelAttachments),end);
    DiagFAppendKeyU(p," attachRefused=",static_cast<U32>(g_audioModelAttachRefused),end);
    DiagFAppendKeyU(p," fallbackHit=",static_cast<U32>(g_audioModelFallbackHit),end);
    DiagFAppendKeyU(p," fallbackMiss=",static_cast<U32>(g_audioModelFallbackMiss),end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_audioModelInvalid),end);
    DiagFAppendKeyU(p," instanceOverflow=",static_cast<U32>(g_audioModelInstanceOverflow),end);
    DiagFAppendKeyU(p," keyOverflow=",static_cast<U32>(g_audioModelKeyOverflow),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    for(U32 i=0;i<8u;++i) {
        const AudioModelInstance& r=g_audioModelInstances[i];if(!r.owner)continue;
        p=line;AppendText(p,"DSFV29s_AUDIO_MODEL_REGISTRY",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyHex(p," owner=",static_cast<U32>(r.owner),end);
        AppendText(p," state=",end);AppendNumber(p,r.state,end);
        DiagFAppendKeyU(p," capacity=",r.state==1?192u:45u,end);
        DiagFAppendKeyHex(p," external=",AudioModelBuffer(&r),end);
        const char* names[10]={" generations="," attempts="," stores="," full="," wouldStockFull="," removes="," hits="," misses="," peak="," current="};
        const U32 values[10]={static_cast<U32>(r.generations),static_cast<U32>(r.attempts),static_cast<U32>(r.stores),static_cast<U32>(r.full),static_cast<U32>(r.wouldStockFull),static_cast<U32>(r.removes),static_cast<U32>(r.hits),static_cast<U32>(r.misses),static_cast<U32>(r.peak),static_cast<U32>(r.current)};
        for(U32 n=0;n<10u;++n)DiagFAppendKeyU(p,names[n],values[n],end);
        U32 bad=0;if(g_audioModelStorage)for(U32 n=0;n<4u;++n)if(Read32(AudioModelBuffer(&r)+192u*16u+n*4u)!=kAudioModelGuard)++bad;
        DiagFAppendKeyU(p," guardBad=",bad,end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    static U32 previous[513]={};
    for(U32 i=0;i<513u;++i) {
        const AudioModelKey& k=i==512u?g_audioModelZeroKey:g_audioModelKeys[i];if(i<512u && !k.key)continue;
        const U32 version=static_cast<U32>(k.attempts+k.stores+k.full+k.removes+k.hits+k.misses+k.fallbackHit+k.fallbackMiss+k.setup+k.bankReady+k.bankNotReady+k.setupOk+k.setupFail);
        if(version==previous[i])continue;previous[i]=version;
        p=line;AppendText(p,"DSFV29s_AUDIO_MODEL_KEY",end);DiagFAppendKeyU(p," tick=",now,end);DiagFAppendKeyU(p," model=",static_cast<U32>(k.key),end);
        const char* names[15]={" attempts="," stores="," full="," wouldStockFull="," removes="," hits="," misses="," fallbackHit="," fallbackMiss="," setup="," disabled="," bankReady="," bankNotReady="," setupOk="," setupFail="};
        const U32 values[15]={static_cast<U32>(k.attempts),static_cast<U32>(k.stores),static_cast<U32>(k.full),static_cast<U32>(k.wouldStockFull),static_cast<U32>(k.removes),static_cast<U32>(k.hits),static_cast<U32>(k.misses),static_cast<U32>(k.fallbackHit),static_cast<U32>(k.fallbackMiss),static_cast<U32>(k.setup),static_cast<U32>(k.disabled),static_cast<U32>(k.bankReady),static_cast<U32>(k.bankNotReady),static_cast<U32>(k.setupOk),static_cast<U32>(k.setupFail)};
        for(U32 n=0;n<15u;++n)DiagFAppendKeyU(p,names[n],values[n],end);
        DiagFAppendKeyHex(p," lastProfile=",static_cast<U32>(k.lastProfile),end);DiagFAppendKeyU(p," lastSelected=",static_cast<U32>(k.lastSelected),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

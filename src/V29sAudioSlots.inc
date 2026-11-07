#if !defined(DSF_V29S_AUDIO_DEFAULT_64M) || !defined(DSF_V29S_AUDIO_PROBE)
#error Slots2048 comparison requires the existing shared-default64MiB baseline
#endif
#include "V29sAudioSlotsCore.inc"
extern "C" UPtr g_audioSlotsTramps[9];
#include "V29sAudioSlotsBridges.inc"
#include "V29sAudioSlotsPatch.inc"
static void AudioSlotsStartLine() {
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_SLOTS_START revision=1 status=",end);AppendNumber(p,g_audioSlotsStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioSlotsReason,end);
    DiagFAppendKeyU(p," patches=",g_audioSlotsPatched?1u:0u,end);
    DiagFAppendKeyU(p," hooks=",g_audioSlotsInstalled,end);
    AppendText(p," expectedHooks=9 stockMax=255 testMax=2048 managers=2 globalLists=3 perRecordLimits=8,255 defaultBytes=67108864\n",end);
    *p=0;CapacityWrite(line);
}
static void AudioSlotsPoll() {
    if(g_audioSlotsStatus!=1)return;
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;
    if(last && now-last<1000u)return;last=now;
    const char* names[3]={"A0","A1","B"};
    for(U32 i=0;i<3u;++i) {
        const AudioSlotsList& s=g_audioSlotsLists[i];
        char line[1100]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_SLOTS",end);DiagFAppendKeyU(p," tick=",now,end);
        AppendText(p," list=",end);AppendText(p,names[i],end);
        DiagFAppendKeyHex(p," owner=",static_cast<U32>(s.owner),end);
        DiagFAppendKeyU(p," configuredMax=",static_cast<U32>(s.configured),end);
        DiagFAppendKeyU(p," allocatedSlots=",static_cast<U32>(s.allocated),end);
        DiagFAppendKeyU(p," nativeUpdateCount=",static_cast<U32>(s.updateCount),end);
        DiagFAppendKeyU(p," updatePeak=",static_cast<U32>(s.updatePeak),end);
        DiagFAppendKeyU(p," updateSamples=",static_cast<U32>(s.updates),end);
        DiagFAppendKeyU(p," attempts=",static_cast<U32>(s.attempts),end);
        DiagFAppendKeyU(p," peakBefore=",static_cast<U32>(s.peakBefore),end);
        DiagFAppendKeyU(p," inserts=",static_cast<U32>(s.inserts),end);
        DiagFAppendKeyU(p," peakInserted=",static_cast<U32>(s.peakInserted),end);
        DiagFAppendKeyU(p," ceilingHits=",static_cast<U32>(s.ceiling),end);
        DiagFAppendKeyU(p," selectedReuses=",static_cast<U32>(s.reuses),end);
        DiagFAppendKeyU(p," invalid=",static_cast<U32>(s.invalid),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_SLOT_USERS",end);DiagFAppendKeyU(p," tick=",now,end);
    DiagFAppendKeyU(p," eightAttempts=",static_cast<U32>(g_audioSlotsUserAttempts[0]),end);
    DiagFAppendKeyU(p," eightPeakBefore=",static_cast<U32>(g_audioSlotsUserPeak[0]),end);
    DiagFAppendKeyU(p," eightFull=",static_cast<U32>(g_audioSlotsUserFull[0]),end);
    DiagFAppendKeyU(p," byteAttempts=",static_cast<U32>(g_audioSlotsUserAttempts[1]),end);
    DiagFAppendKeyU(p," bytePeakBefore=",static_cast<U32>(g_audioSlotsUserPeak[1]),end);
    DiagFAppendKeyU(p," byteFull=",static_cast<U32>(g_audioSlotsUserFull[1]),end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_audioSlotsInvalid),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

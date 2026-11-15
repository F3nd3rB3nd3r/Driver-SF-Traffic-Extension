static const char* ChapterReasonName(ChapterReason reason) {
    static const char* names[]={"expanded","otherCaller","empty","contextUnavailable","unknownProfile",
        "nativeProfile","ownerMismatch","sourceMismatch","assetMismatch","graphicsNotReady","admissionMismatch","nativeRoster"};
    return reason<=ChapterNative?names[reason]:"invalidReason";
}
static void ChapterEvent(ChapterReason reason,UPtr caller,UPtr owner,U32 hash,U32 frequency,int nativeCount,U32 profile) {
    // Low-frequency request boundary only. Bound benign non-setter diagnostics.
    if(reason==ChapterOtherCaller && g_chapterOther>24)return;
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CHAPTER_EVENT reason=",end);AppendText(p,ChapterReasonName(reason),end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyHex(p," caller=",caller,end);DiagFAppendKeyHex(p," owner=",owner,end);
    DiagFAppendKeyHex(p," hash=",hash,end);DiagFAppendKeyU(p," frequency=",frequency,end);
    AppendText(p," nativeCount=",end);AppendNumber(p,nativeCount,end);
    DiagFAppendKeyU(p," effectiveCount=",reason==ChapterExpand?150u:nativeCount>0?static_cast<U32>(nativeCount):0u,end);
    DiagFAppendKeyU(p," profile=",profile,end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void ChapterStartLine() {
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CHAPTER_START revision=1 status=",end);AppendNumber(p,g_chapterStatus,end);
    AppendText(p," setterCall=0076FE95 callerReturn=0076FE9A profiles=80 expandedProfiles=55 namedProfiles=51 unnamedProfiles=4 requests=150 graphics=160 newNativeHooks=0",end);
#if defined(DSF_V29S_TRAFFIC_POLICY)
    AppendText(p," policy=",end);AppendText(p,TrafficModeName(),end);
    AppendText(p,g_trafficPolicy.mode==TrafficPreset150?" assetCheckedOnRequest=1\n":" assetCheckedOnRequest=0\n",end);
#else
    AppendText(p," policy=canonicalListAndUniform150 assetCheckedOnRequest=1\n",end);
#endif
    *p=0;CapacityWrite(line);
#if defined(DSF_V29S_TRAFFIC_POLICY)
    p=line;AppendText(p,"DSFV29s_TRAFFIC_POLICY_START revision=1 mode=",end);AppendText(p,TrafficModeName(),end);
    AppendText(p," reason=",end);AppendText(p,g_trafficPolicy.reason,end);
    AppendText(p," file=DriverSF_Traffic.ini location=asiDirectory requestCapacity=150 graphics=160 capacityChanges=0 liveReload=0\n",end);
    *p=0;CapacityWrite(line);
#endif
}
static void ChapterPoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;
    if(last && now-last<1000u)return;last=now;
    char line[750]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CHAPTER_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    DiagFAppendKeyU(p," calls=",static_cast<U32>(g_chapterCalls),end);
    DiagFAppendKeyU(p," expanded=",static_cast<U32>(g_chapterExpanded),end);
    DiagFAppendKeyU(p," otherCaller=",static_cast<U32>(g_chapterOther),end);
    DiagFAppendKeyU(p," empty=",static_cast<U32>(g_chapterEmpty),end);
    DiagFAppendKeyU(p," unknown=",static_cast<U32>(g_chapterUnknown),end);
    DiagFAppendKeyU(p," nativeProfile=",static_cast<U32>(g_chapterExcluded),end);
    DiagFAppendKeyU(p," nativeRoster=",static_cast<U32>(g_chapterNative),end);
    DiagFAppendKeyU(p," refused=",static_cast<U32>(g_chapterRefused),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    static U32 printed[80]={};
    for(U32 i=0;i<kChapterProfileCount;++i) {
        const U32 calls=static_cast<U32>(g_chapterCounters[i].calls);
        if(!calls || calls==printed[i])continue;printed[i]=calls;
        p=line;AppendText(p,"DSFV29s_CHAPTER_PROFILE",end);
        DiagFAppendKeyHex(p," hash=",kChapterProfiles[i].hash,end);
        DiagFAppendKeyU(p," frequency=",kChapterProfiles[i].frequency,end);
        DiagFAppendKeyU(p," calls=",calls,end);
        DiagFAppendKeyU(p," expanded=",static_cast<U32>(g_chapterCounters[i].expanded),end);
        DiagFAppendKeyU(p," refused=",static_cast<U32>(g_chapterCounters[i].refused),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

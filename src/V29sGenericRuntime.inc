static void GenericConfigLine(){
    char line[1300]={};char* p=line;const char* end=line+sizeof(line)-1;
#if defined(DSF_V29S_WIDE_LIMITS)
    AppendText(p,"DSFV29s_GENERIC_START revision=2 valid=",end);
#else
    AppendText(p,"DSFV29s_GENERIC_START revision=1 valid=",end);
#endif
    DiagFAppendU(p,g_limits.valid?1u:0u,end);
    AppendText(p," reason=",end);AppendText(p,g_limits.reason,end);
    AppendText(p," rosterSource=",end);AppendText(p,g_limits.assetWeights?"AssetWeights":"Native",end);
#if defined(DSF_V29S_LOGGING)
    DiagFAppendKeyU(p," logLevel=",g_diagnosticLevel,end);
#endif
    const char* keys[]={" requestCapacity="," audioModelEntries="," globalAudioEntries="," defaultAudioMiB="," replayReaderMiB="," retainedReferences="," requestStorage="," parkedStride="," parkedFrame="," notifyFrame="," replayPayloadCapacity="};
    const U32 values[]={g_limits.requests,g_limits.audioModels,g_limits.audioSlots,g_limits.audioMiB,g_limits.replayMiB,g_limits.retained,GenericRequestStorage(),GenericParkStride(),GenericParkFrame(),GenericNotifyFrame(),GenericReplayBytes()};
    for(U32 i=0;i<11u;++i)DiagFAppendKeyU(p,keys[i],values[i],end);
#if defined(DSF_V29S_WIDE_LIMITS)
    DiagFAppendKeyU(p," modelSlots=",g_limits.models,end);DiagFAppendKeyU(p," requestFramePhysicalCapacity=",GenericRequestStorage(),end);
    const char* more[]={" objectEntries="," definitionEntries="," pendingEntries="," callbackEntriesPerGroup="," cleanupEntries="," resourceCollectionEntries="," indexedReferenceEntries="," hashExtraEntries="};
    const U32 sizes[]={g_limits.objects,g_limits.definitions,g_limits.pending,g_limits.callbacks,g_limits.cleanup,g_limits.collection,g_limits.indexed,g_limits.hashExtra};
    for(U32 i=0;i<8u;++i)DiagFAppendKeyU(p,more[i],sizes[i],end);
#if defined(DSF_V29S_STREAM_HANDLERS)
    DiagFAppendKeyU(p," streamingHandlers=",StreamCapacity(),end);
#if defined(DSF_V29S_MANAGER_KEYS)
    DiagFAppendKeyU(p," managerKeyEntries=",ManagerKeyCapacity(),end);
#endif
#endif
    AppendText(p," userCapacities=1 testedDefaultRequests=150 higherConfigurationsUntested=1 compiledCarLists=0 liveReload=0 file=DriverSF_Unlimiter.ini location=asiDirectory\n",end);
#else
    AppendText(p," modelSlots=160 auditedRequestCeiling=150 requestFramePhysicalCapacity=150 compiledCarLists=0 liveReload=0 file=DriverSF_Unlimiter.ini location=asiDirectory\n",end);
#endif
    *p=0;CapacityWrite(line);
}
static void GenericFatal(const char* reason,UPtr owner,U32 required){
    InterlockedIncrement(&g_genericFailures);char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_GENERIC_FATAL reason=",end);AppendText(p,reason,end);DiagFAppendKeyHex(p," owner=",owner,end);
    DiagFAppendKeyU(p," required=",required,end);DiagFAppendKeyU(p," configuredRequests=",g_limits.requests,end);
    AppendText(p," partialListPublished=0\n",end);*p=0;CapacityWrite(line);OutputDebugStringA(line);ExitProcess(0x29F2u);
}
static void GenericEvent(const char* reason,UPtr caller,UPtr owner,U32 hash,U32 frequency,int original,U32 effective){
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_GENERIC_EVENT reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);DiagFAppendKeyHex(p," caller=",caller,end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyHex(p," hash=",hash,end);DiagFAppendKeyU(p," frequency=",frequency,end);
    AppendText(p," originalCount=",end);AppendNumber(p,original,end);DiagFAppendKeyU(p," effectiveCount=",effective,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void ChapterStartLine(){GenericConfigLine();
    char line[300]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CHAPTER_START revision=2 status=",end);AppendNumber(p,g_chapterStatus,end);
    AppendText(p," policy=assetDefined compiledProfiles=0 compiledCarLists=0\n",end);*p=0;CapacityWrite(line);
}
static void ChapterPoll(){static U32 last=0;const U32 tick=GetTickCount()-g_capacityStart;if(last && tick-last<1000u)return;last=tick;
    char line[400]={};char* p=line;const char* end=line+sizeof(line)-1;AppendText(p,"DSFV29s_GENERIC_STATUS",end);
    DiagFAppendKeyU(p," tick=",tick,end);DiagFAppendKeyU(p," calls=",g_genericCalls,end);DiagFAppendKeyU(p," augmented=",g_genericAugmented,end);
    DiagFAppendKeyU(p," preserved=",g_genericPreserved,end);DiagFAppendKeyU(p," failures=",g_genericFailures,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}


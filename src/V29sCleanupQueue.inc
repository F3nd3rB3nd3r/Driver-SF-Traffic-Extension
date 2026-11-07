#if !defined(DSF_V29S_AUDIO_MODEL192) || !defined(DSF_V29S_INDEXED_262144)
#error Cleanup test requires the successful audio192/indexed262144 baseline
#endif
#include "V29sCleanupQueueCore.inc"
static void CleanupFatal(const char* reason) {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CLEANUP_FATAL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyU(p," full=",static_cast<U32>(g_cleanupFull),end);DiagFAppendKeyU(p," guardBad=",CleanupGuards(),end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_cleanupInvalid),end);DiagFAppendKeyU(p," current=",static_cast<U32>(g_cleanupCurrent),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29D1u);
}
extern "C" __declspec(noinline) void __cdecl CleanupEntry(U32 kind,U32* frame){if(!CleanupObserve(kind,frame))CleanupFatal("boundsGuardOrPhase");}
extern "C" UPtr g_cleanupTramps[6];
#include "V29sCleanupQueueBridges.inc"
#include "V29sCleanupQueuePatch.inc"
static void CleanupEvent(const char* kind,U32 count,U32 value,U32 producer) {
    static volatile long ordinary=0;if(kind[0]!='b' && count<32768u && InterlockedIncrement(&ordinary)>64)return;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CLEANUP_EVENT kind=",end);AppendText(p,kind,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);DiagFAppendKeyU(p," count=",count,end);
    DiagFAppendKeyHex(p," value=",value,end);DiagFAppendKeyU(p," producer=",producer,end);
    DiagFAppendKeyU(p," peak=",static_cast<U32>(g_cleanupPeak),end);DiagFAppendKeyU(p," mismatch=",static_cast<U32>(g_cleanupDrainMismatch),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void CleanupStartLine() {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CLEANUP_START revision=1 status=",end);AppendNumber(p,g_cleanupStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_cleanupReason,end);
    DiagFAppendKeyU(p," hooks=",g_cleanupInstalled,end);DiagFAppendKeyHex(p," base=",g_cleanupBase,end);
    AppendText(p," expectedHooks=6 stockCapacity=32768 testCapacity=262144 storageBytes=1048608 nativeCountPreserved=1 nativeOrderPreserved=1 activeMigration=0\n",end);*p=0;CapacityWrite(line);
}
static void CleanupPoll() {
    if(g_cleanupStatus!=1)return;static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    char line[1200]={};char* p=line;const char* end=line+sizeof(line)-1;AppendText(p,"DSFV29s_CLEANUP_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    const char* names[]={" direct="," replacement="," release="," peak="," current="," beyondStock="," oldCrossings="," full="," guardBad="," invalid="," drainStarts="," drainEnds="," nonempty="," drained="," depth="," duringDrain="," drainMismatch="};
    const U32 values[]={static_cast<U32>(g_cleanupAttempts[0]),static_cast<U32>(g_cleanupAttempts[1]),static_cast<U32>(g_cleanupAttempts[2]),static_cast<U32>(g_cleanupPeak),static_cast<U32>(g_cleanupCurrent),static_cast<U32>(g_cleanupBeyondStock),static_cast<U32>(g_cleanupOldCrossings),static_cast<U32>(g_cleanupFull),CleanupGuards(),static_cast<U32>(g_cleanupInvalid),static_cast<U32>(g_cleanupDrainStarts),static_cast<U32>(g_cleanupDrainEnds),static_cast<U32>(g_cleanupNonempty),static_cast<U32>(g_cleanupDrained),static_cast<U32>(g_cleanupDepth),static_cast<U32>(g_cleanupDuringDrain),static_cast<U32>(g_cleanupDrainMismatch)};
    for(U32 n=0;n<17u;++n)DiagFAppendKeyU(p,names[n],values[n],end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

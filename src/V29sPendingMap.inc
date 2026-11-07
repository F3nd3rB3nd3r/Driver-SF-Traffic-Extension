static void PendingFatal(const char* reason,UPtr map) {
    char line[750]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PENDING_FATAL reason=",end);AppendText(p,reason,end);DiagFAppendKeyHex(p," map=",map,end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_pendingInvalid),end);DiagFAppendKeyU(p," full=",static_cast<U32>(g_pendingFull),end);
    DiagFAppendKeyU(p," guardBad=",static_cast<U32>(g_pendingGuardBad),end);DiagFAppendKeyU(p," allocationFailed=",static_cast<U32>(g_pendingAllocFailed),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29E1u);
}
static void PendingEvent(const char* kind,UPtr map,U32 active,U32 issued) {
    static volatile long ordinary=0;if(kind[0]=='w' && active<64u && InterlockedIncrement(&ordinary)>48)return;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PENDING_EVENT kind=",end);AppendText(p,kind,end);DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyHex(p," map=",map,end);DiagFAppendKeyU(p," active=",active,end);DiagFAppendKeyU(p," issued=",issued,end);
    DiagFAppendKeyU(p," generation=",static_cast<U32>(g_pendingGeneration),end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
extern "C" __declspec(noinline) void __cdecl PendingEntry(U32 kind,U32* frame) {
    const UPtr section=frame[kind==0u?6u:1u];
    if(kind>1u || !PendingTraversal(kind,section))PendingFatal("traversalIntegrity",section+0x5F5Cu);
}
extern "C" UPtr g_pendingTramps[2];
#include "V29sPendingMapBridges.inc"
#include "V29sPendingMapPatch.inc"
static void PendingStartLine() {
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PENDING_START revision=1 status=",end);AppendNumber(p,g_pendingStatus,end);AppendText(p," reason=",end);AppendText(p,g_pendingReason,end);
    AppendText(p," stockCapacity=64 testCapacity=512 inlineNodes=64 extraNodes=448 nodeBytes=16 generationBytes=7232 traversalHooks=2 sharedInsertHook=007D7270 nativeSentinelPreserved=1 activeMigration=0\n",end);*p=0;CapacityWrite(line);
}
static void PendingPoll() {
    if(g_pendingStatus!=1)return;static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    char line[1300]={};char* p=line;const char* end=line+sizeof(line)-1;AppendText(p,"DSFV29s_PENDING_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    U32 guards=0;for(const PendingArena* a=g_pendingHead;a;a=reinterpret_cast<const PendingArena*>(a->older))guards+=PendingGuards(a);
    const char* names[]={" generations="," resets="," attempts="," stores="," redirects="," recycledInserts="," activePeak="," issuedPeak="," current="," recycled="," walkStarts="," walkEnds="," emptyChecks="," depth="," invalid="," full="," allocationFailed="," guardBad="};
    const U32 values[]={static_cast<U32>(g_pendingGeneration),static_cast<U32>(g_pendingResets),static_cast<U32>(g_pendingAttempts),static_cast<U32>(g_pendingStores),static_cast<U32>(g_pendingRedirects),static_cast<U32>(g_pendingReuses),static_cast<U32>(g_pendingPeak),static_cast<U32>(g_pendingIssuedPeak),static_cast<U32>(g_pendingCurrent),static_cast<U32>(g_pendingFree),static_cast<U32>(g_pendingWalkStarts),static_cast<U32>(g_pendingWalkEnds),static_cast<U32>(g_pendingEmpty),static_cast<U32>(g_pendingDepth),static_cast<U32>(g_pendingInvalid),static_cast<U32>(g_pendingFull),static_cast<U32>(g_pendingAllocFailed),guards+static_cast<U32>(g_pendingGuardBad)};
    for(U32 n=0;n<18u;++n)DiagFAppendKeyU(p,names[n],values[n],end);DiagFAppendKeyHex(p," lastMap=",static_cast<U32>(g_pendingLastMap),end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

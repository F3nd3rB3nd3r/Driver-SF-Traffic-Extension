#include "V29sCallbackCore.inc"
static void CallbackFatal(const char* reason,UPtr header) {
    char line[750]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CALLBACK_FATAL reason=",end);AppendText(p,reason,end);DiagFAppendKeyHex(p," header=",header,end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_callbackInvalid),end);DiagFAppendKeyU(p," full=",static_cast<U32>(g_callbackFull),end);
    DiagFAppendKeyU(p," guardBad=",static_cast<U32>(g_callbackGuardBad),end);DiagFAppendKeyU(p," concurrent=",static_cast<U32>(g_callbackConcurrent),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29C1u);
}
static void CallbackEvent(U32 slot,const char* kind) {
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CALLBACK_EVENT kind=",end);AppendText(p,kind,end);DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyU(p," group=",slot,end);DiagFAppendKeyHex(p," header=",CallbackHeader(slot),end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
extern "C" __declspec(noinline) void __cdecl CallbackWriterCheck(UPtr handler) {
    U32 slot=0;if(!CallbackHandlerSlot(handler,slot))CallbackFatal(g_callbackReason,0u);
}
#include "V29sCallbackBridges.inc"
#include "V29sCallbackPatch.inc"
static void CallbackStartLine() {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CALLBACK_START revision=1 status=",end);AppendNumber(p,g_callbackStatus,end);AppendText(p," reason=",end);AppendText(p,g_callbackReason,end);
    AppendText(p," groups=21 stockCapacity=64 testCapacity=512 inlineNodes=64 extraNodes=448 nodeBytes=12 hooks=3 sentinelPreserved=1 processLifetime=1 lazyRedirect=1 activeMigration=0\n",end);*p=0;CapacityWrite(line);
}
static void CallbackPoll() {
    if(g_callbackStatus!=1)return;static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    // Published gauges only: the monitor never traverses native lists or waits
    // on a callback thread. Independent fields may be sampled in flight.
    char line[1400]={};const char* end=line+sizeof(line)-1;char* p=line;
    AppendText(p,"DSFV29s_CALLBACK_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    const char* errorNames[]={" invalid="," full="," guardBad="," concurrent="};
    const long errors[]={g_callbackInvalid,g_callbackFull,g_callbackGuardBad,g_callbackConcurrent};
    for(U32 n=0;n<4u;++n)DiagFAppendKeyU(p,errorNames[n],static_cast<U32>(errors[n]),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    for(U32 i=0;i<kCallbackGroups;++i) {
        const CallbackGroup& g=g_callbackGroups[i];if(!g.attempts && !g.polls)continue;
        p=line;AppendText(p,"DSFV29s_CALLBACK_GROUP",end);DiagFAppendKeyU(p," tick=",now,end);DiagFAppendKeyU(p," group=",i,end);
        const char* names[]={" attempts="," stores="," redirects="," recycledInserts="," peak="," issuedPeak="," current="," recycled="," polls="," removed="," depth="," pollDepth="};
        const long values[]={g.attempts,g.stores,g.redirects,g.reuses,g.peak,g.issuedPeak,g.active,g.free,g.polls,g.removed,g.depth,g.pollDepth};
        for(U32 n=0;n<12u;++n)DiagFAppendKeyU(p,names[n],static_cast<U32>(values[n]),end);
        DiagFAppendKeyHex(p," lastHandler=",static_cast<U32>(g.lastHandler),end);DiagFAppendKeyHex(p," lastPayload=",static_cast<U32>(g.lastPayload),end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

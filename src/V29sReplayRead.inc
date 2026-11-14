#if !defined(DSF_V29S_REPLAY_CLIENT) || defined(DSF_V29S_REPLAY_HITCH)
#error Read1M requires the narrow client probe, not the broad timing hooks
#endif
extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(volatile long*,long);
#include "V29sReplayReadCore.inc"
static UPtr RFAddress(U32 at){return Address(at);}
static U32 RFRead32(UPtr p){return Read32(p);}static U8 RFRead8(UPtr p){return Read8(p);}
static void* RFAllocate(U32 bytes,U32 reserveOnly){return VirtualAlloc(nullptr,bytes,reserveOnly?0x2000u:0x3000u,reserveOnly?1u:4u);}
static bool RFCommitMemory(UPtr at,U32 bytes){return VirtualAlloc(reinterpret_cast<void*>(at),bytes,0x1000u,4u)==reinterpret_cast<void*>(at);}
static void RFFree(UPtr p){VirtualFree(reinterpret_cast<void*>(p),0u,0x8000u);}
#include "V29sReplayReadNative.inc"
#include "V29sReplayReadBridges.inc"
#include "V29sReplayReadLayout.inc"
static void RFAppend(char*& p,const char* end,const RFView& v) {
    const char* keys[]={" rfPresent="," rfGeneration="," rfState="," rfBuffer="," rfCapacity="," rfPolls="," rfBackendBusy="," rfCommits="," rfCommittedBytes="," rfWrapChecks="," rfWrapStops="," rfSpaceChecks="," rfSpaceStops="," rfIssues="," rfIssueBytes="," rfIssueRejected="," rfEndChecks="," rfEndStops="," rfMinFree="," rfMaxRequired="," rfLastStop="," rfLastStopPoll="," rfLastStopFree="," rfLastStopRequired="};
    const U32 values[]={v.present,v.generation,v.state,v.buffer,v.capacity,v.polls,v.backendBusy,v.commits,v.committedBytes,v.wrapChecks,v.wrapStops,v.spaceChecks,v.spaceStops,v.issues,v.issueBytes,v.issueRejected,v.endChecks,v.endStops,v.minFree,v.maxRequired,v.lastStop,v.lastStopPoll,v.lastStopFree,v.lastStopRequired};
    for(U32 i=0;i<24u;++i)if(i==3u)DiagFAppendKeyHex(p,keys[i],values[i],end);else DiagFAppendKeyU(p,keys[i],values[i],end);
}
static void RFPoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(now-last<1000u)return;last=now;
    char line[2200]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_READ_STATUS tick=",end);DiagFAppendU(p,now,end);AppendText(p," status=",end);AppendNumber(p,g_rfStatus,end);
    const char* keys[]={" owners="," constructs="," destructs="," reuses="," writerBypass="," allocationFailed="," fallbacks="," faults="," unmanagedCalls="};
    const long values[]={g_rfOwnerCount,g_rfConstructs,g_rfDestructs,g_rfReuses,g_rfWriterBypass,g_rfAllocationFailed,g_rfFallbacks,g_rfFaults,g_rfUnmanaged};
    for(U32 i=0;i<9u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);
    AppendText(p," retainedBackingBytes=",end);DiagFAppendU(p,static_cast<U32>(g_rfOwnerCount)*kRFBytes,end);AppendText(p," processLifetime=1\n",end);*p=0;CapacityWrite(line);
    // Metadata/backing are never deleted, so the monitor cannot follow freed
    // directory nodes. Individual counters are atomic, not a global instant.
    for(RFOwner* r=g_rfOwners;r;r=r->next) {
        p=line;AppendText(p,"DSFV29s_REPLAY_READ_OWNER tick=",end);DiagFAppendU(p,now,end);DiagFAppendKeyHex(p," owner=",r->self,end);
        DiagFAppendKeyHex(p," originalBuffer=",r->originalBuffer,end);DiagFAppendKeyU(p," originalCapacity=",r->originalCapacity,end);
        const RFView v=RFCapture(r->self);RFAppend(p,end,v);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

#include "V29sReplayPauseNative.inc"
#include "V29sReplayPauseCore.inc"
#include "V29sReplayPauseLayout.inc"
static long g_pauseStatus=0;
static UPtr g_pauseUpdateTrampoline=0;
#include "V29sReplayPauseBridge.inc"
static U32 PauseThread(){return GetCurrentThreadId();}
static UPtr PauseRoot(){return Read32(Address(0x00FE7228u));}
static UPtr PauseAddress(U32 at){return Address(at);}
static bool PauseSnapshotOkay(UPtr owner) {
    if(InterlockedCompareExchange(&g_replayLock,1,0))return false;
    ReplaySlot* slot=ReplayFindSlot(owner+0x438E4u,false);
    const bool okay=ReplaySlotOkay(slot) && ReplayMirrorMatches(owner+0x438F0u,slot->data);
    InterlockedExchange(&g_replayLock,0);return okay;
}
static void PauseRunOriginal(UPtr owner){
#if defined(DSF_REPLAY_HITCH_RUNTIME)
    RHScope timing(RHUpdate,owner);
#endif
    typedef void (__thiscall* Fn)(void*);reinterpret_cast<Fn>(g_pauseUpdateTrampoline)(reinterpret_cast<void*>(owner));
}
static void PauseRunCompletion(UPtr owner){PauseCompleteNative(reinterpret_cast<void*>(owner),nullptr);}
static void PauseEvent(const char* kind,const PauseView& before,const PauseView* after,U32 epoch) {
    static U32 lastHeld=0;const U32 tick=GetTickCount()-g_capacityStart;
    if(kind[0]=='n' && g_pauseHeld!=1 && tick-lastHeld<1000u)return;
    if(kind[0]=='n')lastHeld=tick;
    char line[900]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_PAUSE_EVENT kind=",end);AppendText(p,kind,end);
    DiagFAppendKeyU(p," tick=",tick,end);DiagFAppendKeyU(p," thread=",PauseThread(),end);
    DiagFAppendKeyHex(p," owner=",before.owner,end);DiagFAppendKeyHex(p," simulation=",before.simulation,end);
    DiagFAppendKeyU(p," importEpoch=",epoch,end);DiagFAppendKeyU(p," phase=",before.phase,end);
    DiagFAppendKeyU(p," loadingBefore=",before.loading,end);DiagFAppendKeyU(p," lifecycleBefore=",before.lifecycle,end);
    DiagFAppendKeyU(p," pausedBefore=",before.paused,end);DiagFAppendKeyHex(p," rateBefore=",before.rateBits,end);
    DiagFAppendKeyHex(p," clockBefore=",before.clockBits,end);
    if(after){DiagFAppendKeyU(p," loadingAfter=",after->loading,end);DiagFAppendKeyU(p," lifecycleAfter=",after->lifecycle,end);
        DiagFAppendKeyU(p," pausedAfter=",after->paused,end);DiagFAppendKeyHex(p," rateAfter=",after->rateBits,end);DiagFAppendKeyHex(p," clockAfter=",after->clockBits,end);}
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void PauseFatal(const char* reason) {
    char line[400]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_PAUSE_FATAL reason=",end);AppendText(p,reason,end);AppendText(p," exitCode=000029F5\n",end);*p=0;
    CapacityWrite(line);ExitProcess(0x29F5u);
}
static bool PauseDependencies() {
    return PauseEqual(Address(0x007390BEu),kPauseCompleteNative,sizeof(kPauseCompleteNative)) &&
        PauseEqual(Address(0x00738D20u),kPauseReadyNative,sizeof(kPauseReadyNative)) &&
        Read32(Address(0x00E2CA74u))==Address(0x0053AA20u);
}
static void PauseInstall() {
    const UPtr at=Address(0x0073A090u),target=reinterpret_cast<UPtr>(&PauseUpdate);
    bool okay=g_replayStatus==1 && !g_pkg0 && VerifyExecutableIdentity() && PauseUpdateMatches(at,target,false) && PauseDependencies();
    if(okay) {
        g_pauseUpdateTrampoline=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,0x04u));
        okay=g_pauseUpdateTrampoline!=0u;
    }
    DWORD old=0,discarded=0;
    if(okay) {
        PauseFillTrampoline(g_pauseUpdateTrampoline,g_pauseUpdateTrampoline,at+6u);g_pauseCompletionTarget=Address(0x007390BEu);
        okay=VirtualProtect(reinterpret_cast<void*>(g_pauseUpdateTrampoline),0x1000u,0x20u,&discarded)!=0 &&
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(g_pauseUpdateTrampoline),0x1000u)!=0;
    }
    if(okay)okay=VirtualProtect(reinterpret_cast<void*>(at),6u,kPageExecuteReadWrite,&old)!=0;
    if(okay) {
        if(g_pkg0 || !PauseUpdateMatches(at,target,false) || !PauseDependencies()) {
            if(!VirtualProtect(reinterpret_cast<void*>(at),6u,old,&discarded))PauseFatal("precommitProtection");okay=false;
        }else {
            PausePatch(at,target,true);const bool exact=PauseUpdateMatches(at,target,true) && PauseDependencies();
            const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),6u);
            const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(at),6u,old,&discarded);
            okay=exact && flushed && protectedAgain;
            if(!okay) {
                DWORD retry=0;if(!VirtualProtect(reinterpret_cast<void*>(at),6u,kPageExecuteReadWrite,&retry))PauseFatal("rollbackUnlock");
                PausePatch(at,target,false);const bool restored=PauseUpdateMatches(at,target,false);
                const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),6u);
                const BOOL protect=VirtualProtect(reinterpret_cast<void*>(at),6u,old,&discarded);
                if(!restored || !flush || !protect)PauseFatal("rollbackIncomplete");
            }
        }
    }
    if(!okay){g_pauseStatus=-1;CapacityWrite("DSFV29s_REPLAY_PAUSE_START revision=1 status=-1\n");PauseFatal("startupPatchRefused");return;}
    g_pauseStatus=1;
    CapacityWrite("DSFV29s_REPLAY_PAUSE_START revision=1 status=1 hooks=1 site=0073A090 completion=007390BE importTag=1 sameThread=1 nativeReadyGate=1 forceUnpause=0 monitorEngineCalls=0 requests=150 graphics=160\n");
}
static void PausePoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(now-last<1000u)return;last=now;
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_PAUSE_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    AppendText(p," status=",end);AppendNumber(p,g_pauseStatus,end);
    const char* keys[]={" updates="," imports="," candidates="," attempts="," completed="," nativeHeld="," ownerChanged="," missingImport="," snapshotSkipped="," reentrant="," markDrops="};
    const long values[]={g_pauseUpdates,g_pauseImports,g_pauseCandidates,g_pauseAttempts,g_pauseCompleted,g_pauseHeld,g_pauseOwnerChanged,g_pauseNoImport,g_pauseSnapshotSkipped,g_pauseReentrant,g_pauseMarkDrops};
    for(U32 i=0;i<11u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

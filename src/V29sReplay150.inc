#include "V29sReplay150Native.inc"
#include "V29sReplay150Core.inc"
static long g_replayStatus=0;
static void ReplayFatal(const char* reason,UPtr owner,U32 value) {
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY150_FATAL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," value=",value,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29F4u);
}
static void* ReplayAllocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x04u);}
static U32 ReplayThread(){return GetCurrentThreadId();}
static UPtr ReplaySpooler() {
    const UPtr location=Address(0x00FE7228u);UPtr root=Read32(location);
    if(!root){typedef UPtr (__cdecl* Fn)();root=reinterpret_cast<Fn>(Address(0x0040A690u))();Write32(location,root);}
    if(!Readable(root,0x218u)){ReplayFatal("rootUnavailable",root,0u);return 0;}
    const UPtr spooler=Read32(root+0x214u);
    if(!Readable(spooler,0xE6Cu)){ReplayFatal("spoolerUnavailable",spooler,0u);return 0;}return spooler;
}
static bool ReplayIdle(UPtr spooler){return Read32(spooler+0xDACu)==0u && Read32(spooler+0x9A8u)==0u && Read32(spooler+0xC8u)==0u;}
static U32 ReplayRefs(UPtr owner,U32 id){typedef U32 (__thiscall* Fn)(void*,U32);return reinterpret_cast<Fn>(Address(0x005A6B00u))(reinterpret_cast<void*>(owner),id);}
static void ReplayRequest(UPtr owner,const U32* ids,U32 count){
#if defined(DSF_REPLAY_HITCH_RUNTIME)
    RHScope timing(RHRequest,owner,0u,count);
#endif
    typedef void (__thiscall* Fn)(void*,const U32*,U32);reinterpret_cast<Fn>(Address(0x005A79F0u))(reinterpret_cast<void*>(owner),ids,count);
}
static void ReplayReference(UPtr owner,U32 id,bool retain){typedef void (__thiscall* Fn)(void*,U32);reinterpret_cast<Fn>(Address(retain?0x005A7860u:0x005A78D0u))(reinterpret_cast<void*>(owner),id);}
static void ReplayExtra(UPtr owner,U32 value){typedef void (__thiscall* Fn)(void*,U32);reinterpret_cast<Fn>(Address(0x005A6250u))(reinterpret_cast<void*>(owner),value);}
static void ReplayAppend(UPtr stream,const void* source,U32 bytes){typedef void (__thiscall* Fn)(void*,const void*,U32);reinterpret_cast<Fn>(Address(0x007468A0u))(reinterpret_cast<void*>(stream),source,bytes);}
static void ReplayEvent(const char* kind,UPtr owner,const ReplayData& d) {
    // Full counts and identity on every snapshot boundary; no per-frame model dump.
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY150_EVENT kind=",end);AppendText(p,kind,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);DiagFAppendKeyHex(p," owner=",owner,end);
    DiagFAppendKeyU(p," count=",d.count,end);DiagFAppendKeyU(p," retained=",d.retained,end);
    DiagFAppendKeyHex(p," rosterHash=",ReplayHash(d),end);DiagFAppendKeyU(p," id50=",d.count>50u?d.ids[50]:0u,end);
    U32 has181=0;for(U32 i=0;i<d.count;++i)if(d.ids[i]==181u)has181=1u;
    DiagFAppendKeyU(p," has181=",has181,end);DiagFAppendKeyHex(p," extra=",d.extra,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
#include "V29sReplay150Layout.inc"
static void ReplayInstall() {
    bool ok=g_parkStatus==1 && !g_pkg0 && VerifyExecutableIdentity() && ReplayBytesMatch(false);
    const UPtr page=Address(0x0074B000u);DWORD old=0,discarded=0;
    if(ok)ok=VirtualProtect(reinterpret_cast<void*>(page),0x4000u,kPageExecuteReadWrite,&old)!=0;
    if(ok) {
        if(g_pkg0 || !ReplayBytesMatch(false)) {
            if(!VirtualProtect(reinterpret_cast<void*>(page),0x4000u,old,&discarded))ReplayFatal("precommitProtection",page,0u);ok=false;
        }else {
            ReplayApply(true);const bool exact=ReplayBytesMatch(true);
            const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x4000u);
            const BOOL protect=VirtualProtect(reinterpret_cast<void*>(page),0x4000u,old,&discarded);
            ok=exact && flush && protect;
            if(!ok) {
                DWORD retry=0;if(!VirtualProtect(reinterpret_cast<void*>(page),0x4000u,kPageExecuteReadWrite,&retry))ReplayFatal("rollbackUnlock",page,0u);
                ReplayApply(false);const bool restored=ReplayBytesMatch(false);
                const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x4000u);
                const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x4000u,old,&discarded);
                if(!restored || !flushed || !protectedAgain)ReplayFatal("rollbackIncomplete",page,0u);
            }
        }
    }
    if(!ok){g_replayStatus=-1;CapacityWrite("DSFV29s_REPLAY150_START revision=1 status=-1\n");ReplayFatal("startupPatchRefused",page,0u);return;}
    g_replayStatus=1;
#if defined(DSF_V29S_GENERIC_LIMITS)
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY150_START revision=1 status=1 hooks=7",end);DiagFAppendKeyU(p," trafficCapacity=",g_limits.requests,end);
    DiagFAppendKeyU(p," retainedCapacity=",g_limits.retained,end);DiagFAppendKeyU(p," payloadBytes=",GenericReplayBytes(),end);
    AppendText(p," nativeViewBytes=612 nativeObjectBytes=624 nativeRecordFormat=1 tempReuse=1 fixedOwnerLimit=0 requests=150 graphics=160\n",end);*p=0;CapacityWrite(line);
#else
    CapacityWrite("DSFV29s_REPLAY150_START revision=1 status=1 hooks=7 trafficCapacity=150 retainedCapacity=256 payloadBytes=2660 nativeViewBytes=612 nativeObjectBytes=624 nativeRecordFormat=1 tempReuse=1 fixedOwnerLimit=0 requests=150 graphics=160\n");
#endif
}
static void ReplayPoll() {
    static U32 last=0;const U32 tick=GetTickCount()-g_capacityStart;if(tick-last<1000u)return;last=tick;
    char line[1300]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY150_STATUS",end);DiagFAppendKeyU(p," tick=",tick,end);
    AppendText(p," status=",end);AppendNumber(p,g_replayStatus,end);
    const char* keys[]={" captures="," cached="," saves="," loads="," imports="," restores="," deferred="," owners="," tempSlots="," activeTemps="," peakTraffic="," peakRetained="," invalid="};
    const long values[]={g_replayCaptures,g_replayCached,g_replaySaves,g_replayLoads,g_replayImports,g_replayRestores,g_replayDeferred,g_replayOwners,g_replayTempsAllocated,g_replayActive,g_replayPeakTraffic,g_replayPeakRetained,g_replayInvalid};
    for(U32 i=0;i<13u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);
    const UPtr root=Read32(Address(0x00FE7228u));
    if(Readable(root,0x230u)) {
        const UPtr replay=Read32(root+0x22Cu),spooler=Read32(root+0x214u),world=Read32(root+0x204u);
        DiagFAppendKeyHex(p," simulation=",Read32(root+0xCCu),end);DiagFAppendKeyHex(p," replay=",replay,end);
        if(Readable(replay,0x4EA18u)) {
            DiagFAppendKeyU(p," mode=",Read32(replay+0x4E588u),end);DiagFAppendKeyU(p," snapshotState=",Read32(replay+0x438ECu),end);
            DiagFAppendKeyU(p," snapshotCount=",Read32(replay+0x438F0u),end);DiagFAppendKeyU(p," loading=",Read8(replay+0x4E9ECu),end);
            DiagFAppendKeyU(p," transition=",Read32(replay+0x4E9F4u),end);DiagFAppendKeyU(p," lifecycle=",Read32(replay+0x4EA14u),end);
            DiagFAppendKeyHex(p," clockBits=",Read32(replay+0x4E9A0u),end);
            const UPtr recording=Read32(replay+0x4E58Cu);
            if(Readable(recording,0x8594u)) {
                const UPtr cursor=Read32(recording+0x8554u);DiagFAppendKeyHex(p," eventCursor=",cursor,end);
                if(Readable(cursor,0x14u)){DiagFAppendKeyU(p," eventType=",Read32(cursor)&65535u,end);DiagFAppendKeyU(p," eventKind=",Read32(cursor+0x10u),end);}
            }
        }
        if(Readable(spooler,0xE6Cu)){DiagFAppendKeyU(p," pendingTraffic=",Read32(spooler+0xC8u),end);DiagFAppendKeyU(p," pendingRetained=",Read32(spooler+0x9A8u),end);DiagFAppendKeyU(p," spoolState=",Read32(spooler+0xDACu),end);}
        if(Readable(world,0x74DACu)){DiagFAppendKeyU(p," worldState=",Read32(world+0x6FD90u),end);DiagFAppendKeyU(p," worldStatus=",Read32(world+0x74DA8u),end);}
    }
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

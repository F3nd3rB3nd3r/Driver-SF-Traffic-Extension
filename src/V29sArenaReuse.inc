static void LegacyExit(U32 code,const char* reason,UPtr owner,UPtr cursor,U32 active,U32 recycled) {
    char line[1200]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_LEGACY_FATAL reason=",end);AppendText(p,reason,end);DiagFAppendKeyHex(p," exitCode=",code,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);DiagFAppendKeyHex(p," owner=",owner,end);
    DiagFAppendKeyHex(p," cursor=",cursor,end);DiagFAppendKeyU(p," active=",active,end);DiagFAppendKeyU(p," recycled=",recycled,end);
    DiagFAppendKeyU(p," spoolArenas=",static_cast<U32>(g_sSpool0Used),end);DiagFAppendKeyU(p," scratchArenas=",static_cast<U32>(g_sScratchUsed),end);
    DiagFAppendKeyU(p," cloneArenas=",static_cast<U32>(g_sCloneUsed),end);DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_reuseInvalid),end);
    DiagFAppendKeyU(p," guardBad=",static_cast<U32>(g_reuseGuardBad),end);DiagFAppendKeyU(p," allocationFailed=",static_cast<U32>(g_reuseAllocFailed),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);OutputDebugStringA(line);ExitProcess(code);
}
static bool ArenaReuseStartup() {
    // Validate the actual installed150 frame and source-offset merge clone.
    if(g_callbackStatus!=1 || g_sScratchHook!=2 || DSFV27B_RequestFramePatchCount!=13 || DSFV27B_Merge150Fix!=1 || !g_merge150Clone)return false;
    for(const RequestFramePatch& patch:g_requestFramePatches){
#if defined(DSF_V29S_WIDE_LIMITS)
        if(patch.at==0x5A7495u){if(!g_wideRequestProbeInstalled || Read8(Address(patch.at))!=0xE9u)return false;continue;}
#endif
        if(!patch.done || Read32(Address(patch.at)+patch.immOffset)!=patch.replacement)return false;}
    const UPtr clone=reinterpret_cast<UPtr>(g_merge150Clone);
    return Read32(clone+0x10u)==12u*kWideRequestNodes+16u && Read32(clone+0x16u)==12u*kWideRequestNodes+12u;
}
static void ArenaReuseStartLine() {
    const bool ready=ArenaReuseStartup();
    CapacityWrite(ready?"DSFV29s_ARENA_START revision=1 status=1 spoolPolicy=ownerReuse processLifetime=1 fixedOwnerLimit=0 privateNodes=256 scratchPolicy=expandedFrame150 scratchExternal=0 framePatches=13 newHooks=0\n":
        "DSFV29s_ARENA_START revision=1 status=-1 reason=parentFrameOrMerge\n");
    if(!ready)LegacyExit(0x2Du,"parentFrameOrMerge");
}
static void ArenaReusePoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    char line[1200]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_ARENA_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    const char* names[]={" spoolChecks="," scratchChecks="," spoolArenas="," scratchArenas="," spoolRedirects="," spoolReuses="," spoolIssuedPeak="," scratchPeak="," invalid="," guardBad="," allocationFailed="};
    const long values[]={g_reuseSpoolChecks,g_reuseScratchChecks,g_sSpool0Used,g_sScratchUsed,g_reuseRedirects,g_reuseResets,g_reuseSpoolIssuedPeak,g_reuseScratchPeak,g_reuseInvalid,g_reuseGuardBad,g_reuseAllocFailed};
    for(U32 n=0;n<11u;++n)DiagFAppendKeyU(p,names[n],static_cast<U32>(values[n]),end);
    DiagFAppendKeyHex(p," lastOwner=",static_cast<U32>(g_reuseLastOwner),end);DiagFAppendKeyHex(p," lastScratch=",static_cast<U32>(g_reuseLastScratch),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

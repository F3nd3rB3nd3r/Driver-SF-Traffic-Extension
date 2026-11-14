// Quiet-baseline capacity measurements; optional indexed-table test is separate.
#if !defined(DSF_V29S_PERF_CONTROL)
#error Capacity tracing requires the quiet performance-control baseline
#endif
#if defined(DSF_V29S_POOL_DIAG) || defined(DSF_V29S_LISTA_EXPANDED) || defined(DSF_V29S_LISTB_EXPANDED)
#error Capacity hooks overlap the optional pool hooks; keep native pool capacities
#endif

struct CapacityQueue {
    volatile long inserts, projectedPeak, sampledPeak, boundaryAttempts, invalidCounts, ownerChanges;
    volatile long lastOwner, firstReady, firstPrinted;
    U32 tick, owner, header, cursor, recycled, active, caller;
};
static CapacityQueue g_capacityQueues[4]={};
static const char* const kCapacityNames[4]={"audioBanks","retireIds","retainedVehicles","vehicleOperations"};
static DiagFDetour g_capacityDetours[3]={};
static UPtr g_capacityTramps[3]={};
static long g_capacityHooks=0;
static HANDLE g_capacityFile=nullptr;
static U32 g_capacityStart=0,g_capacityFileStatus=0;

static void CapacityWrite(const char* line) {
    if(g_capacityFile) {
        U32 written=0;
        const U32 length=DiagLength(line);
        if(!WriteFile(g_capacityFile,line,length,&written,nullptr) || written!=length)
            g_capacityFileStatus=2u;
    } else OutputDebugStringA(line);
}
static void CapacityOpen() {
    g_capacityStart=GetTickCount();
    char path[300]={};
    const U32 length=GetTempPathA(240u,path);
    if(!length || length>=240u) {g_capacityFileStatus=1u;return;}
    char* p=path+length;const char* end=path+sizeof(path)-1;
#if defined(DSF_V29S_CHAPTERS)
#if defined(DSF_V29S_CALLBACK512)
#if defined(DSF_V29S_ARENA_REUSE)
#if defined(DSF_V29S_BASELINE150)
#if defined(DSF_V29S_NOTIFY150)
#if defined(DSF_V29S_PARKED150)
#if defined(DSF_V29S_REPLAY150)
#if defined(DSF_V29S_REPLAY_PAUSE)
#if defined(DSF_V29S_REGMAP_REUSE)
#if defined(DSF_V29S_REPLAY_CLIENT)
    AppendText(p,"DSFV29s_replay_client_",end);
#elif defined(DSF_V29S_REPLAY_HITCH)
    AppendText(p,"DSFV29s_replay_hitch_",end);
#else
    AppendText(p,"DSFV29s_regmap_reuse_",end);
#endif
#else
    AppendText(p,"DSFV29s_replay_pause_",end);
#endif
#else
    AppendText(p,"DSFV29s_replay150_",end);
#endif
#else
    AppendText(p,"DSFV29s_parked150_",end);
#endif
#else
    AppendText(p,"DSFV29s_notify150_",end);
#endif
#else
    AppendText(p,"DSFV29s_baseline150_",end);
#endif
#else
    AppendText(p,"DSFV29s_arena_reuse_",end);
#endif
#else
    AppendText(p,"DSFV29s_callback512_",end);
#endif
#else
    AppendText(p,"DSFV29s_chapters_",end);
#endif
#elif defined(DSF_V29S_PENDING512)
    AppendText(p,"DSFV29s_pending512_",end);
#elif defined(DSF_V29S_CLEANUP_262144)
    AppendText(p,"DSFV29s_cleanup262144_",end);
#elif defined(DSF_V29S_AUDIO_MODEL192)
    AppendText(p,"DSFV29s_audio_model192_",end);
#elif defined(DSF_V29S_AUDIO_REASON_PROBE)
    AppendText(p,"DSFV29s_audio_reason_",end);
#elif defined(DSF_V29S_AUDIO_SLOTS_2048)
    AppendText(p,"DSFV29s_audio_slots2048_",end);
#elif defined(DSF_V29S_AUDIO_STATIC_64M)
    AppendText(p,"DSFV29s_audio_static64m_",end);
#elif defined(DSF_V29S_AUDIO_DEFAULT_64M)
    AppendText(p,"DSFV29s_audio_default64m_",end);
#elif defined(DSF_V29S_AUDIO_DEFAULT_16M)
    AppendText(p,"DSFV29s_audio_default16m_",end);
#elif defined(DSF_V29S_AUDIO_PROBE)
    AppendText(p,"DSFV29s_audio_probe_",end);
#elif defined(DSF_V29S_INDEXED_262144)
    AppendText(p,"DSFV29s_indexed262144_",end);
#else
    AppendText(p,"DSFV29s_capacity_",end);
#endif
    AppendNumber(p,static_cast<long>(GetCurrentProcessId()),end);
    AppendText(p,".log",end);*p=0;
    // Append-only, shared for reading; opened before hooks can report a boundary.
    g_capacityFile=CreateFileA(path,4u,3u,nullptr,4u,0x80u,nullptr);
    if(reinterpret_cast<UPtr>(g_capacityFile)==0xFFFFFFFFu || !g_capacityFile) {
        g_capacityFile=nullptr;g_capacityFileStatus=1u;
    }
}
#if defined(DSF_V29S_INDEXED_262144)
static void IndexedStartLine() {
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_INDEXED_START status=",end);AppendNumber(p,g_indexedStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_indexedReason,end);
    AppendText(p," startupCheckRevision=2",end);
    DiagFAppendKeyU(p," stockPreflight=",g_indexedStockVerified?1u:0u,end);
    DiagFAppendKeyU(p," ownedHookVerified=",g_indexedStartupVerified?1u:0u,end);
    DiagFAppendKeyU(p," patches=",g_indexedPatchCount,end);
    DiagFAppendKeyU(p," expectedPatches=",13u,end);
    DiagFAppendKeyHex(p," externalBase=",g_indexedBase,end);
    DiagFAppendKeyU(p," nativeSlots=",kIndexedNativeCount,end);
    DiagFAppendKeyU(p," testSlots=",kIndexedTestCount,end);
    DiagFAppendKeyU(p," pendingRedirectPatches=",g_sResourcePatchCount,end);
    DiagFAppendKeyU(p," baselineStatus=",static_cast<U32>(DSF150_Status),end);
    AppendText(p," startupPolicy=untouchedColdOnly activeMigration=0 hotHandleHooks=0\n",end);
    *p=0;CapacityWrite(line);
}
#endif
static void CapacityMax(volatile long& destination,U32 value) {
    if(value>0x7FFFFFFFu)return;
    long previous=InterlockedCompareExchange(&destination,0,0);
    while(value>static_cast<U32>(previous)) {
        const long observed=InterlockedCompareExchange(&destination,static_cast<long>(value),previous);
        if(observed==previous)break;
        previous=observed;
    }
}
static void CapacityFirstLine(U32 kind) {
    CapacityQueue& q=g_capacityQueues[kind];
    if(InterlockedCompareExchange(&q.firstReady,2,2)!=2 ||
       InterlockedCompareExchange(&q.firstPrinted,1,0)!=0)return;
    char line[480]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CAP_BOUNDARY tick=",end);DiagFAppendU(p,q.tick,end);
    AppendText(p," queue=",end);AppendText(p,kCapacityNames[kind],end);
    DiagFAppendKeyHex(p," owner=",q.owner,end);DiagFAppendKeyHex(p," header=",q.header,end);
    DiagFAppendKeyHex(p," cursor=",q.cursor,end);DiagFAppendKeyU(p," recycled=",q.recycled,end);
    DiagFAppendKeyU(p," activeBefore=",q.active,end);DiagFAppendKeyHex(p," callerOrSite=",q.caller,end);
    AppendText(p," nativeCapacity=50 beforeNativeWrite=1 noSuppression=1\n",end);*p=0;
    CapacityWrite(line); // At most one synchronous emergency line per queue kind.
}
static void CapacityObserve(U32 kind,UPtr owner,UPtr header,UPtr begin,UPtr end,U32 stride,U32 caller) {
    // The native allocator is about to read this same header. No VirtualQuery,
    // allocations, list walks or ordinary file writes occur on the normal path.
    CapacityQueue& q=g_capacityQueues[kind];
    const U32 cursor=Read32(header),recycled=Read32(header+4u),active=Read32(header+8u);
    InterlockedIncrement(&q.inserts);
    const long previous=InterlockedExchange(&q.lastOwner,static_cast<long>(owner));
    if(previous && static_cast<UPtr>(previous)!=owner)InterlockedIncrement(&q.ownerChanges);
    if(active<0x7FFFFFFFu)CapacityMax(q.projectedPeak,active+1u);
    else InterlockedIncrement(&q.invalidCounts);
    const bool badFresh=!recycled && (cursor<begin || cursor>=end || ((cursor-begin)%stride)!=0u);
    if(!badFresh && active<50u)return;
    InterlockedIncrement(&q.boundaryAttempts);
    if(InterlockedCompareExchange(&q.firstReady,1,0)!=0)return;
    q.tick=GetTickCount()-g_capacityStart;q.owner=owner;q.header=header;
    q.cursor=cursor;q.recycled=recycled;q.active=active;q.caller=caller;
    InterlockedExchange(&q.firstReady,2);
    CapacityFirstLine(kind);
}
extern "C" __declspec(noinline) void __cdecl CapacityAudioEntry(UPtr owner,U32 caller) {
    CapacityObserve(0u,owner,owner+0x92DCu,owner+0x92E8u,owner+0x9540u,12u,caller);
}
extern "C" __declspec(noinline) void __cdecl CapacityTrafficEntry(UPtr header,U32 caller) {
    const UPtr root=Read32(Address(kRootPtr));
    if(!root)return;
    const UPtr owner=Read32(root+0x214u);
    if(!owner)return;
    if(header==owner+0x5A4u)
        CapacityObserve(2u,owner,header,owner+0x5B0u,owner+0x998u,20u,caller);
    else if(header==owner+0x9A0u)
        CapacityObserve(3u,owner,header,owner+0x9ACu,owner+0xD94u,20u,caller);
}
extern "C" __declspec(noinline) void __cdecl CapacityRetireEntry(UPtr header) {
    const UPtr root=Read32(Address(kRootPtr));
    if(!root)return;
    const UPtr owner=Read32(root+0x214u);
    if(owner && header==owner+0x338u)
        CapacityObserve(1u,owner,header,owner+0x344u,owner+0x59Cu,12u,0x005A7DD1u);
}

// Preserve flags, all integer registers and the complete x87/SSE state.
// EBP anchors pushad's frame; the FXSAVE area below it is 16-byte aligned.
extern "C" __declspec(naked) void CapacityAudioBridge() {
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+36] // original return address
        push dword ptr [ebp+24] // original ECX
        call CapacityAudioEntry
        add esp,8
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        jmp dword ptr [g_capacityTramps]
    }
}
extern "C" __declspec(naked) void CapacityTrafficBridge() {
    __asm {
        pushfd
        // This allocator is shared widely. Unrelated callers pay only this
        // flags-preserving filter, without register/FP saves or C++ calls.
        cmp dword ptr [esp+4],005A68A7h
        je cap_traffic
        cmp dword ptr [esp+4],005A7E3Dh
        je cap_traffic
        cmp dword ptr [esp+4],005A7E6Dh
        jne cap_skip
    cap_traffic:
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+36]
        push dword ptr [ebp+24]
        call CapacityTrafficEntry
        add esp,8
        fxrstor [esp]
        mov esp,ebp
        popad
    cap_skip:
        popfd
        jmp dword ptr [g_capacityTramps+4]
    }
}
extern "C" __declspec(naked) void CapacityRetireBridge() {
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+24]
        call CapacityRetireEntry
        add esp,4
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        jmp dword ptr [g_capacityTramps+8]
    }
}
static bool CapacityInstall() {
    const U8 audio[]={0x83,0xB9,0xE0,0x92,0,0,0};
    const U8 traffic[]={0x83,0x79,0x04,0,0x8B,0x01};
    const struct {UPtr site;const U8* signature;U32 length;UPtr bridge;} sites[]={
        {0x00849870u,audio,sizeof(audio),reinterpret_cast<UPtr>(&CapacityAudioBridge)},
        {0x0083B170u,traffic,sizeof(traffic),reinterpret_cast<UPtr>(&CapacityTrafficBridge)},
        {0x005A7DD1u,traffic,sizeof(traffic),reinterpret_cast<UPtr>(&CapacityRetireBridge)}
    };
    // Stolen spans contain no relative instructions and end before native
    // branch targets. Validate all sites before installing any capacity hook.
    for(U32 i=0u;i<3u;++i) {
        if(!Readable(Address(sites[i].site),sites[i].length))return false;
        for(U32 j=0u;j<sites[i].length;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i].site)+j)!=sites[i].signature[j])return false;
    }
    for(U32 i=0u;i<3u;++i) {
        if(DiagFInstallDetour(g_capacityDetours[i],sites[i].site,sites[i].signature,
            sites[i].length,sites[i].bridge,g_capacityTramps[i]))continue;
        DiagFRestoreDetour(g_capacityDetours[i],g_capacityTramps[i]);
        while(i>0u) {--i;DiagFRestoreDetour(g_capacityDetours[i],g_capacityTramps[i]);}
        return false;
    }
    return true;
}
static void CapacityPoll() {
    static U32 previousStatus=0u,previousQueues=0u;
    static U32 samples=0u,emptySamples=0u,emptyTransitions=0u,pendingPeak=0u,emptySince=0u,emptyMsPeak=0u;
    static bool previouslyEmpty=false,linkedHeadSeen=false;
    const U32 tick=GetTickCount()-g_capacityStart;
    const UPtr rootAddress=Address(kRootPtr);
    const UPtr root=Readable(rootAddress,4u)?Read32(rootAddress):0u;
    const UPtr audio=root && Readable(root+0x50u,4u)?Read32(root+0x50u):0u;
    const UPtr traffic=root && Readable(root+0x214u,4u)?Read32(root+0x214u):0u;
    const UPtr headers[4]={audio?audio+0x92DCu:0u,traffic?traffic+0x338u:0u,
        traffic?traffic+0x5A4u:0u,traffic?traffic+0x9A0u:0u};
    U32 cursors[4]={},freeCounts[4]={},activeCounts[4]={};bool valid[4]={};
    for(U32 i=0u;i<4u;++i) {
        valid[i]=headers[i] && Readable(headers[i],12u);
        if(valid[i]) {
            cursors[i]=Read32(headers[i]);freeCounts[i]=Read32(headers[i]+4u);activeCounts[i]=Read32(headers[i]+8u);
            CapacityMax(g_capacityQueues[i].sampledPeak,activeCounts[i]);
        }
        CapacityFirstLine(i);
    }
    const UPtr resource=Address(0x01732BF0u);
    const bool readable=Readable(resource+0x27E094u,4u) && Readable(resource+0x127AC4u,4u) && Readable(resource+0x157AC8u,4u);
    const U32 head=readable?Read32(resource+0x27E094u):0u;
    const U32 pending=readable?Read32(resource+0x157AC8u):0u;
    const U32 hashCursor=readable?Read32(resource+0x127AC4u):0u;
    UPtr indexedBase=resource+0x17E094u;U32 indexedSlots=131072u;
#if defined(DSF_V29S_INDEXED_262144)
    if(g_indexedBase) {indexedBase=g_indexedBase;indexedSlots=kIndexedTestCount;}
#endif
    const bool headValid=!head || (head>=indexedBase && head-indexedBase<indexedSlots*8u && ((head-indexedBase)%8u)==0u);
    const bool hashInitialized=readable && hashCursor>=resource+0x20000u && hashCursor<=resource+0x127AC0u;
    // Require a previously observed valid non-null native head as well as the
    // constructor's hash cursor; zeroed startup memory is not exhaustion.
    if(hashInitialized && head && headValid)linkedHeadSeen=true;
    const bool initialized=hashInitialized && linkedHeadSeen;
#if defined(DSF_V29S_INDEXED_262144)
    if(initialized && g_indexedStatus==1)g_indexedStatus=2;
#endif
    if(initialized) {
        ++samples;if(pending>pendingPeak)pendingPeak=pending;
        if(!head) {
            ++emptySamples;
            if(!previouslyEmpty) {++emptyTransitions;emptySince=tick;}
            const U32 elapsed=tick-emptySince;if(elapsed>emptyMsPeak)emptyMsPeak=elapsed;
        }
        previouslyEmpty=!head;
    } else previouslyEmpty=false;
    if(tick-previousStatus>=1000u || !previousStatus) {
        previousStatus=tick?tick:1u;
        char line[1024]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_CAP_STATUS tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_capacityHooks),end);
        DiagFAppendKeyU(p," startupStatus=",static_cast<U32>(DSF150_Status),end);
        DiagFAppendKeyU(p," graphicsSlots=",static_cast<U32>(DSF150_GraphicsSlots),end);
        DiagFAppendKeyU(p," requestLast=",static_cast<U32>(DSFV27B_RequestLastCount),end);
        DiagFAppendKeyU(p," chapterHits=",static_cast<U32>(DSF_Chapter7PreloadHits),end);
        DiagFAppendKeyU(p," initializedByObservedHead=",initialized?1u:0u,end);
        DiagFAppendKeyHex(p," indexedFreeHead=",head,end);DiagFAppendKeyU(p," headValid=",headValid?1u:0u,end);
#if defined(DSF_V29S_INDEXED_262144)
        AppendText(p," indexedStatus=",end);AppendNumber(p,g_indexedStatus,end);
        DiagFAppendKeyHex(p," indexedBase=",indexedBase,end);
        DiagFAppendKeyU(p," indexedSlots=",indexedSlots,end);
        DiagFAppendKeyU(p," indexedHeadIndex=",head && headValid?(head-indexedBase)/8u:0xFFFFFFFFu,end);
#endif
        DiagFAppendKeyU(p," pending=",pending,end);DiagFAppendKeyU(p," pendingPeakSampled=",pendingPeak,end);
        DiagFAppendKeyU(p," samples=",samples,end);DiagFAppendKeyU(p," emptySamples=",emptySamples,end);
        DiagFAppendKeyU(p," emptyTransitionsSampled=",emptyTransitions,end);
        DiagFAppendKeyU(p," emptyMsPeakSampled=",emptyMsPeak,end);
        DiagFAppendKeyU(p," fileStatus=",g_capacityFileStatus,end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    if(tick-previousQueues<5000u && previousQueues)return;
    previousQueues=tick?tick:1u;
    for(U32 i=0u;i<4u;++i) {
        const CapacityQueue& q=g_capacityQueues[i];
        char line[520]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_CAP_QUEUE tick=",end);DiagFAppendU(p,tick,end);
        AppendText(p," queue=",end);AppendText(p,kCapacityNames[i],end);
        DiagFAppendKeyU(p," valid=",valid[i]?1u:0u,end);DiagFAppendKeyHex(p," header=",headers[i],end);
        DiagFAppendKeyHex(p," cursor=",cursors[i],end);DiagFAppendKeyU(p," recycled=",freeCounts[i],end);
        DiagFAppendKeyU(p," active=",activeCounts[i],end);
        DiagFAppendKeyU(p," inserts=",static_cast<U32>(q.inserts),end);
        DiagFAppendKeyU(p," peakAfterInsertProjected=",static_cast<U32>(q.projectedPeak),end);
        DiagFAppendKeyU(p," peakActiveSampled=",static_cast<U32>(q.sampledPeak),end);
        DiagFAppendKeyU(p," boundaryAttempts=",static_cast<U32>(q.boundaryAttempts),end);
        DiagFAppendKeyU(p," invalidCounts=",static_cast<U32>(q.invalidCounts),end);
        DiagFAppendKeyU(p," ownerChanges=",static_cast<U32>(q.ownerChanges),end);
        AppendText(p," nativeCapacity=50\n",end);*p=0;CapacityWrite(line);
    }
}
static void CapacityStartLine() {
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CAP_START hooks=",end);DiagFAppendU(p,static_cast<U32>(g_capacityHooks),end);
    AppendText(p," expectedHooks=3 request=150 slots=160 ordered=1 baseline=quiet ED_EF=1 capacityChanges=",end);
#if defined(DSF_V29S_INDEXED_262144)
    DiagFAppendU(p,g_indexedBase?1u:0u,end);
#else
    DiagFAppendU(p,0u,end);
#endif
    AppendText(p," sampleMs=200 statusMs=1000 queueMs=5000 emergencyMax=4 hotHandleHooks=0 preserveFX=1\n",end);
    *p=0;CapacityWrite(line);
    CapacityWrite("DSFV29s_CAP_SCOPE peaksAfterInsert=projected nativeHandleExhaustion=sampled flagsAndReturnValues=preserved boundaryWrites=notPrevented noREADYorDrawableInference=1\n");
}

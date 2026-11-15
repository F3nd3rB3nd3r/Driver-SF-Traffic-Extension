// Diagnostic extension only. No capacity, request selection, release policy or
// native result changes. Native pointers are inspected only on their own paths.
#if defined(DSF_V29S_WIDE_LIMITS)
static const U32 kBaselineEvents=8u,kBaselineMetrics=12u;
#define kBaselineModels (g_limits.requests)
#else
static const U32 kBaselineModels=150u,kBaselineEvents=8u,kBaselineMetrics=12u;
#endif
struct BaselineModel {
    U32 id,ready,firstReady,readyChecks,readyLosses,drawOriginal,drawResolved;
    U32 drawFallback,lastResolved,audioStart[8],audioSample[8];
};
struct BaselineState {
    U32 seq,tick,owner,caller,count,originalValid,duplicates,hash;
    int originalCount;
#if defined(DSF_V29S_WIDE_LIMITS)
U32* original;
#else
U32 original[150];
#endif
    U32 checkpointTick,checkpointDuration,checkpoints,readyCount,allReadyTick,drawMismatch,audioSampleTick;
    U32 metricStart[12],metricPeak[12],metricLast[12],metricSamples;

#if defined(DSF_V29S_WIDE_LIMITS)
BaselineModel* models;
#else
BaselineModel models[150];
#endif
};
struct BaselineEvent {const char* kind;BaselineState state;};
#if defined(DSF_V29S_WIDE_LIMITS)
static void WideBaselineInit(BaselineState& s){if(!s.original)s.original=static_cast<U32*>(WideAllocate(g_limits.requests*4u));if(!s.models)s.models=static_cast<BaselineModel*>(WideAllocate(g_limits.requests*sizeof(BaselineModel)));}
static void WideBaselineReset(BaselineState& s){WideBaselineInit(s);U32* ids=s.original;BaselineModel* models=s.models;memset(&s,0,sizeof(s));s.original=ids;s.models=models;memset(ids,0,g_limits.requests*4u);memset(models,0,g_limits.requests*sizeof(BaselineModel));}
static void WideBaselineCopy(BaselineState& to,const BaselineState& from){WideBaselineInit(to);U32* ids=to.original;BaselineModel* models=to.models;memcpy(&to,&from,sizeof(to));to.original=ids;to.models=models;memcpy(models,from.models,from.count*sizeof(BaselineModel));if(from.originalValid && from.originalCount>0)memcpy(ids,from.original,static_cast<U32>(from.originalCount)*4u);}
static void WideBaselineEventCopy(BaselineEvent& to,const BaselineEvent& from){to.kind=from.kind;WideBaselineCopy(to.state,from.state);}
#endif
static BaselineState g_baseline={},g_baselinePrint={};
static BaselineEvent g_baselineEvents[8]={},g_baselineEventPrint={};
static volatile long g_baselineLock=0,g_baselineDrops=0,g_baselineLockSkips=0,g_baselineObsolete=0;
static volatile long g_baselineReadyBusy=0,g_baselineDrawUnmapped=0,g_baselineDrawCalls=0;
static volatile long g_baselineMemoryErrors=0;
static U32 g_baselineSequence=0,g_baselineRead=0,g_baselineWrite=0,g_baselineLastCheck=0;
static bool g_baselineInstalled=false;
static DiagFDetour g_baselineUpdateDetour={};
extern "C" UPtr g_baselineUpdateTrampoline=0,g_baselineDrawTarget=0;
static bool BaselineLock(){if(!InterlockedCompareExchange(&g_baselineLock,1,0))return true;InterlockedIncrement(&g_baselineLockSkips);return false;}
static void BaselineUnlock(){InterlockedExchange(&g_baselineLock,0);}
static void BaselineAudio(U32 id,U32* out) {
    for(U32 n=0;n<8u;++n)out[n]=0u;
#if defined(DSF_V29S_WIDE_LIMITS)
    while(InterlockedExchange(&g_wideAudioKeyLock,1)){}AudioModelKey* found=g_wideAudioBuckets[id&65535u];while(found && static_cast<U32>(found->key)!=id)found=found->hashNext;InterlockedExchange(&g_wideAudioKeyLock,0);
    if(found) {
#else
    for(U32 n=0;n<512u;++n)if(static_cast<U32>(g_audioModelKeys[n].key)==id) {
#endif
#if defined(DSF_V29S_WIDE_LIMITS)
        const AudioModelKey& k=*found;
#else
        const AudioModelKey& k=g_audioModelKeys[n];
#endif
        const long values[8]={k.stores,k.removes,k.hits,k.misses,k.bankReady,k.bankNotReady,k.setupOk,k.setupFail};
        for(U32 j=0;j<8u;++j)out[j]=static_cast<U32>(values[j]);return;
    }
}
static void BaselineMetrics(U32* out) {
    out[0]=static_cast<U32>(g_cleanupCurrent);out[1]=static_cast<U32>(g_pendingCurrent);
    out[2]=out[3]=0u;
    if(g_callbackGroups)for(U32 i=0;i<21u;++i){const U32 n=static_cast<U32>(g_callbackGroups[i].active);out[2]+=n;if(n>out[3])out[3]=n;}
    for(U32 i=0;i<3u;++i)out[4u+i]=static_cast<U32>(g_audioSlotsLists[i].updateCount);
    out[7]=static_cast<U32>(g_audioProbeEmitterPeak); // Explicitly lifetime peak, not epoch occupancy.
    out[8]=out[9]=0u;DiagFProcessMemoryCounters32 m={};m.cb=sizeof(m);
    if(GetProcessMemoryInfo(GetCurrentProcess(),&m,sizeof(m))){out[8]=m.privateUsage;out[9]=m.workingSetSize;}
    else InterlockedIncrement(&g_baselineMemoryErrors);
    out[10]=static_cast<U32>(DSFV29_HashExtraGranted);out[11]=static_cast<U32>(g_reuseAllocations);
}
static void BaselineCaptureAudio(BaselineState& s) {
    for(U32 i=0;i<s.count;++i)BaselineAudio(s.models[i].id,s.models[i].audioSample);
    s.audioSampleTick=GetTickCount()-g_capacityStart;
}
static void BaselineQueue(const char* kind) {
    if(g_baselineWrite-g_baselineRead>=kBaselineEvents){InterlockedIncrement(&g_baselineDrops);return;}
    BaselineCaptureAudio(g_baseline);
    BaselineEvent& e=g_baselineEvents[g_baselineWrite%kBaselineEvents];e.kind=kind;

#if defined(DSF_V29S_WIDE_LIMITS)
WideBaselineCopy(e.state,g_baseline);
#else
memcpy(&e.state,&g_baseline,sizeof(g_baseline));
#endif
++g_baselineWrite;
}
static void BaselineRequest(UPtr owner,UPtr caller,const U32* original,int originalCount,const U32* effective,U32 count,const char* result) {
    if(!g_baselineInstalled || !BaselineLock())return;
    const bool accepted=result[0]=='a';
    if(accepted && g_baseline.seq)BaselineQueue("superseded");
    // Rejections do not replace the active epoch. Use the ring directly.
    if(!accepted) {
        if(g_baselineWrite-g_baselineRead>=kBaselineEvents){InterlockedIncrement(&g_baselineDrops);BaselineUnlock();return;}
        BaselineEvent& e=g_baselineEvents[g_baselineWrite%kBaselineEvents];e.kind=result;

#if defined(DSF_V29S_WIDE_LIMITS)
WideBaselineReset(e.state);
#else
memset(&e.state,0,sizeof(e.state));
#endif
e.state.seq=++g_baselineSequence;e.state.tick=GetTickCount()-g_capacityStart;
        e.state.owner=owner;e.state.caller=caller;e.state.originalCount=originalCount;++g_baselineWrite;
        BaselineUnlock();return;
    }

#if defined(DSF_V29S_WIDE_LIMITS)
WideBaselineReset(g_baseline);
#else
memset(&g_baseline,0,sizeof(g_baseline));
#endif
g_baseline.seq=++g_baselineSequence;
    g_baseline.tick=GetTickCount()-g_capacityStart;g_baseline.owner=owner;g_baseline.caller=caller;
    g_baseline.count=count;g_baseline.originalCount=originalCount;
    g_baseline.originalValid=originalCount>=0 && originalCount<=static_cast<int>(kBaselineModels) &&
        (!originalCount || (original && Readable(reinterpret_cast<UPtr>(original),static_cast<U32>(originalCount)*4u)));
    if(g_baseline.originalValid)for(int i=0;i<originalCount;++i)g_baseline.original[i]=original[i];
    U32 hash=2166136261u;
    for(U32 i=0;i<count;++i) {
        BaselineModel& m=g_baseline.models[i];m.id=effective[i];hash=(hash^m.id)*16777619u;
        for(U32 j=0;j<i;++j)if(g_baseline.models[j].id==m.id){++g_baseline.duplicates;break;}
        BaselineAudio(m.id,m.audioStart);
    }
    g_baseline.hash=hash;BaselineMetrics(g_baseline.metricStart);
    for(U32 i=0;i<12u;++i)g_baseline.metricPeak[i]=g_baseline.metricLast[i]=g_baseline.metricStart[i];
    BaselineQueue("accepted");BaselineUnlock();
}
extern "C" __declspec(noinline) void __cdecl BaselineReadyEntry(UPtr owner) {
    if(!g_baselineInstalled || InterlockedCompareExchange(&g_baselineReadyBusy,1,0))return;
    const U32 now=GetTickCount()-g_capacityStart;
    if(now-g_baselineLastCheck<1000u){InterlockedExchange(&g_baselineReadyBusy,0);return;}

#if defined(DSF_V29S_WIDE_LIMITS)
WideScratchLease readyLease(g_limits.requests*8u);U32* ids=static_cast<U32*>(readyLease.data);U32* ready=ids+g_limits.requests;U32 count=0,seq=0;
#else
U32 ids[150]={},ready[150]={},count=0,seq=0;
#endif
    if(BaselineLock()) {
        if(g_baseline.owner==owner){count=g_baseline.count;seq=g_baseline.seq;for(U32 i=0;i<count;++i)ids[i]=g_baseline.models[i].id;}
        BaselineUnlock();
    }
    if(!count){InterlockedExchange(&g_baselineReadyBusy,0);return;}
    g_baselineLastCheck=now;
    const UPtr root=Read32(Address(kRootPtr));
    const UPtr manager=Readable(owner+0xDA8u,4u)?Read32(owner+0xDA8u):0u;
    if(!root || !manager || !Readable(root+0x214u,4u) || Read32(root+0x214u)!=owner ||
       !Readable(root+0x204u,4u) || Read32(root+0x204u)!=manager || manager+0xAF68u!=g_pkg0 ||
       !Readable(manager+0x6FD94u,4u) || !Read32(manager+0x6FD94u)) {
        InterlockedIncrement(&g_baselineObsolete);InterlockedExchange(&g_baselineReadyBusy,0);return;
    }
    typedef U8 (__thiscall* ReadyFn)(void*,U32,U32);
    for(U32 i=0;i<count;++i)ready[i]=reinterpret_cast<ReadyFn>(Address(0x0059CFC0u))(reinterpret_cast<void*>(manager),0u,ids[i])?1u:0u;
    if(BaselineLock()) {
        if(g_baseline.seq==seq && g_baseline.owner==owner) {
            U32 total=0;for(U32 i=0;i<count;++i) {
                BaselineModel& m=g_baseline.models[i];if(m.ready && !ready[i])++m.readyLosses;
                m.ready=ready[i];++m.readyChecks;if(ready[i] && !m.firstReady)m.firstReady=now;total+=ready[i];
            }
            g_baseline.readyCount=total;g_baseline.checkpointTick=now;
            g_baseline.checkpointDuration=GetTickCount()-g_capacityStart-now;++g_baseline.checkpoints;
            if(total==count && !g_baseline.allReadyTick)g_baseline.allReadyTick=now;
        }else InterlockedIncrement(&g_baselineObsolete);
        BaselineUnlock();
    }
    InterlockedExchange(&g_baselineReadyBusy,0);
}
extern "C" __declspec(noinline) void __cdecl BaselineDrawEntry(UPtr vehicle) {
    if(!g_baselineInstalled)return;
    // EBX is the live gameVehicle at the exact native body draw CALL.
    const U32 original=Read32(vehicle+0x5ACu);const UPtr model=Read32(vehicle+0x5B4u);
    U32 resolved=0xFFFFFFFFu;
    if(model && Readable(model,4u) && Read32(model)==Address(0x00E39EACu) && Readable(model+0x4850u,4u)) {
        const UPtr definition=Read32(model+0x4850u);if(definition && Readable(definition+0x14u,4u))resolved=Read32(definition+0x14u);
    }
    InterlockedIncrement(&g_baselineDrawCalls);
    if(resolved==0xFFFFFFFFu)InterlockedIncrement(&g_baselineDrawUnmapped);
    if(!BaselineLock())return;
    if(original!=resolved && resolved!=0xFFFFFFFFu)++g_baseline.drawMismatch;
    for(U32 i=0;i<g_baseline.count;++i){BaselineModel& m=g_baseline.models[i];
        if(m.id==original){++m.drawOriginal;m.lastResolved=resolved;if(original!=resolved && resolved!=0xFFFFFFFFu)++m.drawFallback;}
        if(m.id==resolved)++m.drawResolved;
    }
    BaselineUnlock();
}
#define BASELINE_BRIDGE(name,entry,reg,target) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} \
    __asm {pushad} \
    __asm {mov ebp,esp} \
    __asm {sub esp,528} \
    __asm {and esp,0FFFFFFF0h} \
    __asm {fxsave [esp]} \
    __asm {cld} \
    __asm {push reg} \
    __asm {call entry} \
    __asm {add esp,4} \
    __asm {fxrstor [esp]} \
    __asm {mov esp,ebp} \
    __asm {popad} \
    __asm {popfd} \
    __asm {jmp dword ptr [target]} \
}
BASELINE_BRIDGE(BaselineUpdateBridge,BaselineReadyEntry,ecx,g_baselineUpdateTrampoline)
BASELINE_BRIDGE(BaselineDrawBridge,BaselineDrawEntry,ebx,g_baselineDrawTarget)
#undef BASELINE_BRIDGE
static void BaselineInstall() {
    const U8 update[10]={0x51,0x56,0x8B,0xF1,0x8B,0x86,0xAC,0x0D,0,0};
    const U8 draw[5]={0xE8,0x8F,0x34,0xFD,0xFF};
    const U8 ready[8]={0x53,0x55,0x8B,0x6C,0x24,0x0C,0x8B,0xD9};
    bool signatures=ArenaReuseStartup();
    for(U32 i=0;i<10u;++i)if(Read8(Address(0x005A7B80u)+i)!=update[i])signatures=false;
    for(U32 i=0;i<5u;++i)if(Read8(Address(0x008D590Cu)+i)!=draw[i])signatures=false;
    for(U32 i=0;i<8u;++i)if(Read8(Address(0x0059CFC0u)+i)!=ready[i])signatures=false;
    if(signatures && DiagFInstallDetour(g_baselineUpdateDetour,0x005A7B80u,update,10u,reinterpret_cast<UPtr>(&BaselineUpdateBridge),g_baselineUpdateTrampoline)) {
        const UPtr site=Address(0x008D590Cu);DWORD old=0,discarded=0;
        if(VirtualProtect(reinterpret_cast<void*>(site),5u,kPageExecuteReadWrite,&old)) {
            g_baselineDrawTarget=Address(0x008A8DA0u);
            Write32(site+1u,reinterpret_cast<UPtr>(&BaselineDrawBridge)-site-5u);
            const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5u);
            const BOOL restored=VirtualProtect(reinterpret_cast<void*>(site),5u,old,&discarded);
            if(flushed && restored)g_baselineInstalled=true;
            else {DWORD retry=0;if(VirtualProtect(reinterpret_cast<void*>(site),5u,kPageExecuteReadWrite,&retry)) {
                Write32(site+1u,0xFFFD348Fu);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5u);
                VirtualProtect(reinterpret_cast<void*>(site),5u,old,&discarded);
            }}
        }
        if(!g_baselineInstalled)DiagFRestoreDetour(g_baselineUpdateDetour,g_baselineUpdateTrampoline);
    }
    CapacityWrite(g_baselineInstalled?"DSFV29s_BASELINE_START revision=1 status=1 requests=150 graphics=160 capacityChanges=0 ready=0059CFC0 type=0 checkpointMs=1000 draw=008D590C drawMeansPixels=0 eventSlots=8 summaryMs=1000 modelsMs=5000 memoryUnit=bytes MiB=1048576\n":
        "DSFV29s_BASELINE_START revision=1 status=-1 reason=parentSignatureOrHookInstall\n");
}
static void BaselinePrint(const BaselineState& s,const char* kind,bool models) {
#if defined(DSF_V29S_WIDE_LIMITS)
    const U32 lineBytes=g_limits.requests*7u+2300u;WideScratchLease lineLease(lineBytes);char* line=static_cast<char*>(lineLease.data);char* p=line;const char* end=line+lineBytes-1u;
#else
    char line[2300]={};char* p=line;const char* end=line+sizeof(line)-1;
#endif
    AppendText(p,"DSFV29s_BASELINE_PHASE kind=",end);AppendText(p,kind,end);
    DiagFAppendKeyU(p," seq=",s.seq,end);DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyU(p," started=",s.tick,end);DiagFAppendKeyHex(p," owner=",s.owner,end);DiagFAppendKeyHex(p," caller=",s.caller,end);
    AppendText(p," originalCount=",end);AppendNumber(p,s.originalCount,end);DiagFAppendKeyU(p," count=",s.count,end);
    DiagFAppendKeyU(p," originalValid=",s.originalValid,end);DiagFAppendKeyU(p," duplicates=",s.duplicates,end);DiagFAppendKeyHex(p," rosterHash=",s.hash,end);
    DiagFAppendKeyU(p," checkpoints=",s.checkpoints,end);DiagFAppendKeyU(p," checkpointTick=",s.checkpointTick,end);
    DiagFAppendKeyU(p," checkpointDurationMs=",s.checkpointDuration,end);
    DiagFAppendKeyU(p," checkpointAgeMs=",s.checkpoints?GetTickCount()-g_capacityStart-s.checkpointTick:0u,end);
    DiagFAppendKeyU(p," ready=",s.readyCount,end);DiagFAppendKeyU(p," allReadyTick=",s.allReadyTick,end);
    DiagFAppendKeyU(p," drawMismatch=",s.drawMismatch,end);DiagFAppendKeyU(p," metricSamples=",s.metricSamples,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    if(kind[0]=='a')for(U32 which=0;which<2u;++which) {
        p=line;AppendText(p,"DSFV29s_BASELINE_ROSTER",end);DiagFAppendKeyU(p," seq=",s.seq,end);
        AppendText(p,which?" kind=effective ids=":" kind=original ids=",end);
        const U32 count=which?s.count:s.originalValid?static_cast<U32>(s.originalCount):0u;
        for(U32 i=0;i<count;++i){if(i)AppendText(p,",",end);DiagFAppendU(p,which?s.models[i].id:s.original[i],end);}
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    if(!models)return;
    for(U32 i=0;i<s.count;++i) {
        const BaselineModel& m=s.models[i];
        p=line;AppendText(p,"DSFV29s_BASELINE_MODEL",end);DiagFAppendKeyU(p," seq=",s.seq,end);DiagFAppendKeyU(p," model=",m.id,end);
        DiagFAppendKeyU(p," ready=",m.ready,end);DiagFAppendKeyU(p," checks=",m.readyChecks,end);DiagFAppendKeyU(p," firstReady=",m.firstReady,end);
        DiagFAppendKeyU(p," readyLosses=",m.readyLosses,end);DiagFAppendKeyU(p," bodyCallsOriginal=",m.drawOriginal,end);DiagFAppendKeyU(p," bodyCallsResolved=",m.drawResolved,end);
        DiagFAppendKeyU(p," bodyFallbackCalls=",m.drawFallback,end);DiagFAppendKeyU(p," lastResolvedModel=",m.lastResolved,end);
        const char* keys[8]={" audioStoresObserved="," audioRemovesObserved="," profileHitsObserved="," profileMissesObserved="," bankReadyObserved="," bankNotReadyObserved="," setupOkObserved="," setupFailObserved="};
        // Scalar observations are frozen before supersession, never re-read
        // at print time from a later phase. They are not an atomic audio transaction.
        for(U32 n=0;n<8u;++n){AppendText(p,keys[n],end);DiagFAppendU(p,m.audioSample[n],end);}
        AppendText(p," audioAtStart=",end);for(U32 n=0;n<8u;++n){if(n)AppendText(p,",",end);DiagFAppendU(p,m.audioStart[n],end);}
        DiagFAppendKeyU(p," audioSampleTick=",s.audioSampleTick,end);
        AppendText(p," audioCounters=cumulativeSample audibleVerified=0\n",end);*p=0;CapacityWrite(line);
    }
    const char* names[12]={"cleanupEntries","pendingMapEntries","callbackEntries","callbackGroupEntries","audioA0Slots","audioA1Slots","audioBSlots","emitterLifetimePeak","processPrivateBytes","processWorkingBytes","hashExtraLifetimeIssued","spoolLifetimeArenas"};
    for(U32 i=0;i<12u;++i) {
        p=line;AppendText(p,"DSFV29s_BASELINE_METRIC",end);DiagFAppendKeyU(p," seq=",s.seq,end);AppendText(p," name=",end);AppendText(p,names[i],end);
        DiagFAppendKeyU(p," start=",s.metricStart[i],end);DiagFAppendKeyU(p," peakSampled=",s.metricPeak[i],end);DiagFAppendKeyU(p," lastSampled=",s.metricLast[i],end);
        AppendText(p," sampleMs=200 geometryTextureVRAM=unmeasured\n",end);*p=0;CapacityWrite(line);
    }
}
static void BaselinePoll() {
    if(!g_baselineInstalled)return;
    // One monitor consumer. Never inspect game vehicles or invoke native
    // resource APIs here. Only PSAPI and published scalar diagnostic counters.
    U32 metrics[12]={};BaselineMetrics(metrics);
    if(BaselineLock()) {
        for(U32 i=0;i<12u;++i){g_baseline.metricLast[i]=metrics[i];if(metrics[i]>g_baseline.metricPeak[i])g_baseline.metricPeak[i]=metrics[i];}
        if(g_baseline.seq)++g_baseline.metricSamples;BaselineUnlock();
    }
    for(U32 n=0;n<8u;++n) {
        bool have=false;if(BaselineLock()) {
            if(g_baselineRead<g_baselineWrite){
#if defined(DSF_V29S_WIDE_LIMITS)
WideBaselineEventCopy(g_baselineEventPrint,g_baselineEvents[g_baselineRead%8u]);
#else
memcpy(&g_baselineEventPrint,&g_baselineEvents[g_baselineRead%8u],sizeof(g_baselineEventPrint));
#endif
++g_baselineRead;have=true;}
            BaselineUnlock();
        }
        if(!have)break;BaselinePrint(g_baselineEventPrint.state,g_baselineEventPrint.kind,true);
    }
    static U32 last=0,lastModels=0;const U32 now=GetTickCount()-g_capacityStart;
    if(now-last<1000u)return;last=now;
    if(!BaselineLock())return;BaselineCaptureAudio(g_baseline);
#if defined(DSF_V29S_WIDE_LIMITS)
WideBaselineCopy(g_baselinePrint,g_baseline);
#else
memcpy(&g_baselinePrint,&g_baseline,sizeof(g_baselinePrint));
#endif
BaselineUnlock();
    const bool models=now-lastModels>=5000u;if(models)lastModels=now;
    if(g_baselinePrint.seq)BaselinePrint(g_baselinePrint,"sample",models);
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;AppendText(p,"DSFV29s_BASELINE_STATUS",end);
    DiagFAppendKeyU(p," tick=",now,end);DiagFAppendKeyU(p," eventDrops=",static_cast<U32>(g_baselineDrops),end);
    DiagFAppendKeyU(p," lockSkips=",static_cast<U32>(g_baselineLockSkips),end);DiagFAppendKeyU(p," obsoleteChecks=",static_cast<U32>(g_baselineObsolete),end);
    DiagFAppendKeyU(p," bodyCalls=",static_cast<U32>(g_baselineDrawCalls),end);DiagFAppendKeyU(p," bodyUnmapped=",static_cast<U32>(g_baselineDrawUnmapped),end);
    DiagFAppendKeyU(p," memorySnapshotFailures=",static_cast<U32>(g_baselineMemoryErrors),end);
    AppendText(p," sharedAudioBytes=see_AUDIO_PRESSURE GPUBytes=unmeasured noFinalExitSnapshot=1\n",end);*p=0;CapacityWrite(line);
}

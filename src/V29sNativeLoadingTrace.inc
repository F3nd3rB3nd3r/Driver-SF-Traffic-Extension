// V29s: exact Driver.exe 2011 executable-specific, *observational* native
// request/lifecycle probes. Unlike V29q, no per-hash-node or hot-frame hook.
// No resource state, capacity, return value or branch is modified.
// Sites: 005A7490 reconcile, 0059DB40 dispatch gate, 0059D740 dispatch body,
//        0059D778 successful whitelist/membership test, 0059BAD0 handler transfer.
// Context is captured at entry and the original complete instructions execute
// in byte-signature-checked trampolines. No I/O or allocation at hook sites.

extern "C" __declspec(dllexport) volatile long DSFV29s_Hooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_Reconcile=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_GateCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_GateBlocked=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_Dispatched=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_MembershipPass=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_TransferCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_TransferWithHandler=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastRoster=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastGateByte=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastKind=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastId=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastTransferId=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastTransferState=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_IdOverflow=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_WaitHooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_WaitFailA=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_WaitFailB=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_FirstWaitFailA=-1;
extern "C" __declspec(dllexport) volatile long DSFV29s_FirstWaitFailB=-1;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastWaitFailA=-1;
extern "C" __declspec(dllexport) volatile long DSFV29s_LastWaitFailB=-1;

static DiagFDetour g_sHooks[5]={};
static UPtr g_sTramp[5]={};
static DiagFDetour g_sWaitHooks[2]={};
static UPtr g_sWaitTramp[2]={};
static volatile long g_sGateByKind[6]={},g_sBlockedByKind[6]={};
static volatile long g_sGateById[512]={},g_sBlockedById[512]={};
static volatile long g_sDispatchById[512]={},g_sPassById[512]={},g_sTransferById[512]={};
static volatile long g_sTransferState[16]={};
// Failure sites are reached only after a native membership result. A zero here
// means no failure was observed; it does not mean the ID passed or was reached.
static volatile long g_sWaitFailAById[512]={},g_sWaitFailBById[512]={};
static volatile long g_sWaitLastAById[512]={},g_sWaitLastBById[512]={};
static volatile long g_sWaitIdOverflow=0;
#if defined(DSF_V29S_MODEL_DIAG)
// 00890949 follows the per-vehicle catalog lookup. EDI is the model ID read
// from the vehicle definition, EAX is its descriptor, ESI is the new object.
// Counters are bounded; no file I/O or allocation runs on the game thread.
static volatile long g_sModelHook=0,g_sModelCalls=0,g_sModelNull=0,g_sModelOutOfRange=0;
static volatile long g_sModelById[512]={},g_sModelLastDescriptor[512]={};
static volatile long g_sModelPrinted[512]={};
static DiagFDetour g_sModelDetour={};
static UPtr g_sModelTramp=0;
extern "C" __declspec(noinline) void __cdecl DiagSRecordModel(U32 id,UPtr descriptor) {
    InterlockedIncrement(&g_sModelCalls);
    if(!descriptor)InterlockedIncrement(&g_sModelNull);
    if(id<512u) {
        InterlockedIncrement(&g_sModelById[id]);
        g_sModelLastDescriptor[id]=static_cast<long>(descriptor);
    } else InterlockedIncrement(&g_sModelOutOfRange);
}
extern "C" __declspec(naked) void DiagSModelBridge() {
    __asm {
        pushfd
        pushad
        push eax
        push edi
        call DiagSRecordModel
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sModelTramp]
    }
}
static bool DiagSModelInstall() {
    const U8 sig[]={0x89,0x86,0x58,0x48,0x00,0x00};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sModelDetour,0x00890949u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSModelBridge),g_sModelTramp);
}
#endif
#if defined(DSF_V29S_LIVE_MODEL_DIAG)
// These two model-dependent construction paths read the current vehicle ID
// through 0088C8F0. Capture that ID before either path consults the catalog.
static volatile long g_sLiveModelHooks=0,g_sLiveModelCalls[2]={};
static volatile long g_sLiveModelInvalid[2]={},g_sLiveModelOutOfRange[2]={};
static volatile long g_sLiveModelById[2][512]={},g_sLiveModelPrinted[2][512]={};
static DiagFDetour g_sLiveModelDetours[2]={};
static UPtr g_sLiveModelTramps[2]={};
extern "C" __declspec(noinline) void __cdecl DiagSRecordLiveModel(UPtr vehicle,U32 site) {
    if(site>=2u)return;
    InterlockedIncrement(&g_sLiveModelCalls[site]);
    if(!Readable(vehicle+0x4850u,4u)) {
        InterlockedIncrement(&g_sLiveModelInvalid[site]);return;
    }
    const UPtr definition=Read32(vehicle+0x4850u);
    if(!definition || !Readable(definition+0x14u,4u)) {
        InterlockedIncrement(&g_sLiveModelInvalid[site]);return;
    }
    const U32 id=Read32(definition+0x14u);
    if(id<512u)InterlockedIncrement(&g_sLiveModelById[site][id]);
    else InterlockedIncrement(&g_sLiveModelOutOfRange[site]);
}
extern "C" __declspec(naked) void DiagSLiveModel0Bridge() {
    __asm {
        pushfd
        pushad
        push 0
        push edi
        call DiagSRecordLiveModel
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sLiveModelTramps]
    }
}
extern "C" __declspec(naked) void DiagSLiveModel1Bridge() {
    __asm {
        pushfd
        pushad
        push 1
        push ecx
        call DiagSRecordLiveModel
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sLiveModelTramps+4]
    }
}
static bool DiagSLiveModelInstall() {
    const U8 sig0[]={0x8B,0xB0,0x80,0x07,0x00,0x00};
    const U8 sig1[]={0x8B,0xA8,0x80,0x07,0x00,0x00};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sLiveModelDetours[0],0x008DDFC4u,sig0,sizeof(sig0),
        reinterpret_cast<UPtr>(&DiagSLiveModel0Bridge),g_sLiveModelTramps[0]))return false;
    if(!DiagFInstallDetour(g_sLiveModelDetours[1],0x008DA1EAu,sig1,sizeof(sig1),
        reinterpret_cast<UPtr>(&DiagSLiveModel1Bridge),g_sLiveModelTramps[1])) {
        DiagFRestoreDetour(g_sLiveModelDetours[0],g_sLiveModelTramps[0]);return false;
    }
    return true;
}
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
// Two virtual registration methods and their common new-record constructor.
// A method entry without a matching constructor shows a duplicate/rejected ID.
static const U32 kSRegEvents=4096u;
struct DiagSRegEvent {
    volatile long sequence;
    U32 tick,site,id,caller,owner,definition;
};
static DiagSRegEvent g_sRegEvents[kSRegEvents]={};
static volatile long g_sRegNext=0,g_sRegDrops=0,g_sRegHooks=0;
static volatile long g_sRegCalls[3]={},g_sRegInvalid[3]={},g_sRegOutOfRange[3]={};
static volatile long g_sRegById[3][512]={};
static long g_sRegDrained=0;
static DiagFDetour g_sRegDetours[3]={};
static UPtr g_sRegTramps[3]={};
extern "C" __declspec(noinline) void __cdecl DiagSRecordRegistration(UPtr frame,U32 site) {
    if(site>=3u)return;
    InterlockedIncrement(&g_sRegCalls[site]);
    const UPtr caller=Read32(frame+36u);
    const UPtr owner=Read32(frame+24u);
    const UPtr definition=site==2u?Read32(frame+44u):Read32(frame+40u);
    U32 id=0xFFFFFFFFu;
    if(site==2u)id=Read32(frame+40u);
    else if(Readable(definition+0x14u,4u))id=Read32(definition+0x14u);
    if(id==0xFFFFFFFFu)InterlockedIncrement(&g_sRegInvalid[site]);
    else if(id<512u)InterlockedIncrement(&g_sRegById[site][id]);
    else InterlockedIncrement(&g_sRegOutOfRange[site]);
    const long sequence=InterlockedIncrement(&g_sRegNext);
    if(sequence<1 || static_cast<U32>(sequence)>kSRegEvents) {
        InterlockedIncrement(&g_sRegDrops);return;
    }
    DiagSRegEvent& event=g_sRegEvents[sequence-1];
    event.tick=GetTickCount();event.site=site;event.id=id;
    event.caller=caller;event.owner=owner;event.definition=definition;
    InterlockedExchange(&event.sequence,sequence);
}
extern "C" __declspec(naked) void DiagSReg0Bridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push 0
        push eax
        call DiagSRecordRegistration
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sRegTramps]
    }
}
extern "C" __declspec(naked) void DiagSReg1Bridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push 1
        push eax
        call DiagSRecordRegistration
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sRegTramps+4]
    }
}
extern "C" __declspec(naked) void DiagSReg2Bridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push 2
        push eax
        call DiagSRecordRegistration
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sRegTramps+8]
    }
}
static bool DiagSRegInstall() {
    const U8 entry0[]={0x83,0xEC,0x08,0x53,0x56};
    const U8 entry1[]={0x83,0xEC,0x08,0x53,0x56};
    const U8 create[]={0x83,0xEC,0x14,0x53,0x55};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sRegDetours[0],0x0086BCF0u,entry0,sizeof(entry0),
        reinterpret_cast<UPtr>(&DiagSReg0Bridge),g_sRegTramps[0]))return false;
    if(!DiagFInstallDetour(g_sRegDetours[1],0x0086BD40u,entry1,sizeof(entry1),
        reinterpret_cast<UPtr>(&DiagSReg1Bridge),g_sRegTramps[1]))goto fail0;
    if(!DiagFInstallDetour(g_sRegDetours[2],0x0086BAB0u,create,sizeof(create),
        reinterpret_cast<UPtr>(&DiagSReg2Bridge),g_sRegTramps[2]))goto fail1;
    return true;
fail1:
    DiagFRestoreDetour(g_sRegDetours[1],g_sRegTramps[1]);
fail0:
    DiagFRestoreDetour(g_sRegDetours[0],g_sRegTramps[0]);return false;
}
#endif
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
// 0086BE10 inserts a fresh key into the vehicle-definition registry map.
// Its 45 inline 24-byte nodes end at map+444, which is the next object's
// sentinel. Redirect only that exact fresh cursor before 008CCAA0 writes.
static const U32 kSRegMapArenas=4u,kSRegMapNodes=512u;
struct DiagSRegMapArena { UPtr map; U32 nodes[kSRegMapNodes][6]; };
static DiagSRegMapArena g_sRegMapArenas[kSRegMapArenas]={};
static volatile long g_sRegMapHook=0,g_sRegMapRedirects=0,g_sRegMapFault=0;
static volatile long g_sRegMapHigh=0,g_sRegMapLock=0;
static DiagFDetour g_sRegMapDetour={};
static UPtr g_sRegMapTramp=0;
extern "C" __declspec(noinline) void __cdecl DiagSPrepareRegMap(UPtr map) {
    if(!map || !Readable(map,12u))return;
    if(Read32(map+4u)!=0u)return; // native recycled-node path
    const UPtr cursor=Read32(map);
    const UPtr inlineEnd=map+0x444u;
    while(InterlockedExchange(&g_sRegMapLock,1u)){}
    DiagSRegMapArena* arena=nullptr;
    for(U32 i=0;i<kSRegMapArenas;++i)
        if(g_sRegMapArenas[i].map==map) {arena=&g_sRegMapArenas[i];break;}
    if(cursor==inlineEnd) {
        if(!arena)for(U32 i=0;i<kSRegMapArenas;++i)
            if(!g_sRegMapArenas[i].map) {arena=&g_sRegMapArenas[i];arena->map=map;break;}
        if(arena) {
            Write32(map,reinterpret_cast<UPtr>(&arena->nodes[0]));
            InterlockedIncrement(&g_sRegMapRedirects);
        } else InterlockedIncrement(&g_sRegMapFault);
    } else if(arena) {
        const UPtr start=reinterpret_cast<UPtr>(&arena->nodes[0]);
        const UPtr end=reinterpret_cast<UPtr>(&arena->nodes[kSRegMapNodes]);
        if(cursor>=start && cursor<=end && ((cursor-start)%24u)==0u) {
            const long used=static_cast<long>((cursor-start)/24u);
            if(used>g_sRegMapHigh)g_sRegMapHigh=used;
            if(cursor==end)InterlockedIncrement(&g_sRegMapFault);
        } else InterlockedIncrement(&g_sRegMapFault);
    }
    const bool fault=g_sRegMapFault!=0;
    InterlockedExchange(&g_sRegMapLock,0u);
    if(fault) {
        OutputDebugStringA("DSFV29s: FATAL vehicle-definition map arena exhausted or invalid; stopping to prevent overwrite.\n");
        LegacyExit(0x31u,"vehicleDefinitionMap",map);
    }
}
extern "C" __declspec(naked) void DiagSRegMapBridge() {
    __asm {
        pushfd
        pushad
        push ecx
        call DiagSPrepareRegMap
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sRegMapTramp]
    }
}
static bool DiagSRegMapInstall() {
    const U8 sig[]={0x89,0x54,0x24,0x1C,0x89,0x44,0x24,0x28};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sRegMapDetour,0x0086BE5Fu,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSRegMapBridge),g_sRegMapTramp);
}
static void DiagSRegMapRestore() {
    if(g_sRegMapHook==1)DiagFRestoreDetour(g_sRegMapDetour,g_sRegMapTramp);
    g_sRegMapHook=0;
}
#endif
// Samples for the two IDs that remained at the wait failure sites in the
// matched 150 run. These queries execute on the game's own checking thread.
struct DiagSMemberDetail {
    volatile long samples,tick,list,include,exclude,global,effective,valid;
};
static DiagSMemberDetail g_sMemberDetail[2]={};
static volatile long g_sMemberDetailSignature=0;
struct DiagSMutationEvent {
    volatile long seq;
    U32 tick,id,op,tree,caller;
};
static const U32 kSMutationCapacity=4096u;
static DiagSMutationEvent g_sMutationEvents[kSMutationCapacity]={};
static volatile long g_sMutationWrite=0,g_sMutationOverflow=0,g_sMutationHooks=0;
static DiagFDetour g_sMutationDetours[3]={};
static UPtr g_sMutationTramp[3]={};
static volatile UPtr g_sIncludeTree=0;
struct DiagSGuardDetail {
    volatile long calls,at64,firstCount,lastCount,firstTick,lastTick,first64Tick;
    volatile UPtr firstSection,lastSection,first64Section,lastCaller;
};
static DiagSGuardDetail g_sGuardDetail[2]={};
static DiagFDetour g_sGuardDetour={};
static UPtr g_sGuardTramp=0;
static volatile long g_sGuardHook=0;
#if defined(DSF_V29S_BUCKET_DIAG)
// 00809443 writes IDs into 25 stack buckets of 50 DWORDs each. Record the
// selected bucket and its count immediately before the native write.
static volatile long g_sBucketHook=0,g_sBucketSamples=0,g_sBucketOverflow=0;
static volatile long g_sBucketBadIndex=0,g_sBucketFirstOverflowBucket=-1;
static volatile long g_sBucketFirstOverflowId=-1,g_sBucketFirstOverflowTick=0;
static volatile long g_sBucketMax[25]={},g_sBucketLastId[25]={};
static DiagFDetour g_sBucketDetour={};
static UPtr g_sBucketTramp=0;
extern "C" __declspec(noinline) void __cdecl DiagSRecordBucket(UPtr frame) {
    const UPtr base=Read32(frame+28u); // original EAX: bucket data
    const U32 count=Read32(frame+24u); // original ECX: used entries
    const UPtr originalEsp=Read32(frame+12u)+4u;
    const UPtr first=originalEsp+0x288u;
    if(base<first || base>=first+25u*0xCCu || ((base-first)%0xCCu)!=0u) {
        InterlockedIncrement(&g_sBucketBadIndex);return;
    }
    const U32 bucket=static_cast<U32>((base-first)/0xCCu);
    const UPtr node=Read32(frame+4u); // original ESI: list node
    const U32 id=Readable(node+8u,4u)?Read32(node+8u):0xFFFFFFFFu;
    InterlockedIncrement(&g_sBucketSamples);
    g_sBucketLastId[bucket]=static_cast<long>(id);
    long previous=g_sBucketMax[bucket];
    while(count+1u>static_cast<U32>(previous)) {
        const long seen=InterlockedCompareExchange(&g_sBucketMax[bucket],
            static_cast<long>(count+1u),previous);
        if(seen==previous)break;
        previous=seen;
    }
    if(count>=50u) {
        InterlockedIncrement(&g_sBucketOverflow);
        if(InterlockedCompareExchange(&g_sBucketFirstOverflowBucket,
            static_cast<long>(bucket),-1)==-1) {
            g_sBucketFirstOverflowId=static_cast<long>(id);
            g_sBucketFirstOverflowTick=static_cast<long>(GetTickCount()-g_diagStartTick);
        }
    }
}
extern "C" __declspec(naked) void DiagSBucketBridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push eax
        call DiagSRecordBucket
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sBucketTramp]
    }
}
static bool DiagSBucketInstall() {
    const U8 sig[]={0x8D,0x0C,0x88,0x3B,0xCD};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sBucketDetour,0x00809443u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSBucketBridge),g_sBucketTramp);
}
#endif
#if defined(DSF_V29S_CLONE_EXPANDED)
// 005A6920 copies the spooler's +330 list into a stack-local 50-node list.
// The caller at 008093D1 later walks that copy at 00809420. Redirect its
// fresh cursor at the exact +264 sentinel boundary, preserving native links.
static const U32 kSCloneNodes=256u,kSCloneArenas=32u;
struct DiagSCloneArena { UPtr header; U32 nodes[kSCloneNodes][3]; };
static DiagSCloneArena g_sCloneArenas[kSCloneArenas]={};
static volatile long g_sCloneLock=0,g_sCloneUsed=0,g_sCloneHook=0;
static volatile long g_sCloneCalls=0,g_sCloneRedirects=0,g_sCloneReuses=0,g_sCloneFault=0;
static DiagFDetour g_sCloneDetour={};
static UPtr g_sCloneTramp=0;
extern "C" __declspec(noinline) void __cdecl DiagSPrepareClone(UPtr header) {
    InterlockedIncrement(&g_sCloneCalls);
    if(!header || !Readable(header,12u))return;
    const U32 freeCount=Read32(header+4u);
    if(freeCount)return;
    const UPtr cursor=Read32(header);
    const UPtr start=header+0xCu,limit=header+0x264u;
    if(cursor>=start && cursor<limit && ((cursor-start)%12u)==0u)return;
    while(InterlockedExchange(&g_sCloneLock,1)){}
    bool okay=false;
    for(U32 i=0;i<static_cast<U32>(g_sCloneUsed);++i) {
        const DiagSCloneArena& a=g_sCloneArenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSCloneNodes]);
        if(a.header==header && cursor>=lo && cursor<hi &&
           ((cursor-lo)%12u)==0u) {okay=true;break;}
    }
    if(!okay && cursor==limit) {
        // 005A6920 reinitializes this stack-local header and sentinel on
        // every call. A repeated header address is a new, sequential clone;
        // its previous nodes have been consumed by the caller.
        for(U32 i=0;i<static_cast<U32>(g_sCloneUsed);++i) {
            DiagSCloneArena& a=g_sCloneArenas[i];
            if(a.header==header) {
                Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
                InterlockedIncrement(&g_sCloneRedirects);
                InterlockedIncrement(&g_sCloneReuses);
                okay=true;break;
            }
        }
        if(!okay && static_cast<U32>(g_sCloneUsed)<kSCloneArenas) {
            DiagSCloneArena& a=g_sCloneArenas[g_sCloneUsed++];
            a.header=header;
            Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
            InterlockedIncrement(&g_sCloneRedirects);
            okay=true;
        }
    }
    InterlockedExchange(&g_sCloneLock,0);
    if(!okay) {
        InterlockedIncrement(&g_sCloneFault);
        OutputDebugStringA("DSFV29s: spooler clone arena unavailable before insert.\n");
        LegacyExit(0x2Eu,"spoolClone",header,cursor);
    }
}
extern "C" __declspec(naked) void DiagSCloneBridge() {
    __asm {
        pushfd
        pushad
        push dword ptr [esp+1Ch] // original EAX: destination list header
        call DiagSPrepareClone
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sCloneTramp]
    }
}
static bool DiagSCloneInstall() {
    const U8 sig[]={0x83,0x78,0x04,0x00,0x8B,0x08};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sCloneDetour,0x005A6960u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSCloneBridge),g_sCloneTramp);
}
#endif
#if defined(DSF_V29S_SPOOL0_EXPANDED)
// The first spooler pool is [owner+D8, owner+330): 50 x 12-byte nodes.
// Reconcile also builds a 50-node temporary list on its stack at
// [ESP+20, ESP+278). Both cursors must leave their inline regions before
// the 150-ID pass; the sentinel addresses and native links stay in place.
static const U32 kSSpool0Nodes=256u,kSSpool0Arenas=8u;
struct DiagSSpool0Arena { UPtr owner; U32 nodes[kSSpool0Nodes][3]; };
static DiagSSpool0Arena g_sSpool0Arenas[kSSpool0Arenas]={};
static volatile long g_sSpool0Lock=0,g_sSpool0Used=0;
static volatile long g_sSpool0Redirects=0,g_sSpool0Fault=0,g_sSpool0High=0;
static const U32 kSScratchNodes=256u,kSScratchArenas=8u;
struct DiagSScratchArena { UPtr sentinel; U32 nodes[kSScratchNodes][3]; };
static DiagSScratchArena g_sScratchArenas[kSScratchArenas]={};
static volatile long g_sScratchLock=0,g_sScratchUsed=0,g_sScratchHook=0;
static volatile long g_sScratchCalls=0,g_sScratchRedirects=0,g_sScratchFault=0;
static DiagFDetour g_sScratchDetours[2]={};
static UPtr g_sScratchTramps[2]={};
#if defined(DSF_V29S_ARENA_REUSE)
#include "V29sArenaReuseCore.inc"
#endif

extern "C" __declspec(noinline) void __cdecl DiagSPrepareSpool0(UPtr owner) {
#if defined(DSF_V29S_ARENA_REUSE)
    const char* why="spoolIntegrity";while(InterlockedExchange(&g_reuseLock,1)){}
    const bool okay=ReusePrepareSpool(owner,why);
    const bool readable=owner && Readable(owner+0xCCu,12u);
    const U32 cursor=readable?Read32(owner+0xCCu):0u,active=readable?Read32(owner+0xD4u):0u,free=readable?Read32(owner+0xD0u):0u;
    InterlockedExchange(&g_reuseLock,0);
    if(!okay){InterlockedIncrement(&g_sSpool0Fault);LegacyExit(0x2Cu,why,owner,cursor,active,free);}
#else
    if(g_sScratchHook!=2 || !owner || !Readable(owner+0xCCu,12u))return;
    while(InterlockedExchange(&g_sSpool0Lock,1)){}
    const UPtr header=owner+0xCCu,cursor=Read32(header);
    const U32 freeCount=Read32(header+4u),active=Read32(header+8u);
    if(active>static_cast<U32>(g_sSpool0High))g_sSpool0High=static_cast<long>(active);
    bool okay=freeCount!=0u;
    for(U32 i=0;i<static_cast<U32>(g_sSpool0Used);++i) {
        const DiagSSpool0Arena& a=g_sSpool0Arenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSSpool0Nodes]);
        if(a.owner==owner && cursor>=lo && cursor<hi && ((cursor-lo)%12u)==0u)
            okay=true;
    }
    if(!okay && freeCount==0u && active<=50u &&
       cursor>=owner+0xD8u && cursor<=owner+0x330u &&
       ((cursor-(owner+0xD8u))%12u)==0u &&
       static_cast<U32>(g_sSpool0Used)<kSSpool0Arenas) {
        DiagSSpool0Arena& a=g_sSpool0Arenas[g_sSpool0Used++];
        a.owner=owner;
        Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
        InterlockedIncrement(&g_sSpool0Redirects);
        okay=true;
    }
    InterlockedExchange(&g_sSpool0Lock,0);
    if(!okay) {
        InterlockedIncrement(&g_sSpool0Fault);
        OutputDebugStringA("DSFV29s: first spooler pool unavailable before reconcile.\n");
        LegacyExit(0x2Cu,"spoolStorage",owner,cursor,active,freeCount);
    }
#endif
}

extern "C" __declspec(noinline) void __cdecl DiagSPrepareScratch(U32 site,UPtr frame) {
#if defined(DSF_V29S_ARENA_REUSE)
    const char* why="scratchIntegrity";while(InterlockedExchange(&g_reuseLock,1)){}
    const bool okay=ReusePrepareScratch(site,frame,why);
    const bool saved=Readable(frame,36u);const UPtr esp=saved?Read32(frame+12u)+4u:0u;
    const bool readable=esp && Readable(esp+0x14u,12u);
    const U32 active=readable?Read32(esp+0x1Cu):0u,free=readable?Read32(esp+0x18u):0u,cursor=saved?Read32(frame+28u):0u;
    InterlockedExchange(&g_reuseLock,0);
    if(!okay){InterlockedIncrement(&g_sScratchFault);LegacyExit(0x2Du,why,esp,cursor,active,free);}
#else
    InterlockedIncrement(&g_sScratchCalls);
    const UPtr originalEsp=Read32(frame+12u)+4u;
    const UPtr cursor=Read32(frame+28u); // EAX at 005A75DD
    const UPtr sentinel=originalEsp+0x278u;
    const U32 freeCount=Read32(originalEsp+0x18u);
    if(freeCount)return; // native recycled-node path does not use EAX as fresh cursor
    if(cursor>=originalEsp+0x20u && cursor<sentinel &&
       ((cursor-(originalEsp+0x20u))%12u)==0u)return;
    while(InterlockedExchange(&g_sScratchLock,1)){}
    bool okay=false;
    for(U32 i=0;i<static_cast<U32>(g_sScratchUsed);++i) {
        const DiagSScratchArena& a=g_sScratchArenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSScratchNodes]);
        if(a.sentinel==sentinel && cursor>=lo && cursor<hi &&
           ((cursor-lo)%12u)==0u) {okay=true;break;}
    }
    if(!okay && cursor==sentinel &&
       static_cast<U32>(g_sScratchUsed)<kSScratchArenas) {
        DiagSScratchArena& a=g_sScratchArenas[g_sScratchUsed++];
        a.sentinel=sentinel;
        const UPtr replacement=reinterpret_cast<UPtr>(&a.nodes[0]);
        Write32(frame+28u,replacement); // EAX: fresh cursor
        if(site==0u)Write32(frame+20u,replacement); // EDX at 005A75C4
        else Write32(frame+24u,replacement); // ECX at 005A764C
        Write32(originalEsp+0x14u,replacement); // cursor reloaded at 005A75E7
        InterlockedIncrement(&g_sScratchRedirects);
        okay=true;
    }
    InterlockedExchange(&g_sScratchLock,0);
    if(!okay) {
        InterlockedIncrement(&g_sScratchFault);
        OutputDebugStringA("DSFV29s: reconcile scratch arena unavailable before insert.\n");
        LegacyExit(0x2Du,"scratchStorage",sentinel,cursor,Read32(originalEsp+0x1Cu),freeCount);
    }
#endif
}
#if defined(DSF_V29S_ARENA_REUSE)
#include "V29sArenaReuseBridges.inc"
#else
extern "C" __declspec(naked) void DiagSScratchBridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push eax
        push 0
        call DiagSPrepareScratch
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sScratchTramps]
    }
}
extern "C" __declspec(naked) void DiagSScratchSecondBridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push eax
        push 1
        call DiagSPrepareScratch
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sScratchTramps+4]
    }
}
#endif
static bool DiagSScratchInstall() {
    const U8 first[]={0x8D,0x48,0x08,0x85,0xC9};
    const U8 second[]={0x8D,0x50,0x08,0x85,0xD2};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sScratchDetours[0],0x005A75DDu,first,sizeof(first),
        reinterpret_cast<UPtr>(&DiagSScratchBridge),g_sScratchTramps[0]))return false;
    if(!DiagFInstallDetour(g_sScratchDetours[1],0x005A7665u,second,sizeof(second),
        reinterpret_cast<UPtr>(&DiagSScratchSecondBridge),g_sScratchTramps[1])) {
        DiagFRestoreDetour(g_sScratchDetours[0],g_sScratchTramps[0]);return false;
    }
    return true;
}
#endif
#if defined(DSF_V29S_POOL_DIAG)
// A single-run diagnostic for the four adjacent inline pools. Hook callbacks
// only copy registers and words to a fixed ring; the monitor formats the log.
static volatile UPtr g_sPoolReconcileOwner=0,g_sPoolWaitOwner=0;
static const U32 kSPoolEventCapacity=8192u;
struct DiagSPoolEvent {
    volatile long seq;
    U32 tick,site,id;
    UPtr edi,esi,ebp,esp,ebx,edx,ecx,eax,flags;
    UPtr caller,stack0,header,cursor,freeCount,active;
    UPtr rootOwner,reconcileOwner,waitOwner;
};
static DiagSPoolEvent g_sPoolEvents[kSPoolEventCapacity]={};
static volatile long g_sPoolWritten=0,g_sPoolOverflow=0,g_sPoolHooks=0;
static volatile long g_sPoolLastWaitTick[2]={};
static DiagFDetour g_sPoolDetours[2]={};
static UPtr g_sPoolTramps[2]={};
static UPtr DiagSPoolRootOwner() {
    const UPtr location=Address(kRootPtr);
    if(!Readable(location,4u))return 0;
    const UPtr root=Read32(location);
    return Readable(root+0x214u,4u)?Read32(root+0x214u):0u;
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordPool(U32 site,UPtr frame) {
    // 0083B170 is shared widely; retain only the two traffic-spooler calls.
    if(site==1u) {
        const UPtr originalEsp=Read32(frame+12u)+4u; // pushad saved ESP after pushfd
        if(!Readable(originalEsp,4u))return;
        const UPtr caller=Read32(originalEsp);
        if(caller!=Address(0x005A7E3Du) && caller!=Address(0x005A7E6Du))return;
    }
    U32 waitId=0xFFFFFFFFu;
    if(site==3u || site==4u) {
        const UPtr node=Read32(frame);
        if(!Readable(node+8u,2u))return;
        waitId=*reinterpret_cast<volatile U16*>(node+8u);
        if(waitId!=126u && waitId!=231u)return;
        const U32 tick=GetTickCount()-g_diagStartTick;
        volatile long& last=g_sPoolLastWaitTick[site-3u];
        if(tick-static_cast<U32>(last)<1000u)return;
        InterlockedExchange(&last,static_cast<long>(tick));
    }
    const long seq=InterlockedIncrement(&g_sPoolWritten);
    if(seq<=0 || static_cast<U32>(seq)>kSPoolEventCapacity) {
        InterlockedIncrement(&g_sPoolOverflow);return;
    }
    DiagSPoolEvent& e=g_sPoolEvents[seq-1];
    e.tick=GetTickCount()-g_diagStartTick;e.site=site;e.id=waitId;
    e.edi=Read32(frame);e.esi=Read32(frame+4u);e.ebp=Read32(frame+8u);
    e.esp=Read32(frame+12u)+4u;e.ebx=Read32(frame+16u);e.edx=Read32(frame+20u);
    e.ecx=Read32(frame+24u);e.eax=Read32(frame+28u);e.flags=Read32(frame+32u);
    e.header=site<2u?e.ecx:0u;
    if(Readable(e.esp,4u))e.stack0=Read32(e.esp);
    const UPtr callerAddress=e.esp+(site==0u?20u:0u);
    if(site>=3u)e.caller=Address(site==3u?0x005A65DFu:0x005A665Au);
    else if(Readable(callerAddress,4u))e.caller=Read32(callerAddress);
    if(Readable(e.header,12u)) {
        e.cursor=Read32(e.header);e.freeCount=Read32(e.header+4u);
        e.active=Read32(e.header+8u);
    }
    e.rootOwner=DiagSPoolRootOwner();
    e.reconcileOwner=g_sPoolReconcileOwner;e.waitOwner=g_sPoolWaitOwner;
    InterlockedExchange(&e.seq,seq);
}
extern "C" __declspec(naked) void DiagSPool12Bridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push eax
        push 0
        call DiagSRecordPool
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sPoolTramps]
    }
}
extern "C" __declspec(naked) void DiagSPool20Bridge() {
    __asm {
        pushfd
        pushad
        mov eax,esp
        push eax
        push 1
        call DiagSRecordPool
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sPoolTramps+4]
    }
}
static bool DiagSPoolInstall() {
    const U8 sig[]={0x83,0x79,0x04,0x00,0x8B,0x01};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sPoolDetours[0],0x005A7DD1u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSPool12Bridge),g_sPoolTramps[0]))return false;
    if(!DiagFInstallDetour(g_sPoolDetours[1],0x0083B170u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSPool20Bridge),g_sPoolTramps[1])) {
        DiagFRestoreDetour(g_sPoolDetours[0],g_sPoolTramps[0]);return false;
    }
    return true;
}
#endif
#if defined(DSF_V29S_LISTA_EXPANDED)
// The list at +330 owns 50 inline 12-byte nodes at +344..+59C.
// 005A7DD1 inserts without checking its cursor against the next list.
static const U32 kSListAInline=50u,kSListACapacity=512u;
static const U32 kSListAExtra=kSListACapacity-kSListAInline;
static const U32 kSListAArenaCount=4u;
struct DiagSListAArena { UPtr owner; U32 nodes[kSListAExtra][3]; };
static DiagSListAArena g_sListAArenas[kSListAArenaCount]={};
static volatile long g_sListALock=0,g_sListAArenasUsed=0,g_sListAHook=0;
static volatile long g_sListARedirects=0,g_sListAFault=0,g_sListAHigh=0;
static DiagFDetour g_sListADetour={};
static UPtr g_sListATramp=0;
extern "C" __declspec(noinline) void __cdecl DiagSPrepareListA(UPtr header) {
    const UPtr rootLocation=Address(kRootPtr);
    if(!Readable(rootLocation,4u))return;
    const UPtr root=Read32(rootLocation);
    if(!Readable(root+0x214u,4u))return;
    const UPtr owner=Read32(root+0x214u);
    if(!owner || header!=owner+0x338u)return;
    while(InterlockedExchange(&g_sListALock,1)){}
    bool okay=false;
    if(Readable(header,12u)) {
        const UPtr cursor=Read32(header);
        const U32 freeCount=Read32(header+4u),active=Read32(header+8u);
        if(active>static_cast<U32>(g_sListAHigh))g_sListAHigh=static_cast<long>(active);
        if(active<kSListACapacity) {
            if(freeCount)okay=true;
            else if(cursor>=owner+0x344u && cursor<owner+0x59Cu &&
                    ((cursor-(owner+0x344u))%12u)==0u)okay=true;
            else {
                for(U32 i=0;i<static_cast<U32>(g_sListAArenasUsed);++i) {
                    DiagSListAArena& a=g_sListAArenas[i];
                    const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
                    const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSListAExtra]);
                    if(a.owner==owner && cursor>=lo && cursor<hi &&
                       ((cursor-lo)%12u)==0u) {okay=true;break;}
                }
                if(!okay && cursor==owner+0x59Cu &&
                   static_cast<U32>(g_sListAArenasUsed)<kSListAArenaCount) {
                    DiagSListAArena& a=g_sListAArenas[g_sListAArenasUsed++];
                    a.owner=owner;
                    Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
                    InterlockedIncrement(&g_sListARedirects);
                    okay=true;
                }
            }
        }
    }
    InterlockedExchange(&g_sListALock,0);
    if(!okay) {
        InterlockedIncrement(&g_sListAFault);
        OutputDebugStringA("DSFV29s: list A external arena unavailable; stopping before native overwrite.\n");
        LegacyExit(0x2Bu,"trafficListA",header);
    }
}
extern "C" __declspec(naked) void DiagSListABridge() {
    __asm {
        pushfd
        pushad
        push dword ptr [esp+18h] // original ECX: allocator header
        call DiagSPrepareListA
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sListATramp]
    }
}
static bool DiagSListAInstall() {
    const U8 sig[]={0x83,0x79,0x04,0x00,0x8B,0x01};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sListADetour,0x005A7DD1u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSListABridge),g_sListATramp);
}
#endif
#if defined(DSF_V29S_LISTB_EXPANDED)
// The pool shared by the lists at +59C and +998 has 50 inline 20-byte
// nodes (+5B0..+998). 0083B170 has no capacity check before its fresh
// cursor advances. Redirect that cursor at the exact inline boundary.
static const U32 kSListBInline=50u,kSListBCapacity=512u;
static const U32 kSListBExtra=kSListBCapacity-kSListBInline;
static const U32 kSListBArenaCount=4u;
struct DiagSListBArena { UPtr owner; U32 nodes[kSListBExtra][5]; };
static DiagSListBArena g_sListBArenas[kSListBArenaCount]={};
static volatile long g_sListBLock=0,g_sListBArenasUsed=0,g_sListBHook=0;
static volatile long g_sListBRedirects=0,g_sListBFault=0,g_sListBHigh=0;
static DiagFDetour g_sListBDetour={};
static UPtr g_sListBTramp=0;

extern "C" __declspec(noinline) void __cdecl DiagSPrepareListB(UPtr header) {
    const UPtr rootLocation=Address(kRootPtr);
    if(!Readable(rootLocation,4u))return;
    const UPtr root=Read32(rootLocation);
    if(!Readable(root+0x214u,4u))return;
    const UPtr owner=Read32(root+0x214u);
    if(!owner || header!=owner+0x5A4u)return;
    while(InterlockedExchange(&g_sListBLock,1)){}
    bool okay=false;
    if(Readable(header,12u)) {
        const UPtr cursor=Read32(header);
        const U32 freeCount=Read32(header+4u),active=Read32(header+8u);
        if(active>static_cast<U32>(g_sListBHigh))g_sListBHigh=static_cast<long>(active);
        if(active<kSListBCapacity) {
            if(freeCount)okay=true;
            else if(cursor>=owner+0x5B0u && cursor<owner+0x998u &&
                    ((cursor-(owner+0x5B0u))%20u)==0u)okay=true;
            else {
                for(U32 i=0;i<static_cast<U32>(g_sListBArenasUsed);++i) {
                    DiagSListBArena& a=g_sListBArenas[i];
                    const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
                    const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSListBExtra]);
                    if(a.owner==owner && cursor>=lo && cursor<hi &&
                       ((cursor-lo)%20u)==0u) {okay=true;break;}
                }
                if(!okay && cursor==owner+0x998u &&
                   static_cast<U32>(g_sListBArenasUsed)<kSListBArenaCount) {
                    DiagSListBArena& a=g_sListBArenas[g_sListBArenasUsed++];
                    a.owner=owner;
                    Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
                    InterlockedIncrement(&g_sListBRedirects);
                    okay=true;
                }
            }
        }
    }
    InterlockedExchange(&g_sListBLock,0);
    if(!okay) {
        InterlockedIncrement(&g_sListBFault);
        OutputDebugStringA("DSFV29s: list B external arena unavailable; stopping before native overwrite.\n");
        LegacyExit(0x2Au,"trafficListB",header);
    }
}
extern "C" __declspec(naked) void DiagSListBBridge() {
    __asm {
        pushfd
        pushad
        push dword ptr [esp+18h] // original ECX: allocator header
        call DiagSPrepareListB
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sListBTramp]
    }
}
static bool DiagSListBInstall() {
    const U8 sig[]={0x83,0x79,0x04,0x00,0x8B,0x01};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sListBDetour,0x0083B170u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSListBBridge),g_sListBTramp);
}
#endif
#if defined(DSF_V29S_INCLUDE_EXPANDED)
// 005A1C2E initializes a 64 x 12-byte inline arena at section+6698.
// 0059E120 consumes [section+668C] as a fresh cursor and its remove path
// chains freed nodes through that same field. Keep all existing native links;
// only redirect the fresh cursor once it reaches the inline boundary.
static const U32 kSIncludeInline=64u;
static const U32 kSIncludeCapacity=256u;
static const U32 kSIncludeExtra=kSIncludeCapacity-kSIncludeInline;
static const U32 kSIncludeArenaCount=16u;
struct DiagSIncludeArena { UPtr section; Node12 nodes[kSIncludeExtra]; };
static DiagSIncludeArena g_sIncludeArenas[kSIncludeArenaCount]={};
static volatile long g_sIncludeLock=0,g_sIncludeArenasUsed=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_IncludeRedirects=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_IncludeFault=0;
extern "C" __declspec(dllexport) volatile long DSFV29s_IncludeHigh=0;
static UPtr g_sGuardContinue=0;

static void DiagSIncludeLock() { while(InterlockedExchange(&g_sIncludeLock,1)){} }
static void DiagSIncludeUnlock() { InterlockedExchange(&g_sIncludeLock,0); }
static bool DiagSIncludeCursorInArena(UPtr section,UPtr cursor) {
    const U32 used=static_cast<U32>(g_sIncludeArenasUsed);
    for(U32 i=0;i<used && i<kSIncludeArenaCount;++i) {
        const DiagSIncludeArena& a=g_sIncludeArenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSIncludeExtra]);
        if(a.section==section && cursor>=lo && cursor<hi &&
           ((cursor-lo)%sizeof(Node12))==0u)return true;
    }
    return false;
}
extern "C" __declspec(noinline) U8 __cdecl DiagSPrepareInclude(UPtr section) {
    if(!section || !Readable(section+0x6694u,4u)) {
        InterlockedIncrement(&DSFV29s_IncludeFault);return 0;
    }
    DiagSIncludeLock();
    const U32 active=Read32(section+0x6694u);
    const U32 freeCount=Read32(section+0x6690u);
    const UPtr cursor=Read32(section+0x668Cu);
    if(active>static_cast<U32>(DSFV29s_IncludeHigh))DSFV29s_IncludeHigh=static_cast<long>(active);
    bool ready=false;
    if(active<kSIncludeCapacity) {
        if(freeCount)ready=true; // native allocator takes its recycled-node path
        else if(cursor>=section+0x6698u && cursor<section+0x6998u &&
                ((cursor-(section+0x6698u))%sizeof(Node12))==0u)ready=true;
        else if(DiagSIncludeCursorInArena(section,cursor))ready=true;
        else if(cursor==section+0x6998u && active>=kSIncludeInline &&
                static_cast<U32>(g_sIncludeArenasUsed)<kSIncludeArenaCount) {
            DiagSIncludeArena& a=g_sIncludeArenas[g_sIncludeArenasUsed++];
            a.section=section;
            Write32(section+0x668Cu,reinterpret_cast<UPtr>(&a.nodes[0]));
            InterlockedIncrement(&DSFV29s_IncludeRedirects);
            ready=true;
        }
    }
    DiagSIncludeUnlock();
    if(!ready)InterlockedIncrement(&DSFV29s_IncludeFault);
    return ready?1u:0u;
}
extern "C" __declspec(noinline) void __cdecl DiagSPrepareIncludeMutation(UPtr tree) {
    if(!tree || g_sGuardHook!=1)return;
    const UPtr pkgTree=g_pkg0?g_pkg0+0xAF68u+0x668Cu:0u;
    if(tree!=pkgTree && tree!=g_sIncludeTree)return;
    if(!DiagSPrepareInclude(tree-0x668Cu)) {
        OutputDebugStringA("DSFV29s: include-tree external arena unavailable at insert; stopping before overwrite.\n");
        LegacyExit(0x29u,"includeTree",tree);
    }
}
#endif

extern "C" __declspec(noinline) void __cdecl DiagSRecordGuard(UPtr section,U32 id,UPtr caller) {
    if(id!=126u && id!=231u)return;
    g_sIncludeTree=section+0x668Cu;
    const U32 slot=id==126u?0u:1u;
    DiagSGuardDetail& d=g_sGuardDetail[slot];
    const long count=static_cast<long>(Read32(section+0x6694u));
    const long tick=static_cast<long>(GetTickCount()-g_diagStartTick);
    const long calls=InterlockedIncrement(&d.calls);
    if(calls==1) {
        d.firstCount=count;d.firstTick=tick;d.firstSection=section;
    }
    d.lastCount=count;d.lastTick=tick;d.lastSection=section;d.lastCaller=caller;
    if(count==64) {
        if(InterlockedIncrement(&d.at64)==1) {
            d.first64Tick=tick;d.first64Section=section;
        }
    }
}

extern "C" __declspec(noinline) void __cdecl DiagSRecordMutation(
    UPtr tree,UPtr idPointer,UPtr caller,U32 op) {
    if(!idPointer)return; // the original helper also dereferences this argument
    const U32 id=*reinterpret_cast<volatile U16*>(idPointer);
    if(id!=126u && id!=231u)return;
    const U32 seq=static_cast<U32>(InterlockedIncrement(&g_sMutationWrite));
    if(seq==0u || seq>kSMutationCapacity) {
        InterlockedIncrement(&g_sMutationOverflow);return;
    }
    DiagSMutationEvent& event=g_sMutationEvents[seq-1u];
    event.tick=GetTickCount()-g_diagStartTick;
    event.id=id;event.op=op;event.tree=tree;event.caller=caller;
    event.seq=static_cast<long>(seq); // publish after all fields
}

static bool DiagSMemberDetailCheck() {
    const U8 includeSig[]={0x83,0xEC,0x08,0x8D,0x44};
    const U8 excludeSig[]={0x8B,0x81,0xB4,0x6C,0x00};
    const U8 globalSig[]={0x83,0xEC,0x08,0x8D,0x44};
    const UPtr sites[]={0x005A10C0u,0x005A0F60u,0x0059E4E0u};
    const U8* sigs[]={includeSig,excludeSig,globalSig};
    for(U32 i=0;i<3u;++i) {
        if(!Readable(Address(sites[i]),5u))return false;
        for(U32 j=0;j<5u;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i])+j)!=sigs[i][j])return false;
    }
    return true;
}

static void DiagSRecordMemberDetail(UPtr owner,U32 id,U32 list,long tick) {
    const U32 slot=id==126u?0u:1u;
    DiagSMemberDetail& d=g_sMemberDetail[slot];
    if(g_sMemberDetailSignature!=1 ||
       (d.tick && static_cast<U32>(tick-d.tick)<1000u))return;
    d.tick=tick;d.list=static_cast<long>(list);d.valid=0;
    if(!owner || !Readable(owner+0xDA8u,4u))return;
    const UPtr manager=Read32(owner+0xDA8u);
    if(!manager || !Readable(manager+0x6FD94u,4u))return;
    const UPtr section=manager+0xAF68u; // kind 0 at 0059CFC0
    g_sIncludeTree=section+0x668Cu;
    const UPtr global=Read32(manager+0x6FD94u);
    if(!global || !Readable(section+0x699Cu,4u) ||
       !Readable(section+0x6CB4u,4u) || !Readable(global+0x241Cu,4u))return;
    typedef U8 (__thiscall* LookupOne)(void*,U32);
    typedef U8 (__thiscall* LookupGlobal)(void*,U32,U32);
    const U32 include=reinterpret_cast<LookupOne>(Address(0x005A10C0u))(
        reinterpret_cast<void*>(section),id)?1u:0u;
    const U32 exclude=reinterpret_cast<LookupOne>(Address(0x005A0F60u))(
        reinterpret_cast<void*>(section),id)?1u:0u;
    const U32 globallyPresent=reinterpret_cast<LookupGlobal>(Address(0x0059E4E0u))(
        reinterpret_cast<void*>(global),0u,id)?1u:0u;
    d.include=static_cast<long>(include);
    d.exclude=static_cast<long>(exclude);
    d.global=static_cast<long>(globallyPresent);
    d.effective=static_cast<long>(include && !exclude && !globallyPresent);
    d.valid=1;
    InterlockedIncrement(&d.samples);
}

extern "C" __declspec(noinline) void __cdecl DiagSRecordReconcile(UPtr owner) {
#if defined(DSF_V29S_POOL_DIAG)
    if(owner)g_sPoolReconcileOwner=owner;
#endif
    InterlockedIncrement(&DSFV29s_Reconcile);
    if(owner)DSFV29s_LastRoster=static_cast<long>(*reinterpret_cast<volatile U32*>(owner+0xC8u));
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordGate(UPtr owner,U32 caller,U32 kind,U32 id) {
    (void)caller;
    InterlockedIncrement(&DSFV29s_GateCalls);
    const U32 gate=owner?*reinterpret_cast<volatile U8*>(owner+0x74DB9u):0xFFu;
    DSFV29s_LastGateByte=static_cast<long>(gate);
    DSFV29s_LastKind=static_cast<long>(kind);
    DSFV29s_LastId=static_cast<long>(id);
    const U32 k=kind<5u?kind:5u;
    InterlockedIncrement(&g_sGateByKind[k]);
    if(id<512u)InterlockedIncrement(&g_sGateById[id]);
    else InterlockedIncrement(&DSFV29s_IdOverflow);
    if(gate) {
        InterlockedIncrement(&DSFV29s_GateBlocked);
        InterlockedIncrement(&g_sBlockedByKind[k]);
        if(id<512u)InterlockedIncrement(&g_sBlockedById[id]);
    }
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordDispatch(U32 id) {
    InterlockedIncrement(&DSFV29s_Dispatched);
    if(id<512u)InterlockedIncrement(&g_sDispatchById[id]);
    else InterlockedIncrement(&DSFV29s_IdOverflow);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordMembershipPass(U32 id) {
    InterlockedIncrement(&DSFV29s_MembershipPass);
    if(id<512u)InterlockedIncrement(&g_sPassById[id]);
    else InterlockedIncrement(&DSFV29s_IdOverflow);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordTransfer(UPtr control) {
    InterlockedIncrement(&DSFV29s_TransferCalls);
    if(!control)return;
    const U32 id=*reinterpret_cast<volatile U16*>(control+0x28u);
    const U32 state=*reinterpret_cast<volatile U32*>(control+0x2Cu);
    const UPtr handler=*reinterpret_cast<volatile U32*>(control+0x14u);
    DSFV29s_LastTransferId=static_cast<long>(id);
    DSFV29s_LastTransferState=static_cast<long>(state);
    if(handler)InterlockedIncrement(&DSFV29s_TransferWithHandler);
    if(id<512u)InterlockedIncrement(&g_sTransferById[id]);
    else InterlockedIncrement(&DSFV29s_IdOverflow);
    if(state<16u)InterlockedIncrement(&g_sTransferState[state]);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordWaitFail(UPtr owner,UPtr node,U32 list) {
#if defined(DSF_V29S_POOL_DIAG)
    if(owner)g_sPoolWaitOwner=owner;
#endif
    const long id=node?static_cast<long>(*reinterpret_cast<volatile U16*>(node+8u)):-1;
    const long tick=static_cast<long>(GetTickCount()-g_diagStartTick);
    if(id==126 || id==231)DiagSRecordMemberDetail(owner,static_cast<U32>(id),list,tick);
    if(list==0u) {
        InterlockedIncrement(&DSFV29s_WaitFailA);
        InterlockedCompareExchange(&DSFV29s_FirstWaitFailA,id,-1);
        DSFV29s_LastWaitFailA=id;
        if(id>=0 && id<512) {
            InterlockedIncrement(&g_sWaitFailAById[id]);
            g_sWaitLastAById[id]=tick;
        } else InterlockedIncrement(&g_sWaitIdOverflow);
    } else {
        InterlockedIncrement(&DSFV29s_WaitFailB);
        InterlockedCompareExchange(&DSFV29s_FirstWaitFailB,id,-1);
        DSFV29s_LastWaitFailB=id;
        if(id>=0 && id<512) {
            InterlockedIncrement(&g_sWaitFailBById[id]);
            g_sWaitLastBById[id]=tick;
        } else InterlockedIncrement(&g_sWaitIdOverflow);
    }
}

// pushfd+pushad: original ECX=[esp+18h], EDI=[esp], caller=[esp+24h],
// original first two stack args=[esp+28h],[esp+2Ch].
#if !defined(DSF_V29S_ARENA_REUSE)
extern "C" __declspec(naked) void DiagSReconcileBridge(){
    __asm {
        pushfd
        pushad
#if defined(DSF_V29S_SPOOL0_EXPANDED)
        push dword ptr [esp+18h]
        call DiagSPrepareSpool0
        add esp,4
#endif
#if defined(DSF_V29S_POOL_DIAG)
        mov eax,esp
        push eax
        push 2
        call DiagSRecordPool
        add esp,8
#endif
#if !defined(DSF_V29S_PERF_CONTROL)
        push dword ptr [esp+18h]
        call DiagSRecordReconcile
        add esp,4
#endif
        popad
        popfd
        jmp dword ptr [g_sTramp]
    }
}
#endif
// g_sTramp is a flat array. MSVC inline assembly needs a literal +offset.
extern "C" __declspec(naked) void DiagSGateBridge(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        mov edx,dword ptr [esp+2Ch] // second stack arg: model ID
        mov ebx,dword ptr [esp+28h] // first stack arg: kind
        mov eax,dword ptr [esp+24h] // caller return address
        mov ecx,dword ptr [esp+18h] // manager (ECX)
        push edx
        push ebx
        push eax
        push ecx
        call DiagSRecordGate
        add esp,10h
#endif
        popad
        popfd
        jmp dword ptr [g_sTramp+4]
    }
}
extern "C" __declspec(naked) void DiagSDispatchBridge(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        push dword ptr [esp+2Ch] // model ID (second stack arg)
        call DiagSRecordDispatch
        add esp,4
#endif
        popad
        popfd
        jmp dword ptr [g_sTramp+8]
    }
}
extern "C" __declspec(naked) void DiagSPassBridge(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        push dword ptr [esp] // original EDI = resource ID
        call DiagSRecordMembershipPass
        add esp,4
#endif
        popad
        popfd
        jmp dword ptr [g_sTramp+12]
    }
}
extern "C" __declspec(naked) void DiagSTransferBridge(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        push dword ptr [esp+18h] // original ECX: control
        call DiagSRecordTransfer
        add esp,4
#endif
        popad
        popfd
        jmp dword ptr [g_sTramp+16]
    }
}
// 005A65DF: first failed 0059CFC0 check for the +0330 list.
// At this point ESI=spooler and EDI=list node. Preserve all registers/flags.
extern "C" __declspec(naked) void DiagSWaitFailABridge(){
    __asm {
        pushfd
        pushad
#if defined(DSF_V29S_POOL_DIAG)
        mov eax,esp
        push eax
        push 3
        call DiagSRecordPool
        add esp,8
#endif
        mov eax,dword ptr [esp]   // original EDI: node
        mov edx,dword ptr [esp+4] // original ESI: spooler
        push 0
        push eax
        push edx
        call DiagSRecordWaitFail
        add esp,0Ch
        popad
        popfd
        jmp dword ptr [g_sWaitTramp]
    }
}
// 005A665A: failed 0059CFC0 check for the +059C list.
extern "C" __declspec(naked) void DiagSWaitFailBBridge(){
    __asm {
        pushfd
        pushad
#if defined(DSF_V29S_POOL_DIAG)
        mov eax,esp
        push eax
        push 4
        call DiagSRecordPool
        add esp,8
#endif
        mov eax,dword ptr [esp]
        mov edx,dword ptr [esp+4]
        push 1
        push eax
        push edx
        call DiagSRecordWaitFail
        add esp,0Ch
        popad
        popfd
        jmp dword ptr [g_sWaitTramp+4]
    }
}

// Generic native ID-container helpers. Filter to IDs 126/231 in the recorder;
// the original instruction sequences continue through complete trampolines.
extern "C" __declspec(naked) void DiagSMutationInsertA(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        mov eax,dword ptr [esp+18h]
        mov edx,dword ptr [esp+28h]
        mov ebx,dword ptr [esp+24h]
        push 0
        push ebx
        push edx
        push eax
        call DiagSRecordMutation
        add esp,10h
#endif
        popad
        popfd
        jmp dword ptr [g_sMutationTramp]
    }
}
extern "C" __declspec(naked) void DiagSMutationInsertB(){
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+18h]
#if defined(DSF_V29S_INCLUDE_EXPANDED)
        push eax
        call DiagSPrepareIncludeMutation
        add esp,4
        mov eax,dword ptr [esp+18h]
#endif
#if !defined(DSF_V29S_PERF_CONTROL)
        mov edx,dword ptr [esp+28h]
        mov ebx,dword ptr [esp+24h]
        push 1
        push ebx
        push edx
        push eax
        call DiagSRecordMutation
        add esp,10h
#endif
        popad
        popfd
        jmp dword ptr [g_sMutationTramp+4]
    }
}
extern "C" __declspec(naked) void DiagSMutationRemove(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        mov eax,dword ptr [esp+18h]
        mov edx,dword ptr [esp+28h]
        mov ebx,dword ptr [esp+24h]
        push 2
        push ebx
        push edx
        push eax
        call DiagSRecordMutation
        add esp,10h
#endif
        popad
        popfd
        jmp dword ptr [g_sMutationTramp+8]
    }
}

static void DiagSMutationRestore() {
    for(int i=2;i>=0;--i)DiagFRestoreDetour(g_sMutationDetours[i],g_sMutationTramp[i]);
    g_sMutationHooks=0;
}
static bool DiagSMutationInstall() {
    const U8 insertSig[]={0x83,0xEC,0x08,0x56,0x57};
    const U8 removeSig[]={0x8B,0x44,0x24,0x04,0x83,0xEC,0x08};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sMutationDetours[0],0x005A2740u,insertSig,sizeof(insertSig),
        reinterpret_cast<UPtr>(&DiagSMutationInsertA),g_sMutationTramp[0]))goto fail;
    if(!DiagFInstallDetour(g_sMutationDetours[1],0x005A2790u,insertSig,sizeof(insertSig),
        reinterpret_cast<UPtr>(&DiagSMutationInsertB),g_sMutationTramp[1]))goto fail;
    if(!DiagFInstallDetour(g_sMutationDetours[2],0x005A26A0u,removeSig,sizeof(removeSig),
        reinterpret_cast<UPtr>(&DiagSMutationRemove),g_sMutationTramp[2]))goto fail;
    g_sMutationHooks=3;return true;
fail:
    DiagSMutationRestore();return false;
}

// 005A200E: cmp dword ptr [edi+6694h],40h; the following JE skips the
// include-tree insertion at 005A202D. Original EDI=section, [ESP+18h]=ID.
extern "C" __declspec(naked) void DiagSGuardBridge(){
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_PERF_CONTROL)
        mov eax,dword ptr [esp]     // original EDI: section
        mov edx,dword ptr [esp+3Ch] // original [ESP+18h]: ID
        mov ebx,dword ptr [esp+34h] // original [ESP+10h]: caller
        push ebx
        push edx
        push eax
        call DiagSRecordGuard
        add esp,0Ch
#endif
#if defined(DSF_V29S_INCLUDE_EXPANDED)
        push dword ptr [esp]        // original EDI: section
        call DiagSPrepareInclude
        add esp,4
        test al,al
        jnz guard_allow
        or dword ptr [esp+20h],40h // ZF=1: take native skip at 005A2019
        jmp guard_resume
    guard_allow:
        and dword ptr [esp+20h],0FFFFFFBFh // ZF=0: native insert path
    guard_resume:
#endif
        popad
        popfd
#if defined(DSF_V29S_INCLUDE_EXPANDED)
        jmp dword ptr [g_sGuardContinue] // 005A2015 mov esi,[esp+18h]; je
#else
        jmp dword ptr [g_sGuardTramp]
#endif
    }
}
static bool DiagSGuardInstall() {
    const U8 sig[]={0x83,0xBF,0x94,0x66,0x00,0x00,0x40};
    if(DSF150_Status!=1)return false;
    const bool okay=DiagFInstallDetour(g_sGuardDetour,0x005A200Eu,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSGuardBridge),g_sGuardTramp);
#if defined(DSF_V29S_INCLUDE_EXPANDED)
    if(okay)g_sGuardContinue=Address(0x005A2015u);
#endif
    return okay;
}

static void DiagSWaitRestore() {
    for(int i=1;i>=0;--i)DiagFRestoreDetour(g_sWaitHooks[i],g_sWaitTramp[i]);
    DSFV29s_WaitHooks=0;
}
static bool DiagSWaitInstall() {
    // Validated against exact Driver.exe SHA256 c79987b...a26616.
    const U8 failA[]={0xC6,0x44,0x24,0x13,0x00};
    const U8 failB[]={0x5F,0x5E,0x5D,0x32,0xC0};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sWaitHooks[0],0x005A65DFu,failA,sizeof(failA),
            reinterpret_cast<UPtr>(&DiagSWaitFailABridge),g_sWaitTramp[0]))goto fail;
    if(!DiagFInstallDetour(g_sWaitHooks[1],0x005A665Au,failB,sizeof(failB),
            reinterpret_cast<UPtr>(&DiagSWaitFailBBridge),g_sWaitTramp[1]))goto fail;
    DSFV29s_WaitHooks=2;
    return true;
fail:
    DiagSWaitRestore();return false;
}

static void DiagSRestore() {
    for(int i=4;i>=0;--i)DiagFRestoreDetour(g_sHooks[i],g_sTramp[i]);
    DSFV29s_Hooks=0;
}
static bool DiagSInstall() {
    // Confirmed bytes in Driver.exe SHA256 c79987b...a26616.
    const U8 reconcile[]={0xA1,0x28,0x72,0xFE,0x00};
    const U8 gate[]={0x80,0xB9,0xB9,0x4D,0x07,0x00,0x00};
    const U8 body[]={0x83,0xEC,0x14,0x8B,0x44,0x24,0x18};
    const U8 passed[]={0x8B,0x4C,0x24,0x28,0x57,0x51};
    const U8 transfer[]={0x56,0x8B,0xF1,0x80,0x7E,0x4C,0x00};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sHooks[0],0x005A7490u,reconcile,sizeof(reconcile),
             reinterpret_cast<UPtr>(&DiagSReconcileBridge),g_sTramp[0]))goto fail;
    if(!DiagFInstallDetour(g_sHooks[1],0x0059DB40u,gate,sizeof(gate),
             reinterpret_cast<UPtr>(&DiagSGateBridge),g_sTramp[1]))goto fail;
    if(!DiagFInstallDetour(g_sHooks[2],0x0059D740u,body,sizeof(body),
             reinterpret_cast<UPtr>(&DiagSDispatchBridge),g_sTramp[2]))goto fail;
    if(!DiagFInstallDetour(g_sHooks[3],0x0059D778u,passed,sizeof(passed),
             reinterpret_cast<UPtr>(&DiagSPassBridge),g_sTramp[3]))goto fail;
    if(!DiagFInstallDetour(g_sHooks[4],0x0059BAD0u,transfer,sizeof(transfer),
             reinterpret_cast<UPtr>(&DiagSTransferBridge),g_sTramp[4]))goto fail;
    DSFV29s_Hooks=5;
    return true;
fail:
    DiagSRestore();return false;
}
static void DiagSPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    char line[560]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29s_STATUS tick=",end);DiagFAppendU(c,tick,end);
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29s_Hooks),end);
    DiagFAppendKeyU(c," reconcile=",static_cast<U32>(DSFV29s_Reconcile),end);
    DiagFAppendKeyU(c," rosterLast=",static_cast<U32>(DSFV29s_LastRoster),end);
    DiagFAppendKeyU(c," gate=",static_cast<U32>(DSFV29s_GateCalls),end);
    DiagFAppendKeyU(c," blocked=",static_cast<U32>(DSFV29s_GateBlocked),end);
    DiagFAppendKeyU(c," dispatch=",static_cast<U32>(DSFV29s_Dispatched),end);
    DiagFAppendKeyU(c," memberPass=",static_cast<U32>(DSFV29s_MembershipPass),end);
    DiagFAppendKeyU(c," transfers=",static_cast<U32>(DSFV29s_TransferCalls),end);
    DiagFAppendKeyU(c," transferHasHandler=",static_cast<U32>(DSFV29s_TransferWithHandler),end);
    DiagFAppendKeyU(c," lastGate=",static_cast<U32>(DSFV29s_LastGateByte),end);
    DiagFAppendKeyU(c," lastKind=",static_cast<U32>(DSFV29s_LastKind),end);
    DiagFAppendKeyU(c," lastId=",static_cast<U32>(DSFV29s_LastId),end);
    DiagFAppendKeyU(c," lastTransferId=",static_cast<U32>(DSFV29s_LastTransferId),end);
    DiagFAppendKeyU(c," lastTransferState=",static_cast<U32>(DSFV29s_LastTransferState),end);
    AppendText(c," map=",end);AppendNumber(c,DSF150_CompletedActive,end);
    DiagFAppendKeyU(c," reqHigh=",static_cast<U32>(DSFV27B_RequestHighWater),end);
    DiagFAppendKeyU(c," hashLeft=",static_cast<U32>(DSFV29_HashNativeRemaining),end);
    DiagFAppendKeyU(c," efGuard=",static_cast<U32>(DSFV29n_EFGuard),end);
    DiagFAppendKeyU(c," idOverflow=",static_cast<U32>(DSFV29s_IdOverflow),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
    for(U32 k=0;k<6u;++k){
        if(!g_sGateByKind[k])continue;
        char row[180]={};char* p=row;const char* lim=row+sizeof(row)-1;
        AppendText(p,"DSFV29s_KIND tick=",lim);DiagFAppendU(p,tick,lim);
        DiagFAppendKeyU(p," kind=",k,lim);
        DiagFAppendKeyU(p," gate=",static_cast<U32>(g_sGateByKind[k]),lim);
        DiagFAppendKeyU(p," blocked=",static_cast<U32>(g_sBlockedByKind[k]),lim);
        AppendText(p,"\n",lim);*p=0;DiagPrint(row);
    }
    static U32 oldGate[512]={},oldBlocked[512]={},oldDispatch[512]={},oldPass[512]={},oldTransfer[512]={};
    for(U32 id=0;id<512u;++id){
        const U32 g=static_cast<U32>(g_sGateById[id]);
        const U32 b=static_cast<U32>(g_sBlockedById[id]);
        const U32 d=static_cast<U32>(g_sDispatchById[id]);
        const U32 p=static_cast<U32>(g_sPassById[id]);
        const U32 t=static_cast<U32>(g_sTransferById[id]);
        if(g==oldGate[id]&&b==oldBlocked[id]&&d==oldDispatch[id]&&p==oldPass[id]&&t==oldTransfer[id])continue;
        oldGate[id]=g;oldBlocked[id]=b;oldDispatch[id]=d;oldPass[id]=p;oldTransfer[id]=t;
        char row[270]={};char* q=row;const char* lim=row+sizeof(row)-1;
        AppendText(q,"DSFV29s_ID tick=",lim);DiagFAppendU(q,tick,lim);
        DiagFAppendKeyU(q," id=",id,lim);
        DiagFAppendKeyU(q," gate=",g,lim);
        DiagFAppendKeyU(q," blocked=",b,lim);
        DiagFAppendKeyU(q," dispatch=",d,lim);
        DiagFAppendKeyU(q," pass=",p,lim);
        DiagFAppendKeyU(q," transfer=",t,lim);
        AppendText(q,"\n",lim);*q=0;DiagPrint(row);
    }
}

#if defined(DSF_V29S_POOL_DIAG)
static void DiagSPrintPoolEvents() {
    static U32 next=1u;
    const U32 written=static_cast<U32>(g_sPoolWritten);
    while(next<=written && next<=kSPoolEventCapacity) {
        const DiagSPoolEvent& e=g_sPoolEvents[next-1u];
        if(static_cast<U32>(e.seq)!=next)break;
        char line[800]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_POOL_CALL seq=",end);DiagFAppendU(p,next,end);
        DiagFAppendKeyU(p," tick=",e.tick,end);
        DiagFAppendKeyU(p," site=",e.site,end);
        if(e.id!=0xFFFFFFFFu)DiagFAppendKeyU(p," id=",e.id,end);
        DiagFAppendKeyHex(p," caller=",e.caller,end);
        DiagFAppendKeyHex(p," header=",e.header,end);
        DiagFAppendKeyHex(p," cursor=",e.cursor,end);
        DiagFAppendKeyU(p," free=",e.freeCount,end);
        DiagFAppendKeyU(p," active=",e.active,end);
        DiagFAppendKeyHex(p," rootOwner=",e.rootOwner,end);
        DiagFAppendKeyHex(p," reconcileOwner=",e.reconcileOwner,end);
        DiagFAppendKeyHex(p," waitOwner=",e.waitOwner,end);
        DiagFAppendKeyHex(p," eax=",e.eax,end);
        DiagFAppendKeyHex(p," ebx=",e.ebx,end);
        DiagFAppendKeyHex(p," ecx=",e.ecx,end);
        DiagFAppendKeyHex(p," edx=",e.edx,end);
        DiagFAppendKeyHex(p," esi=",e.esi,end);
        DiagFAppendKeyHex(p," edi=",e.edi,end);
        DiagFAppendKeyHex(p," ebp=",e.ebp,end);
        DiagFAppendKeyHex(p," esp=",e.esp,end);
        DiagFAppendKeyHex(p," eflags=",e.flags,end);
        DiagFAppendKeyHex(p," stack0=",e.stack0,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        ++next;
    }
}
static void DiagSPrintPoolState(U32 loops) {
    if(loops%5u!=0u)return; // one second, including first observation
    const UPtr owners[3]={DiagSPoolRootOwner(),g_sPoolReconcileOwner,g_sPoolWaitOwner};
    const U32 headers[4]={0xCCu,0x338u,0x5A4u,0x9A0u};
    const U32 ends[4]={0x330u,0x59Cu,0x998u,0xD94u};
    const U32 strides[4]={12u,12u,20u,20u};
    for(U32 role=0;role<3u;++role) {
        const UPtr owner=owners[role];
        if(!owner)continue;
        bool duplicate=false;
        for(U32 j=0;j<role;++j)if(owners[j]==owner)duplicate=true;
        if(duplicate)continue;
        for(U32 pool=0;pool<4u;++pool) {
            const UPtr h=owner+headers[pool],sentinel=owner+ends[pool];
            if(!Readable(h,12u) || !Readable(sentinel,8u))continue;
            const UPtr cursor=Read32(h);
            const U32 freeCount=Read32(h+4u),active=Read32(h+8u);
            const UPtr start=h+12u,limit=sentinel;
            const U32 cursorInline=cursor>=start && cursor<=limit &&
                ((cursor-start)%strides[pool])==0u?1u:0u;
            char line[370]={};char* p=line;const char* end=line+sizeof(line)-1;
            AppendText(p,"DSFV29s_POOL_STATE tick=",end);
            DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
            DiagFAppendKeyU(p," role=",role,end);
            DiagFAppendKeyU(p," pool=",pool,end);
            DiagFAppendKeyHex(p," owner=",owner,end);
            DiagFAppendKeyHex(p," header=",h,end);
            DiagFAppendKeyHex(p," cursor=",cursor,end);
            DiagFAppendKeyU(p," free=",freeCount,end);
            DiagFAppendKeyU(p," active=",active,end);
            DiagFAppendKeyU(p," cursorInline=",cursorInline,end);
            DiagFAppendKeyHex(p," sentinelNext=",Read32(sentinel),end);
            DiagFAppendKeyHex(p," sentinelPrev=",Read32(sentinel+4u),end);
            AppendText(p,"\n",end);*p=0;DiagPrint(line);
        }
    }
    if(loops%25u==0u) {
        char line[150]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_POOL_STATUS tick=",end);
        DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
        DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sPoolHooks),end);
        DiagFAppendKeyU(p," written=",static_cast<U32>(g_sPoolWritten),end);
        DiagFAppendKeyU(p," overflow=",static_cast<U32>(g_sPoolOverflow),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
}
#endif

#if defined(DSF_V29S_SPOOL0_EXPANDED)
static void DiagSPrintSpool0(U32 loops) {
    if(loops%25u!=0u)return;
    char line[300]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_SPOOL0 tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," capacity=",kSSpool0Nodes,end);
    DiagFAppendKeyU(p," redirects=",static_cast<U32>(g_sSpool0Redirects),end);
    DiagFAppendKeyU(p," arenas=",static_cast<U32>(g_sSpool0Used),end);
    DiagFAppendKeyU(p," high=",static_cast<U32>(g_sSpool0High),end);
    DiagFAppendKeyU(p," fault=",static_cast<U32>(g_sSpool0Fault),end);
    DiagFAppendKeyU(p," scratchHook=",static_cast<U32>(g_sScratchHook),end);
    DiagFAppendKeyU(p," scratchCalls=",static_cast<U32>(g_sScratchCalls),end);
    DiagFAppendKeyU(p," scratchRedirects=",static_cast<U32>(g_sScratchRedirects),end);
    DiagFAppendKeyU(p," scratchArenas=",static_cast<U32>(g_sScratchUsed),end);
    DiagFAppendKeyU(p," scratchFault=",static_cast<U32>(g_sScratchFault),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}
#endif
#if defined(DSF_V29S_CLONE_EXPANDED)
static void DiagSPrintClone(U32 loops) {
    if(loops%25u!=0u)return;
    char line[210]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CLONE tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," hook=",static_cast<U32>(g_sCloneHook),end);
    DiagFAppendKeyU(p," capacity=",kSCloneNodes,end);
    DiagFAppendKeyU(p," calls=",static_cast<U32>(g_sCloneCalls),end);
    DiagFAppendKeyU(p," redirects=",static_cast<U32>(g_sCloneRedirects),end);
    DiagFAppendKeyU(p," reuses=",static_cast<U32>(g_sCloneReuses),end);
    DiagFAppendKeyU(p," arenas=",static_cast<U32>(g_sCloneUsed),end);
    DiagFAppendKeyU(p," fault=",static_cast<U32>(g_sCloneFault),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}
#endif
#if defined(DSF_V29S_BUCKET_DIAG)
static void DiagSPrintBucket(U32 loops) {
    if(loops%25u!=0u)return;
    U32 maxBucket=0,maxCount=0;
    for(U32 i=0;i<25u;++i) {
        const U32 count=static_cast<U32>(g_sBucketMax[i]);
        if(count>maxCount) {maxCount=count;maxBucket=i;}
    }
    char line[280]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_BUCKET tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," hook=",static_cast<U32>(g_sBucketHook),end);
    DiagFAppendKeyU(p," samples=",static_cast<U32>(g_sBucketSamples),end);
    DiagFAppendKeyU(p," maxBucket=",maxBucket,end);
    DiagFAppendKeyU(p," maxCount=",maxCount,end);
    DiagFAppendKeyU(p," overflowWrites=",static_cast<U32>(g_sBucketOverflow),end);
    DiagFAppendKeyU(p," badIndex=",static_cast<U32>(g_sBucketBadIndex),end);
    AppendText(p," firstOverflowBucket=",end);
    AppendNumber(p,g_sBucketFirstOverflowBucket,end);
    AppendText(p," firstOverflowId=",end);
    AppendNumber(p,g_sBucketFirstOverflowId,end);
    DiagFAppendKeyU(p," firstOverflowTick=",static_cast<U32>(g_sBucketFirstOverflowTick),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    for(U32 i=0;i<25u;++i) {
        const U32 count=static_cast<U32>(g_sBucketMax[i]);
        if(!count)continue;
        char row[125]={};char* q=row;const char* limit=row+sizeof(row)-1;
        AppendText(q,"DSFV29s_BUCKET_ID index=",limit);DiagFAppendU(q,i,limit);
        DiagFAppendKeyU(q," high=",count,limit);
        DiagFAppendKeyU(q," lastId=",static_cast<U32>(g_sBucketLastId[i]),limit);
        AppendText(q,"\n",limit);*q=0;DiagPrint(row);
    }
}
#endif

#if defined(DSF_V29S_MODEL_DIAG)
static void DiagSPrintModel(U32 loops) {
    if(loops%25u!=0u)return;
    char line[260]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_MODEL tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," hook=",static_cast<U32>(g_sModelHook),end);
    DiagFAppendKeyU(p," calls=",static_cast<U32>(g_sModelCalls),end);
    DiagFAppendKeyU(p," nullDescriptor=",static_cast<U32>(g_sModelNull),end);
    DiagFAppendKeyU(p," outOfRange=",static_cast<U32>(g_sModelOutOfRange),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    for(U32 id=0;id<512u;++id) {
        const long count=g_sModelById[id];
        if(!count || count==g_sModelPrinted[id])continue;
        g_sModelPrinted[id]=count;
        char row[180]={};char* q=row;const char* limit=row+sizeof(row)-1;
        AppendText(q,"DSFV29s_MODEL_ID id=",limit);DiagFAppendU(q,id,limit);
        DiagFAppendKeyU(q," count=",static_cast<U32>(count),limit);
        DiagFAppendKeyHex(q," descriptor=",static_cast<U32>(g_sModelLastDescriptor[id]),limit);
        AppendText(q,"\n",limit);*q=0;DiagPrint(row);
    }
}
#endif

#if defined(DSF_V29S_LIVE_MODEL_DIAG)
static void DiagSPrintLiveModel(U32 loops) {
    if(loops%25u!=0u)return;
    for(U32 site=0;site<2u;++site) {
        char line[250]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_LIVE_MODEL tick=",end);
        DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
        DiagFAppendKeyU(p," site=",site,end);
        DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sLiveModelHooks),end);
        DiagFAppendKeyU(p," calls=",static_cast<U32>(g_sLiveModelCalls[site]),end);
        DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_sLiveModelInvalid[site]),end);
        DiagFAppendKeyU(p," outOfRange=",static_cast<U32>(g_sLiveModelOutOfRange[site]),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        for(U32 id=0;id<512u;++id) {
            const long count=g_sLiveModelById[site][id];
            if(!count || count==g_sLiveModelPrinted[site][id])continue;
            g_sLiveModelPrinted[site][id]=count;
            char row[150]={};char* q=row;const char* limit=row+sizeof(row)-1;
            AppendText(q,"DSFV29s_LIVE_MODEL_ID site=",limit);DiagFAppendU(q,site,limit);
            DiagFAppendKeyU(q," id=",id,limit);
            DiagFAppendKeyU(q," count=",static_cast<U32>(count),limit);
            AppendText(q,"\n",limit);*q=0;DiagPrint(row);
        }
    }
}
#endif

#if defined(DSF_V29S_REG_SOURCE_DIAG)
static void DiagSPrintRegSource(U32 loops) {
    long batchEnd=g_sRequestBatchNext;
    if(batchEnd>static_cast<long>(kSRequestBatchCapacity))batchEnd=kSRequestBatchCapacity;
    while(g_sRequestBatchDrained<batchEnd) {
        const long sequence=g_sRequestBatchDrained+1;
        const DiagSRequestBatch& batch=g_sRequestBatches[sequence-1];
        if(batch.sequence!=sequence)break;
        char header[220]={};char* p=header;const char* end=header+sizeof(header)-1;
        AppendText(p,"DSFV29s_REQ_BATCH seq=",end);DiagFAppendU(p,static_cast<U32>(sequence),end);
        DiagFAppendKeyU(p," tick=",batch.tick-g_diagStartTick,end);
        DiagFAppendKeyHex(p," context=",batch.context,end);
        DiagFAppendKeyU(p," original=",batch.originalCount,end);
        DiagFAppendKeyU(p," effective=",batch.effectiveCount,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(header);
        for(U32 which=0;which<2u;++which) {
            char line[1800]={};char* q=line;const char* limit=line+sizeof(line)-1;
            AppendText(q,which?"DSFV29s_REQ_EFFECTIVE seq=":"DSFV29s_REQ_ORIGINAL seq=",limit);
            DiagFAppendU(q,static_cast<U32>(sequence),limit);
            AppendText(q," ids=",limit);
            const U32 count=which?batch.effectiveCount:batch.originalCount;
            const U32 copy=count<kSRequestBatchIds?count:kSRequestBatchIds;
            for(U32 i=0;i<copy;++i) {
                if(i)AppendText(q,",",limit);
                DiagFAppendU(q,which?batch.effectiveIds[i]:batch.originalIds[i],limit);
            }
            if(copy<count)AppendText(q,",...",limit);
            AppendText(q,"\n",limit);*q=0;DiagPrint(line);
        }
        g_sRequestBatchDrained=sequence;
    }
    long eventEnd=g_sRegNext;
    if(eventEnd>static_cast<long>(kSRegEvents))eventEnd=kSRegEvents;
    while(g_sRegDrained<eventEnd) {
        const long sequence=g_sRegDrained+1;
        const DiagSRegEvent& event=g_sRegEvents[sequence-1];
        if(event.sequence!=sequence)break;
        char line[260]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_REG_EVENT seq=",end);DiagFAppendU(p,static_cast<U32>(sequence),end);
        DiagFAppendKeyU(p," tick=",event.tick-g_diagStartTick,end);
        DiagFAppendKeyU(p," site=",event.site,end);
        DiagFAppendKeyU(p," id=",event.id,end);
        DiagFAppendKeyHex(p," caller=",event.caller,end);
        DiagFAppendKeyHex(p," owner=",event.owner,end);
        DiagFAppendKeyHex(p," definition=",event.definition,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        g_sRegDrained=sequence;
    }
    if(loops%25u!=0u)return;
    char line[280]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REG_STATUS tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sRegHooks),end);
    for(U32 site=0;site<3u;++site) {
        U32 distinct=0;
        for(U32 id=0;id<512u;++id)if(g_sRegById[site][id])++distinct;
        AppendText(p,site==0u?" entry0=":site==1u?" entry1=":" create=",end);
        DiagFAppendU(p,static_cast<U32>(g_sRegCalls[site]),end);
        AppendText(p,site==0u?" unique0=":site==1u?" unique1=":" uniqueCreate=",end);
        DiagFAppendU(p,distinct,end);
    }
    DiagFAppendKeyU(p," eventDrops=",static_cast<U32>(g_sRegDrops),end);
    DiagFAppendKeyU(p," batchDrops=",static_cast<U32>(g_sRequestBatchDrops),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}
#endif

#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
static void DiagSPrintModelPool(U32 loops) {
    if(loops%25u!=0u)return;
    const UPtr global=Address(0x0144BCFCu);
    const UPtr pool=Readable(global,4u)?Read32(global):0u;
    const bool initialized=pool && Readable(pool+0x64u,4u);
    const U32 capacity=initialized?Read32(pool+0x48u):0u;
    const U32 freeCount=initialized?Read32(pool+0x4Cu):0u;
    const U32 freeHead=initialized?Read32(pool+0x50u):0u;
    const U32 endCapacity=initialized?Read32(pool+0x64u):0u;
    char line[270]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_MODEL_POOL tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," sourcePatched=",g_sModelPoolPatched?1u:0u,end);
    DiagFAppendKeyU(p," initialized=",initialized?1u:0u,end);
    DiagFAppendKeyHex(p," pool=",pool,end);
    DiagFAppendKeyU(p," capacity=",capacity,end);
    DiagFAppendKeyU(p," free=",freeCount,end);
    DiagFAppendKeyU(p," endCapacity=",endCapacity,end);
    DiagFAppendKeyHex(p," freeHead=",freeHead,end);
    DiagFAppendKeyU(p," fault=",initialized &&
        (capacity!=160u || endCapacity!=160u || freeCount>160u)?1u:0u,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    for(U32 i=0;i<kSRegMapArenas;++i) {
        const UPtr map=g_sRegMapArenas[i].map;
        if(!map)continue;
        const bool valid=Readable(map,12u);
        char row[240]={};char* q=row;const char* limit=row+sizeof(row)-1;
        AppendText(q,"DSFV29s_REG_MAP tick=",limit);
        DiagFAppendU(q,GetTickCount()-g_diagStartTick,limit);
        DiagFAppendKeyU(q," hook=",static_cast<U32>(g_sRegMapHook),limit);
        DiagFAppendKeyU(q," arena=",i,limit);
        DiagFAppendKeyHex(q," map=",map,limit);
        DiagFAppendKeyHex(q," cursor=",valid?Read32(map):0u,limit);
        DiagFAppendKeyU(q," active=",valid?Read32(map+8u):0u,limit);
        DiagFAppendKeyU(q," redirects=",static_cast<U32>(g_sRegMapRedirects),limit);
        DiagFAppendKeyU(q," externalHigh=",static_cast<U32>(g_sRegMapHigh),limit);
        DiagFAppendKeyU(q," fault=",static_cast<U32>(g_sRegMapFault),limit);
        AppendText(q,"\n",limit);*q=0;DiagPrint(row);
    }
}
#endif

#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
static void DiagSPrintResourceCollection(U32 loops) {
    if(loops%25u!=0u)return;
    const UPtr countAddress=Address(0x0188A6B8u);
    const U32 count=Readable(countAddress,4u)?Read32(countAddress):0xFFFFFFFFu;
    static U32 high=0u;
    if(count!=0xFFFFFFFFu && count>high)high=count;
    char line[270]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_RESOURCE_COLLECTION tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," patches=",g_sResourcePatchCount,end);
    DiagFAppendKeyU(p," count=",count,end);
    DiagFAppendKeyU(p," high=",high,end);
    DiagFAppendKeyU(p," capacity=",kResourceCollectionCapacity,end);
    DiagFAppendKeyHex(p," pointers=",reinterpret_cast<UPtr>(g_sResourcePointers),end);
    DiagFAppendKeyHex(p," pairs=",reinterpret_cast<UPtr>(g_sResourcePairs),end);
    DiagFAppendKeyU(p," fault=",g_sResourcePatchCount!=
        sizeof(g_sResourceArrayPatches)/sizeof(g_sResourceArrayPatches[0]) ||
        count>=kResourceCollectionCapacity?1u:0u,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}
#endif

#if defined(DSF_V29S_LOD_TRACE)
// 005A7FC6 is the branch-taken portion of VehicleLodSpooler.RequestVehicle.
// It records the object before the game writes base N, N+1000 and N+2000.
struct DiagSLodEvent {
    volatile long seq;
    U32 tick,owner,id,caller;
};
static const U32 kSLodEvents=512u;
static DiagSLodEvent g_sLodEvents[kSLodEvents]={};
static volatile long g_sLodNext=0,g_sLodDrops=0,g_sLodHook=0;
static DiagFDetour g_sLodDetour={};
static UPtr g_sLodTramp=0u;
extern "C" __declspec(noinline) void __cdecl DiagSRecordLod(UPtr owner,UPtr frame) {
    const long sequence=InterlockedIncrement(&g_sLodNext);
    if(sequence<1 || static_cast<U32>(sequence)>kSLodEvents) {
        InterlockedIncrement(&g_sLodDrops);return;
    }
    DiagSLodEvent& event=g_sLodEvents[sequence-1];
    event.tick=GetTickCount()-g_diagStartTick;
    event.owner=owner;
    event.id=Read32(frame+4u);
    event.caller=Read32(frame);
    InterlockedExchange(&event.seq,sequence);
}
extern "C" __declspec(naked) void DiagSLodBridge() {
    __asm {
        pushfd
        pushad
        lea eax,[esp+36]
        push eax
        push ecx
        call DiagSRecordLod
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sLodTramp]
    }
}
static bool DiagSLodInstall() {
    const U8 sig[]={0x8B,0x44,0x24,0x04,0x8D,0x90,0xE8,0x03,0x00,0x00};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sLodDetour,0x005A7FC6u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSLodBridge),g_sLodTramp);
}
static void DiagSPrintLod(U32 loops) {
    static U32 next=1u;
    static UPtr recentOwners[8]={};
    static U32 recentIds[8]={},recentTicks[8]={};
    const U32 written=static_cast<U32>(g_sLodNext);
    while(next<=written && next<=kSLodEvents) {
        const DiagSLodEvent& e=g_sLodEvents[next-1u];
        if(static_cast<U32>(e.seq)!=next)break;
        char line[270]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_LOD_REQUEST seq=",end);DiagFAppendU(p,next,end);
        DiagFAppendKeyU(p," tick=",e.tick,end);
        DiagFAppendKeyHex(p," owner=",e.owner,end);
        DiagFAppendKeyU(p," baseId=",e.id,end);
        DiagFAppendKeyU(p," extId=",e.id+2000u,end);
        DiagFAppendKeyHex(p," caller=",e.caller,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        const U32 index=next%8u;
        recentOwners[index]=e.owner;recentIds[index]=e.id;recentTicks[index]=e.tick;
        ++next;
    }
    if(loops%5u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    for(U32 i=0;i<8u;++i) {
        const UPtr owner=recentOwners[i];
        if(!owner || tick-recentTicks[i]>15000u)continue;
        const bool readable=Readable(owner,0x20u);
        const U32 base=readable?Read32(owner+8u):0xFFFFFFFFu;
        const bool same=readable && base==recentIds[i];
        char line[420]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_LOD_STATE tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyHex(p," owner=",owner,end);
        DiagFAppendKeyU(p," expectedId=",recentIds[i],end);
        DiagFAppendKeyU(p," same=",same?1u:0u,end);
        if(same) {
            const U32 active=Read32(owner+0xCu),requested=Read32(owner+0x10u);
            const U32 flags=Read32(owner+0x1Cu);
            DiagFAppendKeyU(p," state=",Read32(owner),end);
            DiagFAppendKeyU(p," intActive=",active&0xFFFFu,end);
            DiagFAppendKeyU(p," extActive=",active>>16u,end);
            DiagFAppendKeyU(p," intRequest=",requested&0xFFFFu,end);
            DiagFAppendKeyU(p," extRequest=",requested>>16u,end);
            DiagFAppendKeyU(p," intLoadedFlag=",flags&0xFFu,end);
            DiagFAppendKeyU(p," extLoadedFlag=",(flags>>8u)&0xFFu,end);
            DiagFAppendKeyU(p," gate1E=",(flags>>16u)&0xFFu,end);
        }
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
    if(loops%25u==0u) {
        char line[150]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_LOD_STATUS tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," hook=",static_cast<U32>(g_sLodHook),end);
        DiagFAppendKeyU(p," requests=",written,end);
        DiagFAppendKeyU(p," drops=",static_cast<U32>(g_sLodDrops),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
}
#endif

#if defined(DSF_V29S_EXTERIOR_TRACE)
// SceneModelType2 slot assignment. The first argument is recorded raw until
// runtime evidence establishes whether it is a VEHC ID. The second argument
// is the per-kind graphics slot written to model+564. No game-thread I/O.
struct DiagSExteriorEvent {
    volatile long seq;
    U32 tick,model,arg0,slot,returnAddress;
};
static const U32 kSExteriorEvents=2048u;
static DiagSExteriorEvent g_sExteriorEvents[kSExteriorEvents]={};
static volatile long g_sExteriorNext=0,g_sExteriorDrops=0,g_sExteriorHook=0;
static DiagFDetour g_sExteriorDetour={};
static UPtr g_sExteriorTramp=0u;
static volatile long g_sSelectedDrawModel=0,g_sSelectedNative=0,g_sDrawHooks=0;
static volatile long g_sSelectedBaseId=-1,g_sExteriorDrawModel=0;
static volatile long g_sDrawAll=0,g_sDrawSelected=0,g_sDrawMaskCalls=0;
static volatile long g_sDrawMasks[7]={}; // 0,1,2,4,8,16,other
static volatile long g_sExteriorDrawCalls=0,g_sExteriorMaskCalls=0;
static volatile long g_sExteriorMasks[7]={};
static DiagFDetour g_sDrawDetours[2]={};
static UPtr g_sDrawTramps[2]={};
#if defined(DSF_V29S_LOD_TRACE)
// 008D56A0 is the native vehicle body function. Its exact-exe body calls
// 008A8DA0 at 008D590C; count entry separately to locate the missing call.
struct DiagSBodySample {
    volatile long seq;
    U32 vehicle,model,caller;
};
static const U32 kSBodySamples=2048u;
static DiagSBodySample g_sBodySamples[kSBodySamples]={};
static volatile long g_sBodySampleNext=0,g_sBodyHook=0;
static DiagFDetour g_sBodyDetour={};
static UPtr g_sBodyTramp=0u;
extern "C" __declspec(noinline) void __cdecl DiagSRecordBodyEntry(UPtr vehicle,UPtr frame) {
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    const long sequence=InterlockedIncrement(&g_sBodySampleNext);
    DiagSBodySample& sample=g_sBodySamples[static_cast<U32>(sequence-1)%kSBodySamples];
    sample.vehicle=vehicle;
    sample.model=vehicle && Readable(vehicle+0x5B4u,4u)?Read32(vehicle+0x5B4u):0u;
    sample.caller=Read32(frame);
    InterlockedExchange(&sample.seq,sequence);
}
extern "C" __declspec(naked) void DiagSBodyEntryBridge() {
    __asm {
        pushfd
        pushad
        lea eax,[esp+36]
        push eax
        push ecx
        call DiagSRecordBodyEntry
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sBodyTramp]
    }
}
static bool DiagSBodyInstall() {
    const U8 sig[]={0x83,0xEC,0x74,0x53,0x8B,0xD9};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sBodyDetour,0x008D56A0u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSBodyEntryBridge),g_sBodyTramp);
}
// Two shared stages after both native and Shift-selected callers reach
// 008A8DA0: the branch merge and the model-definition submodel walk.
struct DiagSCommonSample {
    volatile long seq;
    U32 model,site,mode,flags;
};
static const U32 kSCommonSamples=2048u;
static DiagSCommonSample g_sCommonSamples[kSCommonSamples]={};
static volatile long g_sCommonNext=0,g_sCommonHooks=0;
static DiagFDetour g_sCommonDetours[2]={};
static UPtr g_sCommonTramps[2]={};
static void DiagSStoreCommon(UPtr model,U32 site,U32 mode,U32 flags) {
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    const long sequence=InterlockedIncrement(&g_sCommonNext);
    DiagSCommonSample& sample=g_sCommonSamples[static_cast<U32>(sequence-1)%kSCommonSamples];
    sample.model=model;sample.site=site;sample.mode=mode;sample.flags=flags;
    InterlockedExchange(&sample.seq,sequence);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordCommonMerge(UPtr frame,U32 mode) {
    const U32 flags=(Read32(frame+0x1154u)&0xFFu) |
        ((Read32(frame+0x1158u)&0xFFu)<<8u);
    DiagSStoreCommon(Read32(frame+0x111Cu),0u,mode,flags);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordSubmodelWalk(UPtr frame) {
    DiagSStoreCommon(Read32(frame+8u),1u,0u,0u);
}
extern "C" __declspec(naked) void DiagSCommonMergeBridge() {
    __asm {
        pushfd
        pushad
        mov edx,dword ptr [esp+4] // original ESI selector
        lea eax,[esp+36]          // original ESP
        push edx
        push eax
        call DiagSRecordCommonMerge
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sCommonTramps]
    }
}
extern "C" __declspec(naked) void DiagSSubmodelWalkBridge() {
    __asm {
        pushfd
        pushad
        lea eax,[esp+36]
        push eax
        call DiagSRecordSubmodelWalk
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sCommonTramps+4]
    }
}
static bool DiagSCommonInstall() {
    const U8 merge[]={0x8B,0x94,0x24,0x34,0x11,0x00,0x00};
    const U8 walk[]={0x81,0xEC,0xA8,0x00,0x00,0x00};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sCommonDetours[0],0x008A8ECAu,merge,sizeof(merge),
        reinterpret_cast<UPtr>(&DiagSCommonMergeBridge),g_sCommonTramps[0]))return false;
    if(!DiagFInstallDetour(g_sCommonDetours[1],0x008A7D10u,walk,sizeof(walk),
        reinterpret_cast<UPtr>(&DiagSSubmodelWalkBridge),g_sCommonTramps[1])) {
        DiagFRestoreDetour(g_sCommonDetours[0],g_sCommonTramps[0]);return false;
    }
    return true;
}
// Retain a short window of actual draw arguments. The render thread only
// records values; formatting and disk I/O stay on the monitor thread.
struct DiagSDrawSample {
    volatile long seq;
    U32 tick,model,phase,mask,caller;
};
static const U32 kSDrawSamples=2048u;
static DiagSDrawSample g_sDrawSamples[kSDrawSamples]={};
static volatile long g_sDrawSampleNext=0;
static void DiagSStoreDrawSample(UPtr model,U32 phase,U32 mask,UPtr caller) {
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    const long sequence=InterlockedIncrement(&g_sDrawSampleNext);
    DiagSDrawSample& sample=g_sDrawSamples[static_cast<U32>(sequence-1)%kSDrawSamples];
    sample.tick=GetTickCount()-g_diagStartTick;
    sample.model=model;
    sample.phase=phase;
    sample.mask=mask;
    sample.caller=caller;
    InterlockedExchange(&sample.seq,sequence);
}
#endif
extern "C" __declspec(noinline) void __cdecl DiagSRecordExterior(UPtr model,UPtr frame) {
    const long sequence=InterlockedIncrement(&g_sExteriorNext);
    if(sequence<1 || static_cast<U32>(sequence)>kSExteriorEvents) {
        InterlockedIncrement(&g_sExteriorDrops);return;
    }
    DiagSExteriorEvent& event=g_sExteriorEvents[sequence-1];
    event.tick=GetTickCount()-g_diagStartTick;
    event.model=model;
    event.arg0=Read32(frame+4u);
    event.slot=Read32(frame+8u);
    event.returnAddress=Read32(frame);
    InterlockedExchange(&event.seq,sequence);
}
extern "C" __declspec(naked) void DiagSExteriorBridge() {
    __asm {
        pushfd
        pushad
        lea eax,[esp+36]
        push eax
        push ecx
        call DiagSRecordExterior
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sExteriorTramp]
    }
}
static bool DiagSExteriorInstall() {
    const U8 sig[]={0x8B,0x44,0x24,0x08,0x89,0x81,0x64,0x05,0x00,0x00};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_sExteriorDetour,0x00858860u,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagSExteriorBridge),g_sExteriorTramp);
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordDrawEntry(UPtr model,UPtr caller) {
    InterlockedIncrement(&g_sDrawAll);
#if defined(DSF_V29S_LOD_TRACE)
    DiagSStoreDrawSample(model,0u,0xFFFFFFFFu,caller);
#endif
    if(model && model==static_cast<UPtr>(g_sSelectedDrawModel))
        InterlockedIncrement(&g_sDrawSelected);
#if defined(DSF_V29S_LOD_TRACE)
    else if(model && static_cast<U32>(g_sSelectedBaseId)<1000u &&
            Readable(model+0x4850u,4u)) {
        const UPtr definition=Read32(model+0x4850u);
        if(definition && Readable(definition+0x14u,4u) &&
           Read32(definition+0x14u)==static_cast<U32>(g_sSelectedBaseId)+2000u) {
            g_sExteriorDrawModel=static_cast<long>(model);
            InterlockedIncrement(&g_sExteriorDrawCalls);
        }
    }
#endif
}
extern "C" __declspec(noinline) void __cdecl DiagSRecordDrawMask(UPtr model,U32 mask) {
#if defined(DSF_V29S_LOD_TRACE)
    DiagSStoreDrawSample(model,1u,mask,0u);
#endif
    const U32 index=mask==0u?0u:mask==1u?1u:mask==2u?2u:
        mask==4u?3u:mask==8u?4u:mask==16u?5u:6u;
    if(model && model==static_cast<UPtr>(g_sSelectedDrawModel)) {
        InterlockedIncrement(&g_sDrawMaskCalls);
        InterlockedIncrement(&g_sDrawMasks[index]);
    }
#if defined(DSF_V29S_LOD_TRACE)
    else if(model && model==static_cast<UPtr>(g_sExteriorDrawModel)) {
        InterlockedIncrement(&g_sExteriorMaskCalls);
        InterlockedIncrement(&g_sExteriorMasks[index]);
    }
#endif
}
extern "C" __declspec(naked) void DiagSDrawEntryBridge() {
    __asm {
        pushfd
        pushad
        lea edx,[esp+36]
        push dword ptr [edx]
        push dword ptr [edx+16]
        call DiagSRecordDrawEntry
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sDrawTramps]
    }
}
extern "C" __declspec(naked) void DiagSDrawMaskBridge() {
    __asm {
        pushfd
        pushad
        lea edx,[esp+36]
        push eax
        push dword ptr [edx+0x111C]
        call DiagSRecordDrawMask
        add esp,8
        popad
        popfd
        jmp dword ptr [g_sDrawTramps+4]
    }
}
static bool DiagSDrawInstall() {
    const U8 entry[]={0xB8,0x04,0x11,0x00,0x00};
    const U8 mask[]={0x8B,0xF0,0x83,0xFE,0x01};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_sDrawDetours[0],0x008A8DA0u,entry,sizeof(entry),
        reinterpret_cast<UPtr>(&DiagSDrawEntryBridge),g_sDrawTramps[0]))return false;
    if(!DiagFInstallDetour(g_sDrawDetours[1],0x008A8E53u,mask,sizeof(mask),
        reinterpret_cast<UPtr>(&DiagSDrawMaskBridge),g_sDrawTramps[1])) {
        DiagFRestoreDetour(g_sDrawDetours[0],g_sDrawTramps[0]);return false;
    }
    return true;
}
#if defined(DSF_V29S_LOD_TRACE)
static void DiagSPrintCommonSamples(U32 tick) {
    struct CommonObject {U32 model,merge,walk,mode,flag54,flag58;} objects[64]={};
    U32 objectCount=0u,omitted=0u,missed=0u,merges=0u,walks=0u;
    static U32 next=1u;
    const U32 written=static_cast<U32>(g_sCommonNext);
    if(written>=next+kSCommonSamples) {
        missed=written-next-kSCommonSamples+1u;
        next=written-kSCommonSamples+1u;
    }
    while(next<=written) {
        const DiagSCommonSample& sample=g_sCommonSamples[(next-1u)%kSCommonSamples];
        if(static_cast<U32>(sample.seq)!=next) {++missed;++next;continue;}
        U32 index=0u;
        while(index<objectCount && objects[index].model!=sample.model)++index;
        if(index==objectCount) {
            if(objectCount==sizeof(objects)/sizeof(objects[0])) {
                ++omitted;++next;continue;
            }
            objects[index].model=sample.model;
            ++objectCount;
        }
        if(sample.site==0u) {
            ++merges;++objects[index].merge;
            objects[index].mode=sample.mode;
            if(sample.flags&0xFFu)++objects[index].flag54;
            if(sample.flags&0xFF00u)++objects[index].flag58;
        } else {++walks;++objects[index].walk;}
        ++next;
    }
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    char summary[270]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_COMMON_WINDOW tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," baseId=",static_cast<U32>(g_sSelectedBaseId),end);
    DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sCommonHooks),end);
    DiagFAppendKeyU(p," merge=",merges,end);
    DiagFAppendKeyU(p," walk=",walks,end);
    DiagFAppendKeyU(p," objects=",objectCount,end);
    DiagFAppendKeyU(p," omitted=",omitted,end);
    DiagFAppendKeyU(p," missed=",missed,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
    for(U32 i=0u;i<objectCount;++i) {
        const CommonObject& o=objects[i];
        U32 id=0xFFFFFFFFu,submodels=0xFFFFFFFFu;
        if(o.model && Readable(o.model+0x4850u,4u)) {
            const UPtr definition=Read32(o.model+0x4850u);
            if(definition && Readable(definition,0x76u)) {
                id=Read32(definition+0x14u);
                submodels=*reinterpret_cast<volatile U16*>(definition+0x74u);
            }
        }
        char line[350]={};char* q=line;const char* lineEnd=line+sizeof(line)-1;
        AppendText(q,"DSFV29s_COMMON_OBJECT tick=",lineEnd);DiagFAppendU(q,tick,lineEnd);
        DiagFAppendKeyHex(q," model=",o.model,lineEnd);
        DiagFAppendKeyU(q," id=",id,lineEnd);
        DiagFAppendKeyU(q," submodels=",submodels,lineEnd);
        DiagFAppendKeyU(q," merge=",o.merge,lineEnd);
        DiagFAppendKeyU(q," walk=",o.walk,lineEnd);
        DiagFAppendKeyU(q," mode=",o.mode,lineEnd);
        DiagFAppendKeyU(q," flag54=",o.flag54,lineEnd);
        DiagFAppendKeyU(q," flag58=",o.flag58,lineEnd);
        DiagFAppendKeyU(q," selected=",o.model==static_cast<U32>(g_sSelectedDrawModel)?1u:0u,lineEnd);
        AppendText(q,"\n",lineEnd);*q=0;DiagPrint(line);
    }
}
static void DiagSPrintBodySamples(U32 tick) {
    struct BodyObject {U32 model,vehicle,caller,calls;} objects[64]={};
    U32 objectCount=0u,omitted=0u,missed=0u,calls=0u;
    static U32 next=1u;
    const U32 written=static_cast<U32>(g_sBodySampleNext);
    if(written>=next+kSBodySamples) {
        missed=written-next-kSBodySamples+1u;
        next=written-kSBodySamples+1u;
    }
    while(next<=written) {
        const DiagSBodySample& sample=g_sBodySamples[(next-1u)%kSBodySamples];
        if(static_cast<U32>(sample.seq)!=next) {++missed;++next;continue;}
        ++calls;
        U32 index=0u;
        while(index<objectCount && objects[index].model!=sample.model)++index;
        if(index==objectCount) {
            if(objectCount==sizeof(objects)/sizeof(objects[0])) {
                ++omitted;++next;continue;
            }
            objects[index].model=sample.model;
            ++objectCount;
        }
        objects[index].vehicle=sample.vehicle;
        objects[index].caller=sample.caller;
        ++objects[index].calls;
        ++next;
    }
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    char summary[270]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_BODY_WINDOW tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," baseId=",static_cast<U32>(g_sSelectedBaseId),end);
    DiagFAppendKeyHex(p," selected=",static_cast<U32>(g_sSelectedDrawModel),end);
    DiagFAppendKeyU(p," calls=",calls,end);
    DiagFAppendKeyU(p," objects=",objectCount,end);
    DiagFAppendKeyU(p," omitted=",omitted,end);
    DiagFAppendKeyU(p," missed=",missed,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
    for(U32 i=0u;i<objectCount;++i) {
        const BodyObject& o=objects[i];
        U32 id=0xFFFFFFFFu;
        if(o.model && Readable(o.model+0x4850u,4u)) {
            const UPtr definition=Read32(o.model+0x4850u);
            if(definition && Readable(definition+0x14u,4u))
                id=Read32(definition+0x14u);
        }
        char line[310]={};char* q=line;const char* lineEnd=line+sizeof(line)-1;
        AppendText(q,"DSFV29s_BODY_OBJECT tick=",lineEnd);DiagFAppendU(q,tick,lineEnd);
        DiagFAppendKeyHex(q," vehicle=",o.vehicle,lineEnd);
        DiagFAppendKeyHex(q," model=",o.model,lineEnd);
        DiagFAppendKeyHex(q," caller=",o.caller,lineEnd);
        DiagFAppendKeyU(q," id=",id,lineEnd);
        DiagFAppendKeyU(q," calls=",o.calls,lineEnd);
        DiagFAppendKeyU(q," selected=",o.model==static_cast<U32>(g_sSelectedDrawModel)?1u:0u,lineEnd);
        AppendText(q,"\n",lineEnd);*q=0;DiagPrint(line);
    }
}
static void DiagSPrintDrawSamples(U32 tick) {
    struct DrawObject {
        U32 model,entry,masks,mask4,lastMask,selectedCaller,nativeCaller,otherCaller;
    } objects[64]={};
    U32 objectCount=0u,omitted=0u,missed=0u,samples=0u;
    static U32 next=1u;
    const U32 written=static_cast<U32>(g_sDrawSampleNext);
    if(written>=next+kSDrawSamples) {
        missed=written-next-kSDrawSamples+1u;
        next=written-kSDrawSamples+1u;
    }
    while(next<=written) {
        const DiagSDrawSample& sample=g_sDrawSamples[(next-1u)%kSDrawSamples];
        if(static_cast<U32>(sample.seq)!=next) {++missed;++next;continue;}
        ++samples;
        U32 index=0u;
        while(index<objectCount && objects[index].model!=sample.model)++index;
        if(index==objectCount) {
            if(objectCount==sizeof(objects)/sizeof(objects[0])) {
                ++omitted;++next;continue;
            }
            objects[index].model=sample.model;
            ++objectCount;
        }
        if(sample.phase==0u) {
            ++objects[index].entry;
            if(sample.caller==Address(0x00716C0Eu))++objects[index].selectedCaller;
            else if(sample.caller==Address(0x008D5911u))++objects[index].nativeCaller;
            else ++objects[index].otherCaller;
        }
        else {
            ++objects[index].masks;
            if(sample.mask==4u)++objects[index].mask4;
            objects[index].lastMask=sample.mask;
        }
        ++next;
    }
    if(static_cast<U32>(g_sSelectedBaseId)>=1000u)return;
    char summary[270]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_DRAW_WINDOW tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," baseId=",static_cast<U32>(g_sSelectedBaseId),end);
    DiagFAppendKeyHex(p," selected=",static_cast<U32>(g_sSelectedDrawModel),end);
    DiagFAppendKeyU(p," samples=",samples,end);
    DiagFAppendKeyU(p," objects=",objectCount,end);
    DiagFAppendKeyU(p," omitted=",omitted,end);
    DiagFAppendKeyU(p," missed=",missed,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
    for(U32 i=0u;i<objectCount;++i) {
        const DrawObject& o=objects[i];
        const U32 vptr=o.model && Readable(o.model,4u)?Read32(o.model):0u;
        U32 id=0xFFFFFFFFu,slot=0xFFFFFFFFu;
        if(vptr==Address(0x00E39EACu) && Readable(o.model+0x4850u,4u)) {
            const UPtr definition=Read32(o.model+0x4850u);
            if(definition && Readable(definition+0x14u,4u))
                id=Read32(definition+0x14u);
        } else if(vptr==Address(0x00E37118u) && Readable(o.model+0x564u,4u))
            slot=Read32(o.model+0x564u);
        char line[360]={};char* q=line;const char* lineEnd=line+sizeof(line)-1;
        AppendText(q,"DSFV29s_DRAW_OBJECT tick=",lineEnd);DiagFAppendU(q,tick,lineEnd);
        DiagFAppendKeyHex(q," model=",o.model,lineEnd);
        DiagFAppendKeyHex(q," vptr=",vptr,lineEnd);
        DiagFAppendKeyU(q," id=",id,lineEnd);
        DiagFAppendKeyU(q," slot=",slot,lineEnd);
        DiagFAppendKeyU(q," entry=",o.entry,lineEnd);
        DiagFAppendKeyU(q," masks=",o.masks,lineEnd);
        DiagFAppendKeyU(q," m4=",o.mask4,lineEnd);
        DiagFAppendKeyU(q," caller716C=",o.selectedCaller,lineEnd);
        DiagFAppendKeyU(q," caller8D59=",o.nativeCaller,lineEnd);
        DiagFAppendKeyU(q," callerOther=",o.otherCaller,lineEnd);
        DiagFAppendKeyHex(q," lastMask=",o.lastMask,lineEnd);
        DiagFAppendKeyU(q," selected=",o.model==static_cast<U32>(g_sSelectedDrawModel)?1u:0u,lineEnd);
        AppendText(q,"\n",lineEnd);*q=0;DiagPrint(line);
    }
}
#endif
static void DiagSPrintExterior(U32 loops) {
    static U32 next=1u;
    static UPtr recent[4]={};
    static U32 recentTicks[4]={};
    const U32 written=static_cast<U32>(g_sExteriorNext);
    while(next<=written && next<=kSExteriorEvents) {
        const DiagSExteriorEvent& e=g_sExteriorEvents[next-1u];
        if(static_cast<U32>(e.seq)!=next)break;
        char line[300]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_EXT_SLOT seq=",end);DiagFAppendU(p,next,end);
        DiagFAppendKeyU(p," tick=",e.tick,end);
        DiagFAppendKeyHex(p," model=",e.model,end);
        DiagFAppendKeyU(p," arg0=",e.arg0,end);
        DiagFAppendKeyU(p," slot=",e.slot,end);
        DiagFAppendKeyHex(p," caller=",e.returnAddress,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        const U32 index=next%4u;
        recent[index]=e.model;recentTicks[index]=e.tick;
        ++next;
    }
    if(loops%5u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
#if defined(DSF_V29S_LOD_TRACE)
    DiagSPrintCommonSamples(tick);
    DiagSPrintBodySamples(tick);
    DiagSPrintDrawSamples(tick);
#endif
    for(U32 i=0;i<4u;++i) {
        const UPtr model=recent[i];
        if(!model || tick-recentTicks[i]>10000u)continue;
        const bool readable=Readable(model,0x570u);
        const U32 vptr=readable?Read32(model):0u;
        const bool live=readable && vptr==Address(0x00E37118u);
        char line[440]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_EXT_MODEL tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyHex(p," model=",model,end);
        DiagFAppendKeyU(p," live=",live?1u:0u,end);
        DiagFAppendKeyHex(p," vptr=",vptr,end);
        if(live) {
            DiagFAppendKeyU(p," state544=",Read32(model+0x544u),end);
            DiagFAppendKeyHex(p," field548=",Read32(model+0x548u),end);
            DiagFAppendKeyHex(p," field54C=",Read32(model+0x54Cu),end);
            DiagFAppendKeyHex(p," field550=",Read32(model+0x550u),end);
            DiagFAppendKeyHex(p," field55C=",Read32(model+0x55Cu),end);
            DiagFAppendKeyU(p," slot564=",Read32(model+0x564u),end);
            DiagFAppendKeyU(p," flags568=",Read32(model+0x568u),end);
        }
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
    if(loops%25u==0u) {
        char line[150]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_EXT_STATUS tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," hook=",static_cast<U32>(g_sExteriorHook),end);
        DiagFAppendKeyU(p," events=",written,end);
        DiagFAppendKeyU(p," drops=",static_cast<U32>(g_sExteriorDrops),end);
        DiagFAppendKeyU(p," drawHooks=",static_cast<U32>(g_sDrawHooks),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
    static U32 priorAll=0u,priorSelected=0u,priorMask=0u;
    static U32 priorMasks[7]={};
    const U32 all=static_cast<U32>(g_sDrawAll);
    const U32 selected=static_cast<U32>(g_sDrawSelected);
    const U32 mask=static_cast<U32>(g_sDrawMaskCalls);
    char line[340]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_EXT_DRAW tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyHex(p," selectedModel=",static_cast<U32>(g_sSelectedDrawModel),end);
    DiagFAppendKeyU(p," allDelta=",all-priorAll,end);
    DiagFAppendKeyU(p," selectedDelta=",selected-priorSelected,end);
    DiagFAppendKeyU(p," maskDelta=",mask-priorMask,end);
    const char* names[7]={" m0="," m1="," m2="," m4="," m8="," m16="," mOther="};
    for(U32 i=0;i<7u;++i) {
        const U32 now=static_cast<U32>(g_sDrawMasks[i]);
        DiagFAppendKeyU(p,names[i],now-priorMasks[i],end);
        priorMasks[i]=now;
    }
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    priorAll=all;priorSelected=selected;priorMask=mask;
#if defined(DSF_V29S_LOD_TRACE)
    static U32 priorExterior=0u,priorExteriorMask=0u,priorExteriorMasks[7]={};
    const U32 exterior=static_cast<U32>(g_sExteriorDrawCalls);
    const U32 exteriorMask=static_cast<U32>(g_sExteriorMaskCalls);
    char exteriorLine[360]={};char* q=exteriorLine;
    const char* exteriorEnd=exteriorLine+sizeof(exteriorLine)-1;
    AppendText(q,"DSFV29s_LOD_DRAW tick=",exteriorEnd);DiagFAppendU(q,tick,exteriorEnd);
    DiagFAppendKeyU(q," baseId=",static_cast<U32>(g_sSelectedBaseId),exteriorEnd);
    DiagFAppendKeyHex(q," exteriorModel=",static_cast<U32>(g_sExteriorDrawModel),exteriorEnd);
    DiagFAppendKeyU(q," exteriorDelta=",exterior-priorExterior,exteriorEnd);
    DiagFAppendKeyU(q," maskDelta=",exteriorMask-priorExteriorMask,exteriorEnd);
    for(U32 i=0;i<7u;++i) {
        const U32 now=static_cast<U32>(g_sExteriorMasks[i]);
        DiagFAppendKeyU(q,names[i],now-priorExteriorMasks[i],exteriorEnd);
        priorExteriorMasks[i]=now;
    }
    AppendText(q,"\n",exteriorEnd);*q=0;DiagPrint(exteriorLine);
    priorExterior=exterior;priorExteriorMask=exteriorMask;
#endif
}
#endif

#if defined(DSF_V29S_ZAP_TARGET_DIAG)
// Mirrors the no-argument GetTargetedGameVehicle binding at 0065EDD0:
// singleton+678 -> player-0 controller, +338 -> selection, +1C or +18.
// The returned gameVehicle has vtable E3A74C and relevant state through at
// least +5EF. It is NOT the model object whose definition is at +4850.
// Only the monitor thread reads these fields. Retain the last target for ten
// seconds to observe the same native vehicle after the Zap target disappears.
static void DiagSZapWords(U32 tick,UPtr vehicle,const char* section,
                          UPtr address,U32 baseOffset,U32 wordCount) {
    if(!Readable(address,wordCount*4u))return;
    for(U32 first=0;first<wordCount;first+=8u) {
        char line[460]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_ZAP_WORDS tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyHex(p," vehicle=",vehicle,end);
        AppendText(p," section=",end);AppendText(p,section,end);
        DiagFAppendKeyHex(p," base=",address,end);
        for(U32 i=first;i<first+8u && i<wordCount;++i) {
            AppendText(p," ",end);AppendHex32(p,baseOffset+i*4u,end);
            AppendText(p,":",end);AppendHex32(p,Read32(address+i*4u),end);
        }
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
}
static void DiagSZapModel(U32 tick,UPtr vehicle,UPtr model) {
    // target+5B4 is the model object in the observed exact build. Its
    // E39EAC vtable dispatches to 0088C910, which reads model+4860.
    const bool valid=model && Readable(model,4u) &&
        Read32(model)==Address(0x00E39EACu) &&
        Readable(model+0x4800u,0x140u);
    const UPtr definition=valid?Read32(model+0x4850u):0u;
    const UPtr descriptor=valid?Read32(model+0x4858u):0u;
    const U32 id=definition && Readable(definition+0x14u,4u)
        ?Read32(definition+0x14u):0xFFFFFFFFu;
    char line[340]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_ZAP_MODEL tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyHex(p," vehicle=",vehicle,end);
    DiagFAppendKeyHex(p," model=",model,end);
    DiagFAppendKeyU(p," valid=",valid?1u:0u,end);
    DiagFAppendKeyHex(p," vptr=",model && Readable(model,4u)?Read32(model):0u,end);
    DiagFAppendKeyHex(p," definition=",definition,end);
    DiagFAppendKeyU(p," id=",id,end);
    DiagFAppendKeyHex(p," descriptor=",descriptor,end);
#if defined(DSF_V29S_MODEL_DIAG)
    DiagFAppendKeyHex(p," cachedDescriptor=",id<512u?
        static_cast<U32>(g_sModelLastDescriptor[id]):0u,end);
#endif
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    if(!valid)return;
    DiagSZapWords(tick,vehicle,"modelFields",model+0x4800u,0x4800u,80u);
    if(definition && Readable(definition,64u))
        DiagSZapWords(tick,vehicle,"definition",definition,0u,16u);
    if(descriptor && Readable(descriptor,64u))
        DiagSZapWords(tick,vehicle,"descriptor",descriptor,0u,16u);
}
static U32 DiagSZapSnapshot(U32 tick,UPtr vehicle,U32 phase) {
    const bool valid=vehicle && Readable(vehicle,0x600u) &&
        Read32(vehicle)==Address(0x00E3A74Cu);
    char line[430]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_ZAP_VEHICLE tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," phase=",phase,end);
    DiagFAppendKeyHex(p," vehicle=",vehicle,end);
    DiagFAppendKeyU(p," valid=",valid?1u:0u,end);
    DiagFAppendKeyHex(p," vptr=",vehicle && Readable(vehicle,4u)?Read32(vehicle):0u,end);
    DiagFAppendKeyHex(p," owner53C=",valid?Read32(vehicle+0x53Cu):0u,end);
    DiagFAppendKeyHex(p," handle558=",valid?Read32(vehicle+0x558u):0u,end);
    DiagFAppendKeyHex(p," handle560=",valid?Read32(vehicle+0x560u):0u,end);
    DiagFAppendKeyHex(p," handle564=",valid?Read32(vehicle+0x564u):0u,end);
    DiagFAppendKeyHex(p," state5A8=",valid?Read32(vehicle+0x5A8u):0u,end);
    DiagFAppendKeyU(p," collection=",Readable(Address(0x0188A6B8u),4u)?
        Read32(Address(0x0188A6B8u)):0xFFFFFFFFu,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    if(!valid)return 0u;
    DiagSZapModel(tick,vehicle,Read32(vehicle+0x5B4u));
    if(phase<=2u)DiagSZapWords(tick,vehicle,"object",vehicle,0u,384u);
    else {
        DiagSZapWords(tick,vehicle,"objectHead",vehicle,0u,32u);
        DiagSZapWords(tick,vehicle,"objectTail",vehicle+0x500u,0x500u,64u);
    }
    // Constructor/destructor paths use +53C, +558..+56C and +5CE..+5EF.
    // Log readable pointer candidates here without assuming their roles.
    if(phase>2u)return Read32(vehicle);
    for(U32 offset=0x500u;offset<=0x5BCu;offset+=4u) {
        const UPtr candidate=Read32(vehicle+offset);
        if(candidate<0x10000u || (candidate&3u) || !Readable(candidate,32u))continue;
        char row[180]={};char* q=row;const char* limit=row+sizeof(row)-1;
        AppendText(q,"DSFV29s_ZAP_CANDIDATE tick=",limit);DiagFAppendU(q,tick,limit);
        DiagFAppendKeyHex(q," vehicle=",vehicle,limit);
        DiagFAppendKeyHex(q," field=",offset,limit);
        DiagFAppendKeyHex(q," pointer=",candidate,limit);
        AppendText(q,"\n",limit);*q=0;DiagPrint(row);
        DiagSZapWords(tick,vehicle,"candidate",candidate,0u,8u);
    }
    return Read32(vehicle);
}
static void DiagSPrintZapTarget() {
    if(DSF150_Status!=2)return;
    const UPtr singletonAddress=Address(0x00FE7228u);
    const UPtr singleton=Readable(singletonAddress,4u)?Read32(singletonAddress):0u;
    const UPtr manager=singleton && Readable(singleton+0x678u,4u)
        ?Read32(singleton+0x678u):0u;
    const UPtr selection=manager && Readable(manager+0x350u,8u)
        ?manager+0x338u:0u;
    const UPtr first=selection?Read32(selection+0x1Cu):0u;
    const UPtr second=selection?Read32(selection+0x18u):0u;
    const UPtr target=first?first:second;
    const U32 tick=GetTickCount()-g_diagStartTick;
    static UPtr previous=0u,retained=0u;
    static U32 retainedTick=0u,lastSnapshot=0u,retainedVptr=0u;
    if(target!=previous) {
        char line[270]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_ZAP_TARGET tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyHex(p," manager=",manager,end);
        DiagFAppendKeyHex(p," first=",first,end);
        DiagFAppendKeyHex(p," second=",second,end);
        DiagFAppendKeyHex(p," target=",target,end);
        DiagFAppendKeyHex(p," previous=",previous,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        previous=target;
        if(target) {
            retained=target;retainedTick=tick;retainedVptr=DiagSZapSnapshot(tick,target,1u);
#if defined(DSF_V29S_EXTERIOR_TRACE)
            g_sSelectedNative=static_cast<long>(target);
            g_sSelectedBaseId=-1;
            g_sExteriorDrawModel=0;
            if(Readable(target+0x5B4u,4u)) {
                const UPtr model=Read32(target+0x5B4u);
                g_sSelectedDrawModel=static_cast<long>(model);
                if(model && Readable(model+0x4850u,4u)) {
                    const UPtr definition=Read32(model+0x4850u);
                    if(definition && Readable(definition+0x14u,4u))
                        g_sSelectedBaseId=static_cast<long>(Read32(definition+0x14u));
                }
            }
#endif
        }
        else if(retained) {retainedTick=tick;retainedVptr=DiagSZapSnapshot(tick,retained,2u);}
        lastSnapshot=tick;
    } else if(retained && tick-retainedTick<=10000u && tick-lastSnapshot>=1000u) {
        const U32 vptr=DiagSZapSnapshot(tick,retained,target?3u:4u);
        if(retainedVptr && vptr!=retainedVptr)retained=0u;
        lastSnapshot=tick;
    }
}
#endif

static void DiagSPrintWait(U32 loops) {
    if(loops%25u!=0u)return;
    const UPtr rootLocation=Address(kRootPtr);
    UPtr spooler=0;
    if(Readable(rootLocation,4u)) {
        const UPtr root=Read32(rootLocation);
        if(Readable(root+0x214u,4u))spooler=Read32(root+0x214u);
    }
    const bool valid=spooler && Readable(spooler+0xC8u,0xCF4u);
    char line[480]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29s_WAIT tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29s_WaitHooks),end);
    DiagFAppendKeyU(c," valid=",valid?1u:0u,end);
    if(valid) {
        DiagFAppendKeyU(c," countC8=",Read32(spooler+0xC8u),end);
        DiagFAppendKeyU(c," phaseDAC=",Read32(spooler+0xDACu),end);
        DiagFAppendKeyU(c," list33C=",Read32(spooler+0x33Cu),end);
        DiagFAppendKeyU(c," list340=",Read32(spooler+0x340u),end);
#if defined(DSF_V29S_LISTA_EXPANDED)
        DiagFAppendKeyHex(c," list338=",Read32(spooler+0x338u),end);
#endif
        DiagFAppendKeyU(c," list5A8=",Read32(spooler+0x5A8u),end);
        DiagFAppendKeyU(c," list5AC=",Read32(spooler+0x5ACu),end);
#if defined(DSF_V29S_LISTB_EXPANDED)
        DiagFAppendKeyHex(c," list5A4=",Read32(spooler+0x5A4u),end);
#endif
        DiagFAppendKeyU(c," list9A8=",Read32(spooler+0x9A8u),end);
        DiagFAppendKeyU(c," idD9C=",Read32(spooler+0xD9Cu),end);
        DiagFAppendKeyU(c," idDB0=",Read32(spooler+0xDB0u),end);
        DiagFAppendKeyU(c," idDB4=",Read32(spooler+0xDB4u),end);
    }
    DiagFAppendKeyU(c," failA=",static_cast<U32>(DSFV29s_WaitFailA),end);
    AppendText(c," firstA=",end);AppendNumber(c,DSFV29s_FirstWaitFailA,end);
    AppendText(c," lastA=",end);AppendNumber(c,DSFV29s_LastWaitFailA,end);
    DiagFAppendKeyU(c," failB=",static_cast<U32>(DSFV29s_WaitFailB),end);
    AppendText(c," firstB=",end);AppendNumber(c,DSFV29s_FirstWaitFailB,end);
    AppendText(c," lastB=",end);AppendNumber(c,DSFV29s_LastWaitFailB,end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
#if defined(DSF_V29S_LISTA_EXPANDED)
    char poolA[230]={};char* a=poolA;const char* aend=poolA+sizeof(poolA)-1;
    AppendText(a,"DSFV29s_LISTA tick=",aend);DiagFAppendU(a,GetTickCount()-g_diagStartTick,aend);
    DiagFAppendKeyU(a," hook=",static_cast<U32>(g_sListAHook),aend);
    DiagFAppendKeyU(a," inline=",kSListAInline,aend);
    DiagFAppendKeyU(a," capacity=",kSListACapacity,aend);
    DiagFAppendKeyU(a," redirects=",static_cast<U32>(g_sListARedirects),aend);
    DiagFAppendKeyU(a," arenas=",static_cast<U32>(g_sListAArenasUsed),aend);
    DiagFAppendKeyU(a," high=",static_cast<U32>(g_sListAHigh),aend);
    DiagFAppendKeyU(a," fault=",static_cast<U32>(g_sListAFault),aend);
    AppendText(a,"\n",aend);*a=0;DiagPrint(poolA);
#endif
#if defined(DSF_V29S_LISTB_EXPANDED)
    char pool[230]={};char* q=pool;const char* limit=pool+sizeof(pool)-1;
    AppendText(q,"DSFV29s_LISTB tick=",limit);DiagFAppendU(q,GetTickCount()-g_diagStartTick,limit);
    DiagFAppendKeyU(q," hook=",static_cast<U32>(g_sListBHook),limit);
    DiagFAppendKeyU(q," inline=",kSListBInline,limit);
    DiagFAppendKeyU(q," capacity=",kSListBCapacity,limit);
    DiagFAppendKeyU(q," redirects=",static_cast<U32>(g_sListBRedirects),limit);
    DiagFAppendKeyU(q," arenas=",static_cast<U32>(g_sListBArenasUsed),limit);
    DiagFAppendKeyU(q," high=",static_cast<U32>(g_sListBHigh),limit);
    DiagFAppendKeyU(q," fault=",static_cast<U32>(g_sListBFault),limit);
    AppendText(q,"\n",limit);*q=0;DiagPrint(pool);
#endif
}

static void DiagSPrintWaitIds(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    U32 distinctA=0,distinctB=0,activeA=0,activeB=0;
    static U32 oldA[512]={},oldB[512]={};
    for(U32 id=0;id<512u;++id) {
        const U32 a=static_cast<U32>(g_sWaitFailAById[id]);
        const U32 b=static_cast<U32>(g_sWaitFailBById[id]);
        if(a)++distinctA;
        if(b)++distinctB;
        if(a && tick-static_cast<U32>(g_sWaitLastAById[id])<=5000u)++activeA;
        if(b && tick-static_cast<U32>(g_sWaitLastBById[id])<=5000u)++activeB;
        if(a==oldA[id] && b==oldB[id])continue;
        oldA[id]=a;oldB[id]=b;
        char row[200]={};char* p=row;const char* end=row+sizeof(row)-1;
        AppendText(p,"DSFV29s_WAIT_ID tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," id=",id,end);
        DiagFAppendKeyU(p," failA=",a,end);
        DiagFAppendKeyU(p," lastA=",static_cast<U32>(g_sWaitLastAById[id]),end);
        DiagFAppendKeyU(p," failB=",b,end);
        DiagFAppendKeyU(p," lastB=",static_cast<U32>(g_sWaitLastBById[id]),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(row);
    }
    char summary[220]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_WAIT_IDS tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," distinctA=",distinctA,end);
    DiagFAppendKeyU(p," distinctB=",distinctB,end);
    DiagFAppendKeyU(p," recentA=",activeA,end);
    DiagFAppendKeyU(p," recentB=",activeB,end);
    DiagFAppendKeyU(p," overflow=",static_cast<U32>(g_sWaitIdOverflow),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
}

static void DiagSPrintMemberDetail(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    for(U32 i=0;i<2u;++i) {
        const DiagSMemberDetail& d=g_sMemberDetail[i];
        char line[240]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_MEMBER_DETAIL tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," id=",i==0u?126u:231u,end);
        DiagFAppendKeyU(p," samples=",static_cast<U32>(d.samples),end);
        DiagFAppendKeyU(p," sampleTick=",static_cast<U32>(d.tick),end);
        DiagFAppendKeyU(p," list=",static_cast<U32>(d.list),end);
        DiagFAppendKeyU(p," valid=",static_cast<U32>(d.valid),end);
        DiagFAppendKeyHex(p," includeTree=",g_sIncludeTree,end);
        if(d.valid) {
            DiagFAppendKeyU(p," include=",static_cast<U32>(d.include),end);
            DiagFAppendKeyU(p," exclude=",static_cast<U32>(d.exclude),end);
            DiagFAppendKeyU(p," global=",static_cast<U32>(d.global),end);
            DiagFAppendKeyU(p," effective=",static_cast<U32>(d.effective),end);
        }
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
}

static void DiagSPrintMutationEvents(U32 loops) {
    if(loops%25u!=0u)return;
    static U32 next=1u;
    const U32 written=static_cast<U32>(g_sMutationWrite);
    while(next<=written && next<=kSMutationCapacity) {
        const DiagSMutationEvent& event=g_sMutationEvents[next-1u];
        if(static_cast<U32>(event.seq)!=next)break;
        char row[240]={};char* p=row;const char* end=row+sizeof(row)-1;
        AppendText(p,"DSFV29s_MUTATION seq=",end);DiagFAppendU(p,next,end);
        DiagFAppendKeyU(p," tick=",event.tick,end);
        DiagFAppendKeyU(p," id=",event.id,end);
        DiagFAppendKeyU(p," op=",event.op,end);
        DiagFAppendKeyHex(p," tree=",event.tree,end);
        DiagFAppendKeyHex(p," caller=",event.caller,end);
        DiagFAppendKeyU(p," includeTree=",g_sIncludeTree?
            (event.tree==g_sIncludeTree?1u:0u):2u,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(row);
        ++next;
    }
    char summary[160]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_MUTATION_STATUS tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sMutationHooks),end);
    DiagFAppendKeyU(p," written=",written,end);
    DiagFAppendKeyU(p," drained=",next-1u,end);
    DiagFAppendKeyU(p," overflow=",static_cast<U32>(g_sMutationOverflow),end);
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
}

static void DiagSPrintGuard(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    for(U32 i=0;i<2u;++i) {
        const DiagSGuardDetail& d=g_sGuardDetail[i];
        char row[330]={};char* p=row;const char* end=row+sizeof(row)-1;
        AppendText(p,"DSFV29s_GUARD tick=",end);DiagFAppendU(p,tick,end);
        DiagFAppendKeyU(p," id=",i==0u?126u:231u,end);
        DiagFAppendKeyU(p," hooks=",static_cast<U32>(g_sGuardHook),end);
        DiagFAppendKeyU(p," calls=",static_cast<U32>(d.calls),end);
        DiagFAppendKeyU(p," at64=",static_cast<U32>(d.at64),end);
        if(d.calls) {
            DiagFAppendKeyU(p," firstCount=",static_cast<U32>(d.firstCount),end);
            DiagFAppendKeyU(p," lastCount=",static_cast<U32>(d.lastCount),end);
            DiagFAppendKeyU(p," firstTick=",static_cast<U32>(d.firstTick),end);
            DiagFAppendKeyU(p," lastTick=",static_cast<U32>(d.lastTick),end);
            DiagFAppendKeyHex(p," firstSection=",d.firstSection,end);
            DiagFAppendKeyHex(p," lastSection=",d.lastSection,end);
            DiagFAppendKeyHex(p," caller=",d.lastCaller,end);
            DiagFAppendKeyU(p," matchesInclude=",g_sIncludeTree?
                (d.lastSection+0x668Cu==g_sIncludeTree?1u:0u):2u,end);
        }
        if(d.at64) {
            DiagFAppendKeyU(p," first64Tick=",static_cast<U32>(d.first64Tick),end);
            DiagFAppendKeyHex(p," first64Section=",d.first64Section,end);
        }
        AppendText(p,"\n",end);*p=0;DiagPrint(row);
    }
#if defined(DSF_V29S_INCLUDE_EXPANDED)
    char summary[210]={};char* p=summary;const char* end=summary+sizeof(summary)-1;
    AppendText(p,"DSFV29s_INCLUDE tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyU(p," capacity=",kSIncludeCapacity,end);
    DiagFAppendKeyU(p," redirects=",static_cast<U32>(DSFV29s_IncludeRedirects),end);
    DiagFAppendKeyU(p," arenas=",static_cast<U32>(g_sIncludeArenasUsed),end);
    DiagFAppendKeyU(p," high=",static_cast<U32>(DSFV29s_IncludeHigh),end);
    DiagFAppendKeyU(p," fault=",static_cast<U32>(DSFV29s_IncludeFault),end);
    DiagFAppendKeyHex(p," tree=",g_sIncludeTree,end);
    if(g_sIncludeTree && Readable(g_sIncludeTree,12u)) {
        DiagFAppendKeyU(p," free=",Read32(g_sIncludeTree+4u),end);
        DiagFAppendKeyU(p," active=",Read32(g_sIncludeTree+8u),end);
        DiagFAppendKeyHex(p," cursor=",Read32(g_sIncludeTree),end);
    }
    AppendText(p,"\n",end);*p=0;DiagPrint(summary);
#endif
}

static void DiagSPrintRequestProbe(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    char summary[180]={};char* s=summary;const char* se=summary+sizeof(summary)-1;
    AppendText(s,"DSFV29s_REQ tick=",se);DiagFAppendU(s,tick,se);
    DiagFAppendKeyU(s," target=",DSF_CHAPTER7_PRELOAD_COUNT,se);
    DiagFAppendKeyU(s," high=",static_cast<U32>(DSFV27B_RequestHighWater),se);
    DiagFAppendKeyU(s," extraReads=",static_cast<U32>(DSFV27B_RequestExtraReads),se);
    DiagFAppendKeyU(s," fault=",static_cast<U32>(DSFV27B_RequestFault),se);
    DiagFAppendKeyU(s," bad=",static_cast<U32>(DSFV29s_RequestBoundaryBad),se);
    AppendText(s,"\n",se);*s=0;DiagPrint(summary);
    for(U32 i=0;i<10u;++i) {
        const U32 calls=static_cast<U32>(g_requestProbeCalls[i]);
        if(!calls)continue;
        const U32 index=kRequestProbeIndex[i];
        const U32 expected=DSF_CHAPTER7_PRELOAD_COUNT==45 ?
            (index<45u?kChapter7Trial45Ids[index]:0xFFFFu) :
            kChapter7Active150Ids[index];
        char row[230]={};char* c=row;const char* end=row+sizeof(row)-1;
        AppendText(c,"DSFV29s_REQ_ID tick=",end);DiagFAppendU(c,tick,end);
        DiagFAppendKeyU(c," index=",index,end);
        DiagFAppendKeyU(c," calls=",calls,end);
        DiagFAppendKeyU(c," id=",static_cast<U32>(g_requestProbeId[i]),end);
        DiagFAppendKeyU(c," expected=",expected,end);
        DiagFAppendKeyU(c," source=",index<kReqInline?0u:1u,end);
        DiagFAppendKeyU(c," found=",static_cast<U32>(g_requestProbeFound[i]),end);
        DiagFAppendKeyU(c," bad=",static_cast<U32>(g_requestProbeBad[i]),end);
        AppendText(c,"\n",end);*c=0;DiagPrint(row);
    }
}

// Read-only snapshots of completed-map handlers. The exact executable's
// 00E37118 vtable and 00859000 poll method use the fields below, but their
// values do not by themselves establish drawable READY. Sample every five
// seconds on the existing monitor thread.
static void DiagSPrintHandlerWords(U32 loops) {
    if(loops%25u!=0u || !g_pkg0)return;
    const UPtr pkg=g_pkg0;
    if(!Readable(pkg+0x5F54u,4u) || !Readable(pkg+0x5B44u,12u))return;
    const UPtr header=pkg+0x5B44u;
    const UPtr cursor=Read32(header);
    const U32 freeCount=Read32(header+4u);
    const U32 active=Read32(header+8u);
    const UPtr head=Read32(pkg+0x5F54u);
    const UPtr sentinel=pkg+0x5F50u;
    if(active>kDiagMaxMapNodes || freeCount>kDiagMaxMapNodes ||
       active+freeCount>kDiagMaxMapNodes)return;
    struct Row {
        U16 id; U32 handler; U32 words[16]; U32 readable;
        U32 stateReadable, count33C, work540, phase544, work548;
        U32 value54C, value550, count554, cursor558, object55C;
        U32 index560, value564, flags568, flags56C;
    };
    static Row rows[kDiagMaxMapNodes]={};
    const UPtr extraLo=reinterpret_cast<UPtr>(&g_mapExtra[0]);
    const UPtr extraHi=reinterpret_cast<UPtr>(&g_mapExtra[kMapExtraNodes]);
    UPtr current=head;
    for(U32 i=0;i<active;++i) {
        rows[i]={};
        const bool stock=current>=pkg+0x5B50u && current<sentinel &&
                         ((current-(pkg+0x5B50u))%16u)==0u;
        const bool extra=current>=extraLo && current<extraHi &&
                         ((current-extraLo)%16u)==0u;
        if((!stock && !extra) || !Readable(current,16u))return;
        rows[i].id=static_cast<U16>(Read32(current+8u));
        rows[i].handler=Read32(current+12u);
        if(rows[i].handler && Readable(rows[i].handler,64u)) {
            rows[i].readable=1u;
            for(U32 j=0;j<16u;++j)rows[i].words[j]=Read32(rows[i].handler+j*4u);
            if(rows[i].words[0]==0x00E37118u && Readable(rows[i].handler,0x570u)) {
                const UPtr h=rows[i].handler;
                rows[i].stateReadable=1u;
                rows[i].count33C=Read32(h+0x33Cu);
                rows[i].work540=Read32(h+0x540u);
                rows[i].phase544=Read32(h+0x544u);
                rows[i].work548=Read32(h+0x548u);
                rows[i].value54C=Read32(h+0x54Cu);
                rows[i].value550=Read32(h+0x550u);
                rows[i].count554=Read32(h+0x554u);
                rows[i].cursor558=Read32(h+0x558u);
                rows[i].object55C=Read32(h+0x55Cu);
                rows[i].index560=Read32(h+0x560u);
                rows[i].value564=Read32(h+0x564u);
                rows[i].flags568=Read32(h+0x568u);
                rows[i].flags56C=Read32(h+0x56Cu);
            }
        }
        current=Read32(current+4u);
    }
    if(current!=sentinel || Read32(header)!=cursor ||
       Read32(header+4u)!=freeCount || Read32(header+8u)!=active ||
       Read32(pkg+0x5F54u)!=head)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    char summary[160]={};char* s=summary;const char* se=summary+sizeof(summary)-1;
    AppendText(s,"DSFV29s_HANDLER_SAMPLE tick=",se);DiagFAppendU(s,tick,se);
    DiagFAppendKeyU(s," active=",active,se);
    AppendText(s," words=16 semantic=unknown\n",se);*s=0;DiagPrint(summary);
    for(U32 i=0;i<active;++i) {
        char line[440]={};char* c=line;const char* end=line+sizeof(line)-1;
        AppendText(c,"DSFV29s_HANDLER tick=",end);DiagFAppendU(c,tick,end);
        DiagFAppendKeyU(c," id=",rows[i].id,end);
        DiagFAppendKeyHex(c," ptr=",rows[i].handler,end);
        DiagFAppendKeyU(c," readable=",rows[i].readable,end);
        if(rows[i].readable) {
            for(U32 j=0;j<16u;++j) {
                AppendText(c," w",end);DiagFAppendU(c,j,end);
                AppendText(c,"=",end);AppendHex32(c,rows[i].words[j],end);
            }
        }
        AppendText(c,"\n",end);*c=0;DiagPrint(line);
        if(rows[i].stateReadable) {
            char state[480]={};char* q=state;const char* lim=state+sizeof(state)-1;
            AppendText(q,"DSFV29s_HANDLER_STATE tick=",lim);DiagFAppendU(q,tick,lim);
            DiagFAppendKeyU(q," id=",rows[i].id,lim);
            DiagFAppendKeyU(q," count33C=",rows[i].count33C,lim);
            DiagFAppendKeyHex(q," work540=",rows[i].work540,lim);
            DiagFAppendKeyU(q," phase544=",rows[i].phase544,lim);
            DiagFAppendKeyHex(q," work548=",rows[i].work548,lim);
            DiagFAppendKeyHex(q," value54C=",rows[i].value54C,lim);
            DiagFAppendKeyHex(q," value550=",rows[i].value550,lim);
            DiagFAppendKeyU(q," count554=",rows[i].count554,lim);
            DiagFAppendKeyU(q," cursor558=",rows[i].cursor558,lim);
            DiagFAppendKeyHex(q," object55C=",rows[i].object55C,lim);
            DiagFAppendKeyU(q," index560=",rows[i].index560,lim);
            DiagFAppendKeyHex(q," value564=",rows[i].value564,lim);
            DiagFAppendKeyHex(q," flags568=",rows[i].flags568,lim);
            DiagFAppendKeyHex(q," flags56C=",rows[i].flags56C,lim);
            AppendText(q,"\n",lim);*q=0;DiagPrint(state);
        }
    }
}

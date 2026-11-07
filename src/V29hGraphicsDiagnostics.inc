// V29h bounded observation of the exact-build shared graphics free/reject paths.
// No disk I/O or game calls run in either hook. The monitor drains a fixed ring.
extern "C" __declspec(dllexport) volatile long DSFV29h_Hooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29h_FreeCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29h_RejectCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29h_EventDrops=0;
static const U32 kDiagHMaxEvents=256u;
struct DiagHEvent {
    volatile long sequence;
    U32 kind,tick,pool,slot,part,subpool,raw14,raw1C;
    U32 caller,size,alignment;
};
static DiagHEvent g_diagHEvents[kDiagHMaxEvents]={};
static volatile long g_diagHNext=0;
static long g_diagHDrained=0;
static volatile long g_diagHFirstFree[18]={};
static volatile long g_diagHLastLowRaw[18]={};
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
// Separate circular ring keeps late vehicle-pool events available even if the
// earlier city-pool diagnostic fills its small startup ring.
static const U32 kDiagHVehicleEvents=4096u;
struct DiagHVehicleEvent {
    volatile long sequence;
    U32 tick,kind,pool,slot,part,raw14,raw1C,caller,size,alignment;
};
static DiagHVehicleEvent g_diagHVehicleEvents[kDiagHVehicleEvents]={};
static volatile long g_diagHVehicleNext=0;
static U32 g_diagHVehicleDrained=0u,g_diagHVehicleMissed=0u;
static bool DiagHVehicleClassify(UPtr sub,U32& pool,U32& slot,U32& part) {
    for(U32 row=0u;row<4u;++row) {
        const U32 array=g_diagFVehicleArrays[row],count=g_diagFVehicleCounts[row];
        if(!array || !count || sub<array || sub-array>=count*0x60u)continue;
        const U32 offset=sub-array,within=offset%0x60u;
        if(within!=0u && within!=0x2Cu)continue;
        pool=row+8u;slot=offset/0x60u;part=within?1u:0u;
        return true;
    }
    return false;
}
static void DiagHVehicleRecord(U32 kind,U32 pool,U32 slot,U32 part,
                               UPtr sub,UPtr caller,U32 size,U32 alignment) {
    const long sequence=InterlockedIncrement(&g_diagHVehicleNext);
    if(sequence<=0)return;
    DiagHVehicleEvent& e=g_diagHVehicleEvents[
        (static_cast<U32>(sequence)-1u)%kDiagHVehicleEvents];
    e.tick=GetTickCount()-g_diagStartTick;e.kind=kind;
    e.pool=pool;e.slot=slot;e.part=part;
    e.raw14=Read32(sub+0x14u);e.raw1C=Read32(sub+0x1Cu);
    e.caller=caller;e.size=size;e.alignment=alignment;
    InterlockedExchange(&e.sequence,sequence);
}
static void DiagHVehicleDrain() {
    const U32 written=static_cast<U32>(g_diagHVehicleNext);
    if(written>g_diagHVehicleDrained+kDiagHVehicleEvents) {
        g_diagHVehicleMissed+=written-g_diagHVehicleDrained-kDiagHVehicleEvents;
        g_diagHVehicleDrained=written-kDiagHVehicleEvents;
    }
    while(g_diagHVehicleDrained<written) {
        const U32 sequence=g_diagHVehicleDrained+1u;
        const DiagHVehicleEvent& e=g_diagHVehicleEvents[(sequence-1u)%kDiagHVehicleEvents];
        if(static_cast<U32>(e.sequence)!=sequence)break;
        char line[310]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_VPOOL_EVENT tick=",end);DiagFAppendU(p,e.tick,end);
        DiagFAppendKeyU(p," kind=",e.kind,end);
        DiagFAppendKeyU(p," pool=",e.pool,end);
        DiagFAppendKeyU(p," slot=",e.slot,end);
        DiagFAppendKeyU(p," part=",e.part,end);
        DiagFAppendKeyU(p," preRaw14=",e.raw14,end);
        DiagFAppendKeyU(p," preRaw1C=",e.raw1C,end);
        DiagFAppendKeyHex(p," callerCandidate=",e.caller,end);
        DiagFAppendKeyU(p," size=",e.size,end);
        DiagFAppendKeyU(p," alignment=",e.alignment,end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
        g_diagHVehicleDrained=sequence;
    }
}
static void DiagHVehicleStatus(U32 loops) {
    if(loops%25u!=0u)return;
    char line[220]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_VPOOL_STATUS tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," changes=",static_cast<U32>(g_diagFVehicleChanges),end);
    DiagFAppendKeyU(p," skipped=",static_cast<U32>(g_diagFVehicleSkipped),end);
    DiagFAppendKeyU(p," events=",static_cast<U32>(g_diagHVehicleNext),end);
    DiagFAppendKeyU(p," missed=",g_diagHVehicleMissed,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}
#endif
static DiagFDetour g_diagHFreeDetour={},g_diagHRejectDetour={};
extern "C" UPtr g_diagHFreeTrampoline=0;
extern "C" UPtr g_diagHRejectTrampoline=0;

static void DiagHClassify(UPtr sub,U32& pool,U32& slot,U32& part) {
    pool=slot=part=0xFFFFFFFFu;
    for(U32 i=0;i<21u;++i) {
        const UPtr runtime=Address(kGraphicsManager)+0x0Cu+i*0x5Cu;
        if(!Readable(runtime,8u))continue;
        const U32 array=Read32(runtime),count=Read32(runtime+4u);
        if(!array || !count || count>512u || sub<array ||
           sub-array>=count*0x60u)continue;
        const U32 offset=sub-array,within=offset%0x60u;
        if(within!=0u && within!=0x2Cu)continue;
        pool=i;slot=offset/0x60u;part=within?1u:0u;return;
    }
}
static void DiagHRecord(U32 kind,U32 pool,U32 slot,U32 part,UPtr sub,
                        U32 raw,U32 raw1C,UPtr caller,U32 size,U32 alignment) {
    const long sequence=InterlockedIncrement(&g_diagHNext);
    if(sequence<1 || static_cast<U32>(sequence)>kDiagHMaxEvents) {
        InterlockedIncrement(&DSFV29h_EventDrops);return;
    }
    DiagHEvent& e=g_diagHEvents[sequence-1];
    e.kind=kind;e.tick=GetTickCount()-g_diagStartTick;
    e.pool=pool;e.slot=slot;e.part=part;e.subpool=sub;
    e.raw14=raw;e.raw1C=raw1C;e.caller=caller;
    e.size=size;e.alignment=alignment;
    InterlockedExchange(&e.sequence,sequence);
}
extern "C" __declspec(noinline) void __cdecl DiagHObserveFree(UPtr sub,UPtr caller) {
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
    U32 vehiclePool=0u,vehicleSlot=0u,vehiclePart=0u;
    if(DiagHVehicleClassify(sub,vehiclePool,vehicleSlot,vehiclePart))
        DiagHVehicleRecord(1u,vehiclePool,vehicleSlot,vehiclePart,sub,caller,0u,0u);
#endif
    const U32 array=g_diagGArray;
    if(!array || sub<array || sub-array>=18u*0x60u)return;
    const U32 offset=sub-array;
    if(offset%0x60u!=0u)return; // first suballocator only
    const U32 slot=offset/0x60u;
    // The game is executing this suballocator method, so this header is live.
    const U32 raw=Read32(sub+0x14u),raw1C=Read32(sub+0x1Cu);
    InterlockedIncrement(&DSFV29h_FreeCalls);
    const bool first=InterlockedExchange(&g_diagHFirstFree[slot],1)==0;
    const bool low=raw<=5u && InterlockedExchange(&g_diagHLastLowRaw[slot],
        static_cast<long>(raw))!=static_cast<long>(raw);
    if(first || low)DiagHRecord(1u,7u,slot,0u,sub,raw,raw1C,caller,0u,0u);
}
extern "C" __declspec(noinline) void __cdecl DiagHObserveReject(
    UPtr sub,UPtr caller,U32 size,U32 alignment) {
    InterlockedIncrement(&DSFV29h_RejectCalls);
    U32 pool,slot,part;DiagHClassify(sub,pool,slot,part);
    U32 raw=0xFFFFFFFFu,raw1C=0xFFFFFFFFu;
    if(pool!=0xFFFFFFFFu && Readable(sub+0x14u,12u)) {
        raw=Read32(sub+0x14u);raw1C=Read32(sub+0x1Cu);
    }
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
    if(pool>=8u && pool<=11u)
        DiagHVehicleRecord(2u,pool,slot,part,sub,caller,size,alignment);
#endif
    DiagHRecord(2u,pool,slot,part,sub,raw,raw1C,caller,size,alignment);
}
extern "C" __declspec(naked) void DiagHFreeEntry() {
    __asm {
        pushfd
        pushad
#if !defined(DSF_V29S_VEHICLE_POOL_TRACE)
        mov eax,dword ptr [g_diagGArray]
        test eax,eax
        jz h_free_done
        mov edx,dword ptr [esp+24]
        cmp edx,eax
        jb h_free_done
        add eax,6C0h
        cmp edx,eax
        jae h_free_done
#endif
        // BB4890 entry ESP+28 is the return address of BB4EE0's caller.
        mov eax,dword ptr [esp+64]
        push eax
        push edx
        call DiagHObserveFree
        add esp,8
#if !defined(DSF_V29S_VEHICLE_POOL_TRACE)
    h_free_done:
#endif
        popad
        popfd
        jmp dword ptr [g_diagHFreeTrampoline]
    }
}
extern "C" __declspec(naked) void DiagHRejectEntry() {
    __asm {
        pushfd
        pushad
        // At BB52B9: ESI=suballocator, EBP=size, EBX=alignment.
        // Original ESP+64 is BB5470's caller (verify from stack in capture).
        mov eax,dword ptr [esp+4]
        mov edx,dword ptr [esp+100]
        mov ecx,dword ptr [esp+8]
        mov ebx,dword ptr [esp+16]
        push ebx
        push ecx
        push edx
        push eax
        call DiagHObserveReject
        add esp,16
        popad
        popfd
        jmp dword ptr [g_diagHRejectTrampoline]
    }
}
static bool DiagHInstallHooks() {
    const U8 freeSig[]={0x83u,0xECu,0x18u,0x8Bu,0x44u,0x24u,0x1Cu};
    const U8 rejectSig[]={0x5Fu,0x5Eu,0x5Du,0x33u,0xC0u};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_diagHFreeDetour,0x00BB4890u,freeSig,
        sizeof(freeSig),reinterpret_cast<UPtr>(&DiagHFreeEntry),
        g_diagHFreeTrampoline))return false;
    if(!DiagFInstallDetour(g_diagHRejectDetour,0x00BB52B9u,rejectSig,
        sizeof(rejectSig),reinterpret_cast<UPtr>(&DiagHRejectEntry),
        g_diagHRejectTrampoline)) {
        DiagFRestoreDetour(g_diagHFreeDetour,g_diagHFreeTrampoline);
        return false;
    }
    return true;
}
static void DiagHDrain() {
    long end=g_diagHNext;
    if(end>static_cast<long>(kDiagHMaxEvents))end=kDiagHMaxEvents;
    while(g_diagHDrained<end) {
        const long sequence=g_diagHDrained+1;
        const DiagHEvent& e=g_diagHEvents[sequence-1];
        if(e.sequence!=sequence)break;
        char line[360]={};char* c=line;const char* limit=line+sizeof(line)-1;
        AppendText(c,"DSFV29h_GRAPHICS kind=",limit);
        DiagFAppendU(c,e.kind,limit);
        DiagFAppendKeyU(c," tick=",e.tick,limit);
        DiagFAppendKeyU(c," pool=",e.pool,limit);
        DiagFAppendKeyU(c," slot=",e.slot,limit);
        DiagFAppendKeyU(c," part=",e.part,limit);
        DiagFAppendKeyHex(c," sub=",e.subpool,limit);
        DiagFAppendKeyU(c," preRaw14=",e.raw14,limit);
        DiagFAppendKeyU(c," preRaw1C=",e.raw1C,limit);
        DiagFAppendKeyHex(c," callerCandidate=",e.caller,limit);
        DiagFAppendKeyU(c," size=",e.size,limit);
        DiagFAppendKeyU(c," alignment=",e.alignment,limit);
        AppendText(c,"\n",limit);*c=0;DiagPrint(line);
        g_diagHDrained=sequence;
    }
}

static void DiagHPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;
    char line[240]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29h_STATUS tick=",end);
    DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29h_Hooks),end);
    DiagFAppendKeyU(c," freeCalls=",static_cast<U32>(DSFV29h_FreeCalls),end);
    DiagFAppendKeyU(c," rejectCalls=",static_cast<U32>(DSFV29h_RejectCalls),end);
    DiagFAppendKeyU(c," saved=",static_cast<U32>(g_diagHDrained),end);
    DiagFAppendKeyU(c," drops=",static_cast<U32>(DSFV29h_EventDrops),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
}

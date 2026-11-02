// V29r: passive, fixed-storage CALLER attribution for the recurring 00BAE790
// collection maintenance routine. No 00BAE81D hook: V29q measured nearly
// 2 million detours/second at the stall, distorting performance.
//
// At function entry ESP points to the CALLER return address. The bridge
// preserves all registers and flags before invoking a small cdecl recorder.
// Recorder performs no allocation, file I/O or game-memory reads/writes.
// A 64-slot lock-free table counts distinct return sites, with overflow explicit.
// Hook installation is all-or-nothing (entry and common exit); failure retains
// V29p behavior without optional V29r probes.

extern "C" __declspec(dllexport) volatile long DSFV29r_Hooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_Enter=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_Return=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_CallerOverflow=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_LastCaller=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_LastThread=0;
extern "C" __declspec(dllexport) volatile long DSFV29r_MaxOutstanding=0;

struct DiagRCallerSlot {
    volatile long caller;
    volatile long calls;
    volatile long firstTid;
    volatile long lastTid;
    volatile long lastOwner;
};
static DiagRCallerSlot g_diagRCallers[64]={};
static DiagFDetour g_diagREntryDetour={},g_diagRReturnDetour={};
extern "C" UPtr g_diagREntryTrampoline=0;
extern "C" UPtr g_diagRReturnTrampoline=0;

extern "C" __declspec(noinline) void __cdecl DiagRRecordEntry(U32 caller,U32 owner,U32 tid) {
    InterlockedIncrement(&DSFV29r_Enter);
    DSFV29r_LastCaller=static_cast<long>(caller);
    DSFV29r_LastThread=static_cast<long>(tid);
    if(!caller)return;
    const U32 mask=63u;
    U32 index=((caller>>2u)^(caller>>11u)^(caller>>19u))&mask;
    for(U32 tries=0;tries<64u;++tries) {
        DiagRCallerSlot& slot=g_diagRCallers[index];
        // On slot collision, only try to claim an empty slot. This is safe
        // with concurrent entries from multiple game threads.
        const long current=slot.caller;
        if(current==static_cast<long>(caller) ||
           (current==0 && InterlockedCompareExchange(&slot.caller,
               static_cast<long>(caller),0)==0)) {
            InterlockedIncrement(&slot.calls);
            InterlockedCompareExchange(&slot.firstTid,static_cast<long>(tid),0);
            slot.lastTid=static_cast<long>(tid);
            slot.lastOwner=static_cast<long>(owner);
            return;
        }
        index=(index+1u)&mask;
    }
    InterlockedIncrement(&DSFV29r_CallerOverflow);
}

extern "C" __declspec(naked) void DiagREntryBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+24h] // Original [ESP]: return address
        mov ecx,dword ptr [esp+18h] // Original ECX: collection owner
        mov edx,dword ptr fs:[24h]  // TEB ClientId.UniqueThread
        push edx
        push ecx
        push eax
        call DiagRRecordEntry
        add esp,0Ch
        popad
        popfd
        jmp dword ptr [g_diagREntryTrampoline]
    }
}
extern "C" __declspec(naked) void DiagRReturnBridge() {
    __asm {
        pushfd
        lock inc dword ptr [DSFV29r_Return]
        popfd
        jmp dword ptr [g_diagRReturnTrampoline]
    }
}
static void DiagRRestoreHooks() {
    DiagFRestoreDetour(g_diagRReturnDetour,g_diagRReturnTrampoline);
    DiagFRestoreDetour(g_diagREntryDetour,g_diagREntryTrampoline);
    DSFV29r_Hooks=0;
}
static bool DiagRInstallHooks() {
    // Matched to exact Driver.exe c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616
    const U8 entry[]={0x83u,0xECu,0x10u,0x33u,0xC0u};
    const U8 leave[]={0x5Eu,0x83u,0xC4u,0x10u,0xC3u};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_diagREntryDetour,0x00BAE790u,entry,sizeof(entry),
       reinterpret_cast<UPtr>(&DiagREntryBridge),g_diagREntryTrampoline))return false;
    if(!DiagFInstallDetour(g_diagRReturnDetour,0x00BAE921u,leave,sizeof(leave),
       reinterpret_cast<UPtr>(&DiagRReturnBridge),g_diagRReturnTrampoline)) {
        DiagRRestoreHooks();return false;
    }
    DSFV29r_Hooks=2;
    return true;
}
static void DiagRPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 entered=static_cast<U32>(DSFV29r_Enter);
    const U32 returned=static_cast<U32>(DSFV29r_Return);
    const U32 outstanding=entered>=returned?entered-returned:0xFFFFFFFFu;
    if(outstanding!=0xFFFFFFFFu && outstanding>static_cast<U32>(DSFV29r_MaxOutstanding))
        DSFV29r_MaxOutstanding=static_cast<long>(outstanding);
    static U32 prevEnter=0,prevReturn=0;
    char line[530]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29r_STATUS tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29r_Hooks),end);
    DiagFAppendKeyU(c," enter=",entered,end);
    DiagFAppendKeyU(c," return=",returned,end);
    DiagFAppendKeyU(c," enterDelta=",entered-prevEnter,end);
    DiagFAppendKeyU(c," returnDelta=",returned-prevReturn,end);
    DiagFAppendKeyU(c," outstanding=",outstanding,end);
    DiagFAppendKeyU(c," maxOutstanding=",static_cast<U32>(DSFV29r_MaxOutstanding),end);
    DiagFAppendKeyHex(c," lastCaller=",static_cast<U32>(DSFV29r_LastCaller),end);
    DiagFAppendKeyU(c," lastTid=",static_cast<U32>(DSFV29r_LastThread),end);
    DiagFAppendKeyU(c," overflow=",static_cast<U32>(DSFV29r_CallerOverflow),end);
    AppendText(c," map=",end);AppendNumber(c,DSF150_CompletedActive,end);
    DiagFAppendKeyU(c," hashLeft=",static_cast<U32>(DSFV29_HashNativeRemaining),end);
    DiagFAppendKeyU(c," hashExtra=",static_cast<U32>(DSFV29_HashExtraGranted),end);
    DiagFAppendKeyU(c," efExtra=",static_cast<U32>(DSFV29n_EFExtraInserts),end);
    DiagFAppendKeyU(c," efGuard=",static_cast<U32>(DSFV29n_EFGuard),end);
    const UPtr observedCount=Address(0x0188A6B8u);
    if(Readable(observedCount,4u))
        DiagFAppendKeyU(c," collectionCount=",Read32(observedCount),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
    prevEnter=entered;prevReturn=returned;

    // Fixed-size snapshot and top-8 selection only on existing monitor thread.
    // Delta means new calls since preceding five-second interval.
    static U32 previous[64]={};
    U32 bestIndex[8]={},bestDelta[8]={};
    U32 activeCallers=0;
    for(U32 i=0;i<64u;++i) {
        const U32 caller=static_cast<U32>(g_diagRCallers[i].caller);
        const U32 total=static_cast<U32>(g_diagRCallers[i].calls);
        const U32 delta=total-previous[i];previous[i]=total;
        if(!caller)continue;
        ++activeCallers;
        if(!delta || delta<=bestDelta[7])continue;
        for(U32 j=0;j<8u;++j) {
            if(delta>bestDelta[j]) {
                for(U32 k=7u;k>j;--k){bestDelta[k]=bestDelta[k-1];bestIndex[k]=bestIndex[k-1];}
                bestDelta[j]=delta;bestIndex[j]=i;break;
            }
        }
    }
    for(U32 rank=0;rank<8u;++rank) {
        if(!bestDelta[rank])break;
        const DiagRCallerSlot& slot=g_diagRCallers[bestIndex[rank]];
        char entryLine[280]={};char* p=entryLine;const char* lim=entryLine+sizeof(entryLine)-1;
        AppendText(p,"DSFV29r_CALLER tick=",lim);DiagFAppendU(p,GetTickCount()-g_diagStartTick,lim);
        DiagFAppendKeyU(p," rank=",rank+1u,lim);
        DiagFAppendKeyHex(p," caller=",static_cast<U32>(slot.caller),lim);
        DiagFAppendKeyU(p," total=",static_cast<U32>(slot.calls),lim);
        DiagFAppendKeyU(p," delta=",bestDelta[rank],lim);
        DiagFAppendKeyU(p," firstTid=",static_cast<U32>(slot.firstTid),lim);
        DiagFAppendKeyU(p," lastTid=",static_cast<U32>(slot.lastTid),lim);
        DiagFAppendKeyHex(p," lastOwner=",static_cast<U32>(slot.lastOwner),lim);
        AppendText(p,"\n",lim);*p=0;DiagPrint(entryLine);
    }
    if(activeCallers>=64u)DiagPrint("DSFV29r_WARNING callerTableFull=1 check overflow counter\n");
}

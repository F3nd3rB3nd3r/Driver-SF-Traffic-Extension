// V29q: observation-only instrumentation for the exact Driver.exe 0x00BAE790
// hash collection compaction/traversal routine seen on the V29p main thread.
//
// No game-owned memory is modified beyond three verified instruction detours.
// No file I/O, memory allocation, game callbacks, or debug breaks in hot hooks.
// Count arrivals, the common epilogue, and next-node traversals separately.
// This does NOT classify the routine as a loading dependency by itself.
//
// EIP 00BAE81D: mov eax,[eax+8]; test eax,eax. ChainSteps is a non-atomic,
// approximate count for minimal overhead. Enter/Return counters are atomic.
// Samples capture current node, lookup keys, EBP and ESI every 16384 steps.
// Hook installation is all-or-nothing; failure leaves V29p playable.
extern "C" __declspec(dllexport) volatile long DSFV29q_Hooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_Enter=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_Return=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_ChainSteps=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_LastEntryThread=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_FirstEntryThread=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_LastEntryOwner=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_SampleNode=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_SampleKeyA=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_SampleKeyB=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_SampleIterator=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_SampleOwner=0;
extern "C" __declspec(dllexport) volatile long DSFV29q_MaxOutstanding=0;

static DiagFDetour g_diagQEntryDetour={},g_diagQStepDetour={},g_diagQReturnDetour={};
extern "C" UPtr g_diagQEntryTrampoline=0;
extern "C" UPtr g_diagQStepTrampoline=0;
extern "C" UPtr g_diagQReturnTrampoline=0;

extern "C" __declspec(naked) void DiagQEntryBridge() {
    __asm {
        pushfd
        push eax
        mov eax,dword ptr fs:[24h]  // TEB.ClientId.UniqueThread, Win32
        mov dword ptr [DSFV29q_LastEntryThread],eax
        cmp dword ptr [DSFV29q_FirstEntryThread],0
        jne q_entry_seen
        mov dword ptr [DSFV29q_FirstEntryThread],eax
    q_entry_seen:
        mov dword ptr [DSFV29q_LastEntryOwner],ecx
        pop eax
        lock inc dword ptr [DSFV29q_Enter]
        popfd
        jmp dword ptr [g_diagQEntryTrampoline]
    }
}
extern "C" __declspec(naked) void DiagQStepBridge() {
    __asm {
        pushfd
        // Non-atomic deliberately: telemetry is approximate if concurrent.
        inc dword ptr [DSFV29q_ChainSteps]
        test dword ptr [DSFV29q_ChainSteps],3FFFh
        jne q_step_done
        mov dword ptr [DSFV29q_SampleNode],eax
        mov dword ptr [DSFV29q_SampleKeyA],ecx
        mov dword ptr [DSFV29q_SampleKeyB],edi
        mov dword ptr [DSFV29q_SampleIterator],ebp
        mov dword ptr [DSFV29q_SampleOwner],esi
    q_step_done:
        popfd
        jmp dword ptr [g_diagQStepTrampoline]
    }
}
extern "C" __declspec(naked) void DiagQReturnBridge() {
    __asm {
        pushfd
        lock inc dword ptr [DSFV29q_Return]
        popfd
        jmp dword ptr [g_diagQReturnTrampoline]
    }
}

static void DiagQRestoreHooks() {
    // Restore in reverse order. Trampolines are intentionally process-lifetime.
    DiagFRestoreDetour(g_diagQReturnDetour,g_diagQReturnTrampoline);
    DiagFRestoreDetour(g_diagQStepDetour,g_diagQStepTrampoline);
    DiagFRestoreDetour(g_diagQEntryDetour,g_diagQEntryTrampoline);
    DSFV29q_Hooks=0;
}
static bool DiagQInstallHooks() {
    // Fully aligned instructions from SHA-256
    // c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616.
    const U8 entry[]={0x83u,0xECu,0x10u,0x33u,0xC0u};
    const U8 step[]={0x8Bu,0x40u,0x08u,0x85u,0xC0u};
    const U8 leave[]={0x5Eu,0x83u,0xC4u,0x10u,0xC3u};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_diagQEntryDetour,0x00BAE790u,entry,sizeof(entry),
        reinterpret_cast<UPtr>(&DiagQEntryBridge),g_diagQEntryTrampoline))return false;
    if(!DiagFInstallDetour(g_diagQStepDetour,0x00BAE81Du,step,sizeof(step),
        reinterpret_cast<UPtr>(&DiagQStepBridge),g_diagQStepTrampoline)) {
        DiagQRestoreHooks();return false;
    }
    if(!DiagFInstallDetour(g_diagQReturnDetour,0x00BAE921u,leave,sizeof(leave),
        reinterpret_cast<UPtr>(&DiagQReturnBridge),g_diagQReturnTrampoline)) {
        DiagQRestoreHooks();return false;
    }
    DSFV29q_Hooks=3;
    return true;
}
static void DiagQPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;  // 5 s; no hot-path logging
    const U32 entered=static_cast<U32>(DSFV29q_Enter);
    const U32 returned=static_cast<U32>(DSFV29q_Return);
    const U32 steps=static_cast<U32>(DSFV29q_ChainSteps);
    const U32 outstanding=entered>=returned?entered-returned:0xFFFFFFFFu;
    if(outstanding!=0xFFFFFFFFu && outstanding>static_cast<U32>(DSFV29q_MaxOutstanding))
        DSFV29q_MaxOutstanding=static_cast<long>(outstanding);
    static U32 prevEnter=0,prevReturn=0,prevSteps=0;
    char line[550]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29q_TRAVERSE tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29q_Hooks),end);
    DiagFAppendKeyU(c," enter=",entered,end);
    DiagFAppendKeyU(c," return=",returned,end);
    DiagFAppendKeyU(c," stepsApprox=",steps,end);
    DiagFAppendKeyU(c," enterDelta=",entered-prevEnter,end);
    DiagFAppendKeyU(c," returnDelta=",returned-prevReturn,end);
    DiagFAppendKeyU(c," stepsDeltaApprox=",steps-prevSteps,end);
    DiagFAppendKeyU(c," outstanding=",outstanding,end);
    DiagFAppendKeyU(c," maxOutstanding=",static_cast<U32>(DSFV29q_MaxOutstanding),end);
    DiagFAppendKeyU(c," firstTid=",static_cast<U32>(DSFV29q_FirstEntryThread),end);
    DiagFAppendKeyU(c," lastTid=",static_cast<U32>(DSFV29q_LastEntryThread),end);
    DiagFAppendKeyHex(c," lastOwner=",static_cast<U32>(DSFV29q_LastEntryOwner),end);
    DiagFAppendKeyHex(c," sampleNode=",static_cast<U32>(DSFV29q_SampleNode),end);
    DiagFAppendKeyHex(c," sampleA=",static_cast<U32>(DSFV29q_SampleKeyA),end);
    DiagFAppendKeyHex(c," sampleB=",static_cast<U32>(DSFV29q_SampleKeyB),end);
    DiagFAppendKeyHex(c," sampleEBP=",static_cast<U32>(DSFV29q_SampleIterator),end);
    DiagFAppendKeyHex(c," sampleESI=",static_cast<U32>(DSFV29q_SampleOwner),end);
    const UPtr observedCount=Address(0x0188A6B8u);
    if(Readable(observedCount,4u))
        DiagFAppendKeyU(c," collectionCount=",Read32(observedCount),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
    prevEnter=entered;prevReturn=returned;prevSteps=steps;
}

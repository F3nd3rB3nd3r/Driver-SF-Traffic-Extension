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

static DiagFDetour g_sHooks[5]={};
static UPtr g_sTramp[5]={};
static volatile long g_sGateByKind[6]={},g_sBlockedByKind[6]={};
static volatile long g_sGateById[512]={},g_sBlockedById[512]={};
static volatile long g_sDispatchById[512]={},g_sPassById[512]={},g_sTransferById[512]={};
static volatile long g_sTransferState[16]={};

extern "C" __declspec(noinline) void __cdecl DiagSRecordReconcile(UPtr owner) {
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

// pushfd+pushad: original ECX=[esp+18h], EDI=[esp], caller=[esp+24h],
// original first two stack args=[esp+28h],[esp+2Ch].
extern "C" __declspec(naked) void DiagSReconcileBridge(){
    __asm {
        pushfd
        pushad
        push dword ptr [esp+18h]
        call DiagSRecordReconcile
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sTramp]
    }
}
// g_sTramp is a flat array. MSVC inline assembly needs a literal +offset.
extern "C" __declspec(naked) void DiagSGateBridge(){
    __asm {
        pushfd
        pushad
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
        popad
        popfd
        jmp dword ptr [g_sTramp+4]
    }
}
extern "C" __declspec(naked) void DiagSDispatchBridge(){
    __asm {
        pushfd
        pushad
        push dword ptr [esp+2Ch] // model ID (second stack arg)
        call DiagSRecordDispatch
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sTramp+8]
    }
}
extern "C" __declspec(naked) void DiagSPassBridge(){
    __asm {
        pushfd
        pushad
        push dword ptr [esp] // original EDI = resource ID
        call DiagSRecordMembershipPass
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sTramp+12]
    }
}
extern "C" __declspec(naked) void DiagSTransferBridge(){
    __asm {
        pushfd
        pushad
        push dword ptr [esp+18h] // original ECX: control
        call DiagSRecordTransfer
        add esp,4
        popad
        popfd
        jmp dword ptr [g_sTramp+16]
    }
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

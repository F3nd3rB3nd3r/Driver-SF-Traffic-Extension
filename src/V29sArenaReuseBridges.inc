// Mid-function/entry diagnostics preserve the whole state, including DF/FX.
#define REUSE_SCRATCH_BRIDGE(name,site,offset) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} \
    __asm {pushad} \
    __asm {mov ebp,esp} \
    __asm {sub esp,528} \
    __asm {and esp,0FFFFFFF0h} \
    __asm {fxsave [esp]} \
    __asm {cld} \
    __asm {push ebp} \
    __asm {push site} \
    __asm {call DiagSPrepareScratch} \
    __asm {add esp,8} \
    __asm {fxrstor [esp]} \
    __asm {mov esp,ebp} \
    __asm {popad} \
    __asm {popfd} \
    __asm {jmp dword ptr [g_sScratchTramps+offset]} \
}
REUSE_SCRATCH_BRIDGE(DiagSScratchBridge,0,0)
REUSE_SCRATCH_BRIDGE(DiagSScratchSecondBridge,1,4)
#undef REUSE_SCRATCH_BRIDGE
extern "C" __declspec(naked) void DiagSReconcileBridge(){
    __asm {pushfd}
    __asm {pushad}
    __asm {mov ebp,esp}
    __asm {sub esp,528}
    __asm {and esp,0FFFFFFF0h}
    __asm {fxsave [esp]}
    __asm {cld}
    __asm {push dword ptr [ebp+18h]}
    __asm {call DiagSPrepareSpool0}
    __asm {add esp,4}
    __asm {fxrstor [esp]}
    __asm {mov esp,ebp}
    __asm {popad}
    __asm {popfd}
    __asm {jmp dword ptr [g_sTramp]}
}

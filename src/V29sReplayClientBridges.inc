extern "C" UPtr g_scTargets[3]={};
extern "C" UPtr g_scGateReturn=0;
// Preserve input self separately from native output ECX. Observe only their
// return to the original gate. The normal-clear path needs no C++/FXSAVE.
#define SC_BRIDGE(N,K) extern "C" __declspec(naked) void N(){ \
    __asm { push ecx } __asm { call dword ptr [g_scTargets+K*4] } \
    __asm { pushfd } __asm { push eax } __asm { mov eax,g_scGateReturn } \
    __asm { cmp dword ptr [esp+12],eax } __asm { pop eax } __asm { jne done } \
    __asm { test al,al } __asm { jnz observe } \
    __asm { cmp dword ptr [g_scOpenCount],0 } __asm { jne observe } \
    __asm { lock inc dword ptr [g_scFastClear] } __asm { jmp done } \
observe: __asm { pushad } __asm { mov ebp,esp } \
    __asm { and esp,0FFFFFFF0h } __asm { sub esp,200h } __asm { fxsave [esp] } __asm { cld } \
    __asm { push K } __asm { push ebp } __asm { call SCObserve } __asm { add esp,8 } \
    __asm { fxrstor [esp] } __asm { mov esp,ebp } __asm { popad } \
done: __asm { popfd } __asm { lea esp,[esp+4] } __asm { ret } }
SC_BRIDGE(SCBridgeSamples,0)
SC_BRIDGE(SCBridgeVehicle,1)
SC_BRIDGE(SCBridgePlayer,2)
#undef SC_BRIDGE

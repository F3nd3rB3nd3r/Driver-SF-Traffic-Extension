extern "C" UPtr g_rhTargets[5]={};
// Save flags, every GPR and the complete FXSAVE state around each observer.
// Duplicate only native stack arguments; original callee and bridge each
// clean their own argument copies. Observers never alter the native result.
#define RH_OBSERVE(K,F) __asm { pushfd } __asm { pushad } \
    __asm { mov ebp,esp } __asm { and esp,0FFFFFFF0h } \
    __asm { sub esp,200h } __asm { fxsave [esp] } __asm { cld } \
    __asm { push K } __asm { push ebp } __asm { call F } __asm { add esp,8 } \
    __asm { fxrstor [esp] } __asm { mov esp,ebp } __asm { popad } __asm { popfd }
#define RH_BRIDGE0(N,K) extern "C" __declspec(naked) void N(){ \
    RH_OBSERVE(K,RHEnter) __asm { call dword ptr [g_rhTargets+K*4] } RH_OBSERVE(K,RHLeave) __asm { ret } }
RH_BRIDGE0(RHBridge20,0)
RH_BRIDGE0(RHBridge24,1)
RH_BRIDGE0(RHBridgeStep,2)
extern "C" __declspec(naked) void RHBridgeSeek(){
    RH_OBSERVE(3,RHEnter)
    __asm { push dword ptr [esp+8] }
    __asm { push dword ptr [esp+8] }
    __asm { call dword ptr [g_rhTargets+12] }
    RH_OBSERVE(3,RHLeave)
    __asm { ret 8 }
}
extern "C" __declspec(naked) void RHBridgeStream(){
    RH_OBSERVE(4,RHEnter)
    __asm { push dword ptr [esp+4] }
    __asm { call dword ptr [g_rhTargets+16] }
    RH_OBSERVE(4,RHLeave)
    __asm { ret 4 }
}
#undef RH_BRIDGE0
#undef RH_OBSERVE

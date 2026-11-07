#define AUDIO_MODEL_BRIDGE(name,ordinal) \
extern "C" __declspec(naked) void name() { \
    __asm { pushfd } \
    __asm { pushad } \
    __asm { mov ebp,esp } \
    __asm { sub esp,528 } \
    __asm { and esp,0FFFFFFF0h } \
    __asm { fxsave [esp] } \
    __asm { cld } \
    __asm { push ebp } \
    __asm { push ordinal } \
    __asm { call AudioModelEntry } \
    __asm { add esp,8 } \
    __asm { fxrstor [esp] } \
    __asm { mov esp,ebp } \
    __asm { popad } \
    __asm { popfd } \
    __asm { jmp dword ptr [g_audioModelTramps+ordinal*4] } \
}
// Relative native branches at these two sites are implemented by explicit,
// per-call continuations, with the original CMP/TEST flag semantics preserved.
#define AUDIO_MODEL_BRANCH(name,ordinal) \
extern "C" __declspec(naked) void name() { \
    __asm { pushfd } \
    __asm { pushad } \
    __asm { mov ebp,esp } \
    __asm { sub esp,528 } \
    __asm { and esp,0FFFFFFF0h } \
    __asm { fxsave [esp] } \
    __asm { cld } \
    __asm { push ebp } \
    __asm { push ordinal } \
    __asm { call AudioModelEntry } \
    __asm { add esp,8 } \
    __asm { fxrstor [esp] } \
    __asm { mov esp,ebp } \
    __asm { popad } \
    __asm { popfd } \
    __asm { jmp dword ptr [esp-24] } \
}
AUDIO_MODEL_BRANCH(AudioModelGateBridge,0)
AUDIO_MODEL_BRIDGE(AudioModelStoreBridge,1)
AUDIO_MODEL_BRIDGE(AudioModelRemoveBridge,2)
AUDIO_MODEL_BRIDGE(AudioModelLookupBridge,3)
AUDIO_MODEL_BRIDGE(AudioModelSetupBridge,4)
AUDIO_MODEL_BRANCH(AudioModelBankBridge,5)
AUDIO_MODEL_BRIDGE(AudioModelSuccessBridge,6)
AUDIO_MODEL_BRIDGE(AudioModelFailureBridge,7)
#undef AUDIO_MODEL_BRANCH
#undef AUDIO_MODEL_BRIDGE

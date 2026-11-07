// Pushad frame: EDI0, ESI4, EBP8, original ESP12, EBX16, EDX20,
// ECX24, EAX28, flags32; native stack starts36. No argument-frame mutation.
#define AUDIO_REASON_BRIDGE(name,ordinal) \
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
    __asm { call AudioReasonEntry } \
    __asm { add esp,8 } \
    __asm { fxrstor [esp] } \
    __asm { mov esp,ebp } \
    __asm { popad } \
    __asm { popfd } \
    __asm { jmp dword ptr [g_audioReasonTramps+ordinal*4] } \
}
AUDIO_REASON_BRIDGE(AudioReasonLookupBridge,0)
AUDIO_REASON_BRIDGE(AudioReasonCreateBridge,1)
AUDIO_REASON_BRIDGE(AudioReasonGateBridge,2)
AUDIO_REASON_BRIDGE(AudioReasonResultBridge,3)
AUDIO_REASON_BRIDGE(AudioReasonDeliveredBridge,4)
AUDIO_REASON_BRIDGE(AudioReasonHandledBridge,5)
AUDIO_REASON_BRIDGE(AudioReasonRemoveBridge,6)
AUDIO_REASON_BRIDGE(AudioReasonNameBridge,7)
AUDIO_REASON_BRIDGE(AudioReasonNameDefaultBridge,8)
AUDIO_REASON_BRIDGE(AudioReasonRegisterBridge,9)
#undef AUDIO_REASON_BRIDGE

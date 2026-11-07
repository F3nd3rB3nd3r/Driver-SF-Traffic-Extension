// All stolen spans are complete instructions without relative control flow.
// Save GP registers, flags, x87 and SSE; helpers never change native decisions.
#define AUDIO_SLOTS_BRIDGE(name,ordinal,reg) \
extern "C" __declspec(naked) void name() { \
    __asm { pushfd } \
    __asm { pushad } \
    __asm { mov ebp,esp } \
    __asm { sub esp,528 } \
    __asm { and esp,0FFFFFFF0h } \
    __asm { fxsave [esp] } \
    __asm { cld } \
    __asm { push reg } \
    __asm { push ordinal } \
    __asm { call AudioSlotsEntry } \
    __asm { add esp,8 } \
    __asm { fxrstor [esp] } \
    __asm { mov esp,ebp } \
    __asm { popad } \
    __asm { popfd } \
    __asm { jmp dword ptr [g_audioSlotsTramps+ordinal*4] } \
}
AUDIO_SLOTS_BRIDGE(AudioSlotsAGateBridge,0,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsBGateBridge,1,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsAInsertBridge,2,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsBInsertBridge,3,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsAReuseBridge,4,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsEightBridge,5,ebx)
AUDIO_SLOTS_BRIDGE(AudioSlotsByteBridge,6,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsAUpdateBridge,7,edi)
AUDIO_SLOTS_BRIDGE(AudioSlotsBUpdateBridge,8,edi)
#undef AUDIO_SLOTS_BRIDGE

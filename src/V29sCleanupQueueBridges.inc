#define CLEANUP_QUEUE_BRIDGE(name,ordinal) \
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
    __asm { call CleanupEntry } \
    __asm { add esp,8 } \
    __asm { fxrstor [esp] } \
    __asm { mov esp,ebp } \
    __asm { popad } \
    __asm { popfd } \
    __asm { jmp dword ptr [g_cleanupTramps+ordinal*4] } \
}
CLEANUP_QUEUE_BRIDGE(CleanupDirectBridge,0)
CLEANUP_QUEUE_BRIDGE(CleanupReplaceBridge,1)
CLEANUP_QUEUE_BRIDGE(CleanupReleaseBridge,2)
CLEANUP_QUEUE_BRIDGE(CleanupBeginBridge,3)
CLEANUP_QUEUE_BRIDGE(CleanupEndBridge,4)
CLEANUP_QUEUE_BRIDGE(CleanupEmptyBridge,5)
#undef CLEANUP_QUEUE_BRIDGE

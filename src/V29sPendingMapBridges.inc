#define PENDING_BRIDGE(name,ordinal) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} \
    __asm {pushad} \
    __asm {mov ebp,esp} \
    __asm {sub esp,528} \
    __asm {and esp,0FFFFFFF0h} \
    __asm {fxsave [esp]} \
    __asm {cld} \
    __asm {push ebp} \
    __asm {push ordinal} \
    __asm {call PendingEntry} \
    __asm {add esp,8} \
    __asm {fxrstor [esp]} \
    __asm {mov esp,ebp} \
    __asm {popad} \
    __asm {popfd} \
    __asm {jmp dword ptr [g_pendingTramps+ordinal*4]} \
}
PENDING_BRIDGE(PendingBeginBridge,0)
PENDING_BRIDGE(PendingEndBridge,1)
#undef PENDING_BRIDGE

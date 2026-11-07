extern "C" UPtr g_callbackTramps[3];
extern "C" __declspec(naked) void CallbackWriterBridge() {
    __asm {pushfd}
    __asm {pushad}
    __asm {mov ebp,esp}
    __asm {sub esp,528}
    __asm {and esp,0FFFFFFF0h}
    __asm {fxsave [esp]}
    __asm {cld}
    __asm {push dword ptr [ebp+4]} // savedESI = handler, before selector arithmetic
    __asm {call CallbackWriterCheck}
    __asm {add esp,4}
    __asm {fxrstor [esp]}
    __asm {mov esp,ebp}
    __asm {popad}
    __asm {popfd}
    __asm {jmp dword ptr [g_callbackTramps+4]}
}

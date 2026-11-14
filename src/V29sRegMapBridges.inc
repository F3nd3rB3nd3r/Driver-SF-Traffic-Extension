extern "C" UPtr g_regMapInsertTrampoline=0;
extern "C" __declspec(naked) void DiagSRegMapBridge(){__asm {
    pushfd
    pushad
    mov ebp,esp
    and esp,0FFFFFFF0h
    sub esp,200h
    fxsave [esp]
    cld
    push ecx
    call DiagSPrepareRegMap
    add esp,4
    fxrstor [esp]
    mov esp,ebp
    popad
    popfd
    jmp dword ptr [g_regMapInsertTrampoline]
}}

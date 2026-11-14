extern "C" UPtr g_pauseCompletionTarget=0;
// Recreate the native step's saved-register frame, then enter its unchanged
// completion tail. That tail restores EDI/ESI/ECX and returns to our caller.
extern "C" __declspec(naked) void __fastcall PauseCompleteNative(void*,void*) {__asm {
    push ecx
    push esi
    mov esi,ecx
    push edi
    jmp dword ptr [g_pauseCompletionTarget]
}}

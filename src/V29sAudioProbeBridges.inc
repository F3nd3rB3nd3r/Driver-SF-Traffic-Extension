// Shared verbatim with the isolated Win32 ABI test. Pushad offsets:
// EDI=0, ESI=4, ECX=24, EAX=28, flags=32, original stack starts at36.
extern "C" __declspec(naked) void AudioProbeBankBridge() {
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+52] // cookie, NOT result
        push dword ptr [ebp+48] // signed pool ID
        push dword ptr [ebp+44] // actual AKRESULT
        push dword ptr [ebp+40] // bank ID
        call AudioProbeBankEntry
        add esp,16
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        jmp dword ptr [g_audioProbeTramps]
    }
}
extern "C" __declspec(naked) void AudioProbeEmitterBridge() {
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+4] // ESI owner; native function already has locals
        call AudioProbeEmitterEntry
        add esp,4
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        jmp dword ptr [g_audioProbeTramps+4]
    }
}
extern "C" __declspec(naked) void AudioProbeMallocBridge() {
    __asm {
        pushfd
        test edi,edi
        jnz ap_malloc_ok
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push 0 // no alignment argument on Malloc
        push dword ptr [ebp+48] // original caller
        push dword ptr [ebp+56] // bytes
        push dword ptr [ebp+52] // pool ID
        push 0 // Malloc
        call AudioProbeAllocEntry
        add esp,20
        fxrstor [esp]
        mov esp,ebp
        popad
    ap_malloc_ok:
        popfd
        jmp dword ptr [g_audioProbeTramps+8]
    }
}
extern "C" __declspec(naked) void AudioProbeMalignBridge() {
    __asm {
        pushfd
        test edi,edi
        jnz ap_malign_ok
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+60] // alignment
        push dword ptr [ebp+48] // original caller
        push dword ptr [ebp+56] // bytes
        push dword ptr [ebp+52] // pool ID
        push 1 // Malign
        call AudioProbeAllocEntry
        add esp,20
        fxrstor [esp]
        mov esp,ebp
        popad
    ap_malign_ok:
        popfd
        jmp dword ptr [g_audioProbeTramps+12]
    }
}
extern "C" __declspec(naked) void AudioProbePostBridge() {
    __asm {
        pushfd
        lock inc dword ptr [g_audioProbePostNormal]
        test esi,esi
        jnz ap_post_ok
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+64] // caller
        push dword ptr [ebp+72] // game object ID
        push dword ptr [ebp+68] // event ID
        push 0 // normal zero
        call AudioProbePostEntry
        add esp,16
        fxrstor [esp]
        mov esp,ebp
        popad
    ap_post_ok:
        popfd
        jmp dword ptr [g_audioProbeTramps+16]
    }
}
extern "C" __declspec(naked) void AudioProbePostEarlyBridge() {
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        cld
        push dword ptr [ebp+60] // caller, no native saved ESI on this path
        push dword ptr [ebp+68] // game object ID
        push dword ptr [ebp+64] // event ID
        push 1 // early external-source allocation failure
        call AudioProbePostEntry
        add esp,16
        fxrstor [esp]
        mov esp,ebp
        popad
        popfd
        jmp dword ptr [g_audioProbeTramps+20]
    }
}

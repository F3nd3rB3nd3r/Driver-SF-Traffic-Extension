// ESI identifies the native reader at all six observation sites. Each bridge
// restores inputs before reproducing the displaced native instruction/branch.
#define RF_SAVE __asm { pushfd } __asm { pushad } __asm { mov ebx,esp } __asm { and esp,0FFFFFFF0h } __asm { sub esp,200h } __asm { fxsave [esp] } __asm { cld }
#define RF_ARGS(K,V,R) __asm { push R } __asm { push V } __asm { push K } __asm { push esi } __asm { call RFObserve } __asm { add esp,10h }
#define RF_RESTORE __asm { fxrstor [esp] } __asm { mov esp,ebx } __asm { popad } __asm { popfd }
extern "C" __declspec(naked) void RFBridgePoll(){RF_SAVE RF_ARGS(0,eax,0) RF_RESTORE __asm { test al,al } __asm { jnz busy } __asm { jmp dword ptr [g_rfContinue] } busy: __asm { jmp dword ptr [g_rfStop] }}
extern "C" __declspec(naked) void RFBridgeCommit(){RF_SAVE RF_ARGS(1,0,0) RF_RESTORE __asm { mov dword ptr [esi+8568h],eax } __asm { jmp dword ptr [g_rfContinue+4] }}
extern "C" __declspec(naked) void RFBridgeWrap(){RF_SAVE RF_ARGS(2,eax,ebp) RF_RESTORE __asm { cmp eax,ebp } __asm { jb blocked } __asm { jmp dword ptr [g_rfContinue+8] } blocked: __asm { jmp dword ptr [g_rfStop+4] }}
extern "C" __declspec(naked) void RFBridgeSpace(){RF_SAVE RF_ARGS(3,eax,8000h) RF_RESTORE __asm { cmp eax,8000h } __asm { jbe blocked } __asm { jmp dword ptr [g_rfContinue+12] } blocked: __asm { jmp dword ptr [g_rfStop+8] }}
extern "C" __declspec(naked) void RFBridgeIssue(){RF_SAVE RF_ARGS(4,eax,0) RF_RESTORE __asm { mov eax,dword ptr [esi+8584h] } __asm { jmp dword ptr [g_rfContinue+16] }}
extern "C" __declspec(naked) void RFBridgeEnd(){RF_SAVE RF_ARGS(5,0,0) RF_RESTORE __asm { cmp byte ptr [esi+858Ch],0 } __asm { jne ended } __asm { jmp dword ptr [g_rfContinue+20] } ended: __asm { jmp dword ptr [g_rfStop+12] }}
#undef RF_RESTORE
#undef RF_ARGS
#undef RF_SAVE

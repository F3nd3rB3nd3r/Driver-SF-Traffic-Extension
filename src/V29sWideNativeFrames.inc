extern "C" U32 g_wideRequestFrame=0,g_wideNotifyFrame=0;
extern "C" UPtr g_wideRequestResume=0,g_wideNotifyResume=0;
static bool g_wideRequestProbeInstalled=false;
static U8 g_wideRequestSub[6]={};
static bool g_wideCollectionGuardInstalled=false;
static U8 g_wideCollectionStore[7]={};
extern "C" UPtr g_wideCollectionResume=0;
extern "C" U32 g_wideCollectionLimit=0;
static void WideCollectionFull(){WideAllocationFailure("Unlimiter: ResourceCollectionEntries exhausted before native append.\n");}
extern "C" __declspec(naked) void WideCollectionBridge(){__asm {
    pushfd
    cmp edi,dword ptr [g_wideCollectionLimit]
    jae wide_collection_full
    popfd
    push eax
    mov eax,dword ptr [g_sResourcePairs]
    mov dword ptr [eax+edi*8],edx
    pop eax
    jmp dword ptr [g_wideCollectionResume]
wide_collection_full:
    call WideCollectionFull
    int 3
}}
extern "C" __declspec(naked) void WideRequestFrameBridge(){__asm {
    push eax
    mov eax,dword ptr [g_wideRequestFrame]
    call WideProbeStack
    pop eax
    sub esp,dword ptr [g_wideRequestFrame]
    jmp dword ptr [g_wideRequestResume]
}}
extern "C" __declspec(naked) void WideNotifyFrameBridge(){__asm {
    push eax
    mov eax,dword ptr [g_wideNotifyFrame]
    call WideProbeStack
    pop eax
    sub esp,dword ptr [g_wideNotifyFrame]
    jmp dword ptr [g_wideNotifyResume]
}}
static bool WideNativeJump(UPtr at,UPtr bridge){
    DWORD old=0,unused=0;if(!VirtualProtect(reinterpret_cast<void*>(at),6u,kPageExecuteReadWrite,&old))return false;
    *reinterpret_cast<U8*>(at)=0xE9;Write32(at+1u,bridge-at-5u);*reinterpret_cast<U8*>(at+5u)=0x90;
    const bool exact=Read8(at)==0xE9 && Read32(at+1u)==bridge-at-5u && Read8(at+5u)==0x90;
    const bool flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),6u)!=0;
    const bool protect=VirtualProtect(reinterpret_cast<void*>(at),6u,old,&unused)!=0;
    return exact && flush && protect;
}
static bool WideInstallRequestProbe(){
    const UPtr at=Address(0x5A7495u);if(Read8(at)!=0x81 || Read8(at+1u)!=0xEC || Read32(at+2u)!=g_wideRequestFrame)return false;
    for(U32 i=0;i<6u;++i)g_wideRequestSub[i]=Read8(at+i);
    g_wideRequestResume=at+6u;g_wideRequestProbeInstalled=true;
    return WideNativeJump(at,reinterpret_cast<UPtr>(&WideRequestFrameBridge));
}
static void WideRestoreRequestProbe(){
    if(!g_wideRequestProbeInstalled)return;const UPtr at=Address(0x5A7495u);DWORD old=0,unused=0;
    if(!VirtualProtect(reinterpret_cast<void*>(at),6u,kPageExecuteReadWrite,&old))WideAllocationFailure("Unlimiter: request-probe rollback failed.\n");
    for(U32 i=0;i<6u;++i)*reinterpret_cast<U8*>(at+i)=g_wideRequestSub[i];
    const bool flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),6u)!=0;
    const bool protect=VirtualProtect(reinterpret_cast<void*>(at),6u,old,&unused)!=0;
    if(!flush || !protect)WideAllocationFailure("Unlimiter: request-probe rollback protection failed.\n");g_wideRequestProbeInstalled=false;
}
static bool WideInstallNotifyProbe(UPtr callback){
    const UPtr at=callback+5u;if(Read8(at)!=0x81 || Read8(at+1u)!=0xEC || Read32(at+2u)!=GenericNotifyFrame())return false;
    g_wideNotifyFrame=GenericNotifyFrame();g_wideNotifyResume=at+6u;
    return WideNativeJump(at,reinterpret_cast<UPtr>(&WideNotifyFrameBridge));
}
static bool WideInstallCollectionGuard(){
    const UPtr at=Address(0x6A593Fu);if(Read8(at)!=0x89 || Read8(at+1u)!=0x14 || Read8(at+2u)!=0xFD || Read32(at+3u)!=reinterpret_cast<UPtr>(g_sResourcePairs))return false;
    for(U32 i=0;i<7u;++i)g_wideCollectionStore[i]=Read8(at+i);g_wideCollectionResume=at+7u;g_wideCollectionLimit=g_limits.collection;
    g_wideCollectionGuardInstalled=true;
    if(!WideNativeJump(at,reinterpret_cast<UPtr>(&WideCollectionBridge)))return false;
    // Seventh byte is unreachable; preserve it for exact rollback.
    g_wideCollectionGuardInstalled=true;return true;
}
static void WideRestoreCollectionGuard(){
    if(!g_wideCollectionGuardInstalled)return;const UPtr at=Address(0x6A593Fu);DWORD old=0,unused=0;
    if(!VirtualProtect(reinterpret_cast<void*>(at),7u,kPageExecuteReadWrite,&old))WideAllocationFailure("Unlimiter: collection guard rollback failed.\n");
    for(U32 i=0;i<7u;++i)*reinterpret_cast<U8*>(at+i)=g_wideCollectionStore[i];
    const bool flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),7u)!=0;
    const bool protect=VirtualProtect(reinterpret_cast<void*>(at),7u,old,&unused)!=0;
    if(!flush || !protect)WideAllocationFailure("Unlimiter: collection guard rollback protection failed.\n");g_wideCollectionGuardInstalled=false;
}

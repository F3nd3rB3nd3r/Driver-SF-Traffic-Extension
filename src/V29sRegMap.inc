#include "V29sRegMapNative.inc"
#include "V29sRegMapCore.inc"
#include "V29sRegMapLayout.inc"
#include "V29sRegMapBridges.inc"
static volatile long g_sRegMapHook=0;
static UPtr RegMapAddress(U32 at){return Address(at);}
static void* RegMapAllocate(U32 n){return VirtualAlloc(nullptr,n,kMemCommitReserve,kPageReadWrite);}
static RegMapPatch g_regMapPatches[]={
    {0x86BFA0u,5u,sizeof(kRegMapCtorNative),kRegMapCtorNative,reinterpret_cast<UPtr>(&RegMapConstruct),0u,0u,false},
    {0x86BE5Fu,8u,8u,kRegMapInsertNative+0x4Fu,reinterpret_cast<UPtr>(&DiagSRegMapBridge),0u,0u,false}
};
static DWORD g_regMapPageProtection=0;
static bool RegMapInstallRefused(const char*,UPtr=0u);
static UPtr RegMapRunConstructor(UPtr owner){typedef UPtr (__thiscall* Fn)(void*);return reinterpret_cast<Fn>(g_regMapPatches[0].trampoline)(reinterpret_cast<void*>(owner));}
static bool RegMapDependencies(){
    return RegMapEqual(Address(0x86BE10u),kRegMapInsertNative,0x4Fu) &&
        RegMapEqual(Address(0x86BE67u),kRegMapInsertNative+0x57u,sizeof(kRegMapInsertNative)-0x57u) &&
        RegMapEqual(Address(0x86BDC0u),kRegMapLookupNative,sizeof(kRegMapLookupNative)) &&
        RegMapEqual(Address(0x86BE90u),kRegMapRemoveNative,sizeof(kRegMapRemoveNative)) &&
        RegMapEqual(Address(0x8CCAA0u),kRegMapAllocateNative,sizeof(kRegMapAllocateNative));
}
static void RegMapPatchFatal(const char* reason){RegMapDetail d={};d.reason=reason;RegMapFatal(d);}
static bool RegMapSitesMatch(bool on){return RegMapMatches(g_regMapPatches[0],on) && RegMapMatches(g_regMapPatches[1],on) && RegMapDependencies();}
static void DiagSRegMapRestore(){
    if(!g_regMapPatches[0].installed && !g_regMapPatches[1].installed){g_sRegMapHook=0;return;}
    DWORD old=0,unused=0;const UPtr page=Address(0x86B000u);
    if(!RegMapSitesMatch(true) || !VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&old)){RegMapPatchFatal("rollbackPreflight");return;}
    for(U32 i=0;i<2u;++i)RegMapPatchBytes(g_regMapPatches[i],false);
    const bool exact=RegMapSitesMatch(false);const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
    const BOOL protect=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,g_regMapPageProtection,&unused);
    if(!exact || !flush || !protect){RegMapPatchFatal("rollbackIncomplete");return;}
    for(U32 i=0;i<2u;++i)g_regMapPatches[i].installed=false;g_sRegMapHook=0;
}
static bool DiagSRegMapInstall(){
    if(DSF150_Status!=1)return RegMapInstallRefused("parentStatus");
    if(!VerifyExecutableIdentity())return RegMapInstallRefused("imageIdentity");
    if(g_pkg0)return RegMapInstallRefused("activeOwner");
    if(Read32(Address(0x144BCFCu))!=0u)return RegMapInstallRefused("definitionAllocatorInitialized");
    if(!RegMapSitesMatch(false))return RegMapInstallRefused("stockSignatures");
    const UPtr mem=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,kMemCommitReserve,kPageReadWrite));if(!mem)return RegMapInstallRefused("trampolineAllocation");
    for(U32 i=0;i<2u;++i){g_regMapPatches[i].trampoline=mem+i*0x100u;RegMapFillTrampoline(g_regMapPatches[i]);}
    g_regMapInsertTrampoline=g_regMapPatches[1].trampoline;
    DWORD unused=0;if(!VirtualProtect(reinterpret_cast<void*>(mem),0x1000u,0x20u,&unused))return RegMapInstallRefused("trampolineProtection",mem);
    if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(mem),0x1000u))return RegMapInstallRefused("trampolineFlush",mem);
    const UPtr page=Address(0x86B000u);
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&g_regMapPageProtection))return RegMapInstallRefused("pageUnlock",page);
    if(g_pkg0 || Read32(Address(0x144BCFCu))!=0u || !RegMapSitesMatch(false)){
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,g_regMapPageProtection,&unused))RegMapPatchFatal("precommitProtection");return RegMapInstallRefused("precommitRecheck",page);
    }
    for(U32 i=0;i<2u;++i){RegMapPatchBytes(g_regMapPatches[i],true);g_regMapPatches[i].installed=true;}
    const bool exact=RegMapSitesMatch(true);const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
    const BOOL protect=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,g_regMapPageProtection,&unused);
    if(!exact || !flush || !protect){DiagSRegMapRestore();return RegMapInstallRefused(!exact?"commitVerify":(!flush?"pageFlush":"pageProtection"),page);}
    return true;
}

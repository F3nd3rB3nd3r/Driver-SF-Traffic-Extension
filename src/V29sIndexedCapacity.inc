// Isolated native indexed-reference table experiment; NOT the pending arrays.
// Install only into untouched pre-constructor state. No live migration/unload.
#if !defined(DSF_V29S_CAPACITY_TRACE) || !defined(DSF_V29S_PERF_CONTROL) || !defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
#error Indexed capacity requires the quiet capacity diagnostic and pending-array redirect
#endif
#if defined(DSF_V29S_HANDLE_CHAIN_TRACE) || defined(DSF_V29S_HASH_HEAD_TRACE)
#error Old hot diagnostics assume the original indexed table
#endif
static_assert(DSF_CHAPTER7_PRELOAD_COUNT==150 && DSF_GRAPHICS_ADMISSION_TARGET==160,
              "Preserve the 150 request / 160 vehicle-slot comparison");
#if defined(DSF_V29S_WIDE_LIMITS)
static const U32 kIndexedNativeCount=131072u;
#define kIndexedTestCount (g_limits.indexed)
#else
static const U32 kIndexedNativeCount=131072u,kIndexedTestCount=262144u;
#endif
static UPtr g_indexedBase=0u;
// 0=not attempted, 1=armed before native constructor, 2=external head observed.
// Negative values mean refusal/rollback, not a successful capacity run.
static long g_indexedStatus=0;
static const char* g_indexedReason="notAttempted";
static U32 g_indexedPatchCount=0u;
static bool g_indexedStartupVerified=false;
static void CapacityWrite(const char* line);
static void IndexedFatal(const char* message,U32 code) {
    CapacityWrite(message);Log(message);ExitProcess(code);
}
struct IndexedPatch { UPtr site; U8 offset,length,stock[7],kind; };
// kind: 0=base displacement, 1=base+4, 2=last slot, 3=N-1, 4=N.
static const IndexedPatch g_indexedPatches[]={
    {0x00BAE86Eu,2,6,{0x81,0xE9,0x94,0xE0,0x17,0x00},0},
    {0x00BAE8B3u,3,7,{0x89,0x94,0xC6,0x94,0xE0,0x17,0x00},0},
    {0x00BAE968u,2,6,{0x8D,0xBE,0x94,0xE0,0x17,0x00},0},
    {0x00BAEB5Du,3,7,{0x8B,0x8C,0xFE,0x94,0xE0,0x17,0x00},0},
    {0x00BAEB64u,3,7,{0x8D,0x9C,0xFE,0x94,0xE0,0x17,0x00},0},
    {0x00BAEB80u,3,7,{0x8D,0xAC,0xC6,0x94,0xE0,0x17,0x00},0},
    {0x00BAEBA3u,3,7,{0x8D,0x84,0xC6,0x94,0xE0,0x17,0x00},0},
    {0x00BAED0Fu,3,7,{0x8D,0x84,0xC7,0x94,0xE0,0x17,0x00},0},
    {0x00BAEC7Fu,3,7,{0x8B,0xB4,0xC7,0x98,0xE0,0x17,0x00},1},
    {0x00BAED08u,3,7,{0x8B,0x8C,0xC7,0x98,0xE0,0x17,0x00},1},
    {0x00BAE9A3u,2,6,{0x89,0x96,0x8C,0xE0,0x27,0x00},2},
    {0x00BAE984u,1,5,{0xBA,0xFF,0xFF,0x01,0x00},3},
    {0x00BAE8A9u,1,5,{0x3D,0x00,0x00,0x02,0x00},4}
};
static_assert(sizeof(g_indexedPatches)/sizeof(g_indexedPatches[0])==13u,"Relocation inventory");
static U32 IndexedValue(const IndexedPatch& p,UPtr base) {
    const U32 displacement=base-Address(0x01732BF0u); // x86 modulo-32 arithmetic
    if(p.kind==1u)return displacement+4u;
    if(p.kind==2u)return displacement+8u*(kIndexedTestCount-1u);
    if(p.kind==3u)return kIndexedTestCount-1u;
    if(p.kind==4u)return kIndexedTestCount;
    return displacement;
}
static bool IndexedInstructionsMatch(bool patched,UPtr base) {
    for(const IndexedPatch& p:g_indexedPatches) {
        const UPtr site=Address(p.site);
        if(!Readable(site,p.length))return false;
        const U32 value=IndexedValue(p,base);
        for(U32 j=0u;j<p.length;++j) {
            const U8 expected=patched && j>=p.offset && j<p.offset+4u
                ? static_cast<U8>(value>>(8u*(j-p.offset))) : p.stock[j];
            if(*reinterpret_cast<volatile const U8*>(site+j)!=expected)return false;
        }
    }
    return true;
}
static bool IndexedCold() {
    const UPtr r=Address(0x01732BF0u),slots=r+0x17E094u;
    if(!Readable(slots,kIndexedNativeCount*8u) || !Readable(r+0x27E094u,4u) ||
       !Readable(r+0x127AC4u,4u) || !Readable(r+0x157AC8u,4u) ||
       !Readable(r+0x17E090u,4u) || !Readable(r+0x15E088u,8u) ||
       !Readable(Address(kRootPtr),4u) || !Readable(Address(kGraphicsManager)+0x348u,8u))return false;
    if(Read32(r+0x27E094u) || Read32(r+0x127AC4u) || Read32(r+0x157AC8u) ||
       Read32(r+0x17E090u) || Read32(r+0x15E088u) || Read32(r+0x15E08Cu) ||
       Read32(Address(kRootPtr)) || Read32(Address(kGraphicsManager)+0x348u))return false;
    // A complete zero scan rejects both initialized all-free and active tables.
    // No initialized-manager migration is attempted by this experiment.
    for(U32 i=0u;i<kIndexedNativeCount*2u;++i)if(Read32(slots+i*4u))return false;
    return true;
}
static bool IndexedPendingRedirectIntact() {
    if(g_sResourcePatchCount!=31u)return false;
    for(const ResourceArrayPatch& p:g_sResourceArrayPatches){
#if defined(DSF_V29S_WIDE_LIMITS)
        if(p.site==0x6A593Fu){if(!g_wideCollectionGuardInstalled || Read8(Address(p.site))!=0xE9u)return false;continue;}
#endif
        if(!p.done || Read32(Address(p.site)+p.offset)!=ResourceArrayReplacement(p))return false;}
    return true;
}
static bool IndexedRefuse(long status,const char* reason,UPtr allocation=0u) {
    g_indexedStatus=status;g_indexedReason=reason;
    if(allocation)VirtualFree(reinterpret_cast<void*>(allocation),0u,0x8000u);
    return false;
}
static bool IndexedInstall() {
    if(!VerifyIndexedStartup(g_indexedStockVerified,reinterpret_cast<UPtr>(&HandlerInitBridge),
        reinterpret_cast<UPtr>(g_handlerTrampoline),reinterpret_cast<UPtr>(g_originalHandlerInit),
        DSFV27B_HookInstalled))return IndexedRefuse(-1,"preflightIdentityOrOwnedHandlerHook");
    g_indexedStartupVerified=true;
    if(!IndexedPendingRedirectIntact())return IndexedRefuse(-9,"pendingRedirect");
    if(!IndexedInstructionsMatch(false,0u))return IndexedRefuse(-3,"instructionSignature");
    if(!IndexedCold())return IndexedRefuse(-2,"managerNotUntouchedCold");
    const UPtr base=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,kIndexedTestCount*8u,
        kMemCommitReserve,kPageReadWrite));
    if(!base)return IndexedRefuse(-4,"allocation");
    // All 13 instructions occupy ONE page: protect it once, then install or
    // roll back the entire set before returning to native startup code.
    const UPtr page=Address(0x00BAE000u);
    DWORD old=0u,discarded=0u;
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&old))
        return IndexedRefuse(-5,"unlockCodePage",base);
    if(!IndexedCold() || !IndexedInstructionsMatch(false,0u)) {
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded)) {
            IndexedFatal("DSFV29s_INDEXED_FATAL protection restore failed during preflight\n",0x29C1u);
        }
        return IndexedRefuse(-2,"startupChangedDuringPreflight",base);
    }
    for(const IndexedPatch& p:g_indexedPatches)Write32(Address(p.site)+p.offset,IndexedValue(p,base));
    const bool exact=IndexedInstructionsMatch(true,base);
    const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u)!=0;
    // Do not short circuit protection restoration when a prior check failed.
    const bool protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded)!=0;
    if(!exact || !flushed || !protectedAgain) {
        DWORD scratch=0u;
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&scratch)) {
            IndexedFatal("DSFV29s_INDEXED_FATAL cannot unlock code for rollback\n",0x29C2u);
        }
        for(const IndexedPatch& p:g_indexedPatches)
            for(U32 j=p.offset;j<p.offset+4u;++j)
                *reinterpret_cast<volatile U8*>(Address(p.site)+j)=p.stock[j];
        const bool restored=IndexedInstructionsMatch(false,0u);
        const bool rollbackFlushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u)!=0;
        const bool rollbackProtected=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded)!=0;
        if(!restored || !rollbackFlushed || !rollbackProtected) {
            IndexedFatal("DSFV29s_INDEXED_FATAL rollback incomplete; refusing native execution\n",0x29C3u);
        }
        return IndexedRefuse(-6,"writeFlushOrProtectionRolledBack",base);
    }
    // Native 00BAE930 will initialize this zero-filled allocation itself.
    // Keep it for process lifetime. Do not rewrite the free head here.
    g_indexedBase=base;g_indexedPatchCount=13u;
    g_indexedStatus=1;g_indexedReason="armedColdNativeConstructorPending";
    return true;
}

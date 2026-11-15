// Only queue storage changes; native count, resource ownership and timing remain.
#if defined(DSF_V29S_WIDE_LIMITS)
#define kCleanupCapacity (g_limits.cleanup)
static const U32 kCleanupGuard=0x51473236u;
#else
static const U32 kCleanupCapacity=262144u,kCleanupGuard=0x51473236u;
#endif
static UPtr g_cleanupAllocation=0,g_cleanupBase=0;
static volatile long g_cleanupAttempts[3]={},g_cleanupPeak=0,g_cleanupCurrent=0;
static volatile long g_cleanupBeyondStock=0,g_cleanupOldCrossings=0,g_cleanupFull=0,g_cleanupGuardBad=0,g_cleanupInvalid=0;
static volatile long g_cleanupDrainStarts=0,g_cleanupDrainEnds=0,g_cleanupNonempty=0,g_cleanupDrained=0;
static volatile long g_cleanupDepth=0,g_cleanupDuringDrain=0,g_cleanupDrainMismatch=0;
static U32 g_cleanupExpected=0,g_cleanupStartAttempts=0;
static U32 CleanupAttempts(){return static_cast<U32>(g_cleanupAttempts[0]+g_cleanupAttempts[1]+g_cleanupAttempts[2]);}
static U32 CleanupGuards() {
    if(!g_cleanupBase)return 8u;
    U32 bad=0;for(U32 n=0;n<4u;++n) {
        if(Read32(g_cleanupBase-16u+n*4u)!=kCleanupGuard)++bad;
        if(Read32(g_cleanupBase+kCleanupCapacity*4u+n*4u)!=kCleanupGuard)++bad;
    }return bad;
}
static void CleanupEvent(const char* kind,U32 count,U32 value,U32 producer);
static bool CleanupObserve(U32 kind,U32* frame) {
    if(kind>=6u){InterlockedIncrement(&g_cleanupInvalid);return false;}
    const UPtr f=reinterpret_cast<UPtr>(frame),sp=f+36u;
    const UPtr owner=Read32(f+(kind==0u || kind==3u?24u:kind>=4u?8u:0u));
    if(owner!=Address(0x01732BF0u)){InterlockedIncrement(&g_cleanupInvalid);return false;}
    const U32 bad=CleanupGuards();if(bad){InterlockedExchange(&g_cleanupGuardBad,static_cast<long>(bad));return false;}
    const U32 count=kind<3u?Read32(f+28u):Read32(owner+0x17E090u);
    if(kind<3u) {
        InterlockedIncrement(&g_cleanupAttempts[kind]);InterlockedExchange(&g_cleanupCurrent,static_cast<long>(count));
        if(count>=kCleanupCapacity){InterlockedIncrement(&g_cleanupFull);return false;}
        CapacityMax(g_cleanupPeak,count+1u);
        if(count>=32768u)InterlockedIncrement(&g_cleanupBeyondStock);
        if(count==32768u)InterlockedIncrement(&g_cleanupOldCrossings);
        if(g_cleanupDepth)InterlockedIncrement(&g_cleanupDuringDrain);
        const U32 value=kind==0u?Read32(sp+4u):Read32(f+(kind==1u?20u:4u));
        if(count==32768u || count==65536u || count==131072u || count==kCleanupCapacity-1u)CleanupEvent("boundary",count,value,kind);
        return true;
    }
    if(count>kCleanupCapacity){InterlockedIncrement(&g_cleanupInvalid);return false;}
    if(kind==3u) {
        InterlockedIncrement(&g_cleanupDrainStarts);
        if(InterlockedIncrement(&g_cleanupDepth)!=1){InterlockedIncrement(&g_cleanupInvalid);return false;}
        g_cleanupExpected=count;g_cleanupStartAttempts=CleanupAttempts();InterlockedExchange(&g_cleanupCurrent,static_cast<long>(count));
        if(count){InterlockedIncrement(&g_cleanupNonempty);CleanupEvent("drainBegin",count,0u,kind);}return true;
    }
    InterlockedIncrement(&g_cleanupDrainEnds);if(g_cleanupDepth!=1){InterlockedIncrement(&g_cleanupInvalid);return false;}
    const U32 processed=Read32(Address(0x01732BE0u));
    if(count!=g_cleanupExpected || processed!=g_cleanupExpected || CleanupAttempts()!=g_cleanupStartAttempts)InterlockedIncrement(&g_cleanupDrainMismatch);
    if(kind==5u && Read32(f+28u)!=0u){InterlockedIncrement(&g_cleanupInvalid);return false;}
    InterlockedExchange(&g_cleanupDrained,g_cleanupDrained+static_cast<long>(processed));
    if(g_cleanupExpected)CleanupEvent("drainEnd",count,processed,kind);
    InterlockedExchange(&g_cleanupCurrent,0);InterlockedExchange(&g_cleanupDepth,0);return true;
}

// V29g passive City Region descriptor sampling at the 200 ms monitor cadence.
// Exact-build allocator initialization adds three internal nodes to +0x14.
// This raw field is allocator bookkeeping, not a resource residency count.
extern "C" __declspec(dllexport) volatile long DSFV29g_CityTransitions=0;
static U32 g_diagGArray=0,g_diagGCount=0;
static U32 g_diagGRaw[32]={},g_diagGUsed[32]={};
static bool g_diagGSeen=false,g_diagGLimitLogged=false;
static const U32 kDiagGMaxEvents=8192u;

static void DiagGCitySample() {
    const UPtr runtime=Address(kGraphicsManager)+0x0Cu+7u*0x5Cu;
    if(!Readable(runtime,8u))return;
    const U32 array=Read32(runtime),count=Read32(runtime+4u);
    // Count 18 comes from the observed V29f sessions, not a game invariant.
    if(!array || count!=18u || !Readable(array,count*0x60u)) {
        g_diagGSeen=false;g_diagGArray=0;g_diagGCount=0;return;
    }
    const bool fresh=!g_diagGSeen || array!=g_diagGArray || count!=g_diagGCount;
    const U32 tick=GetTickCount()-g_diagStartTick;
    for(U32 i=0;i<count;++i) {
        const UPtr at=array+i*0x60u;
        const U32 raw=Read32(at+0x14u),used=Read32(at+0x1Cu);
        if(fresh || raw!=g_diagGRaw[i] || used!=g_diagGUsed[i]) {
            if(static_cast<U32>(DSFV29g_CityTransitions)<kDiagGMaxEvents) {
                char line[320]={};char* c=line;const char* end=line+sizeof(line)-1;
                AppendText(c,"DSFV29g_CITY7 tick=",end);DiagFAppendU(c,tick,end);
                DiagFAppendKeyU(c," slot=",i,end);
                DiagFAppendKeyHex(c," array=",array,end);
                DiagFAppendKeyU(c," fresh=",fresh?1u:0u,end);
                DiagFAppendKeyU(c," oldRaw=",fresh?0xFFFFFFFFu:g_diagGRaw[i],end);
                DiagFAppendKeyU(c," raw=",raw,end);
                DiagFAppendKeyU(c," oldRaw1C=",fresh?0xFFFFFFFFu:g_diagGUsed[i],end);
                DiagFAppendKeyU(c," raw1C=",used,end);
                DiagFAppendKeyU(c," nodeCapacity=",Read32(at+4u),end);
                DiagFAppendKeyU(c," secondRaw=",Read32(at+0x40u),end);
                AppendText(c,"\n",end);*c=0;DiagPrint(line);
                ++DSFV29g_CityTransitions;
            } else if(!g_diagGLimitLogged) {
                DiagPrint("DSFV29g_CITY7 trace limit reached.\n");
                g_diagGLimitLogged=true;
            }
        }
        g_diagGRaw[i]=raw;g_diagGUsed[i]=used;
    }
    g_diagGSeen=true;g_diagGArray=array;g_diagGCount=count;
}

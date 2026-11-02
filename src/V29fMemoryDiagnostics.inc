// V29f observational world-memory diagnostics.
// Data-only snapshot of all 21 EXACT_GRAPHICS_POOLS rows, PSAPI process
// memory counters, guarded VirtualQuery VA walk, and two historically
// signature-checked streaming call counters. No allocation/release suppression.
// Integration: include after V29cDiagnostics.inc, invoke DiagF* from monitor.
extern "C" __declspec(dllexport) volatile long DSFV29f_ForceSnapshot=0;
extern "C" __declspec(dllexport) volatile long DSFV29f_StreamHooks=0;
extern "C" __declspec(dllexport) volatile long DSFV29f_ReleaseCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29f_ResidencyCalls=0;
extern "C" __declspec(dllexport) volatile long DSFV29f_ResidencyFalse=0;

struct DiagFProcessMemoryCounters32 {
    U32 cb,pageFaultCount,peakWorkingSetSize,workingSetSize;
    U32 quotaPeakPagedPoolUsage,quotaPagedPoolUsage;
    U32 quotaPeakNonPagedPoolUsage,quotaNonPagedPoolUsage;
    U32 pagefileUsage,peakPagefileUsage,privateUsage;
};
static_assert(sizeof(DiagFProcessMemoryCounters32)==44,"PSAPI Win32 counter layout");
static const char* const kDiagFPoolNames[21] = {
    "Misc","Runtime","RuntimeSys","Movie_Large","Movie_Small",
    "Dynamic_Ad_Textures","City_Global","City_Region","Vehicle_Global",
    "Vehicle_Spooled","Vehicle_Spooled_Int","Vehicle_Spooled_Ext",
    "Character_Global","Character_Spooled","Sky_Spooled","City_Locking",
    "MegaMesh_etc","GUI_Image_small","GUI_Image_medium","GUI_Image_large","Billboard"
};
struct DiagFPool {
    U32 descriptor, runtimeCount, cfgCount, budgetA,budgetB, flags, block;
    U32 above3, maxRaw, nodeCount0, nodeCount1, firstRaw, lastRaw;
    bool valid, scanned;
};
static DiagFPool g_diagFPrevPools[21]={};
static bool g_diagFHavePrevPools=false;
static long g_diagFLastReleases=0,g_diagFLastQueries=0,g_diagFLastFalse=0;

static DiagFPool DiagFReadPool(U32 poolIndex) {
    DiagFPool r={};
    const UPtr cfg=Address(0x00FD2678u+poolIndex*0x14u);
    const UPtr runtime=Address(kGraphicsManager)+0x0Cu+poolIndex*0x5Cu;
    if(!Readable(cfg,20u)||!Readable(runtime,8u))return r;
    r.flags=Read32(cfg);r.budgetA=Read32(cfg+4u);r.budgetB=Read32(cfg+8u);
    r.cfgCount=Read32(cfg+12u);r.block=Read32(cfg+16u);
    r.descriptor=Read32(runtime);r.runtimeCount=Read32(runtime+4u);
    r.valid=true;
    // Runtime slots are descriptor-counts, not proof of loaded/READY resources.
    // Avoid walking arbitrary corrupt pointers or oversized declared counts.
    const U32 count=r.runtimeCount;
    if(count && count<=512u && r.descriptor && count<=0xFFFFFFFFu/0x60u &&
       Readable(r.descriptor,count*0x60u)) {
        r.scanned=true;
        for(U32 i=0;i<count;++i) {
            const UPtr at=r.descriptor+i*0x60u;
            const U32 raw=Read32(at+0x14u);
            if(i==0u) {r.firstRaw=raw;r.nodeCount0=Read32(at+4u);r.nodeCount1=Read32(at+0x30u);}
            if(i==count-1u)r.lastRaw=raw;
            if(raw>3u)++r.above3;
            if(raw>r.maxRaw)r.maxRaw=raw;
        }
    }
    return r;
}

static void DiagFAppendU(char*& c,U32 v,const char* e) {AppendNumber(c,static_cast<long>(v),e);}
static void DiagFAppendKeyU(char*& c,const char* key,U32 v,const char* e) {
    AppendText(c,key,e);DiagFAppendU(c,v,e);
}
static void DiagFAppendKeyHex(char*& c,const char* key,U32 v,const char* e) {
    AppendText(c,key,e);AppendHex32(c,v,e);
}
static void DiagFPrintPool(U32 idx,const DiagFPool& p) {
    char line[480]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29f_POOL tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," index=",idx,end);
    AppendText(c," name=",end);AppendText(c,kDiagFPoolNames[idx],end);
    DiagFAppendKeyU(c," valid=",p.valid?1u:0u,end);
    if(p.valid) {
        DiagFAppendKeyHex(c," cfg=",Address(0x00FD2678u+idx*0x14u),end);
        DiagFAppendKeyU(c," cfgSlots=",p.cfgCount,end);
        DiagFAppendKeyHex(c," budgetA=",p.budgetA,end);
        DiagFAppendKeyHex(c," budgetB=",p.budgetB,end);
        DiagFAppendKeyHex(c," flags=",p.flags,end);
        DiagFAppendKeyHex(c," block=",p.block,end);
        DiagFAppendKeyHex(c," array=",p.descriptor,end);
        DiagFAppendKeyU(c," runtimeSlots=",p.runtimeCount,end);
        DiagFAppendKeyU(c," scanned=",p.scanned?1u:0u,end);
        if(p.scanned) {
            DiagFAppendKeyU(c," rawAbove3=",p.above3,end);
            DiagFAppendKeyU(c," rawMax=",p.maxRaw,end);
            DiagFAppendKeyU(c," rawFirst=",p.firstRaw,end);
            DiagFAppendKeyU(c," rawLast=",p.lastRaw,end);
            DiagFAppendKeyU(c," firstNodes0=",p.nodeCount0,end);
            DiagFAppendKeyU(c," firstNodes1=",p.nodeCount1,end);
        }
    }
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
}
static bool DiagFPoolChanged(const DiagFPool& a,const DiagFPool& b) {
    return a.valid!=b.valid || a.scanned!=b.scanned ||
        a.descriptor!=b.descriptor || a.runtimeCount!=b.runtimeCount ||
        a.cfgCount!=b.cfgCount || a.budgetA!=b.budgetA || a.budgetB!=b.budgetB ||
        a.above3!=b.above3 || a.maxRaw!=b.maxRaw ||
        a.firstRaw!=b.firstRaw || a.lastRaw!=b.lastRaw;
}

static void DiagFLogVAMemory() {
    // VirtualQuery is read-only. This walk is bounded to keep the monitor from
    // stalling if the game's address space becomes extremely fragmented.
    unsigned long long address=0x10000ull;
    unsigned long long committed=0, privateBytes=0, mappedBytes=0, imageBytes=0;
    unsigned long long freeBytes=0, maxFree=0;
    U32 regions=0, failures=0;
    while(address<0x100000000ull && regions<65536u) {
        MemoryBasicInformation32 info={};
        const U32 got=VirtualQuery(reinterpret_cast<void*>(static_cast<UPtr>(address)),&info,sizeof(info));
        if(got<sizeof(info) || !info.regionSize) {++failures;break;}
        const unsigned long long start=reinterpret_cast<UPtr>(info.base);
        const unsigned long long next=start+info.regionSize;
        if(next<=address) {++failures;break;}
        if(info.state==0x1000u) {
            committed+=info.regionSize;
            if(info.type==0x20000u)privateBytes+=info.regionSize; // MEM_PRIVATE
            else if(info.type==0x40000u)mappedBytes+=info.regionSize; // MEM_MAPPED
            else if(info.type==0x1000000u)imageBytes+=info.regionSize; // MEM_IMAGE
        } else if(info.state==0x10000u) { // MEM_FREE
            freeBytes+=info.regionSize;
            if(info.regionSize>maxFree)maxFree=info.regionSize;
        }
        address=next;++regions;
    }
    char line[440]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29f_VA tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," regions=",regions,end);DiagFAppendKeyU(c," failures=",failures,end);
    DiagFAppendKeyU(c," truncated=",address<0x100000000ull?1u:0u,end);
    DiagFAppendKeyU(c," commitMiB=",static_cast<U32>(committed>>20),end);
    DiagFAppendKeyU(c," privateCommitMiB=",static_cast<U32>(privateBytes>>20),end);
    DiagFAppendKeyU(c," mappedCommitMiB=",static_cast<U32>(mappedBytes>>20),end);
    DiagFAppendKeyU(c," imageCommitMiB=",static_cast<U32>(imageBytes>>20),end);
    DiagFAppendKeyU(c," freeMiB=",static_cast<U32>(freeBytes>>20),end);
    DiagFAppendKeyU(c," largestFreeMiB=",static_cast<U32>(maxFree>>20),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
}

static void DiagFPrintProcess() {
    DiagFProcessMemoryCounters32 m={};m.cb=sizeof(m);
    const BOOL ok=GetProcessMemoryInfo(GetCurrentProcess(),&m,sizeof(m));
    const long releases=DSFV29f_ReleaseCalls,queries=DSFV29f_ResidencyCalls;
    const long misses=DSFV29f_ResidencyFalse;
    char line[460]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29f_PROCESS tick=",end);DiagFAppendU(c,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(c," psapiOk=",ok?1u:0u,end);
    if(ok) {
        DiagFAppendKeyU(c," privateKiB=",m.privateUsage>>10,end);
        DiagFAppendKeyU(c," workingKiB=",m.workingSetSize>>10,end);
        DiagFAppendKeyU(c," peakWorkingKiB=",m.peakWorkingSetSize>>10,end);
        DiagFAppendKeyU(c," pageFaults=",m.pageFaultCount,end);
    }
    DiagFAppendKeyU(c," hooks=",static_cast<U32>(DSFV29f_StreamHooks),end);
    DiagFAppendKeyU(c," releaseCalls=",static_cast<U32>(releases),end);
    DiagFAppendKeyU(c," releaseDelta=",static_cast<U32>(releases-g_diagFLastReleases),end);
    DiagFAppendKeyU(c," residentQueries=",static_cast<U32>(queries),end);
    DiagFAppendKeyU(c," residentDelta=",static_cast<U32>(queries-g_diagFLastQueries),end);
    DiagFAppendKeyU(c," residentFalse=",static_cast<U32>(misses),end);
    DiagFAppendKeyU(c," residentFalseDelta=",static_cast<U32>(misses-g_diagFLastFalse),end);
    AppendText(c,"\n",end);*c=0;DiagPrint(line);
    g_diagFLastReleases=releases;g_diagFLastQueries=queries;g_diagFLastFalse=misses;
}

static void DiagFMemorySample(U32 loop) {
    // Called on Monitor thread at 200ms. Read pools and log one-second
    // summaries. Full rows every 5 sec, or on any manager/config changes.
    const bool force=DSFV29f_ForceSnapshot!=0;
    if(force)DSFV29f_ForceSnapshot=0;
    if(loop%5u!=0u && !force)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    for(U32 i=0;i<21u;++i) {
        const DiagFPool p=DiagFReadPool(i);
        if(force || !g_diagFHavePrevPools || loop%25u==0u ||
           DiagFPoolChanged(p,g_diagFPrevPools[i]))DiagFPrintPool(i,p);
        g_diagFPrevPools[i]=p;
    }
    g_diagFHavePrevPools=true;
    DiagFPrintProcess();
    if(force || loop%25u==0u)DiagFLogVAMemory();
    if(force) {
        char line[120]={};char* c=line;const char* end=line+sizeof(line)-1;
        AppendText(c,"DSFV29f_MARKER tick=",end);DiagFAppendU(c,tick,end);
        AppendText(c," userForce=1\n",end);*c=0;DiagPrint(line);
    }
}

// The two stream hooks below were independently researched in V29b StreamingProbe.
// They count DISPATCHES / QUERY RESULTS, not successful release/READY events.
struct DiagFDetour {UPtr target;U32 len;U8 original[16];U8* trampoline;bool installed;};
static DiagFDetour g_diagFReleaseDetour={},g_diagFResidentDetour={};
extern "C" UPtr g_diagFReleaseTrampoline=0;
extern "C" UPtr g_diagFResidentTrampoline=0;
extern "C" __declspec(naked) void DiagFReleaseEntry() {
    __asm {
        pushfd
        pushad
        lock inc dword ptr [DSFV29f_ReleaseCalls]
        popad
        popfd
        jmp dword ptr [g_diagFReleaseTrampoline]
    }
}
extern "C" __declspec(naked) void DiagFResidencyEntry() {
    __asm {
        push dword ptr [esp+4]
        call dword ptr [g_diagFResidentTrampoline]
        pushfd
        pushad
        lock inc dword ptr [DSFV29f_ResidencyCalls]
        cmp byte ptr [esp+28],0
        jne f_resident
        lock inc dword ptr [DSFV29f_ResidencyFalse]
    f_resident:
        popad
        popfd
        ret 4
    }
}
static bool DiagFInstallDetour(DiagFDetour& hook,UPtr target,const U8* sig,U32 len,UPtr redirect,UPtr& trampolineOut) {
    if(len<5u || len>16u || !Readable(Address(target),len))return false;
    for(U32 i=0;i<len;++i)if(*reinterpret_cast<volatile U8*>(Address(target)+i)!=sig[i])return false;
    U8* trampoline=reinterpret_cast<U8*>(VirtualAlloc(nullptr,len+5u,kMemCommitReserve,kPageExecuteReadWrite));
    if(!trampoline)return false;
    for(U32 i=0;i<len;++i)trampoline[i]=sig[i];
    trampoline[len]=0xE9u;
    Write32(reinterpret_cast<UPtr>(trampoline)+len+1u,
        static_cast<U32>((Address(target)+len)-(reinterpret_cast<UPtr>(trampoline)+len+5u)));
    trampolineOut=reinterpret_cast<UPtr>(trampoline);
    hook.target=Address(target);hook.len=len;hook.trampoline=trampoline;
    for(U32 i=0;i<len;++i)hook.original[i]=sig[i];
    DWORD old=0,unused=0;
    if(!VirtualProtect(reinterpret_cast<void*>(hook.target),len,kPageExecuteReadWrite,&old)) {
        trampolineOut=0;VirtualFree(trampoline,0,kMemRelease);return false;
    }
    *reinterpret_cast<volatile U8*>(hook.target)=0xE9u;
    Write32(hook.target+1u,static_cast<U32>(redirect-(hook.target+5u)));
    for(U32 i=5u;i<len;++i)*reinterpret_cast<volatile U8*>(hook.target+i)=0x90u;
    const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(hook.target),len);
    const BOOL restore=VirtualProtect(reinterpret_cast<void*>(hook.target),len,old,&unused);
    hook.installed=true;
    if(flush && restore)return true;
    // Restore original if verification fails.
    DWORD prot=0,ignore=0;
    if(VirtualProtect(reinterpret_cast<void*>(hook.target),len,kPageExecuteReadWrite,&prot)) {
        for(U32 i=0;i<len;++i)*reinterpret_cast<volatile U8*>(hook.target+i)=sig[i];
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(hook.target),len);
        VirtualProtect(reinterpret_cast<void*>(hook.target),len,old,&ignore);
        hook.installed=false;trampolineOut=0;
    }
    return false;
}
static void DiagFRestoreDetour(DiagFDetour& hook,UPtr& trampolineOut) {
    if(hook.installed) {
        DWORD old=0,unused=0;
        if(VirtualProtect(reinterpret_cast<void*>(hook.target),hook.len,kPageExecuteReadWrite,&old)) {
            for(U32 i=0;i<hook.len;++i)*reinterpret_cast<volatile U8*>(hook.target+i)=hook.original[i];
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(hook.target),hook.len);
            VirtualProtect(reinterpret_cast<void*>(hook.target),hook.len,old,&unused);
            hook.installed=false;
        }
    }
    trampolineOut=0; // trampoline is intentionally process-lifetime if installed.
}
static bool DiagFInstallStreamHooks() {
    const U8 releaseSig[]={0x55u,0x8Bu,0xECu,0x83u,0xE4u,0xF8u};
    const U8 residentSig[]={0x8Bu,0x41u,0x38u,0x53u,0x8Du,0x14u,0xC5u,0,0,0,0};
    const UPtr rel=Address(0x0059CD80u),res=Address(0x0059F3A0u);
    if(DSF150_Status!=1 || !Readable(rel,6u) || !Readable(res,11u))return false;
    for(U32 i=0;i<6u;++i)if(*reinterpret_cast<volatile U8*>(rel+i)!=releaseSig[i])return false;
    for(U32 i=0;i<11u;++i)if(*reinterpret_cast<volatile U8*>(res+i)!=residentSig[i])return false;
    if(!DiagFInstallDetour(g_diagFReleaseDetour,0x0059CD80u,releaseSig,6u,
          reinterpret_cast<UPtr>(&DiagFReleaseEntry),g_diagFReleaseTrampoline))return false;
    if(!DiagFInstallDetour(g_diagFResidentDetour,0x0059F3A0u,residentSig,11u,
          reinterpret_cast<UPtr>(&DiagFResidencyEntry),g_diagFResidentTrampoline)) {
        DiagFRestoreDetour(g_diagFReleaseDetour,g_diagFReleaseTrampoline);return false;
    }
    DSFV29f_StreamHooks=2;
    return true;
}

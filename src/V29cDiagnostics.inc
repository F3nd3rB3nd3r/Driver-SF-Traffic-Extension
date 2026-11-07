// V29c passive diagnostics. Included once after AppendText/AppendNumber/AppendHex32.
// Ordinary file I/O and formatting run on Monitor. Hot hooks append compact
// bounded records, with ONE synchronous emergency file line on the first invalid
// cleanup receiver so evidence can survive an immediate in-game crash.
constexpr U32 kDiagRingSize = 2048u;
constexpr U32 kDiagMaxMapNodes = 320u;
struct DiagCallbackEvent {
    volatile long sequence; // published last with an interlocked exchange
    U32 type, owner, slot, count, record, receiver, vptr, method;
    U32 flags;
};
static DiagCallbackEvent g_diagEvents[kDiagRingSize] = {};
static HANDLE g_diagFile = nullptr;
static U32 g_diagLastFileStatus = 0u;
static U32 g_diagStartTick = 0u;
static volatile long g_diagEmergencyWritten = 0;
static U32 g_diagPreviousMapCount = 0xFFFFFFFFu;
static U16 g_diagPreviousIds[kDiagMaxMapNodes] = {};
static U32 g_diagPreviousIdCount = 0;
static UPtr g_diagRegResume = 0;
static UPtr g_diagCleanResume = 0;

static U32 DiagLength(const char* s) {
    U32 n=0;
    while (n<2048u && s[n]) ++n;
    return n;
}
#if defined(DSF_V29S_HASH_HEAD_TRACE)
static bool DiagHasPrefix(const char* s,const char* prefix) {
    while(*prefix) {if(*s++!=*prefix++)return false;}
    return true;
}
static bool DiagHashKeepLine(const char* s) {
    const char* const keep[]={
        "DSFV29c_START", "DSFV29f_START", "DSFV29n_START",
        "DSFV29s_REQ_START", "DSFV29s_MODEL_POOL",
        "DSFV29s_RESOURCE_COLLECTION", "DSFV29s_HANDLE_",
        "DSFV29s_LIFE_", "DSFV29s_NEXT_BAD_WRITE",
        "DSFV29s_HASH_HEAD_", "DSFV29s_AUDIO_"
    };
    for(U32 i=0u;i<sizeof(keep)/sizeof(keep[0]);++i)
        if(DiagHasPrefix(s,keep[i]))return true;
    return false;
}
#endif
static void DiagPrint(const char* s) {
#if defined(DSF_V29S_HASH_HEAD_TRACE)
    if(!DiagHashKeepLine(s))return;
#endif
    OutputDebugStringA(s);
    if (g_diagFile && reinterpret_cast<UPtr>(g_diagFile)!=0xFFFFFFFFu) {
        U32 written=0;
        const U32 n=DiagLength(s);
        if (n && !WriteFile(g_diagFile,s,n,&written,nullptr)) g_diagLastFileStatus=2;
    }
}
static void DiagOpenFile() {
    g_diagStartTick=GetTickCount();
    char path[300]={};
    U32 len=GetTempPathA(240u,path);
    if (!len || len>=240u) {
        g_diagLastFileStatus=1;
        OutputDebugStringA("DSFV29c: TEMP path unavailable; diagnostics only via OutputDebugString.\n");
        return;
    }
    char* cur=path+len;
    const char* end=path+sizeof(path)-1;
    AppendText(cur,"DSFV29f_worldmem_",end);
    AppendNumber(cur,static_cast<long>(GetCurrentProcessId()),end);
    AppendText(cur,".log",end);
    *cur=0;
    // FILE_APPEND_DATA, FILE_SHARE_READ|WRITE, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL.
    g_diagFile=CreateFileA(path,4u,3u,nullptr,4u,0x80u,nullptr);
    if (reinterpret_cast<UPtr>(g_diagFile)==0xFFFFFFFFu || !g_diagFile) {
        g_diagFile=nullptr;
        g_diagLastFileStatus=1;
        OutputDebugStringA("DSFV29c: diagnostic file creation failed; OutputDebugString still works.\n");
        return;
    }
#if defined(DSFV29C_DISABLE_PRELOAD)
    DiagPrint("DSFV29c_START version=baseline18 sampleMs=200 mapMax=320 callbackSlots=2048 exactExe=1 output=TEMP\n");
#else
    DiagPrint("DSFV29c_START version=diag45 sampleMs=200 mapMax=320 callbackSlots=2048 exactExe=1 output=TEMP\n");
#endif
    DiagPrint("DSFV29c_SCOPE completed-map!=READY_or_drawable render_peaks=sampled callback=observational_not_suppressed\n");
    DiagPrint("DSFV29f_START worldPools=21 poolMs=1000 processMs=1000 vaMs=5000 exactPoolCSV=1 streamingHooks=conditional noReleaseSuppression=1\n");
    DiagPrint("DSFV29e_START watchVptrs=00DDA128,00DDA418 watchMethods=00490530,004951D0 watchCapacity=512 sampleMs=200 noDebugBreak=1 noWriteToGameObjects=1\n");
}
static bool DiagMethodExecutable(UPtr method) {
    if (!method || method<0x10000u) return false;
    MemoryBasicInformation32 m={};
    if (VirtualQuery(reinterpret_cast<void*>(method),&m,sizeof(m))<sizeof(m)) return false;
    if (m.state!=kCommitted || (m.protect & kPageGuard)) return false;
    const U32 prot=m.protect & 0xffu;
    return prot==0x10u || prot==0x20u || prot==0x40u || prot==0x80u;
}
// V29e: observational lifetime watch for both previously corrupted callback classes.
// The native registration hook observes the receiver BEFORE stock registration.
// Entries are never written back to game objects. No guard or breakpoints used.
constexpr U32 kV29dVtableA = 0x00DDA128u;
constexpr U32 kV29dVtableB = 0x00DDA418u;
constexpr U32 kV29dWatchCapacity = 512u;
struct DiagV29dWatch {
    volatile long state;   // 0 unavailable, 1 active, 2 reached cleanup
    U32 owner, slot, record, receiver, registeredTick;
    U32 originalVptr, originalMethod;
    U32 lastSampledVptr, cleanupVptr, cleanupTick;
    U32 armPrinted, cleanupPrinted, changeEvents;
};
static DiagV29dWatch g_v29dWatches[kV29dWatchCapacity] = {};
static volatile long g_v29dWatchCount = 0;
static volatile long g_v29dWatchDrops = 0;
static U32 g_v29dWatchChanges = 0;
// Set to 1 in the debugger shortly before the chosen registration window.
// The next matching A receiver pauses before native registration resumes.
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchArmEnabled = 0;
// Set to 1 only after the hardware watch is installed in the stopped debugger.
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchRelease = 0;
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchTargetVptr = 0x00DDA128;
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchReceiver = 0;
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchState = 0;
extern "C" __declspec(dllexport) volatile long DSFV29j_WatchCleanupTick = 0;

static void DiagV29jPauseForWatch(const DiagV29dWatch& w) {
    char line[320]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29j_WATCH_HOLD op=ARM tick=",end);
    AppendNumber(c,static_cast<long>(w.registeredTick),end);
    AppendText(c," owner=",end);AppendHex32(c,w.owner,end);
    AppendText(c," slot=",end);AppendNumber(c,static_cast<long>(w.slot),end);
    AppendText(c," record=",end);AppendHex32(c,w.record,end);
    AppendText(c," receiver=",end);AppendHex32(c,w.receiver,end);
    AppendText(c," old=",end);AppendHex32(c,w.originalVptr,end);
    AppendText(c," cmd=bphws 0x",end);AppendHex32(c,w.receiver,end);
    AppendText(c,",w,4\n",end);*c=0;
    OutputDebugStringA(line);
    // The caller already checked IsDebuggerPresent. Hold without raising INT3,
    // which x32dbg can repeatedly redeliver instead of advancing this thread.
    while (DSFV29j_WatchRelease==0) Sleep(10);
    InterlockedExchange(&DSFV29j_WatchRelease,0);
}

static void DiagV29dArm(U32 owner,U32 slot,U32 record,U32 receiver,U32 vptr,U32 method) {
    const bool watchA = vptr == kV29dVtableA && method == 0x00490530u;
    const bool watchB = vptr == kV29dVtableB && method == 0x004951D0u;
    if (!receiver || (!watchA && !watchB)) return;
    const long index = InterlockedIncrement(&g_v29dWatchCount)-1;
    if (index < 0 || static_cast<U32>(index)>=kV29dWatchCapacity) {
        InterlockedIncrement(&g_v29dWatchDrops);
        return;
    }
    DiagV29dWatch& w=g_v29dWatches[index];
    w.owner=owner;w.slot=slot;w.record=record;w.receiver=receiver;
    w.registeredTick=GetTickCount()-g_diagStartTick;
    w.originalVptr=vptr;w.originalMethod=method;
    w.lastSampledVptr=vptr;
    w.cleanupTick=0u;w.cleanupVptr=0u;
    w.armPrinted=0u;w.cleanupPrinted=0u;w.changeEvents=0u;
    InterlockedExchange(&w.state,1);
    if ((watchA || watchB) &&
        vptr == static_cast<U32>(DSFV29j_WatchTargetVptr) && IsDebuggerPresent() &&
        InterlockedExchange(&DSFV29j_WatchArmEnabled,0)==1) {
        InterlockedExchange(&DSFV29j_WatchReceiver,static_cast<long>(receiver));
        InterlockedExchange(&DSFV29j_WatchCleanupTick,0);
        InterlockedExchange(&DSFV29j_WatchState,1);
        DiagV29jPauseForWatch(w);
    }
}
static void DiagV29dCleanup(U32 owner,U32 slot,U32 record,U32 receiver,U32 vptr) {
    // Match *all* currently active registrations for this owner/slot/record.
    // Mark them retired so later legitimate deallocation isn't counted as a
    // callback corruption. The pre-cleanup value is still retained.
    const long n=g_v29dWatchCount;
    for (long i=0;i<n && i<static_cast<long>(kV29dWatchCapacity);++i) {
        DiagV29dWatch& w=g_v29dWatches[i];
        if (w.state!=1 || w.owner!=owner || w.slot!=slot ||
            w.record!=record || w.receiver!=receiver) continue;
        w.cleanupVptr=vptr;
        w.cleanupTick=GetTickCount()-g_diagStartTick;
        InterlockedExchange(&w.state,2);
        if (w.receiver==static_cast<U32>(DSFV29j_WatchReceiver)) {
            InterlockedExchange(&DSFV29j_WatchCleanupTick,static_cast<long>(w.cleanupTick));
            InterlockedExchange(&DSFV29j_WatchState,2);
            char line[180]={};char* c=line;const char* end=line+sizeof(line)-1;
            AppendText(c,"DSFV29j_BREAK op=CLEAN tick=",end);
            AppendNumber(c,static_cast<long>(w.cleanupTick),end);
            AppendText(c," receiver=",end);AppendHex32(c,w.receiver,end);
            AppendText(c," preVptr=",end);AppendHex32(c,vptr,end);
            AppendText(c,"\n",end);*c=0;
            OutputDebugStringA(line);
        }
    }
}
static const DiagV29dWatch* DiagV29dFind(U32 owner,U32 slot,U32 record,U32 receiver) {
    // Use most recent matching registration, if any.
    const long n=g_v29dWatchCount;
    for (long i=(n>static_cast<long>(kV29dWatchCapacity) ? static_cast<long>(kV29dWatchCapacity) : n)-1;i>=0;--i) {
        const DiagV29dWatch& w=g_v29dWatches[i];
        if (w.state && w.owner==owner && w.slot==slot &&
            w.record==record && w.receiver==receiver) return &w;
    }
    return nullptr;
}
static void DiagV29dPrintWatch(const char* kind, const DiagV29dWatch& w, U32 currentVptr) {
    char line[480]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29e_WATCH op=",end);AppendText(c,kind,end);
    AppendText(c," tick=",end);AppendNumber(c,static_cast<long>(GetTickCount()-g_diagStartTick),end);
    AppendText(c," owner=",end);AppendHex32(c,w.owner,end);
    AppendText(c," slot=",end);AppendNumber(c,static_cast<long>(w.slot),end);
    AppendText(c," record=",end);AppendHex32(c,w.record,end);
    AppendText(c," receiver=",end);AppendHex32(c,w.receiver,end);
    AppendText(c," regTick=",end);AppendNumber(c,static_cast<long>(w.registeredTick),end);
    AppendText(c," old=",end);AppendHex32(c,w.originalVptr,end);
    AppendText(c," current=",end);AppendHex32(c,currentVptr,end);
    const UPtr ptr=currentVptr;
    U32 indirectMethod=(ptr && Readable(ptr+8u,4u))?Read32(ptr+8u):0u;
    AppendText(c," indirect8=",end);AppendHex32(c,indirectMethod,end);
    // Receiver may now coincide with the +8 next field of a 16-byte
    // vehicle linked-list node. Show adjacent raw words, not a diagnosis.
    AppendText(c," wordM8=",end);AppendHex32(c, w.receiver>=8u && Readable(w.receiver-8u,4u)?Read32(w.receiver-8u):0u,end);
    AppendText(c," wordM4=",end);AppendHex32(c, w.receiver>=4u && Readable(w.receiver-4u,4u)?Read32(w.receiver-4u):0u,end);
    AppendText(c," cmd=bphws 0x",end);AppendHex32(c,w.receiver,end);
    AppendText(c,",w,4",end);
    AppendText(c,"\n",end);*c=0;
    DiagPrint(line);
}
static void DiagV29dPoll() {
    const long n=g_v29dWatchCount;
    for (long i=0;i<n && i<static_cast<long>(kV29dWatchCapacity);++i) {
        DiagV29dWatch& w=g_v29dWatches[i];
        const long state=w.state;
        if (!state) continue;
        if (!w.armPrinted) {
            DiagV29dPrintWatch("ARM",w,w.originalVptr);
            w.armPrinted=1u;
        }
        if (state==2) {
            if (!w.cleanupPrinted) {
                if (w.cleanupVptr!=w.originalVptr) {
                    ++g_v29dWatchChanges;
                    DiagV29dPrintWatch("CLEAN_CHANGED",w,w.cleanupVptr);
                }
                // Printing every CLEAN_OK would duplicate thousands of lines.
                w.cleanupPrinted=1u;
            }
            continue;
        }
        // A sample cannot establish the writer; use x32dbg if needed.
        const U32 value=Readable(w.receiver,4u)?Read32(w.receiver):0xFFFFFFFFu;
        if (value!=w.lastSampledVptr) {
            w.lastSampledVptr=value;
            ++g_v29dWatchChanges;
            if (w.changeEvents<4u) {
                DiagV29dPrintWatch("CHANGE",w,value);
                ++w.changeEvents;
            }
        }
    }
}

static void DiagRecordCallback(U32 type, UPtr owner, U32 slot, U32 count, UPtr record) {
    const long seq=InterlockedIncrement(&DSFV29c_CallbackTotalEvents);
    DiagCallbackEvent& e=g_diagEvents[static_cast<U32>(seq-1) % kDiagRingSize];
    InterlockedExchange(&e.sequence,-seq);
    e.type=type;
    e.owner=owner;
    e.slot=slot;
    e.count=count;
    e.record=record;
    e.receiver=0;
    e.vptr=0;
    e.method=0;
    e.flags=0;
    if (type != 3u) {
        // `record+4` is the callback receiver. `record` itself is a wrapper.
        if (record && Readable(record+4u,4u)) {
            e.receiver=Read32(record+4u);
            if (e.receiver && Readable(e.receiver,4u)) {
                e.vptr=Read32(e.receiver);
                if (e.vptr && Readable(e.vptr+8u,4u)) {
                    e.method=Read32(e.vptr+8u);
                    if (DiagMethodExecutable(e.method)) e.flags=1u;
                }
            }
        }
        if (!(e.flags&1u)) InterlockedIncrement(&DSFV29c_CallbackSuspicious);
    }
    if (type==1u) DiagV29dArm(owner,slot,record,e.receiver,e.vptr,e.method);
    if (type==2u) DiagV29dCleanup(owner,slot,record,e.receiver,e.vptr);
    // A bad receiver may crash the game immediately at 008587CC before the
    // monitoring thread can drain the ring. Preserve the FIRST such event.
    if (type==2u && !(e.flags&1u) &&
        InterlockedExchange(&g_diagEmergencyWritten,1)==0) {
        char line[320]={};char* c=line;const char* end=line+sizeof(line)-1;
        AppendText(c,"DSFV29c_EMERGENCY op=CLEAN_BAD owner=",end);
        AppendHex32(c,owner,end);
        AppendText(c," slot=",end);AppendNumber(c,static_cast<long>(slot),end);
        AppendText(c," count=",end);AppendNumber(c,static_cast<long>(count),end);
        AppendText(c," ref=",end);AppendHex32(c,e.record,end);
        AppendText(c," receiver=",end);AppendHex32(c,e.receiver,end);
        AppendText(c," vptr=",end);AppendHex32(c,e.vptr,end);
        AppendText(c," method8=",end);AppendHex32(c,e.method,end);
        const DiagV29dWatch* w=DiagV29dFind(owner,slot,record,e.receiver);
        if (w) {
            AppendText(c," regVptr=",end);AppendHex32(c,w->originalVptr,end);
            AppendText(c," regTick=",end);AppendNumber(c,static_cast<long>(w->registeredTick),end);
            AppendText(c," lastPolledVptr=",end);AppendHex32(c,w->lastSampledVptr,end);
        }
        AppendText(c,"\n",end);*c=0;
        DiagPrint(line); // one exceptional synchronous write; no suppression
    }
    InterlockedExchange(&e.sequence,seq);
}

// g_diagEvents never stores a live object pointer for later dereferencing:
// receiver/vptr/method are values copied at event time.
extern "C" __declspec(noinline) void __cdecl V29cCallbackHookBridge(U32 kind, const CpuRegs* r) {
    const UPtr owner=r->ecx;
    if (!Readable(owner+0x200u,4u)) return;
    const U32 count=Read32(owner+0x200u);
    if (kind==1u) { // registration entry at 00858720, before its first store
        // savedEsp equals original ESP-4 because pushfd ran before pushad.
        const UPtr stackArg2=r->savedEsp+12u;
        const UPtr ref=Readable(stackArg2,4u) ? Read32(stackArg2) : 0u;
        DiagRecordCallback(1u,owner,count,count,ref);
    } else if (kind==2u) { // teardown entry at 008587B0, BEFORE any callback
        DiagRecordCallback(3u,owner,0u,count,0u);
        // Bound the inspection; log all entries for common small owners.
        const U32 inspect=count>32u ? 32u : count;
        for (U32 i=0;i<inspect;++i) {
            const UPtr at=owner+16u*i;
            if (!Readable(at,4u)) break;
            DiagRecordCallback(2u,owner,i,count,Read32(at));
        }
    }
}
extern "C" __declspec(naked) void V29cCallbackRegistrationStub() {
    __asm {
        pushfd
        pushad
        mov eax, esp
        push eax
        push 1
        call V29cCallbackHookBridge
        add esp, 8
        popad
        popfd
        // Stock 00858720: mov eax,[ecx+200]
        mov eax, dword ptr [ecx+0x200]
        jmp dword ptr [g_diagRegResume]
    }
}
extern "C" __declspec(naked) void V29cCallbackCleanupStub() {
    __asm {
        pushfd
        pushad
        mov eax, esp
        push eax
        push 2
        call V29cCallbackHookBridge
        add esp, 8
        popad
        popfd
        // Stock 008587B0..B9 (10 complete bytes)
        push ebx
        mov ebx, ecx
        push esi
        mov esi, dword ptr [ebx+0x200]
        jmp dword ptr [g_diagCleanResume]
    }
}
static const U8 kV29cRegistrationBytes[]={0x8B,0x81,0,2,0,0};
static const U8 kV29cCleanupBytes[]={0x53,0x8B,0xD9,0x56,0x8B,0xB3,0,2,0,0};
static bool DiagPatchCallback(UPtr addr,const U8* original,U32 count,void* replacement) {
    const UPtr target=Address(addr);
    if (count<5u || !Readable(target,count)) return false;
    for (U32 i=0;i<count;++i)
        if (*reinterpret_cast<volatile const U8*>(target+i)!=original[i]) return false;
    DWORD old=0;
    if (!VirtualProtect(reinterpret_cast<void*>(target),count,kPageExecuteReadWrite,&old)) return false;
    *reinterpret_cast<volatile U8*>(target)=0xE9u;
    Write32(target+1u,static_cast<U32>(reinterpret_cast<UPtr>(replacement)-(target+5u)));
    for (U32 i=5u;i<count;++i) *reinterpret_cast<volatile U8*>(target+i)=0x90u;
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(target),count);
    DWORD ignored=0;
    const BOOL restored=VirtualProtect(reinterpret_cast<void*>(target),count,old,&ignored);
    // With a failed protection/flush step, leave existing executable intact in
    // preference to blindly unpatching bytes another thread might execute.
    return flushed && restored;
}
static void DiagRestoreCallback(UPtr addr,const U8* original,U32 count) {
    const UPtr target=Address(addr);
    DWORD old=0;
    if (VirtualProtect(reinterpret_cast<void*>(target),count,kPageExecuteReadWrite,&old)) {
        for (U32 i=0;i<count;++i) *reinterpret_cast<volatile U8*>(target+i)=original[i];
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(target),count);
        DWORD ignored=0;
        VirtualProtect(reinterpret_cast<void*>(target),count,old,&ignored);
    }
}
static bool InstallV29cCallbackHooks() {
    // Optional passive instrumentation, no V29b game behavior/limits changed.
    const UPtr reg=Address(0x00858720u), clean=Address(0x008587B0u);
    if (!Readable(reg,sizeof(kV29cRegistrationBytes)) ||
        !Readable(clean,sizeof(kV29cCleanupBytes))) {
        DSFV29c_CallbackHooks=-1;
        Log("DSFV29c: callback hook sites unreadable; only baseline monitoring active.\n");
        return false;
    }
    for(U32 i=0;i<sizeof(kV29cRegistrationBytes);++i)
        if (*reinterpret_cast<volatile const U8*>(reg+i)!=kV29cRegistrationBytes[i]) {
            DSFV29c_CallbackHooks=-2;
            Log("DSFV29c: callback register signature mismatch; no callback hooks installed.\n");
            return false;
        }
    for(U32 i=0;i<sizeof(kV29cCleanupBytes);++i)
        if (*reinterpret_cast<volatile const U8*>(clean+i)!=kV29cCleanupBytes[i]) {
            DSFV29c_CallbackHooks=-3;
            Log("DSFV29c: callback teardown signature mismatch; no callback hooks installed.\n");
            return false;
        }
    g_diagRegResume=reg+sizeof(kV29cRegistrationBytes);
    g_diagCleanResume=clean+sizeof(kV29cCleanupBytes);
    if (!DiagPatchCallback(0x00858720u,kV29cRegistrationBytes,
                           sizeof(kV29cRegistrationBytes),reinterpret_cast<void*>(&V29cCallbackRegistrationStub))) {
        DSFV29c_CallbackHooks=-4;
        Log("DSFV29c: callback register patch failed.\n");
        return false;
    }
    if (!DiagPatchCallback(0x008587B0u,kV29cCleanupBytes,
                           sizeof(kV29cCleanupBytes),reinterpret_cast<void*>(&V29cCallbackCleanupStub))) {
        DiagRestoreCallback(0x00858720u,kV29cRegistrationBytes,sizeof(kV29cRegistrationBytes));
        DSFV29c_CallbackHooks=-5;
        Log("DSFV29c: callback teardown patch failed; register hook restored.\n");
        return false;
    }
    DSFV29c_CallbackHooks=2;
    Log("DSFV29c: passive callback entry hooks installed (registration+teardown).\n");
    return true;
}

struct DiagMetric {
    long draw0, draw1, frame, filter, pending, released, submodel, gfxBusy, gfxMaxRaw;
};
static DiagMetric g_diagPeaks={0,0,0,0,0,0,0,0,0};
static void DiagUpdatePeak(long& peak,long value) {if (value>peak) peak=value;}
static long DiagReadableWord(UPtr at) {
    return Readable(at,4u) ? static_cast<long>(Read32(at)) : -1;
}
static DiagMetric DiagReadMetrics() {
    DiagMetric v={-1,-1,-1,-1,-1,-1,-1,-1,-1};
    const long dman=DiagReadableWord(Address(0x019B6434u));
    if (dman>0) {
        const UPtr dm=static_cast<UPtr>(dman);
        for (U32 q=0;q<2u;++q) {
            const UPtr h=dm+(q ? 0x980B0u : 0x14u);
            if (!Readable(h,16u)) continue;
            const U32 count=Read32(h), capacity=Read32(h+4u);
            if (capacity>=1u && capacity<=65536u) {
                if (q==0) v.draw0=static_cast<long>(count);
                else v.draw1=static_cast<long>(count);
            }
        }
    }
    const long arena=DiagReadableWord(Address(0x019B0CC4u));
    if (arena>0 && Readable(static_cast<UPtr>(arena),20u)) {
        const UPtr a=static_cast<UPtr>(arena);
        const U32 base=Read32(a), size=Read32(a+4u), left=Read32(a+8u);
        const U32 end=Read32(a+16u);
        if (base && size==4194304u && left<=end && end<=size)
            v.frame=static_cast<long>(left+size-end);
    }
    const long fc=DiagReadableWord(Address(0x01D4670Cu));
    const UPtr fb=Address(0x019C66C0u), fe=Address(0x01D466C0u);
    if (fc>=0) v.filter=(static_cast<UPtr>(fc)>=fb && static_cast<UPtr>(fc)<=fe) ?
        static_cast<long>(static_cast<UPtr>(fc)-fb) : -2;
    v.pending=DiagReadableWord(Address(0x0188A6B8u));
    v.released=DiagReadableWord(Address(0x018B0C80u));
    const long smOwner=DiagReadableWord(Address(0x019BBA74u));
    if (smOwner>0) {
        const UPtr table=static_cast<UPtr>(smOwner)+0x30u;
        const long cursor=DiagReadableWord(table);
        if (cursor>=0) {
            const UPtr begin=table+0x10u;
            const U32 c=static_cast<U32>(cursor);
            if (c>=begin && c<=begin+15000u*24u && ((c-begin)%24u)==0u)
                v.submodel=static_cast<long>((c-begin)/24u);
        }
    }
    const UPtr pool9=Address(kGraphicsManager)+0x348u;
    if (Readable(pool9,8u)) {
        const UPtr desc=Read32(pool9);
        const U32 slots=Read32(pool9+4u);
        if (slots==150u && desc && Readable(desc,slots*0x60u)) {
            v.gfxBusy=0;v.gfxMaxRaw=0;
            for (U32 i=0;i<slots;++i) {
                const U32 active=Read32(desc+i*0x60u+0x14u);
                if (active>3u) ++v.gfxBusy;
                if (active>static_cast<U32>(v.gfxMaxRaw)) v.gfxMaxRaw=static_cast<long>(active);
            }
        }
    }
    return v;
}

struct DiagMapSample {
    bool valid;
    U32 active, free, external, distinct, nullHandlers, duplicates;
    U16 ids[kDiagMaxMapNodes];
};
static bool DiagHasId(const U16* ids,U32 count,U16 id) {
    for (U32 i=0;i<count;++i) if(ids[i]==id)return true;
    return false;
}
static DiagMapSample DiagReadMap() {
    DiagMapSample out={};
    const UPtr pkg=g_pkg0;
    if (!pkg || !Readable(pkg+0x5F54u,4u) || !Readable(pkg+0x5B44u,12u)) return out;
    const UPtr header=pkg+0x5B44u;
    const UPtr cursor=Read32(header), sentinel=pkg+0x5F50u;
    out.free=Read32(header+4u);
    out.active=Read32(header+8u);
    const UPtr head=Read32(pkg+0x5F54u);
    if (out.active>kDiagMaxMapNodes || out.free>kDiagMaxMapNodes ||
        out.active+out.free>kDiagMaxMapNodes) return out;
    const UPtr external=reinterpret_cast<UPtr>(&g_mapExtra[0]);
    const UPtr externalEnd=reinterpret_cast<UPtr>(&g_mapExtra[kMapExtraNodes]);
    UPtr current=head;
    U32 distinct=0;
    for (U32 i=0;i<out.active;++i) {
        const bool stock=current>=pkg+0x5B50u && current<sentinel &&
                         ((current-(pkg+0x5B50u))%16u)==0u;
        const bool extra=current>=external && current<externalEnd &&
                         ((current-external)%16u)==0u;
        if ((!stock && !extra) || !Readable(current,16u)) return out;
        if (extra) ++out.external;
        const U16 id=static_cast<U16>(Read32(current+8u));
        const UPtr handler=Read32(current+12u);
        out.ids[i]=id;
        if(!handler) ++out.nullHandlers;
        if(DiagHasId(out.ids,i,id)) ++out.duplicates;
        else ++distinct;
        current=Read32(current+4u);
    }
    if (current!=sentinel || Read32(header)!=cursor ||
        Read32(header+4u)!=out.free || Read32(header+8u)!=out.active ||
        Read32(pkg+0x5F54u)!=head) return out;
    out.distinct=distinct;
    out.valid=true;
    return out;
}
static void DiagAppendU32(char*& p,U32 n,const char* end) {
    AppendNumber(p,static_cast<long>(n),end);
}
static void DiagMapEvent(const DiagMapSample& m) {
    if(!m.valid) return;
    U32 added=0, removed=0;
    char line[700]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29c_MAP tick=",end);DiagAppendU32(c,GetTickCount()-g_diagStartTick,end);
    AppendText(c," active=",end);DiagAppendU32(c,m.active,end);
    AppendText(c," unique=",end);DiagAppendU32(c,m.distinct,end);
    AppendText(c," free=",end);DiagAppendU32(c,m.free,end);
    AppendText(c," extra=",end);DiagAppendU32(c,m.external,end);
    AppendText(c," duplicates=",end);DiagAppendU32(c,m.duplicates,end);
    AppendText(c," nullHandlers=",end);DiagAppendU32(c,m.nullHandlers,end);
    AppendText(c," added=",end);
    for(U32 i=0;i<m.active;++i) if (!DiagHasId(g_diagPreviousIds,g_diagPreviousIdCount,m.ids[i])) ++added;
    DiagAppendU32(c,added,end);
    AppendText(c," removed=",end);
    for(U32 i=0;i<g_diagPreviousIdCount;++i) if (!DiagHasId(m.ids,m.active,g_diagPreviousIds[i])) ++removed;
    DiagAppendU32(c,removed,end);
    AppendText(c," addIds=",end);
    U32 printed=0;
    for(U32 i=0;i<m.active && printed<24u;++i) {
        if (!DiagHasId(g_diagPreviousIds,g_diagPreviousIdCount,m.ids[i])) {
            if(printed) AppendText(c,",",end);
            DiagAppendU32(c,m.ids[i],end);++printed;
        }
    }
    if(added>printed)AppendText(c,",...",end);
    AppendText(c," remIds=",end);
    printed=0;
    for(U32 i=0;i<g_diagPreviousIdCount && printed<24u;++i) {
        if (!DiagHasId(m.ids,m.active,g_diagPreviousIds[i])) {
            if(printed) AppendText(c,",",end);
            DiagAppendU32(c,g_diagPreviousIds[i],end);++printed;
        }
    }
    if(removed>printed)AppendText(c,",...",end);
    AppendText(c,"\n",end);*c=0;
    DiagPrint(line);
    g_diagPreviousMapCount=m.active;
    g_diagPreviousIdCount=m.active;
    for (U32 i=0;i<m.active;++i)g_diagPreviousIds[i]=m.ids[i];
}
static void DiagSnapshot(U32 loop) {
    const DiagMetric v=DiagReadMetrics();
    DiagUpdatePeak(g_diagPeaks.draw0,v.draw0);
    DiagUpdatePeak(g_diagPeaks.draw1,v.draw1);
    DiagUpdatePeak(g_diagPeaks.frame,v.frame);
    DiagUpdatePeak(g_diagPeaks.filter,v.filter);
    DiagUpdatePeak(g_diagPeaks.pending,v.pending);
    DiagUpdatePeak(g_diagPeaks.released,v.released);
    DiagUpdatePeak(g_diagPeaks.submodel,v.submodel);
    DiagUpdatePeak(g_diagPeaks.gfxBusy,v.gfxBusy);
    DiagUpdatePeak(g_diagPeaks.gfxMaxRaw,v.gfxMaxRaw);
    // Sample all counters at 200 ms. Print each second and on threshold crossings.
    if ((loop%5u)!=0u) return;
    char line[900]={};char* c=line;const char* end=line+sizeof(line)-1;
    AppendText(c,"DSFV29c_RESOURCE tick=",end);DiagAppendU32(c,GetTickCount()-g_diagStartTick,end);
#define DIAG_VAL(literal,val) do {AppendText(c,literal,end);AppendNumber(c,val,end);} while(0)
    DIAG_VAL(" draw0=",v.draw0);DIAG_VAL(" draw1=",v.draw1);
    DIAG_VAL(" peakDraw0=",g_diagPeaks.draw0);DIAG_VAL(" peakDraw1=",g_diagPeaks.draw1);
    DIAG_VAL(" frame=",v.frame);DIAG_VAL(" peakFrame=",g_diagPeaks.frame);
    DIAG_VAL(" filter=",v.filter);DIAG_VAL(" peakFilter=",g_diagPeaks.filter);
    DIAG_VAL(" pending=",v.pending);DIAG_VAL(" released=",v.released);
    DIAG_VAL(" submodel=",v.submodel);DIAG_VAL(" peakSubmodel=",g_diagPeaks.submodel);
    DIAG_VAL(" gfxBusyRaw=",v.gfxBusy);DIAG_VAL(" gfxMaxRaw=",v.gfxMaxRaw);
    DIAG_VAL(" map=",DSF150_CompletedActive);DIAG_VAL(" ext=",DSFV27B_MapExternal);
    DIAG_VAL(" c7pre=",DSF_Chapter7PreloadHits);DIAG_VAL(" reqHigh=",DSFV27B_RequestHighWater);
    DIAG_VAL(" hashLeft=",DSFV29_HashNativeRemaining);DIAG_VAL(" hashExtra=",DSFV29_HashExtraGranted);
    DIAG_VAL(" destroyActual=",DSFV29c_DestroyActualCount);
    AppendText(c," destroyRedirect=",end);
    AppendHex32(c,static_cast<U32>(DSFV29c_DestroyRedirectDword),end);
    DIAG_VAL(" cbHooks=",DSFV29c_CallbackHooks);
    DIAG_VAL(" cbSuspicious=",DSFV29c_CallbackSuspicious);
    DIAG_VAL(" boundary=",DSFV27B_UnsafeBoundary);
#undef DIAG_VAL
    AppendText(c,"\n",end);*c=0;
    DiagPrint(line);
    const DiagMapSample map=DiagReadMap();
    if(map.valid) {
        bool changed=(g_diagPreviousMapCount!=map.active || g_diagPreviousIdCount!=map.active);
        if(!changed)for(U32 i=0;i<map.active;++i)
            if(!DiagHasId(g_diagPreviousIds,g_diagPreviousIdCount,map.ids[i])) {changed=true;break;}
        if(changed || map.duplicates || map.nullHandlers) DiagMapEvent(map);
    } else if(g_pkg0 && (loop%25u)==0u) {
        DiagPrint("DSFV29c_MAP_UNSTABLE header/list changed, or unreadable node; retry after quiescence\n");
    }
    if(v.draw0>=15000 || v.draw1>=15000 || v.frame>=3800000 ||
       v.filter>=3300000 || v.pending>=15000 || v.released>=30000 ||
       v.submodel>=14000 || v.filter==-2 || DSFV27B_UnsafeBoundary)
        DiagPrint("DSFV29c_WARNING sampled resource pressure/invalid cursor/boundary; capture screenshot and log now\n");
}
static void DiagDrainCallbacks() {
    static long drained=0;
    const long total=DSFV29c_CallbackTotalEvents;
    const long oldest=(total>static_cast<long>(kDiagRingSize)) ?
        total-static_cast<long>(kDiagRingSize) : 0;
    if(drained<oldest) {
        DSFV29c_CallbackEventDrops+=oldest-drained;
        drained=oldest;
    }
    U32 taken=0;
    while(drained<total && taken<24u) {
        const long seq=drained+1;
        const DiagCallbackEvent& e=g_diagEvents[static_cast<U32>(seq-1)%kDiagRingSize];
        if(e.sequence!=seq)break; // producer not done / slot overwritten
        const DiagCallbackEvent copy={e.sequence,e.type,e.owner,e.slot,e.count,
                                      e.record,e.receiver,e.vptr,e.method,e.flags};
        if(e.sequence!=seq)break;
        char line[380]={};char* c=line;const char* end=line+sizeof(line)-1;
        AppendText(c,"DSFV29c_CB tick=",end);DiagAppendU32(c,GetTickCount()-g_diagStartTick,end);
        AppendText(c," op=",end);
        AppendText(c,copy.type==1u ? "REG" : (copy.type==2u ? "CLEAN_ITEM" : "CLEAN_BEGIN"),end);
        AppendText(c," owner=",end);AppendHex32(c,copy.owner,end);
        AppendText(c," slot=",end);DiagAppendU32(c,copy.slot,end);
        AppendText(c," count=",end);DiagAppendU32(c,copy.count,end);
        AppendText(c," ref=",end);AppendHex32(c,copy.record,end);
        AppendText(c," receiver=",end);AppendHex32(c,copy.receiver,end);
        AppendText(c," vptr=",end);AppendHex32(c,copy.vptr,end);
        AppendText(c," method8=",end);AppendHex32(c,copy.method,end);
        AppendText(c," callable=",end);DiagAppendU32(c,copy.flags&1u,end);
        AppendText(c,"\n",end);*c=0;
        DiagPrint(line);
        ++drained;++taken;
    }
}

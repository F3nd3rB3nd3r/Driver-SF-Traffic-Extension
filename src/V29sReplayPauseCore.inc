// Completion-only service. Engine calls run on the importing game thread;
// the monitor only prints counters. No clock/rate/physics writes here.
struct PauseView {
    UPtr root,owner,simulation,recording;
    U32 mode,loading,lifecycle,transition,phase,countdown,paused,rateBits,playbackBits,clockBits;
};
struct PauseMark {UPtr owner,simulation,recording;U32 thread,epoch,valid;};
static PauseMark g_pauseMark={};
static volatile long g_pauseMarkLock=0,g_pauseBusy=0;
static volatile long g_pauseUpdates=0,g_pauseImports=0,g_pauseCandidates=0,g_pauseAttempts=0;
static volatile long g_pauseCompleted=0,g_pauseHeld=0,g_pauseOwnerChanged=0,g_pauseNoImport=0;
static volatile long g_pauseSnapshotSkipped=0,g_pauseReentrant=0,g_pauseMarkDrops=0;
static UPtr PauseRoot();
static UPtr PauseAddress(U32);
static bool PauseReadView(UPtr owner,PauseView& v) {
    const UPtr root=PauseRoot();
    if(!Readable(root,0x36Cu) || Read32(root+0x22Cu)!=owner || !Readable(owner,0x4EA18u))return false;
    const UPtr simulation=Read32(root+0xCCu),recording=Read32(owner+0x4E58Cu);
    if(!Readable(simulation,0x198u) || Read32(simulation)!=PauseAddress(0x00E2CA94u) ||
       Read32(simulation+0x168u)!=PauseAddress(0x00E2CA70u) || !Readable(recording,0x8594u))return false;
    // The unchanged native completion tail calls these current-root services.
    const UPtr notifier=Read32(root+0x368u),debug=Read32(root+0x1Cu);
    if(!Readable(notifier,0x10u) || !Readable(debug,4u) || !Readable(Read32(debug),0x14u) || !Readable(Read32(Read32(debug)+0x10u),1u))return false;
    v.root=root;v.owner=owner;v.simulation=simulation;v.recording=recording;
    v.mode=Read32(owner+0x4E588u);v.loading=Read8(owner+0x4E9ECu);v.lifecycle=Read32(owner+0x4EA14u);
    v.transition=Read32(owner+0x4E9F4u);v.phase=Read32(owner+0x438ECu);v.countdown=Read32(owner+0x4E9F0u);
    v.paused=Read8(simulation+0x188u);v.rateBits=Read32(simulation+0x190u);
    v.playbackBits=Read32(owner+0x4E99Cu);v.clockBits=Read32(owner+0x4E9A0u);return true;
}

static bool PauseSnapshotOkay(UPtr);
static U32 PauseThread();
static void PauseRunOriginal(UPtr);
static void PauseRunCompletion(UPtr);
static void PauseEvent(const char*,const PauseView&,const PauseView*,U32);
static bool PauseCopyMark(PauseMark& copy) {
    if(InterlockedCompareExchange(&g_pauseMarkLock,1,0))return false;
    copy=g_pauseMark;InterlockedExchange(&g_pauseMarkLock,0);return true;
}
static void PauseForget(UPtr owner,U32 epoch=0u) {
    if(InterlockedCompareExchange(&g_pauseMarkLock,1,0))return;
    if(g_pauseMark.owner==owner && (!epoch || g_pauseMark.epoch==epoch))g_pauseMark.valid=0u;
    InterlockedExchange(&g_pauseMarkLock,0);
}
extern "C" __declspec(noinline) void __cdecl PauseOnImport(UPtr snapshot) {
    InterlockedIncrement(&g_pauseImports);
    if(snapshot<0x438E4u)return;
    PauseView view={};const UPtr owner=snapshot-0x438E4u;
    if(!PauseReadView(owner,view))return;
    if(InterlockedCompareExchange(&g_pauseMarkLock,1,0)){InterlockedIncrement(&g_pauseMarkDrops);return;}
    g_pauseMark.owner=owner;g_pauseMark.simulation=view.simulation;g_pauseMark.recording=view.recording;
    g_pauseMark.thread=PauseThread();++g_pauseMark.epoch;g_pauseMark.valid=1u;
    InterlockedExchange(&g_pauseMarkLock,0);
}
static bool PauseCandidate(const PauseView& v) {
    return v.mode==2u && v.loading==1u && v.lifecycle==5u && v.transition==0u &&
        v.phase==0u && v.countdown==0u && v.paused==1u &&
        !(v.rateBits&0x7FFFFFFFu) && !(v.playbackBits&0x7FFFFFFFu);
}
extern "C" __declspec(noinline) void __fastcall PauseUpdate(void* self,void*) {
    const UPtr owner=reinterpret_cast<UPtr>(self);
    // Preserve the complete original update, including its paused-rate path.
    PauseRunOriginal(owner);InterlockedIncrement(&g_pauseUpdates);
    if(InterlockedCompareExchange(&g_pauseBusy,1,0)){InterlockedIncrement(&g_pauseReentrant);return;}
    PauseView before={};
    if(!PauseReadView(owner,before)){PauseForget(owner);InterlockedExchange(&g_pauseBusy,0);return;}
    if(before.mode!=2u || !before.loading){PauseForget(owner);InterlockedExchange(&g_pauseBusy,0);return;}
    if(!PauseCandidate(before)){InterlockedExchange(&g_pauseBusy,0);return;}
    InterlockedIncrement(&g_pauseCandidates);PauseMark mark={};
    if(!PauseCopyMark(mark) || !mark.valid || mark.owner!=owner || mark.simulation!=before.simulation ||
       mark.recording!=before.recording || mark.thread!=PauseThread()) {
        InterlockedIncrement(&g_pauseNoImport);InterlockedExchange(&g_pauseBusy,0);return;
    }
    if(!PauseSnapshotOkay(owner)){InterlockedIncrement(&g_pauseSnapshotSkipped);InterlockedExchange(&g_pauseBusy,0);return;}
    // Revalidate identity/state immediately before entering the native tail.
    PauseView current={};
    if(!PauseReadView(owner,current) || current.root!=before.root || current.simulation!=before.simulation ||
       current.recording!=before.recording || !PauseCandidate(current)) {
        InterlockedIncrement(&g_pauseOwnerChanged);InterlockedExchange(&g_pauseBusy,0);return;
    }
    InterlockedIncrement(&g_pauseAttempts);
    // 007390BE performs the native busy check itself. A busy result keeps
    // loading set and is retried on a later update; no readiness bypass.
    PauseRunCompletion(owner);
    PauseView after={};
    if(!PauseReadView(owner,after) || after.simulation!=before.simulation || after.recording!=before.recording) {
        InterlockedIncrement(&g_pauseOwnerChanged);PauseForget(owner,mark.epoch);
        PauseEvent("ownerChanged",before,nullptr,mark.epoch);
    }else if(!after.loading) {
        InterlockedIncrement(&g_pauseCompleted);PauseForget(owner,mark.epoch);
        PauseEvent("completed",before,&after,mark.epoch);
    }else {
        InterlockedIncrement(&g_pauseHeld);PauseEvent("nativeHeld",before,&after,mark.epoch);
    }
    InterlockedExchange(&g_pauseBusy,0);
}

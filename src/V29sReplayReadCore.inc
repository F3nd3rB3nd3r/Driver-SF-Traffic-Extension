// Private, process-lifetime backing per native file-wrapper address. Metadata
// and payload are never freed while hooks can observe them; reuse needs teardown.
static const U32 kRFBytes=0x100000u,kRFReserve=0x120000u,kRFGuard=0x10000u;
enum RFKind {RFPollResult,RFCommit,RFWrap,RFSpace,RFIssue,RFEnd};
struct RFCounts {volatile long polls,backendBusy,commits,committedBytes,wrapChecks,wrapStops,spaceChecks,spaceStops,issues,issueBytes,issueRejected,endChecks,endStops;};
struct RFOwner {
    RFOwner* next;UPtr self,reservation,buffer;
    volatile long generation,state; //0=closed,1=reader,2=constructing,3=invalid
    UPtr originalBuffer;U32 originalCapacity;
    RFCounts counts;
    volatile long minFree,maxRequired,lastStop,lastStopPoll,lastStopFree,lastStopRequired;
};
struct RFView {
    U32 present,generation,state,buffer,capacity;
    U32 polls,backendBusy,commits,committedBytes,wrapChecks,wrapStops,spaceChecks,spaceStops,issues,issueBytes,issueRejected,endChecks,endStops;
    U32 minFree,maxRequired,lastStop,lastStopPoll,lastStopFree,lastStopRequired;
};
static RFOwner* volatile g_rfOwners=nullptr;
static volatile long g_rfOwnerCount=0,g_rfConstructs=0,g_rfDestructs=0,g_rfReuses=0,g_rfWriterBypass=0;
static volatile long g_rfAllocationFailed=0,g_rfFallbacks=0,g_rfFaults=0,g_rfUnmanaged=0;
static long g_rfStatus=0;
extern "C" UPtr g_rfCtorTarget=0,g_rfDtorTarget=0;
extern "C" UPtr g_rfContinue[6]={},g_rfStop[4]={};
static UPtr RFAddress(U32);static void* RFAllocate(U32,U32);static bool RFCommitMemory(UPtr,U32);static void RFFree(UPtr);
static U32 RFRead32(UPtr);static U8 RFRead8(UPtr);
static RFOwner* RFFind(UPtr self) {
    for(RFOwner* r=g_rfOwners;r;r=r->next)if(r->self==self)return r;
    return nullptr;
}
static RFOwner* RFMakeOwner(UPtr self) {
    RFOwner* r=RFFind(self);if(r)return r;
    r=reinterpret_cast<RFOwner*>(RFAllocate(0x1000u,0u));if(!r)return nullptr;
    r->reservation=reinterpret_cast<UPtr>(RFAllocate(kRFReserve,1u));
    if(!r->reservation){RFFree(reinterpret_cast<UPtr>(r));return nullptr;}
    r->buffer=r->reservation+kRFGuard;
    if(!RFCommitMemory(r->buffer,kRFBytes)){RFFree(r->reservation);RFFree(reinterpret_cast<UPtr>(r));return nullptr;}
    r->self=self;
    // Different owners may be created concurrently. Same-owner concurrent
    // construction would already violate the native wrapper's lifetime.
    do{r->next=g_rfOwners;}while(InterlockedCompareExchange(reinterpret_cast<volatile long*>(&g_rfOwners),reinterpret_cast<long>(r),reinterpret_cast<long>(r->next))!=reinterpret_cast<long>(r->next));
    InterlockedIncrement(&g_rfOwnerCount);return r;
}
typedef void* (__thiscall* RFCtorFn)(void*,const char*,void*,U32,U32);
typedef void* (__thiscall* RFDtorFn)(void*,U32);
extern "C" __declspec(noinline) void* __fastcall RFConstruct(void* object,void*,const char* name,void* original,U32 capacity,U32 mode) {
    const UPtr self=reinterpret_cast<UPtr>(object);RFOwner* r=nullptr;void* backing=original;U32 bytes=capacity;
    if(mode!=0u)InterlockedIncrement(&g_rfWriterBypass);
    else if(g_rfStatus==1 && original && capacity==0x40000u) {
        r=RFMakeOwner(self);
        if(!r){InterlockedIncrement(&g_rfAllocationFailed);InterlockedIncrement(&g_rfFallbacks);}
        else if(r->state!=0){InterlockedIncrement(&g_rfFaults);InterlockedIncrement(&g_rfFallbacks);r=nullptr;}
        else {
            if(r->generation)InterlockedIncrement(&g_rfReuses);
            InterlockedExchange(&r->state,2);InterlockedIncrement(&r->generation);
            r->originalBuffer=reinterpret_cast<UPtr>(original);r->originalCapacity=capacity;
            r->minFree=static_cast<long>(kRFBytes);r->maxRequired=r->lastStop=r->lastStopPoll=r->lastStopFree=r->lastStopRequired=0;
            backing=reinterpret_cast<void*>(r->buffer);bytes=kRFBytes;
        }
    }else {InterlockedIncrement(&g_rfFallbacks);}
    void* result=reinterpret_cast<RFCtorFn>(g_rfCtorTarget)(object,name,backing,bytes,mode);
    if(r) {
        if(result!=object || RFRead32(self)!=RFAddress(0xE2A088u) || RFRead32(self+4u)!=0u || RFRead32(self+0x855Cu)!=kRFBytes || RFRead32(self+0x8560u)!=r->buffer) {
            InterlockedIncrement(&g_rfFaults);InterlockedExchange(&r->state,3);
        }else {InterlockedIncrement(&g_rfConstructs);InterlockedExchange(&r->state,1);}
    }
    return result;
}
extern "C" __declspec(noinline) void* __fastcall RFDestruct(void* object,void*,U32 flags) {
    RFOwner* r=RFFind(reinterpret_cast<UPtr>(object));const bool managed=r && r->state==1;
    // Native dtor closes/drains the handle. Do not dereference object afterward:
    // flags may ask native code to delete its object allocation.
    void* result=reinterpret_cast<RFDtorFn>(g_rfDtorTarget)(object,flags);
    if(managed){InterlockedExchange(&r->state,0);InterlockedIncrement(&g_rfDestructs);}
    return result;
}
static void RFStopAt(RFOwner* r,U32 kind,U32 free,U32 required) {
    r->lastStop=static_cast<long>(kind);r->lastStopPoll=r->counts.polls;
    r->lastStopFree=static_cast<long>(free);r->lastStopRequired=static_cast<long>(required);
}
// Only integer counters and already-valid native reader fields. No clock,
// VirtualQuery, allocation, engine method call or file output on this path.
extern "C" __declspec(noinline) void __cdecl RFObserve(UPtr self,U32 kind,U32 value,U32 required) {
    RFOwner* r=RFFind(self);
    if(!r || r->state!=1){InterlockedIncrement(&g_rfUnmanaged);return;}
    if(RFRead32(self+0x8560u)!=r->buffer || RFRead32(self+0x855Cu)!=kRFBytes){InterlockedIncrement(&g_rfFaults);return;}
    RFCounts& c=r->counts;
    if(kind==RFPollResult){InterlockedIncrement(&c.polls);if(value&255u){InterlockedIncrement(&c.backendBusy);RFStopAt(r,1u,0u,0u);}}
    else if(kind==RFCommit){InterlockedIncrement(&c.commits);InterlockedExchangeAdd(&c.committedBytes,static_cast<long>(RFRead32(self+0x8584u)));}
    else if(kind==RFWrap || kind==RFSpace) {
        if(value<static_cast<U32>(r->minFree))r->minFree=static_cast<long>(value);
        if(required>static_cast<U32>(r->maxRequired))r->maxRequired=static_cast<long>(required);
        if(kind==RFWrap){InterlockedIncrement(&c.wrapChecks);if(value<required){InterlockedIncrement(&c.wrapStops);RFStopAt(r,2u,value,required);}}
        else {InterlockedIncrement(&c.spaceChecks);if(value<=0x8000u){InterlockedIncrement(&c.spaceStops);RFStopAt(r,3u,value,0x8000u);}}
    }else if(kind==RFIssue){InterlockedIncrement(&c.issues);InterlockedExchangeAdd(&c.issueBytes,static_cast<long>(RFRead32(self+0x8584u)));if(!(value&255u))InterlockedIncrement(&c.issueRejected);}
    else if(kind==RFEnd){InterlockedIncrement(&c.endChecks);if(RFRead8(self+0x858Cu)){InterlockedIncrement(&c.endStops);RFStopAt(r,4u,0u,0u);}}
}
static RFView RFCapture(UPtr self) {
    RFView v={};RFOwner* r=RFFind(self);if(!r)return v;
    v.present=1u;v.generation=static_cast<U32>(r->generation);v.state=static_cast<U32>(r->state);v.buffer=r->buffer;v.capacity=kRFBytes;
    const RFCounts& c=r->counts;
    v.polls=c.polls;v.backendBusy=c.backendBusy;v.commits=c.commits;v.committedBytes=c.committedBytes;
    v.wrapChecks=c.wrapChecks;v.wrapStops=c.wrapStops;v.spaceChecks=c.spaceChecks;v.spaceStops=c.spaceStops;
    v.issues=c.issues;v.issueBytes=c.issueBytes;v.issueRejected=c.issueRejected;v.endChecks=c.endChecks;v.endStops=c.endStops;
    v.minFree=r->minFree;v.maxRequired=r->maxRequired;v.lastStop=r->lastStop;v.lastStopPoll=r->lastStopPoll;v.lastStopFree=r->lastStopFree;v.lastStopRequired=r->lastStopRequired;
    return v;
}

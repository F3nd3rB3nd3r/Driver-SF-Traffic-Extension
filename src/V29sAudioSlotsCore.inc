// Native-thread observations only. The monitor reads counters, never live
// middleware objects, vectors or records which can be destroyed concurrently.
struct AudioSlotsList {
    volatile long attempts,inserts,ceiling,reuses,peakBefore,peakInserted;
    volatile long updateCount,updatePeak,updates,invalid,owner,configured,allocated;
};
static AudioSlotsList g_audioSlotsLists[3]={};
static volatile long g_audioSlotsUserAttempts[2]={},g_audioSlotsUserFull[2]={},g_audioSlotsUserPeak[2]={};
static volatile long g_audioSlotsInvalid=0;
#if defined(DSF_V29S_GENERIC_LIMITS)
#define kAudioSlotsCount (g_limits.audioSlots)
#else
static const U32 kAudioSlotsCount=2048u;
#endif
static U32 AudioSlotsCount(UPtr header,bool& valid) {
    const U32 begin=Read32(header),end=Read32(header+4u);
    valid=end>=begin && ((end-begin)&3u)==0u;
    return valid?(end-begin)/4u:0u;
}
extern "C" __declspec(noinline) void __cdecl AudioSlotsEntry(U32 kind,UPtr pointer) {
    if(kind==5u || kind==6u) {
        const U32 index=kind-5u;
        bool valid=true;
        const U32 count=index?Read8(pointer+0x39u):AudioSlotsCount(pointer+0x24u,valid);
        if(!valid){InterlockedIncrement(&g_audioSlotsInvalid);return;}
        InterlockedIncrement(&g_audioSlotsUserAttempts[index]);
        CapacityMax(g_audioSlotsUserPeak[index],count);
        if(count>=(index?255u:8u))InterlockedIncrement(&g_audioSlotsUserFull[index]);
        return;
    }
    const bool managerB=kind==1u || kind==3u || kind==8u;
    const UPtr owner=managerB?pointer:Read32(Address(0x01D7184Cu));
    const UPtr header=managerB?owner+4u:pointer;
    U32 index=2u;
    if(!managerB) {
        if(header==owner+8u)index=0u;
        else if(header==owner+0x14u)index=1u;
        else {InterlockedIncrement(&g_audioSlotsInvalid);return;}
    }
    AudioSlotsList& s=g_audioSlotsLists[index];
    const U32 max=Read32(owner+(managerB?0x10u:4u)),capacity=Read32(header+8u);
    bool valid=false;const U32 count=AudioSlotsCount(header,valid);
    if(!valid || count>capacity || capacity>kAudioSlotsCount || max>kAudioSlotsCount) {
        InterlockedIncrement(&s.invalid);return;
    }
    InterlockedExchange(&s.owner,static_cast<long>(owner));
    InterlockedExchange(&s.configured,static_cast<long>(max));
    InterlockedExchange(&s.allocated,static_cast<long>(capacity));
    if(kind<=1u) {
        InterlockedIncrement(&s.attempts);CapacityMax(s.peakBefore,count);
        if(count>=max)InterlockedIncrement(&s.ceiling);
    } else if(kind==2u || kind==3u) {
        InterlockedIncrement(&s.inserts);CapacityMax(s.peakInserted,count);
    } else if(kind==4u)InterlockedIncrement(&s.reuses);
    else if(kind==7u || kind==8u) {
        InterlockedExchange(&s.updateCount,static_cast<long>(count));
        CapacityMax(s.updatePeak,count);InterlockedIncrement(&s.updates);
    }
}

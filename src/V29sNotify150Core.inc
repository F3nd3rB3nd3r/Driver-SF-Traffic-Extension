// Scoped replacement for the traffic-notification memcpy call, not global memcpy.
static volatile long g_notifyCopies=0,g_notifyStock=0,g_notifySidecar=0,g_notifyInvalid=0,g_notifyPeak=0;
static void NotifyFatal(const char* reason,UPtr source,U32 bytes);
static void NotifyEvent(UPtr source,U32 count,U32 hash,U32 first,U32 last,bool sidecar);
extern "C" __declspec(noinline) void* __cdecl NotifyCopy(void* destination,const void* source,U32 bytes) {
    const UPtr owner=reinterpret_cast<UPtr>(source);const U32 count=bytes/4u;
    U32 capacity=150u;
#if defined(DSF_V29S_GENERIC_LIMITS)
    capacity=g_limits.requests;
#endif
    if((bytes&3u) || count>capacity || (bytes && (!destination || !source))) {
        InterlockedIncrement(&g_notifyInvalid);NotifyFatal("copyBounds",owner,bytes);return nullptr;
    }
    InterlockedIncrement(&g_notifyCopies);U32 hash=2166136261u,first=0u,last=0u;
    if(count>50u) {
        RequestLock();RequestSidecar* slot=RequestSlot(owner,false);
        bool valid=slot && slot->count==count && Read32(owner+0xC8u)==count;
        if(valid)for(U32 i=0;i<50u;++i)if(Read32(owner+i*4u)!=slot->ids[i])valid=false;
        if(!valid) {
            RequestUnlock();InterlockedIncrement(&g_notifyInvalid);
            NotifyFatal("missingOrStaleRequestSidecar",owner,bytes);return nullptr;
        }
        for(U32 i=0;i<count;++i){const U32 id=slot->ids[i];reinterpret_cast<U32*>(destination)[i]=id;
            if(!i)first=id;last=id;hash=(hash^id)*16777619u;}
        RequestUnlock();InterlockedIncrement(&g_notifySidecar);
    }else {
        // Preserve the native inline source and memcpy return convention.
        if(bytes)memcpy(destination,source,bytes);
        for(U32 i=0;i<count;++i){const U32 id=reinterpret_cast<const U32*>(destination)[i];
            if(!i)first=id;last=id;hash=(hash^id)*16777619u;}
        InterlockedIncrement(&g_notifyStock);
    }
    if(count>static_cast<U32>(g_notifyPeak))g_notifyPeak=static_cast<long>(count);
    NotifyEvent(owner,count,hash,first,last,count>50u);return destination;
}

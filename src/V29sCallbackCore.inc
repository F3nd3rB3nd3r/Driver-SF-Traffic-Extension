// Process-lived callback storage. Preserve all native headers/sentinels and ABI.
#if defined(DSF_V29S_WIDE_LIMITS)
static const U32 kCallbackGroups=21u;
#define kCallbackCapacity (g_limits.callbacks)
#define kCallbackExtra (g_limits.callbacks-64u)
#else
static const U32 kCallbackGroups=21u,kCallbackCapacity=512u,kCallbackExtra=448u;
#endif
static const U32 kCallbackGuard=0x43423531u;
struct CallbackNode {UPtr previous,next,payload;};
struct CallbackGroup {
    U32 before[4];
#if defined(DSF_V29S_WIDE_LIMITS)
CallbackNode* nodes;U32* after;
#else
CallbackNode nodes[kCallbackExtra];U32 after[4];
#endif
    U32 inlineUsed,extraUsed;
    volatile long thread,depth,pollDepth,attempts,stores,redirects,reuses,peak,issuedPeak;
    volatile long active,free,polls,removed,lastHandler,lastPayload;
};
static_assert(sizeof(CallbackNode)==12u,"Native callback node");
static CallbackGroup* g_callbackGroups=nullptr;
static long g_callbackStatus=0;static const char* g_callbackReason="notAttempted";
static volatile long g_callbackInvalid=0,g_callbackFull=0,g_callbackGuardBad=0,g_callbackConcurrent=0;
typedef UPtr (__thiscall* CallbackNativeInsert)(void*,UPtr);
typedef U8 (__thiscall* CallbackNativePoll)(void*);
static CallbackNativeInsert g_callbackNativeInsert=nullptr;
static CallbackNativePoll g_callbackNativePoll=nullptr;
static void CallbackFatal(const char* reason,UPtr header);
static void CallbackEvent(U32 slot,const char* kind);
static UPtr CallbackHeader(U32 slot){return Address(0x014427ECu)+slot*0x314u;}
static bool CallbackSlot(UPtr header,U32& slot) {
    const UPtr first=CallbackHeader(0u);
    if(header<first || header-first>=kCallbackGroups*0x314u || (header-first)%0x314u)return false;
    slot=(header-first)/0x314u;return true;
}
static bool CallbackBad(const char* reason) {g_callbackReason=reason;InterlockedIncrement(&g_callbackInvalid);return false;}
static bool CallbackEnter(CallbackGroup& g) {
    const long tid=static_cast<long>(GetCurrentThreadId());
    const long owner=InterlockedCompareExchange(&g.thread,tid,0);
    if(owner && owner!=tid){InterlockedIncrement(&g_callbackConcurrent);g_callbackReason="concurrentGroupAccess";return false;}
    ++g.depth;return true; // Same-thread registration from a native callback is allowed.
}
static void CallbackLeave(CallbackGroup& g){if(!--g.depth)InterlockedExchange(&g.thread,0);}
static bool CallbackIndex(U32 slot,const CallbackGroup& g,UPtr node,U32& index) {
    const UPtr first=CallbackHeader(slot)+12u,extra=reinterpret_cast<UPtr>(&g.nodes[0]);
    if(node>=first && node-first<g.inlineUsed*12u && !((node-first)%12u)){index=(node-first)/12u;return true;}
    if(node>=extra && node-extra<g.extraUsed*12u && !((node-extra)%12u)){index=64u+(node-extra)/12u;return true;}
    return false;
}
static bool CallbackValidate(U32 slot) {
    CallbackGroup& g=g_callbackGroups[slot];const UPtr h=CallbackHeader(slot),end=h+0x30Cu;
    for(U32 n=0;n<4u;++n)if(g.before[n]!=kCallbackGuard || g.after[n]!=kCallbackGuard){
        InterlockedIncrement(&g_callbackGuardBad);g_callbackReason="arenaGuard";return false;}
    const U32 active=Read32(h+8u),free=Read32(h+4u),issued=g.inlineUsed+g.extraUsed;
    if(g.inlineUsed>64u || g.extraUsed>kCallbackExtra || active>kCallbackCapacity || free>kCallbackCapacity || active+free!=issued)
        return CallbackBad("countsOrReset");

#if defined(DSF_V29S_WIDE_LIMITS)
WideScratchLease seenLease(((kCallbackCapacity+31u)/32u)*4u);U32* seen=static_cast<U32*>(seenLease.data);U32 count=0,index=0;
#else
U32 seen[16]={},count=0,index=0;
#endif
UPtr previous=end,node=Read32(end+4u);
    while(node!=end) {
        if(count>=active || !CallbackIndex(slot,g,node,index) || (seen[index/32u]&(1u<<(index%32u))))return CallbackBad("activeChain");
        seen[index/32u]|=1u<<(index%32u);
        if(Read32(node)!=previous)return CallbackBad("backLink");
        previous=node;node=Read32(node+4u);++count;
    }
    if(count!=active || Read32(end)!=previous)return CallbackBad("sentinel");
    node=Read32(h);
    for(U32 n=0;n<free;++n) {
        if(!CallbackIndex(slot,g,node,index) || (seen[index/32u]&(1u<<(index%32u))))return CallbackBad("recycledChain");
        seen[index/32u]|=1u<<(index%32u);node=Read32(node);
    }
    const UPtr fresh=g.extraUsed?reinterpret_cast<UPtr>(&g.nodes[0])+g.extraUsed*12u:h+12u+g.inlineUsed*12u;
    if(node!=fresh)return CallbackBad("freshCursor");
    g.active=static_cast<long>(active);g.free=static_cast<long>(free);
    if(active>static_cast<U32>(g.peak))g.peak=static_cast<long>(active);
    if(issued>static_cast<U32>(g.issuedPeak))g.issuedPeak=static_cast<long>(issued);
    return true;
}
static bool CallbackReady(U32 slot) {
    if(g_callbackStatus!=1 || !g_callbackGroups || slot>=kCallbackGroups)return CallbackBad("notArmedOrSelector");
    return true;
}
static bool CallbackHandlerSlot(UPtr handler,U32& slot) {
    if(!Readable(handler,0x570u))return CallbackBad("handlerUnreadable");
    slot=Read32(handler+0x560u);
    if(slot>=kCallbackGroups)return CallbackBad("selectorRange");
    return CallbackReady(slot);
}
static UPtr __fastcall CallbackInsertBridge(void* self,void*,UPtr argument) {
    const UPtr h=reinterpret_cast<UPtr>(self);U32 slot=0;
    if(!CallbackSlot(h,slot))return g_callbackNativeInsert(self,argument);
    if(!CallbackReady(slot)){CallbackFatal(g_callbackReason,h);return 0u;}
    CallbackGroup& g=g_callbackGroups[slot];
    if(!CallbackEnter(g)){CallbackFatal(g_callbackReason,h);return 0u;}
    ++g.attempts;
    if(!CallbackValidate(slot) || !Readable(argument,4u)) {
        if(!Readable(argument,4u))CallbackBad("argumentUnreadable");
        CallbackLeave(g);CallbackFatal(g_callbackReason,h);return 0u;
    }
    const U32 active=Read32(h+8u),free=Read32(h+4u),payload=Read32(argument);
    const bool fresh=free==0u,extra=fresh && g.inlineUsed==64u;
    if(active>=kCallbackCapacity || (extra && g.extraUsed==kCallbackExtra)) {
        InterlockedIncrement(&g_callbackFull);CallbackLeave(g);CallbackFatal("capacity",h);return 0u;
    }
    // Redirect only a fresh cursor, before the native65th payload write.
    if(extra && !g.extraUsed){Write32(h,reinterpret_cast<UPtr>(&g.nodes[0]));++g.redirects;CallbackEvent(slot,"redirect");}
    const UPtr expected=Read32(h);
    if(fresh){if(extra)++g.extraUsed;else ++g.inlineUsed;}else ++g.reuses;
    const UPtr result=g_callbackNativeInsert(self,argument);
    if(result!=expected || Read32(result+8u)!=payload || Read32(h+8u)!=active+1u || !CallbackValidate(slot)) {
        CallbackBad("nativeInsertResult");CallbackLeave(g);CallbackFatal(g_callbackReason,h);return 0u;
    }
    ++g.stores;g.lastPayload=static_cast<long>(payload);CallbackLeave(g);return result;
}
static U8 __fastcall CallbackPollBridge(void* self,void*) {
    const UPtr handler=reinterpret_cast<UPtr>(self);U32 slot=0;
    if(!CallbackHandlerSlot(handler,slot)){CallbackFatal(g_callbackReason,0u);return 0u;}
    const UPtr h=CallbackHeader(slot);CallbackGroup& g=g_callbackGroups[slot];
    if(!CallbackEnter(g)){CallbackFatal(g_callbackReason,h);return 0u;}
    if(g.pollDepth || !CallbackValidate(slot)) {
        if(g.pollDepth)CallbackBad("recursiveGroupPoll");
        CallbackLeave(g);CallbackFatal(g_callbackReason,h);return 0u;
    }
    ++g.pollDepth;++g.polls;g.lastHandler=static_cast<long>(handler);
    const U32 active=Read32(h+8u),stores=static_cast<U32>(g.stores);
    const U8 result=g_callbackNativePoll(self);
    const U32 available=active+static_cast<U32>(g.stores)-stores,after=Read32(h+8u);
    if(after>available || !CallbackValidate(slot)) {
        CallbackBad("nativePollResult");--g.pollDepth;CallbackLeave(g);CallbackFatal(g_callbackReason,h);return 0u;
    }
    g.removed+=static_cast<long>(available-after);--g.pollDepth;CallbackLeave(g);return result;
}

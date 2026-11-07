// Read-only indexed resource-handle link trace. The fault at 00BAEBB2 is
// in this chain, not in the separate 45000-node hash arena free list.
static const U32 kVHandleEvents=4096u,kVHandleSlots=0x20000u;
struct DiagVHandleEvent {
    volatile long seq;
    U32 tick,op,slot,before,after,pair,pairHigh,issue;
};
static DiagVHandleEvent g_vHandleEvents[kVHandleEvents]={};
static volatile long g_vHandleNext=0,g_vHandleIssue=0,g_vHandleEmergency=0;
static volatile long g_vHandleHooks=0;
static volatile long g_vHandleOwner=0;
static U32 g_vHandleDrained=0u,g_vHandleMissed=0u;
static DiagFDetour g_vHandleDetours[8]={};
static UPtr g_vHandleTramps[8]={};
struct DiagVSlotLife {
    volatile long state; // 0 unseen, 1 handle written, 2 freed, 3 link rejected
    U32 generation,allocTick,freeTick,allocHandle,freeHandle;
    U32 allocKey,freeCaller,entryHandle,entryCaller;
    U32 rejectTick,rejectHandle,rejectNeighbor,rejectHigh,rejectNode;
    U32 nextTick,nextSite,nextValue,nextHigh,nextIssue,nextBefore;
};
static DiagVSlotLife g_vSlotLife[kVHandleSlots]={};
static volatile long g_vLifeAlloc=0,g_vLifeFree=0,g_vLifeReject=0,g_vLifeSuspect=0;
static volatile long g_vNextWrites=0,g_vNextBad=0;
static U32 g_vLifeAllocPrinted=0u,g_vLifeFreePrinted=0u,g_vLifeRejectPrinted=0u;
static U32 g_vNextWritesPrinted=0u;

static void DiagVLifePrint(const char* prefix,U32 slot,U32 handle,U32 caller,
                           const DiagVSlotLife& life,U32 prior,U32 first,U32 second) {
    char line[700]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,prefix,end);
    DiagFAppendKeyU(q," tick=",GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyHex(q," slot=",slot,end);
    DiagFAppendKeyHex(q," handle=",handle,end);
    DiagFAppendKeyHex(q," caller=",caller,end);
    DiagFAppendKeyU(q," prior=",prior,end);
    DiagFAppendKeyU(q," gen=",life.generation,end);
    DiagFAppendKeyU(q," allocTick=",life.allocTick,end);
    DiagFAppendKeyHex(q," allocHandle=",life.allocHandle,end);
    DiagFAppendKeyHex(q," allocKey=",life.allocKey,end);
    DiagFAppendKeyU(q," freeTick=",life.freeTick,end);
    DiagFAppendKeyHex(q," freeHandle=",life.freeHandle,end);
    DiagFAppendKeyHex(q," freeCaller=",life.freeCaller,end);
    DiagFAppendKeyU(q," rejectTick=",life.rejectTick,end);
    DiagFAppendKeyHex(q," rejectHandle=",life.rejectHandle,end);
    DiagFAppendKeyHex(q," rejectNeighbor=",life.rejectNeighbor,end);
    DiagFAppendKeyHex(q," rejectHigh=",life.rejectHigh,end);
    DiagFAppendKeyHex(q," rejectNode=",life.rejectNode,end);
    DiagFAppendKeyU(q," nextTick=",life.nextTick,end);
    DiagFAppendKeyHex(q," nextSite=",life.nextSite,end);
    DiagFAppendKeyHex(q," nextValue=",life.nextValue,end);
    DiagFAppendKeyHex(q," nextHigh=",life.nextHigh,end);
    DiagFAppendKeyU(q," nextIssue=",life.nextIssue,end);
    DiagFAppendKeyHex(q," nextBefore=",life.nextBefore,end);
    DiagFAppendKeyHex(q," first=",first,end);
    DiagFAppendKeyHex(q," second=",second,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
}
static void DiagVLifeSuspect(const char* prefix,U32 slot,U32 handle,U32 caller,
                             const DiagVSlotLife& life,U32 prior,U32 first,U32 second) {
    const long n=InterlockedIncrement(&g_vLifeSuspect);
    if(n<=8)
        DiagVLifePrint(prefix,slot,handle,caller,life,prior,first,second);
}
extern "C" __declspec(noinline) void __cdecl DiagVLifeAllocate(UPtr owner,UPtr slot,
                                      UPtr handle,U32 key,U32 caller) {
    if(!owner || slot<owner+0x17E094u)return;
    const U32 delta=slot-(owner+0x17E094u);
    if((delta&7u)!=0u || delta/8u>=kVHandleSlots)return;
    DiagVSlotLife& life=g_vSlotLife[delta/8u];
    const U32 prior=static_cast<U32>(InterlockedExchange(&life.state,1));
    ++life.generation;life.allocTick=GetTickCount()-g_diagStartTick;
    life.allocHandle=handle;life.allocKey=key;
    life.entryHandle=0u;life.entryCaller=0u;
    life.rejectTick=0u;life.rejectHandle=0u;
    life.rejectNeighbor=0u;life.rejectHigh=0u;
    life.rejectNode=0u;
    InterlockedExchange(&g_vHandleOwner,static_cast<long>(owner));
    life.nextTick=0u;life.nextSite=0u;life.nextValue=0u;
    life.nextHigh=0u;life.nextIssue=0u;life.nextBefore=0u;
    InterlockedIncrement(&g_vLifeAlloc);
    if(prior==1u)DiagVLifeSuspect("DSFV29s_LIFE_ALLOC_BUSY",slot,handle,caller,
                                  life,prior,Read32(slot),Read32(slot+4u));
}
extern "C" __declspec(noinline) void __cdecl DiagVLifeReject(UPtr owner,UPtr slot,
                                    UPtr handle,UPtr neighbor,U32 rawHigh,UPtr node) {
    const U32 index=rawHigh&0x7FFFFFFFu;
    if(index<kVHandleSlots || !owner || slot<owner+0x17E094u)return;
    const U32 delta=slot-(owner+0x17E094u);
    if((delta&7u)!=0u || delta/8u>=kVHandleSlots)return;
    DiagVSlotLife& life=g_vSlotLife[delta/8u];
    life.rejectTick=GetTickCount()-g_diagStartTick;
    life.rejectHandle=handle;life.rejectNeighbor=neighbor;
    life.rejectHigh=rawHigh;
    life.rejectNode=node;
    InterlockedExchange(&life.state,3);
    const long n=InterlockedIncrement(&g_vLifeReject);
    if(n<=16)DiagVLifePrint("DSFV29s_LIFE_REJECT",slot,handle,0x00BAE8AEu,
                             life,3u,Read32(slot),Read32(slot+4u));
}
extern "C" __declspec(noinline) void __cdecl DiagVLifeFree(UPtr owner,UPtr slot,
                                      U32 oldHead,U32 caller,U32 explicitHandle) {
    if(!owner || slot<owner+0x17E094u)return;
    const U32 delta=slot-(owner+0x17E094u);
    if((delta&7u)!=0u || delta/8u>=kVHandleSlots)return;
    DiagVSlotLife& life=g_vSlotLife[delta/8u];
    const U32 prior=static_cast<U32>(InterlockedExchange(&life.state,2));
    const U32 entryHandle=explicitHandle?explicitHandle:life.entryHandle;
    const U32 entryCaller=explicitHandle?caller:life.entryCaller;
    if(prior==2u)DiagVLifeSuspect("DSFV29s_LIFE_FREE_TWICE",slot,entryHandle,
                                  caller,life,prior,Read32(slot),oldHead);
    life.freeTick=GetTickCount()-g_diagStartTick;
    life.freeHandle=entryHandle;life.freeCaller=entryCaller?entryCaller:caller;
    InterlockedIncrement(&g_vLifeFree);
}

static U32 DiagVHandleCheckPair(UPtr pair,U32& high) {
    high=0u;
    if(!pair)return 0u;
    if(!Readable(pair,8u))return 1u;
    high=Read32(pair+4u);
    return (high&0x7FFFFFFFu)>=kVHandleSlots?2u:0u;
}
static void DiagVNextWrite(UPtr owner,UPtr slot,UPtr pair,U32 site) {
    if(!owner || slot<owner+0x17E094u)return;
    const U32 delta=slot-(owner+0x17E094u);
    if((delta&7u)!=0u || delta/8u>=kVHandleSlots || !Readable(slot+4u,4u))return;
    DiagVSlotLife& life=g_vSlotLife[delta/8u];
    U32 high=0u;
    const U32 issue=DiagVHandleCheckPair(pair,high);
    life.nextTick=GetTickCount()-g_diagStartTick;
    life.nextSite=site;life.nextValue=pair;
    life.nextHigh=high;life.nextIssue=issue;
    life.nextBefore=Read32(slot+4u);
    InterlockedIncrement(&g_vNextWrites);
    if(issue && InterlockedIncrement(&g_vNextBad)<=8)
        DiagVLifePrint("DSFV29s_NEXT_BAD_WRITE",slot,life.allocHandle,
                       site,life,static_cast<U32>(life.state),Read32(slot),pair);
}
static void DiagVHandlePrintEvent(const DiagVHandleEvent& e,const char* prefix) {
    char line[280]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,prefix,end);
    DiagFAppendKeyU(q," tick=",e.tick,end);
    DiagFAppendKeyU(q," op=",e.op,end);
    DiagFAppendKeyHex(q," slot=",e.slot,end);
    DiagFAppendKeyHex(q," before=",e.before,end);
    DiagFAppendKeyHex(q," after=",e.after,end);
    DiagFAppendKeyHex(q," pair=",e.pair,end);
    DiagFAppendKeyHex(q," high=",e.pairHigh,end);
    DiagFAppendKeyU(q," issue=",e.issue,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
}
static void DiagVHandleRecord(U32 op,UPtr slot,U32 before,U32 after,UPtr pair,U32 high,U32 issue) {
    const long sequence=InterlockedIncrement(&g_vHandleNext);
    if(sequence<=0)return;
    DiagVHandleEvent& e=g_vHandleEvents[(static_cast<U32>(sequence)-1u)%kVHandleEvents];
    e.tick=GetTickCount()-g_diagStartTick;e.op=op;e.slot=slot;
    e.before=before;e.after=after;e.pair=pair;e.pairHigh=high;e.issue=issue;
    InterlockedExchange(&e.seq,sequence);
    if(!issue)return;
    InterlockedIncrement(&g_vHandleIssue);
    if(InterlockedCompareExchange(&g_vHandleEmergency,1,0)!=0)return;
    DiagVHandlePrintEvent(e,"DSFV29s_HANDLE_BAD");
    const U32 first=sequence>12?static_cast<U32>(sequence-12):1u;
    for(U32 n=first;n<static_cast<U32>(sequence);++n) {
        const DiagVHandleEvent& prior=g_vHandleEvents[(n-1u)%kVHandleEvents];
        if(static_cast<U32>(prior.seq)==n)DiagVHandlePrintEvent(prior,"DSFV29s_HANDLE_RECENT");
    }
}
extern "C" __declspec(noinline) void __cdecl DiagVHandleLinkFirst(UPtr owner,U32 index,UPtr pair) {
    if(index>=kVHandleSlots || !owner)return;
    const UPtr slot=owner+0x17E094u+index*8u;
    if(!Readable(slot,4u))return;
    U32 high=0u;
    const U32 issue=DiagVHandleCheckPair(pair,high);
    DiagVHandleRecord(1u,slot,Read32(slot),pair,pair,high,issue);
}
extern "C" __declspec(noinline) void __cdecl DiagVHandleLinkSecond(UPtr owner,UPtr slot,UPtr pair) {
    if(!slot || !Readable(slot+4u,4u))return;
    DiagVNextWrite(owner,slot,pair,0x00BAE8C4u);
    U32 high=0u;
    const U32 issue=DiagVHandleCheckPair(pair,high);
    DiagVHandleRecord(2u,slot+4u,Read32(slot+4u),pair,pair,high,issue);
}
extern "C" __declspec(noinline) void __cdecl DiagVHandleUnlinkNext(UPtr owner,
                                                              UPtr predecessorSlot,
                                                              UPtr currentSlot) {
    if(!predecessorSlot || !currentSlot || !Readable(currentSlot+4u,4u))return;
    // The instruction at 00BAEB8B loads this value; if EBP is nonzero,
    // 00BAEBB2 subsequently writes it into predecessorSlot+4.
    DiagVNextWrite(owner,predecessorSlot,Read32(currentSlot+4u),0x00BAEBB2u);
}
extern "C" __declspec(noinline) void __cdecl DiagVHandleUnlink(UPtr owner,UPtr frame) {
    if(!owner || !frame || !Readable(frame+4u,4u))return;
    const UPtr handle=Read32(frame+4u);
    if(!handle || !Readable(handle,8u))return;
    const U32 high=Read32(handle+4u);
    if(!(high&0x80000000u))return; // the linear-collection branch
    const U32 index=high&0x7FFFFFFFu;
    if(index>=kVHandleSlots) {
        DiagVHandleRecord(3u,0u,0u,0u,handle,high,3u);return;
    }
    const UPtr slot=owner+0x17E094u+index*8u;
    if(!Readable(slot,8u)) {
        DiagVHandleRecord(3u,slot,0u,0u,handle,high,4u);return;
    }
    const UPtr previous=Read32(slot),next=Read32(slot+4u);
    DiagVSlotLife& life=g_vSlotLife[index];
    const U32 prior=static_cast<U32>(life.state);
    const U32 caller=Readable(frame,4u)?Read32(frame):0u;
    if(prior==2u)DiagVLifeSuspect("DSFV29s_LIFE_UNLINK_FREE",slot,handle,
                                  caller,life,prior,previous,next);
    life.entryHandle=handle;life.entryCaller=caller;
    U32 previousHigh=0u,nextHigh=0u;
    const U32 previousIssue=DiagVHandleCheckPair(previous,previousHigh);
    const U32 nextIssue=DiagVHandleCheckPair(next,nextHigh);
    if((previousIssue || nextIssue) && g_vHandleEmergency==0)
        DiagVLifePrint("DSFV29s_LIFE_BAD_CONTEXT",slot,handle,caller,
                       life,prior,previous,next);
    // Record the exact neighbor the crashing unlink uses, plus the other
    // neighbor as a separate event if it is malformed.
    DiagVHandleRecord(3u,slot,previous,next,previous,previousHigh,
        previousIssue?5u+previousIssue:0u);
    if(nextIssue)DiagVHandleRecord(4u,slot,previous,next,next,nextHigh,7u+nextIssue);
}
extern "C" __declspec(naked) void DiagVLifeAllocBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI owner
        mov edx,dword ptr [esp+16] // EBX free slot
        mov ecx,dword ptr [esp+28] // EAX output handle
        mov ebx,dword ptr [esp+20] // EDX key
        mov esi,dword ptr [esp+68] // caller of 00BAE790
        push esi
        push ebx
        push ecx
        push edx
        push eax
        call DiagVLifeAllocate
        add esp,20
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+12]
    }
}
extern "C" __declspec(naked) void DiagVLifeFreeBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI owner
        mov edx,dword ptr [esp+16] // EBX slot
        mov ecx,dword ptr [esp+28] // EAX old free head
        mov ebx,dword ptr [esp+52] // caller of 00BAEA80
        push 0
        push ebx
        push ecx
        push edx
        push eax
        call DiagVLifeFree
        add esp,20
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+16]
    }
}
extern "C" __declspec(naked) void DiagVLifeFreeAltBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp]    // EDI owner
        mov edx,dword ptr [esp+28] // EAX slot
        mov ecx,dword ptr [esp+20] // EDX old free head
        mov ebx,dword ptr [esp+4]  // ESI handle being replaced
        push ebx
        push 0BAECA0h             // alternate free path
        push ecx
        push edx
        push eax
        call DiagVLifeFree
        add esp,20
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+20]
    }
}
extern "C" __declspec(naked) void DiagVLifeRejectBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI owner
        mov edx,dword ptr [esp+16] // EBX allocated slot
        mov ecx,dword ptr [esp+8]  // EBP pointer to handle pointer
        mov ecx,dword ptr [ecx]    // assigned handle
        mov ebx,dword ptr [esp]    // EDI neighboring pair
        mov esi,dword ptr [esp+28] // EAX raw neighboring high word
        mov edi,dword ptr [esp+60] // [original ESP+18]: hash lookup node
        push edi
        push esi
        push ebx
        push ecx
        push edx
        push eax
        call DiagVLifeReject
        add esp,24
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+24]
    }
}
extern "C" __declspec(naked) void DiagVHandleFirstBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI: owner
        mov edx,dword ptr [esp+28] // EAX: index
        mov ecx,dword ptr [esp+20] // EDX: pair written to the slot
        push ecx
        push edx
        push eax
        call DiagVHandleLinkFirst
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vHandleTramps]
    }
}
extern "C" __declspec(naked) void DiagVHandleSecondBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+16] // EBX: slot
        mov edx,dword ptr [esp]    // EDI: next pair
        mov ecx,dword ptr [esp+4]  // ESI: owner
        push edx
        push eax
        push ecx
        call DiagVHandleLinkSecond
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+4]
    }
}
extern "C" __declspec(naked) void DiagVHandleUnlinkNextBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI: owner
        mov edx,dword ptr [esp+8]  // EBP: predecessor slot
        mov ecx,dword ptr [esp+16] // EBX: current slot
        push ecx
        push edx
        push eax
        call DiagVHandleUnlinkNext
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+28]
    }
}
extern "C" __declspec(naked) void DiagVHandleUnlinkBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+24] // ECX: owner
        lea edx,[esp+36]          // original ESP: return + input handle
        push edx
        push eax
        call DiagVHandleUnlink
        add esp,8
        popad
        popfd
        jmp dword ptr [g_vHandleTramps+8]
    }
}
static bool DiagVHandleInstall() {
    const U8 first[]={0x89,0x94,0xC6,0x94,0xE0,0x17,0x00};
    const U8 second[]={0x89,0x7B,0x04,0x8B,0x4D,0x00};
    const U8 unlink[]={0x53,0x8B,0x5C,0x24,0x08};
    const U8 alloc[]={0x89,0x10,0x89,0x48,0x04};
    const U8 release[]={0x89,0x03,0x89,0x9E,0x94,0xE0,0x27,0x00};
    const U8 releaseAlt[]={0x89,0x10,0x89,0x87,0x94,0xE0,0x27,0x00};
    const U8 reject[]={0x25,0xFF,0xFF,0xFF,0x7F};
    const U8 unlinkNext[]={0x8B,0x7B,0x04,0x85,0xFF};
    if(DSF150_Status!=1)return false;
    const struct {UPtr site;const U8* bytes;U32 len;UPtr bridge;} sites[]={
        {0x00BAE8B3u,first,sizeof(first),reinterpret_cast<UPtr>(&DiagVHandleFirstBridge)},
        {0x00BAE8C4u,second,sizeof(second),reinterpret_cast<UPtr>(&DiagVHandleSecondBridge)},
        {0x00BAEA80u,unlink,sizeof(unlink),reinterpret_cast<UPtr>(&DiagVHandleUnlinkBridge)},
        {0x00BAE888u,alloc,sizeof(alloc),reinterpret_cast<UPtr>(&DiagVLifeAllocBridge)},
        {0x00BAEBCCu,release,sizeof(release),reinterpret_cast<UPtr>(&DiagVLifeFreeBridge)},
        {0x00BAED21u,releaseAlt,sizeof(releaseAlt),reinterpret_cast<UPtr>(&DiagVLifeFreeAltBridge)},
        {0x00BAE8A4u,reject,sizeof(reject),reinterpret_cast<UPtr>(&DiagVLifeRejectBridge)},
        {0x00BAEB8Bu,unlinkNext,sizeof(unlinkNext),reinterpret_cast<UPtr>(&DiagVHandleUnlinkNextBridge)}
    };
    for(U32 i=0u;i<8u;++i) {
        if(!Readable(Address(sites[i].site),sites[i].len))return false;
        for(U32 j=0u;j<sites[i].len;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i].site)+j)!=sites[i].bytes[j])
                return false;
    }
    for(U32 i=0u;i<8u;++i) {
        if(DiagFInstallDetour(g_vHandleDetours[i],sites[i].site,sites[i].bytes,
            sites[i].len,sites[i].bridge,g_vHandleTramps[i]))continue;
        while(i>0u) {--i;DiagFRestoreDetour(g_vHandleDetours[i],g_vHandleTramps[i]);}
        return false;
    }
    return true;
}
static void DiagVHandlePrint(U32 loops) {
    static U32 linksFirst=0u,linksSecond=0u,unlinks=0u;
    const U32 written=static_cast<U32>(g_vHandleNext);
    if(written>g_vHandleDrained+kVHandleEvents) {
        g_vHandleMissed+=written-g_vHandleDrained-kVHandleEvents;
        g_vHandleDrained=written-kVHandleEvents;
    }
    while(g_vHandleDrained<written) {
        const U32 sequence=g_vHandleDrained+1u;
        const DiagVHandleEvent& e=g_vHandleEvents[(sequence-1u)%kVHandleEvents];
        if(static_cast<U32>(e.seq)!=sequence)break;
        g_vHandleDrained=sequence;
        if(e.op==1u)++linksFirst;
        else if(e.op==2u)++linksSecond;
        else if(e.op==3u)++unlinks;
    }
    if(loops%5u!=0u)return;
    char line[320]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_HANDLE_STATUS tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(q," linkFirst=",linksFirst,end);
    DiagFAppendKeyU(q," linkSecond=",linksSecond,end);
    DiagFAppendKeyU(q," unlink=",unlinks,end);
    DiagFAppendKeyU(q," issues=",static_cast<U32>(g_vHandleIssue),end);
    DiagFAppendKeyU(q," missed=",g_vHandleMissed,end);
    DiagFAppendKeyU(q," alloc=",static_cast<U32>(g_vLifeAlloc)-g_vLifeAllocPrinted,end);
    DiagFAppendKeyU(q," free=",static_cast<U32>(g_vLifeFree)-g_vLifeFreePrinted,end);
    DiagFAppendKeyU(q," reject=",static_cast<U32>(g_vLifeReject)-g_vLifeRejectPrinted,end);
    DiagFAppendKeyU(q," nextWrites=",static_cast<U32>(g_vNextWrites)-g_vNextWritesPrinted,end);
    DiagFAppendKeyU(q," nextBad=",static_cast<U32>(g_vNextBad),end);
    const U32 owner=static_cast<U32>(g_vHandleOwner);
    const U32 freeHead=owner && Readable(owner+0x27E094u,4u)
        ?Read32(owner+0x27E094u):0u;
    DiagFAppendKeyHex(q," freeHead=",freeHead,end);
    DiagFAppendKeyU(q," slotBalance=",static_cast<U32>(g_vLifeAlloc)-static_cast<U32>(g_vLifeFree),end);
    DiagFAppendKeyU(q," lifeSuspect=",static_cast<U32>(g_vLifeSuspect),end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    linksFirst=linksSecond=unlinks=0u;
    g_vLifeAllocPrinted=static_cast<U32>(g_vLifeAlloc);
    g_vLifeFreePrinted=static_cast<U32>(g_vLifeFree);
    g_vLifeRejectPrinted=static_cast<U32>(g_vLifeReject);
    g_vNextWritesPrinted=static_cast<U32>(g_vNextWrites);
}

// Read-only provenance for indexed resource-handle hash-chain heads.
// Records three native writes to node+8 and samples the lookup result before
// 00BAEA80 follows a potentially malformed neighboring pair.
static const U32 kVHashLedgerSize=131072u;
struct DiagVHashHeadEntry {
    volatile long node;
    U32 tick,site,oldHead,newHead,oldHigh,newHigh;
    U32 oldIssue,newIssue,sourceSlot,writes;
};
static DiagVHashHeadEntry g_vHashLedger[kVHashLedgerSize]={};
static volatile long g_vHashTracked=0,g_vHashLost=0,g_vHashWrites=0;
static volatile long g_vHashBadWrite=0,g_vHashBadLookup=0,g_vHashHeadHooks=0;
static U32 g_vHashWritesPrinted=0u;
static DiagFDetour g_vHashDetours[4]={};
static UPtr g_vHashTramps[4]={};

static DiagVHashHeadEntry* DiagVHashFind(UPtr node,bool create) {
    if(!node)return nullptr;
    U32 index=((node>>4u)*2654435761u)&(kVHashLedgerSize-1u);
    for(U32 n=0u;n<128u;++n) {
        DiagVHashHeadEntry& e=g_vHashLedger[(index+n)&(kVHashLedgerSize-1u)];
        const U32 key=static_cast<U32>(e.node);
        if(key==node)return &e;
        if(!key && create && InterlockedCompareExchange(&e.node,
                static_cast<long>(node),0)==0) {
            InterlockedIncrement(&g_vHashTracked);
            return &e;
        }
    }
    if(create)InterlockedIncrement(&g_vHashLost);
    return nullptr;
}
static void DiagVHashPrint(const char* prefix,UPtr node,UPtr handle,UPtr slot,
                           U32 currentHead,U32 currentHigh,U32 currentIssue,
                           const DiagVHashHeadEntry* e) {
    char line[620]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,prefix,end);
    DiagFAppendKeyU(q," tick=",GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyHex(q," node=",node,end);
    DiagFAppendKeyHex(q," handle=",handle,end);
    DiagFAppendKeyHex(q," slot=",slot,end);
    DiagFAppendKeyHex(q," current=",currentHead,end);
    DiagFAppendKeyHex(q," currentHigh=",currentHigh,end);
    DiagFAppendKeyU(q," currentIssue=",currentIssue,end);
    if(e) {
        DiagFAppendKeyU(q," writeTick=",e->tick,end);
        DiagFAppendKeyHex(q," writeSite=",e->site,end);
        DiagFAppendKeyHex(q," old=",e->oldHead,end);
        DiagFAppendKeyHex(q," oldHigh=",e->oldHigh,end);
        DiagFAppendKeyU(q," oldIssue=",e->oldIssue,end);
        DiagFAppendKeyHex(q," new=",e->newHead,end);
        DiagFAppendKeyHex(q," newHigh=",e->newHigh,end);
        DiagFAppendKeyU(q," newIssue=",e->newIssue,end);
        DiagFAppendKeyHex(q," sourceSlot=",e->sourceSlot,end);
        DiagFAppendKeyU(q," writes=",e->writes,end);
    } else AppendText(q," ledger=missing",end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
}
extern "C" __declspec(noinline) void __cdecl DiagVHashHeadWrite(UPtr node,
                                            UPtr newHead,UPtr sourceSlot,U32 site) {
    if(!node || !Readable(node+8u,4u))return;
    const U32 oldHead=Read32(node+8u);
    U32 oldHigh=0u,newHigh=0u;
    const U32 oldIssue=DiagVHandleCheckPair(oldHead,oldHigh);
    const U32 newIssue=DiagVHandleCheckPair(newHead,newHigh);
    DiagVHashHeadEntry* e=DiagVHashFind(node,true);
    if(e) {
        e->tick=GetTickCount()-g_diagStartTick;e->site=site;
        e->oldHead=oldHead;e->newHead=newHead;
        e->oldHigh=oldHigh;e->newHigh=newHigh;
        e->oldIssue=oldIssue;e->newIssue=newIssue;
        e->sourceSlot=sourceSlot;++e->writes;
    }
    InterlockedIncrement(&g_vHashWrites);
    if(newIssue && InterlockedIncrement(&g_vHashBadWrite)<=12)
        DiagVHashPrint("DSFV29s_HASH_HEAD_BAD_WRITE",node,0u,sourceSlot,
                       newHead,newHigh,newIssue,e);
}
extern "C" __declspec(noinline) void __cdecl DiagVHashHeadLookup(UPtr owner,
                                               UPtr handle,UPtr node) {
    if(!owner || !handle || !node || !Readable(handle,8u) || !Readable(node+8u,4u))return;
    const U32 encoded=Read32(handle+4u);
    if(!(encoded&0x80000000u))return;
    const U32 index=encoded&0x7FFFFFFFu;
    if(index>=kVHandleSlots)return;
    const UPtr slot=owner+0x17E094u+index*8u;
    if(!Readable(slot,8u))return;
    U32 firstHigh=0u,nextHigh=0u,headHigh=0u;
    const U32 firstIssue=DiagVHandleCheckPair(Read32(slot),firstHigh);
    const U32 nextIssue=DiagVHandleCheckPair(Read32(slot+4u),nextHigh);
    const U32 head=Read32(node+8u);
    const U32 headIssue=DiagVHandleCheckPair(head,headHigh);
    if(!(firstIssue || nextIssue || headIssue))return;
    if(InterlockedIncrement(&g_vHashBadLookup)>12)return;
    DiagVHashPrint("DSFV29s_HASH_HEAD_BAD_LOOKUP",node,handle,slot,
                   head,headHigh,headIssue,DiagVHashFind(node,false));
}
extern "C" __declspec(naked) void DiagVHashInsertBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+20] // EDX node
        mov edx,dword ptr [esp+8]  // EBP points to new head pointer
        mov edx,dword ptr [edx]    // new head
        mov ecx,dword ptr [esp+16] // EBX source slot
        push 0BAE8BEh
        push ecx
        push edx
        push eax
        call DiagVHashHeadWrite
        add esp,16
        popad
        popfd
        jmp dword ptr [g_vHashTramps]
    }
}
extern "C" __declspec(naked) void DiagVHashUnlinkBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+56] // [original ESP+14]: lookup node
        mov edx,dword ptr [esp]    // EDI new head
        mov ecx,dword ptr [esp+16] // EBX source slot
        push 0BAEBBBh
        push ecx
        push edx
        push eax
        call DiagVHashHeadWrite
        add esp,16
        popad
        popfd
        jmp dword ptr [g_vHashTramps+4]
    }
}
extern "C" __declspec(naked) void DiagVHashAlternateBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+16] // EBX node
        mov edx,dword ptr [esp+24] // ECX new head
        mov ecx,dword ptr [esp+28] // EAX returned slot
        push 0BAED29h
        push ecx
        push edx
        push eax
        call DiagVHashHeadWrite
        add esp,16
        popad
        popfd
        jmp dword ptr [g_vHashTramps+8]
    }
}
extern "C" __declspec(naked) void DiagVHashLookupBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+4]  // ESI owner
        mov edx,dword ptr [esp+56] // original [ESP+14]: input handle
        mov ecx,dword ptr [esp+28] // EAX lookup node
        push ecx
        push edx
        push eax
        call DiagVHashHeadLookup
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vHashTramps+12]
    }
}
static bool DiagVHashHeadInstall() {
    if(DSF150_Status!=1 || g_vHandleHooks!=8)return false;
    // The insertion site ends exactly where the existing second-link hook
    // begins (00BAE8C4). The predecessor-unlink jump lands at 00BAEBBE.
    const U8 insert[]={0xC7,0x03,0x00,0x00,0x00,0x00};
    const U8 unlink[]={0x8B,0x4C,0x24,0x14,0x89,0x79,0x08};
    const U8 alternate[]={0x89,0x4B,0x08,0x85,0xC9};
    const U8 lookup[]={0x89,0x44,0x24,0x14,0x85,0xC0};
    const struct {UPtr site;const U8* bytes;U32 len;UPtr bridge;} sites[]={
        {0x00BAE8BEu,insert,sizeof(insert),reinterpret_cast<UPtr>(&DiagVHashInsertBridge)},
        {0x00BAEBB7u,unlink,sizeof(unlink),reinterpret_cast<UPtr>(&DiagVHashUnlinkBridge)},
        {0x00BAED29u,alternate,sizeof(alternate),reinterpret_cast<UPtr>(&DiagVHashAlternateBridge)},
        {0x00BAEB4Fu,lookup,sizeof(lookup),reinterpret_cast<UPtr>(&DiagVHashLookupBridge)}
    };
    for(U32 i=0u;i<4u;++i) {
        if(!Readable(Address(sites[i].site),sites[i].len))return false;
        for(U32 j=0u;j<sites[i].len;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i].site)+j)!=sites[i].bytes[j])return false;
    }
    for(U32 i=0u;i<4u;++i) {
        if(DiagFInstallDetour(g_vHashDetours[i],sites[i].site,sites[i].bytes,
            sites[i].len,sites[i].bridge,g_vHashTramps[i]))continue;
        while(i>0u) {--i;DiagFRestoreDetour(g_vHashDetours[i],g_vHashTramps[i]);}
        return false;
    }
    return true;
}
static void DiagVHashHeadPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;
    const U32 writes=static_cast<U32>(g_vHashWrites);
    char line[260]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_HASH_HEAD_STATUS tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(q," writes=",writes-g_vHashWritesPrinted,end);
    DiagFAppendKeyU(q," tracked=",static_cast<U32>(g_vHashTracked),end);
    DiagFAppendKeyU(q," lost=",static_cast<U32>(g_vHashLost),end);
    DiagFAppendKeyU(q," badWrite=",static_cast<U32>(g_vHashBadWrite),end);
    DiagFAppendKeyU(q," badLookup=",static_cast<U32>(g_vHashBadLookup),end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    g_vHashWritesPrinted=writes;
}

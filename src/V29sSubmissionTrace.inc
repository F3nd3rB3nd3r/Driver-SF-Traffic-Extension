// V29s read-only observation after the per-submodel 00697F10 dispatch.
// This is a late command boundary, not proof that the GPU drew pixels.
// Keep all game-thread work to bounded copies; format output on Monitor.
static const U32 kVSubmitEvents=8192u,kVSubmitParts=160u;
struct DiagVSubmitEvent {
    volatile long seq;
    U32 tick,model,native,sub,route,index,renderer,tableEntry,tableValid,method;
    U32 resourceValid,resourceCount,resourceItems,firstMask,firstGroupCount;
    U32 args[8]; // original 00697F10 arguments 2..9
};
static DiagVSubmitEvent g_vSubmitEvents[kVSubmitEvents]={};
static volatile long g_vSubmitNext=0,g_vSubmitHook=0,g_vSubmitObserved=0;
static U32 g_vSubmitDrained=0u,g_vSubmitMissed=0u;
static DiagFDetour g_vSubmitDetour={};
static UPtr g_vSubmitTramp=0u;
static DiagFDetour g_vSubmitGeneralDetour={};
static UPtr g_vSubmitGeneralTramp=0u;

extern "C" __declspec(noinline) void __cdecl DiagVRecordSubmission(UPtr frame,UPtr sub,U32 route) {
    InterlockedIncrement(&g_vSubmitObserved);
    const UPtr selected=static_cast<UPtr>(g_sSelectedDrawModel);
    if(!selected || !Readable(frame,route?40u:0x70u))return;
    // 008A8A50 arg7 is 008A8DA0 arg4 (model). At 008A8B99 the
    // 40 dispatch bytes and 40 local/saved bytes are still on the stack.
    const UPtr model=route?selected:Read32(frame+0x6Cu);
    if(!model || model!=selected)return;
    if(!Readable(model+0x4850u,4u))return;
    if(route) {
        const UPtr definition=Read32(model+0x4850u);
        if(!definition || !Readable(definition+0x74u,2u))return;
        const U32 count=*reinterpret_cast<volatile U16*>(definition+0x74u);
        if(!count)return;
        const UPtr first=definition+0x60u;
        if(sub<first || sub-first>=count*0x50u || (sub-first)%0x50u)return;
        if((sub-first)/0x50u>=kVSubmitParts)return;
    }
    const U32 renderer=Read32(frame),index=Read32(frame+4u);
    U32 tableEntry=0u,tableValid=0u,method=0u;
    if(index<=(0xFFFFFFFFu-0x0113A1F8u)/4u) {
        const UPtr table=Address(0x0113A1F8u)+index*4u;
        if(Readable(table,4u)) {tableEntry=Read32(table);tableValid=1u;}
    }
    if(renderer && Readable(renderer,4u)) {
        const UPtr vtable=Read32(renderer);
        if(vtable && Readable(vtable+0x18u,4u))method=Read32(vtable+0x18u);
    }
    const long sequence=InterlockedIncrement(&g_vSubmitNext);
    if(sequence<=0)return;
    DiagVSubmitEvent& e=g_vSubmitEvents[(static_cast<U32>(sequence)-1u)%kVSubmitEvents];
    e.tick=GetTickCount()-g_diagStartTick;
    e.model=model;e.native=static_cast<U32>(g_sSelectedNative);e.sub=sub;e.route=route;
    e.index=index;e.renderer=renderer;e.tableEntry=tableEntry;
    e.tableValid=tableValid;e.method=method;
    e.resourceValid=0u;e.resourceCount=e.resourceItems=0u;
    e.firstMask=e.firstGroupCount=0u;
    if(tableValid && tableEntry && method==Address(0x006C0690u) &&
       tableEntry<=0xFFFFFFFFu-0x50u && Readable(tableEntry+0x48u,8u)) {
        e.resourceValid=1u;
        e.resourceCount=Read32(tableEntry+0x48u);
        e.resourceItems=Read32(tableEntry+0x4Cu);
        if(e.resourceCount && e.resourceItems && Readable(e.resourceItems,8u)) {
            e.firstMask=Read32(e.resourceItems);
            e.firstGroupCount=Read32(e.resourceItems+4u);
        }
    }
    for(U32 i=0u;i<8u;++i)e.args[i]=Read32(frame+8u+i*4u);
    InterlockedExchange(&e.seq,sequence);
}
extern "C" __declspec(naked) void DiagVSubmissionBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+8] // original EBP: submodel
        lea edx,[esp+36]          // original ESP: 00697F10 arguments
        push 1
        push eax
        push edx
        call DiagVRecordSubmission
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vSubmitTramp]
    }
}
extern "C" __declspec(naked) void DiagVSubmissionGeneralBridge() {
    __asm {
        pushfd
        pushad
        lea edx,[esp+36] // 008A8B99, before 40-byte argument cleanup
        push 0
        push 0
        push edx
        call DiagVRecordSubmission
        add esp,12
        popad
        popfd
        jmp dword ptr [g_vSubmitGeneralTramp]
    }
}
static bool DiagVSubmissionInstall() {
    // 008A8D6F is after the relative CALL and before its 40-byte stack cleanup.
    // The copied instructions contain no PC-relative operand.
    const U8 sig[]={0x83,0xC4,0x28,0x8B,0x4C,0x24,0x24};
    const U8 generalSig[]={0x83,0xC4,0x28,0x5F,0x5E,0x5D,0x5B};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_vSubmitDetour,0x008A8D6Fu,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagVSubmissionBridge),g_vSubmitTramp))return false;
    if(!DiagFInstallDetour(g_vSubmitGeneralDetour,0x008A8B99u,generalSig,sizeof(generalSig),
        reinterpret_cast<UPtr>(&DiagVSubmissionGeneralBridge),g_vSubmitGeneralTramp)) {
        DiagFRestoreDetour(g_vSubmitDetour,g_vSubmitTramp);return false;
    }
    return true;
}
struct DiagVSubmitPart {
    bool seen,changed;
    U32 calls,sub,index,renderer,tableEntry,tableValid,method,native,args[8];
    U32 resourceValid,resourceCount,resourceItems,firstMask,firstGroupCount;
    U32 tableZero,tableInvalid,indexTransitions,tableTransitions;
    U32 resourceCountTransitions,resourceItemsTransitions;
};
static DiagVSubmitPart g_vSubmitParts[kVSubmitParts]={};
static DiagVSubmitPart g_vSubmitGeneral={};
static U32 g_vSubmitModel=0u,g_vSubmitWindow=0u,g_vSubmitUnknown=0u;
static void DiagVSubmissionPrint(U32 loops) {
    const U32 model=static_cast<U32>(g_sSelectedDrawModel);
    if(model!=g_vSubmitModel) {
        g_vSubmitModel=model;g_vSubmitWindow=g_vSubmitUnknown=0u;
        g_vSubmitGeneral=DiagVSubmitPart{};
        for(U32 i=0u;i<kVSubmitParts;++i)g_vSubmitParts[i]=DiagVSubmitPart{};
    }
    const U32 written=static_cast<U32>(g_vSubmitNext);
    if(written>g_vSubmitDrained+kVSubmitEvents) {
        g_vSubmitMissed+=written-g_vSubmitDrained-kVSubmitEvents;
        g_vSubmitDrained=written-kVSubmitEvents;
    }
    UPtr definition=0u;U32 partCount=0u;
    if(model && Readable(model+0x4850u,4u))definition=Read32(model+0x4850u);
    if(definition && Readable(definition+0x74u,2u))
        partCount=*reinterpret_cast<volatile U16*>(definition+0x74u);
    while(g_vSubmitDrained<written) {
        const U32 sequence=g_vSubmitDrained+1u;
        const DiagVSubmitEvent& e=g_vSubmitEvents[(sequence-1u)%kVSubmitEvents];
        if(static_cast<U32>(e.seq)!=sequence)break;
        g_vSubmitDrained=sequence;
        if(e.route==0u && e.model==model) {
            DiagVSubmitPart& p=g_vSubmitGeneral;
            if(!e.tableValid)++p.tableInvalid;
            else if(!e.tableEntry)++p.tableZero;
            if(p.seen && p.index!=e.index)++p.indexTransitions;
            if(p.seen && (p.tableEntry!=e.tableEntry ||
               p.tableValid!=e.tableValid))++p.tableTransitions;
            if(p.seen && (p.resourceValid!=e.resourceValid ||
               p.resourceCount!=e.resourceCount))++p.resourceCountTransitions;
            if(p.seen && p.resourceItems!=e.resourceItems)++p.resourceItemsTransitions;
            if(!p.seen || p.renderer!=e.renderer || p.index!=e.index ||
               p.tableEntry!=e.tableEntry || p.tableValid!=e.tableValid ||
               p.method!=e.method || p.resourceValid!=e.resourceValid ||
               p.resourceCount!=e.resourceCount || p.resourceItems!=e.resourceItems ||
               p.firstMask!=e.firstMask || p.firstGroupCount!=e.firstGroupCount)
                p.changed=true;
            if(p.seen)for(U32 j=0u;j<8u;++j)
                if(p.args[j]!=e.args[j]) {p.changed=true;break;}
            p.seen=true;++p.calls;
            p.index=e.index;p.renderer=e.renderer;p.tableEntry=e.tableEntry;
            p.tableValid=e.tableValid;p.method=e.method;p.native=e.native;
            p.resourceValid=e.resourceValid;p.resourceCount=e.resourceCount;
            p.resourceItems=e.resourceItems;p.firstMask=e.firstMask;
            p.firstGroupCount=e.firstGroupCount;
            for(U32 j=0u;j<8u;++j)p.args[j]=e.args[j];
            continue;
        }
        if(e.model!=model || !definition || e.sub<definition+0x60u ||
           e.sub-definition-0x60u>=partCount*0x50u ||
           (e.sub-definition-0x60u)%0x50u) {++g_vSubmitUnknown;continue;}
        const U32 part=(e.sub-definition-0x60u)/0x50u;
        if(part>=kVSubmitParts) {++g_vSubmitUnknown;continue;}
        DiagVSubmitPart& p=g_vSubmitParts[part];
        if(!p.seen || p.renderer!=e.renderer || p.index!=e.index ||
           p.tableEntry!=e.tableEntry || p.tableValid!=e.tableValid ||
           p.method!=e.method || p.resourceValid!=e.resourceValid ||
           p.resourceCount!=e.resourceCount || p.resourceItems!=e.resourceItems ||
           p.firstMask!=e.firstMask || p.firstGroupCount!=e.firstGroupCount)
            p.changed=true;
        if(p.seen)for(U32 j=0u;j<8u;++j)
            if(p.args[j]!=e.args[j]) {p.changed=true;break;}
        p.seen=true;++p.calls;++g_vSubmitWindow;
        p.sub=e.sub;p.index=e.index;p.renderer=e.renderer;
        p.tableEntry=e.tableEntry;p.tableValid=e.tableValid;p.method=e.method;
        p.resourceValid=e.resourceValid;p.resourceCount=e.resourceCount;
        p.resourceItems=e.resourceItems;p.firstMask=e.firstMask;
        p.firstGroupCount=e.firstGroupCount;
        p.native=e.native;
        for(U32 j=0u;j<8u;++j)p.args[j]=e.args[j];
    }
    if(loops%5u!=0u || !model)return;
    char line[310]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_SUBMIT_WINDOW tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyHex(q," model=",model,end);
    DiagFAppendKeyHex(q," targetNative=",static_cast<U32>(g_sSelectedNative),end);
    DiagFAppendKeyU(q," parts=",partCount,end);
    DiagFAppendKeyU(q," calls=",g_vSubmitWindow,end);
    DiagFAppendKeyU(q," observed=",static_cast<U32>(g_vSubmitObserved),end);
    DiagFAppendKeyU(q," missed=",g_vSubmitMissed,end);
    DiagFAppendKeyU(q," unknown=",g_vSubmitUnknown,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    g_vSubmitWindow=g_vSubmitUnknown=0u;
    if(g_vSubmitGeneral.calls) {
        const DiagVSubmitPart& p=g_vSubmitGeneral;
        char detail[580]={};char* c=detail;const char* lim=detail+sizeof(detail)-1;
        AppendText(c,"DSFV29s_SUBMIT_GENERAL tick=",lim);
        DiagFAppendU(c,GetTickCount()-g_diagStartTick,lim);
        DiagFAppendKeyHex(c," model=",model,lim);
        DiagFAppendKeyU(c," calls=",p.calls,lim);
        DiagFAppendKeyU(c," tableZero=",p.tableZero,lim);
        DiagFAppendKeyU(c," tableInvalid=",p.tableInvalid,lim);
        DiagFAppendKeyU(c," indexTransitions=",p.indexTransitions,lim);
        DiagFAppendKeyU(c," tableTransitions=",p.tableTransitions,lim);
        DiagFAppendKeyU(c," countTransitions=",p.resourceCountTransitions,lim);
        DiagFAppendKeyU(c," itemsTransitions=",p.resourceItemsTransitions,lim);
        DiagFAppendKeyU(c," changed=",p.changed?1u:0u,lim);
        DiagFAppendKeyU(c," index=",p.index,lim);
        DiagFAppendKeyHex(c," table=",p.tableEntry,lim);
        DiagFAppendKeyU(c," tableValid=",p.tableValid,lim);
        DiagFAppendKeyHex(c," renderer=",p.renderer,lim);
        DiagFAppendKeyHex(c," method=",p.method,lim);
        DiagFAppendKeyU(c," resourceValid=",p.resourceValid,lim);
        DiagFAppendKeyU(c," resourceCount=",p.resourceCount,lim);
        DiagFAppendKeyHex(c," resourceItems=",p.resourceItems,lim);
        DiagFAppendKeyHex(c," firstMask=",p.firstMask,lim);
        DiagFAppendKeyHex(c," firstGroupsRaw=",p.firstGroupCount,lim);
        DiagFAppendKeyHex(c," targetNative=",p.native,lim);
        for(U32 j=0u;j<8u;++j) {
            char key[]=" a0=";key[2]=static_cast<char>('2'+j);
            DiagFAppendKeyHex(c,key,p.args[j],lim);
        }
        AppendText(c,"\n",lim);*c=0;DiagPrint(detail);
        g_vSubmitGeneral.calls=0u;g_vSubmitGeneral.changed=false;
        g_vSubmitGeneral.tableZero=g_vSubmitGeneral.tableInvalid=0u;
        g_vSubmitGeneral.indexTransitions=g_vSubmitGeneral.tableTransitions=0u;
        g_vSubmitGeneral.resourceCountTransitions=0u;
        g_vSubmitGeneral.resourceItemsTransitions=0u;
    }
    for(U32 i=0u;i<kVSubmitParts;++i) {
        DiagVSubmitPart& p=g_vSubmitParts[i];
        if(!p.calls)continue;
        if(p.changed || loops%25u==0u) {
            char detail[580]={};char* c=detail;const char* lim=detail+sizeof(detail)-1;
            AppendText(c,"DSFV29s_SUBMIT_PART tick=",lim);
            DiagFAppendU(c,GetTickCount()-g_diagStartTick,lim);
            DiagFAppendKeyHex(c," model=",model,lim);
            DiagFAppendKeyU(c," part=",i,lim);
            DiagFAppendKeyU(c," calls=",p.calls,lim);
            DiagFAppendKeyU(c," changed=",p.changed?1u:0u,lim);
            DiagFAppendKeyHex(c," sub=",p.sub,lim);
            DiagFAppendKeyU(c," index=",p.index,lim);
            DiagFAppendKeyHex(c," table=",p.tableEntry,lim);
            DiagFAppendKeyU(c," tableValid=",p.tableValid,lim);
            DiagFAppendKeyHex(c," renderer=",p.renderer,lim);
            DiagFAppendKeyHex(c," method=",p.method,lim);
            DiagFAppendKeyU(c," resourceValid=",p.resourceValid,lim);
            DiagFAppendKeyU(c," resourceCount=",p.resourceCount,lim);
            DiagFAppendKeyHex(c," resourceItems=",p.resourceItems,lim);
            DiagFAppendKeyHex(c," firstMask=",p.firstMask,lim);
            DiagFAppendKeyHex(c," firstGroupsRaw=",p.firstGroupCount,lim);
            DiagFAppendKeyHex(c," targetNative=",p.native,lim);
            for(U32 j=0u;j<8u;++j) {
                char key[]=" a0=";key[2]=static_cast<char>('2'+j);
                DiagFAppendKeyHex(c,key,p.args[j],lim);
            }
            AppendText(c,"\n",lim);*c=0;DiagPrint(detail);
        }
        p.calls=0u;p.changed=false;
    }
}
// The graphics constructor sums all 21 configured A budgets (in KiB) and
// passes that byte total to 00BAF3F0. The constructor's 00BAFDB0 allocator
// then advances +2AEC and decrements +2AF0, so total = used + remaining.
static void DiagVGraphicsBudgetPrint(U32 loops) {
    if(loops%25u!=0u)return;
    unsigned long long totalA=0u,totalB=0u;
    U32 validRows=0u,vehicleA=0u,vehicleB=0u;
    for(U32 i=0u;i<21u;++i) {
        const UPtr row=Address(0x00FD2678u+i*0x14u);
        if(!Readable(row,0x14u))continue;
        const U32 a=Read32(row+4u),b=Read32(row+8u);
        totalA+=a;totalB+=b;++validRows;
        if(i==9u) {vehicleA=a;vehicleB=b;}
    }
    const UPtr global=Address(0x019B0CD0u);
    const bool globalValid=Readable(global+0x2AE8u,12u);
    const U32 base=globalValid?Read32(global+0x2AE8u):0u;
    const U32 cursor=globalValid?Read32(global+0x2AECu):0u;
    const U32 remaining=globalValid?Read32(global+0x2AF0u):0u;
    const U32 used=globalValid && cursor>=base?cursor-base:0u;
    const U32 total=globalValid && cursor>=base &&
        used<=0xFFFFFFFFu-remaining?used+remaining:0u;
    char line[360]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_GRAPHICS_BUDGET tick=",end);
    DiagFAppendU(p,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(p," rows=",validRows,end);
    DiagFAppendKeyU(p," sumAKiB=",static_cast<U32>(totalA),end);
    DiagFAppendKeyU(p," sumBKiB=",static_cast<U32>(totalB),end);
    DiagFAppendKeyU(p," vehicle9AKiB=",vehicleA,end);
    DiagFAppendKeyU(p," vehicle9BKiB=",vehicleB,end);
    DiagFAppendKeyU(p," globalValid=",globalValid?1u:0u,end);
    DiagFAppendKeyU(p," globalTotalBytes=",total,end);
    DiagFAppendKeyU(p," globalUsedBytes=",used,end);
    DiagFAppendKeyU(p," globalRemainingBytes=",remaining,end);
    DiagFAppendKeyHex(p," globalBase=",base,end);
    DiagFAppendKeyHex(p," globalCursor=",cursor,end);
    DiagFAppendKeyU(p," totalLessVehicle9A=",globalValid && total &&
        total<vehicleA*1024u?1u:0u,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
}

// V29s read-only per-submodel decision trace for the current Shift target.
// 0088D1A0 returns two mask words. 008A8BB0 skips its command path only when
// their AND is FFFFFFFF. This is a command-eligibility test, not GPU residency.
static const U32 kVVisualEvents=16384u,kVVisualParts=160u;
struct DiagVVisualEvent {
    volatile long seq;
    U32 model,submodel,maskA,maskB,site;
};
static DiagVVisualEvent g_vVisualEvents[kVVisualEvents]={};
static volatile long g_vVisualNext=0,g_vVisualObserved=0,g_vVisualSelected=0;
static U32 g_vVisualDrained=0u,g_vVisualMissed=0u,g_vVisualUnknown=0u;
static volatile long g_vVisualHooks=0;
static DiagFDetour g_vVisualDetours[2]={};
static UPtr g_vVisualTramps[2]={};
struct DiagVPartStat {
    U32 submodel,decisions,eligible,skipped,candidates,lastA,lastB;
    U32 printedA,printedB;
    bool printed;
};
static DiagVPartStat g_vVisualParts[kVVisualParts]={};
static U32 g_vVisualModel=0u,g_vVisualPartCount=0u;
static U32 g_vWindowDecisions=0u,g_vWindowEligible=0u;
static U32 g_vWindowSkipped=0u,g_vWindowCandidates=0u;

// Capture the render context of the other direct 008A8DA0 caller after its
// call returns. The hook is at five non-relative bytes, so the trampoline is
// safe with the existing byte-copy detour helper. Context is not assumed to
// be the native vehicle or the world object.
static const U32 kVContexts=4096u;
struct DiagVContextEvent {volatile long seq;U32 context,model,selected;};
static DiagVContextEvent g_vContextEvents[kVContexts]={};
static volatile long g_vContextNext=0,g_vContextHook=0;
static U32 g_vContextDrained=0u,g_vContextMissed=0u;
static DiagFDetour g_vContextDetour={};
static UPtr g_vContextTramp=0u;
extern "C" __declspec(noinline) void __cdecl DiagVRecordContext(UPtr context) {
    if(!context)return;
    const U32 model=Read32(context+0x38u);
    const long sequence=InterlockedIncrement(&g_vContextNext);
    if(sequence<=0)return;
    DiagVContextEvent& e=g_vContextEvents[
        (static_cast<U32>(sequence)-1u)%kVContexts];
    e.context=context;e.model=model;
    e.selected=model==static_cast<U32>(g_sSelectedDrawModel)?1u:0u;
    InterlockedExchange(&e.seq,sequence);
}
extern "C" __declspec(naked) void DiagVContextBridge() {
    __asm {
        pushfd
        pushad
        push ebx
        call DiagVRecordContext
        add esp,4
        popad
        popfd
        jmp dword ptr [g_vContextTramp]
    }
}
static bool DiagVContextInstall() {
    const U8 sig[]={0x8B,0x4B,0x38,0x8B,0x11};
    if(DSF150_Status!=1)return false;
    return DiagFInstallDetour(g_vContextDetour,0x00716C3Au,sig,sizeof(sig),
        reinterpret_cast<UPtr>(&DiagVContextBridge),g_vContextTramp);
}
static void DiagVContextPrint(U32 loops) {
    struct ContextWindow {U32 context,model,calls,selected;};
    static ContextWindow contexts[8]={};
    static U32 count=0u,omitted=0u,calls=0u,selected=0u;
    const U32 written=static_cast<U32>(g_vContextNext);
    if(written>g_vContextDrained+kVContexts) {
        g_vContextMissed+=written-g_vContextDrained-kVContexts;
        g_vContextDrained=written-kVContexts;
    }
    while(g_vContextDrained<written) {
        const U32 sequence=g_vContextDrained+1u;
        const DiagVContextEvent& e=g_vContextEvents[(sequence-1u)%kVContexts];
        if(static_cast<U32>(e.seq)!=sequence)break;
        g_vContextDrained=sequence;++calls;selected+=e.selected;
        U32 index=0u;
        for(;index<count;++index)if(contexts[index].context==e.context)break;
        if(index==count) {
            if(count>=8u) {++omitted;continue;}
            contexts[index].context=e.context;contexts[index].model=e.model;++count;
        }
        ++contexts[index].calls;contexts[index].selected+=e.selected;
    }
    if(loops%5u!=0u || !g_sSelectedDrawModel)return;
    const U32 tick=GetTickCount()-g_diagStartTick;
    char line[250]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_CONTEXT_WINDOW tick=",end);DiagFAppendU(p,tick,end);
    DiagFAppendKeyHex(p," selectedModel=",static_cast<U32>(g_sSelectedDrawModel),end);
    DiagFAppendKeyHex(p," native=",static_cast<U32>(g_sSelectedNative),end);
    DiagFAppendKeyU(p," calls=",calls,end);
    DiagFAppendKeyU(p," modelMatches=",selected,end);
    DiagFAppendKeyU(p," contexts=",count,end);
    DiagFAppendKeyU(p," omitted=",omitted,end);
    DiagFAppendKeyU(p," missed=",g_vContextMissed,end);
    AppendText(p,"\n",end);*p=0;DiagPrint(line);
    for(U32 i=0u;i<count;++i) {
        const U32 ctx=contexts[i].context;
        U32 nativeOffset=0xFFFFFFFFu;
        if(Readable(ctx,0x100u))for(U32 off=0u;off<0x100u;off+=4u)
            if(Read32(ctx+off)==static_cast<U32>(g_sSelectedNative)) {
                nativeOffset=off;break;
            }
        char item[350]={};char* q=item;const char* limit=item+sizeof(item)-1;
        AppendText(q,"DSFV29s_CONTEXT_OBJECT tick=",limit);DiagFAppendU(q,tick,limit);
        DiagFAppendKeyHex(q," ctx=",ctx,limit);
        DiagFAppendKeyU(q," calls=",contexts[i].calls,limit);
        DiagFAppendKeyU(q," modelMatches=",contexts[i].selected,limit);
        DiagFAppendKeyHex(q," model=",contexts[i].model,limit);
        DiagFAppendKeyHex(q," nativeOffset=",nativeOffset,limit);
        const U32 offsets[]={0u,0x20u,0x30u,0x38u,0x40u,0x44u,0x48u,0x4Cu,0x50u,0x54u};
        const char* labels[]={" p00="," p20="," p30="," p38="," p40="," p44="," p48="," p4C="," p50="," p54="};
        for(U32 j=0u;j<sizeof(offsets)/sizeof(offsets[0]);++j)
            if(Readable(ctx+offsets[j],4u))
                DiagFAppendKeyHex(q,labels[j],Read32(ctx+offsets[j]),limit);
        AppendText(q,"\n",limit);*q=0;DiagPrint(item);
    }
    count=omitted=calls=selected=0u;
    for(U32 i=0u;i<8u;++i)contexts[i]=ContextWindow{};
}

extern "C" __declspec(noinline) void __cdecl DiagVRecordVisual(
    UPtr frame,UPtr submodel,U32 maskA,U32 maskB,U32 site) {
    InterlockedIncrement(&g_vVisualObserved);
    const U32 model=Read32(frame+0x50u);
    if(!model || model!=static_cast<U32>(g_sSelectedDrawModel))return;
    InterlockedIncrement(&g_vVisualSelected);
    const long sequence=InterlockedIncrement(&g_vVisualNext);
    if(sequence<=0)return;
    DiagVVisualEvent& e=g_vVisualEvents[
        (static_cast<U32>(sequence)-1u)%kVVisualEvents];
    e.model=model;e.submodel=submodel;e.maskA=maskA;e.maskB=maskB;e.site=site;
    InterlockedExchange(&e.seq,sequence);
}
extern "C" __declspec(naked) void DiagVVisualDecisionBridge() {
    __asm {
        pushfd
        pushad
        lea ebx,[esp+36]
        push 0
        push dword ptr [esp+24] // original EDX after one push
        push dword ptr [esp+36] // original EAX after two pushes
        push dword ptr [esp+20] // original EBP after three pushes
        push ebx
        call DiagVRecordVisual
        add esp,20
        popad
        popfd
        jmp dword ptr [g_vVisualTramps]
    }
}
extern "C" __declspec(naked) void DiagVVisualCandidateBridge() {
    __asm {
        pushfd
        pushad
        lea ebx,[esp+36]
        push 1
        push dword ptr [esp+24]
        push dword ptr [esp+36]
        push dword ptr [esp+20]
        push ebx
        call DiagVRecordVisual
        add esp,20
        popad
        popfd
        jmp dword ptr [g_vVisualTramps+4]
    }
}
static bool DiagVVisualInstall() {
    const U8 decision[]={0x8B,0xF8,0x8B,0xDA,0x8B,0xCF};
    const U8 candidate[]={0x8B,0x54,0x24,0x14,0x8B,0x44,0x24,0x4C};
    if(DSF150_Status!=1)return false;
    if(!DiagFInstallDetour(g_vVisualDetours[0],0x008A8D05u,decision,
        sizeof(decision),reinterpret_cast<UPtr>(&DiagVVisualDecisionBridge),
        g_vVisualTramps[0]))return false;
    if(!DiagFInstallDetour(g_vVisualDetours[1],0x008A8D12u,candidate,
        sizeof(candidate),reinterpret_cast<UPtr>(&DiagVVisualCandidateBridge),
        g_vVisualTramps[1])) {
        DiagFRestoreDetour(g_vVisualDetours[0],g_vVisualTramps[0]);return false;
    }
    return true;
}
static void DiagVVisualPrint(U32 loops) {
    const U32 model=static_cast<U32>(g_sSelectedDrawModel);
    if(model!=g_vVisualModel) {
        g_vVisualModel=model;g_vVisualPartCount=0u;
        g_vWindowDecisions=g_vWindowEligible=g_vWindowSkipped=g_vWindowCandidates=0u;
        for(U32 i=0u;i<kVVisualParts;++i)g_vVisualParts[i]=DiagVPartStat{};
        if(model && Readable(model+0x4850u,4u)) {
            const UPtr definition=Read32(model+0x4850u);
            if(definition && Readable(definition+0x74u,2u))
                g_vVisualPartCount=*reinterpret_cast<volatile U16*>(definition+0x74u);
        }
    }
    const U32 written=static_cast<U32>(g_vVisualNext);
    if(written>g_vVisualDrained+kVVisualEvents) {
        g_vVisualMissed+=written-g_vVisualDrained-kVVisualEvents;
        g_vVisualDrained=written-kVVisualEvents;
    }
    UPtr definition=0u;
    if(model && Readable(model+0x4850u,4u))definition=Read32(model+0x4850u);
    while(g_vVisualDrained<written) {
        const U32 sequence=g_vVisualDrained+1u;
        const DiagVVisualEvent& e=g_vVisualEvents[(sequence-1u)%kVVisualEvents];
        if(static_cast<U32>(e.seq)!=sequence)break;
        g_vVisualDrained=sequence;
        if(e.model!=model || !definition || e.submodel<definition+0x60u ||
           e.submodel-definition-0x60u>=g_vVisualPartCount*0x50u ||
           (e.submodel-definition-0x60u)%0x50u!=0u) {
            ++g_vVisualUnknown;continue;
        }
        const U32 index=(e.submodel-definition-0x60u)/0x50u;
        if(index>=kVVisualParts) {++g_vVisualUnknown;continue;}
        DiagVPartStat& p=g_vVisualParts[index];p.submodel=e.submodel;
        if(e.site==0u) {
            ++g_vWindowDecisions;++p.decisions;
            if((e.maskA&e.maskB)==0xFFFFFFFFu) {++g_vWindowSkipped;++p.skipped;}
            else {++g_vWindowEligible;++p.eligible;}
            p.lastA=e.maskA;p.lastB=e.maskB;
        } else {++g_vWindowCandidates;++p.candidates;}
    }
    if(loops%5u!=0u || !model)return;
    char line[330]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_VIS_WINDOW tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyHex(q," model=",model,end);
    DiagFAppendKeyU(q," parts=",g_vVisualPartCount,end);
    DiagFAppendKeyU(q," decisions=",g_vWindowDecisions,end);
    DiagFAppendKeyU(q," eligible=",g_vWindowEligible,end);
    DiagFAppendKeyU(q," skipped=",g_vWindowSkipped,end);
    DiagFAppendKeyU(q," candidates=",g_vWindowCandidates,end);
    DiagFAppendKeyU(q," observed=",static_cast<U32>(g_vVisualObserved),end);
    DiagFAppendKeyU(q," selected=",static_cast<U32>(g_vVisualSelected),end);
    DiagFAppendKeyU(q," missed=",g_vVisualMissed,end);
    DiagFAppendKeyU(q," unknown=",g_vVisualUnknown,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    g_vWindowDecisions=g_vWindowEligible=g_vWindowSkipped=g_vWindowCandidates=0u;
    const U32 count=g_vVisualPartCount<kVVisualParts?g_vVisualPartCount:kVVisualParts;
    for(U32 i=0u;i<count;++i) {
        DiagVPartStat& p=g_vVisualParts[i];
        if(!p.decisions && !p.candidates)continue;
        const bool changed=!p.printed || p.printedA!=p.lastA || p.printedB!=p.lastB ||
            (p.skipped && p.eligible) || p.candidates!=p.eligible;
        if(changed) {
            char partLine[310]={};char* c=partLine;const char* partEnd=partLine+sizeof(partLine)-1;
            AppendText(c,"DSFV29s_VIS_PART tick=",partEnd);
            DiagFAppendU(c,GetTickCount()-g_diagStartTick,partEnd);
            DiagFAppendKeyHex(c," model=",model,partEnd);
            DiagFAppendKeyU(c," index=",i,partEnd);
            DiagFAppendKeyHex(c," sub=",p.submodel,partEnd);
            DiagFAppendKeyU(c," decisions=",p.decisions,partEnd);
            DiagFAppendKeyU(c," eligible=",p.eligible,partEnd);
            DiagFAppendKeyU(c," skipped=",p.skipped,partEnd);
            DiagFAppendKeyU(c," candidates=",p.candidates,partEnd);
            DiagFAppendKeyHex(c," maskA=",p.lastA,partEnd);
            DiagFAppendKeyHex(c," maskB=",p.lastB,partEnd);
            if(Readable(p.submodel+0x3Eu,2u))
                DiagFAppendKeyU(c," field3E=",
                    *reinterpret_cast<volatile U16*>(p.submodel+0x3Eu),partEnd);
            if(Readable(p.submodel+0x42u,2u))
                DiagFAppendKeyU(c," field42=",
                    *reinterpret_cast<volatile U16*>(p.submodel+0x42u),partEnd);
            AppendText(c,"\n",partEnd);*c=0;DiagPrint(partLine);
            p.printed=true;p.printedA=p.lastA;p.printedB=p.lastB;
        }
        p.decisions=p.eligible=p.skipped=p.candidates=0u;
    }
}

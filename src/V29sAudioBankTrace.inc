// Passive vehicle-bank pipeline counts. The callback's third argument is
// recorded raw; its result code is not assumed to equal the submit result.
static volatile long g_audioHooks=0;
static volatile long g_audioSubmit=0,g_audioSubmitAccepted=0,g_audioSubmitRejected=0;
static volatile long g_audioSubmitZeroId=0,g_audioCallback=0,g_audioCallbackZeroId=0;
static volatile long g_audioCallbackResultOne=0,g_audioCallbackResultOther=0;
static volatile long g_audioOpen=0,g_audioOpenFailed=0;
static volatile long g_audioLookup=0,g_audioLookupMatched=0,g_audioLookupReady=0;
static volatile U32 g_audioLastSubmitJob=0,g_audioLastSubmitBuffer=0;
static volatile U32 g_audioLastSubmitSize=0,g_audioLastSubmitId=0,g_audioLastSubmitResult=0;
static volatile U32 g_audioLastCallbackJob=0,g_audioLastCallbackId=0,g_audioLastCallbackResult=0;
static volatile U32 g_audioLastOpenParser=0,g_audioLastLookupId=0,g_audioLastLookupState=0;
static DiagFDetour g_audioDetours[5]={};
static UPtr g_audioTramps[5]={};

extern "C" __declspec(noinline) void __cdecl DiagAudioSubmit(UPtr job,U32 result) {
    InterlockedIncrement(&g_audioSubmit);
    if(result==1u)InterlockedIncrement(&g_audioSubmitAccepted);
    else InterlockedIncrement(&g_audioSubmitRejected);
    g_audioLastSubmitJob=job;g_audioLastSubmitResult=result;
    if(!job || !Readable(job+4u,0x14u))return;
    g_audioLastSubmitBuffer=Read32(job+4u);
    g_audioLastSubmitSize=Read32(job+8u);
    g_audioLastSubmitId=Read32(job+0xCu);
    if(!g_audioLastSubmitId)InterlockedIncrement(&g_audioSubmitZeroId);
}
extern "C" __declspec(noinline) void __cdecl DiagAudioCallback(UPtr job,U32 rawResult) {
    InterlockedIncrement(&g_audioCallback);
    if(rawResult==1u)InterlockedIncrement(&g_audioCallbackResultOne);
    else InterlockedIncrement(&g_audioCallbackResultOther);
    g_audioLastCallbackJob=job;g_audioLastCallbackResult=rawResult;
    if(!job || !Readable(job+0xCu,4u))return;
    g_audioLastCallbackId=Read32(job+0xCu);
    if(!g_audioLastCallbackId)InterlockedIncrement(&g_audioCallbackZeroId);
}
extern "C" __declspec(noinline) void __cdecl DiagAudioOpen(UPtr parser,U32 result) {
    InterlockedIncrement(&g_audioOpen);
    if(result!=0xFFFFFFFFu)return;
    InterlockedIncrement(&g_audioOpenFailed);
    g_audioLastOpenParser=parser;
}
extern "C" __declspec(noinline) void __cdecl DiagAudioLookupEntry() {
    InterlockedIncrement(&g_audioLookup);
}
extern "C" __declspec(noinline) void __cdecl DiagAudioLookupMatched(UPtr entry) {
    InterlockedIncrement(&g_audioLookupMatched);
    if(!entry || !Readable(entry+0x18u,0xCu))return;
    const U32 id=Read32(entry+0x18u),state=Read32(entry+0x20u);
    g_audioLastLookupId=id;g_audioLastLookupState=state;
    if(state==4u && id)InterlockedIncrement(&g_audioLookupReady);
}
extern "C" __declspec(naked) void DiagAudioSubmitBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+28] // Wwise immediate result
        mov ecx,dword ptr [esp+4]  // ESI bank job
        push eax
        push ecx
        call DiagAudioSubmit
        add esp,8
        popad
        popfd
        jmp dword ptr [g_audioTramps]
    }
}
extern "C" __declspec(naked) void DiagAudioCallbackBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+28] // EAX bank job
        mov edx,dword ptr [esp+48] // original [ESP+0Ch]: raw result
        push edx
        push eax
        call DiagAudioCallback
        add esp,8
        popad
        popfd
        jmp dword ptr [g_audioTramps+4]
    }
}
extern "C" __declspec(naked) void DiagAudioOpenBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+28] // EAX file-handle slot or -1
        mov ecx,dword ptr [esp+4]  // ESI CHNK reader
        push eax
        push ecx
        call DiagAudioOpen
        add esp,8
        popad
        popfd
        jmp dword ptr [g_audioTramps+8]
    }
}
extern "C" __declspec(naked) void DiagAudioLookupEntryBridge() {
    __asm {
        pushfd
        pushad
        call DiagAudioLookupEntry
        popad
        popfd
        jmp dword ptr [g_audioTramps+12]
    }
}
extern "C" __declspec(naked) void DiagAudioLookupMatchedBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+24] // ECX matching audio-bank entry
        push eax
        call DiagAudioLookupMatched
        add esp,4
        popad
        popfd
        jmp dword ptr [g_audioTramps+16]
    }
}
static bool DiagAudioInstall() {
    if(DSF150_Status!=1)return false;
    // Each span ends at an instruction boundary and before any branch target.
    const U8 submit[]={0x83,0xC4,0x14,0x83,0xF8,0x01};
    const U8 callback[]={0xC7,0x40,0x14,0x04,0x00,0x00,0x00};
    const U8 open[]={0x89,0x86,0x00,0x56,0x00,0x00};
    const U8 lookup[]={0x8B,0x81,0x3C,0x02,0x00,0x00};
    const U8 matched[]={0x8B,0xC1,0x83,0xC0,0x0C};
    const struct {UPtr site;const U8* bytes;U32 len;UPtr bridge;} sites[]={
        {0x0084BED2u,submit,sizeof(submit),reinterpret_cast<UPtr>(&DiagAudioSubmitBridge)},
        {0x0084BE74u,callback,sizeof(callback),reinterpret_cast<UPtr>(&DiagAudioCallbackBridge)},
        {0x008FC54Eu,open,sizeof(open),reinterpret_cast<UPtr>(&DiagAudioOpenBridge)},
        {0x0048F780u,lookup,sizeof(lookup),reinterpret_cast<UPtr>(&DiagAudioLookupEntryBridge)},
        {0x0048F7B0u,matched,sizeof(matched),reinterpret_cast<UPtr>(&DiagAudioLookupMatchedBridge)}
    };
    for(U32 i=0u;i<5u;++i) {
        if(!Readable(Address(sites[i].site),sites[i].len))return false;
        for(U32 j=0u;j<sites[i].len;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i].site)+j)!=sites[i].bytes[j])return false;
    }
    for(U32 i=0u;i<5u;++i) {
        if(DiagFInstallDetour(g_audioDetours[i],sites[i].site,sites[i].bytes,
            sites[i].len,sites[i].bridge,g_audioTramps[i]))continue;
        while(i>0u) {--i;DiagFRestoreDetour(g_audioDetours[i],g_audioTramps[i]);}
        return false;
    }
    return true;
}
static void DiagAudioPrintStatus(U32 loops) {
    if(loops%25u!=0u)return;
    char line[620]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_AUDIO_STATUS tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(q," submit=",static_cast<U32>(g_audioSubmit),end);
    DiagFAppendKeyU(q," accepted=",static_cast<U32>(g_audioSubmitAccepted),end);
    DiagFAppendKeyU(q," rejected=",static_cast<U32>(g_audioSubmitRejected),end);
    DiagFAppendKeyU(q," submitZeroId=",static_cast<U32>(g_audioSubmitZeroId),end);
    DiagFAppendKeyU(q," callback=",static_cast<U32>(g_audioCallback),end);
    DiagFAppendKeyU(q," cbResult1=",static_cast<U32>(g_audioCallbackResultOne),end);
    DiagFAppendKeyU(q," cbResultOther=",static_cast<U32>(g_audioCallbackResultOther),end);
    DiagFAppendKeyU(q," cbZeroId=",static_cast<U32>(g_audioCallbackZeroId),end);
    DiagFAppendKeyU(q," open=",static_cast<U32>(g_audioOpen),end);
    DiagFAppendKeyU(q," openFailed=",static_cast<U32>(g_audioOpenFailed),end);
    DiagFAppendKeyU(q," lookup=",static_cast<U32>(g_audioLookup),end);
    DiagFAppendKeyU(q," matched=",static_cast<U32>(g_audioLookupMatched),end);
    DiagFAppendKeyU(q," ready=",static_cast<U32>(g_audioLookupReady),end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    if(!g_audioSubmitRejected && !g_audioCallbackZeroId && !g_audioOpenFailed &&
       g_audioLookupMatched==g_audioLookupReady)return;
    char detail[450]={};q=detail;end=detail+sizeof(detail)-1;
    AppendText(q,"DSFV29s_AUDIO_DETAIL tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyHex(q," job=",g_audioLastSubmitJob,end);
    DiagFAppendKeyHex(q," buffer=",g_audioLastSubmitBuffer,end);
    DiagFAppendKeyHex(q," size=",g_audioLastSubmitSize,end);
    DiagFAppendKeyHex(q," id=",g_audioLastSubmitId,end);
    DiagFAppendKeyHex(q," result=",g_audioLastSubmitResult,end);
    DiagFAppendKeyHex(q," cbJob=",g_audioLastCallbackJob,end);
    DiagFAppendKeyHex(q," cbId=",g_audioLastCallbackId,end);
    DiagFAppendKeyHex(q," cbRaw=",g_audioLastCallbackResult,end);
    DiagFAppendKeyHex(q," openParser=",g_audioLastOpenParser,end);
    DiagFAppendKeyHex(q," lookupId=",g_audioLastLookupId,end);
    DiagFAppendKeyHex(q," lookupState=",g_audioLastLookupState,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(detail);
}

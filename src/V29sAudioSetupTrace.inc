// R9 diagnostics only: preserve native outcomes; no capacities or policies change.
static long g_audioSetupStatus=0;
static U32 g_audioSetupHooks=0;
static volatile long g_audioSetupCounts[10]={},g_audioSetupDropped=0,g_audioSetupLock=0;
static const char* kAudioSetupNames[10]={"vehicleDisabled","bankNotReady","modelProfileMissing","defaultProfileMissing",
    "previewSuppressed","invalidMode","modeInitializer","vehicleState","snapshotGetter","snapshotFlag"};
struct AudioSetupDetail {
    U32 tick,reason,caller,controller,vehicle,model,selected,mode,profile,defaults,state,preview,previewKnown,flags,observed;
};
static AudioSetupDetail g_audioSetupRing[1024]={};
static U32 g_audioSetupWrite=0,g_audioSetupRead=0;
static void AudioSetupRecord(U32 reason,U32* frame,bool inner,U32 observed){
    if(g_audioSetupStatus!=1)return;
    const UPtr f=reinterpret_cast<UPtr>(frame),sp=f+36u;
    // Other callers of the mode helpers are reconfiguration, not setup attempts.
    if(inner){const U32 c=Read32(sp+0xCCu);if(c!=Address(0x5539AAu) && c!=Address(0x5539B5u) && c!=Address(0x5539C0u))return;}
    InterlockedIncrement(&g_audioSetupCounts[reason]);
    AudioSetupDetail r={};r.tick=GetTickCount()-g_capacityStart;r.reason=reason;
    r.controller=frame[1];r.caller=Read32(sp+(inner?0xD8u:4u));
    r.vehicle=inner?Read32(r.controller+0xB4u):Read32(sp+8u);
    // Setup reuses its mode argument slot as output scratch at00553966.
    // Only the eligibility gate precedes the stable controller+88 store.
    r.model=Read32(r.vehicle+0x5ACu);r.mode=reason==0u?Read32(sp+12u):Read32(r.controller+0x88u);
    r.selected=reason==0u?r.model:Read32(r.controller+0x10u);
    r.profile=reason==0u?0u:Read32(r.controller+0xBCu);r.defaults=reason==0u?0u:Read32(r.controller+0xC0u);
    r.state=Read32(r.vehicle+0x544u);r.flags=reason==1u?AudioModelTestFlags(frame[7],frame[8]):frame[8];r.observed=observed;
    const UPtr root=Read32(Address(0xFE7228u)),manager=root?Read32(root+0x25Cu):0u;
    r.previewKnown=manager && Readable(manager+4u,1u);r.preview=r.previewKnown?Read8(manager+4u):0u;
    // Bounded, nonblocking producer; cumulative reason counters never depend
    // on detail availability. Consumer releases the lock before file I/O.
    if(InterlockedCompareExchange(&g_audioSetupLock,1,0)){InterlockedIncrement(&g_audioSetupDropped);return;}
    if(g_audioSetupWrite-g_audioSetupRead>=1024u)InterlockedIncrement(&g_audioSetupDropped);
    else {g_audioSetupRing[g_audioSetupWrite&1023u]=r;++g_audioSetupWrite;}
    InterlockedExchange(&g_audioSetupLock,0);
}
static void AudioSetupBankFailure(U32* frame){AudioSetupRecord(1u,frame,false,0u);}
static U32 AudioSetupByteCompare(U32 v,U32 previous){
    U32 flags=0;__asm {mov eax,v
        cmp al,0
        pushfd
        pop flags}
    return (previous&~0x8D5u)|(flags&0x8D5u);
}
extern "C" __declspec(noinline) void __cdecl AudioSetupEntry(U32 site,U32* frame){
    const UPtr sp=reinterpret_cast<UPtr>(frame)+36u;const U32 flags=frame[8];
    if(site<6u){
        const bool zero=(flags&0x40u)!=0u;
        const bool failed=site==3u?!zero:site==4u?!(flags&0x41u):zero;
        static const U32 next[6]={0x553874u,0x55392Du,0x55393Au,0x553966u,0x55399Au,0x5539D3u};
        static const U32 reasons[6]={0u,2u,3u,4u,5u,6u};
        frame[3]=Address(failed?0x553AD5u:next[site]);
        if(failed)AudioSetupRecord(reasons[site],frame,false,flags);
        return; // Original CMP/TEST flags are already saved; leave them intact.
    }
    const bool first=site<8u;const U32 bad=first?0x5535E1u:0x551F11u;
    if(site==6u || site==8u){
        const U32 state=Read32(frame[6]+0x544u); // Original ECX, read exactly once.
        frame[8]=AudioModelCompareFlags(state,3u,flags);
        frame[3]=Address(state==3u?(first?0x5535EDu:0x551F1Du):bad);
        if(state!=3u)AudioSetupRecord(7u,frame,true,state);
    }else{
        frame[8]=AudioModelTestFlags(frame[7],flags);
        if(!(frame[7]&255u)){frame[3]=Address(bad);AudioSetupRecord(8u,frame,true,0u);return;}
        const U32 value=Read8(sp+0x9Bu); // Native snapshot +97 at the point tested.
        frame[8]=AudioSetupByteCompare(value,frame[8]);
        frame[3]=Address(value?bad:(first?0x553605u:0x551F35u));
        if(value)AudioSetupRecord(9u,frame,true,value);
    }
}
#define AUDIO_SETUP_BRIDGE(name,index) \
extern "C" __declspec(naked) void name(){ \
    __asm {pushfd} __asm {pushad} __asm {mov ebp,esp} \
    __asm {sub esp,528} __asm {and esp,0FFFFFFF0h} __asm {fxsave [esp]} __asm {cld} \
    __asm {push ebp} __asm {push index} __asm {call AudioSetupEntry} __asm {add esp,8} \
    __asm {fxrstor [esp]} __asm {mov esp,ebp} __asm {popad} __asm {popfd} \
    __asm {jmp dword ptr [esp-24]} \
}
AUDIO_SETUP_BRIDGE(AudioSetupDisabledBridge,0)
AUDIO_SETUP_BRIDGE(AudioSetupProfileBridge,1)
AUDIO_SETUP_BRIDGE(AudioSetupDefaultBridge,2)
AUDIO_SETUP_BRIDGE(AudioSetupPreviewBridge,3)
AUDIO_SETUP_BRIDGE(AudioSetupModeBridge,4)
AUDIO_SETUP_BRIDGE(AudioSetupInitializerBridge,5)
AUDIO_SETUP_BRIDGE(AudioSetupState02Bridge,6)
AUDIO_SETUP_BRIDGE(AudioSetupSnapshot02Bridge,7)
AUDIO_SETUP_BRIDGE(AudioSetupState1Bridge,8)
AUDIO_SETUP_BRIDGE(AudioSetupSnapshot1Bridge,9)
#undef AUDIO_SETUP_BRIDGE
struct AudioSetupSite {U32 address,length;U8 bytes[16];UPtr bridge;};
static const AudioSetupSite kAudioSetupSites[10]={
    {0x55386Eu,6,{0x0F,0x84,0x61,2,0,0},reinterpret_cast<UPtr>(&AudioSetupDisabledBridge)},
    {0x553927u,6,{0x0F,0x84,0xA8,1,0,0},reinterpret_cast<UPtr>(&AudioSetupProfileBridge)},
    {0x553934u,6,{0x0F,0x84,0x9B,1,0,0},reinterpret_cast<UPtr>(&AudioSetupDefaultBridge)},
    {0x553960u,6,{0x0F,0x85,0x6F,1,0,0},reinterpret_cast<UPtr>(&AudioSetupPreviewBridge)},
    {0x553994u,6,{0x0F,0x87,0x3B,1,0,0},reinterpret_cast<UPtr>(&AudioSetupModeBridge)},
    {0x5539CDu,6,{0x0F,0x84,2,1,0,0},reinterpret_cast<UPtr>(&AudioSetupInitializerBridge)},
    {0x5535D8u,9,{0x83,0xB9,0x44,5,0,0,3,0x74,0x0C},reinterpret_cast<UPtr>(&AudioSetupState02Bridge)},
    {0x5535F7u,14,{0x84,0xC0,0x74,0xE6,0x80,0xBC,0x24,0x9B,0,0,0,0,0x75,0xDC},reinterpret_cast<UPtr>(&AudioSetupSnapshot02Bridge)},
    {0x551F08u,9,{0x83,0xB9,0x44,5,0,0,3,0x74,0x0C},reinterpret_cast<UPtr>(&AudioSetupState1Bridge)},
    {0x551F27u,14,{0x84,0xC0,0x74,0xE6,0x80,0xBC,0x24,0x9B,0,0,0,0,0x75,0xDC},reinterpret_cast<UPtr>(&AudioSetupSnapshot1Bridge)}
};
static DiagFDetour g_audioSetupDetours[10]={};static UPtr g_audioSetupUnusedTramps[10]={};
static bool AudioSetupSignature(bool patched){
    for(const auto& s:kAudioSetupSites){const UPtr at=Address(s.address);if(!Readable(at,s.length))return false;
        for(U32 j=0;j<s.length;++j){const U8 expected=patched?(j==0u?0xE9u:j>=5u?0x90u:static_cast<U8>((s.bridge-(at+5u))>>((j-1u)*8u))):s.bytes[j];
            if(Read8(at+j)!=expected)return false;}}
    return true;
}
static bool AudioSetupInstall(){
    g_audioSetupStatus=-1;
    if(g_audioModelStatus!=1 || !AudioModelSignature(true) || !AudioSlotsCold() || !VerifyExecutableIdentity() || !AudioSetupSignature(false))return false;
    bool good=true;
    // Generic installer supplies rollback storage. Relative branches in its
    // copied bytes are NEVER executed: all continuations are explicit above.
    for(U32 i=0;i<10u;++i){const auto& s=kAudioSetupSites[i];
        if(!DiagFInstallDetour(g_audioSetupDetours[i],s.address,s.bytes,s.length,s.bridge,g_audioSetupUnusedTramps[i])){good=false;break;}}
    if(good && AudioSetupSignature(true)){g_audioSetupHooks=10u;g_audioSetupStatus=1;return true;}
    for(U32 i=10u;i;--i)DiagFRestoreDetour(g_audioSetupDetours[i-1u],g_audioSetupUnusedTramps[i-1u]);
    if(!AudioSetupSignature(false))g_audioSetupStatus=-2;
    return false;
}
static void AudioSetupStartLine(){
    char line[400]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_AUDIO_SETUP_START revision=1 status=",end);AppendNumber(p,g_audioSetupStatus,end);
    DiagFAppendKeyU(p," hooks=",g_audioSetupHooks,end);
    AppendText(p," expectedHooks=10 outerReasons=7 subReasons=3 ringCapacity=1024 bankHook=reused nativeOutcomes=1 capacitiesChanged=0\n",end);*p=0;CapacityWrite(line);
}
static void AudioSetupPoll(){
    if(g_audioSetupStatus!=1)return;
    for(U32 n=0;n<1024u;++n){AudioSetupDetail r={};
        if(InterlockedCompareExchange(&g_audioSetupLock,1,0))break;
        const bool available=g_audioSetupRead!=g_audioSetupWrite;
        if(available){r=g_audioSetupRing[g_audioSetupRead&1023u];++g_audioSetupRead;}
        InterlockedExchange(&g_audioSetupLock,0);if(!available)break;
        char line[1100]={};char* p=line;const char* end=line+sizeof(line)-1u;
        AppendText(p,"DSFV29s_AUDIO_SETUP_DETAIL reason=",end);AppendText(p,kAudioSetupNames[r.reason],end);
        DiagFAppendKeyU(p," tick=",r.tick,end);DiagFAppendKeyU(p," subreason=",r.reason>=7u,end);
        DiagFAppendKeyHex(p," caller=",r.caller,end);DiagFAppendKeyHex(p," controller=",r.controller,end);
        DiagFAppendKeyHex(p," vehicle=",r.vehicle,end);DiagFAppendKeyU(p," model=",r.model,end);
        DiagFAppendKeyU(p," selected=",r.selected,end);DiagFAppendKeyU(p," mode=",r.mode,end);
        DiagFAppendKeyHex(p," profileSample=",r.profile,end);DiagFAppendKeyHex(p," defaultSample=",r.defaults,end);
        DiagFAppendKeyU(p," stateSample=",r.state,end);DiagFAppendKeyU(p," previewKnown=",r.previewKnown,end);
        DiagFAppendKeyU(p," previewSample=",r.preview,end);DiagFAppendKeyHex(p," testedFlags=",r.flags,end);
        DiagFAppendKeyU(p," observed=",r.observed,end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(last && now-last<1000u)return;last=now;
    char line[900]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_AUDIO_SETUP_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    U32 outer=0;for(U32 i=0;i<10u;++i){AppendText(p," ",end);AppendText(p,kAudioSetupNames[i],end);AppendText(p,"=",end);
        const U32 count=static_cast<U32>(InterlockedCompareExchange(&g_audioSetupCounts[i],0,0));DiagFAppendU(p,count,end);if(i<7u)outer+=count;}
    DiagFAppendKeyU(p," outerTotal=",outer,end);DiagFAppendKeyU(p," setupFailSample=",static_cast<U32>(g_audioModelCalls[7]),end);
    DiagFAppendKeyU(p," detailDropped=",static_cast<U32>(g_audioSetupDropped),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

#if !defined(DSF_V29S_REGMAP_REUSE) || !defined(DSF_V29S_REPLAY_PAUSE)
#error Replay timing requires the current R2 map and pause baseline
#endif
extern "C" __declspec(dllimport) BOOL __stdcall QueryPerformanceCounter(__int64*);
extern "C" __declspec(dllimport) BOOL __stdcall QueryPerformanceFrequency(__int64*);
static __int64 g_rhFrequency=0;
static U32 RHThread(){return GetCurrentThreadId();}
static U32 RHTick(){return GetTickCount()-g_capacityStart;}
static __int64 RHNow(){__int64 n=0;if(!QueryPerformanceCounter(&n))InterlockedIncrement(&g_rhClockFailures);return n;}
static RHView RHReadView(U32 kind,UPtr self) {
    RHView v={};const UPtr root=Read32(Address(0x00FE7228u));if(!root || !Readable(root,0x230u))return v;
    const UPtr owner=Read32(root+0x22Cu);
    const bool direct=kind<=RHSeek || kind==RHUpdate;
    if(!owner || (direct && owner!=self) || !Readable(owner,0x4EA18u) || Read32(owner+0x4E588u)!=2u)return v;
    v.valid=1u;v.owner=owner;v.recording=Read32(owner+0x4E58Cu);v.mode=2u;
    v.phase=Read32(owner+0x438ECu);v.transition=Read32(owner+0x4E9F4u);v.countdown=Read32(owner+0x4E9F0u);
    v.loading=Read8(owner+0x4E9ECu);v.lifecycle=Read32(owner+0x4EA14u);v.clock=Read32(owner+0x4E9A0u);v.requested=Read32(owner+0x4E99Cu);
    const UPtr sim=Read32(root+0xCCu),spool=Read32(root+0x214u);
    v.paused=v.applied=v.traffic=v.retained=v.spool=0xFFFFFFFFu;
    if(Readable(sim,0x194u)){v.paused=Read8(sim+0x188u);v.applied=Read32(sim+0x190u);}
    if(Readable(spool,0xDB0u)){v.traffic=Read32(spool+0xC8u);v.retained=Read32(spool+0x9A8u);v.spool=Read32(spool+0xDACu);}
    if(kind==RHSeek || kind==RHStream){const UPtr selected=Read32(owner+0x4E9F8u);if(Readable(selected,8u))v.selected=Read32(selected);}
    if(kind==RHStep || kind==RHUpdate) {
        if(Readable(v.recording+0x8554u,4u)) {
            const UPtr event=Read32(v.recording+0x8554u);
            if(Readable(event,0x14u)){v.eventType=Read32(event)&65535u;v.eventKind=Read32(event+0x10u);}
        }
    }
    return v;
}
#include "V29sReplayHitchNative.inc"
#include "V29sReplayHitchBridges.inc"
#include "V29sReplayHitchLayout.inc"
static void RHAppendView(char*& p,const char* end,const RHView& v,bool before) {
    const char* a[]={" valid="," owner="," recording="," mode="," phase="," transition="," countdown="," loading="," lifecycle="," clockBits="," requestedBits="," appliedBits="," paused="," pendingTraffic="," pendingRetained="," spoolState="," selectedBits="," eventType="," eventKind="};
    const char* b[]={" validBefore="," ownerBefore="," recordingBefore="," modeBefore="," phaseBefore="," transitionBefore="," countdownBefore="," loadingBefore="," lifecycleBefore="," clockBeforeBits="," requestedBeforeBits="," appliedBeforeBits="," pausedBefore="," pendingTrafficBefore="," pendingRetainedBefore="," spoolStateBefore="," selectedBeforeBits="," eventTypeBefore="," eventKindBefore="};
    const U32 values[]={v.valid,v.owner,v.recording,v.mode,v.phase,v.transition,v.countdown,v.loading,v.lifecycle,v.clock,v.requested,v.applied,v.paused,v.traffic,v.retained,v.spool,v.selected,v.eventType,v.eventKind};
    for(U32 i=0;i<19u;++i)if(i==1u || i==2u || i==9u || i==10u || i==11u || i==16u)DiagFAppendKeyHex(p,before?b[i]:a[i],values[i],end);else DiagFAppendKeyU(p,before?b[i]:a[i],values[i],end);
}
static const char* const kRHNames[]={"gate20","gate24","step","seek","stream","update","import","restore","request"};
static void RHPoll() {
    RHRecord r={};U32 drained=0;
    while(drained++<512u && RHPop(r)) {
        static char line[3500]={};char* p=line;const char* end=line+sizeof(line)-1;
        const char* names[]={"window","state","call","hold","recovered","gateWait"};
        AppendText(p,"DSFV29s_REPLAY_HITCH_EVENT kind=",end);AppendText(p,names[r.kind],end);AppendText(p," op=",end);AppendText(p,kRHNames[r.op],end);
        DiagFAppendKeyU(p," tick=",r.tick,end);DiagFAppendKeyU(p," thread=",r.thread,end);DiagFAppendKeyU(p," duration=",r.duration,end);
        DiagFAppendKeyU(p," spanMs=",r.span,end);DiagFAppendKeyU(p," observerBodyTicks=",r.overhead,end);
        DiagFAppendKeyHex(p," caller=",r.caller,end);DiagFAppendKeyHex(p," arg0=",r.arg0,end);DiagFAppendKeyHex(p," arg1=",r.arg1,end);DiagFAppendKeyHex(p," result=",r.result,end);
        RHAppendView(p,end,r.before,true);RHAppendView(p,end,r.after,false);
        if(r.kind==0u || r.kind==3u || r.kind==4u) {
            for(U32 i=0;i<RHKinds;++i){AppendText(p," ",end);AppendText(p,kRHNames[i],end);AppendText(p,"Calls=",end);DiagFAppendU(p,r.stats.calls[i],end);
                AppendText(p," ",end);AppendText(p,kRHNames[i],end);AppendText(p,"Ticks=",end);DiagFAppendU(p,r.stats.ticks[i],end);
                AppendText(p," ",end);AppendText(p,kRHNames[i],end);AppendText(p,"MaxTicks=",end);DiagFAppendU(p,r.stats.maximum[i],end);}
            const char* keys[]={" busy20="," busy24="," subscriberOnlyBusy="," snapshotPendingAtBusy="," trafficPendingAtBusy="," unknownBusy="," advanced="," unchanged="," backwards="," maxStepGapMs="," maxUpdateGapMs="};
            const U32 values[]={r.stats.busy20,r.stats.busy24,r.stats.subscriber,r.stats.snapshot,r.stats.traffic,r.stats.unknown,r.stats.advanced,r.stats.unchanged,r.stats.backwards,r.stats.maxStepGap,r.stats.maxUpdateGap};
            for(U32 i=0;i<11u;++i)DiagFAppendKeyU(p,keys[i],values[i],end);
        }
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    static U32 last=0;const U32 now=RHTick();if(now-last<1000u)return;last=now;
    char line[400]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_HITCH_STATUS tick=",end);DiagFAppendU(p,now,end);AppendText(p," status=",end);AppendNumber(p,g_rhStatus,end);
    DiagFAppendKeyU(p," dropped=",static_cast<U32>(g_rhDropped),end);DiagFAppendKeyU(p," threadDrops=",static_cast<U32>(g_rhThreadDrops),end);
    DiagFAppendKeyU(p," depthDrops=",static_cast<U32>(g_rhDepthDrops),end);DiagFAppendKeyU(p," mismatches=",static_cast<U32>(g_rhMismatches),end);DiagFAppendKeyU(p," clockFailures=",static_cast<U32>(g_rhClockFailures),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

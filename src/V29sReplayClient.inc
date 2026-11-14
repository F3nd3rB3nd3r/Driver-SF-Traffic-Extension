#if defined(DSF_V29S_REPLAY_HITCH)
#error The narrow replay client probe replaces the broad timing probe
#endif
#if !defined(DSF_V29S_REGMAP_REUSE) || !defined(DSF_V29S_REPLAY_PAUSE)
#error Replay client probe requires the current R2 baseline
#endif
extern "C" __declspec(dllimport) BOOL __stdcall QueryPerformanceCounter(__int64*);
extern "C" __declspec(dllimport) BOOL __stdcall QueryPerformanceFrequency(__int64*);
#include "V29sReplayClientCore.inc"
static __int64 g_scFrequency=0;
static UPtr SCAddress(U32 at){return Address(at);}
static U32 SCThreadId(){return GetCurrentThreadId();}
static U32 SCTick(){return GetTickCount()-g_capacityStart;}
static __int64 SCNow(){__int64 n=0;if(!QueryPerformanceCounter(&n))InterlockedIncrement(&g_scClockFailures);return n;}
#include "V29sReplayClientData.inc"
#include "V29sReplayClientNative.inc"
#include "V29sReplayClientBridges.inc"
#include "V29sReplayClientLayout.inc"
static void SCAppendNode(char*& p,const char* end,const SCNode& n,U32 i) {
    const char* prefix[]={" cur"," next"," future"};
    const char* suffix[]={"Ptr=","Valid=","TypeFlags=","TimeBits=","Bytes=","Link="};
    const U32 values[]={n.pointer,n.valid,n.typeFlags,n.timeBits,n.bytes,n.next};
    for(U32 j=0;j<6u;++j){AppendText(p,prefix[i],end);AppendText(p,suffix[j],end);if(j==1u || j==4u)DiagFAppendU(p,values[j],end);else DiagFAppendKeyHex(p,"",values[j],end);}
}
static void SCAppendView(char*& p,const char* end,const SCView& v) {
    const char* keys[]={" owner="," self="," recording="," vtable="," mode="," clockBits="," requestedBits="," phase="," transition="," loading="," countdown="," vehiclePending=",
        " recordingValid="," backend="," recordingVtable="," ioMode="," capacity="," base="," retained="," filled="," parse="," ringValid="," freeBytes="," usedBytes="," unparsedBytes="," filePosition="," inFlightBytes="," endOfData="," active="," eventHead="};
    const U32 values[]={v.owner,v.self,v.recording,v.vtable,v.mode,v.clockBits,v.requestedBits,v.phase,v.transition,v.loading,v.countdown,v.vehiclePending,
        v.recordingValid,v.backend,v.recordingVtable,v.ioMode,v.capacity,v.base,v.retained,v.filled,v.parse,v.ringValid,v.freeBytes,v.usedBytes,v.unparsedBytes,v.filePosition,v.inFlightBytes,v.endOfData,v.active,v.eventHead};
    for(U32 i=0;i<30u;++i)if(i<4u || i==5u || i==6u || i==14u || (i>=17u && i<=20u) || i==29u)DiagFAppendKeyHex(p,keys[i],values[i],end);else DiagFAppendKeyU(p,keys[i],values[i],end);
    for(U32 i=0;i<3u;++i)SCAppendNode(p,end,v.nodes[i],i);
#if defined(DSF_V29S_REPLAY_READ1M)
    RFAppend(p,end,v.refill);
#endif
}
static void SCPoll() {
    static char line[4000]={};SCRecord r={};U32 drained=0;
    while(drained++<512u && SCPop(r)) {
        char* p=line;const char* end=line+sizeof(line)-1;const char* names[]={"busy","clear","held","contextEnd"};
        AppendText(p,"DSFV29s_REPLAY_CLIENT_EVENT kind=",end);AppendText(p,names[r.kind],end);
        DiagFAppendKeyU(p," client=",kSCClients[r.index].id,end);DiagFAppendKeyU(p," sequence=",r.sequence,end);DiagFAppendKeyU(p," thread=",r.thread,end);
        DiagFAppendKeyU(p," tick=",r.tick,end);DiagFAppendKeyHex(p," caller=",r.caller,end);DiagFAppendKeyU(p," elapsedTicks=",r.elapsedTicks,end);DiagFAppendKeyU(p," captureTicks=",r.captureTicks,end);
        SCAppendView(p,end,r.view);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    static U32 last=0;const U32 now=SCTick();if(now-last<1000u)return;last=now;
    char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_CLIENT_STATUS tick=",end);DiagFAppendU(p,now,end);AppendText(p," status=",end);AppendNumber(p,g_scStatus,end);
    const char* keys[]={" fastClear="," observed="," busy="," began="," cleared="," contextEnds="," held="," open="," dropped="," threadDrops="," unknown="," readFailures="," readChecks="," clockFailures="," busy1="," busy2="," busy3="," busy7="};
    const long values[]={g_scFastClear,g_scObserved,g_scBusy,g_scBegin,g_scEnd,g_scAbandoned,g_scHeld,g_scOpenCount,g_scDropped,g_scThreadDrops,g_scUnknown,g_scReadFailures,g_scReadChecks,g_scClockFailures,g_scClientBusy[0],g_scClientBusy[1],g_scClientBusy[2],g_scClientBusy[3]};
    for(U32 i=0;i<18u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

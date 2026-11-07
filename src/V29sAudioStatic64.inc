#if !defined(DSF_V29S_AUDIO_DEFAULT_16M) || !defined(DSF_V29S_AUDIO_PROBE)
#error Static64 comparison requires the working default16MiB audio-probe baseline
#endif
#include "V29sAudioStatic64Patch.inc"
static void AudioStaticPoolStartLine() {
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_STATIC_START revision=1 status=",end);AppendNumber(p,g_audioStaticStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioStaticReason,end);
    DiagFAppendKeyU(p," patches=",g_audioStaticPatched?1u:0u,end);
    DiagFAppendKeyU(p," stockRequestedBytes=",kAudioStaticStockBytes,end);
    DiagFAppendKeyU(p," testRequestedBytes=",kAudioStaticTestBytes,end);
    AppendText(p," changedPool=StaticBanksPool defaultRequestedBytes=16777216 thresholdChanged=0 fixedObjectPoolsChanged=0 comparisonBaseline=run6736\n",end);
    *p=0;CapacityWrite(line);
}
static void AudioStaticPoolPoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;
    if(now-last<1000u && last)return;last=now;
    if(!Readable(Address(kRootPtr),4u) || !Readable(Address(0x01D72388u),12u))return;
    const UPtr root=Read32(Address(kRootPtr));
    if(!root || !Readable(root+0x50u,4u))return;
    const UPtr owner=Read32(root+0x50u);
    if(!owner || !Readable(owner+0xA34u,4u) || !Readable(owner+0x9548u,1u) || Read8(owner+0x9548u)!=1u)return;
    const U32 id=Read32(owner+0xA34u),max=Read32(Address(0x01D72388u));
    const UPtr table=Read32(Address(0x01D72390u));
    if(!max || max>128u || id>=max || !table)return;
    const UPtr record=table+id*0x4Cu;
    if(!Readable(record,0x4Cu) || !Read32(record))return;
    const U32 used=Read32(record+0x44u),total=Read32(record+0x48u),backing=Read32(record+8u);
    if(Read32(Address(0x01D72390u))!=table || Read32(owner+0xA34u)!=id ||
       Read32(record+8u)!=backing || Read32(record+0x48u)!=total)return;
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_STATIC_POOL",end);DiagFAppendKeyU(p," tick=",now,end);
    AppendText(p," patchStatus=",end);AppendNumber(p,g_audioStaticStatus,end);
    DiagFAppendKeyU(p," poolId=",id,end);DiagFAppendKeyU(p," requestedBytes=",Read32(Address(kAudioStaticSizeSite)+1u),end);
    DiagFAppendKeyU(p," usedBytes=",used,end);DiagFAppendKeyU(p," usableBytes=",total,end);
    DiagFAppendKeyU(p," blocks=",Read32(record),end);DiagFAppendKeyU(p," blockBytes=",Read32(record+4u),end);
    DiagFAppendKeyHex(p," backing=",backing,end);
    AppendText(p," playbackGateAppliesToThisPool=0\n",end);*p=0;CapacityWrite(line);
}

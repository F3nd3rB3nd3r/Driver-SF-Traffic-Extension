#if !defined(DSF_V29S_AUDIO_PROBE) || !defined(DSF_V29S_INDEXED_262144)
#error Audio default-pool experiment requires the R2 audio-probe baseline
#endif
static void AudioDefaultPoolStartLine() {
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_LIMIT_START revision=1 status=",end);AppendNumber(p,g_audioDefaultStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioDefaultReason,end);
    DiagFAppendKeyU(p," patches=",g_audioDefaultPatched?1u:0u,end);
    DiagFAppendKeyU(p," stockRequestedBytes=",kAudioDefaultStockBytes,end);
    DiagFAppendKeyU(p," testRequestedBytes=",kAudioDefaultTestBytes,end);
    AppendText(p," thresholdBits=3F666666 thresholdChanged=0 changedPool=sharedDefault fixedPoolsChanged=0\n",end);
    *p=0;CapacityWrite(line);
}
static void AudioDefaultPoolPoll() {
    static U32 last=0,checks=0,above=0;
    const U32 now=GetTickCount()-g_capacityStart;
    if(!Readable(Address(0x01D72388u),12u) || !Readable(Address(0x00FDD3B8u),4u) ||
       !Readable(Address(0x01D71870u),8u))return;
    const U32 max=Read32(Address(0x01D72388u)),id=Read32(Address(0x00FDD3B8u));
    const UPtr table=Read32(Address(0x01D72390u));
    if(!max || max>128u || id>=max || !table)return;
    const UPtr record=table+id*0x4Cu;
    if(!Readable(record,0x4Cu) || !Read32(record))return;
    const U32 used=Read32(record+0x44u),total=Read32(record+0x48u);
    const U32 configured=Read32(Address(0x01D71870u));
    const U32 thresholdBits=Read32(Address(0x01D71874u));
    if(!total || thresholdBits!=0x3F666666u)return;
    if(Read32(Address(0x01D72390u))!=table || Read32(Address(0x00FDD3B8u))!=id || Read32(record+0x48u)!=total)return;
    // Same strict used/total > threshold policy as native C66530. Snapshot only;
    // never invoke middleware, change the policy or walk playback objects here.
    const bool pressured=AudioDefaultAbove90(used,total);
    ++checks;if(pressured)++above;
    if(now-last<1000u && last)return;last=now;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_PRESSURE",end);DiagFAppendKeyU(p," tick=",now,end);
    AppendText(p," patchStatus=",end);AppendNumber(p,g_audioDefaultStatus,end);
    DiagFAppendKeyU(p," poolId=",id,end);DiagFAppendKeyU(p," configuredBytes=",configured,end);
    DiagFAppendKeyU(p," usedBytes=",used,end);DiagFAppendKeyU(p," usableBytes=",total,end);
    DiagFAppendKeyHex(p," thresholdBits=",thresholdBits,end);
    DiagFAppendKeyU(p," aboveThresholdSampled=",pressured?1u:0u,end);
    DiagFAppendKeyU(p," pressureSamples=",above,end);DiagFAppendKeyU(p," samples=",checks,end);
    AppendText(p," exactNativeGateHooked=0\n",end);*p=0;CapacityWrite(line);
}

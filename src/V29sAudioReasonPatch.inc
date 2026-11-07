static long g_audioReasonStatus=0;
static const char* g_audioReasonReason="notAttempted";
static U32 g_audioReasonInstalled=0;
extern "C" UPtr g_audioReasonTramps[10]={};
struct AudioReasonSite { U32 address,length;U8 bytes[12];UPtr bridge; };
static const AudioReasonSite kAudioReasonSites[10]={
    {0x00C5400Bu,7u,{0x3B,0xC6,0x89,0x44,0x24,0x20,0x5E},reinterpret_cast<UPtr>(&AudioReasonLookupBridge)},
    {0x00C54057u,6u,{0x8B,0x4C,0x24,0x1C,0x8B,0x11},reinterpret_cast<UPtr>(&AudioReasonCreateBridge)},
    {0x0084CC98u,7u,{0x83,0xBE,0x6C,0x01,0x00,0x00,0x10},reinterpret_cast<UPtr>(&AudioReasonGateBridge)},
    {0x0084CCC9u,5u,{0x83,0xC4,0x1C,0x3B,0xC3},reinterpret_cast<UPtr>(&AudioReasonResultBridge)},
    {0x0084C9B0u,6u,{0x8B,0x0D,0xFC,0xD0,0x43,0x01},reinterpret_cast<UPtr>(&AudioReasonDeliveredBridge)},
    {0x0084C920u,5u,{0x83,0xEC,0x08,0x53,0x55},reinterpret_cast<UPtr>(&AudioReasonHandledBridge)},
    {0x0084CE5Fu,6u,{0xFF,0x89,0x40,0x01,0x00,0x00},reinterpret_cast<UPtr>(&AudioReasonRemoveBridge)},
    {0x0084CD90u,5u,{0x8B,0x44,0x24,0x04,0x56},reinterpret_cast<UPtr>(&AudioReasonNameBridge)},
    {0x0084CDE0u,5u,{0x8B,0x44,0x24,0x04,0x56},reinterpret_cast<UPtr>(&AudioReasonNameDefaultBridge)},
    {0x0084C2F0u,6u,{0x56,0x8B,0xF1,0x56,0x89,0x36},reinterpret_cast<UPtr>(&AudioReasonRegisterBridge)}
};
static bool AudioReasonSignature(bool patched) {
    for(U32 i=0;i<10u;++i) {
        const AudioReasonSite& s=kAudioReasonSites[i];const UPtr at=Address(s.address);
        if(!Readable(at,s.length))return false;
        for(U32 j=0;j<s.length;++j) {
            const U8 expected=patched?(j==0u?0xE9u:j>=5u?0x90u:static_cast<U8>((s.bridge-(at+5u))>>((j-1u)*8u))):s.bytes[j];
            if(Read8(at+j)!=expected)return false;
        }
    }
    return true;
}
static bool AudioReasonInstall() {
    g_audioReasonStatus=-1;g_audioReasonReason="baselineUnavailable";
    if(g_audioSlotsStatus!=1 || !g_audioSlotsPatched || g_audioSlotsInstalled!=9u ||
       !AudioSlotsSignature(true) || !VerifyExecutableIdentity())return false;
    g_audioReasonReason="audioAlreadyInitialized";if(!AudioSlotsCold())return false;
    g_audioReasonReason="signature";if(!AudioReasonSignature(false))return false;
    g_audioReasonReason="trampolineAllocation";
    for(U32 i=0;i<10u;++i) {
        const AudioReasonSite& s=kAudioReasonSites[i];
        const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_audioReasonTramps[i]=t;
        for(U32 j=0;j<s.length;++j)*reinterpret_cast<U8*>(t+j)=s.bytes[j];
        *reinterpret_cast<U8*>(t+s.length)=0xE9u;
        Write32(t+s.length+1u,Address(s.address)+s.length-(t+s.length+5u));
    }
    const UPtr pages[2]={Address(0x0084C000u),Address(0x00C54000u)};
    DWORD old[2]={},unused=0;U32 writable=0;
    g_audioReasonReason="protectPreflight";
    for(;writable<2u;++writable)
        if(!VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,kPageExecuteReadWrite,&old[writable]))break;
    if(writable!=2u) {
        while(writable){--writable;VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,old[writable],&unused);}
        return false;
    }
    if(!AudioSlotsCold() || !AudioSlotsSignature(true) || !AudioReasonSignature(false)) {
        for(U32 i=0;i<2u;++i)VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused);
        g_audioReasonReason="precommitChanged";return false;
    }
    for(U32 i=0;i<10u;++i) {
        const AudioReasonSite& s=kAudioReasonSites[i];const UPtr at=Address(s.address);
        *reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-(at+5u));
        for(U32 j=5u;j<s.length;++j)*reinterpret_cast<volatile U8*>(at+j)=0x90u;
    }
    const bool matches=AudioReasonSignature(true);
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),nullptr,0u);
    bool protectedOk=true;
    for(U32 i=0;i<2u;++i)if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))protectedOk=false;
    if(matches && flushed && protectedOk) {
        g_audioReasonInstalled=10u;g_audioReasonStatus=1;g_audioReasonReason="armedBeforeAudioInit";return true;
    }
    bool restored=true;
    for(U32 i=0;i<2u;++i) {
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,kPageExecuteReadWrite,&unused)){restored=false;continue;}
        for(U32 n=0;n<10u;++n) {
            const AudioReasonSite& s=kAudioReasonSites[n];
            if((s.address&~0xFFFu)!=(i==0u?0x0084C000u:0x00C54000u))continue;
            for(U32 j=0;j<s.length;++j)*reinterpret_cast<volatile U8*>(Address(s.address)+j)=s.bytes[j];
        }
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))restored=false;
    }
    if(!FlushInstructionCache(GetCurrentProcess(),nullptr,0u) || !AudioReasonSignature(false))restored=false;
    g_audioReasonStatus=restored?-1:-2;g_audioReasonReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

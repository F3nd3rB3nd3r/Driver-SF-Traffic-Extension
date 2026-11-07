// Shared verbatim with the fault-injected Win32 installer fixture.
static long g_audioSlotsStatus=0;
static const char* g_audioSlotsReason="notAttempted";
static U32 g_audioSlotsInstalled=0;
static bool g_audioSlotsPatched=false;
extern "C" UPtr g_audioSlotsTramps[9]={};
struct AudioSlotsSite { U32 address,length;U8 bytes[12];UPtr bridge; };
static const AudioSlotsSite kAudioSlotsSites[9]={
    {0x00C6B42Fu,7u,{0x8B,0x47,0x04,0x2B,0x07,0x33,0xF6},reinterpret_cast<UPtr>(&AudioSlotsAGateBridge)},
    {0x00C6B92Bu,6u,{0xC1,0xF9,0x02,0x3B,0x4F,0x10},reinterpret_cast<UPtr>(&AudioSlotsBGateBridge)},
    {0x00C6B4F4u,7u,{0x80,0x7C,0x24,0x10,0x00,0x89,0x30},reinterpret_cast<UPtr>(&AudioSlotsAInsertBridge)},
    {0x00C6B975u,6u,{0x89,0x30,0x8B,0xC6,0x5E,0x5F},reinterpret_cast<UPtr>(&AudioSlotsBInsertBridge)},
    {0x00C6B4A0u,9u,{0x6A,0x00,0x8B,0xCE,0xF3,0x0F,0x11,0x56,0x1C},reinterpret_cast<UPtr>(&AudioSlotsAReuseBridge)},
    {0x00C6B9B5u,6u,{0x8B,0x53,0x28,0x2B,0x53,0x24},reinterpret_cast<UPtr>(&AudioSlotsEightBridge)},
    {0x00C6B589u,9u,{0x80,0x7F,0x39,0xFF,0xBE,0x1B,0x00,0x00,0x00},reinterpret_cast<UPtr>(&AudioSlotsByteBridge)},
    {0x00C6B2B6u,5u,{0x8B,0x2F,0x3B,0x6F,0x04},reinterpret_cast<UPtr>(&AudioSlotsAUpdateBridge)},
    {0x00C6B794u,6u,{0x8B,0x77,0x04,0x3B,0x77,0x08},reinterpret_cast<UPtr>(&AudioSlotsBUpdateBridge)}
};
static bool AudioSlotsCold() {
    return AudioDefaultCold() && Readable(Address(0x01D7184Cu),8u) &&
        !Read32(Address(0x01D7184Cu)) && !Read32(Address(0x01D71850u));
}
static bool AudioSlotsSignature(bool patched) {
    const UPtr at=Address(0x00C50F0Cu);
    if(!Readable(at,5u) || Read8(at)!=0xB9u || Read32(at+1u)!=(patched?kAudioSlotsCount:255u))return false;
    if(!Readable(Address(0x00C50F17u),6u) || Read32(Address(0x00C50F17u))!=0x89044889u ||
       Read8(Address(0x00C50F1Bu))!=0x48u || Read8(Address(0x00C50F1Cu))!=8u)return false;
    for(U32 i=0;i<9u;++i) {
        const AudioSlotsSite& s=kAudioSlotsSites[i];const UPtr p=Address(s.address);
        if(!Readable(p,s.length))return false;
        for(U32 j=0;j<s.length;++j) {
            const U8 expected=patched?(j==0u?0xE9u:j>=5u?0x90u:static_cast<U8>((s.bridge-(p+5u))>>((j-1u)*8u))):s.bytes[j];
            if(Read8(p+j)!=expected)return false;
        }
    }
    return true;
}
static bool AudioSlotsInstall() {
    g_audioSlotsStatus=-1;g_audioSlotsReason="baselineUnavailable";
    if(g_audioDefaultStatus!=1 || g_audioProbeStatus!=1 || g_capacityHooks!=3 ||
       g_indexedStatus!=1 || !g_indexedStartupVerified || !VerifyExecutableIdentity())return false;
    g_audioSlotsReason="audioAlreadyInitialized";if(!AudioSlotsCold())return false;
    g_audioSlotsReason="signature";if(!AudioSlotsSignature(false))return false;
    g_audioSlotsReason="trampolineAllocation";
    for(U32 i=0;i<9u;++i) {
        const AudioSlotsSite& s=kAudioSlotsSites[i];
        const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_audioSlotsTramps[i]=t;
        for(U32 j=0;j<s.length;++j)*reinterpret_cast<U8*>(t+j)=s.bytes[j];
        *reinterpret_cast<U8*>(t+s.length)=0xE9;
        Write32(t+s.length+1u,Address(s.address)+s.length-(t+s.length+5u));
    }
    const UPtr pages[2]={Address(0x00C50000u),Address(0x00C6B000u)};
    DWORD old[2]={},unused=0;U32 writable=0;
    g_audioSlotsReason="protectPreflight";
    for(;writable<2u;++writable)
        if(!VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,kPageExecuteReadWrite,&old[writable]))break;
    if(writable!=2u) {
        while(writable){--writable;VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,old[writable],&unused);}
        return false;
    }
    // DllMain cold phase: no native audio manager exists yet. Recheck everything
    // after page protection and commit hooks + paired settings as one unit.
    if(!AudioSlotsCold() || !AudioSlotsSignature(false)) {
        for(U32 i=0;i<2u;++i)VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused);
        g_audioSlotsReason="precommitChanged";return false;
    }
    for(U32 i=0;i<9u;++i) {
        const AudioSlotsSite& s=kAudioSlotsSites[i];const UPtr at=Address(s.address);
        *reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-(at+5u));
        for(U32 j=5u;j<s.length;++j)*reinterpret_cast<volatile U8*>(at+j)=0x90u;
    }
    Write32(Address(0x00C50F0Du),kAudioSlotsCount);
    const bool matches=AudioSlotsSignature(true);
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),nullptr,0u);
    bool protectedOk=true;
    for(U32 i=0;i<2u;++i)if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))protectedOk=false;
    if(matches && flushed && protectedOk) {
        g_audioSlotsPatched=true;g_audioSlotsInstalled=9u;g_audioSlotsStatus=1;
        g_audioSlotsReason="armedBeforeAudioInit";return true;
    }
    bool rollbackOk=true;
    for(U32 i=0;i<2u;++i) {
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,kPageExecuteReadWrite,&unused)){rollbackOk=false;continue;}
        if(i==0u)Write32(Address(0x00C50F0Du),255u);
        else for(U32 n=0;n<9u;++n) {
            const AudioSlotsSite& s=kAudioSlotsSites[n];
            for(U32 j=0;j<s.length;++j)*reinterpret_cast<volatile U8*>(Address(s.address)+j)=s.bytes[j];
        }
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))rollbackOk=false;
    }
    if(!FlushInstructionCache(GetCurrentProcess(),nullptr,0u) || !AudioSlotsSignature(false))rollbackOk=false;
    g_audioSlotsPatched=Read32(Address(0x00C50F0Du))!=255u;
    g_audioSlotsStatus=rollbackOk?-1:-2;g_audioSlotsReason=rollbackOk?"commitFailedRestored":"rollbackFailed";
    return false;
}

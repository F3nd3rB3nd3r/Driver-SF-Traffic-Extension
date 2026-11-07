// Optional one-variable comparison against the working default16MiB baseline.
static const UPtr kAudioStaticSizeSite=0x0084A683u;
static const U32 kAudioStaticStockBytes=0x0042B800u,kAudioStaticTestBytes=0x04000000u;
static long g_audioStaticStatus=0;
static const char* g_audioStaticReason="notAttempted";
static bool g_audioStaticPatched=false;
static bool AudioStaticInstructionMatches(U32 bytes) {
    const UPtr site=Address(kAudioStaticSizeSite);
    return Readable(site,5u) && Read8(site)==0x68u && Read32(site+1u)==bytes;
}
static bool AudioStaticCold() {
    // Unlike the default pool, this pool is created before SoundEngine::Init.
    // Require the entire audio memory manager and engine root still untouched.
    return AudioDefaultCold() && Readable(Address(kRootPtr),4u) && Read32(Address(kRootPtr))==0u &&
        Readable(Address(0x01D72388u),12u) && Read32(Address(0x01D72388u))==0u &&
        Read32(Address(0x01D7238Cu))==0u && Read32(Address(0x01D72390u))==0u;
}
static bool AudioStaticPoolInstall() {
    g_audioStaticStatus=-1;g_audioStaticReason="baselineUnavailable";
    if(g_audioProbeStatus!=1 || g_audioDefaultStatus!=1 || !g_audioDefaultPatched ||
       g_capacityHooks!=3 || g_indexedStatus!=1 || !g_indexedStockVerified ||
       !g_indexedStartupVerified || !VerifyExecutableIdentity() ||
       !AudioDefaultInstructionMatches(kAudioDefaultTestBytes))return false;
    g_audioStaticReason="audioAlreadyInitialized";
    if(!AudioStaticCold())return false;
    g_audioStaticReason="signature";
    const U8 context[8]={0x68,0x00,0x04,0x00,0x00,0x83,0xCB,0xFF}; // block1024, EBX=-1
    const U8 create[5]={0xE8,0xF5,0xF6,0x44,0x00}; // call C99D90
    if(!AudioStaticInstructionMatches(kAudioStaticStockBytes) ||
       !Readable(Address(0x0084A67Bu),8u) || !Readable(Address(0x0084A696u),5u))return false;
    for(U32 i=0;i<8u;++i)if(Read8(Address(0x0084A67Bu)+i)!=context[i])return false;
    for(U32 i=0;i<5u;++i)if(Read8(Address(0x0084A696u)+i)!=create[i])return false;
    const UPtr site=Address(kAudioStaticSizeSite);DWORD old=0,unused=0;
    g_audioStaticReason="protectFailed";
    if(!VirtualProtect(reinterpret_cast<void*>(site),5u,kPageExecuteReadWrite,&old))return false;
    if(!AudioStaticCold() || !AudioStaticInstructionMatches(kAudioStaticStockBytes)) {
        VirtualProtect(reinterpret_cast<void*>(site),5u,old,&unused);
        g_audioStaticReason="precommitChanged";return false;
    }
    Write32(site+1u,kAudioStaticTestBytes);
    g_audioStaticPatched=AudioStaticInstructionMatches(kAudioStaticTestBytes);
    const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5u);
    const BOOL protect=VirtualProtect(reinterpret_cast<void*>(site),5u,old,&unused);
    if(g_audioStaticPatched && flush && protect) {
        g_audioStaticStatus=1;g_audioStaticReason="armedBeforeAudioMemoryInit";return true;
    }
    bool restored=false;
    if(VirtualProtect(reinterpret_cast<void*>(site),5u,kPageExecuteReadWrite,&unused)) {
        Write32(site+1u,kAudioStaticStockBytes);
        const BOOL rollbackFlush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5u);
        const BOOL rollbackProtect=VirtualProtect(reinterpret_cast<void*>(site),5u,old,&unused);
        restored=AudioStaticInstructionMatches(kAudioStaticStockBytes) && rollbackFlush && rollbackProtect;
    }
    g_audioStaticPatched=!AudioStaticInstructionMatches(kAudioStaticStockBytes);
    g_audioStaticStatus=restored?-1:-2;
    g_audioStaticReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

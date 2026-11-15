// Shared with the isolated startup/rollback harness. A single allocation-size
// operand changes; the native pressure policy and all fixed object pools remain.
static const UPtr kAudioDefaultSizeSite=0x0084A64Du;
static const U32 kAudioDefaultStockBytes=0x00599999u;
#if defined(DSF_V29S_GENERIC_LIMITS)
#define kAudioDefaultTestBytes (g_limits.audioMiB*1048576u)
#elif defined(DSF_V29S_AUDIO_DEFAULT_64M)
static const U32 kAudioDefaultTestBytes=0x04000000u;
#else
static const U32 kAudioDefaultTestBytes=0x01000000u;
#endif
static const U8 kAudioDefaultStockInstruction[7]={0xC7,0x45,0x40,0x99,0x99,0x59,0x00};
static long g_audioDefaultStatus=0;
static const char* g_audioDefaultReason="notAttempted";
static bool g_audioDefaultPatched=false;
static bool AudioDefaultAbove90(U32 used,U32 total) {
    // 0x3F666666 is exactly 0xE66666 / 2^24. Compare products without
    // floating-point state, 32-bit overflow, rounding or CRT division helpers.
    U32 above=0;
    __asm {
        mov eax,total
        mov ecx,00E66666h
        mul ecx
        mov ecx,used
        mov ebx,ecx
        shl ecx,24
        shr ebx,8
        cmp ebx,edx
        ja audio_ratio_above
        jb audio_ratio_done
        cmp ecx,eax
        jbe audio_ratio_done
    audio_ratio_above:
        mov above,1
    audio_ratio_done:
    }
    return total!=0u && above!=0u;
}
static bool AudioDefaultInstructionMatches(U32 bytes) {
    const UPtr site=Address(kAudioDefaultSizeSite);
    return Readable(site,7u) && Read8(site)==0xC7u && Read8(site+1u)==0x45u &&
        Read8(site+2u)==0x40u && Read32(site+3u)==bytes;
}
static bool AudioDefaultCold() {
    return Readable(Address(0x00FDD3B8u),4u) && Read32(Address(0x00FDD3B8u))==0xFFFFFFFFu &&
        Readable(Address(0x01D7183Cu),16u) && Read32(Address(0x01D7183Cu))==0u &&
        Read32(Address(0x01D71848u))==0u && Readable(Address(0x01D71940u),4u) &&
        Read32(Address(0x01D71940u))==0u;
}
static bool AudioDefaultPoolInstall() {
    g_audioDefaultStatus=-1;g_audioDefaultReason="baselineUnavailable";
    if(g_capacityHooks!=3 || g_indexedStatus!=1 || !g_indexedStockVerified ||
       !g_indexedStartupVerified || !VerifyExecutableIdentity())return false;
    g_audioDefaultReason="audioAlreadyInitialized";
    if(!AudioDefaultCold())return false;
    g_audioDefaultReason="signatureOrThreshold";
    if(!AudioDefaultInstructionMatches(kAudioDefaultStockBytes) ||
       !Readable(Address(0x00DD6CD4u),4u) || Read32(Address(0x00DD6CD4u))!=0x3F666666u)return false;
#if defined(DSF_V29S_AUDIO_DEFAULT_64M)
    // This comparison expands only the shared default pool. Refuse an already
    // modified StaticBanksPool budget instead of accidentally combining tests.
    if(!Readable(Address(0x0084A683u),5u) || Read8(Address(0x0084A683u))!=0x68u ||
       Read32(Address(0x0084A684u))!=0x0042B800u)return false;
#endif
    // Check the exact source and both stores of the 0.9 pressure threshold.
    const U8 thresholdLoad[8]={0xF3,0x0F,0x10,0x05,0xD4,0x6C,0xDD,0x00};
    const U8 thresholdStores[10]={0xF3,0x0F,0x11,0x45,0xE8,0xF3,0x0F,0x11,0x45,0x44};
    if(!Readable(Address(0x0084A62Eu),8u) || !Readable(Address(0x0084A654u),10u))return false;
    for(U32 i=0;i<8u;++i)if(Read8(Address(0x0084A62Eu)+i)!=thresholdLoad[i])return false;
    for(U32 i=0;i<10u;++i)if(Read8(Address(0x0084A654u)+i)!=thresholdStores[i])return false;
    const UPtr site=Address(kAudioDefaultSizeSite);
    DWORD old=0,unused=0;
    g_audioDefaultReason="protectFailed";
    if(!VirtualProtect(reinterpret_cast<void*>(site),7u,kPageExecuteReadWrite,&old))return false;
    // A late cold/signature change must still leave the allocation request stock.
    if(!AudioDefaultCold() || !AudioDefaultInstructionMatches(kAudioDefaultStockBytes)) {
        VirtualProtect(reinterpret_cast<void*>(site),7u,old,&unused);
        g_audioDefaultReason="precommitChanged";return false;
    }
    Write32(site+3u,kAudioDefaultTestBytes);
    g_audioDefaultPatched=AudioDefaultInstructionMatches(kAudioDefaultTestBytes);
    const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),7u);
    const BOOL protect=VirtualProtect(reinterpret_cast<void*>(site),7u,old,&unused);
    if(g_audioDefaultPatched && flush && protect) {
        g_audioDefaultStatus=1;g_audioDefaultReason="armedBeforeAudioInit";return true;
    }
    // Revert on ordinary OS commit failure; never silently claim success.
    bool restored=false;
    if(VirtualProtect(reinterpret_cast<void*>(site),7u,kPageExecuteReadWrite,&unused)) {
        Write32(site+3u,kAudioDefaultStockBytes);
        const BOOL rollbackFlush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),7u);
        const BOOL rollbackProtect=VirtualProtect(reinterpret_cast<void*>(site),7u,old,&unused);
        restored=AudioDefaultInstructionMatches(kAudioDefaultStockBytes) && rollbackFlush && rollbackProtect;
    }
    g_audioDefaultPatched=!AudioDefaultInstructionMatches(kAudioDefaultStockBytes);
    g_audioDefaultStatus=restored?-1:-2;
    g_audioDefaultReason=restored?"commitFailedRestored":"rollbackFailed";
    return false;
}

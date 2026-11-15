static long g_audioModelStatus=0;
static const char* g_audioModelReason="notAttempted";
static U32 g_audioModelInstalled=0;
extern "C" UPtr g_audioModelTramps[8]={};
struct AudioModelSite {U32 address,length;U8 bytes[12];UPtr bridge;};
static const AudioModelSite kAudioModelSites[8]={
    {0x00556E30u,6u,{0x83,0x79,0x0C,0x2D,0x73,0x13},reinterpret_cast<UPtr>(&AudioModelGateBridge)},
    {0x00556E43u,6u,{0x8B,0x4C,0x24,0x04,0x89,0x08},reinterpret_cast<UPtr>(&AudioModelStoreBridge)},
    {0x00556E80u,6u,{0xFF,0x46,0x04,0xFF,0x4E,0x08},reinterpret_cast<UPtr>(&AudioModelRemoveBridge)},
    {0x00556EBBu,6u,{0x81,0xC7,0xE0,0x02,0x00,0x00},reinterpret_cast<UPtr>(&AudioModelLookupBridge)},
    {0x00553860u,7u,{0x56,0x8B,0xF1,0x8B,0x4C,0x24,0x08},reinterpret_cast<UPtr>(&AudioModelSetupBridge)},
    {0x00553918u,8u,{0x84,0xC0,0x0F,0x84,0xB5,0x01,0x00,0x00},reinterpret_cast<UPtr>(&AudioModelBankBridge)},
    {0x00553ACFu,6u,{0xB0,0x01,0x5E,0xC2,0x08,0x00},reinterpret_cast<UPtr>(&AudioModelSuccessBridge)},
    {0x00553AD5u,6u,{0x32,0xC0,0x5E,0xC2,0x08,0x00},reinterpret_cast<UPtr>(&AudioModelFailureBridge)}
};
static bool AudioModelSignature(bool patched) {
    for(U32 i=0;i<8u;++i) {
        const AudioModelSite& s=kAudioModelSites[i];const UPtr at=Address(s.address);
        if(!Readable(at,s.length))return false;
        for(U32 j=0;j<s.length;++j) {
            const U8 expected=patched?(j==0u?0xE9u:j>=5u?0x90u:static_cast<U8>((s.bridge-(at+5u))>>((j-1u)*8u))):s.bytes[j];
            if(Read8(at+j)!=expected)return false;
        }
    }
    return true;
}
static bool AudioModelLayoutSignature() {
    // Preserve the factory, sentinel/default access and generic insertion code.
    return Readable(Address(0x00557000u),5u) && Read8(Address(0x00557000u))==0x68u &&
        Read32(Address(0x00557001u))==0x2F0u && Readable(Address(0x00DE3824u),24u) &&
        Read32(Address(0x00DE3828u))==0x00556E30u && Read32(Address(0x00DE3834u))==0x00556E90u &&
        Read32(Address(0x00DE3838u))==0x00556E10u &&
        Readable(Address(0x00556E10u),7u) && Read32(Address(0x00556E12u))==0x2ECu &&
        Readable(Address(0x007D72ACu),6u) && Read32(Address(0x007D72ACu))==0xFF100183u;
}
static bool AudioModelInstall() {
    g_audioModelStatus=-1;g_audioModelReason="baselineUnavailable";
    if(g_audioReasonStatus!=1 || !AudioReasonSignature(true) || !VerifyExecutableIdentity())return false;
    g_audioModelReason="audioAlreadyInitialized";if(!AudioSlotsCold())return false;
    g_audioModelReason="signature";if(!AudioModelSignature(false) || !AudioModelLayoutSignature())return false;
    g_audioModelReason="nodeStorageAllocation";
#if defined(DSF_V29S_WIDE_LIMITS)
    g_wideAudioBuckets=static_cast<AudioModelKey**>(WideAllocate(65536u*4u));
    // Nonzero armed marker; each stable manager owns its own guarded nodes.
    g_audioModelStorage=reinterpret_cast<UPtr>(g_wideAudioBuckets);
#else
    g_audioModelStorage=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,kAudioModelInstances*kAudioModelStride,kMemCommitReserve,0x04u));
    if(!g_audioModelStorage)return false;
    for(U32 i=0;i<8u;++i)for(U32 n=0;n<4u;++n)Write32(g_audioModelStorage+i*kAudioModelStride+kAudioModelCapacity*16u+n*4u,kAudioModelGuard);
#endif
    g_audioModelReason="trampolineAllocation";
    for(U32 i=0;i<8u;++i) {
        if(i==0u || i==5u)continue; // Explicit CMP/TEST branch bridges.
        const AudioModelSite& s=kAudioModelSites[i];
        const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_audioModelTramps[i]=t;
        for(U32 j=0;j<s.length;++j)*reinterpret_cast<U8*>(t+j)=s.bytes[j];
        *reinterpret_cast<U8*>(t+s.length)=0xE9u;Write32(t+s.length+1u,Address(s.address)+s.length-(t+s.length+5u));
    }
    const UPtr pages[2]={Address(0x00553000u),Address(0x00556000u)};
    DWORD old[2]={},unused=0;U32 writable=0;
    g_audioModelReason="protectPreflight";
    for(;writable<2u;++writable)if(!VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,kPageExecuteReadWrite,&old[writable]))break;
    if(writable!=2u) {
        while(writable){--writable;VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,old[writable],&unused);}return false;
    }
    if(!AudioSlotsCold() || !AudioReasonSignature(true) || !AudioModelSignature(false) || !AudioModelLayoutSignature()) {
        for(U32 i=0;i<2u;++i)VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused);
        g_audioModelReason="precommitChanged";return false;
    }
    for(U32 i=0;i<8u;++i) {
        const AudioModelSite& s=kAudioModelSites[i];const UPtr at=Address(s.address);
        *reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-(at+5u));
        for(U32 j=5u;j<s.length;++j)*reinterpret_cast<volatile U8*>(at+j)=0x90u;
    }
    const bool matches=AudioModelSignature(true);const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),nullptr,0u);
    bool protectedOk=true;for(U32 i=0;i<2u;++i)if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))protectedOk=false;
    if(matches && flushed && protectedOk){g_audioModelInstalled=8u;g_audioModelStatus=1;g_audioModelReason="armedColdEmptyOnly";return true;}
    bool restored=true;
    for(U32 i=0;i<2u;++i) {
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,kPageExecuteReadWrite,&unused)){restored=false;continue;}
        for(U32 n=0;n<8u;++n) {
            const AudioModelSite& s=kAudioModelSites[n];if((s.address&~0xFFFu)!=(i==0u?0x00553000u:0x00556000u))continue;
            for(U32 j=0;j<s.length;++j)*reinterpret_cast<volatile U8*>(Address(s.address)+j)=s.bytes[j];
        }
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))restored=false;
    }
    if(!FlushInstructionCache(GetCurrentProcess(),nullptr,0u) || !AudioModelSignature(false))restored=false;
    g_audioModelStatus=restored?-1:-2;g_audioModelReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

extern "C" UPtr g_pendingTramps[2]={};
struct PendingSite{U32 address,length;U8 bytes[8];UPtr bridge;};
static const PendingSite kPendingSites[2]={
    {0x005A2230u,6u,{0x83,0xEC,0x14,0x56,0x8B,0xF1},reinterpret_cast<UPtr>(&PendingBeginBridge)},
    {0x005A2466u,6u,{0x8A,0x44,0x24,0x0B,0x5F,0x5E},reinterpret_cast<UPtr>(&PendingEndBridge)}
};
static bool PendingSignature(bool patched) {
    for(const PendingSite& s:kPendingSites) {
        const UPtr at=Address(s.address);if(!Readable(at,s.length))return false;
        for(U32 n=0;n<s.length;++n) {
            const U8 expected=patched?(n==0u?0xE9u:n>=5u?0x90u:static_cast<U8>((s.bridge-at-5u)>>((n-1u)*8u))):s.bytes[n];
            if(Read8(at+n)!=expected)return false;
        }
    }return true;
}
static bool PendingParent(){return g_cleanupStatus==1 && CleanupSignature(true) && VerifyExecutableIdentity() && g_origMapInsert!=nullptr;}
static bool PendingInstall() {
    g_pendingStatus=-1;g_pendingReason="baselineUnavailable";if(!PendingParent())return false;
    g_pendingReason="notCold";if(!IndexedCold() || g_pkg0)return false;
    g_pendingReason="signature";if(!PendingSignature(false))return false;
    g_pendingReason="trampolineAllocation";
    for(U32 i=0;i<2u;++i) {
        const PendingSite& s=kPendingSites[i];const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_pendingTramps[i]=t;
        for(U32 n=0;n<s.length;++n)*reinterpret_cast<U8*>(t+n)=s.bytes[n];
        *reinterpret_cast<U8*>(t+s.length)=0xE9u;Write32(t+s.length+1u,Address(s.address)-t-5u);
    }
    const UPtr page=Address(0x005A2000u);DWORD old=0,unused=0;g_pendingReason="protectPreflight";
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&old))return false;
    if(!PendingParent() || !IndexedCold() || g_pkg0 || !PendingSignature(false)) {
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused)){g_pendingStatus=-2;g_pendingReason="precommitProtectionFailed";return false;}
        g_pendingReason="precommitChanged";return false;
    }
    for(const PendingSite& s:kPendingSites){const UPtr at=Address(s.address);*reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-at-5u);*reinterpret_cast<volatile U8*>(at+5u)=0x90u;}
    const bool exact=PendingSignature(true);const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
    const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused);
    if(exact && flushed && protectedAgain){g_pendingStatus=1;g_pendingReason="armedColdPrivateGenerations";return true;}
    bool restored=true;
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&unused))restored=false;
    else {
        for(const PendingSite& s:kPendingSites)for(U32 n=0;n<s.length;++n)*reinterpret_cast<volatile U8*>(Address(s.address)+n)=s.bytes[n];
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused))restored=false;
        if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u) || !PendingSignature(false))restored=false;
    }
    g_pendingStatus=restored?-1:-2;g_pendingReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

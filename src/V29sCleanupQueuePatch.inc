static long g_cleanupStatus=0;static const char* g_cleanupReason="notAttempted";static U32 g_cleanupInstalled=0;
extern "C" UPtr g_cleanupTramps[6]={};
struct CleanupSite{U32 address,length;U8 bytes[12];UPtr bridge;};
static const CleanupSite kCleanupSites[6]={
    {0x00BAEA66u,7u,{0x8D,0x84,0x81,0x90,0xE0,0x15,0x00},reinterpret_cast<UPtr>(&CleanupDirectBridge)},
    {0x00BAEC1Fu,7u,{0x8D,0x84,0x87,0x90,0xE0,0x15,0x00},reinterpret_cast<UPtr>(&CleanupReplaceBridge)},
    {0x00BAED3Au,7u,{0x8D,0x84,0x87,0x90,0xE0,0x15,0x00},reinterpret_cast<UPtr>(&CleanupReleaseBridge)},
    {0x00BAE6B0u,6u,{0x8B,0x15,0xA4,0x49,0x6F,0x01},reinterpret_cast<UPtr>(&CleanupBeginBridge)},
    {0x00BAE753u,10u,{0xC7,0x85,0x90,0xE0,0x17,0x00,0x00,0x00,0x00,0x00},reinterpret_cast<UPtr>(&CleanupEndBridge)},
    {0x00BAE761u,6u,{0x89,0x85,0x90,0xE0,0x17,0x00},reinterpret_cast<UPtr>(&CleanupEmptyBridge)}
};
struct CleanupOperand{U32 address,offset;U8 bytes[6];};
static const CleanupOperand kCleanupOperands[2]={
    {0x00BAE6D3u,2u,{0x8D,0xB5,0x90,0xE0,0x15,0x00}},
    {0x00BAE6E4u,2u,{0x8B,0x8E,0x00,0x00,0x02,0x00}}
};
static U32 CleanupOperandValue(U32 n){return n==0u?g_cleanupBase-Address(0x01732BF0u):Address(0x01732BF0u)+0x17E090u-g_cleanupBase;}
static bool CleanupSignature(bool patched) {
    for(U32 i=0;i<6u;++i) {
        const CleanupSite& s=kCleanupSites[i];const UPtr at=Address(s.address);if(!Readable(at,s.length))return false;
        for(U32 n=0;n<s.length;++n) {
            const U8 expected=patched?(n==0u?0xE9u:n>=5u?0x90u:static_cast<U8>((s.bridge-at-5u)>>((n-1u)*8u))):s.bytes[n];
            if(Read8(at+n)!=expected)return false;
        }
    }
    for(U32 i=0;i<2u;++i) {
        const CleanupOperand& s=kCleanupOperands[i];const UPtr at=Address(s.address);if(!Readable(at,6u))return false;
        for(U32 n=0;n<6u;++n) {
            const U8 expected=patched && n>=s.offset?static_cast<U8>(CleanupOperandValue(i)>>((n-s.offset)*8u)):s.bytes[n];
            if(Read8(at+n)!=expected)return false;
        }
    }return true;
}
static bool CleanupCold(){return IndexedCold() && Read32(Address(0x01732BF0u)+0x17E090u)==0u;}
static bool CleanupParent(){return g_audioModelStatus==1 && g_indexedStatus==1 && VerifyExecutableIdentity() && AudioModelSignature(true) && IndexedInstructionsMatch(true,g_indexedBase) && IndexedPendingRedirectIntact();}
static bool CleanupInstall() {
    g_cleanupStatus=-1;g_cleanupReason="baselineUnavailable";if(!CleanupParent())return false;
    g_cleanupReason="managerNotCold";if(!CleanupCold())return false;
    g_cleanupReason="signature";if(!CleanupSignature(false))return false;
    g_cleanupReason="storageAllocation";
    g_cleanupAllocation=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,kCleanupCapacity*4u+32u,kMemCommitReserve,0x04u));
    if(!g_cleanupAllocation)return false;g_cleanupBase=g_cleanupAllocation+16u;
    for(U32 n=0;n<4u;++n){Write32(g_cleanupBase-16u+n*4u,kCleanupGuard);Write32(g_cleanupBase+kCleanupCapacity*4u+n*4u,kCleanupGuard);}
    g_cleanupReason="trampolineAllocation";
    for(U32 i=0;i<6u;++i) {
        const CleanupSite& s=kCleanupSites[i];const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_cleanupTramps[i]=t;
        for(U32 n=0;n<s.length;++n)*reinterpret_cast<U8*>(t+n)=s.bytes[n];
        if(i<3u)Write32(t+3u,g_cleanupBase-Address(0x01732BF0u));
        *reinterpret_cast<U8*>(t+s.length)=0xE9u;Write32(t+s.length+1u,Address(s.address)-t-5u);
    }
    DWORD old=0,unused=0;const UPtr page=Address(0x00BAE000u);g_cleanupReason="protectPreflight";
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&old))return false;
    if(!CleanupParent() || !CleanupCold() || !CleanupSignature(false)) {
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused)){g_cleanupStatus=-2;g_cleanupReason="precommitProtectionFailed";return false;}
        g_cleanupReason="precommitChanged";return false;
    }
    for(U32 i=0;i<6u;++i) {
        const CleanupSite& s=kCleanupSites[i];const UPtr at=Address(s.address);
        *reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-at-5u);
        for(U32 n=5u;n<s.length;++n)*reinterpret_cast<volatile U8*>(at+n)=0x90u;
    }
    for(U32 i=0;i<2u;++i)Write32(Address(kCleanupOperands[i].address)+kCleanupOperands[i].offset,CleanupOperandValue(i));
    const bool exact=CleanupSignature(true);const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
    const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused);
    if(exact && flushed && protectedAgain){g_cleanupInstalled=6u;g_cleanupStatus=1;g_cleanupReason="armedColdNativeOrder";return true;}
    bool restored=true;
    if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&unused))restored=false;
    else {
        for(const CleanupSite& s:kCleanupSites)for(U32 n=0;n<s.length;++n)*reinterpret_cast<volatile U8*>(Address(s.address)+n)=s.bytes[n];
        for(const CleanupOperand& s:kCleanupOperands)for(U32 n=s.offset;n<6u;++n)*reinterpret_cast<volatile U8*>(Address(s.address)+n)=s.bytes[n];
        if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&unused))restored=false;
        if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u) || !CleanupSignature(false))restored=false;
    }
    g_cleanupStatus=restored?-1:-2;g_cleanupReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

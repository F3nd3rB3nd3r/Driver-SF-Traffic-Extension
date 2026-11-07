extern "C" UPtr g_callbackTramps[3]={};
struct CallbackSite {U32 address,length;U8 bytes[8];UPtr bridge;};
static const CallbackSite kCallbackSites[3]={
    {0x005567D0u,6u,{0x83,0x79,0x04,0x00,0x8B,0x01},reinterpret_cast<UPtr>(&CallbackInsertBridge)},
    {0x00858FB5u,6u,{0x8B,0x8E,0x60,0x05,0x00,0x00},reinterpret_cast<UPtr>(&CallbackWriterBridge)},
    {0x00859000u,5u,{0x51,0x53,0x56,0x8B,0xF1},reinterpret_cast<UPtr>(&CallbackPollBridge)}
};
static bool CallbackSignature(bool patched) {
    for(const CallbackSite& s:kCallbackSites) {
        const UPtr at=Address(s.address);if(!Readable(at,s.length))return false;
        for(U32 n=0;n<s.length;++n) {
            const U8 expected=patched?(n==0u?0xE9u:n>=5u?0x90u:static_cast<U8>((s.bridge-at-5u)>>((n-1u)*8u))):s.bytes[n];
            if(Read8(at+n)!=expected)return false;
        }
    }return true;
}
static bool CallbackParent(){return g_pendingStatus==1 && PendingParent() && PendingSignature(true) && g_chapterStatus==1 && ChapterSignature();}
static bool CallbackInstall() {
    g_callbackStatus=-1;g_callbackReason="parentUnavailable";if(!CallbackParent())return false;
    g_callbackReason="notCold";if(!IndexedCold() || g_pkg0)return false;
    g_callbackReason="signature";if(!CallbackSignature(false))return false;
    g_callbackReason="storageAllocation";
    g_callbackGroups=reinterpret_cast<CallbackGroup*>(VirtualAlloc(nullptr,sizeof(CallbackGroup)*kCallbackGroups,kMemCommitReserve,kPageReadWrite));
    if(!g_callbackGroups)return false;
    for(U32 i=0;i<kCallbackGroups;++i)for(U32 n=0;n<4u;++n)g_callbackGroups[i].before[n]=g_callbackGroups[i].after[n]=kCallbackGuard;
    g_callbackReason="trampolineAllocation";
    for(U32 i=0;i<3u;++i) {
        const CallbackSite& s=kCallbackSites[i];const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;g_callbackTramps[i]=t;
        for(U32 n=0;n<s.length;++n)*reinterpret_cast<U8*>(t+n)=s.bytes[n];
        *reinterpret_cast<U8*>(t+s.length)=0xE9u;Write32(t+s.length+1u,Address(s.address)-t-5u);
    }
    g_callbackNativeInsert=reinterpret_cast<CallbackNativeInsert>(g_callbackTramps[0]);
    g_callbackNativePoll=reinterpret_cast<CallbackNativePoll>(g_callbackTramps[2]);
    const UPtr pages[3]={Address(0x00556000u),Address(0x00858000u),Address(0x00859000u)};
    DWORD old[3]={},unused=0;U32 protectedPages=0;g_callbackReason="protectPreflight";
    for(;protectedPages<3u;++protectedPages)if(!VirtualProtect(reinterpret_cast<void*>(pages[protectedPages]),0x1000u,kPageExecuteReadWrite,&old[protectedPages]))break;
    if(protectedPages!=3u || !CallbackParent() || !IndexedCold() || g_pkg0 || !CallbackSignature(false)) {
        bool restored=true;for(U32 i=0;i<protectedPages;++i)if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))restored=false;
        if(!restored){g_callbackStatus=-2;g_callbackReason="precommitProtectionFailed";}return false;
    }
    for(const CallbackSite& s:kCallbackSites){const UPtr at=Address(s.address);*reinterpret_cast<volatile U8*>(at)=0xE9u;Write32(at+1u,s.bridge-at-5u);
        for(U32 n=5u;n<s.length;++n)*reinterpret_cast<volatile U8*>(at+n)=0x90u;}
    bool committed=CallbackSignature(true);
    for(U32 i=0;i<3u;++i){if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(pages[i]),0x1000u))committed=false;
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))committed=false;}
    if(committed){g_callbackStatus=1;g_callbackReason="armedColdLazyProcessStorage";return true;}
    bool restored=true;
    for(U32 i=0;i<3u;++i)if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,kPageExecuteReadWrite,&unused))restored=false;
    // On partial rollback protection failure avoid writing an unprotected page.
    if(restored)for(const CallbackSite& s:kCallbackSites)for(U32 n=0;n<s.length;++n)*reinterpret_cast<volatile U8*>(Address(s.address)+n)=s.bytes[n];
    for(U32 i=0;i<3u;++i){if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))restored=false;
        if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(pages[i]),0x1000u))restored=false;}
    if(!CallbackSignature(false))restored=false;
    g_callbackStatus=restored?-1:-2;g_callbackReason=restored?"commitFailedRestored":"rollbackFailed";return false;
}

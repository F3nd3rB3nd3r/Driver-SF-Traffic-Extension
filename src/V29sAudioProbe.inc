// Passive audio diagnostics on the R2 visual baseline. No audio capacity writes.
#if !defined(DSF_V29S_INDEXED_262144) || !defined(DSF_V29S_CAPACITY_TRACE) || defined(DSF_V29S_AUDIO_TRACE)
#error Audio probe requires the quiet R2 baseline without the legacy bank trace
#endif
struct AudioProbeRecord { volatile long ready; U32 tick,a,b,c,d,e,f; };
static AudioProbeRecord g_audioProbeRecords[6][512]={};
static volatile long g_audioProbeCounts[6]={},g_audioProbeBankOne=0;
static volatile long g_audioProbeEmitterCalls=0,g_audioProbeEmitterPeak=0;
extern "C" volatile long g_audioProbePostNormal=0;
static const U32 kAudioProbeBounds[6]={512u,64u,64u,64u,128u,64u};
static const char* const kAudioProbeKinds[6]={"bank","emitterRefused","mallocFailed","malignFailed","postZero","postEarlyZero"};
static U32 g_audioProbePrinted[6]={};
extern "C" UPtr g_audioProbeTramps[6]={};
static long g_audioProbeStatus=0;
static const char* g_audioProbeReason="notAttempted";
static U32 g_audioProbeInstalled=0;

static void AudioProbeRecordEvent(U32 kind,U32 a,U32 b,U32 c,U32 d,U32 e,U32 f=0u) {
    const long ordinal=InterlockedIncrement(&g_audioProbeCounts[kind]);
    if(ordinal<=0 || static_cast<U32>(ordinal)>kAudioProbeBounds[kind])return;
    AudioProbeRecord& r=g_audioProbeRecords[kind][ordinal-1];
    r.tick=GetTickCount()-g_capacityStart;r.a=a;r.b=b;r.c=c;r.d=d;r.e=e;r.f=f;
    InterlockedExchange(&r.ready,1); // Unique slot per producer; monitor reads after publication.
}
extern "C" __declspec(noinline) void __cdecl AudioProbeBankEntry(U32 bank,U32 result,U32 pool,UPtr cookie) {
    if(result==1u)InterlockedIncrement(&g_audioProbeBankOne);
    // The native callback is about to write cookie+14; do not call VQ or middleware.
    AudioProbeRecordEvent(0u,bank,result,pool,cookie,cookie?Read32(cookie+8u):0u,cookie?Read32(cookie+4u):0u);
}
extern "C" __declspec(noinline) void __cdecl AudioProbeEmitterEntry(UPtr owner) {
    const U32 active=Read32(owner+0xEA0u);
    InterlockedIncrement(&g_audioProbeEmitterCalls);CapacityMax(g_audioProbeEmitterPeak,active);
    if(active>=90u)AudioProbeRecordEvent(1u,owner,active,Read32(owner+0xE98u),Read32(owner+0xE9Cu),0u);
}
extern "C" __declspec(noinline) void __cdecl AudioProbeAllocEntry(U32 kind,U32 pool,U32 bytes,U32 caller,U32 alignment) {
    // A zero-size request is not evidence of pool exhaustion.
    if(bytes)AudioProbeRecordEvent(2u+kind,pool,bytes,caller,alignment,0u);
}
extern "C" __declspec(noinline) void __cdecl AudioProbePostEntry(U32 early,U32 event,U32 object,U32 caller) {
    AudioProbeRecordEvent(early?5u:4u,event,object,caller,0u,0u);
}
#include "V29sAudioProbeBridges.inc"

struct AudioProbeSite { U32 address,length; U8 bytes[8]; UPtr bridge; };
static const AudioProbeSite kAudioProbeSites[6]={
    {0x0084BE74u,7u,{0xC7,0x40,0x14,4,0,0,0},reinterpret_cast<UPtr>(&AudioProbeBankBridge)},
    {0x0084AA29u,7u,{0x83,0xBE,0xA0,0x0E,0,0,0x5A},reinterpret_cast<UPtr>(&AudioProbeEmitterBridge)},
    {0x00C99AA5u,5u,{0x5E,0x5B,0x8B,0xC7,0x5F},reinterpret_cast<UPtr>(&AudioProbeMallocBridge)},
    {0x00C99B04u,5u,{0x5E,0x5B,0x8B,0xC7,0x5F},reinterpret_cast<UPtr>(&AudioProbeMalignBridge)},
    {0x00C54E17u,8u,{0x8B,0xC6,0x5E,0x5F,0x5B,0x83,0xC4,0x10},reinterpret_cast<UPtr>(&AudioProbePostBridge)},
    {0x00C54DD3u,5u,{0x5F,0x5B,0x83,0xC4,0x10},reinterpret_cast<UPtr>(&AudioProbePostEarlyBridge)}
};
static bool AudioProbeInstall() {
    g_audioProbeStatus=-1;g_audioProbeReason="baselineUnavailable";
    if(g_capacityHooks!=3 || g_indexedStatus!=1 || !g_indexedStartupVerified)return false;
    g_audioProbeReason="signature";
    for(U32 i=0;i<6u;++i) {
        const AudioProbeSite& s=kAudioProbeSites[i];
        if(!Readable(Address(s.address),s.length))return false;
        for(U32 j=0;j<s.length;++j)
            if(*reinterpret_cast<volatile U8*>(Address(s.address)+j)!=s.bytes[j])return false;
    }
    // Allocate ALL trampolines before touching game code. They remain process-lifetime.
    g_audioProbeReason="trampolineAllocation";
    for(U32 i=0;i<6u;++i) {
        const AudioProbeSite& s=kAudioProbeSites[i];
        const UPtr t=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,s.length+5u,kMemCommitReserve,kPageExecuteReadWrite));
        if(!t)return false;
        g_audioProbeTramps[i]=t;
        for(U32 j=0;j<s.length;++j)*reinterpret_cast<U8*>(t+j)=s.bytes[j];
        *reinterpret_cast<U8*>(t+s.length)=0xE9;
        Write32(t+s.length+1u,Address(s.address)+s.length-(t+s.length+5u));
    }
    // Four distinct executable pages. Pre-protect all before writing any detour;
    // sharing a page between epilogues must not lose the original protection.
    const UPtr pages[4]={Address(0x0084B000u),Address(0x0084A000u),Address(0x00C99000u),Address(0x00C54000u)};
    DWORD old[4]={},unused=0;U32 writable=0;
    g_audioProbeReason="protectPreflight";
    for(;writable<4u;++writable)
        if(!VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,kPageExecuteReadWrite,&old[writable]))break;
    if(writable!=4u) {
        while(writable){--writable;VirtualProtect(reinterpret_cast<void*>(pages[writable]),0x1000u,old[writable],&unused);}
        return false;
    }
    for(U32 i=0;i<6u;++i) {
        const AudioProbeSite& s=kAudioProbeSites[i];const UPtr at=Address(s.address);
        *reinterpret_cast<volatile U8*>(at)=0xE9;
        Write32(at+1u,s.bridge-(at+5u));
        for(U32 j=5u;j<s.length;++j)*reinterpret_cast<volatile U8*>(at+j)=0x90;
    }
    bool bytesOk=true;
    for(U32 i=0;i<6u;++i) {
        const AudioProbeSite& s=kAudioProbeSites[i];const UPtr at=Address(s.address);
        if(*reinterpret_cast<volatile U8*>(at)!=0xE9 || Read32(at+1u)!=s.bridge-(at+5u))bytesOk=false;
        for(U32 j=5u;j<s.length;++j)if(*reinterpret_cast<volatile U8*>(at+j)!=0x90)bytesOk=false;
    }
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),nullptr,0u);
    bool protectedOk=true;
    for(U32 i=0;i<4u;++i)
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i]),0x1000u,old[i],&unused))protectedOk=false;
    if(bytesOk && flushed && protectedOk) {g_audioProbeInstalled=6u;g_audioProbeStatus=1;g_audioProbeReason="armed";return true;}
    // OS failure rollback. Never clear/free a trampoline if its jump might remain.
    bool rollbackOk=true;
    for(U32 page=0;page<4u;++page) {
        if(!VirtualProtect(reinterpret_cast<void*>(pages[page]),0x1000u,kPageExecuteReadWrite,&unused)) {rollbackOk=false;continue;}
        for(U32 i=0;i<6u;++i) {
            const AudioProbeSite& s=kAudioProbeSites[i];const UPtr at=Address(s.address);
            if((at&~0xFFFu)!=pages[page])continue;
            for(U32 j=0;j<s.length;++j)*reinterpret_cast<volatile U8*>(at+j)=s.bytes[j];
        }
        if(!VirtualProtect(reinterpret_cast<void*>(pages[page]),0x1000u,old[page],&unused))rollbackOk=false;
    }
    if(!FlushInstructionCache(GetCurrentProcess(),nullptr,0u))rollbackOk=false;
    g_audioProbeReason=rollbackOk?"commitFailedRestored":"osRollbackFailed";
    g_audioProbeStatus=rollbackOk?-1:-2;return false;
}
static void AudioProbeStartLine() {
    char line[640]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_AUDIO_START revision=1 status=",end);AppendNumber(p,g_audioProbeStatus,end);
    AppendText(p," reason=",end);AppendText(p,g_audioProbeReason,end);
    DiagFAppendKeyU(p," hooks=",g_audioProbeInstalled,end);
    AppendText(p," expectedHooks=6 actualBankResultStack=8 audioCapacityChanges=",end);
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
    DiagFAppendU(p,g_audioDefaultPatched?1u:0u,end);
#else
    DiagFAppendU(p,0u,end);
#endif
    AppendText(p," emitterLimit=90 poolLimit=20 bankQueueLimit=50 requests=150 graphicsSlots=160 indexedSlots=262144 detailBounds=512,64,64,64,128,64 monitorMs=200\n",end);
    *p=0;CapacityWrite(line);
}
struct AudioProbePoolPeak { U32 table,backing,attrs,total,blocks,blockSize,peakUsed,minFree,generation; bool active; };
static AudioProbePoolPeak g_audioProbePoolPeaks[128]={};
static U32 g_audioProbePoolPeakCount=0,g_audioProbeLastSummary=0,g_audioProbeLastPools=0;
static void AudioProbeDrain() {
    for(U32 kind=0;kind<6u;++kind) {
        U32& next=g_audioProbePrinted[kind];
        while(next<kAudioProbeBounds[kind]) {
            AudioProbeRecord& r=g_audioProbeRecords[kind][next];
            if(InterlockedCompareExchange(&r.ready,1,1)!=1)break;
            char line[512]={};char* p=line;const char* end=line+sizeof(line)-1;
            AppendText(p,"DSFV29s_AUDIO_DETAIL kind=",end);AppendText(p,kAudioProbeKinds[kind],end);
            DiagFAppendKeyU(p," ordinal=",next+1u,end);DiagFAppendKeyU(p," tick=",r.tick,end);
            const char* keys[6][6]={
                {" bank="," result="," pool="," cookie="," inputBytes="," inputBuffer="},
                {" owner="," activeBefore="," cursor="," recycled="," reserved="," reserved2="},
                {" pool="," bytes="," caller="," alignment="," reserved="," reserved2="},
                {" pool="," bytes="," caller="," alignment="," reserved="," reserved2="},
                {" event="," gameObject="," caller="," playingId="," reserved="," reserved2="},
                {" event="," gameObject="," caller="," playingId="," reserved="," reserved2="}};
            const U32 values[6]={r.a,r.b,r.c,r.d,r.e,r.f};
            for(U32 j=0;j<6u;++j)DiagFAppendKeyHex(p,keys[kind][j],values[j],end);
            AppendText(p,"\n",end);*p=0;CapacityWrite(line);++next;
        }
    }
}
static void AudioProbePoll() {
    if(g_audioProbeStatus!=1)return;
    AudioProbeDrain();
    const U32 now=GetTickCount()-g_capacityStart;
    const bool summary=now-g_audioProbeLastSummary>=1000u;
    const bool details=now-g_audioProbeLastPools>=5000u;
    if(summary)g_audioProbeLastSummary=now;if(details)g_audioProbeLastPools=now;
    const UPtr root=Readable(Address(kRootPtr),4u)?Read32(Address(kRootPtr)):0u;
    const UPtr owner=root && Readable(root+0x50u,4u)?Read32(root+0x50u):0u;
    const bool ownerOk=owner && Readable(owner,0x954Au);
    const UPtr globals=Address(0x01D72388u);
    const bool globalsOk=Readable(globals,12u);
    const U32 max=globalsOk?Read32(globals):0u,num=globalsOk?Read32(globals+4u):0u;
    const UPtr table=globalsOk?Read32(globals+8u):0u;
    const bool poolsOk=max>0u && max<=128u && num<=max && table && Readable(table,max*0x4Cu);
    if(poolsOk && num>g_audioProbePoolPeakCount)g_audioProbePoolPeakCount=num;
    if(summary) {
        char line[1400]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyU(p," bankCallbacks=",static_cast<U32>(g_audioProbeCounts[0]),end);
        DiagFAppendKeyU(p," bankResult1=",static_cast<U32>(g_audioProbeBankOne),end);
        DiagFAppendKeyU(p," emitterAttempts=",static_cast<U32>(g_audioProbeEmitterCalls),end);
        DiagFAppendKeyU(p," emitterPeakBefore=",static_cast<U32>(g_audioProbeEmitterPeak),end);
        const char* keys[5]={" emitterRefused="," mallocFailed="," malignFailed="," postZero="," postEarlyZero="};
        for(U32 i=1;i<6u;++i)DiagFAppendKeyU(p,keys[i-1u],static_cast<U32>(g_audioProbeCounts[i]),end);
        DiagFAppendKeyU(p," postNormalReturns=",static_cast<U32>(g_audioProbePostNormal),end);
        DiagFAppendKeyU(p," poolsReadable=",poolsOk?1u:0u,end);
        DiagFAppendKeyU(p," poolMax=",max,end);DiagFAppendKeyU(p," poolCount=",num,end);
        DiagFAppendKeyU(p," poolCountPeak=",g_audioProbePoolPeakCount,end);
        DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," ownerReadable=",ownerOk?1u:0u,end);
        if(ownerOk) {
            DiagFAppendKeyU(p," managerByte30=",*reinterpret_cast<volatile U8*>(owner+0x30u),end);
            DiagFAppendKeyU(p," soundInit=",*reinterpret_cast<volatile U8*>(owner+0x9548u),end);
            DiagFAppendKeyU(p," musicInit=",*reinterpret_cast<volatile U8*>(owner+0x9549u),end);
            DiagFAppendKeyU(p," emitterActive=",Read32(owner+0xEA0u),end);
            DiagFAppendKeyU(p," emitterRecycled=",Read32(owner+0xE9Cu),end);
            DiagFAppendKeyHex(p," emitterCursor=",Read32(owner+0xE98u),end);
            DiagFAppendKeyU(p," cleanupActive=",Read32(owner+0xA54u),end);
            DiagFAppendKeyU(p," cleanupRecycled=",Read32(owner+0xA50u),end);
            DiagFAppendKeyHex(p," cleanupCursor=",Read32(owner+0xA4Cu),end);
        }
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    if(details && ownerOk) {
        char line[1000]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_STATE",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyHex(p," filePoolId=",Read32(owner+0xA2Cu),end);
        DiagFAppendKeyHex(p," preparePoolId=",Read32(owner+0xA30u),end);
        DiagFAppendKeyHex(p," staticBankPoolId=",Read32(owner+0xA34u),end);
        if(Readable(Address(0x00FDD3B8u),12u)) {
            DiagFAppendKeyHex(p," defaultPoolId=",Read32(Address(0x00FDD3B8u)),end);
            DiagFAppendKeyHex(p," platformPoolId=",Read32(Address(0x00FDD3C0u)),end);
        }
        AppendText(p," raw50to90=",end);
        for(U32 off=0x50u;off<=0x90u;off+=4u) {if(off!=0x50u)AppendText(p,",",end);AppendHex32(p,Read32(owner+off),end);}
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
    if(!poolsOk)return;
    for(U32 id=0;id<max;++id) {
        const UPtr at=table+id*0x4Cu;AudioProbePoolPeak& peak=g_audioProbePoolPeaks[id];
        const U32 blocks=Read32(at),blockSize=Read32(at+4u),backing=Read32(at+8u),attrs=Read32(at+0x1Cu);
        const U32 free=Read32(at+0x18u),used=Read32(at+0x44u),total=Read32(at+0x48u);
        if(!blocks){peak.active=false;continue;}
        // Concurrent best-effort snapshot: reject changed identity/header, never lock middleware.
        if(Read32(at)!=blocks || Read32(at+4u)!=blockSize || Read32(at+8u)!=backing ||
           Read32(at+0x1Cu)!=attrs || Read32(at+0x48u)!=total || Read32(globals+8u)!=table)continue;
        if(!peak.active || peak.table!=table || peak.backing!=backing || peak.attrs!=attrs || peak.total!=total || peak.blocks!=blocks || peak.blockSize!=blockSize) {
            ++peak.generation;peak.table=table;peak.backing=backing;peak.attrs=attrs;peak.total=total;
            peak.blocks=blocks;peak.blockSize=blockSize;peak.peakUsed=used;peak.minFree=free;peak.active=true;
        }
        if(used>peak.peakUsed)peak.peakUsed=used;if(free<peak.minFree)peak.minFree=free;
        if(!details)continue;
        char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29s_AUDIO_POOL",end);DiagFAppendKeyU(p," tick=",now,end);
        DiagFAppendKeyU(p," id=",id,end);DiagFAppendKeyU(p," generation=",peak.generation,end);
        DiagFAppendKeyHex(p," table=",table,end);DiagFAppendKeyHex(p," backing=",backing,end);
        DiagFAppendKeyHex(p," attrs=",attrs,end);DiagFAppendKeyU(p," fixedBlocks=",attrs&8u?1u:0u,end);
        DiagFAppendKeyU(p," blocks=",blocks,end);DiagFAppendKeyU(p," blockBytes=",blockSize,end);
        DiagFAppendKeyU(p," freeBlocks=",free,end);DiagFAppendKeyU(p," sampledMinFree=",peak.minFree,end);
        DiagFAppendKeyU(p," mallocUsedBytes=",used,end);DiagFAppendKeyU(p," usableBytes=",total,end);
        DiagFAppendKeyU(p," sampledPeakMallocUsed=",peak.peakUsed,end);
        AppendText(p,"\n",end);*p=0;CapacityWrite(line);
    }
}

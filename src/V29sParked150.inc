#include "V29sParked150Native.inc"
#include "V29sParked150Layout.inc"
static long g_parkStatus=0;
static UPtr g_parkBuilder=0;
extern "C" UPtr g_parkBucketTrampoline=0;
static void ParkFatal(const char* reason,UPtr owner,U32 value) {
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PARKED150_FATAL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," value=",value,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29F3u);
}
static void* ParkAllocate(U32 n){return VirtualAlloc(nullptr,n,0x3000u,0x04u);}
static UPtr ParkNativeDescribe(UPtr owner,U32 id) {
    typedef UPtr (__thiscall* Fn)(void*,U32);
    return reinterpret_cast<Fn>(Address(0x0086C5B0u))(reinterpret_cast<void*>(owner),id);
}
static void ParkRunBuilder(UPtr owner) {
#if defined(DSF_V29S_WIDE_LIMITS)
    U32 stackPointer=0;__asm {mov stackPointer,esp} WideCheckStack(stackPointer,GenericParkFrame());
#endif
    typedef void (__thiscall* Fn)(void*);reinterpret_cast<Fn>(g_parkBuilder)(reinterpret_cast<void*>(owner));
}
static void ParkBuildEvent(UPtr owner,U32 count,U32 generation,U32 hash,const U32* ids) {
#if defined(DSF_V29S_WIDE_LIMITS)
    WideScratchLease text(count*7u+512u);char* line=static_cast<char*>(text.data);char* p=line;const char* end=line+count*7u+511u;
#else
    char line[2000]={};char* p=line;const char* end=line+sizeof(line)-1;
#endif
    AppendText(p,"DSFV29s_PARKED150_BUILD",end);DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," generation=",generation,end);
    DiagFAppendKeyU(p," count=",count,end);DiagFAppendKeyHex(p," rosterHash=",hash,end);
    AppendText(p," ids=",end);for(U32 i=0;i<count;++i){if(i)AppendText(p,",",end);DiagFAppendU(p,ids[i],end);}
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
#include "V29sParked150Core.inc"
extern "C" __declspec(naked) void ParkBucketBridge(){__asm {
    pushfd
    pushad
    mov ebp,esp
    sub esp,528
    and esp,0FFFFFFF0h
    fxsave [esp]
    cld
    push ebp
    call ParkBucketCheck
    add esp,4
    fxrstor [esp]
    mov esp,ebp
    popad
    popfd
    jmp dword ptr [g_parkBucketTrampoline]
}}
static void ParkImage(UPtr out,UPtr logical) {
    ParkFillImage(out,logical,Address(0x00FE7228u),Address(0x00C287F0u),Address(0x0040A690u),
        Address(0x005A6920u),reinterpret_cast<UPtr>(&ParkDescribe),reinterpret_cast<UPtr>(&ParkAppend),
        reinterpret_cast<UPtr>(&ParkBucketBridge),true);
}
static bool ParkCommittedMatches() {
    const UPtr b=Address(0x00809370u),q=Address(0x00808610u);
    if(Read8(b)!=0xE9u || Read32(b+1u)!=reinterpret_cast<UPtr>(&ParkRebuild)-b-5u ||
       Read8(q)!=0xE9u || Read32(q+1u)!=reinterpret_cast<UPtr>(&ParkPick)-q-5u)return false;
    for(U32 i=5u;i<sizeof(kParkBuildNative);++i)if(Read8(b+i)!=kParkBuildNative[i])return false;
    for(U32 i=5u;i<9u;++i)if(Read8(q+i)!=0x90u)return false;
    for(U32 i=9u;i<sizeof(kParkPickNative);++i)if(Read8(q+i)!=kParkPickNative[i])return false;
    U8 expected[0x400]={};ParkImage(reinterpret_cast<UPtr>(expected),g_parkBuilder);
    for(U32 i=0;i<sizeof(expected);++i)if(Read8(g_parkBuilder+i)!=expected[i])return false;
    return true;
}
static void ParkInstall() {
    const UPtr b=Address(0x00809370u),q=Address(0x00808610u),page=Address(0x00808000u);
    bool ok=g_notifyStatus==1 && g_sCloneHook==1 && !g_pkg0 && VerifyExecutableIdentity() &&
        Readable(b,sizeof(kParkBuildNative)) && Readable(q,sizeof(kParkPickNative)) && ParkOriginalMatches(b,q);
    if(ok) {
        g_parkBuilder=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,0x04u));ok=g_parkBuilder!=0u;
    }
    DWORD old=0,discarded=0;
    if(ok) {
        ParkImage(g_parkBuilder,g_parkBuilder);g_parkBucketTrampoline=g_parkBuilder+0x300u;
        ok=VirtualProtect(reinterpret_cast<void*>(g_parkBuilder),0x1000u,0x20u,&discarded)!=0 &&
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(g_parkBuilder),0x1000u)!=0;
    }
    if(ok)ok=VirtualProtect(reinterpret_cast<void*>(page),0x2000u,kPageExecuteReadWrite,&old)!=0;
    if(ok) {
        if(g_pkg0 || !ParkOriginalMatches(b,q)) {
            if(!VirtualProtect(reinterpret_cast<void*>(page),0x2000u,old,&discarded))ParkFatal("precommitProtection",b,0);
            ok=false;
        } else {
            ParkJump(b,reinterpret_cast<UPtr>(&ParkRebuild),5u);ParkJump(q,reinterpret_cast<UPtr>(&ParkPick),9u);
            const bool exact=ParkCommittedMatches();
            const BOOL flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x2000u);
            const BOOL protect=VirtualProtect(reinterpret_cast<void*>(page),0x2000u,old,&discarded);
            ok=exact && flush && protect;
            if(!ok) {
                DWORD retry=0;if(!VirtualProtect(reinterpret_cast<void*>(page),0x2000u,kPageExecuteReadWrite,&retry))ParkFatal("rollbackUnlock",b,0);
                for(U32 i=0;i<5u;++i)*reinterpret_cast<U8*>(b+i)=kParkBuildNative[i];
                for(U32 i=0;i<9u;++i)*reinterpret_cast<U8*>(q+i)=kParkPickNative[i];
                const bool restored=ParkOriginalMatches(b,q);
                const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x2000u);
                const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x2000u,old,&discarded);
                if(!restored || !flushed || !protectedAgain)ParkFatal("rollbackIncomplete",b,0);
            }
        }
    }
    if(!ok){g_parkStatus=-1;CapacityWrite("DSFV29s_PARKED150_START revision=1 status=-1\n");ParkFatal("startupPatchRefused",b,0);return;}
    g_parkStatus=1;
#if defined(DSF_V29S_GENERIC_LIMITS)
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PARKED150_START revision=1 status=1 hooks=2 nativeCapacity=50",end);
    DiagFAppendKeyU(p," capacity=",g_limits.requests,end);AppendText(p," categories=25",end);DiagFAppendKeyU(p," perCategory=",g_limits.requests,end);
    DiagFAppendKeyU(p," frameBytes=",GenericParkFrame(),end);
    AppendText(p," inlineMirror=50 privatePerOwner=1 processLifetime=1 fixedOwnerLimit=0 parkedPopulationChanged=0 refreshTimingChanged=0 weightsChanged=0 requests=150 graphics=160\n",end);*p=0;CapacityWrite(line);
#else
    CapacityWrite("DSFV29s_PARKED150_START revision=1 status=1 hooks=2 nativeCapacity=50 capacity=150 categories=25 perCategory=150 frameBytes=15732 inlineMirror=50 privatePerOwner=1 processLifetime=1 fixedOwnerLimit=0 parkedPopulationChanged=0 refreshTimingChanged=0 weightsChanged=0 requests=150 graphics=160\n");
#endif
}
static void ParkPoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(now-last<1000u)return;last=now;
    char line[900]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_PARKED150_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    AppendText(p," status=",end);AppendNumber(p,g_parkStatus,end);
    const char* keys[]={" owners="," builds="," peakCandidates="," peakBucket="," pickCalls="," selected="," empty="," invalid="," guardBad="," concurrentSkips="," peakUniqueSelections="," extraSelections="};
    const long values[]={g_parkOwners,g_parkBuilds,g_parkPeak,g_parkBucketPeak,g_parkPicks,g_parkSelected,g_parkEmpty,g_parkInvalid,g_parkGuardBad,g_parkConcurrent,g_parkUniquePeak,g_parkExtraSelections};
    for(U32 i=0;i<12u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);
    AppendText(p," lastSelected=",end);AppendNumber(p,g_parkLastId,end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

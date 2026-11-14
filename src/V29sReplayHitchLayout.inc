struct RHPatch {U32 at,len,size;const U8* bytes;UPtr target;};
static const RHPatch kRHPatches[]={
    {0x7379D0u,11u,sizeof(kRHGate20Native),kRHGate20Native,reinterpret_cast<UPtr>(&RHBridge20)},
    {0x737A10u,11u,sizeof(kRHGate24Native),kRHGate24Native,reinterpret_cast<UPtr>(&RHBridge24)},
    {0x738F20u,5u,sizeof(kRHStepNative),kRHStepNative,reinterpret_cast<UPtr>(&RHBridgeStep)},
    {0x739F60u,10u,sizeof(kRHSeekNative),kRHSeekNative,reinterpret_cast<UPtr>(&RHBridgeSeek)},
    {0x746000u,6u,sizeof(kRHStreamNative),kRHStreamNative,reinterpret_cast<UPtr>(&RHBridgeStream)}
};
static bool RHMatches(bool patched) {
    for(U32 i=0;i<5u;++i) {
        const RHPatch& s=kRHPatches[i];const UPtr at=Address(s.at);if(!RegMapReadableSpan(at,s.size))return false;
        for(U32 j=0;j<s.size;++j){U8 b=s.bytes[j];if(patched && j<s.len)b=j==0u?0xE9u:(j<5u?static_cast<U8>((s.target-at-5u)>>((j-1u)*8u)):0x90u);if(Read8(at+j)!=b)return false;}
    }return true;
}
static void RHPatchBytes(bool on) {
    for(U32 i=0;i<5u;++i){const RHPatch& s=kRHPatches[i];const UPtr at=Address(s.at);
        for(U32 j=0;j<s.len;++j){U8 b=s.bytes[j];if(on)b=j==0u?0xE9u:(j<5u?static_cast<U8>((s.target-at-5u)>>((j-1u)*8u)):0x90u);*reinterpret_cast<volatile U8*>(at+j)=b;}}
}
static void RHInstall() {
    const char* reason="preflight";bool okay=g_pauseStatus==1 && g_sRegMapHook==1 && !g_pkg0 && VerifyExecutableIdentity() && RHMatches(false);
    if(okay){reason="performanceClock";okay=QueryPerformanceFrequency(&g_rhFrequency)!=0 && g_rhFrequency>0;
        g_rhWaitMinTicks=g_rhFrequency>>10;if(g_rhWaitMinTicks<1)g_rhWaitMinTicks=1;}
    UPtr mem=0;DWORD unused=0;
    if(okay){reason="trampolineAllocation";mem=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));okay=mem!=0;}
    if(okay) {
        for(U32 i=0;i<5u;++i){const RHPatch& s=kRHPatches[i];const UPtr out=mem+i*0x100u;g_rhTargets[i]=out;
            for(U32 j=0;j<s.len;++j)*reinterpret_cast<U8*>(out+j)=s.bytes[j];
            *reinterpret_cast<U8*>(out+s.len)=0xE9u;Write32(out+s.len+1u,Address(s.at)+s.len-out-s.len-5u);}
        reason="trampolineProtection";okay=VirtualProtect(reinterpret_cast<void*>(mem),0x1000u,0x20u,&unused)!=0 && FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(mem),0x1000u)!=0;
    }
    const U32 pages[]={0x737000u,0x738000u,0x739000u,0x746000u};DWORD protections[4]={};U32 unlocked=0u;
    if(okay){reason="pageUnlock";for(;unlocked<4u;++unlocked)if(!VirtualProtect(reinterpret_cast<void*>(Address(pages[unlocked])),0x1000u,kPageExecuteReadWrite,&protections[unlocked]))break;okay=unlocked==4u;}
    bool changed=false;
    if(okay){reason="precommitRecheck";okay=!g_pkg0 && RHMatches(false);}
    if(okay){RHPatchBytes(true);changed=true;reason="commitVerify";okay=RHMatches(true);}
    if(okay){reason="instructionFlush";for(U32 i=0;i<4u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(Address(pages[i])),0x1000u))okay=false;}
    if(!okay && changed){RHPatchBytes(false);if(!RHMatches(false))PauseFatal("hitchRollbackBytes");for(U32 i=0;i<4u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(Address(pages[i])),0x1000u))PauseFatal("hitchRollbackFlush");}
    bool protectedAgain=true;
    for(U32 i=0;i<unlocked;++i)if(!VirtualProtect(reinterpret_cast<void*>(Address(pages[i])),0x1000u,protections[i],&unused))protectedAgain=false;
    // A failed protection restore is not allowed to masquerade as a clean probe.
    if(!protectedAgain)PauseFatal("hitchProtectionRestore");
    g_rhStatus=okay?1:-1;
    char line[600]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_HITCH_START revision=1 status=",end);AppendNumber(p,g_rhStatus,end);
    AppendText(p," reason=",end);AppendText(p,okay?"okay":reason,end);
    DiagFAppendKeyU(p," qpcFrequencyLo=",static_cast<U32>(g_rhFrequency),end);DiagFAppendKeyU(p," qpcFrequencyHi=",static_cast<U32>(g_rhFrequency>>32),end);
    AppendText(p," hooks=5 ring=512 threads=16 depth=24 holdMs=50 nativeGates=1 duplicateCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n",end);*p=0;CapacityWrite(line);
}

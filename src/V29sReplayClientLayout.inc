struct SCPatch {U32 at,len,size;const U8* bytes;UPtr target;};
static const SCPatch kSCPatches[]={
    {0x7549D0u,5u,sizeof(kSCSamplesNative),kSCSamplesNative,reinterpret_cast<UPtr>(&SCBridgeSamples)},
    {0x755DF0u,7u,sizeof(kSCVehicleNative),kSCVehicleNative,reinterpret_cast<UPtr>(&SCBridgeVehicle)},
    {0x7523B0u,5u,sizeof(kSCPlayerNative),kSCPlayerNative,reinterpret_cast<UPtr>(&SCBridgePlayer)}
};
static bool SCMatches(bool patched) {
    for(U32 i=0;i<3u;++i){const SCPatch& s=kSCPatches[i];const UPtr at=SCAddress(s.at);if(!RegMapReadableSpan(at,s.size))return false;
        for(U32 j=0;j<s.size;++j){U8 b=s.bytes[j];if(patched && j<s.len)b=j==0u?0xE9u:(j<5u?static_cast<U8>((s.target-at-5u)>>((j-1u)*8u)):0x90u);if(Read8(at+j)!=b)return false;}}
    return true;
}
static bool SCDependencies() {
    for(U32 i=0;i<4u;++i){const SCClient& c=kSCClients[i];const UPtr slot=SCAddress(c.vtable)+0x20u;
        if(!Readable(slot,4u) || Read32(slot)!=SCAddress(kSCPatches[c.method].at))return false;}
    return RegMapEqual(SCAddress(0x7379D0u),kSCGateNative,sizeof(kSCGateNative));
}
static void SCPatchBytes(bool on) {
    for(U32 i=0;i<3u;++i){const SCPatch& s=kSCPatches[i];const UPtr at=SCAddress(s.at);
        for(U32 j=0;j<s.len;++j){U8 b=s.bytes[j];if(on)b=j==0u?0xE9u:(j<5u?static_cast<U8>((s.target-at-5u)>>((j-1u)*8u)):0x90u);*reinterpret_cast<volatile U8*>(at+j)=b;}}
}
static void SCInstall() {
    const char* reason="preflight";bool okay=g_pauseStatus==1 && g_sRegMapHook==1 && !g_pkg0 && VerifyExecutableIdentity() && SCMatches(false) && SCDependencies();
    if(okay){reason="performanceClock";okay=QueryPerformanceFrequency(&g_scFrequency)!=0 && g_scFrequency>0;}
    UPtr mem=0;DWORD unused=0;
    if(okay){reason="trampolineAllocation";mem=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));okay=mem!=0;}
    if(okay) {
        for(U32 i=0;i<3u;++i){const SCPatch& s=kSCPatches[i];const UPtr out=mem+i*0x100u;g_scTargets[i]=out;
            for(U32 j=0;j<s.len;++j)*reinterpret_cast<U8*>(out+j)=s.bytes[j];
            *reinterpret_cast<U8*>(out+s.len)=0xE9u;Write32(out+s.len+1u,SCAddress(s.at)+s.len-out-s.len-5u);}
        reason="trampolineProtection";okay=VirtualProtect(reinterpret_cast<void*>(mem),0x1000u,0x20u,&unused)!=0 && FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(mem),0x1000u)!=0;
    }
    const U32 pages[]={0x754000u,0x755000u,0x752000u};DWORD protections[3]={};U32 unlocked=0u;
    if(okay){reason="pageUnlock";for(;unlocked<3u;++unlocked)if(!VirtualProtect(reinterpret_cast<void*>(SCAddress(pages[unlocked])),0x1000u,kPageExecuteReadWrite,&protections[unlocked]))break;okay=unlocked==3u;}
    bool changed=false;
    if(okay){reason="precommitRecheck";okay=!g_pkg0 && SCMatches(false) && SCDependencies();}
    if(okay){g_scGateReturn=SCAddress(0x7379F1u);SCPatchBytes(true);changed=true;reason="commitVerify";okay=SCMatches(true) && SCDependencies();}
    if(okay){reason="instructionFlush";for(U32 i=0;i<3u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(SCAddress(pages[i])),0x1000u))okay=false;}
    if(!okay && changed){SCPatchBytes(false);if(!SCMatches(false))PauseFatal("clientRollbackBytes");for(U32 i=0;i<3u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(SCAddress(pages[i])),0x1000u))PauseFatal("clientRollbackFlush");}
    bool protectedAgain=true;
    for(U32 i=0;i<unlocked;++i)if(!VirtualProtect(reinterpret_cast<void*>(SCAddress(pages[i])),0x1000u,protections[i],&unused))protectedAgain=false;
    if(!protectedAgain)PauseFatal("clientProtectionRestore");
    g_scStatus=okay?1:-1;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_CLIENT_START revision=1 status=",end);AppendNumber(p,g_scStatus,end);
    AppendText(p," reason=",end);AppendText(p,okay?"okay":reason,end);
    DiagFAppendKeyHex(p," gateCaller=",g_scGateReturn,end);
    DiagFAppendKeyU(p," qpcFrequencyLo=",static_cast<U32>(g_scFrequency),end);DiagFAppendKeyU(p," qpcFrequencyHi=",static_cast<U32>(g_scFrequency>>32),end);
    AppendText(p," hooks=3 clients=4 broadHooks=0 ring=512 threads=8 boundaryOnly=1 clearFastPath=1 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n",end);*p=0;CapacityWrite(line);
}

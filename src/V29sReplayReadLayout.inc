struct RFPatch {U32 at,len,size;const U8* stock;UPtr target;};
static const RFPatch kRFPatches[]={
    {0x73FAB0u,8u,sizeof(kRFCtorNative),kRFCtorNative,reinterpret_cast<UPtr>(&RFConstruct)},
    {0x73FB40u,9u,sizeof(kRFDtorNative),kRFDtorNative,reinterpret_cast<UPtr>(&RFDestruct)},
    {0x74694Au,8u,8u,kRFReaderNative+0xAu,reinterpret_cast<UPtr>(&RFBridgePoll)},
    {0x74698Cu,6u,6u,kRFReaderNative+0x4Cu,reinterpret_cast<UPtr>(&RFBridgeCommit)},
    {0x746AD9u,8u,8u,kRFReaderNative+0x199u,reinterpret_cast<UPtr>(&RFBridgeWrap)},
    {0x746B62u,7u,7u,kRFReaderNative+0x222u,reinterpret_cast<UPtr>(&RFBridgeSpace)},
    {0x746BD7u,6u,6u,kRFReaderNative+0x297u,reinterpret_cast<UPtr>(&RFBridgeIssue)},
    {0x746B2Bu,13u,13u,kRFReaderNative+0x1EBu,reinterpret_cast<UPtr>(&RFBridgeEnd)}
};
static U8 RFPatchedByte(const RFPatch& s,UPtr at,U32 i){return i==0u?0xE9u:(i<5u?static_cast<U8>((s.target-at-5u)>>((i-1u)*8u)):0x90u);}
static bool RFMatches(bool on) {
    for(U32 i=0;i<8u;++i){const RFPatch& s=kRFPatches[i];const UPtr at=RFAddress(s.at);if(!RegMapReadableSpan(at,s.size))return false;
        for(U32 j=0;j<s.size;++j)if(Read8(at+j)!=(on && j<s.len?RFPatchedByte(s,at,j):s.stock[j]))return false;}
    if(!RegMapReadableSpan(RFAddress(0x746940u),sizeof(kRFReaderNative)))return false;
    for(U32 i=0;i<sizeof(kRFReaderNative);++i){U8 b=kRFReaderNative[i];if(on)for(U32 j=2;j<8u;++j){const RFPatch& s=kRFPatches[j];if(0x746940u+i>=s.at && 0x746940u+i<s.at+s.len)b=RFPatchedByte(s,RFAddress(s.at),0x746940u+i-s.at);}if(Read8(RFAddress(0x746940u)+i)!=b)return false;}
    return true;
}
static bool RFDependencies() {
    const U32 methods[]={0x73FB40u,0x73FB20u,0x8203B0u,0x73FB30u,0x73F910u,0x73F930u,0x73F950u,0x73FA00u,0x73F9F0u,0x73F970u};
    if(!RegMapReadableSpan(RFAddress(0xE2A088u),40u))return false;
    for(U32 i=0;i<10u;++i)if(Read32(RFAddress(0xE2A088u)+i*4u)!=RFAddress(methods[i]))return false;
    return RegMapEqual(RFAddress(0x746CA0u),kRFDispatchNative,sizeof(kRFDispatchNative));
}
static void RFPatchBytes(bool on) {
    for(U32 i=0;i<8u;++i){const RFPatch& s=kRFPatches[i];const UPtr at=RFAddress(s.at);for(U32 j=0;j<s.len;++j)*reinterpret_cast<volatile U8*>(at+j)=on?RFPatchedByte(s,at,j):s.stock[j];}
}
static void RFInstall(bool clientOkay) {
    bool okay=clientOkay && g_pauseStatus==1 && !g_pkg0 && VerifyExecutableIdentity() && RFMatches(false) && RFDependencies();
    const char* reason="preflight";UPtr tramp=0;DWORD unused=0;
    if(okay){reason="trampolineAllocation";tramp=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x1000u,0x3000u,4u));okay=tramp!=0;}
    if(okay) {
        for(U32 i=0;i<2u;++i){const RFPatch& s=kRFPatches[i];const UPtr dest=tramp+i*0x100u;for(U32 j=0;j<s.len;++j)*reinterpret_cast<U8*>(dest+j)=s.stock[j];*reinterpret_cast<U8*>(dest+s.len)=0xE9u;Write32(dest+s.len+1u,RFAddress(s.at)+s.len-(dest+s.len)-5u);}
        g_rfCtorTarget=tramp;g_rfDtorTarget=tramp+0x100u;
        const U32 resumes[]={0x746952u,0x746992u,0x746AE1u,0x746B69u,0x746BDDu,0x746B38u};
        const U32 stops[]={0x746BE5u,0x746BE3u,0x746BE3u,0x746BE3u};
        for(U32 i=0;i<6u;++i)g_rfContinue[i]=RFAddress(resumes[i]);for(U32 i=0;i<4u;++i)g_rfStop[i]=RFAddress(stops[i]);
        reason="trampolineProtection";okay=VirtualProtect(reinterpret_cast<void*>(tramp),0x1000u,0x20u,&unused)!=0 && FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(tramp),0x1000u)!=0;
    }
    const U32 pages[]={0x73F000u,0x746000u};DWORD protections[2]={};U32 unlocked=0;
    if(okay){reason="pageUnlock";for(;unlocked<2u;++unlocked)if(!VirtualProtect(reinterpret_cast<void*>(RFAddress(pages[unlocked])),0x1000u,kPageExecuteReadWrite,&protections[unlocked]))break;okay=unlocked==2u;}
    bool changed=false;
    if(okay){reason="precommitRecheck";okay=RFMatches(false) && RFDependencies() && !g_pkg0;}
    if(okay){RFPatchBytes(true);changed=true;reason="commitVerify";okay=RFMatches(true) && RFDependencies();}
    if(okay){reason="instructionFlush";for(U32 i=0;i<2u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(RFAddress(pages[i])),0x1000u))okay=false;}
    if(!okay && changed){RFPatchBytes(false);if(!RFMatches(false))PauseFatal("read1mRollbackBytes");for(U32 i=0;i<2u;++i)if(!FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(RFAddress(pages[i])),0x1000u))PauseFatal("read1mRollbackFlush");}
    bool protectedAgain=true;for(U32 i=0;i<unlocked;++i)if(!VirtualProtect(reinterpret_cast<void*>(RFAddress(pages[i])),0x1000u,protections[i],&unused))protectedAgain=false;
    if(!protectedAgain)PauseFatal("read1mProtectionRestore");
    g_rfStatus=okay?1:-1;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REPLAY_READ_START revision=1 status=",end);AppendNumber(p,g_rfStatus,end);AppendText(p," reason=",end);AppendText(p,okay?"okay":reason,end);
    AppendText(p," hooks=8",end);DiagFAppendKeyU(p," readerBytes=",kRFBytes,end);
    AppendText(p," blockBytes=32768 readerOnly=1 writerChanged=0 separateBacking=1 guardBytes=65536 processLifetime=1 reusable=1 fixedOwnerLimit=0 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n",end);*p=0;CapacityWrite(line);
}

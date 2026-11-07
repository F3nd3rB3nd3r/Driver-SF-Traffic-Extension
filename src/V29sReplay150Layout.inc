struct ReplayPatch {U32 at,length,nativeLength;const U8* native;UPtr target;U8 opcode;};
static const ReplayPatch kReplayPatches[]={
    {0x0074B7A0u,6u,sizeof(kReplayCaptureNative),kReplayCaptureNative,reinterpret_cast<UPtr>(&ReplayCapture),0xE9u},
    {0x0074BA20u,6u,sizeof(kReplayRestoreNative),kReplayRestoreNative,reinterpret_cast<UPtr>(&ReplayRestore),0xE9u},
    {0x0074BB80u,6u,sizeof(kReplayImportNative),kReplayImportNative,reinterpret_cast<UPtr>(&ReplayImport),0xE9u},
    {0x0074C57Du,5u,5u,kReplaySaveTrafficNative,reinterpret_cast<UPtr>(&ReplaySaveTraffic),0xE8u},
    {0x0074C59Fu,5u,5u,kReplaySaveRetainedNative,reinterpret_cast<UPtr>(&ReplaySaveRetained),0xE8u},
    {0x0074DF09u,5u,5u,kReplayLoadTrafficNative,reinterpret_cast<UPtr>(&ReplayLoadTraffic),0xE8u},
    {0x0074DF4Au,5u,5u,kReplayLoadRetainedNative,reinterpret_cast<UPtr>(&ReplayLoadRetained),0xE8u}
};
static bool ReplayBytesMatch(bool patched) {
    for(U32 i=0;i<7u;++i) {
        const ReplayPatch& s=kReplayPatches[i];const UPtr at=Address(s.at);
        if(!Readable(at,s.nativeLength))return false;
        const U32 rel=s.target-at-5u;
        for(U32 j=0;j<s.nativeLength;++j) {
            U8 v=s.native[j];if(patched && j<s.length)v=j==0u?s.opcode:(j<5u?static_cast<U8>(rel>>((j-1u)*8u)):0x90u);
            if(Read8(at+j)!=v)return false;
        }
    }return true;
}
static void ReplayApply(bool patched) {
    for(U32 i=0;i<7u;++i){const ReplayPatch& s=kReplayPatches[i];const UPtr at=Address(s.at);
        if(patched){*reinterpret_cast<U8*>(at)=s.opcode;Write32(at+1u,s.target-at-5u);for(U32 j=5u;j<s.length;++j)*reinterpret_cast<U8*>(at+j)=0x90u;}
        else for(U32 j=0;j<s.length;++j)*reinterpret_cast<U8*>(at+j)=s.native[j];}
}

static bool PauseEqual(UPtr at,const U8* expected,U32 n){if(!Readable(at,n))return false;for(U32 i=0;i<n;++i)if(Read8(at+i)!=expected[i])return false;return true;}
static bool PauseUpdateMatches(UPtr at,UPtr target,bool patched) {
    if(!Readable(at,sizeof(kPauseUpdateNative)))return false;
    const U32 rel=target-at-5u;
    for(U32 i=0;i<sizeof(kPauseUpdateNative);++i) {
        U8 v=kPauseUpdateNative[i];if(patched && i<6u)v=i==0u?0xE9u:(i<5u?static_cast<U8>(rel>>((i-1u)*8u)):0x90u);
        if(Read8(at+i)!=v)return false;
    }return true;
}
static void PauseFillTrampoline(UPtr output,UPtr logical,UPtr resume) {
    for(U32 i=0;i<6u;++i)*reinterpret_cast<U8*>(output+i)=kPauseUpdateNative[i];
    *reinterpret_cast<U8*>(output+6u)=0xE9u;Write32(output+7u,resume-logical-11u);
}
static void PausePatch(UPtr at,UPtr target,bool patched) {
    if(patched){*reinterpret_cast<U8*>(at)=0xE9u;Write32(at+1u,target-at-5u);*reinterpret_cast<U8*>(at+5u)=0x90u;}
    else for(U32 i=0;i<6u;++i)*reinterpret_cast<U8*>(at+i)=kPauseUpdateNative[i];
}

struct RegMapPatch {U32 at,len,size;const U8* bytes;UPtr target,trampoline;DWORD protection;bool installed;};
static bool RegMapEqual(UPtr at,const U8* bytes,U32 n){if(!Readable(at,n))return false;for(U32 i=0;i<n;++i)if(Read8(at+i)!=bytes[i])return false;return true;}
static bool RegMapMatches(const RegMapPatch& s,bool patched){
    const UPtr at=RegMapAddress(s.at);if(!Readable(at,s.size))return false;const U32 rel=s.target-at-5u;
    for(U32 i=0;i<s.size;++i){U8 b=s.bytes[i];if(patched && i<s.len)b=i==0u?0xE9u:(i<5u?static_cast<U8>(rel>>((i-1u)*8u)):0x90u);if(Read8(at+i)!=b)return false;}return true;
}
static void RegMapPatchBytes(const RegMapPatch& s,bool on){
    const UPtr at=RegMapAddress(s.at);
    for(U32 i=0;i<s.len;++i){U8 b=s.bytes[i];if(on)b=i==0u?0xE9u:(i<5u?static_cast<U8>((s.target-at-5u)>>((i-1u)*8u)):0x90u);*reinterpret_cast<volatile U8*>(at+i)=b;}
}
static void RegMapFillTrampoline(const RegMapPatch& s){
    for(U32 i=0;i<s.len;++i)*reinterpret_cast<U8*>(s.trampoline+i)=s.bytes[i];
    *reinterpret_cast<U8*>(s.trampoline+s.len)=0xE9u;Write32(s.trampoline+s.len+1u,RegMapAddress(s.at)+s.len-s.trampoline-s.len-5u);
}

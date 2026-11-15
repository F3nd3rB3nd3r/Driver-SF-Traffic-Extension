// Relocated native builder: expand category arrays; preserve clone header/layout.
struct ParkOperand {U32 offset,stock,replacement;};
static const ParkOperand kParkOperands[]={
    {1u,0x1664u,0x3D74u},
    {0x6Eu,0x350u,0x4E0u},
    {0x75u,0xCCu,0x25Cu},
    {0xC1u,0xCCu,0x25Cu},
    {0xC8u,0x350u,0x4E0u},
    {0xE1u,0xC8u,0x258u},
    {0x112u,0xC8u,0x258u},
    {0x151u,0xCCu,0x25Cu},
    {0x160u,0x1664u,0x3D74u}
};
static const U32 kParkCalls[][2]={
    {5u,0xC287F0u},{0x4Cu,0x40A690u},{0x61u,0x5A6920u},
    {0x87u,0x40A690u},{0xB6u,0x86C5B0u},{0x144u,0x809F30u}
};
static const U32 kParkRoots[]={0x28u,0x7Fu};
static void ParkJump(UPtr at,UPtr target,U32 n) {
    *reinterpret_cast<U8*>(at)=0xE9u;Write32(at+1u,target-at-5u);
    for(U32 i=5u;i<n;++i)*reinterpret_cast<U8*>(at+i)=0x90u;
}
static bool ParkOriginalMatches(UPtr builder,UPtr picker) {
    for(U32 i=0;i<sizeof(kParkBuildNative);++i)if(Read8(builder+i)!=kParkBuildNative[i])return false;
    for(U32 i=0;i<sizeof(kParkPickNative);++i)if(Read8(picker+i)!=kParkPickNative[i])return false;
    return true;
}
// This builder is used by the production installer and the native fixture.
static void ParkFillImage(UPtr output,UPtr logical,UPtr root,UPtr probe,UPtr factory,
    UPtr clone,UPtr describe,UPtr append,UPtr guard,bool expanded) {
    for(U32 i=0;i<0x400u;++i)*reinterpret_cast<U8*>(output+i)=0u;
    for(U32 i=0;i<sizeof(kParkBuildNative);++i)*reinterpret_cast<U8*>(output+i)=kParkBuildNative[i];
    if(expanded)for(U32 i=0;i<sizeof(kParkOperands)/sizeof(kParkOperands[0]);++i){
        U32 value=kParkOperands[i].replacement;
#if defined(DSF_V29S_GENERIC_LIMITS)
        value=(i==0u || i==8u)?GenericParkFrame():(i==1u || i==4u)?0x288u+GenericRequestStorage()*4u:
            (i==5u || i==6u)?GenericRequestStorage()*4u:GenericParkStride();
#endif
        Write32(output+kParkOperands[i].offset,value);
    }
    const UPtr targets[]={probe,factory,clone,factory,describe,append};
    for(U32 i=0;i<6u;++i)Write32(output+kParkCalls[i][0]+1u,targets[i]-(logical+kParkCalls[i][0]+5u));
    for(U32 i=0;i<2u;++i)Write32(output+kParkRoots[i],root);
    if(guard) {
        // Replay complete LEA/CMP instructions before returning to the native JE.
        for(U32 i=0;i<5u;++i)*reinterpret_cast<U8*>(output+0x300u+i)=kParkBuildNative[0xD3u+i];
        *reinterpret_cast<U8*>(output+0x305u)=0xE9u;Write32(output+0x306u,(logical+0xD8u)-(logical+0x30Au));
        *reinterpret_cast<U8*>(output+0xD3u)=0xE9u;Write32(output+0xD4u,guard-(logical+0xD8u));
    }
}

// Relative DWORD operand offsets within the original 00754C70 callback.
// Same-length patches: all internal branch destinations remain unchanged.
struct NotifyOperand {U32 offset,stock,replacement;};
static const NotifyOperand kNotifyOperands[5]={
    {7u,0xD0u,0x260u},       // local frame:8 header bytes +150 DWORD IDs
    {55u,0xE4u,0x274u},     // count argument after three saved registers
    {78u,0xE0u,0x270u},     // source argument after three saved registers
    {97u,0x004D3B4Bu,0u},   // CALL memcpy -> owner-specific source adapter
    {150u,0xD0u,0x260u}     // matching stack restoration
};
static U32 NotifyOperandValue(U32 index,UPtr callback,UPtr copy) {
#if defined(DSF_V29S_GENERIC_LIMITS)
    return index==3u?copy-(callback+101u):GenericNotifyFrame()+(index==1u?20u:index==2u?16u:0u);
#else
    return index==3u?copy-(callback+101u):kNotifyOperands[index].replacement;
#endif
}
static void NotifyApply(UPtr callback,UPtr copy,bool patched) {
    for(U32 i=0;i<5u;++i)Write32(callback+kNotifyOperands[i].offset,
        patched?NotifyOperandValue(i,callback,copy):kNotifyOperands[i].stock);
}
static bool NotifyBytesMatch(UPtr callback,UPtr copy,bool patched) {
    for(U32 at=0;at<sizeof(kNotifyNative);++at) {
        U8 expected=kNotifyNative[at];
        if(patched)for(U32 i=0;i<5u;++i)if(at>=kNotifyOperands[i].offset && at<kNotifyOperands[i].offset+4u)
            expected=static_cast<U8>(NotifyOperandValue(i,callback,copy)>>(8u*(at-kNotifyOperands[i].offset)));
        if(Read8(callback+at)!=expected)return false;
    }
    return true;
}

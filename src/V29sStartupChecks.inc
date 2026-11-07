// Shared by production and the isolated Win32 startup-check regression test.
// Image identity is stable across our patches; stock handler bytes are not.
static bool VerifyExecutableIdentity() {
    if (g_base != kPreferredBase || !Readable(g_base, 0x1000u)) return false;
    if (Read8(g_base) != 'M' || Read8(g_base+1u) != 'Z') return false;
    const U32 peOffset = Read32(g_base + 0x3cu);
    if (peOffset < 0x40u || peOffset > 0x800u) return false;
    const UPtr nt = g_base + peOffset;
    if (Read32(nt) != 0x00004550u ||
        Read16(nt + 4u) != 0x014cu ||
        Read32(nt + 8u) != kTimestamp ||
        Read16(nt + 24u) != 0x010bu ||
        Read32(nt + 0x50u) != kSizeOfImage) return false;
    const U8 ctor[] = {0x81,0xEC,0x5C,0x02,0x00,0x00};
    const UPtr ctorPtr = Address(kGraphicsCtor);
    if (!Readable(ctorPtr,sizeof(ctor))) return false;
    for (U32 i=0u;i<sizeof(ctor);++i)
        if (Read8(ctorPtr+i)!=ctor[i]) return false;
    return true;
}
static bool VerifyStockHandlerBytes(UPtr target) {
    const U8 stock[]={0xA1,0x28,0x72,0xFE,0x00};
    if(!Readable(target,sizeof(stock)))return false;
    for(U32 i=0u;i<sizeof(stock);++i)
        if(Read8(target+i)!=stock[i])return false;
    return true;
}
static bool VerifyExecutable() {
    return VerifyExecutableIdentity() && VerifyStockHandlerBytes(Address(kHandlerInit));
}
#if defined(DSF_V29S_INDEXED_262144)
static bool VerifyInstalledHandlerHook(UPtr bridge,UPtr trampoline,UPtr originalFn,long installed) {
    const UPtr target=Address(kHandlerInit);
    if(installed!=1 || !bridge || !trampoline || originalFn!=trampoline ||
       !Readable(target,5u) || !Readable(bridge,1u) || !Readable(trampoline,10u))return false;
    if(Read8(target)!=0xE9u ||
       Read32(target+1u)!=bridge-(target+5u))return false;
    if(!VerifyStockHandlerBytes(trampoline) ||
       Read8(trampoline+5u)!=0xE9u ||
       Read32(trampoline+6u)!=(target+5u)-(trampoline+10u))return false;
    return true;
}
// Require the successful stock check from BEFORE any baseline hooks, and
// independently revalidate the stable identity and exact owned detour NOW.
static bool VerifyIndexedStartup(bool stockVerified,UPtr bridge,UPtr trampoline,UPtr originalFn,long installed) {
    return stockVerified && VerifyExecutableIdentity() &&
        VerifyInstalledHandlerHook(bridge,trampoline,originalFn,installed);
}
#endif

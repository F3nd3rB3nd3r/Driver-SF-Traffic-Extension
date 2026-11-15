// 0059C5C0 delegates to this byte getter after its native manager-state gate.
// Keep the byte metadata for the two small subtypes and all other packages.
// Only the vehicle package already widened by HandlerInitBridge owns models.
typedef U32 (__thiscall* WideCapacityFn)(void*,U32);
static WideCapacityFn g_wideCapacityOriginal=nullptr;
static bool g_wideCapacityInstalled=false;
static volatile long g_wideCapacityOverrides=0,g_wideCapacityFallbacks=0;
static volatile UPtr g_wideCapacityLastPackage=0;
static volatile U32 g_wideCapacityNativeByte=0;
static const U8 kWideCapacitySig[]={0x8B,0x81,0xF8,0x72,0x00,0x00,
    0x8B,0x4C,0x24,0x04,0x0F,0xB6,0x44,0x08,0x18,0xC2,0x04,0x00};
static U32 __fastcall WideCapacityQuery(void* self,void*,U32 subtype){
    const UPtr package=reinterpret_cast<UPtr>(self);
    if(IsPkg(package) && subtype==0u){
        g_wideCapacityLastPackage=package;
        g_wideCapacityNativeByte=Read8(Read32(package+0x72F8u)+0x18u);
        InterlockedIncrement(&g_wideCapacityOverrides);
        return g_limits.models;
    }
    InterlockedIncrement(&g_wideCapacityFallbacks);
    return g_wideCapacityOriginal(self,subtype);
}

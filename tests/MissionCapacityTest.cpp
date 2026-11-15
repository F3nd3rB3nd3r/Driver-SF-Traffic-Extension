// Execute retail admission/catalog/list/getter code in a copied image only.
// No game entry point, renderer, disk loader or audio middleware is started.
#define WideTestMain UnusedWideTestMain
#include "WideLimitsTest.cpp"
#undef WideTestMain
static UPtr g_queryAbiTarget=0,g_queryAbiEsp=0;
extern "C" __declspec(naked) U32 __stdcall CapacityAbiProbe(UPtr,U32){__asm{
    push ebp
    push ebx
    push esi
    push edi
    mov dword ptr [g_queryAbiEsp],esp
    mov ecx,[esp+20]
    push dword ptr [esp+24]
    mov ebp,12345678h
    mov ebx,23456789h
    mov esi,3456789Ah
    mov edi,456789ABh
    call dword ptr [g_queryAbiTarget]
    cmp esp,dword ptr [g_queryAbiEsp]
    jne abi_bad
    cmp ebp,12345678h
    jne abi_bad
    cmp ebx,23456789h
    jne abi_bad
    cmp esi,3456789Ah
    jne abi_bad
    cmp edi,456789ABh
    jne abi_bad
    jmp abi_done
abi_bad:
    mov esp,dword ptr [g_queryAbiEsp]
    mov eax,0DEADBEEFh
abi_done:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 8
}}
typedef U32 (__thiscall* ManagerCapacity)(void*,U32,U32);
typedef U8 (__thiscall* MissionRequest)(void*,U32);
static U32 Capacity(UPtr manager,U32 kind,U32 subtype){
    return reinterpret_cast<ManagerCapacity>(Address(0x59C5C0u))(reinterpret_cast<void*>(manager),kind,subtype);
}
static bool Request(UPtr owner,U32 id){
    return reinterpret_cast<MissionRequest>(Address(0x5A6FD0u))(reinterpret_cast<void*>(owner),id)!=0;
}
static bool Loaded(UPtr owner,U32 id){
    return reinterpret_cast<MissionRequest>(Address(0x5A6EA0u))(reinterpret_cast<void*>(owner),id)!=0;
}
static UPtr FixtureOwner(UPtr manager,U32 count){
    const UPtr owner=Alloc(0x1000u);Write32(owner+0xDA8u,manager);
    // The completed traffic list uses the real owner sentinel and native links.
    EmptyList(owner+0xCCu,0x264u);Write32(owner+0xD4u,count);
    const UPtr end=owner+0x330u,nodes=Alloc(count*12u);
    Write32(end,nodes+(count-1u)*12u);Write32(end+4u,nodes);
    for(U32 i=0;i<count;++i){const UPtr node=nodes+i*12u;
        Write32(node,i?node-12u:end);Write32(node+4u,i+1u<count?node+12u:end);Write32(node+8u,1000u+i);}
    EmptyList(owner+0x5A4u,0x3F4u);EmptyList(owner+0x9A0u,0x3F4u);
    return owner;
}
static UPtr FixturePackage(UPtr manager,U32 count,U32 models,bool existing){
    const UPtr package=manager+0xAF68u,meta=existing?Read32(package+0x72F8u):Alloc(0x100u);
    Write32(manager+0x6FD90u,2u);Write32(package+0x72F8u,meta);
    *reinterpret_cast<U8*>(meta+0x18u)=static_cast<U8>(models>255u?255u:models);
    *reinterpret_cast<U8*>(meta+0x19u)=2u;*reinterpret_cast<U8*>(meta+0x1Au)=2u;
    const UPtr catalog=Alloc((count+1u)*6u);Write32(package+0x72FCu,catalog);
    *reinterpret_cast<U16*>(meta+2u)=static_cast<U16>(count+1u);
    for(U32 i=0;i<count;++i)*reinterpret_cast<U16*>(catalog+i*6u)=static_cast<U16>(1000u+i);
    *reinterpret_cast<U16*>(catalog+count*6u)=123u;
    return package;
}
static void EmptyMission(UPtr owner){EmptyList(owner+0x9A0u,0x3F4u);}
static void Pending(UPtr owner,U32 count){
    CHECK(Read32(owner+0x9A8u)==count);
    if(count){const UPtr node=Read32(owner+0xD98u);CHECK(Read32(node+8u)==123u && Read32(node+12u)==0u && Read32(node+16u)>0u);}
}
extern "C" void MissionTestMain(){
    SetErrorMode(3u);const U32 n=Argument();
    g_limits={n,WideMax(384u,n+32u),4096,128,2,WideMax(512u,n),1,true,"configured",0,0,0,0,0,0,0,0,180000};
    g_limits=GenericValidate(g_limits);CHECK(g_limits.valid && g_limits.models==WideMax(160u,n+10u));
    MapNativeImage();CHECK(WideStartupStorage());g_wideFailureSink=&FixtureFailure;
    g_queryAbiTarget=Address(0x5A0AD0u);
    const UPtr beforeManager=Alloc(0x80000u),beforePackage=FixturePackage(beforeManager,n,g_limits.models,false);
    const UPtr beforeOwner=FixtureOwner(beforeManager,n);const U32 byteCapacity=Read8(Read32(beforePackage+0x72F8u)+0x18u);
    CHECK(Capacity(beforeManager,0u,0u)==byteCapacity && CapacityAbiProbe(beforePackage,0u)==byteCapacity);
    CHECK(!Loaded(beforeOwner,123u));
    const bool before=Request(beforeOwner,123u);CHECK(before==(n+1u<byteCapacity));Pending(beforeOwner,before?1u:0u);
    if(n==280u){CHECK(!before && byteCapacity==255u);Say("PASS native reproduction: 280 traffic + mission123 rejected against byte255\n");}
    if(WideContains(GetCommandLineA()," signature")){
        // A suffix mismatch beyond the stolen instruction must refuse every site.
        *reinterpret_cast<U8*>(Address(0x5A0ADFu))=0xC3;
        CHECK(!InstallExtendedSites() && !g_wideCapacityInstalled && !g_codeStubs);
        CHECK(Read8(Address(0x5A14EAu))==kWideAdmissionSig[0]);
        Say("PASS full getter signature refusal before mutation\n");ExitProcess(0u);
    }
    CHECK(InstallPool512Patches() && InstallRequestFramePatches() && InstallMerge150Fix() && InstallExtendedSites());
    CHECK(g_wideCapacityInstalled && g_wideCapacityOriginal);
    // Real patched native initialization proves full-width backing/counts;
    // only the unrelated handler factory is supplied by the inherited fixture.
    NativeInitializer(g_limits.models);
    const UPtr manager=Read32(Read32(Address(kRootPtr))+0x204u);
    const UPtr package=FixturePackage(manager,n,g_limits.models,true),owner=FixtureOwner(manager,n);
    CHECK(package==g_pkg0 && Read32(package+0x5834u)==g_limits.models);
    CHECK(Capacity(manager,0u,0u)==g_limits.models && CapacityAbiProbe(package,0u)==g_limits.models);
    CHECK(Capacity(manager,0u,1u)==2u && Capacity(manager,0u,2u)==2u);
    CHECK(CapacityAbiProbe(package,1u)==2u);
    CHECK(!Loaded(owner,123u) && Request(owner,123u));Pending(owner,1u);
    CHECK(!Loaded(owner,123u)); // admission queues work; it cannot fabricate READY.
    CHECK(Read32(owner+0xD4u)==n && Read32(owner+0xDACu)==0u);
    CHECK(Request(owner,123u));Pending(owner,1u); // real native duplicate/reference path.
    EmptyMission(owner);const U32 models=g_limits.models;
    g_limits.models=n+1u;CHECK(!Request(owner,123u));Pending(owner,0u); // native >= boundary retained.
    g_limits.models=n+2u;CHECK(Request(owner,123u));Pending(owner,1u);EmptyMission(owner);
    Write32(owner+0x5ACu,1u);CHECK(!Request(owner,123u));Pending(owner,0u);
    Write32(owner+0x5ACu,0u);g_limits.models=models;
    const long overrides=g_wideCapacityOverrides;
    CHECK(!Request(owner,65500u));Pending(owner,0u);CHECK(g_wideCapacityOverrides==overrides);
    Write32(manager+0x6FD90u,1u);CHECK(!Request(owner,123u) && Capacity(manager,0u,0u)==0u);
    CHECK(g_wideCapacityOverrides==overrides);Write32(manager+0x6FD90u,2u);
    const UPtr other=package+0x734Cu,otherMeta=Alloc(0x100u);Write32(other+0x72F8u,otherMeta);
    *reinterpret_cast<U8*>(otherMeta+0x18u)=17u;*reinterpret_cast<U8*>(otherMeta+0x19u)=3u;
    CHECK(Capacity(manager,1u,0u)==17u && Capacity(manager,1u,1u)==3u && CapacityAbiProbe(other,0u)==17u);
    // Getter owns no cached generation storage. Follow the same g_pkg0 update
    // as HandlerInitBridge, including no recognized package and a reused address.
    g_pkg0=0;CHECK(Capacity(manager,0u,0u)==byteCapacity);
    for(U32 i=0;i<12u;++i){
        const UPtr nextManager=Alloc(0x80000u),nextPackage=FixturePackage(nextManager,n,models,false);
        g_pkg0=nextPackage;CHECK(Capacity(nextManager,0u,0u)==models && Capacity(manager,0u,0u)==byteCapacity);
        CHECK(Capacity(nextManager,0u,1u)==2u);g_pkg0=package;
        CHECK(Capacity(manager,0u,0u)==models && Capacity(nextManager,0u,0u)==byteCapacity);
    }
    CHECK(Read8(Read32(package+0x72F8u)+0x18u)==byteCapacity);
    CHECK(g_wideCapacityOverrides>0 && g_wideCapacityFallbacks>0);
    g_capacityFile=GetStdHandle(0xFFFFFFF5u);WideCapacityLog(true,0u);WideCapacityLog(false,100u);
    RestoreExtendedSites();CHECK(!g_wideCapacityInstalled && Capacity(manager,0u,0u)==byteCapacity);
    CHECK(CapacityAbiProbe(package,0u)==byteCapacity);
    for(U32 i=0;i<sizeof(kWideCapacitySig);++i)CHECK(Read8(Address(0x5A0AD0u)+i)==kWideCapacitySig[i]);
    EmptyMission(owner);CHECK(Request(owner,123u)==before);Pending(owner,before?1u:0u);
    Say("PASS native mission admission requests=");Number(n);Say(" models=");Number(models);Say(" checks=");Number(g_checks);Say("\n");ExitProcess(0u);
}

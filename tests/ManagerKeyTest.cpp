// Retail list initialization, search, insertion, removal, reference access and
// actual R8 hooks in a private copied image; no game entry point or loader.
#define StreamTestMain UnusedStreamTestMain
#include "StreamHandlerTest.cpp"
#undef StreamTestMain
static UPtr g_keyInitSlice=0,g_keyDestroySlice=0,g_keyCanary=0;static U32 g_keyCanaryValue=0;
extern "C" __declspec(naked) void __stdcall FixtureKeyInit(UPtr){__asm{
    push ebx
    push esi
    mov esi,[esp+12]
    xor ebx,ebx
    call dword ptr [g_keyInitSlice]
    pop esi
    pop ebx
    ret 4
}}
extern "C" __declspec(naked) void __stdcall FixtureKeyDestroy(UPtr){__asm{
    push ebx
    mov ebx,[esp+8]
    call dword ptr [g_keyDestroySlice]
    pop ebx
    ret 4
}}
static void KeyFailure(const char* s){
    if(g_keyCanary)CHECK(Read32(g_keyCanary)==g_keyCanaryValue);
    CHECK(WideContains(s,"manager key list"));Say("PASS key fail-before-native-search/write\n");
}
static U32 AddKey(UPtr h,U32 id){
    typedef UPtr (__thiscall* Counter)(void*,const U32*);
    const UPtr slot=reinterpret_cast<Counter>(Address(0x59E290u))(reinterpret_cast<void*>(h),&id);
    CHECK(Read16(slot-2u)==id);const U32 refs=Read16(slot)+1u;*reinterpret_cast<U16*>(slot)=static_cast<U16>(refs);return refs;
}
static U32 QueryKey(UPtr manager,U32 kind,U32 id){
    typedef U32 (__thiscall* Query)(void*,U32,U32);
    return reinterpret_cast<Query>(Address(0x59CD20u))(reinterpret_cast<void*>(manager),kind,id);
}
static bool RemoveKey(UPtr h,U32 id){
    typedef U32 (__thiscall* Remove)(void*,const U32*);
    return reinterpret_cast<Remove>(Address(0x59E030u))(reinterpret_cast<void*>(h),&id)!=0;
}
static void DirectInsert(UPtr h,U32 value){
    typedef UPtr (__thiscall* Insert)(void*,UPtr,UPtr,const U32*);
    U32 result=0;reinterpret_cast<Insert>(Address(0x59E120u))(reinterpret_cast<void*>(h),reinterpret_cast<UPtr>(&result),h+0xC0Cu,&value);
}
static void CheckKeyList(UPtr owner,U32 kind,U32 active){
    auto* a=ManagerKeyFindOwner(owner);CHECK(a && a->alive);const char* why=nullptr;ManagerKeyState s={};
    CHECK(ManagerKeyValidate(ManagerKeyHeader(owner,kind),a->lists[kind],s,why));CHECK(s.active==active && s.issued<=ManagerKeyCapacity());
}
static void InitSlices(){
    StreamFixtureRelocations();g_keyInitSlice=Address(0x59D1F7u);g_keyDestroySlice=Address(0x59D566u);
    *reinterpret_cast<U8*>(Address(0x59D223u))=0xC3; // stop after the real14-list constructor loop.
    *reinterpret_cast<U8*>(Address(0x59D56Du))=0xC3; // stop after the real end-of-destructor hook.
}
extern "C" void ManagerKeyTestMain(){
    SetErrorMode(3u);const U32 n=Argument();g_limits={n,WideMax(384u,n+32u),4096,128,2,WideMax(512u,n),1,true,"configured",0,0,0,0,0,0,0,0,180000};
    g_limits=GenericValidate(g_limits);CHECK(g_limits.valid);MapNativeImage();g_wideFailureSink=&KeyFailure;InitSlices();
    const UPtr owner=Alloc(0x80000u);FixtureKeyInit(owner);const UPtr h=ManagerKeyHeader(owner,0u),end=h+0xC0Cu;
    if(WideContains(GetCommandLineA()," legacy")){
        for(U32 i=1;i<=548u;++i)DirectInsert(h,0x10000u+i);
        CHECK(Read32(h+8u)==548u && Read32(end+8u)==0x10101u);
        CHECK(Read32(ManagerKeyHeader(owner,1u)+8u)!=0u);
        Say("PASS native legacy548 insertions overwrite256-node sentinel and adjacent header\n");ExitProcess(0u);
    }
    if(WideContains(GetCommandLineA()," captured-hang")){
        const UPtr other=ManagerKeyHeader(owner,1u)+0xC0Cu;
        Write32(h+8u,548u);Write32(end+4u,other);Write32(other+8u,0x10029u);
        Say("EXPECTED original native lookup repeats41/self while searching42\n");QueryKey(owner,0u,42u);CHECK(false);
    }
    if(WideContains(GetCommandLineA()," signature")){
        *reinterpret_cast<U8*>(Address(0x59D56Cu))=0x01;
        CHECK(!ManagerKeyInstall() && Read8(Address(0x59E120u))==0x83 && Read8(Address(0x59E0D0u))==0x8B);
        Say("PASS key all-site signature refusal\n");ExitProcess(0u);
    }
    CHECK(StreamInstall() && ManagerKeyInstall());FixtureConstructManager(owner);
    CHECK(ManagerKeyFindOwner(owner)->alive);g_capacityFile=GetStdHandle(0xFFFFFFF5u);ManagerKeyLog(true,0u);
    if(WideContains(GetCommandLineA()," cycle")){
        const UPtr other=ManagerKeyHeader(owner,1u)+0xC0Cu;Write32(h+8u,548u);Write32(end+4u,other);Write32(other+8u,0x10029u);
        g_keyCanary=h+8u;g_keyCanaryValue=548u;QueryKey(owner,0u,42u);CHECK(false);
    }
    const U32 cap=ManagerKeyCapacity();
    for(U32 kind=0;kind<14u;++kind){
        const UPtr list=ManagerKeyHeader(owner,kind),next=list+0xC18u;const U32 saved[3]={Read32(next),Read32(next+4u),Read32(next+8u)};
        const U32 count=kind==0u || n==280u?cap:257u;
        for(U32 id=1;id<=count;++id)CHECK(AddKey(list,id)==1u);
        CheckKeyList(owner,kind,count);CHECK(Read32(next)==saved[0] && Read32(next+4u)==saved[1] && Read32(next+8u)==saved[2]);
        CHECK(Read32(list+0xC14u)==0u && QueryKey(owner,kind,1u)==1u && QueryKey(owner,kind,count)==1u && QueryKey(owner,kind,65500u)==0u);
        const UPtr nodes=ManagerKeyFindOwner(owner)->lists[kind].nodes;CHECK(nodes);
        if(kind==0u && WideContains(GetCommandLineA()," full")){g_keyCanary=next;g_keyCanaryValue=saved[0];AddKey(list,count+1u);CHECK(false);}
        if(kind==0u && WideContains(GetCommandLineA()," guard")){Write32(nodes-16u,0u);g_keyCanary=next;g_keyCanaryValue=saved[0];QueryKey(owner,0u,42u);CHECK(false);}
        CHECK(AddKey(list,100u)==2u && QueryKey(owner,kind,100u)==2u);CheckKeyList(owner,kind,count);
        for(U32 id=1;id<=count;id+=2u)CHECK(RemoveKey(list,id));
        for(U32 id=1;id<=count;id+=2u)CHECK(AddKey(list,5000u+id)==1u);
        CheckKeyList(owner,kind,count);CHECK(ManagerKeyFindOwner(owner)->lists[kind].nodes==nodes);
        for(U32 id=2;id<=count;id+=2u)CHECK(RemoveKey(list,id));
        for(U32 id=1;id<=count;id+=2u)CHECK(RemoveKey(list,5000u+id));CheckKeyList(owner,kind,0u);
    }
    // Simulate overlapping280 rosters: twelve retained IDs,548 distinct keys
    // before retirement. All actual search/insert/remove operations stay native.
    const UPtr stable=ManagerKeyFindOwner(owner)->lists[0].nodes;
    for(U32 epoch=0;epoch<5u;++epoch){
        for(U32 id=1;id<=280u;++id)CHECK(AddKey(h,id)==1u);
        for(U32 id=269u;id<=548u;++id)CHECK(AddKey(h,id)==(id<=280u?2u:1u));CheckKeyList(owner,0u,548u);
        for(U32 id=1;id<=268u;++id)CHECK(RemoveKey(h,id));CheckKeyList(owner,0u,280u);CHECK(QueryKey(owner,0u,42u)==0u);
        for(U32 id=269u;id<=548u;++id)CHECK(RemoveKey(h,id));CheckKeyList(owner,0u,0u);
        CHECK(ManagerKeyFindOwner(owner)->lists[0].nodes==stable);
    }
    // Shared helper's unrelated native caller retains its inline behavior.
    const UPtr foreign=Alloc(0xC18u);EmptyList(foreign,0xC0Cu);for(U32 i=1;i<=100u;++i)CHECK(AddKey(foreign,i)==1u);
    CHECK(Read32(foreign+8u)==100u && Read32(foreign)==foreign+12u+100u*12u);
    for(U32 generation=0;generation<12u;++generation){
        auto* prior=ManagerKeyFindOwner(owner);const UPtr oldNodes=prior->lists[0].nodes;
        FixtureDestroyManager(owner);FixtureKeyDestroy(owner);CHECK(!prior->alive && ManagerKeyGuards(prior->lists[0]));
        FixtureKeyInit(owner);FixtureConstructManager(owner);auto* next=ManagerKeyFindOwner(owner);CHECK(next!=prior && next->generation==prior->generation+1u);
        for(U32 id=1;id<=257u;++id)CHECK(AddKey(h,id)==1u);CheckKeyList(owner,0u,257u);
        CHECK(next->lists[0].nodes!=oldNodes && ManagerKeyGuards(prior->lists[0]));
    }
    FixtureDestroyManager(owner);FixtureKeyDestroy(owner);ManagerKeyLog(false,2000u);
    CHECK(g_managerKeyFailures==0 && g_managerKeyGuardBad==0 && g_managerKeyDestructions==13 && g_managerKeyNative>0);
    Say("PASS manager keys requests=");Number(n);Say(" capacity=");Number(cap);Say(" checks=");Number(g_checks);Say("\n");ExitProcess(0u);
}

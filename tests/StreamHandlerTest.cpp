// Isolated native slices and production hooks; never runs the game entry point.
#define WideTestMain UnusedWideTestMain
#include "WideLimitsTest.cpp"
#undef WideTestMain
static UPtr g_streamFixtureConstruct=0,g_streamFixtureDestroy=0,g_streamFixtureMember=0,g_streamFixtureMemberEnd=0;
static UPtr g_streamFixtureScan=0,g_streamFixtureNull=0;
static UPtr g_streamFixtureFourth=0;
static U8 __cdecl FixturePointerLess(UPtr left,UPtr right){return left<right?1u:0u;}
extern "C" __declspec(naked) void __stdcall FixtureFourthInsert(UPtr,U32){
    __asm {
        push ebx
        push esi
        push edi
        mov edi,[esp+16]
        mov eax,[esp+20]
        sub esp,20h
        mov [esp+0Ch],eax
        call dword ptr [g_streamFixtureFourth]
        add esp,20h
        pop edi
        pop esi
        pop ebx
        ret 8
    }
}
extern "C" __declspec(naked) UPtr __stdcall FixtureDispatchScan(UPtr){
    __asm {
        push esi
        push ebp
        mov eax,[esp+12]
        sub esp,20h
        mov [esp+10h],eax
        call dword ptr [g_streamFixtureScan]
        add esp,20h
        mov eax,esi
        pop ebp
        pop esi
        ret 4
    }
}
extern "C" __declspec(naked) UPtr __stdcall FixtureNullSite(UPtr,UPtr){
    __asm {
        push esi
        push edi
        mov esi,[esp+12]
        mov edi,292
        mov eax,[esp+16]
        sub esp,20h
        mov [esp+10h],eax
        call dword ptr [g_streamFixtureNull]
        add esp,20h
        pop edi
        pop esi
        ret 8
    }
}
extern "C" __declspec(naked) void __stdcall FixtureConstructManager(UPtr){
    __asm {
        push ebx
        push esi
        mov esi,[esp+12]
        xor ebx,ebx
        call dword ptr [g_streamFixtureConstruct]
        pop esi
        pop ebx
        ret 4
    }
}
extern "C" __declspec(naked) void __stdcall FixtureDestroyManager(UPtr){
    __asm {
        push ebx
        push esi
        push edi
        mov ebx,[esp+16]
        call dword ptr [g_streamFixtureDestroy]
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}
extern "C" __declspec(naked) void __stdcall FixtureMemberInsert(UPtr,U32,U32){
    __asm {
        sub esp,28h
        push ebx
        push ebp
        push esi
        push edi
        mov esi,[esp+3Ch]
        mov eax,[esp+40h]
        mov ecx,dword ptr [g_streamFixtureMemberEnd]
        xor edx,edx
        jmp dword ptr [g_streamFixtureMember]
    }
}
static void StreamFixtureRelocations(){
    // The copied image lives at20000000; only these fixture-executed absolute
    // operands are relocated. Production signature checks remain retail-only.
    CHECK(Read32(Address(0x59BB99u))==0xDE44B4u);Write32(Address(0x59BB99u),Address(0xDE44B4u));
    CHECK(Read32(Address(0x59BBA1u))==0xDE4A80u);Write32(Address(0x59BBA1u),Address(0xDE4A80u));
    CHECK(Read32(Address(0x59E1D8u))==0xDE4A80u);Write32(Address(0x59E1D8u),Address(0xDE4A80u));
    for(U32 i=0;i<39u;++i){const UPtr at=Address(0xDE4A80u)+i*4u;Write32(at,Address(Read32(at)));}
    // Native manager constructor/destructor bodies have unrelated factories.
    // Stop after the actual patched constructor/count and destructor-loop
    // slices; do not stub any pool iteration or native record constructor.
    *reinterpret_cast<U8*>(Address(0x59D259u))=0xC3;
    *reinterpret_cast<U8*>(Address(0x59D54Du))=0xC3;
    g_streamFixtureConstruct=Address(0x59D253u);g_streamFixtureDestroy=Address(0x59D51Eu);
    g_streamFixtureMember=Address(0x5A3F08u);
    g_streamFixtureScan=Address(0x59D9D2u);*reinterpret_cast<U8*>(Address(0x59DA0Au))=0xC3;
    g_streamFixtureNull=Address(0x59DA4Au);*reinterpret_cast<U8*>(Address(0x59DA4Fu))=0xC3;
    // Execute the exact caller/base/count/search/insertion path in dump19620.
    // Only the resource comparator is replaced; raw pointer ordering supplies
    // deterministic fixture data without invoking middleware resource methods.
    g_streamFixtureFourth=Address(0x5A4D46u);*reinterpret_cast<U8*>(Address(0x5A4D7Du))=0xC3;
    CHECK(Read32(Address(0x5A4D55u))==0x5A2830u);Write32(Address(0x5A4D55u),reinterpret_cast<UPtr>(&FixturePointerLess));
}
static void NativeSchedulerInit(UPtr owner){
    typedef UPtr (__thiscall* Init)(void*);CHECK(reinterpret_cast<Init>(Address(0x5A3390u))(reinterpret_cast<void*>(owner))==owner);
    CHECK(StreamSchedulerFind(owner) && StreamSchedulerFind(owner)->alive && StreamSchedulerGuards(owner));
    for(U32 i=0;i<4u;++i)CHECK(Read32(owner+StreamVectorBase(i)+StreamCapacity()*4u)==0u);
    for(U32 i=0;i<14u;++i){const UPtr h=owner+StreamVectorStride()*3u+i*StreamMemberStride(),end=h+StreamMemberStride()-12u;
        CHECK(Read32(h)==h+12u && !Read32(h+4u) && !Read32(h+8u) && Read32(end)==end && Read32(end+4u)==end);}
}
static void NativeSchedulerDestroy(UPtr owner){
    // The empty fixture scheduler has no middleware-owned buffers. Replace
    // just its native CRT free(NULL) call, avoiding game import resolution.
    const UPtr at=Address(0x5A2F30u);Write32(at+1u,reinterpret_cast<UPtr>(&FixtureReturn)-(at+5u));
    typedef void (__thiscall* Destroy)(void*);reinterpret_cast<Destroy>(Address(0x5A2F00u))(reinterpret_cast<void*>(owner));
    CHECK(!StreamSchedulerFind(owner)->alive && StreamSchedulerGuards(owner));
}
static void StreamFixtureFailure(const char* s){CHECK(WideContains(s,"streaming handler"));Say("PASS guarded streaming failure\n");}
static void NativeVectors(UPtr scheduler){
    typedef UPtr (__thiscall* Insert)(void*,UPtr,UPtr,UPtr);
    typedef UPtr (__thiscall* Erase)(void*,UPtr,UPtr);
    const U32 cap=StreamCapacity();
    for(U32 kind=0;kind<4u;++kind){
        const UPtr vector=scheduler+StreamVectorBase(kind);
        for(U32 i=0;i<cap;++i){U32 value=0xA1000000u+i,out=0;
            CHECK(reinterpret_cast<Insert>(Address(kind==3u?0x5A55A0u:0x5A5520u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector+i*4u,reinterpret_cast<UPtr>(&value))==reinterpret_cast<UPtr>(&out));
            CHECK(out==vector+i*4u && Read32(out)==value);}
        CHECK(Read32(vector+cap*4u)==cap && StreamSchedulerGuards(scheduler));
        if(WideContains(GetCommandLineA()," vector-full")){g_wideFailureSink=&StreamFixtureFailure;U32 out=0,value=123;
            reinterpret_cast<Insert>(Address(0x5A5520u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector+cap*4u,reinterpret_cast<UPtr>(&value));CHECK(false);}
        // Erase beyond the stock256th element and insert into the middle.
        U32 out=0;reinterpret_cast<Erase>(Address(0x5A5370u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector+257u*4u);
        CHECK(Read32(vector+cap*4u)==cap-1u && Read32(vector+257u*4u)==0xA1000102u);
        U32 value=0x12345678u;reinterpret_cast<Insert>(Address(0x5A5520u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector+100u*4u,reinterpret_cast<UPtr>(&value));
        CHECK(Read32(vector+100u*4u)==value && Read32(vector+cap*4u)==cap);
        while(Read32(vector+cap*4u))reinterpret_cast<Erase>(Address(0x5A5370u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector);
        CHECK(StreamSchedulerGuards(scheduler));
    }
}
static void NativeFourthQueue(UPtr scheduler){
    const U32 cap=StreamCapacity();const UPtr vector=scheduler+StreamFourthBase(),count=vector+cap*4u;
    // Poison the obsolete counter location with the low30 count bits implied
    // by the actual dump's EAX/ECX arithmetic. Correct code never reads it.
    const UPtr obsolete=scheduler+0x1655Cu;const U32 saved=Read32(obsolete);Write32(obsolete,0x1C9F9F01u);
    if(WideContains(GetCommandLineA()," legacy-repro")){
        // Restore precisely the missing R5 caller-base/helper-count routing.
        // This disposable process intentionally takes the native access fault.
        DiagFRestoreDetour(g_streamDetours[7],g_streamTramps[7]);
        Write32(Address(0x5A4D50u),0x15D5Cu);
        const UPtr sites[]={0x5A55A2u,0x5A55C7u,0x5A55DEu};for(UPtr p:sites)Write32(Address(p),0x800u);
        Say("Reproducing R5 fourth-vector base/count mismatch\n");FixtureFourthInsert(scheduler,0xA1000000u);CHECK(false);
    }
    for(U32 i=0;i<cap;++i){FixtureFourthInsert(scheduler,0xA1000000u+cap-i);CHECK(Read32(count)==i+1u);}
    for(U32 i=0;i<cap;++i)CHECK(Read32(vector+i*4u)==0xA1000001u+i);
    CHECK(Read32(obsolete)==0x1C9F9F01u && StreamSchedulerGuards(scheduler));
    if(WideContains(GetCommandLineA()," fourth-full")){g_wideFailureSink=&StreamFixtureFailure;FixtureFourthInsert(scheduler,0xA0000000u);CHECK(false);}
    // Isolated native count/iterator geometry also works for fourth-list erase.
    typedef UPtr (__thiscall* Erase)(void*,UPtr,UPtr);U32 out=0;
    while(Read32(count))reinterpret_cast<Erase>(Address(0x5A5370u))(reinterpret_cast<void*>(vector),reinterpret_cast<UPtr>(&out),vector);
    Write32(obsolete,saved);
}
static void NativeMembers(UPtr scheduler){
    typedef U32 (__thiscall* Has)(void*,U32,U32);
    const U32 cap=StreamCapacity();
    for(U32 kind=0;kind<14u;++kind){
        const UPtr h=scheduler+StreamVectorStride()*3u+kind*StreamMemberStride(),end=h+StreamMemberStride()-12u;
        g_streamFixtureMemberEnd=end;
        for(U32 i=0;i<cap;++i)FixtureMemberInsert(h,1000u+i,0u);
        CHECK(Read32(h+8u)==cap && Read32(h)==end && Read32(end)==h+12u+(cap-1u)*12u);
        CHECK(reinterpret_cast<Has>(Address(0x59E4E0u))(reinterpret_cast<void*>(scheduler),kind,1000u+cap-1u)==1u);
        CHECK(reinterpret_cast<Has>(Address(0x59E4E0u))(reinterpret_cast<void*>(scheduler),kind,50000u)==0u);
        CHECK(StreamSchedulerGuards(scheduler));
        if(WideContains(GetCommandLineA()," member-full")){g_wideFailureSink=&StreamFixtureFailure;FixtureMemberInsert(h,50000u,0u);CHECK(false);}
        // Native remove recycles a node; reinsert must reuse it without moving
        // the sentinel or writing beyond the configured inline membership.
        typedef void (__thiscall* Remove)(void*,U32,U32);
        reinterpret_cast<Remove>(Address(0x5A3F60u))(reinterpret_cast<void*>(scheduler),kind,1000u+cap-1u);
        CHECK(Read32(h+8u)==cap-1u && Read32(h+4u)==1u);
        FixtureMemberInsert(h,1000u+cap-1u,0u);CHECK(Read32(h+8u)==cap && !Read32(h+4u));
    }
}
extern "C" void StreamTestMain(){
    SetErrorMode(3u);U32 n=Argument();
    g_limits={n,WideMax(192u,n+32u),4096,128,2,WideMax(512u,n),1,true,"configured",0,0,0,0,0,0,0,0,0};
    g_limits=GenericValidate(g_limits);CHECK(g_limits.valid);MapNativeImage();
    if(WideContains(GetCommandLineA()," signature")){
        const UPtr at=Address(kStreamLayoutOperands[20].site);Write32(at,0xDEADBEEFu);
        const U32 before=Read32(Address(kStreamLayoutOperands[0].site));CHECK(!StreamInstall());
        CHECK(Read32(Address(kStreamLayoutOperands[0].site))==before && Read8(Address(0x59D253u))==0x89u);
        Say("PASS all-site signature preflight\n");ExitProcess(0u);
    }
    CHECK(StreamInstall());g_capacityFile=GetStdHandle(0xFFFFFFF5u);StreamLog(true,0u);StreamFixtureRelocations();
    const UPtr owner=Alloc(0x75000u),source=Alloc(80u);Write32(owner+0x74DA4u,0xA1234567u);
    FixtureConstructManager(owner);StreamArena* a=StreamFind(owner);CHECK(a && a->alive && !Read32(owner+0x74DA0u));
    typedef UPtr (__thiscall* Default)(void*);reinterpret_cast<Default>(Address(0x59BB90u))(reinterpret_cast<void*>(source));
    typedef void (__thiscall* Resize)(void*,U32,UPtr);
    reinterpret_cast<Resize>(&StreamResizeBridge)(reinterpret_cast<void*>(owner+0x6FDA0u),256u,source);
    const U32 cap=StreamCapacity();CHECK(Read32(owner+0x74DA0u)==cap && a->constructed==cap && StreamGuards(a));
    // Repeated native initialization preserves active records and pointers.
    const UPtr nodes=a->nodes;Write32(nodes+40u*80u+0x30u,0x11223344u);
    reinterpret_cast<Resize>(&StreamResizeBridge)(reinterpret_cast<void*>(owner+0x6FDA0u),256u,source);
    CHECK(StreamFind(owner)==a && a->nodes==nodes && Read32(nodes+40u*80u+0x30u)==0x11223344u);
    typedef UPtr (__thiscall* Acquire)(void*);
    for(U32 i=0;i<cap;++i)CHECK(reinterpret_cast<Acquire>(Address(0x59C700u))(reinterpret_cast<void*>(owner))==nodes+i*80u);
    CHECK(!reinterpret_cast<Acquire>(Address(0x59C700u))(reinterpret_cast<void*>(owner)) && StreamBusy(a)==cap);
    CHECK(Read32(owner+0x74DA4u)==0xA1234567u && StreamGuards(a));
    typedef void (__thiscall* Flag)(void*,U32);reinterpret_cast<Flag>(Address(0x59C2F0u))(reinterpret_cast<void*>(nodes+300u*80u),0u);
    CHECK(reinterpret_cast<Acquire>(Address(0x59C700u))(reinterpret_cast<void*>(owner))==nodes+300u*80u);
    reinterpret_cast<Flag>(Address(0x59C2F0u))(reinterpret_cast<void*>(nodes+(cap-1u)*80u),0u);
    CHECK(FixtureDispatchScan(owner)==nodes+(cap-1u)*80u && StreamBusy(a)==cap);
    CHECK(FixtureDispatchScan(owner)==0u);
    CHECK(FixtureNullSite(nodes,owner)==Address(0xDE4A80u));
    const UPtr last=nodes+(cap-1u)*80u;
    Write32(last+0x34u,3u);Write32(last+0x38u,1234u);*reinterpret_cast<U16*>(last+0x28u)=4001u;
    typedef U8 (__thiscall* Lookup)(void*,U32,U32);
    CHECK(reinterpret_cast<Lookup>(Address(0x59C750u))(reinterpret_cast<void*>(owner),3u,1234u)==1u);
    CHECK(reinterpret_cast<Lookup>(Address(0x59C7D0u))(reinterpret_cast<void*>(owner),3u,4001u)==1u);
    CHECK(reinterpret_cast<Lookup>(Address(0x59C7D0u))(reinterpret_cast<void*>(owner),3u,4002u)==0u);
    if(WideContains(GetCommandLineA()," null")){
        g_wideFailureSink=&StreamFixtureFailure;FixtureNullSite(0u,owner);CHECK(false);
    }
    // Exact replacement of the captured null dereference is also entered with
    // a valid handler, through its compiled preservation bridge.
    if(WideContains(GetCommandLineA()," guard")){Write32(a->storage,0u);g_wideFailureSink=&StreamFixtureFailure;StreamLog(false,1u);CHECK(false);}
    const UPtr scheduler=Alloc(StreamSchedulerBytes()+16u);NativeSchedulerInit(scheduler);
    NativeVectors(scheduler);NativeFourthQueue(scheduler);NativeMembers(scheduler);
    StreamLog(false,1000u);NativeSchedulerDestroy(scheduler);NativeSchedulerInit(scheduler);NativeSchedulerDestroy(scheduler);
    FixtureDestroyManager(owner);CHECK(!a->alive && StreamGuards(a));
    FixtureConstructManager(owner);CHECK(StreamFind(owner)!=a && StreamFind(owner)->generation==2u && StreamFind(owner)->nodes!=nodes && StreamGuards(a));
    reinterpret_cast<Resize>(&StreamResizeBridge)(reinterpret_cast<void*>(owner+0x6FDA0u),256u,source);FixtureDestroyManager(owner);
    // Stable owner directories do not reintroduce a fixed lifetime limit.
    for(U32 i=0;i<10u;++i){const UPtr o=Alloc(0x75000u);FixtureConstructManager(o);reinterpret_cast<Resize>(&StreamResizeBridge)(reinterpret_cast<void*>(o+0x6FDA0u),256u,source);FixtureDestroyManager(o);}
    CHECK(!g_streamFailures && !g_streamGuardBad && g_streamGenerations==12 && g_streamDestructions==12);
    StreamLog(false,2000u);Say("PASS streaming handlers requested=");Number(n);Say(" capacity=");Number(cap);Say(" checks=");Number(g_checks);Say("\n");ExitProcess(0u);
}

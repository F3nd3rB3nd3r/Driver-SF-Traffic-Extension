// Uses the existing copied-image fixture; never calls Driver.exe's entry point.
#include "WideLimitsTest.cpp"
static U32 tf[128]={};static UPtr vehicle=0,controller=0;
static void Frame(bool inner,U32 mode=1u){
    memset(tf,0,sizeof(tf));tf[1]=controller;tf[6]=vehicle;tf[8]=0x202u;
    UPtr sp=reinterpret_cast<UPtr>(tf)+36u;
    Write32(sp+4u,0x12345678u);Write32(sp+8u,vehicle);Write32(sp+12u,mode);
    Write32(sp+0xCCu,Address(inner?0x5539AAu:0x5540E0u));Write32(sp+0xD8u,0x12345678u);
    Write32(controller+0xB4u,vehicle);Write32(controller+0x88u,mode);
}
static U32 NativeCmp(U32 a,U32 b){U32 f=0;__asm {mov eax,a
    cmp eax,b
    pushfd
    pop f}return f;}
static U32 NativeByte(U32 a){U32 f=0;__asm {mov eax,a
    cmp al,0
    pushfd
    pop f}return f;}
static void Decisions(){
    const U32 next[]={0x553874u,0x55392Du,0x55393Au,0x553966u,0x55399Au,0x5539D3u};
    const U32 reason[]={0,2,3,4,5,6};
    for(U32 site=0;site<6;++site)for(U32 bits=0;bits<4;++bits){
        Frame(false);tf[8]=0x602u|((bits&1u)?1u:0u)|((bits&2u)?0x40u:0u);
        U32 before[9];memcpy(before,tf,sizeof(before));const U32 count=g_audioSetupCounts[reason[site]];
        const bool z=(bits&2u)!=0u,failed=site==3?!z:site==4?bits==0:z;
        AudioSetupEntry(site,tf);CHECK(tf[3]==Address(failed?0x553AD5u:next[site]));
        for(U32 i=0;i<9;++i)if(i!=3)CHECK(tf[i]==before[i]);
        CHECK(static_cast<U32>(g_audioSetupCounts[reason[site]])==count+(failed?1u:0u));
    }
    const U32 states[]={0,1,2,3,4,0x80000000u,0xFFFFFFFFu};
    for(U32 site=6;site<10;site+=2)for(U32 v:states){
        Frame(true);Write32(vehicle+0x544u,v);const U32 count=g_audioSetupCounts[7];
        AudioSetupEntry(site,tf);CHECK((tf[8]&0x8D5u)==(NativeCmp(v,3u)&0x8D5u));
        CHECK(tf[3]==Address(v==3?(site==6?0x5535EDu:0x551F1Du):(site==6?0x5535E1u:0x551F11u)));
        CHECK(static_cast<U32>(g_audioSetupCounts[7])==count+(v!=3u));
    }
    for(U32 site=7;site<10;site+=2)for(U32 al=0;al<2;++al)for(U32 v:states){
        Frame(true);tf[7]=al?0x12340001u:0x12340000u;*reinterpret_cast<U8*>(reinterpret_cast<UPtr>(tf)+36u+0x9Bu)=static_cast<U8>(v);
        const U32 a=g_audioSetupCounts[8],b=g_audioSetupCounts[9];AudioSetupEntry(site,tf);
        CHECK(tf[3]==Address(!al || static_cast<U8>(v)?(site==7?0x5535E1u:0x551F11u):(site==7?0x553605u:0x551F35u)));
        if(al)CHECK((tf[8]&0x8D5u)==(NativeByte(v)&0x8D5u));else CHECK((tf[8]&0x8C5u)==(AudioModelTestFlags(0u,0)&0x8C5u));
        CHECK(static_cast<U32>(g_audioSetupCounts[8])==a+!al);
        CHECK(static_cast<U32>(g_audioSetupCounts[9])==b+(al && static_cast<U8>(v)));
    }
    Frame(false);const U32 sub=g_audioSetupCounts[7];Write32(vehicle+0x544u,2u);AudioSetupEntry(6,tf);CHECK(static_cast<U32>(g_audioSetupCounts[7])==sub);
    Frame(false);Write32(reinterpret_cast<UPtr>(tf)+48u,0xBADC0DEu); // Reused native mode-argument scratch.
    const U32 bank=g_audioSetupCounts[1];AudioSetupBankFailure(tf);CHECK(static_cast<U32>(g_audioSetupCounts[1])==bank+1u);
    const AudioSetupDetail& r=g_audioSetupRing[(g_audioSetupWrite-1u)&1023u];CHECK(r.caller==0x12345678u && r.vehicle==vehicle && r.controller==controller && r.model==191u && r.mode==1u);
    CHECK((r.flags&0x8C5u)==(AudioModelTestFlags(0u,tf[8])&0x8C5u));
    // Failures long after the old first128 successes still produce details.
    g_audioSetupRead=g_audioSetupWrite;g_capacityStart=GetTickCount()-900000u;AudioSetupBankFailure(tf);
    CHECK(g_audioSetupRing[(g_audioSetupWrite-1u)&1023u].tick>=900000u);
    const U32 oldDropped=g_audioSetupDropped;g_audioSetupLock=1;AudioSetupBankFailure(tf);g_audioSetupLock=0;CHECK(static_cast<U32>(g_audioSetupDropped)==oldDropped+1u);
    g_audioSetupRead=g_audioSetupWrite;
    for(U32 i=0;i<1025;++i)AudioSetupBankFailure(tf);
    CHECK(g_audioSetupWrite-g_audioSetupRead==1024u && static_cast<U32>(g_audioSetupDropped)==oldDropped+2u);
    g_audioSetupRead=g_audioSetupWrite;AudioSetupBankFailure(tf);CHECK(g_audioSetupWrite-g_audioSetupRead==1u);
}
static UPtr abiEsp=0,abiBridge=0;static U32 abiFlags=0,abiEax=0,abiEsi=0,abiEcx=0,abiEbx=0,abiEdx=0,abiEdi=0;
static __declspec(align(16)) U8 fxBefore[512]={},fxAfter[512]={};
extern "C" __declspec(naked) void AudioTraceCapture(){__asm {
    mov abiEax,eax
    mov abiEsi,esi
    mov abiEcx,ecx
    mov abiEbx,ebx
    mov abiEdx,edx
    mov abiEdi,edi
    pushfd
    pop abiFlags
    fxsave fxAfter
    ret
}}
static void Sink(U32 va){UPtr p=Address(va);*reinterpret_cast<U8*>(p)=0xE9u;Write32(p+1u,reinterpret_cast<UPtr>(&AudioTraceCapture)-(p+5u));}
static U32 inputFlags=0,inputAl=0;
static void AbiCall(){
    __asm {
        pushad
        mov abiEsp,esp
        sub esp,400h
        mov dword ptr [esp],12345678h
        mov eax,vehicle
        mov [esp+4],eax
        mov dword ptr [esp+8],1
        mov eax,0
        mov byte ptr [esp+97h],al
        mov eax,dword ptr [g_base]
        add eax,1539AAh
        mov [esp+0C8h],eax
        mov dword ptr [esp+0D4h],12345678h
        fninit
        fld1
        pcmpeqd xmm0,xmm0
        fxsave fxBefore
        mov esi,controller
        mov ecx,vehicle
        mov eax,inputAl
        mov ebx,11223344h
        mov edx,55667788h
        mov edi,76543210h
        push inputFlags
        popfd
        call dword ptr [abiBridge]
        cld
        mov esp,abiEsp
        popad
    }
}
static void Abi(){
    const U32 sinks[]={0x553AD5u,0x553874u,0x55392Du,0x55393Au,0x553966u,0x55399Au,0x5539D3u,0x5535EDu,0x5535E1u,0x551F1Du,0x551F11u,0x553605u,0x551F35u};
    for(U32 v:sinks)Sink(v);
    for(U32 site=0;site<10;++site)for(U32 z=0;z<2;++z){
        abiBridge=kAudioSetupSites[site].bridge;inputFlags=0x602u|(z?0x40u:0u);inputAl=z?0u:1u;
        Write32(vehicle+0x544u,z?2u:3u);AbiCall();
        CHECK(abiEax==inputAl && abiEsi==controller && abiEcx==vehicle && abiEbx==0x11223344u && abiEdx==0x55667788u && abiEdi==0x76543210u);
        if(site<6)CHECK((abiFlags&0xCD5u)==(inputFlags&0xCD5u));
        CHECK(abiFlags&0x400u); // Direction flag restored despite helper's CLD.
        for(U32 i=0;i<4;++i)CHECK(fxBefore[i]==fxAfter[i]);
        for(U32 i=32;i<42;++i)CHECK(fxBefore[i]==fxAfter[i]);
        for(U32 i=160;i<176;++i)CHECK(fxBefore[i]==fxAfter[i]);
    }
}
static void Profiles(){
    const char* paths[]={"dist/USERCAP_R9/DriverSF_Unlimiter.ini","dist/USERCAP_R9/profiles/Baseline150.ini","dist/USERCAP_R9/profiles/Headroom320.ini"};
    const U32 expectedRequests[]={280,150,320},expectedModels[]={290,160,330},expectedWork[]={802,512,1098};
    for(U32 i=0;i<3;++i){
        HANDLE file=CreateFileA(paths[i],0x80000000u,1u,nullptr,3u,0u,nullptr);CHECK(file!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
        static char bytes[4097]={};U32 read=0;CHECK(ReadFile(file,bytes,sizeof(bytes),&read,nullptr));CloseHandle(file);
        GenericLimits v=GenericParse(bytes,read);CHECK(v.valid && v.requests==expectedRequests[i] && v.models==expectedModels[i]);
        CHECK(WideMax(512u,v.models+v.retained)==expectedWork[i]);
    }
}
extern "C" void AudioSetupTestMain(){
    SetErrorMode(3u);g_limits=GenericValidate(g_limits);MapNativeImage();CHECK(WideStartupStorage());
    vehicle=Alloc(0x700u);controller=Alloc(0x300u);Write32(vehicle+0x5ACu,191u);
    Write32(controller+0x10u,191u);Write32(controller+0xBCu,0xA001u);Write32(controller+0xC0u,0xA002u);
    CHECK(AudioSetupSignature(false));CHECK(!AudioSetupInstall() && g_audioSetupStatus==-1);
    for(const auto& s:kAudioSetupSites){UPtr at=Address(s.address);U8 v=Read8(at);*reinterpret_cast<U8*>(at)^=1u;CHECK(!AudioSetupSignature(false));*reinterpret_cast<U8*>(at)=v;}
    g_audioSetupStatus=1;Decisions();Abi();Profiles();
    Say("PASS audio setup decisions/ring/20 bridge ABI invocations checks=");Number(g_checks);Say("\n");ExitProcess(0);
}

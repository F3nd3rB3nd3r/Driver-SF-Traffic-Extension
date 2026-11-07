typedef unsigned char U8;typedef unsigned int U32;typedef unsigned int UPtr;typedef unsigned long DWORD;typedef int BOOL;
extern "C" int _fltused=0;
extern "C" __declspec(dllimport) void __stdcall ExitProcess(U32);
extern "C" __declspec(dllimport) void* __stdcall VirtualAlloc(void*,U32,U32,U32);
extern "C" __declspec(dllimport) BOOL __stdcall FlushInstructionCache(void*,const void*,U32);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentProcess();
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(DWORD);
extern "C" __declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
extern "C" __declspec(noinline) void* __cdecl memset(void* p,int v,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=static_cast<U8>(v);return p;}
extern "C" __declspec(noinline) void* __cdecl memcpy(void* p,const void* q,U32 n){for(U32 i=0;i<n;++i)reinterpret_cast<volatile U8*>(p)[i]=reinterpret_cast<const U8*>(q)[i];return p;}
static void Say(const char* s){U32 n=0;while(s[n])++n;DWORD written=0;WriteFile(GetStdHandle(static_cast<DWORD>(-11)),s,n,&written,nullptr);}
static void CheckAt(bool v,U32 line){if(v)return;Say("parked fixture failure at line ");char s[12]={};U32 n=0;
    do{s[n++]=static_cast<char>('0'+line%10);line/=10;}while(line);for(U32 i=0;i<n/2;++i){char c=s[i];s[i]=s[n-i-1];s[n-i-1]=c;}Say(s);Say("\n");ExitProcess(1);}
#define Check(v) CheckAt((v),__LINE__)
static long InterlockedIncrement(volatile long* p){return ++*p;}
static long InterlockedExchange(volatile long* p,long n){long old=*p;*p=n;return old;}
static long InterlockedCompareExchange(volatile long* p,long n,long old){long v=*p;if(v==old)*p=n;return v;}
static U32 Read32(UPtr p){return *reinterpret_cast<volatile U32*>(p);}static U8 Read8(UPtr p){return *reinterpret_cast<volatile U8*>(p);}
static void Write32(UPtr p,U32 v){*reinterpret_cast<volatile U32*>(p)=v;}
static bool Readable(UPtr p,U32){return p!=0u;}
struct CpuRegs {U32 edi,esi,ebp,savedEsp,ebx,edx,ecx,eax,eflags;};
static U32 fatals=0,buildEvents=0,eventCount=0;static bool failAllocation=false;
static UPtr code=0,entry=0,stackTop=0,hostEsp=0,ownerArg=0,rootPointer=0;
static U32 sourceCount=0,sourceIds[151]={},categories[151]={},typeArg=0,seedArg=0,resultEax=0;
static U32 afterRegs[4]={},afterEsp=0;static unsigned short afterFp=0;
static U8 root[0x800]={},descriptor[0x20]={};static U32 cloneNodes[151][3]={};
static U32 oldPicks[26][53]={};
static void ParkFatal(const char*,UPtr,U32){++fatals;}
static void* ParkAllocate(U32 n){return failAllocation?nullptr:VirtualAlloc(nullptr,n,0x3000u,4u);}
static UPtr ParkNativeDescribe(UPtr,U32 id){
    U32 i=0;while(i<sourceCount && sourceIds[i]!=id)++i;Check(i<sourceCount);
    descriptor[0x1B]=static_cast<U8>(categories[i]);return reinterpret_cast<UPtr>(descriptor);
}
static void ParkRunBuilder(UPtr owner){typedef void (__thiscall* Fn)(void*);reinterpret_cast<Fn>(code)(reinterpret_cast<void*>(owner));}
static void ParkBuildEvent(UPtr,U32 count,U32,U32,const U32*){++buildEvents;eventCount=count;}
#include "../src/V29sParked150Native.inc"
#include "../src/V29sParked150Layout.inc"
#include "../src/V29sParked150Core.inc"
extern "C" UPtr g_testTrampoline=0;
extern "C" __declspec(naked) void TestBucketBridge(){__asm {
    pushfd
    pushad
    mov ebp,esp
    sub esp,528
    and esp,0FFFFFFF0h
    fxsave [esp]
    cld
    push ebp
    call ParkBucketCheck
    add esp,4
    fxrstor [esp]
    mov esp,ebp
    popad
    popfd
    jmp dword ptr [g_testTrampoline]
}}
// Isolated stack is fully committed. Preserve the native __chkstk stack contract.
extern "C" __declspec(naked) void Probe(){__asm {
    push ecx
    lea ecx,[esp+8]
    sub ecx,eax
    mov eax,esp
    mov esp,ecx
    mov ecx,[eax]
    mov eax,[eax+4]
    push eax
    ret
}}
static UPtr __cdecl Factory(){return rootPointer;}
static void __fastcall Clone(void*,void*,UPtr header){
    const UPtr sentinel=header+0x264u;
    Write32(header+0x268u,sourceCount?reinterpret_cast<UPtr>(cloneNodes):sentinel);
    for(U32 i=0;i<sourceCount;++i){cloneNodes[i][2]=sourceIds[i];cloneNodes[i][1]=i+1u<sourceCount?reinterpret_cast<UPtr>(&cloneNodes[i+1u]):sentinel;}
}
static UPtr __fastcall Describe(void* p,void*,U32 id){return ParkNativeDescribe(reinterpret_cast<UPtr>(p),id);}
extern "C" __declspec(naked) void InvokeBuild(){__asm {
    pushad
    mov hostEsp,esp
    fninit
    mov esp,stackTop
    mov ebp,77777777h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,ownerArg
    call dword ptr [entry]
    mov afterRegs,ebx
    mov afterRegs+4,esi
    mov afterRegs+8,edi
    mov afterRegs+12,ebp
    mov afterEsp,esp
    fnstsw afterFp
    fninit
    mov esp,hostEsp
    popad
    ret
}}
extern "C" __declspec(naked) void InvokePick(){__asm {
    pushad
    mov hostEsp,esp
    mov esp,stackTop
    mov ebp,77777777h
    mov ebx,44444444h
    mov esi,55555555h
    mov edi,66666666h
    mov ecx,ownerArg
    push seedArg
    push typeArg
    call dword ptr [entry]
    mov resultEax,eax
    mov afterRegs,ebx
    mov afterRegs+4,esi
    mov afterRegs+8,edi
    mov afterRegs+12,ebp
    mov afterEsp,esp
    mov esp,hostEsp
    popad
    ret
}}
static void Abi(){Check(afterEsp==stackTop);Check(afterRegs[0]==0x44444444u && afterRegs[1]==0x55555555u && afterRegs[2]==0x66666666u && afterRegs[3]==0x77777777u);Check(!(afterFp&0x3800u));}
static void Fill(bool expanded,bool full){
    ParkFillImage(code,code,reinterpret_cast<UPtr>(&rootPointer),reinterpret_cast<UPtr>(&Probe),reinterpret_cast<UPtr>(&Factory),
        reinterpret_cast<UPtr>(&Clone),full?reinterpret_cast<UPtr>(&ParkDescribe):reinterpret_cast<UPtr>(&Describe),
        full?reinterpret_cast<UPtr>(&ParkAppend):code+0x1000u,full?reinterpret_cast<UPtr>(&TestBucketBridge):0u,expanded);
    g_testTrampoline=code+0x300u;Check(FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(code),0x2000u));
}
static void Source(U32 n,bool concentrated,U32 bias=0u){sourceCount=n;for(U32 i=0;i<n;++i){sourceIds[i]=100u+i+bias;categories[i]=concentrated?0u:i%25u;}}
static void CheckFull(U32 n,bool concentrated){
    Source(n,concentrated);Fill(true,true);entry=reinterpret_cast<UPtr>(&ParkRebuild);InvokeBuild();Abi();Check(!fatals && eventCount==n);
    Check(Read32(ownerArg+0x24D0u)==n);U32 index=0,expected[150]={};
    for(U32 category=0;category<25u;++category){Check(Read32(ownerArg+0x24D4u+category*8u)==index);U32 count=0;
        for(U32 i=0;i<n;++i)if(categories[i]==category)expected[index+count++]=sourceIds[i];
        Check(Read32(ownerArg+0x24D8u+category*8u)==count);index+=count;}
    Check(index==n);ParkLock();ParkSlot* slot=ParkFind(ownerArg,false);Check(ParkSlotGuard(slot));
    for(U32 i=0;i<n;++i)Check(slot->ids[i]==expected[i]);ParkUnlock();
    for(U32 i=0;i<n && i<50u;++i)Check(Read32(ownerArg+0x2408u+i*4u)==expected[i]);
    entry=reinterpret_cast<UPtr>(&ParkPick);
    for(U32 category=0;category<26u;++category){typeArg=category?0x709u+category:0x709u;
        U32 begin=category?Read32(ownerArg+0x24D4u+(category-1u)*8u):0u;
        U32 count=category?Read32(ownerArg+0x24D8u+(category-1u)*8u):n;
        for(U32 seed=0;seed<151u;++seed){seedArg=seed;InvokePick();Abi();Check(resultEax==(count?expected[begin+seed%count]:0xFFFFFFFFu));}
        seedArg=0xFFFFFFFFu;InvokePick();Abi();Check(resultEax==(count?expected[begin+seedArg%count]:0xFFFFFFFFu));
    }
    Check(!fatals);Check(Read32(ownerArg+0x2600u)==0xA5A5A5A5u);
}
extern "C" void __stdcall ParkedTestMain(){
    code=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x2000u,0x3000u,0x40u));Check(code);
    const UPtr mockBuild=code+0x1400u,mockPick=code+0x1700u;
    memcpy(reinterpret_cast<void*>(mockBuild),kParkBuildNative,sizeof(kParkBuildNative));
    memcpy(reinterpret_cast<void*>(mockPick),kParkPickNative,sizeof(kParkPickNative));Check(ParkOriginalMatches(mockBuild,mockPick));
    reinterpret_cast<U8*>(mockBuild)[40]^=1u;Check(!ParkOriginalMatches(mockBuild,mockPick));reinterpret_cast<U8*>(mockBuild)[40]^=1u;
    ParkJump(mockBuild,reinterpret_cast<UPtr>(&ParkRebuild),5u);ParkJump(mockPick,reinterpret_cast<UPtr>(&ParkPick),9u);
    Check(Read8(mockBuild)==0xE9u && mockBuild+5u+Read32(mockBuild+1u)==reinterpret_cast<UPtr>(&ParkRebuild));
    Check(Read8(mockPick+8u)==0x90u && mockPick+5u+Read32(mockPick+1u)==reinterpret_cast<UPtr>(&ParkPick));
    memcpy(reinterpret_cast<void*>(mockBuild),kParkBuildNative,5u);memcpy(reinterpret_cast<void*>(mockPick),kParkPickNative,9u);Check(ParkOriginalMatches(mockBuild,mockPick));
    memcpy(reinterpret_cast<void*>(code+0x1000u),kParkCopyNative,sizeof(kParkCopyNative));
    memcpy(reinterpret_cast<void*>(code+0x1100u),kParkPickNative,sizeof(kParkPickNative));
    rootPointer=reinterpret_cast<UPtr>(root);Write32(rootPointer+0x214u,1u);Write32(rootPointer+0x780u,1u);
    const UPtr stack=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x20000u,0x3000u,4u));Check(stack);stackTop=stack+0x18000u;
    ownerArg=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,4u));Check(ownerArg);memset(reinterpret_cast<void*>(ownerArg),0xA5,0x10000u);
    for(U32 n=18;n<=50;n+=32){Source(n,false);Fill(false,false);entry=code;InvokeBuild();Abi();Check(Read32(ownerArg+0x24D0u)==n);
        for(U32 c=0;c<26u;++c){typeArg=0x709u+c;for(U32 seed=0;seed<53u;++seed){seedArg=seed;entry=code+0x1100u;InvokePick();Abi();oldPicks[c][seed]=resultEax;}}
        CheckFull(n,false);for(U32 c=0;c<26u;++c){typeArg=0x709u+c;for(U32 seed=0;seed<53u;++seed){seedArg=seed;entry=reinterpret_cast<UPtr>(&ParkPick);InvokePick();Check(resultEax==oldPicks[c][seed]);}}
    }
    Source(51,false);Fill(false,false);entry=code;InvokeBuild();Abi();Check(Read32(ownerArg+0x24D0u)!=51u);
    Say("native stock51 candidate-count overwrite reproduced\n");
    Source(150,false);Fill(true,false);entry=code;InvokeBuild();Abi();Check(Read32(ownerArg+0x24D0u)!=150u);
    Say("bucket-only expansion retains candidate-cache overwrite\n");
    memset(reinterpret_cast<void*>(ownerArg),0xA5,0x10000u);
    for(U32 n=0;n<=150u;++n){CheckFull(n,false);CheckFull(n,true);}
    Say("all counts0..150 native builder and category/general picker passed\n");
    const UPtr savedOwner=ownerArg;
    for(U32 n=0;n<40u;++n){ownerArg=reinterpret_cast<UPtr>(VirtualAlloc(nullptr,0x10000u,0x3000u,4u));Check(ownerArg);memset(reinterpret_cast<void*>(ownerArg),0xA5,0x10000u);CheckFull(3,false);}
    ownerArg=savedOwner;CheckFull(150,false);Check(g_parkOwners==41 && !fatals && g_parkExtraSelections>0);
    // The same owner address rebuilds without retaining old picks or model IDs.
    Source(18,true,1000u);Fill(true,true);ParkRebuild(reinterpret_cast<void*>(ownerArg),nullptr);
    Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0x709u,0u)==1100u);
    Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0u,0u)==0xFFFFFFFFu);
    Write32(ownerArg+0x24D0u,0u);Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0x709u,0u)==0xFFFFFFFFu && !fatals);
    ParkRebuild(reinterpret_cast<void*>(ownerArg),nullptr);Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0x709u,0u)==1100u && !fatals);
    Say("same-owner rebuild and41 private owners passed\n");
    // Invalid inputs are checked directly because production fatal exits never return.
    descriptor[0x1B]=0;categories[0]=25;Check(!ParkDescribe(reinterpret_cast<void*>(1u),nullptr,sourceIds[0]));Check(fatals==1);categories[0]=0;
    CpuRegs reg={};reg.savedEsp=stackTop-4u;reg.eax=stackTop+0x288u;reg.ecx=150u;ParkBucketCheck(&reg);Check(fatals==2);
    reg.ecx=0;reg.eax+=4u;ParkBucketCheck(&reg);Check(fatals==3);
    U32 out=0u;Check(!ParkAppend(&out,sourceIds,sourceIds+151,ownerArg+0x2408u,0,0,0));Check(fatals==4 && !out);
    ParkLock();ParkSlot* s=ParkFind(ownerArg,false);s->busy=1u;s->count=150u;Write32(ownerArg+0x24D0u,150u);ParkUnlock();
    Check(!ParkAppend(&out,sourceIds,sourceIds+1,ownerArg+0x2408u,0,0,0));Check(fatals==5 && !out && s->count==150u);
    Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0x709u,0u)==0xFFFFFFFFu);Check(g_parkConcurrent==1 && fatals==5);
    s->busy=0u;s->guardB=0;Check(ParkPick(reinterpret_cast<void*>(ownerArg),nullptr,0x709u,0)==0xFFFFFFFFu);Check(fatals==6);
    failAllocation=true;ParkRebuild(reinterpret_cast<void*>(ownerArg+0x8000u),nullptr);Check(fatals==7);
    Say("bounds/category/guard/allocation refusals passed\n");
    Say("parked150 native checks passed\n");ExitProcess(0);
}

// Read-only frame draw-queue diagnostic for the exact retail Driver.exe.
// The game-thread hooks only copy counters; Monitor formats log lines.
static const U32 kVDrawQueueEvents=8192u;
struct DiagVDrawQueueEvent {
    volatile long seq;
    U32 tick,manager,queue,count,capacity;
};
static DiagVDrawQueueEvent g_vDrawQueueEvents[kVDrawQueueEvents]={};
static volatile long g_vDrawQueueNext=0,g_vDrawQueueHook=0;
static volatile long g_vDrawQueueJobFull=0,g_vDrawQueueTrapEqual=0;
static volatile long g_vDrawQueueTrapOver=0;
static U32 g_vDrawQueueDrained=0u,g_vDrawQueueMissed=0u;
static U32 g_vDrawQueueLastJobFull=0u,g_vDrawQueueLastTrapEqual=0u;
static U32 g_vDrawQueueLastTrapOver=0u;
static DiagFDetour g_vDrawQueueDetours[3]={};
static UPtr g_vDrawQueueTramps[3]={};

extern "C" __declspec(noinline) void __cdecl DiagVDrawQueueRecordFrame(UPtr manager) {
    if(!manager)return;
    const UPtr queue=Read32(manager+4u);
    if(!queue)return;
    const U32 count=Read32(queue);
    const U32 capacity=Read32(queue+4u);
    const long sequence=InterlockedIncrement(&g_vDrawQueueNext);
    if(sequence<=0)return;
    DiagVDrawQueueEvent& e=g_vDrawQueueEvents[(static_cast<U32>(sequence)-1u)%kVDrawQueueEvents];
    e.tick=GetTickCount()-g_diagStartTick;
    e.manager=manager;e.queue=queue;e.count=count;e.capacity=capacity;
    InterlockedExchange(&e.seq,sequence);
}
extern "C" __declspec(naked) void DiagVDrawQueueFlipBridge() {
    __asm {
        pushfd
        pushad
        push dword ptr [esp+24] // original ECX is the queue manager
        call DiagVDrawQueueRecordFrame
        add esp,4
        popad
        popfd
        jmp dword ptr [g_vDrawQueueTramps]
    }
}
extern "C" __declspec(naked) void DiagVDrawQueueJobBridge() {
    __asm {
        pushfd
        cmp esi,4000h
        jb q_job_not_full
        lock inc dword ptr [g_vDrawQueueJobFull]
    q_job_not_full:
        popfd
        jmp dword ptr [g_vDrawQueueTramps+4]
    }
}
extern "C" __declspec(naked) void DiagVDrawQueueTrapBridge() {
    __asm {
        pushfd
        pushad
        mov eax,dword ptr [esp+28] // original EAX is the queue header
        mov ecx,dword ptr [eax]
        cmp ecx,dword ptr [eax+4]
        jne q_trap_not_equal
        lock inc dword ptr [g_vDrawQueueTrapEqual]
        jmp q_trap_done
    q_trap_not_equal:
        jb q_trap_done
        lock inc dword ptr [g_vDrawQueueTrapOver]
    q_trap_done:
        popad
        popfd
        jmp dword ptr [g_vDrawQueueTramps+8]
    }
}
static bool DiagVDrawQueueInstall() {
    // Copied instructions contain no relative control flow. Each site is
    // checked against the exact executable before modifying any code.
    const U8 flip[]={0x8B,0x01,0x48,0x83,0xE0,0x01};
    const U8 job[]={0x81,0xFE,0x00,0x40,0x00,0x00};
    const U8 trap[]={0x8B,0x08,0x89,0x54,0x24,0x0C,0x3B,0x48,0x04};
    if(DSF150_Status!=1)return false;
    const struct {UPtr site;const U8* bytes;U32 len;UPtr bridge;} sites[]={
        {0x00BB5780u,flip,sizeof(flip),reinterpret_cast<UPtr>(&DiagVDrawQueueFlipBridge)},
        {0x006C21F9u,job,sizeof(job),reinterpret_cast<UPtr>(&DiagVDrawQueueJobBridge)},
        {0x00BB5A8Cu,trap,sizeof(trap),reinterpret_cast<UPtr>(&DiagVDrawQueueTrapBridge)}
    };
    for(U32 i=0u;i<3u;++i) {
        if(!Readable(Address(sites[i].site),sites[i].len))return false;
        for(U32 j=0u;j<sites[i].len;++j)
            if(*reinterpret_cast<volatile U8*>(Address(sites[i].site)+j)!=sites[i].bytes[j])
                return false;
    }
    for(U32 i=0u;i<3u;++i) {
        if(DiagFInstallDetour(g_vDrawQueueDetours[i],sites[i].site,sites[i].bytes,
            sites[i].len,sites[i].bridge,g_vDrawQueueTramps[i]))continue;
        while(i>0u) {--i;DiagFRestoreDetour(g_vDrawQueueDetours[i],g_vDrawQueueTramps[i]);}
        return false;
    }
    return true;
}
static void DiagVDrawQueuePrint(U32 loops) {
    static U32 frames=0u,peak=0u,last=0u,capacity=0u,atCap=0u,overCap=0u;
    static U32 high12k=0u,high15k=0u,lastTick=0u,lastQueue=0u;
    static U32 badCapacity=0u;
    const U32 written=static_cast<U32>(g_vDrawQueueNext);
    if(written>g_vDrawQueueDrained+kVDrawQueueEvents) {
        g_vDrawQueueMissed+=written-g_vDrawQueueDrained-kVDrawQueueEvents;
        g_vDrawQueueDrained=written-kVDrawQueueEvents;
    }
    while(g_vDrawQueueDrained<written) {
        const U32 sequence=g_vDrawQueueDrained+1u;
        const DiagVDrawQueueEvent& e=g_vDrawQueueEvents[(sequence-1u)%kVDrawQueueEvents];
        if(static_cast<U32>(e.seq)!=sequence)break;
        g_vDrawQueueDrained=sequence;
        ++frames;last=e.count;capacity=e.capacity;lastTick=e.tick;lastQueue=e.queue;
        if(e.count>peak)peak=e.count;
        if(e.count>=12000u)++high12k;
        if(e.count>=15000u)++high15k;
        if(e.capacity!=0x4000u)++badCapacity;
        if(e.capacity && e.count==e.capacity)++atCap;
        if(e.capacity && e.count>e.capacity)++overCap;
    }
    if(loops%5u!=0u)return;
    const U32 jobFull=static_cast<U32>(g_vDrawQueueJobFull);
    const U32 trapEqual=static_cast<U32>(g_vDrawQueueTrapEqual);
    const U32 trapOver=static_cast<U32>(g_vDrawQueueTrapOver);
    char line[420]={};char* q=line;const char* end=line+sizeof(line)-1;
    AppendText(q,"DSFV29s_DRAW_QUEUE tick=",end);
    DiagFAppendU(q,GetTickCount()-g_diagStartTick,end);
    DiagFAppendKeyU(q," frames=",frames,end);
    DiagFAppendKeyU(q," peak=",peak,end);
    DiagFAppendKeyU(q," last=",last,end);
    DiagFAppendKeyU(q," capacity=",capacity,end);
    DiagFAppendKeyU(q," atCap=",atCap,end);
    DiagFAppendKeyU(q," overCap=",overCap,end);
    DiagFAppendKeyU(q," high12k=",high12k,end);
    DiagFAppendKeyU(q," high15k=",high15k,end);
    DiagFAppendKeyU(q," jobFull=",jobFull-g_vDrawQueueLastJobFull,end);
    DiagFAppendKeyU(q," trapEqual=",trapEqual-g_vDrawQueueLastTrapEqual,end);
    DiagFAppendKeyU(q," trapOver=",trapOver-g_vDrawQueueLastTrapOver,end);
    DiagFAppendKeyU(q," badCapacity=",badCapacity,end);
    DiagFAppendKeyU(q," missed=",g_vDrawQueueMissed,end);
    DiagFAppendKeyHex(q," queue=",lastQueue,end);
    DiagFAppendKeyU(q," frameTick=",lastTick,end);
    AppendText(q,"\n",end);*q=0;DiagPrint(line);
    frames=peak=atCap=overCap=high12k=high15k=badCapacity=0u;
    g_vDrawQueueLastJobFull=jobFull;
    g_vDrawQueueLastTrapEqual=trapEqual;
    g_vDrawQueueLastTrapOver=trapOver;
}

static void (*g_wideFailureSink)(const char*)=nullptr;
static void WideAllocationFailure(const char* reason){if(g_wideFailureSink)g_wideFailureSink(reason);OutputDebugStringA(reason);ExitProcess(0x29F3u);}
static void* WideAllocate(U32 bytes){
    if(!bytes || bytes>0x7FFFFFFFu)WideAllocationFailure("Unlimiter: allocation byte overflow.\n");
    void* p=VirtualAlloc(nullptr,bytes,0x3000u,4u);if(!p)WideAllocationFailure("Unlimiter: configured storage allocation failed.\n");return p;
}
// Entries never move. Engine pointers into sidecars remain valid until exit.
template<class T> struct WideDirectory {
    struct Node {Node* volatile next;T data;};Node* volatile first;Node* last;volatile long count;
    T& Append(){if(count==0x7FFFFFFF)WideAllocationFailure("Unlimiter: owner directory counter exhausted.\n");Node* n=static_cast<Node*>(WideAllocate(sizeof(Node)));WideInitialize(n->data);if(last)last->next=n;else first=n;last=n;InterlockedIncrement(&count);return n->data;}
    T& operator[](U32 index){while(index>=static_cast<U32>(count))Append();Node* n=first;while(index--)n=n->next;return n->data;}
    struct Iterator {Node* node;T& operator*()const{return node->data;}Iterator& operator++(){node=node->next;return *this;}bool operator!=(const Iterator& b)const{return node!=b.node;}};
    Iterator begin(){return {first};}Iterator end(){return {nullptr};}
};
struct WideScratchRecord {WideScratchRecord* next;U32 thread,bytes,busy;void* data;};
static WideScratchRecord* g_wideScratch=nullptr;static volatile long g_wideScratchLock=0;
struct WideScratchLease {
    WideScratchRecord* record;void* data;
    explicit WideScratchLease(U32 bytes):record(nullptr),data(nullptr){
        while(InterlockedExchange(&g_wideScratchLock,1)){}const U32 thread=GetCurrentThreadId();
        for(WideScratchRecord* p=g_wideScratch;p;p=p->next)if(p->thread==thread && !p->busy && p->bytes>=bytes){record=p;break;}
        if(!record){record=static_cast<WideScratchRecord*>(WideAllocate(sizeof(WideScratchRecord)));record->thread=thread;record->bytes=bytes;record->data=WideAllocate(bytes);record->next=g_wideScratch;g_wideScratch=record;}
        record->busy=1;data=record->data;InterlockedExchange(&g_wideScratchLock,0);
        memset(data,0,bytes);
    }
    ~WideScratchLease(){while(InterlockedExchange(&g_wideScratchLock,1)){}record->busy=0;InterlockedExchange(&g_wideScratchLock,0);}
    WideScratchLease(const WideScratchLease&)=delete;WideScratchLease& operator=(const WideScratchLease&)=delete;
};
// Reject a frame that cannot fit the current thread's real reserved stack.
// Keep native callees' stack headroom, then probe each page before SUB ESP.
struct WideMBI {void* base;void* allocation;U32 allocationProtect,size,state,protect,type;};
static void __cdecl WideCheckStack(U32 sp,U32 bytes){
    WideMBI m={};if(VirtualQuery(reinterpret_cast<void*>(sp),&m,sizeof(m))!=sizeof(m) ||
        sp<reinterpret_cast<UPtr>(m.allocation) || sp-reinterpret_cast<UPtr>(m.allocation)<=bytes+0x20000u)
        WideAllocationFailure("Unlimiter: configured native frame exceeds this thread's stack reservation.\n");
}
extern "C" __declspec(naked) void WideProbeStack(){
    __asm {
        pushfd
        pushad
        mov ebp,esp
        sub esp,528
        and esp,0FFFFFFF0h
        fxsave [esp]
        push dword ptr [ebp+28]
        lea ecx,[ebp+36]
        push ecx
        call WideCheckStack
        add esp,8
        fxrstor [esp]
        mov esp,ebp
        // Saved EAX is the requested frame size; probe below our lowest frame.
        mov ecx,dword ptr [ebp+28]
        mov eax,esp
    wide_probe_page:
        cmp ecx,1000h
        jb wide_probe_tail
        sub eax,1000h
        test dword ptr [eax],eax
        sub ecx,1000h
        jmp wide_probe_page
    wide_probe_tail:
        sub eax,ecx
        test dword ptr [eax],eax
        popad
        popfd
        ret
    }
}

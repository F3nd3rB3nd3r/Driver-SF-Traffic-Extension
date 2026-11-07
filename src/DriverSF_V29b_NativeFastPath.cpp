// Driver: San Francisco V29: Admission150 + 512-record native object-pool expansion (EXPERIMENTAL)
// Pre-init graphics descriptor allocation is 150; PACKAGE ZERO resource
// admission metadata target is 150, with first-pass handler and map redirection.
// This attempts first-pass redirects for handler arrays and the completed map. expanded 150-ID request writer/reader/stack frame,
// and 150 drawable residents remain UNVERIFIED. HIGH-RISK EXPERIMENT.
// Unverified executable / late initialization -> no admission patch.
//
// Intentionally no Windows SDK or C runtime dependency: this file is also
// cross-built with clang-cl + lld-link for PE32 using generated COFF import
// objects. MSVC Win32 works with kernel32.lib normally.
#include "Chapter7Preload.h"
#ifndef DSF_CHAPTER7_PRELOAD_COUNT
#define DSF_CHAPTER7_PRELOAD_COUNT 150
#endif
#ifndef DSF_GRAPHICS_ADMISSION_TARGET
#define DSF_GRAPHICS_ADMISSION_TARGET 150
#endif
#if defined(DSFV29C_DISABLE_PRELOAD)
#error This 150-ID build cannot disable the chapter preload
#endif
static_assert(DSF_CHAPTER7_PRELOAD_COUNT == 45 || DSF_CHAPTER7_PRELOAD_COUNT == 150,
              "This experiment supports only the verified 45 or 150 Chapter 7 rosters");
static_assert(DSF_GRAPHICS_ADMISSION_TARGET == 150 || DSF_GRAPHICS_ADMISSION_TARGET == 160,
              "Only 150 and 160 descriptor/admission targets are audited here");
static_assert(sizeof(kChapter7AllPositiveIds) / sizeof(kChapter7AllPositiveIds[0]) == 150,
              "The installed Chapter 7 positive roster must contain 150 models");
constexpr bool Check150Roster() {
    for (unsigned i = 0; i < 150; ++i) {
        if (kChapter7AllPositiveIds[i] > 0xFFFFu) return false;
        for (unsigned j = i + 1; j < 150; ++j)
            if (kChapter7AllPositiveIds[i] == kChapter7AllPositiveIds[j]) return false;
    }
    for (unsigned i = 0; i < 18; ++i) {
        bool found = false;
        for (unsigned j = 0; j < 150; ++j)
            if (kChapter7StockIds[i] == kChapter7AllPositiveIds[j]) found = true;
        if (!found) return false;
    }
    return true;
}
static_assert(Check150Roster(), "150-ID roster is invalid or omits stock models");

// Test whether request order affects the later civilian vehicle visual delay.
// Keep the baseline roster intact; only the ordered build defines this flag.
#if defined(DSF_CHAPTER7_ORDERED_ROSTER)
static_assert(DSF_CHAPTER7_PRELOAD_COUNT == 150,
              "The ordered-roster experiment is defined only for 150 requests");
struct OrderedChapter7Roster {
    unsigned ids[150]{};
    constexpr OrderedChapter7Roster() {
        for (unsigned i = 0; i < 150; ++i) ids[i] = kChapter7AllPositiveIds[i];
        for (unsigned i = 1; i < 150; ++i) {
            const unsigned value = ids[i];
            unsigned j = i;
            while (j > 0 && ids[j - 1] > value) {
                ids[j] = ids[j - 1];
                --j;
            }
            ids[j] = value;
        }
    }
};
constexpr OrderedChapter7Roster kChapter7OrderedRoster{};
constexpr bool CheckOrdered150Roster() {
    for (unsigned i = 0; i < 150; ++i) {
        if (kChapter7OrderedRoster.ids[i] != kChapter7AllPositiveIds[i]) {
            bool found = false;
            for (unsigned j = 0; j < 150; ++j)
                if (kChapter7OrderedRoster.ids[i] == kChapter7AllPositiveIds[j]) found = true;
            if (!found) return false;
        }
        if (i && kChapter7OrderedRoster.ids[i - 1] >= kChapter7OrderedRoster.ids[i]) return false;
    }
    return true;
}
static_assert(CheckOrdered150Roster(), "Ordered roster must contain the same 150 unique IDs");
constexpr const unsigned* kChapter7Active150Ids = kChapter7OrderedRoster.ids;
#else
constexpr const unsigned* kChapter7Active150Ids = kChapter7AllPositiveIds;
#endif

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;
typedef unsigned int   UPtr; // target x86 only
typedef int            BOOL;
typedef void*          HANDLE;
typedef void*          HMODULE;
typedef const wchar_t* LPCWSTR;
typedef unsigned long  DWORD; // 32-bit on Windows

typedef DWORD (__stdcall* ThreadProc)(void*);
extern "C" {
__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(LPCWSTR);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void*, U32, DWORD, DWORD*);
__declspec(dllimport) U32 __stdcall VirtualQuery(const void*, void*, U32);
__declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
__declspec(dllimport) BOOL __stdcall IsDebuggerPresent();
__declspec(dllimport) void __stdcall Sleep(DWORD);
__declspec(dllimport) HANDLE __stdcall CreateThread(void*, U32, ThreadProc, void*, DWORD, DWORD*);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE);
__declspec(dllimport) BOOL __stdcall DisableThreadLibraryCalls(HMODULE);
__declspec(dllimport) void* __stdcall VirtualAlloc(void*, U32, U32, DWORD);
__declspec(dllimport) BOOL __stdcall VirtualFree(void*, U32, U32);
__declspec(dllimport) BOOL __stdcall FlushInstructionCache(HANDLE, const void*, U32);
__declspec(dllimport) HANDLE __stdcall GetCurrentProcess();
__declspec(dllimport) void __stdcall ExitProcess(U32);
__declspec(dllimport) BOOL __stdcall GetProcessMemoryInfo(HANDLE, void*, DWORD); // PSAPI.dll
__declspec(dllimport) long __stdcall InterlockedIncrement(volatile long*);
__declspec(dllimport) long __stdcall InterlockedDecrement(volatile long*);
__declspec(dllimport) long __stdcall InterlockedExchange(volatile long*, long);
__declspec(dllimport) long __stdcall InterlockedCompareExchange(volatile long*, long, long);
__declspec(dllimport) U32 __stdcall GetTempPathA(U32, char*);
__declspec(dllimport) HANDLE __stdcall CreateFileA(const char*, U32, U32, void*, U32, U32, HANDLE);
__declspec(dllimport) BOOL __stdcall WriteFile(HANDLE, const void*, U32, U32*, void*);
__declspec(dllimport) U32 __stdcall GetCurrentProcessId();
__declspec(dllimport) U32 __stdcall GetCurrentThreadId();
__declspec(dllimport) DWORD __stdcall GetTickCount();
}

extern "C" void* _ReturnAddress();
#pragma intrinsic(_ReturnAddress)


// Compiler-generated aggregate zeroing must not introduce a CRT dependency.
extern "C" __declspec(noinline) void* __cdecl memset(void* destination, int value, unsigned size) {
    volatile U8* bytes = reinterpret_cast<volatile U8*>(destination);
    for (unsigned i = 0; i < size; ++i) bytes[i] = static_cast<U8>(value);
    return destination;
}

// clang-cl may lower local aggregate initialization to memcpy even at /Od.
extern "C" __declspec(noinline) void* __cdecl memcpy(void* dst, const void* src, unsigned n) {
    volatile U8* d=reinterpret_cast<volatile U8*>(dst);
    const volatile U8* s=reinterpret_cast<const volatile U8*>(src);
    for (unsigned i=0;i<n;++i) d[i]=s[i];
    return dst;
}

struct MemoryBasicInformation32 {
    void* base;
    void* allocationBase;
    U32 allocationProtect;
    U32 regionSize;
    U32 state;
    U32 protect;
    U32 type;
};
static_assert(sizeof(void*) == 4, "Only the 32-bit Driver.exe is supported");
static_assert(sizeof(MemoryBasicInformation32) == 28, "Incorrect Win32 VQ structure");

// Observable exports: visible in x32dbg under module symbols/exports.
// Status: 0 not started; 1 source config patched; 2 graphics pool verified
// at 150; -1 unsupported image; -2 already initialized; -3 bad stock config;
// -4 VirtualProtect failure; -5 failed post-write validation;
// -6 failed to restore protection; -7 pool mismatch after initialization.
extern "C" __declspec(dllexport) volatile long DSF150_Status = 0;
extern "C" __declspec(dllexport) volatile long DSFV29p_ConfigTarget = DSF_GRAPHICS_ADMISSION_TARGET;
extern "C" __declspec(dllexport) volatile long DSFV29p_ConfigBudgetA =
    DSF_GRAPHICS_ADMISSION_TARGET == 160 ? 0x9400 : 0x8C00;
extern "C" __declspec(dllexport) volatile long DSFV29p_ConfigBudgetB =
    DSF_GRAPHICS_ADMISSION_TARGET == 160 ? 0x49400 : 0x44C00;
extern "C" __declspec(dllexport) volatile long DSF150_GraphicsSlots = 0;
extern "C" __declspec(dllexport) volatile long DSFV28_CityRegionSlots = -1;
extern "C" __declspec(dllexport) volatile long DSFV28_CityGlobalSlots = -1;
// V29 hash-node overflow arena (45,000 original nodes + 45,000 external).
// These are allocation counts, not concurrently live records; freed external
// records are recycled by the unmodified game's normal free list.
extern "C" __declspec(dllexport) volatile long DSFV29_HashHook = 0;
extern "C" __declspec(dllexport) volatile long DSFV29_HashNativeGranted = 0;
extern "C" __declspec(dllexport) volatile long DSFV29_HashExtraGranted = 0;
extern "C" __declspec(dllexport) volatile long DSFV29_HashNativeRemaining = -1;
extern "C" __declspec(dllexport) volatile long DSFV29_HashHeadNonzero = -1;
extern "C" __declspec(dllexport) volatile long DSFV29_HashFaults = 0;


extern "C" __declspec(dllexport) volatile long DSF150_CompletedActive = -1;
extern "C" __declspec(dllexport) volatile long DSF150_CompletedAllocated = -1;
extern "C" __declspec(dllexport) volatile long DSF150_GlobalHandlerCount = -1;
extern "C" __declspec(dllexport) volatile long DSF150_Kind0HandlerCount = -1;
extern "C" __declspec(dllexport) volatile long DSF150_MapHighWater = 0;
extern "C" __declspec(dllexport) volatile long DSF150_HandlerHighWater = 0;
extern "C" __declspec(dllexport) volatile long DSF150_AtOrOver64 = 0;
// V22-specific counters, exported for x32dbg.
// AdmissionResult: 0=not reached; 1=byte patched; 2=initializer verified;
// negatives: -1=unexpected metadata, -2=graphics not ready,
// -3=metadata write failed, -4=initializer count mismatch,
// -5=handler initialized already, -6=object identity unreadable.
extern "C" __declspec(dllexport) volatile long DSFV27B_AdmissionResult = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_HandlerCalls = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_AdmissionBefore = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_AdmissionAfter = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_InitHandlerCount = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_InitKind0Count = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_LiveAdmission = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_HookInstalled = 0;

namespace {
#if defined(DSF_V29S_ARENA_REUSE)
static void LegacyExit(U32 code,const char* reason,UPtr owner=0u,UPtr cursor=0u,U32 active=0u,U32 recycled=0u);
#else
static void LegacyExit(U32 code,const char*,UPtr=0u,UPtr=0u,U32=0u,U32=0u){ExitProcess(code);}
#endif
constexpr UPtr kPreferredBase = 0x00400000u;
constexpr U32 kTimestamp = 0x4EC0D92Bu;
constexpr U32 kSizeOfImage = 0x019C0000u;
constexpr UPtr kGraphicsCtor = 0x00BAD960u;
constexpr UPtr kGraphicsCount = 0x00FD261Cu;
constexpr UPtr kBudgetA = 0x00FD2730u;
constexpr UPtr kBudgetB = 0x00FD2734u;
constexpr UPtr kRecordCount = 0x00FD2738u;
constexpr UPtr kBlockSize = 0x00FD273Cu;
// City Region is the separate pool index 7 in the exact 21-pool table:
// 0xFD2704 row = {flags=0, budgetA=0x4000, budgetB=0x20000, count=0, block=0x400}.
// Do NOT change count=0: its meaning / dynamic region ownership is unknown.
// V28 is strictly an A/B test of the two City Region budget fields.
constexpr UPtr kCityRegionBudgetA = 0x00FD2708u;
constexpr UPtr kCityRegionBudgetB = 0x00FD270Cu;
constexpr UPtr kCityRegionCount = 0x00FD2710u;
constexpr UPtr kCityRegionBlockSize = 0x00FD2714u;
constexpr U32 kOldCityBudgetA = 0x4000u;
constexpr U32 kOldCityBudgetB = 0x20000u;
constexpr U32 kNewCityBudgetA = 0x8000u;
constexpr U32 kNewCityBudgetB = 0x40000u;
constexpr UPtr kGraphicsManager = 0x019B3BE0u;
constexpr UPtr kRootPtr = 0x00FE7228u;
constexpr UPtr kHandlerInit = 0x005A1420u;
constexpr U32 kOldAdmission = 35u;
constexpr U32 kAdmissionStep = DSF_GRAPHICS_ADMISSION_TARGET;
constexpr U32 kPageExecuteReadWrite = 0x40u;
constexpr U32 kMemCommitReserve = 0x3000u;
constexpr U32 kMemRelease = 0x8000u;
constexpr U32 kOldSlots = 35u;
constexpr U32 kNewSlots = DSF_GRAPHICS_ADMISSION_TARGET;
// These preserve at least the original per-descriptor budget and align to 0x400.
constexpr U32 kNewBudgetA = DSF_GRAPHICS_ADMISSION_TARGET == 160 ? 0x9400u : 0x8C00u;
constexpr U32 kNewBudgetB = DSF_GRAPHICS_ADMISSION_TARGET == 160 ? 0x49400u : 0x44C00u;
constexpr U32 kPageReadWrite = 4u;
constexpr U32 kCommitted = 0x1000u;
constexpr U32 kPageGuard = 0x100u;
constexpr U32 kPageNoAccess = 1u;
// IMPORTANT: the stock allocator holds hash buckets at root+0x00000,
// 45,000 x 24-byte nodes at root+0x20000, and [freeHead,nextUnused] at
// root+0x127AC0 / root+0x127AC4. We must never write a node at +0x127AC0.
constexpr UPtr kHashAllocationSite = 0x00BAEDB6u;
constexpr UPtr kHashAllocationResume = 0x00BAEDD0u;
constexpr U32 kHashStartOffset = 0x20000u;
constexpr U32 kHashMetadataOffset = 0x127AC0u;
constexpr U32 kHashCursorOffset = 0x127AC4u;
constexpr U32 kHashNodeStride = 0x18u;
constexpr U32 kHashStockNodes = 45000u;
constexpr U32 kHashExtraNodes = 45000u;
static_assert(kHashStartOffset + kHashStockNodes*kHashNodeStride == kHashMetadataOffset,
              "Unexpected native hash-node arena geometry");
static_assert(kHashExtraNodes*kHashNodeStride == 1080000u, "Unexpected extra memory size");
U8* g_hashExternal = nullptr;  // lifetime: game process, never free while engine references nodes
volatile UPtr g_hashFirstOwner = 0;
bool g_hashPatchInstalled = false;
extern "C" UPtr g_hashAllocationResumeAddress = 0;
UPtr g_base = 0;
#if defined(DSF_V29S_INDEXED_262144)
static bool g_indexedStockVerified=false;
#endif
typedef UPtr (__thiscall* HandlerInitFn)(void*);
HandlerInitFn g_originalHandlerInit = nullptr;
void* g_handlerTrampoline = nullptr;

inline UPtr Address(UPtr preferred) { return g_base + (preferred - kPreferredBase); }
inline U8 Read8(UPtr address) { return *reinterpret_cast<volatile const U8*>(address); }
inline U16 Read16(UPtr address) { return *reinterpret_cast<volatile const U16*>(address); }
inline U32 Read32(UPtr address) { return *reinterpret_cast<volatile const U32*>(address); }
inline void Write32(UPtr address, U32 value) { *reinterpret_cast<volatile U32*>(address) = value; }

bool Readable(UPtr address, U32 bytes) {
    if (!address || bytes > 0x100000u || address + bytes < address) return false;
    MemoryBasicInformation32 m = {};
    if (VirtualQuery(reinterpret_cast<void*>(address), &m, sizeof(m)) < sizeof(m)) return false;
    if (m.state != kCommitted || (m.protect & kPageGuard) ||
        (m.protect & 0xffu) == kPageNoAccess) return false;
    const UPtr end = reinterpret_cast<UPtr>(m.base) + m.regionSize;
    return end >= address && address + bytes <= end;
}

#include "V29sStartupChecks.inc"

// V29b: preserve native fast path until the 45000-node cursor reaches the end.
// The game's ordinary recycled-node path (0xBAEDC7) is deliberately untouched.
// Lookup (0xBAEE00) and unlink/recycle (0xBAEEA0) work on pointers and therefore
// also handle nodes from this permanent external arena. Only the linear fresh-
// node cursor is replaced; bucket indexing and node content writes are native.
extern "C" __declspec(noinline) UPtr __cdecl V29AcquireFreshHashNode(UPtr owner) {
    if (!owner || !g_hashExternal || (g_hashFirstOwner && owner != g_hashFirstOwner)) {
        ++DSFV29_HashFaults;
        OutputDebugStringA("DSFV29b: FATAL unexpected hash root or missing arena; stopping to prevent memory overwrite.\n");
        LegacyExit(0x29u,"hashOwnerOrStorage",owner);
        return 0;
    }
    if (!g_hashFirstOwner) g_hashFirstOwner = owner;
    const UPtr first = owner + kHashStartOffset;
    const UPtr end = owner + kHashMetadataOffset;
    const UPtr unused = Read32(owner + kHashCursorOffset);
    if (unused >= first && unused < end &&
        ((unused - first) % kHashNodeStride) == 0u && unused + kHashNodeStride <= end) {
        Write32(owner + kHashCursorOffset, unused + kHashNodeStride);
        ++DSFV29_HashNativeGranted;
        DSFV29_HashNativeRemaining = static_cast<long>((end - (unused + kHashNodeStride)) / kHashNodeStride);
        return unused;
    }
    if (unused == end) {
        // Atomic reservation prevents two loader threads from receiving one
        // extra node. Native fresh-node behavior remains unchanged.
        const long ordinal = InterlockedIncrement(&DSFV29_HashExtraGranted);
        const U32 idx = static_cast<U32>(ordinal - 1);
        if (idx >= kHashExtraNodes) {
            ++DSFV29_HashFaults;
            OutputDebugStringA("DSFV29b: FATAL external 45000-node arena exhausted.\n");
            LegacyExit(0x29u,"hashExternalExhausted",owner,unused);
            return 0;
        }
        if (idx == 0) {
            OutputDebugStringA("DSFV29b: HASH original 45000 exhausted, external 45000-node arena ACTIVE.\n");
        } else if ((idx % 2048u) == 0u) {
            OutputDebugStringA("DSFV29b: HASH external usage crossed another 2048 nodes.\n");
        }
        return reinterpret_cast<UPtr>(g_hashExternal + idx * kHashNodeStride);
    }
    ++DSFV29_HashFaults;
    OutputDebugStringA("DSFV29b: FATAL hash node cursor invalid or 90000-node total reached; stopping before corruption.\n");
    LegacyExit(0x29u,"hashCursor",owner,unused);
    return 0;
}

// Replaces the original fresh-entry path at BAEDB6. Until the stock
// cursor reaches the end, execute equivalent stock MOV/LEA/MOV and resume
// at BAEDD0 without calling the C++ helper. PUSHFD/POPFD restore the flags
// from the TEST at BAEDB2 (the bound comparisons would otherwise alter them).
// Only the at-end/invalid-cursor path invokes the external-arena helper.
// It also leaves the original free-list reuse path at BAEDC7 untouched.
extern "C" __declspec(naked) void V29HashFreshNodeHook() {
    __asm {
        pushfd
        // Seed telemetry with the first allocator root; no helper calls on
        // the native fast path. Keep the root stable for the entire process.
        cmp dword ptr [g_hashFirstOwner], 0
        jne native_root_ready
        mov dword ptr [g_hashFirstOwner], ecx
    native_root_ready:
        mov eax, dword ptr [ecx+0x127AC4]
        lea ebx, [ecx+0x127AC0]
        cmp eax, ebx
        jae slow_path
        lea ebx, [ecx+0x20000]
        cmp eax, ebx
        jb slow_path
        lea ebx, [eax+0x18]
        mov dword ptr [ecx+0x127AC4], ebx
        popfd
        jmp dword ptr [g_hashAllocationResumeAddress]
    slow_path:
        pushad
        push ecx
        call V29AcquireFreshHashNode
        add esp, 4
        mov [esp+28], eax
        popad
        lea ebx, [eax+0x18]
        popfd
        jmp dword ptr [g_hashAllocationResumeAddress]
    }
}

// Do this at startup before the game calls BAEDB6. On failure, restore the
// exact stock bytes. Never make this code-page change at runtime.
bool RemoveHashArenaHook() {
    if (g_hashPatchInstalled) {
        const UPtr at = Address(kHashAllocationSite);
        const U8 stock[] = {0x8B,0x81,0xC4,0x7A,0x12,0x00};
        DWORD old=0;
        if (!VirtualProtect(reinterpret_cast<void*>(at),sizeof(stock),kPageExecuteReadWrite,&old))
            return false;
        for (U32 i=0;i<sizeof(stock);++i)
            *reinterpret_cast<volatile U8*>(at+i)=stock[i];
        const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),sizeof(stock));
        DWORD discard=0;
        const BOOL restored=VirtualProtect(reinterpret_cast<void*>(at),sizeof(stock),old,&discard);
        if (!flushed || !restored) return false;
        g_hashPatchInstalled=false;
    }
    DSFV29_HashHook=0;
    if (g_hashExternal) {
        VirtualFree(g_hashExternal,0,kMemRelease);
        g_hashExternal=nullptr;
    }
    return true;
}

bool InstallHashArenaHook() {
    // Byte-signature checked on the actual loaded image, in addition to full
    // PE identity verification performed during preinit graphics patching.
    constexpr U8 stock[] = {0x8B,0x81,0xC4,0x7A,0x12,0x00,0x8D,0x58,0x18,
                            0x89,0x99,0xC4,0x7A,0x12,0x00,0xEB,0x09};
    const UPtr site=Address(kHashAllocationSite);
    if (!VerifyExecutable() || !Readable(site,sizeof(stock))) return false;
    for (U32 i=0;i<sizeof(stock);++i)
        if (*reinterpret_cast<const U8*>(site+i)!=stock[i]) return false;
    if (!Readable(Address(kHashAllocationResume),2u) ||
        *reinterpret_cast<const U8*>(Address(kHashAllocationResume))!=0x89u ||
        *reinterpret_cast<const U8*>(Address(kHashAllocationResume)+1u)!=0x70u) return false;
    g_hashExternal=reinterpret_cast<U8*>(VirtualAlloc(nullptr,kHashExtraNodes*kHashNodeStride,
                                                         kMemCommitReserve,kPageReadWrite));
    if (!g_hashExternal) return false;
    g_hashAllocationResumeAddress=Address(kHashAllocationResume);
    DWORD old=0;
    if (!VirtualProtect(reinterpret_cast<void*>(site),6u,kPageExecuteReadWrite,&old)) {
        RemoveHashArenaHook(); return false;
    }
    // E9 rel32 + NOP replaces the complete 6-byte MOV instruction.
    *reinterpret_cast<volatile U8*>(site)=0xE9u;
    Write32(site+1u,static_cast<U32>(reinterpret_cast<UPtr>(&V29HashFreshNodeHook)-(site+5u)));
    *reinterpret_cast<volatile U8*>(site+5u)=0x90u;
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),6u);
    DWORD ignored=0;
    const BOOL protectionRestored=VirtualProtect(reinterpret_cast<void*>(site),6u,old,&ignored);
    g_hashPatchInstalled=true;
    if (!flushed || !protectionRestored) {
        RemoveHashArenaHook(); return false;
    }
    DSFV29_HashHook=1;
    OutputDebugStringA("DSFV29b: Hash overflow-only native fast path installed; stock 45000 + external 45000 records.\n");
    return true;
}

void Log(const char* value) { OutputDebugStringA(value); }

// NOTE: This is deliberately limited to a single, exact pre-constructor
// source allocation; it does not patch any admission/handler/selector count.
void ApplyPreinitGraphicsExpansion() {
    if (!VerifyExecutable()) {
        DSF150_Status = -1;
        Log("DSFV29b: executable identity/signature did not match, NO PATCH.\n");
        return;
    }
#if defined(DSF_V29S_INDEXED_262144)
    g_indexedStockVerified=true; // VerifyExecutable passed before our code writes.
#endif
    const UPtr manager = Address(kGraphicsManager);
    if (!Readable(manager + 0x348u, 8u) || Read32(manager + 0x348u) != 0u) {
        DSF150_Status = -2;
        Log("DSFV29b: graphics pool already allocated, NO PATCH.\n");
        return;
    }
    if (Read32(Address(kGraphicsCount)) != kOldSlots ||
        Read32(Address(kBudgetA)) != 0x2000u ||
        Read32(Address(kBudgetB)) != 0x10000u ||
        Read32(Address(kRecordCount)) != kOldSlots ||
        Read32(Address(kBlockSize)) != 0x400u ||
        Read32(Address(kCityRegionBudgetA)) != kOldCityBudgetA ||
        Read32(Address(kCityRegionBudgetB)) != kOldCityBudgetB ||
        Read32(Address(kCityRegionCount)) != 0u ||
        Read32(Address(kCityRegionBlockSize)) != 0x400u) {
        DSF150_Status = -3;
        Log("DSFV29b: stock graphics source values unexpected, NO PATCH.\n");
        return;
    }
    const UPtr start = Address(kGraphicsCount);
    const U32 length = static_cast<U32>(Address(kRecordCount) + 4u - start);
    DWORD oldProt = 0u;
    if (!VirtualProtect(reinterpret_cast<void*>(start), length, kPageReadWrite, &oldProt)) {
        DSF150_Status = -4;
        Log("DSFV29b: cannot temporarily unlock graphics configuration, NO PATCH.\n");
        return;
    }
    Write32(Address(kGraphicsCount), kNewSlots);
    Write32(Address(kBudgetA), kNewBudgetA);
    Write32(Address(kBudgetB), kNewBudgetB);
    Write32(Address(kRecordCount), kNewSlots);
    Write32(Address(kCityRegionBudgetA), kNewCityBudgetA);
    Write32(Address(kCityRegionBudgetB), kNewCityBudgetB);
    const bool verified = Read32(Address(kGraphicsCount)) == kNewSlots &&
        Read32(Address(kBudgetA)) == kNewBudgetA &&
        Read32(Address(kBudgetB)) == kNewBudgetB &&
        Read32(Address(kRecordCount)) == kNewSlots &&
        Read32(Address(kCityRegionBudgetA)) == kNewCityBudgetA &&
        Read32(Address(kCityRegionBudgetB)) == kNewCityBudgetB &&
        Read32(Address(kCityRegionCount)) == 0u;
    if (!verified) {
        // Restore data if the first write failed or was modified externally.
        Write32(Address(kGraphicsCount), kOldSlots);
        Write32(Address(kBudgetA), 0x2000u);
        Write32(Address(kBudgetB), 0x10000u);
        Write32(Address(kRecordCount), kOldSlots);
        Write32(Address(kCityRegionBudgetA), kOldCityBudgetA);
        Write32(Address(kCityRegionBudgetB), kOldCityBudgetB);
    }
    DWORD discarded = 0;
    const BOOL protectionRestored = VirtualProtect(reinterpret_cast<void*>(start), length,
                                                    oldProt, &discarded);
    if (!protectionRestored) {
        DSF150_Status = -6;
        Log("DSFV29b: protection restore failed; stop this game session.\n");
        return;
    }
    if (!verified) {
        DSF150_Status = -5;
        Log("DSFV29b: graphics post-write validation failed; original values restored.\n");
        return;
    }
    DSF150_Status = 1;
    Log(DSF_GRAPHICS_ADMISSION_TARGET == 160
        ? "DSFV29b: early graphics source patched 35 -> 160.\n"
        : "DSFV29b: early graphics source patched 35 -> 150.\n");
    Log("DSFV29b: City Region budget A 0x4000 -> 0x8000, B 0x20000 -> 0x40000; City Region count remains ZERO.\n");
}

#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
// The 0xF8 vehicle-definition allocator uses the third pool initialized by
// 0088A4C0. Its stock capacity is 0x2D (45). The same EDX writes +48 and +64;
// changing only this immediate lets the native allocator size both arrays.
static bool g_sModelPoolPatched=false;
static bool RestoreModelPoolCapacity() {
    if(!g_sModelPoolPatched)return true;
    const UPtr site=Address(0x0088A4F3u);
    DWORD old=0;
    if(!VirtualProtect(reinterpret_cast<void*>(site),4u,kPageExecuteReadWrite,&old))return false;
    Write32(site,45u);
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),4u);
    DWORD discarded=0;
    const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(site),4u,old,&discarded);
    if(flushed && protectedAgain)g_sModelPoolPatched=false;
    return flushed && protectedAgain;
}
static bool InstallModelPoolCapacity() {
    const UPtr entry=Address(0x0088A4F2u);
    const U8 stock[]={0xBA,0x2D,0x00,0x00,0x00,0x89,0x50,0x48};
    const U8 secondWrite[]={0x89,0x50,0x64};
    const UPtr singleton=Address(0x0144BCFCu);
    if(!VerifyExecutable() || !Readable(entry,sizeof(stock)) ||
       !Readable(Address(0x0088A506u),sizeof(secondWrite)) ||
       !Readable(singleton,4u) || Read32(singleton)!=0u)return false;
    for(U32 i=0;i<sizeof(stock);++i)
        if(*reinterpret_cast<const U8*>(entry+i)!=stock[i])return false;
    for(U32 i=0;i<sizeof(secondWrite);++i)
        if(*reinterpret_cast<const U8*>(Address(0x0088A506u)+i)!=secondWrite[i])return false;
    DWORD old=0;
    if(!VirtualProtect(reinterpret_cast<void*>(entry+1u),4u,kPageExecuteReadWrite,&old))return false;
    Write32(entry+1u,160u);
    g_sModelPoolPatched=true;
    const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(entry),8u);
    DWORD discarded=0;
    const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(entry+1u),4u,old,&discarded);
    if(!flushed || !protectedAgain || Read32(entry+1u)!=160u) {
        RestoreModelPoolCapacity();return false;
    }
    Log("DSFV29s: vehicle-definition pool source capacity 45 -> 160 before allocator initialization.\n");
    return true;
}
#endif

#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
// Driver.exe keeps a 16,384-entry pointer array immediately before its
// 16,384-entry pair array, with the count immediately after the pair array.
// The append at 006A594F writes through the count at index 16384. Keep the
// native count and redirect BOTH arrays, including the manager-relative
// traversal/compaction accesses, to one larger pair of backing arrays.
static const U32 kResourceCollectionCapacity=131072u;
static U32 g_sResourcePointers[kResourceCollectionCapacity]={};
static U32 g_sResourcePairs[kResourceCollectionCapacity*2u]={};
struct ResourceArrayPatch { UPtr site; U8 offset; U32 stock; U8 target; bool relative; bool done; };
// target: 0=pointer array, 1=pair first word, 2=pair second word.
static ResourceArrayPatch g_sResourceArrayPatches[]={
    {0x006A51BDu,3u,0x00137AC8u,1u,true,false},
    {0x006A51C7u,3u,0x00137ACCu,2u,true,false},
    {0x006A51D4u,3u,0x00127AC8u,0u,true,false},
    {0x006A593Fu,3u,0x0186A6B8u,1u,false,false},
    {0x006A594Fu,3u,0x0186A6BCu,2u,false,false},
    {0x006A595Cu,3u,0x0185A6B8u,0u,false,false},
    {0x006A5A19u,3u,0x0186A6B8u,1u,false,false},
    {0x006A5A29u,3u,0x0186A6BCu,2u,false,false},
    {0x006A5A36u,3u,0x0185A6B8u,0u,false,false},
    {0x006CC661u,3u,0x0186A6B8u,1u,false,false},
    {0x006CC671u,3u,0x0186A6BCu,2u,false,false},
    {0x006CC683u,3u,0x0185A6B8u,0u,false,false},
    {0x00BAA038u,3u,0x0186A6B8u,1u,false,false},
    {0x00BAA048u,3u,0x0186A6BCu,2u,false,false},
    {0x00BAA055u,3u,0x0185A6B8u,0u,false,false},
    {0x00BAA2CBu,3u,0x0186A6B8u,1u,false,false},
    {0x00BAA2DAu,3u,0x0186A6BCu,2u,false,false},
    {0x00BAA2E7u,3u,0x0185A6B8u,0u,false,false},
    {0x00BAA342u,3u,0x0186A6B8u,1u,false,false},
    {0x00BAA351u,3u,0x0186A6BCu,2u,false,false},
    {0x00BAA35Eu,3u,0x0185A6B8u,0u,false,false},
    {0x00BAE7AEu,2u,0x00137AC8u,1u,true,false},
    {0x00BAE7B9u,2u,0x00127AC8u,0u,true,false},
    {0x00BAE8D9u,3u,0x00127AC8u,0u,true,false},
    {0x00BAE8EDu,3u,0x00137AC8u,1u,true,false},
    {0x00BAE8F6u,3u,0x00137ACCu,2u,true,false},
    {0x00BAEAADu,2u,0x00137AC8u,1u,true,false},
    {0x00BAEAB3u,2u,0x00127AC8u,0u,true,false},
    {0x00BAEAD0u,3u,0x00127AC8u,0u,true,false},
    {0x00BAEADFu,3u,0x00137AC8u,1u,true,false},
    {0x00BAEAE8u,3u,0x00137ACCu,2u,true,false}
};
static U32 g_sResourcePatchCount=0u;
static U32 ResourceArrayReplacement(const ResourceArrayPatch& patch) {
    const UPtr target=patch.target==0u
        ? reinterpret_cast<UPtr>(g_sResourcePointers)
        : reinterpret_cast<UPtr>(g_sResourcePairs)+(patch.target==2u?4u:0u);
    return static_cast<U32>(target-(patch.relative?Address(0x01732BF0u):0u));
}
static bool RestoreResourceCollection() {
    bool okay=true;
    for(U32 i=static_cast<U32>(sizeof(g_sResourceArrayPatches)/sizeof(g_sResourceArrayPatches[0]));i>0u;--i) {
        ResourceArrayPatch& patch=g_sResourceArrayPatches[i-1u];
        if(!patch.done)continue;
        const UPtr site=Address(patch.site)+patch.offset;
        DWORD old=0,discarded=0;
        if(!VirtualProtect(reinterpret_cast<void*>(site),4u,kPageExecuteReadWrite,&old)) {okay=false;continue;}
        Write32(site,patch.stock);
        const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),4u)!=0;
        const bool restored=VirtualProtect(reinterpret_cast<void*>(site),4u,old,&discarded)!=0;
        if(!flushed || !restored)okay=false;
        patch.done=false;
    }
    if(okay)g_sResourcePatchCount=0u;
    return okay;
}
static bool InstallResourceCollection() {
    if(!VerifyExecutable() || !Readable(Address(0x0188A6B8u),4u) ||
       Read32(Address(0x0188A6B8u))!=0u ||
       Address(0x01732BF0u)+0x00157AC8u!=Address(0x0188A6B8u))return false;
    for(const ResourceArrayPatch& patch:g_sResourceArrayPatches) {
        const UPtr site=Address(patch.site)+patch.offset;
        if(!Readable(site,4u) || Read32(site)!=patch.stock)return false;
    }
    for(ResourceArrayPatch& patch:g_sResourceArrayPatches) {
        const UPtr site=Address(patch.site)+patch.offset;
        DWORD old=0,discarded=0;
        if(!VirtualProtect(reinterpret_cast<void*>(site),4u,kPageExecuteReadWrite,&old)) {
            RestoreResourceCollection();return false;
        }
        const U32 replacement=ResourceArrayReplacement(patch);
        Write32(site,replacement);
        patch.done=true;
        ++g_sResourcePatchCount;
        const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),4u)!=0;
        const bool restored=VirtualProtect(reinterpret_cast<void*>(site),4u,old,&discarded)!=0;
        if(!flushed || !restored || Read32(site)!=replacement) {
            RestoreResourceCollection();return false;
        }
    }
    Log("DSFV29s: resource collection pointer/pair arrays redirected together, capacity 16384 -> 131072.\n");
    return true;
}
#endif

// The native initializer at 0x005A1420 begins with a 5-byte MOV EAX,[root].
// No relative control transfer is present in those five bytes. This exact
// prologue is checked before the E9 detour is installed. The trampoline
// executes those original bytes and jumps back to original+5.
// HandlerInitBridge matches V20's verified __fastcall bridge calling convention.
void SetAdmissionError(long status, const char* line) {
    if (DSFV27B_AdmissionResult == 0) {
        DSFV27B_AdmissionResult = status;
        Log(line);
    }
}

bool WriteMetadataByte(UPtr where, U8 value) {
    if (!Readable(where, 1)) return false;
    DWORD oldProt = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(where), 1, kPageReadWrite, &oldProt))
        return false;
    *reinterpret_cast<volatile U8*>(where) = value;
    const bool verified = *reinterpret_cast<volatile U8*>(where) == value;
    DWORD unused = 0;
    const BOOL restored = VirtualProtect(reinterpret_cast<void*>(where), 1, oldProt, &unused);
    return verified && restored;
}

bool GraphicsPoolReady() {
    const UPtr pool9 = Address(kGraphicsManager) + 0x348u;
    return Readable(pool9, 8) && Read32(pool9) != 0u && Read32(pool9 + 4u) == kNewSlots;
}


// ---------------------------------------------------------------------------
// V27b FIRST-PASS (HIGH-RISK) resource pointer redirection for package zero.
// This is NOT a production-safe 150-resident implementation. It leaves native
// request staging and some indirect owner lifetimes unaudited. Do not use with
// normal saves. All entry patch bytes are verified against exact Driver.exe.
// ---------------------------------------------------------------------------
// Derived solely from the SHA256-verified Driver.exe. Exact instruction boundaries verified.
// Pool 512: obj size 0x1818; records 0x10..0x100F; count +0x1010;
// free pointer slots +0x1014..+0x1813; free count +0x1814.
struct PoolPatch { UPtr va; U8 len; U8 immOffset; U8 stock[8]; U32 replace; bool done; };
static PoolPatch g_poolPatches[] = {
    {0x00494240u,5u,1u,{0x68u,0x74u,0x07u,0x00u,0x00u},0x1818u,false}, // allocate_0x1818
    {0x0049415Fu,5u,1u,{0x68u,0x9Du,0x00u,0x00u,0x00u},0x200u,false}, // init_records_512
    {0x0049425Cu,6u,2u,{0x89u,0x88u,0xF8u,0x04u,0x00u,0x00u},0x1010u,false}, // ctor_count_zero
    {0x00494262u,6u,2u,{0x89u,0x88u,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // ctor_free_zero
    {0x0049417Bu,6u,2u,{0x8Bu,0x8Fu,0xE8u,0x04u,0x00u,0x00u},0x1000u,false}, // init_read_records
    {0x00494197u,6u,2u,{0x8Bu,0x8Eu,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // init_read_free
    {0x0049419Du,7u,3u,{0x8Du,0x8Cu,0x8Eu,0xFCu,0x04u,0x00u,0x00u},0x1014u,false}, // init_free_slot
    {0x004941AAu,6u,2u,{0x01u,0xBEu,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // init_add_free
    {0x004941C4u,6u,2u,{0x8Bu,0x83u,0xF8u,0x04u,0x00u,0x00u},0x1010u,false}, // destruct_record_count
    {0x00494288u,6u,2u,{0x8Bu,0x87u,0xE8u,0x04u,0x00u,0x00u},0x1000u,false}, // resize_read_count
    {0x004942B4u,6u,2u,{0x89u,0xAFu,0xE8u,0x04u,0x00u,0x00u},0x1000u,false}, // resize_write_count_a
    {0x004942D6u,6u,2u,{0x89u,0xAFu,0xE8u,0x04u,0x00u,0x00u},0x1000u,false}, // resize_write_count_b
    {0x00493FD0u,6u,2u,{0x8Bu,0x81u,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // pop_read_free
    {0x00493FD6u,7u,3u,{0x8Bu,0x94u,0x81u,0xF8u,0x04u,0x00u,0x00u},0x1010u,false}, // pop_index_ptr
    {0x00493FE1u,6u,2u,{0xFFu,0x89u,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // pop_dec_free
    {0x00494090u,6u,2u,{0x8Bu,0x81u,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // return_read_free
    {0x00494096u,7u,3u,{0x8Du,0x84u,0x81u,0xFCu,0x04u,0x00u,0x00u},0x1014u,false}, // return_ptr_slot
    {0x004940A9u,6u,2u,{0xFFu,0x81u,0x70u,0x07u,0x00u,0x00u},0x1814u,false}, // return_inc_free
};


// V27b pool patches MUST be fully installed before any pool object is created.
// All 18 patches use same-length DWORD operands, do not alter instruction length.
extern "C" __declspec(dllexport) volatile long DSFV27B_PoolPatchCount = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_PoolStatus = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_PoolLiveRecordCount = -1;
extern "C" __declspec(dllexport) volatile long DSFV27B_PoolFreeCount = -1;
// Polling-based low-water statistic, not an exact per-call high-water mark.
extern "C" __declspec(dllexport) volatile long DSFV27B_PoolMinFreeObserved = -1;

void RestorePoolPatches() {
    for (unsigned i=sizeof(g_poolPatches)/sizeof(g_poolPatches[0]);i>0;--i) {
        PoolPatch& patch=g_poolPatches[i-1];
        if(!patch.done) continue;
        UPtr va=Address(patch.va);
        DWORD old=0,ignored=0;
        if(VirtualProtect(reinterpret_cast<void*>(va),patch.len,kPageExecuteReadWrite,&old)) {
            for (U32 j=0;j<patch.len;++j) *reinterpret_cast<volatile U8*>(va+j)=patch.stock[j];
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(va),patch.len);
            VirtualProtect(reinterpret_cast<void*>(va),patch.len,old,&ignored);
        }
        patch.done=false;
    }
    DSFV27B_PoolPatchCount=0;
    DSFV27B_PoolStatus=0;
}

bool InstallPool512Patches() {
    // Verify ALL bytes before touching any instruction.
    for(auto& patch:g_poolPatches) {
        UPtr va=Address(patch.va);
        if(!Readable(va,patch.len)) return false;
        for(unsigned j=0;j<patch.len;++j)
            if(*reinterpret_cast<volatile const U8*>(va+j)!=patch.stock[j]) return false;
    }
    for(auto& patch:g_poolPatches) {
        const UPtr va=Address(patch.va);
        DWORD old=0,discard=0;
        if(!VirtualProtect(reinterpret_cast<void*>(va),patch.len,kPageExecuteReadWrite,&old)) {
            RestorePoolPatches();return false;
        }
        // Only the 32-bit immediate/displacement changes. All opcodes stay identical.
        Write32(va+patch.immOffset,patch.replace);
        bool exact=true;
        for(unsigned j=0;j<patch.len;++j) {
            const U8 desired=(j>=patch.immOffset && j<patch.immOffset+4)
                ? static_cast<U8>(patch.replace>>(8*(j-patch.immOffset)))
                : patch.stock[j];
            if(*reinterpret_cast<volatile const U8*>(va+j)!=desired) exact=false;
        }
        patch.done=true;
        bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(va),patch.len)!=0;
        bool protectedOk=VirtualProtect(reinterpret_cast<void*>(va),patch.len,old,&discard)!=0;
        if(!exact||!flushed||!protectedOk) { RestorePoolPatches();return false; }
        ++DSFV27B_PoolPatchCount;
    }
    DSFV27B_PoolStatus=1;
    Log("DSFV29b: 512-record native object pool: allocation, init, return/pop, destructor offsets patched (18/18).\n");
    return true;
}

constexpr U32 kHandlerArraySidecar = 256u; // 154 handlers plus guard headroom
constexpr U32 kMapExtraNodes = 256u;       // node count can exceed 150 transiently
constexpr U32 kSelectorExtraNodes = 160u;
constexpr U32 kThirdExtraNodes = 160u;
extern "C" __declspec(dllexport) volatile long DSFV27B_NativeHooks = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_MapExternal = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_MapMaxActive = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_ArrayGuard = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_SelectorOverflow = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_ThirdOverflow = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_UnsafeBoundary = 0;

static volatile UPtr g_pkg0 = 0;
static U32 g_globalHandlers[kHandlerArraySidecar] = {};
static U32 g_kind0Handlers[kHandlerArraySidecar] = {};
struct Node16 { U32 next, prev, key, handler; };
struct Node12 { U32 next, prev, value; };
static Node16 g_mapExtra[kMapExtraNodes] = {};

// ED and EF owners have independent 45-node maps at owner+0x23C. Both use
// the same exact-build vtable and native insertion/removal routines.
constexpr U32 kResourceMap16ExtraNodes = 256u;
constexpr U32 kResourceMap16Owners = 96u;
struct ResourceMap16Sidecar {
    UPtr owner;
    U32 used;
    Node16 nodes[kResourceMap16ExtraNodes];
};
static ResourceMap16Sidecar g_resourceMap16[kResourceMap16Owners] = {};
static volatile long g_resourceMap16Lock = 0;
extern "C" __declspec(dllexport) volatile long DSFV29i_Map16ExtraInserts = 0;
extern "C" __declspec(dllexport) volatile long DSFV29i_Map16ActiveHighWater = 0;
extern "C" __declspec(dllexport) volatile long DSFV29i_Map16OwnersSeen = 0;
extern "C" __declspec(dllexport) volatile long DSFV29i_Map16Guard = 0;
extern "C" __declspec(dllexport) volatile long DSFV29n_EFExtraInserts = 0;
extern "C" __declspec(dllexport) volatile long DSFV29n_EFActiveHighWater = 0;
extern "C" __declspec(dllexport) volatile long DSFV29n_EFSidecarResets = 0;
extern "C" __declspec(dllexport) volatile long DSFV29n_EFSidecarReleases = 0;
extern "C" __declspec(dllexport) volatile long DSFV29n_EFGuard = 0;
// V29k diagnostic only: stop before an unexpanded EF owner writes past its
// inline arena. A debugger must be attached and the process must be ended
// after the stop; this does not extend EF capacity or make continuation safe.
extern "C" __declspec(dllexport) volatile long DSFV29k_EFBoundaryHits = 0;
extern "C" __declspec(dllexport) volatile long DSFV29k_EFMap = 0;
extern "C" __declspec(dllexport) volatile long DSFV29k_EFCursor = 0;
extern "C" __declspec(dllexport) volatile long DSFV29k_EFActive = 0;
extern "C" __declspec(dllexport) volatile long DSFV29k_EFRelease = 0;
// V29l receiver-writer experiment: leave EF insertion running so one
// receiver can be watched through its lifetime. Debugger can opt into hold.
extern "C" __declspec(dllexport) volatile long DSFV29l_EFHoldEnabled = 0;

struct SelectorSidecar { UPtr owner; U32 used; Node12 nodes[kSelectorExtraNodes]; };
struct ThirdSidecar { UPtr owner; U32 used; Node12 nodes[kThirdExtraNodes]; };
static SelectorSidecar g_selSidecars[64] = {};
static ThirdSidecar g_thirdSidecars[64] = {};
static volatile long g_selLock=0;
static volatile long g_thirdLock=0;
void SelectorLock(){while(InterlockedExchange(&g_selLock,1u)){} }
void SelectorUnlock(){InterlockedExchange(&g_selLock,0);}
void ThirdLock(){while(InterlockedExchange(&g_thirdLock,1u)){} }
void ThirdUnlock(){InterlockedExchange(&g_thirdLock,0);}
// Compact CPU register snapshot as emitted by pushfd;pushad. When changing a
// field, popad restores it back to the original game register before resume.
struct CpuRegs {
    U32 edi, esi, ebp, savedEsp, ebx, edx, ecx, eax, eflags;
};
static_assert(sizeof(CpuRegs)==36, "Bad native register frame");
// V29c passive diagnostic exports. The file writer runs only on Monitor.
extern "C" __declspec(dllexport) volatile long DSFV29c_CallbackHooks = 0;
extern "C" __declspec(dllexport) volatile long DSFV29c_CallbackTotalEvents = 0;
extern "C" __declspec(dllexport) volatile long DSFV29c_CallbackEventDrops = 0;
extern "C" __declspec(dllexport) volatile long DSFV29c_CallbackSuspicious = 0;
extern "C" __declspec(dllexport) volatile long DSFV29c_DestroyActualCount = -1;
extern "C" __declspec(dllexport) volatile long DSFV29c_DestroyRedirectDword = -1;
extern "C" __declspec(dllexport) volatile long DSFV29c_DestroyEntryCalls = 0;

extern "C" __declspec(dllexport) volatile long DSFV27B_InstructionRedirectCalls = 0;

// V27b: the request's original inline storage is 50 DWORDs at [ctx+0..0xC4].
// [ctx+0xC8] is the count; writing index 50 directly would corrupt it.
// Store the full request outside the game object and preserve the inline prefix.
// This is scoped to the exact verified executable; no extra requests are
// accepted if we cannot retain every item. The native linked-list scratch
// frame at 0x005A7490 is separately expanded to 150 * 12-byte nodes.
constexpr U32 kReqInline = 50u;
constexpr U32 kReqCapacity = 150u;
struct RequestSidecar { UPtr owner; U32 count; U32 ids[kReqCapacity]; };
#if defined(DSF_V29S_BASELINE150)
static void BaselineRequest(UPtr owner,UPtr caller,const U32* original,int originalCount,const U32* effective,U32 count,const char* result);
#endif
static RequestSidecar g_reqSidecars[16] = {};
static volatile long g_reqLock = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestLastCount = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestHighWater = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestExtraReads = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestRejected = 0;
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestFault = 0;
#if defined(DSF_V29S_NATIVE_TRACE)
// Passive snapshots at the original 50-ID inline boundary. The request
// reader still supplies exactly the same EDI value to the game.
static const U32 kRequestProbeIndex[10]={0u,44u,49u,50u,51u,52u,100u,101u,102u,149u};
static volatile long g_requestProbeCalls[10]={};
static volatile long g_requestProbeId[10]={};
static volatile long g_requestProbeFound[10]={};
static volatile long g_requestProbeBad[10]={};
extern "C" __declspec(dllexport) volatile long DSFV29s_RequestBoundaryBad=0;
#endif
extern "C" __declspec(dllexport) volatile long DSFV27B_RequestFramePatchCount = 0;
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadHits = 0;
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadLastHash = 0;
// Diagnostic gate: 0=no exact 18-ID list yet, 1=wrong hash, 2=graphics not ready,
// 3=admission not 150, 4=150-ID expansion applied, 5=18-ID control.
// Confirms the intended roster size from the loaded ASI, not actual residency.
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadConfiguredCount = DSF_CHAPTER7_PRELOAD_COUNT;
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadSeen18 = 0;
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadExact18Matches = 0;
extern "C" __declspec(dllexport) volatile long DSF_Chapter7PreloadGate = 0;

U32 CurrentTrafficHashForPreload() {
    const UPtr rootLocation=Address(kRootPtr);
    if (!Readable(rootLocation,4u)) return 0u;
    const UPtr root=Read32(rootLocation);
    if (!root || !Readable(root+0x758u,4u)) return 0u;
    const UPtr context=Read32(root+0x758u);
    if (!context || !Readable(context+0x5CA840u,4u)) return 0u;
    return Read32(context+0x5CA840u);
}

bool IsExactChapter7StockRequest(const U32* list, U32 count) {
    if (count!=18u || !list || !Readable(reinterpret_cast<UPtr>(list),18u*4u)) return false;
    for (U32 i=0;i<18u;++i)
        if (list[i]!=kChapter7StockIds[i]) return false;
    return true;
}

#if defined(DSF_V29S_CHAPTERS)
static_assert(DSF_CHAPTER7_PRELOAD_COUNT==150 && DSF_GRAPHICS_ADMISSION_TARGET==160,
              "Chapter-independent selection retains the validated 150/160 capacities");
#if !defined(DSF_V29S_PENDING512)
#error Chapter-independent selection requires the validated pending512 baseline
#endif
#include "V29sChapterCore.inc"
#endif

void RequestLock() { while (InterlockedExchange(&g_reqLock, 1u)) {} }
void RequestUnlock() { InterlockedExchange(&g_reqLock,0); }
RequestSidecar* RequestSlot(UPtr ctx, bool create) {
    RequestSidecar* empty = nullptr;
    for (auto& slot : g_reqSidecars) {
        if (slot.owner == ctx) return &slot;
        if (slot.owner == 0u && !empty) empty = &slot;
    }
    if (create && empty) { empty->owner=ctx; return empty; }
    return nullptr;
}

#if defined(DSF_V29S_REG_SOURCE_DIAG)
static const U32 kSRequestBatchCapacity=64u,kSRequestBatchIds=160u;
struct DiagSRequestBatch {
    volatile long sequence;
    U32 tick,context,originalCount,effectiveCount;
    U32 originalIds[kSRequestBatchIds],effectiveIds[kSRequestBatchIds];
};
static DiagSRequestBatch g_sRequestBatches[kSRequestBatchCapacity]={};
static volatile long g_sRequestBatchNext=0,g_sRequestBatchDrops=0;
static long g_sRequestBatchDrained=0;
static void DiagSRecordRequestBatch(UPtr context,const U32* original,int originalCount,
                                    const U32* effective,int effectiveCount) {
    if(originalCount<18 && effectiveCount<18)return;
    const long sequence=InterlockedIncrement(&g_sRequestBatchNext);
    if(sequence<1 || static_cast<U32>(sequence)>kSRequestBatchCapacity) {
        InterlockedIncrement(&g_sRequestBatchDrops);return;
    }
    DiagSRequestBatch& batch=g_sRequestBatches[sequence-1];
    batch.tick=GetTickCount();batch.context=context;
    batch.originalCount=originalCount>0?static_cast<U32>(originalCount):0u;
    batch.effectiveCount=effectiveCount>0?static_cast<U32>(effectiveCount):0u;
    const U32 originalCopy=batch.originalCount<kSRequestBatchIds?
        batch.originalCount:kSRequestBatchIds;
    const U32 effectiveCopy=batch.effectiveCount<kSRequestBatchIds?
        batch.effectiveCount:kSRequestBatchIds;
    for(U32 i=0;i<originalCopy;++i)batch.originalIds[i]=original[i];
    for(U32 i=0;i<effectiveCopy;++i)batch.effectiveIds[i]=effective[i];
    InterlockedExchange(&batch.sequence,sequence);
}
#endif

// ABI: original 0x005A79F0 is thiscall, stack args [list, signed count],
// callee pops 8. __fastcall takes ECX=self and an unused EDX, then the
// same two stack arguments and produces a matching RET 8.
extern "C" void __fastcall RequestWriteBridge(void* self, void*, const U32* list, int requested) {
    const UPtr ctx=reinterpret_cast<UPtr>(self);
#if defined(DSF_V29S_BASELINE150)
    const U32* baselineOriginal=list;const int baselineCount=requested;
    const UPtr baselineCaller=reinterpret_cast<UPtr>(_ReturnAddress());
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
    const U32* const originalList=list;
    const int originalRequested=requested;
#endif
    if (!Readable(ctx, 0xDB0u) || requested > static_cast<int>(kReqCapacity) ||
        (requested > 0 && (!list || !Readable(reinterpret_cast<UPtr>(list),
                                                  static_cast<U32>(requested)*4u)))) {
        ++DSFV27B_RequestRejected;
        Log("DSFV29b: REJECT request with bad count/context/source, previous request retained.\n");
#if defined(DSF_V29S_BASELINE150)
        BaselineRequest(ctx,baselineCaller,baselineOriginal,baselineCount,nullptr,0u,"invalidInput");
#endif
        return;
    }
#if defined(DSF_V29S_CHAPTERS)
    ChapterSelectRequest(reinterpret_cast<UPtr>(_ReturnAddress()),ctx,list,requested);
#else
    // Match the exact 18-ID Chapter 7/f0 native request during normal loading.
    // Direct 45-ID console requests do not match this fingerprint.
    if (requested == 18) ++DSF_Chapter7PreloadSeen18;
    if (IsExactChapter7StockRequest(list,static_cast<U32>(requested))) {
        ++DSF_Chapter7PreloadExact18Matches;
        const U32 hash=CurrentTrafficHashForPreload();
        DSF_Chapter7PreloadLastHash=static_cast<long>(hash);
        if (hash != 0x47EB4D0Eu) {
            DSF_Chapter7PreloadGate=1;
        } else if (DSF150_Status != 2) {
            DSF_Chapter7PreloadGate=2;
        } else if (DSFV27B_AdmissionAfter != static_cast<long>(kAdmissionStep)) {
            DSF_Chapter7PreloadGate=3;
#if defined(DSFV29C_DISABLE_PRELOAD)
        } else {
            // V29c 18-ID control: same capacity/monitoring, normal stock request.
            DSF_Chapter7PreloadGate=5;
#else
        } else {
            DSF_Chapter7PreloadGate=4;
            if (DSF_CHAPTER7_PRELOAD_COUNT == 45) {
                list=kChapter7Trial45Ids;
                requested=45;
            } else {
                list=kChapter7Active150Ids;
                requested=150;
            }
            ++DSF_Chapter7PreloadHits;
#if defined(DSF_CHAPTER7_ORDERED_ROSTER)
            Log("DSFV29b: Chapter 7 150-ID request order=ascending.\n");
#endif
            Log(DSF_CHAPTER7_PRELOAD_COUNT == 45
                ? "DSFV29b: exact Chapter 7 native request expanded to 45 during normal chapter load.\n"
                : "DSFV29b: exact Chapter 7 native request expanded to 150 during normal chapter load.\n");
#endif
        }
    }
// Preserve the original Chapter7 experiment for builds without CHAPTERS.
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
    DiagSRecordRequestBatch(ctx,originalList,originalRequested,list,requested);
#endif
    const U32 count=requested>0 ? static_cast<U32>(requested):0u;
    RequestLock();
    RequestSidecar* slot=RequestSlot(ctx, true);
    if (!slot) {
        RequestUnlock();
        ++DSFV27B_RequestRejected;
        Log("DSFV29b: REJECT request: no free request sidecar.\n");
#if defined(DSF_V29S_BASELINE150)
        BaselineRequest(ctx,baselineCaller,baselineOriginal,baselineCount,nullptr,0u,"requestOwnerFull");
#endif
        return;
    }
    for (U32 i=0;i<count;++i) {
        const U32 id=list[i];
        if (id > 0xFFFFu) {
            RequestUnlock();
            ++DSFV27B_RequestRejected;
            Log("DSFV29b: REJECT request: vehicle ID exceeds 16 bits.\n");
#if defined(DSF_V29S_BASELINE150)
            BaselineRequest(ctx,baselineCaller,baselineOriginal,baselineCount,nullptr,0u,"idOutOfRange");
#endif
            return;
        }
    }
    // Copy the full list BEFORE advertising a new count to the game.
    for (U32 i=0;i<count;++i) slot->ids[i]=list[i];
    slot->count=count;
    for (U32 i=0;i<count && i<kReqInline;++i) Write32(ctx+i*4u,slot->ids[i]);
    Write32(ctx+0xC8u,count);
    RequestUnlock();
#if defined(DSF_V29S_BASELINE150)
    BaselineRequest(ctx,baselineCaller,baselineOriginal,baselineCount,list,count,"accepted");
#endif
    DSFV27B_RequestLastCount=static_cast<long>(count);
    if (static_cast<long>(count)>DSFV27B_RequestHighWater)
        DSFV27B_RequestHighWater=static_cast<long>(count);
    if (count>kReqInline)
        Log("DSFV29b: extended traffic request accepted (51..150); stack expansion armed.\n");
    if (Read32(ctx+0xDACu)==0u) {
        typedef void (__thiscall* ProcessRequestsFn)(void*);
        reinterpret_cast<ProcessRequestsFn>(Address(0x005A7490u))(self);
    }
}

#if defined(DSF_V29S_NATIVE_TRACE)
static void RecordRequestProbe(UPtr ctx,U32 index,U32 id,bool found) {
    if(Read32(ctx+0xC8u)!=static_cast<U32>(DSF_CHAPTER7_PRELOAD_COUNT))return;
    for(U32 i=0;i<10u;++i) {
        if(index!=kRequestProbeIndex[i])continue;
        const U32 expected=DSF_CHAPTER7_PRELOAD_COUNT==45 ?
            (index<45u?kChapter7Trial45Ids[index]:0xFFFFu) :
            kChapter7Active150Ids[index];
        InterlockedIncrement(&g_requestProbeCalls[i]);
        g_requestProbeId[i]=static_cast<long>(id);
        g_requestProbeFound[i]=found?1:0;
        if(!found || id!=expected) {
            InterlockedIncrement(&g_requestProbeBad[i]);
            InterlockedIncrement(&DSFV29s_RequestBoundaryBad);
        }
        break;
    }
}
#endif
extern "C" __declspec(noinline) void __cdecl RequestReadBridge(CpuRegs* r) {
    // Stolen instructions: mov eax,[esi+0x334]; movzx edi,word [esi+ebp*4].
    r->eax=Read32(r->esi+0x334u);
    if (r->ebp<kReqInline) {
        r->edi=Read32(r->esi+4u*r->ebp)&0xFFFFu;
#if defined(DSF_V29S_NATIVE_TRACE)
        RecordRequestProbe(r->esi,r->ebp,r->edi,true);
#endif
        return;
    }
    U32 id=0xFFFFu;
    bool found=false;
    RequestLock();
    RequestSidecar* slot=RequestSlot(r->esi,false);
    if(slot && r->ebp<slot->count && r->ebp<kReqCapacity) {
        id=slot->ids[r->ebp];
        found=true;
    }
    RequestUnlock();
    if(!found) {
        DSFV27B_RequestFault=1;
        Log("DSFV29b: FATAL extended request index unavailable; stop process.\n");
    } else ++DSFV27B_RequestExtraReads;
    r->edi=id&0xFFFFu;
#if defined(DSF_V29S_NATIVE_TRACE)
    RecordRequestProbe(r->esi,r->ebp,r->edi,found);
#endif
}

extern "C" UPtr g_retRequestRead = 0;
extern "C" __declspec(naked) void HookRequestRead() {
    __asm {
        pushfd
        pushad
        mov eax, esp
        push eax
        call RequestReadBridge
        add esp, 4
        popad
        popfd
        jmp dword ptr [g_retRequestRead]
    }
}


static inline bool IsPkg(UPtr p) { return p != 0u && p == g_pkg0; }
static inline U32* GlobalBuffer() { return g_globalHandlers; }
static UPtr SelectGlobalPointer(UPtr pkg, U32 index) {
    if (IsPkg(pkg) && index >= 64u) {
        if(index < kHandlerArraySidecar) return reinterpret_cast<UPtr>(&g_globalHandlers[index]);
        DSFV27B_ArrayGuard = 1;
        return reinterpret_cast<UPtr>(&g_globalHandlers[kHandlerArraySidecar-1u]);
    }
    return pkg + 0x5630u + 4u*index;
}
static UPtr SelectKindPointer(UPtr base, U32 index) {
    if (g_pkg0 && base == g_pkg0+0x5734u && index >= 64u) {
        if(index < kHandlerArraySidecar) return reinterpret_cast<UPtr>(&g_kind0Handlers[index]);
        DSFV27B_ArrayGuard = 2;
        return reinterpret_cast<UPtr>(&g_kind0Handlers[kHandlerArraySidecar-1u]);
    }
    return base + 4u*index;
}
// Site codes are compiler-independent and mapped in InstallExtensionHooks.
extern "C" __declspec(noinline) void __cdecl ProcessNativeSite(CpuRegs* r, U32 site) {
    ++DSFV27B_InstructionRedirectCalls;
    switch(site) {
    case 1: // 5A14A1, LEA ECX,[ESI+4*ECX+5630]
        r->ecx = SelectGlobalPointer(r->esi,r->ecx); break;
    case 2: { // 5A14CB, load last global handler after increment
        const U32 count=r->ecx;
        if (IsPkg(r->esi) && count && count <= kHandlerArraySidecar) {
            if (count<=64u)
                g_globalHandlers[count-1u]=Read32(r->esi+0x5630u+(count-1u)*4u);
            r->ecx=g_globalHandlers[count-1u];
        } else r->ecx=Read32(r->esi+0x562cu+count*4u);
        break;
    }
    case 3: // 5A14BE, LEA EAX,[EDI+4*ECX]
        r->eax=SelectKindPointer(r->edi,r->ecx); break;
    case 4: // 5A0CF8, LEA ESI,[ECX+5630]
    case 5: // 5A0D39, LEA ESI,[ECX+5630]
        r->esi=IsPkg(r->ecx) ? reinterpret_cast<UPtr>(GlobalBuffer()) : r->ecx+0x5630u; break;
    case 6: // 5A0DA6 ADD ECX,5630
        r->ecx=IsPkg(r->ecx) ? reinterpret_cast<UPtr>(GlobalBuffer()) : r->ecx+0x5630u; break;
    case 7: // 5A250A LEA EBX,[ESI+5630]
        // OBSERVATION ONLY: [redirect+0x100] is handler[64], NOT pkg+0x5730.
        if (IsPkg(r->esi) && Readable(r->esi+0x5730u,4u)) {
            DSFV29c_DestroyActualCount=static_cast<long>(Read32(r->esi+0x5730u));
            DSFV29c_DestroyRedirectDword=static_cast<long>(g_globalHandlers[64]);
            InterlockedIncrement(&DSFV29c_DestroyEntryCalls);
        }
        r->ebx=IsPkg(r->esi) ? reinterpret_cast<UPtr>(GlobalBuffer()) : r->esi+0x5630u; break;
    case 8: // 5A2572 LEA EAX,[ESI+5630]
        r->eax=IsPkg(r->esi) ? reinterpret_cast<UPtr>(GlobalBuffer()) : r->esi+0x5630u; break;
    case 9: { // 5A0CAB load kind array[count-1] in ECX
        const U32 count=r->ecx;
        if(g_pkg0 && r->eax==g_pkg0+0x104u*0u && count>64u && count<=kHandlerArraySidecar)
            r->ecx=g_kind0Handlers[count-1u];
        else r->ecx=Read32(r->eax+0x5730u+count*4u);
        break;
    }
    case 10: // 5A0F41: LEA ECX,[EAX+4*ECX]; followed by POP ESI; TEST ECX,ECX
    case 11: // 5A1E2D: LEA ECX,[EAX+4*ECX]
    case 12: // 5A2155: LEA ECX,[EAX+4*ECX]
        r->ecx=SelectKindPointer(r->eax,r->ecx); break;
    case 13: // 5A2338: LEA EDX,[EAX+4*EDX]
        r->edx=SelectKindPointer(r->eax,r->edx); break;
    }
}

// Native inline probes (no relocations in the stolen bytes). For sites where
// the original sequence included `test`/`je`, we resume at the non-null
// write path: all destination pointers are guaranteed nonzero.
extern "C" UPtr g_retGlobalCreate = 0;
extern "C" __declspec(naked) void HookGlobalCreate() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 1
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retGlobalCreate]
  }
}
extern "C" UPtr g_retGlobalLast = 0;
extern "C" __declspec(naked) void HookGlobalLast() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 2
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retGlobalLast]
  }
}
extern "C" UPtr g_retInitKind = 0;
extern "C" __declspec(naked) void HookInitKind() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 3
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retInitKind]
  }
}
extern "C" UPtr g_retLoopGlobalA = 0;
extern "C" __declspec(naked) void HookLoopGlobalA() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 4
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retLoopGlobalA]
  }
}
extern "C" UPtr g_retLoopGlobalB = 0;
extern "C" __declspec(naked) void HookLoopGlobalB() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 5
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retLoopGlobalB]
  }
}
extern "C" UPtr g_retFindGlobal = 0;
extern "C" __declspec(naked) void HookFindGlobal() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 6
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retFindGlobal]
  }
}
extern "C" UPtr g_retDestroyGlobalA = 0;
extern "C" __declspec(naked) void HookDestroyGlobalA() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 7
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retDestroyGlobalA]
  }
}
extern "C" UPtr g_retDestroyGlobalB = 0;
extern "C" __declspec(naked) void HookDestroyGlobalB() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 8
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retDestroyGlobalB]
  }
}
extern "C" UPtr g_retPopKind = 0;
extern "C" __declspec(naked) void HookPopKind() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 9
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retPopKind]
  }
}
extern "C" UPtr g_retAppendKind = 0;
extern "C" __declspec(naked) void HookAppendKind() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 10
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    pop esi
    test ecx, ecx
    jmp dword ptr [g_retAppendKind]
  }
}
extern "C" UPtr g_retRequeueKindA = 0;
extern "C" __declspec(naked) void HookRequeueKindA() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 11
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retRequeueKindA]
  }
}
extern "C" UPtr g_retRequeueKindB = 0;
extern "C" __declspec(naked) void HookRequeueKindB() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 12
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retRequeueKindB]
  }
}
extern "C" UPtr g_retRequeueKindC = 0;
extern "C" __declspec(naked) void HookRequeueKindC() {
  __asm {
    pushfd
    pushad
    mov eax, esp
    push 13
    push eax
    call ProcessNativeSite
    add esp, 8
    popad
    popfd
    jmp dword ptr [g_retRequeueKindC]
  }
}

typedef UPtr (__thiscall* NativeMapInsert)(void*,UPtr,UPtr,UPtr);
typedef UPtr (__thiscall* NativeNodeInsert)(void*,void**);
static NativeMapInsert g_origMapInsert=nullptr;
static NativeNodeInsert g_origNodeInsert=nullptr;
static NativeNodeInsert g_origThirdInsert=nullptr;
static bool IsResourceMap16Variant(UPtr map,U32 variant);
void AppendText(char*& cursor,const char* text,const char* end);
void AppendHex32(char*& cursor,U32 value,const char* end);
static void DiagPrint(const char* text);

#if defined(DSF_V29S_PENDING512)
static void CapacityMax(volatile long& destination,U32 value);
#include "V29sPendingMapCore.inc"
#endif

// Diagnostic-only EF lifecycle events. Producers do no I/O or allocation.
// A single monitor consumer formats and drains the bounded ring below.
constexpr U32 kEfEventSlots=4096u;
constexpr U32 kEfTrackedOwners=256u;
enum EfOp : U32 { kEfCreate=1u,kEfInsertBefore,kEfInsertAfter,kEfRemoveBefore,
                  kEfRemoveAfter,kEfDeleteBefore,kEfDeleteAfter,kEfFreeBefore,kEfFreeAfter,
                  kEfOwnerRemoveBefore,kEfOwnerRemoveAfter };
struct EfEvent {
    volatile long sequence;
    U32 tick,tid,op,owner,generation,caller,map,cursor,freeCount,active,node,key,handler,flags;
};
struct EfTracked { volatile long owner; volatile long generation; };
static EfEvent g_efEvents[kEfEventSlots]={};
static EfTracked g_efOwners[kEfTrackedOwners]={};
static volatile long g_efTotal=0;
static volatile long g_efGeneration=0;
extern "C" __declspec(dllexport) volatile long DSFV29m_EfEventDrops=0;
extern "C" __declspec(dllexport) volatile long DSFV29m_EfTrackDrops=0;
static U32 EfGeneration(UPtr owner) {
    for(U32 i=0;i<kEfTrackedOwners;++i)
        if(static_cast<UPtr>(g_efOwners[i].owner)==owner)
            return static_cast<U32>(g_efOwners[i].generation);
    return 0u;
}
static U32 EfTrack(UPtr owner) {
    const U32 gen=static_cast<U32>(InterlockedIncrement(&g_efGeneration));
    for(U32 i=0;i<kEfTrackedOwners;++i) {
        if(static_cast<UPtr>(g_efOwners[i].owner)==owner || g_efOwners[i].owner==0) {
            g_efOwners[i].generation=static_cast<long>(gen);
            InterlockedExchange(&g_efOwners[i].owner,static_cast<long>(owner));
            return gen;
        }
    }
    InterlockedIncrement(&DSFV29m_EfTrackDrops);
    return gen;
}
static void EfRecord(U32 op,UPtr owner,U32 gen,UPtr caller,UPtr node,U32 key,U32 handler,U32 flags) {
    const long seq=InterlockedIncrement(&g_efTotal);
    EfEvent& e=g_efEvents[static_cast<U32>(seq-1)%kEfEventSlots];
    e.sequence=0;
    e.tick=GetTickCount();e.tid=GetCurrentThreadId();e.op=op;
    e.owner=owner;e.generation=gen;e.caller=caller;e.map=owner+0x23Cu;
    const UPtr map=e.map;
    const bool canSample=op!=kEfFreeAfter && !(op==kEfDeleteAfter && (flags&1u));
    if(canSample && Readable(map,12u)) {e.cursor=Read32(map);e.freeCount=Read32(map+4u);e.active=Read32(map+8u);}
    else {e.cursor=0;e.freeCount=0;e.active=0;e.flags=flags|0x80000000u;}
    e.node=node;e.key=key;e.handler=handler;
    if(canSample && Readable(map,12u)) e.flags=flags;
    InterlockedExchange(&e.sequence,seq);
}
typedef UPtr (__thiscall* EfCtorFn)(void*);
typedef UPtr (__thiscall* EfRemoveFn)(void*,UPtr);
typedef UPtr (__thiscall* EfDeleteFn)(void*,U32);
typedef void (__cdecl* EfFreeFn)(void*);
static EfCtorFn g_origEfCtor=nullptr;
static EfRemoveFn g_origEfRemove=nullptr;
static EfDeleteFn g_origEfDelete=nullptr;
static EfFreeFn g_origEfFree=nullptr;
typedef UPtr (__thiscall* EfOwnerRemoveFn)(void*,UPtr);
static EfOwnerRemoveFn g_origEfOwnerRemove=nullptr;
static void EfResetMapSidecar(UPtr owner,bool release) {
    const UPtr map=owner+0x23Cu;
    while(InterlockedExchange(&g_resourceMap16Lock,1u)) {}
    for(auto& slot:g_resourceMap16) {
        if(slot.owner==map) {
            slot.used=0u;
            if(release) {
                slot.owner=0u;
                InterlockedIncrement(&DSFV29n_EFSidecarReleases);
            } else InterlockedIncrement(&DSFV29n_EFSidecarResets);
            break;
        }
    }
    InterlockedExchange(&g_resourceMap16Lock,0);
}
UPtr __fastcall EfCtorBridge(void* self,void*) {
    const UPtr caller=reinterpret_cast<UPtr>(_ReturnAddress());
    const UPtr result=g_origEfCtor(self);
    const UPtr owner=reinterpret_cast<UPtr>(self);
    if(IsResourceMap16Variant(owner+0x23Cu,0xEFu)) {
        EfResetMapSidecar(owner,false);
        EfRecord(kEfCreate,owner,EfTrack(owner),caller,0,0,0,0);
    }
    return result;
}
UPtr __fastcall EfRemoveBridge(void* self,void*,UPtr key) {
    const UPtr map=reinterpret_cast<UPtr>(self);
    const bool ef=IsResourceMap16Variant(map,0xEFu);
    const UPtr owner=map-0x23Cu;
    const UPtr caller=reinterpret_cast<UPtr>(_ReturnAddress());
    const U32 gen=ef?EfGeneration(owner):0u;
    if(ef)EfRecord(kEfRemoveBefore,owner,gen,caller,0,key,0,0);
    const UPtr result=g_origEfRemove(self,key);
    if(ef)EfRecord(kEfRemoveAfter,owner,gen,caller,0,key,0,result?1u:0u);
    return result;
}
UPtr __fastcall EfDeleteBridge(void* self,void*,U32 flags) {
    const UPtr owner=reinterpret_cast<UPtr>(self);
    const bool ef=IsResourceMap16Variant(owner+0x23Cu,0xEFu);
    const UPtr caller=reinterpret_cast<UPtr>(_ReturnAddress());
    const U32 gen=ef?EfGeneration(owner):0u;
    if(ef)EfRecord(kEfDeleteBefore,owner,gen,caller,0,0,0,flags);
    const UPtr result=g_origEfDelete(self,flags);
    if(ef)EfRecord(kEfDeleteAfter,owner,gen,caller,0,0,0,flags);
    return result;
}
UPtr __fastcall EfOwnerRemoveBridge(void* self,void*,UPtr item) {
    const UPtr owner=reinterpret_cast<UPtr>(self);
    const bool ef=IsResourceMap16Variant(owner+0x23Cu,0xEFu);
    const UPtr caller=reinterpret_cast<UPtr>(_ReturnAddress());
    const U32 gen=ef?EfGeneration(owner):0u;
    if(ef)EfRecord(kEfOwnerRemoveBefore,owner,gen,caller,0,item,
                   Readable(item,4u)?Read32(item):0u,0);
    const UPtr result=g_origEfOwnerRemove(self,item);
    if(ef)EfRecord(kEfOwnerRemoveAfter,owner,gen,caller,0,item,0,0);
    return result;
}
void __cdecl EfFreeBridge(void* pointer) {
    const UPtr owner=reinterpret_cast<UPtr>(pointer);
    const U32 gen=EfGeneration(owner);
    const UPtr caller=reinterpret_cast<UPtr>(_ReturnAddress());
    if(gen)EfRecord(kEfFreeBefore,owner,gen,caller,0,0,0,0);
    g_origEfFree(pointer);
    if(gen) {
        EfResetMapSidecar(owner,true);
        EfRecord(kEfFreeAfter,owner,gen,caller,0,0,0,0);
        for(U32 i=0;i<kEfTrackedOwners;++i)
            if(static_cast<UPtr>(g_efOwners[i].owner)==owner &&
               static_cast<U32>(g_efOwners[i].generation)==gen) {
                InterlockedExchange(&g_efOwners[i].owner,0);
                break;
            }
    }
}

// The exact-build ED and EF constructors share this map geometry and vtable.
static bool IsResourceMap16Variant(UPtr map, U32 variant) {
    if (map <= 0x23Cu) return false;
    const UPtr owner = map - 0x23Cu;
    return Readable(owner, 0x238u) &&
           Read32(owner) == Address(0x00E3966Cu) &&
           Read32(owner + 0x234u) == variant;
}
static bool IsResourceMap16(UPtr map) {
    return IsResourceMap16Variant(map, 0xEDu) ||
           IsResourceMap16Variant(map, 0xEFu);
}

static ResourceMap16Sidecar* FindResourceMap16(UPtr map) {
    ResourceMap16Sidecar* vacant = nullptr;
    for (auto& slot : g_resourceMap16) {
        if (slot.owner == map) return &slot;
        if (!slot.owner && !vacant) vacant = &slot;
    }
    if (vacant) {
        vacant->owner = map;
        vacant->used = 0u;
        ++DSFV29i_Map16OwnersSeen;
    }
    return vacant;
}

UPtr __fastcall MapInsertBridge(void* self,void*,UPtr a,UPtr b,UPtr c) {
    const UPtr map=reinterpret_cast<UPtr>(self);
#if defined(DSF_V29S_PENDING512)
    if(PendingOwns(map))return PendingInsert(map,a,b,c);
#endif
    const bool ef=IsResourceMap16Variant(map,0xEFu);
    const UPtr efOwner=map-0x23Cu;
    const U32 efGen=ef?EfGeneration(efOwner):0u;
    const UPtr efCaller=ef?reinterpret_cast<UPtr>(_ReturnAddress()):0u;
    if(ef)EfRecord(kEfInsertBefore,efOwner,efGen,efCaller,0,a,b,c);
    if (ef && Readable(map, 12u) &&
        Read32(map + 4u) == 0u && Read32(map) >= map + 0x2DCu) {
        DSFV29k_EFMap = static_cast<long>(map);
        DSFV29k_EFCursor = static_cast<long>(Read32(map));
        DSFV29k_EFActive = static_cast<long>(Read32(map + 8u));
        if(InterlockedIncrement(&DSFV29k_EFBoundaryHits)==1) {
            // One emergency line survives a crash before the monitor drains.
            char line[190]={};char* p=line;const char* end=line+sizeof(line)-1;
            AppendText(p,"DSFV29m_EF_FIRST_BOUNDARY owner=",end);AppendHex32(p,efOwner,end);
            AppendText(p," map=",end);AppendHex32(p,map,end);
            AppendText(p," cursor=",end);AppendHex32(p,Read32(map),end);
            AppendText(p," active=",end);AppendHex32(p,Read32(map+8u),end);
            AppendText(p," free=",end);AppendHex32(p,Read32(map+4u),end);
            AppendText(p,"\n",end);*p=0;DiagPrint(line);
        }
        if (IsDebuggerPresent() && DSFV29l_EFHoldEnabled != 0) {
            OutputDebugStringA("DSFV29n_EF_HOLD: EF sidecar redirect pending; set EFRelease=1 to continue.\n");
            while (DSFV29k_EFRelease==0) Sleep(10);
            InterlockedExchange(&DSFV29k_EFRelease,0);
        }
    }
    // ED/EF: 45 inline 16-byte nodes, then a separate static arena per map.
    // Other map classes remain on their original native path.
    if (IsResourceMap16(map)) {
        const UPtr inlineBegin = map + 0xCu;
        const UPtr inlineEnd = map + 0x2DCu;
        const U32 freeCount = Read32(map + 4u);
        const UPtr cursor = Read32(map);
        const U32 active = Read32(map + 8u);
        if (static_cast<long>(active) > DSFV29i_Map16ActiveHighWater)
            DSFV29i_Map16ActiveHighWater = static_cast<long>(active);
        if (ef && static_cast<long>(active) > DSFV29n_EFActiveHighWater)
            DSFV29n_EFActiveHighWater = static_cast<long>(active);
        // Native free-list reuse is unaffected: it does not advance cursor.
        if (freeCount == 0u && !(cursor >= inlineBegin && cursor < inlineEnd)) {
            // Protect both the initial overflow and every subsequent external
            // allocation. The native routine has no bounds check at this site.
            while (InterlockedExchange(&g_resourceMap16Lock, 1u)) {}
            ResourceMap16Sidecar* slot = FindResourceMap16(map);
            if (!slot) {
                DSFV29i_Map16Guard = 1;
                if(ef) DSFV29n_EFGuard = 1;
                DSFV27B_UnsafeBoundary = 32;
                InterlockedExchange(&g_resourceMap16Lock, 0);
                Log("DSFV29i: FATAL resource-map16 owner sidecars exhausted. STOP.\n");
                return 0;
            }
            if (cursor == inlineEnd) {
                // At 45 allocations/free-list empty there are 45 active
                // nodes. A new object at the same address can reset the
                // slot here without recycling native node pointers in place.
                if (active != 45u) {
                    DSFV29i_Map16Guard = 2;
                    if(ef) DSFV29n_EFGuard = 2;
                    DSFV27B_UnsafeBoundary = 33;
                    InterlockedExchange(&g_resourceMap16Lock, 0);
                    Log("DSFV29i: FATAL resource-map16 inline boundary/count mismatch. STOP.\n");
                    return 0;
                }
                slot->used = 0u;
            }
            if (slot->used >= kResourceMap16ExtraNodes) {
                DSFV29i_Map16Guard = 3;
                if(ef) DSFV29n_EFGuard = 3;
                DSFV27B_UnsafeBoundary = 34;
                InterlockedExchange(&g_resourceMap16Lock, 0);
                Log("DSFV29i: FATAL resource-map16 external arena exhausted. STOP.\n");
                return 0;
            }
            const UPtr next = reinterpret_cast<UPtr>(&slot->nodes[slot->used]);
            if (cursor != inlineEnd && cursor != next) {
                DSFV29i_Map16Guard = 4;
                if(ef) DSFV29n_EFGuard = 4;
                DSFV27B_UnsafeBoundary = 35;
                InterlockedExchange(&g_resourceMap16Lock, 0);
                Log("DSFV29i: FATAL resource-map16 cursor is not the expected sidecar position. STOP.\n");
                return 0;
            }
            Write32(map, static_cast<U32>(next));
            const UPtr result = g_origMapInsert(self, a, b, c);
            if (Read32(map) == next + sizeof(Node16)) {
                ++slot->used;
                ++DSFV29i_Map16ExtraInserts;
                if(ef) ++DSFV29n_EFExtraInserts;
            } else {
                DSFV29i_Map16Guard = 5;
                if(ef) DSFV29n_EFGuard = 5;
                DSFV27B_UnsafeBoundary = 36;
                Log("DSFV29i: FATAL resource-map16 native insertion did not advance by 16. STOP.\n");
            }
            InterlockedExchange(&g_resourceMap16Lock, 0);
            if(ef)EfRecord(kEfInsertAfter,efOwner,efGen,efCaller,result,a,b,c);
            return result;
        }
    }
    if (g_pkg0 && map==g_pkg0+0x5B44u && Readable(map,12u)) {
        const U32 free=Read32(map+4u), cursor=Read32(map), active=Read32(map+8u);
        if(static_cast<long>(active)>DSFV27B_MapMaxActive) DSFV27B_MapMaxActive=static_cast<long>(active);
        const UPtr boundary=g_pkg0+0x5F50u;
        const UPtr lo=reinterpret_cast<UPtr>(&g_mapExtra[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&g_mapExtra[kMapExtraNodes]);
        if(free==0 && cursor==boundary) {
            Write32(map, static_cast<U32>(lo));
            DSFV27B_MapExternal=1;
            Log("DSFV29b: type0 completed-map allocator entered external node arena.\n");
        } else if(free==0 && cursor>=lo && cursor>=hi) {
            DSFV27B_UnsafeBoundary=1;
            Log("DSFV29b: FATAL completed-map sidecar exhausted; stop process.\n");
            return 0;
        }
    }
    const UPtr result=g_origMapInsert(self,a,b,c);
    if(ef)EfRecord(kEfInsertAfter,efOwner,efGen,efCaller,result,a,b,c);
    return result;
}

// Selector node sidecar, same first-45-inline-node algorithm as V20, but
// enlarged to 160 extra 12-byte nodes per owner and with pointer provenance.
SelectorSidecar* SelectorSlot(UPtr list) {
    SelectorSidecar* empty=nullptr;
    for(auto& slot:g_selSidecars){
        if(slot.owner==list) return &slot;
        if(!slot.owner && !empty) empty=&slot;
    }
    if(empty){ empty->owner=list; empty->used=0; }
    return empty;
}
ThirdSidecar* ThirdSlot(UPtr owner) {
    ThirdSidecar* empty=nullptr;
    for(auto& slot:g_thirdSidecars){
        if(slot.owner==owner) return &slot;
        if(!slot.owner && !empty) empty=&slot;
    }
    if(empty){ empty->owner=owner; empty->used=0; }
    return empty;
}
UPtr __fastcall NodeInsertBridge(void* self,void*,void** item) {
    const UPtr list=reinterpret_cast<UPtr>(self);
    const bool owner1=list>0x18u && Readable(list-0x18u,4u) &&
        Read32(list-0x18u)==Address(0x00E395D0u);
    const bool owner2=!owner1 && list>4u && Readable(list-4u,4u) &&
        Read32(list-4u)==Address(0x00E3966Cu);
    if(!owner1 && !owner2) return g_origNodeInsert(self,item);
    if(Read32(list+4u)!=0u) return g_origNodeInsert(self,item);
    const UPtr cur=Read32(list),end=list+0x228u;
    if(cur>=list+0xCu && cur<end) return g_origNodeInsert(self,item);
    const U32 sz=Read32(list+0x230u);
    const bool validSize=owner1 ? sz==0xECu : (sz==0xEDu || sz==0xEFu);
    if(cur==end && (Read32(list+8u)!=45u || !validSize)) {
        DSFV27B_UnsafeBoundary=8; return 0;
    }
    SelectorLock();
    SelectorSidecar* slot=SelectorSlot(list);
    if(!slot){ DSFV27B_UnsafeBoundary=2;SelectorUnlock();return 0; }
    if(cur==end) slot->used=0;
    if(slot->used>=kSelectorExtraNodes){SelectorUnlock();DSFV27B_UnsafeBoundary=3;return 0;}
    const UPtr next=reinterpret_cast<UPtr>(&slot->nodes[slot->used]);
    if((cur!=end && cur!=next)){
        DSFV27B_UnsafeBoundary=3;SelectorUnlock();return 0;
    }
    Write32(list,static_cast<U32>(next));
    UPtr out=g_origNodeInsert(self,item);
    if(Read32(list)==next+12u) {
        ++slot->used; ++DSFV27B_SelectorOverflow;
    } else DSFV27B_UnsafeBoundary=4;
    SelectorUnlock();
    return out;
}
UPtr __fastcall ThirdInsertBridge(void* self,void*,void** item) {
    const UPtr owner=reinterpret_cast<UPtr>(self);
    if(!Readable(owner,16u) || Read32(owner)!=Address(0x00DDA058u) ||
       Read32(owner+8u)!=0u) return g_origThirdInsert(self,item);
    const UPtr cur=Read32(owner+4u),end=owner+0x238u;
    if(cur>=owner+0x10u && cur<end) return g_origThirdInsert(self,item);
    if(cur==end && Read32(owner+0xCu)!=46u){
        DSFV27B_UnsafeBoundary=9;return 0;
    }
    ThirdLock();
    ThirdSidecar* slot=ThirdSlot(owner);
    if(!slot){DSFV27B_UnsafeBoundary=5;ThirdUnlock();return 0;}
    if(cur==end) slot->used=0;
    if(slot->used>=kThirdExtraNodes){ThirdUnlock();DSFV27B_UnsafeBoundary=6;return 0;}
    const UPtr next=reinterpret_cast<UPtr>(&slot->nodes[slot->used]);
    if((cur!=end && cur!=next)){
        DSFV27B_UnsafeBoundary=6;ThirdUnlock();return 0;
    }
    Write32(owner+4u,static_cast<U32>(next));
    UPtr out=g_origThirdInsert(self,item);
    if(Read32(owner+4u)==next+12u){ ++slot->used;++DSFV27B_ThirdOverflow; }
    else DSFV27B_UnsafeBoundary=7;
    ThirdUnlock();
    return out;
}


// V20's generic 45-entry selector-pointer array: redirect extra pointers
// into sidecar while preserving the stock in-object count at exactly 45.
// The matching find/remove/reset/ctor/dtor all participate in the lifecycle.
constexpr U32 kStockSelectorSlots=45u;
constexpr U32 kExtraSelectorSlots=160u;
struct SelectorArraySidecar { UPtr owner; U32 count; UPtr overflow[kExtraSelectorSlots]; };
static SelectorArraySidecar g_arrSidecars[96] = {};
static volatile long g_arrLock=0;
extern "C" __declspec(dllexport) volatile long DSFV27B_ArrayOverflowInsert=0;
extern "C" __declspec(dllexport) volatile long DSFV27B_ArrayOverflowFind=0;
extern "C" __declspec(dllexport) volatile long DSFV27B_ArrayOverflowRemove=0;
void ArrayLock(){while(InterlockedExchange(&g_arrLock,1u)){} }
void ArrayUnlock(){InterlockedExchange(&g_arrLock,0);}
SelectorArraySidecar* ArraySlot(UPtr owner,bool create){
    SelectorArraySidecar* empty=nullptr;
    for(auto& slot:g_arrSidecars){
        if(slot.owner==owner)return &slot;
        if(!slot.owner&&!empty)empty=&slot;
    }
    if(create && empty){empty->owner=owner;empty->count=0;}
    return create ? empty:nullptr;
}
void ArrayClear(UPtr owner){
    ArrayLock();
    if(auto* slot=ArraySlot(owner,false)){slot->count=0;slot->owner=0;}
    ArrayUnlock();
}
using ArrayInsertFn=void* (__thiscall*)(void*,void**);
using ArrayFindFn=void* (__thiscall*)(void*,void**,U32,U32);
using ArrayRemoveFn=UPtr (__thiscall*)(void*,void**);
using ArrayResetFn=UPtr (__thiscall*)(void*);
using ArrayCtorFn=void* (__thiscall*)(void*);
using ArrayDtorFn=void* (__thiscall*)(void*,U32);
using ArrayLookupFn=void* (__thiscall*)(void*,U32,U32);
static ArrayInsertFn g_origArrayInsert=nullptr;
static ArrayFindFn g_origArrayFind=nullptr;
static ArrayRemoveFn g_origArrayRemove=nullptr;
static ArrayResetFn g_origArrayReset=nullptr;
static ArrayCtorFn g_origArrayCtor=nullptr;
static ArrayDtorFn g_origArrayDtor=nullptr;
void* __fastcall ArrayCtorBridge(void* self,void*){
    void* ret=g_origArrayCtor(self); ArrayClear(reinterpret_cast<UPtr>(self)); return ret;
}
UPtr __fastcall ArrayResetBridge(void* self,void*){
    UPtr ret=g_origArrayReset(self); ArrayClear(reinterpret_cast<UPtr>(self)); return ret;
}
void* __fastcall ArrayDtorBridge(void* self,void*,U32 flags){
    void* ret=g_origArrayDtor(self,flags); ArrayClear(reinterpret_cast<UPtr>(self)); return ret;
}
void* __fastcall ArrayInsertBridge(void* self,void*,void** out){
    const UPtr obj=reinterpret_cast<UPtr>(self);
    const U32 count=Read32(obj+0xB8u);
    if(count<kStockSelectorSlots || Read32(obj)!=Address(0x00E18184u))
        return g_origArrayInsert(self,out);
    if(count!=kStockSelectorSlots) {DSFV27B_UnsafeBoundary=10; if(out)*out=nullptr;return out;}
    ArrayLock();
    auto* slot=ArraySlot(obj,true);
    if(!slot||slot->count>=kExtraSelectorSlots){ArrayUnlock();DSFV27B_UnsafeBoundary=11;if(out)*out=nullptr;return out;}
    UPtr* inlineArray=reinterpret_cast<UPtr*>(obj+4u);
    slot->overflow[slot->count++]=inlineArray[kStockSelectorSlots-1u];
    Write32(obj+0xB8u,kStockSelectorSlots-1u);
    ArrayUnlock();
    void* ret=g_origArrayInsert(self,out);
    if(Read32(obj+0xB8u)!=kStockSelectorSlots)DSFV27B_UnsafeBoundary=12;
    else ++DSFV27B_ArrayOverflowInsert;
    return ret;
}
void* __fastcall ArrayFindBridge(void* self,void*,void** out,U32 k1,U32 k2){
    void* ret=g_origArrayFind(self,out,k1,k2);
    if(!out||*out)return ret;
    const UPtr obj=reinterpret_cast<UPtr>(self);
    UPtr saved[kExtraSelectorSlots]={};U32 count=0;
    ArrayLock();
    if(auto* slot=ArraySlot(obj,false)){
        count=slot->count;
        if(count>kExtraSelectorSlots)count=kExtraSelectorSlots;
        for(U32 i=0;i<count;++i)saved[i]=slot->overflow[i];
    }
    ArrayUnlock();
    ArrayLookupFn lookup=reinterpret_cast<ArrayLookupFn>(Address(0x006AF970u));
    for(U32 i=0;i<count;++i){
        if(void* found=lookup(reinterpret_cast<void*>(saved[i]),k1,k2)){
            *out=found;++DSFV27B_ArrayOverflowFind;return out;
        }
    }
    return ret;
}
UPtr __fastcall ArrayRemoveBridge(void* self,void*,void** item){
    if(!item)return g_origArrayRemove(self,item);
    const UPtr obj=reinterpret_cast<UPtr>(self);
    const UPtr target=reinterpret_cast<UPtr>(*item);
    U32 overflowIndex=kExtraSelectorSlots;
    UPtr displaced=0;
    bool inlineTarget=false;
    ArrayLock();
    auto* slot=ArraySlot(obj,false);
    if(slot&&slot->count&&Read32(obj+0xB8u)==kStockSelectorSlots){
        auto* inlineArray=reinterpret_cast<UPtr*>(obj+4u);
        for(U32 i=0;i<slot->count;++i)
            if(slot->overflow[i]==target){overflowIndex=i;break;}
        if(overflowIndex<kExtraSelectorSlots){
            displaced=inlineArray[kStockSelectorSlots-1u];
            inlineArray[kStockSelectorSlots-1u]=target;
            slot->overflow[overflowIndex]=displaced;
        } else {
            for(U32 i=0;i<kStockSelectorSlots;++i)
                if(inlineArray[i]==target){inlineTarget=true;break;}
        }
    }
    ArrayUnlock();
    UPtr ret=g_origArrayRemove(self,item);
    if(overflowIndex<kExtraSelectorSlots||inlineTarget){
        ArrayLock();slot=ArraySlot(obj,false);
        if(slot&&slot->count&&Read32(obj+0xB8u)==kStockSelectorSlots-1u){
            auto* inlineArray=reinterpret_cast<UPtr*>(obj+4u);
            inlineArray[kStockSelectorSlots-1u] = overflowIndex<kExtraSelectorSlots ?
                displaced : slot->overflow[slot->count-1u];
            Write32(obj+0xB8u,kStockSelectorSlots);
            if(overflowIndex<kExtraSelectorSlots)
                slot->overflow[overflowIndex]=slot->overflow[slot->count-1u];
            --slot->count;++DSFV27B_ArrayOverflowRemove;
        } else DSFV27B_UnsafeBoundary=13;
        ArrayUnlock();
    }
    return ret;
}

// push imm8 sign-extends, so 0xC0 would mean -64, NOT 192.
// Replace the full original sequences with a JMP to these two code stubs;
// each uses push imm32 192 and reproduces the overwritten instructions.
// Class-0x16 allocator capacity: 192 (expected handler count: 154).
extern "C" UPtr g_retClass16A=0;
extern "C" UPtr g_retClass16B=0;
extern "C" __declspec(naked) void HookClass16A(){
  __asm {
    push 192
    push 0x16
    mov dword ptr [esi+0x23c], eax
    jmp dword ptr [g_retClass16A]
  }
}
extern "C" __declspec(naked) void HookClass16B(){
  __asm {
    push 192
    push 0x16
    mov dword ptr [esi+0x8], ecx
    mov dword ptr [esi+0xC], ecx
    jmp dword ptr [g_retClass16B]
  }
}

struct PatchSite { UPtr original; const U8* expected; U32 size; void* bridge;
                   UPtr* resume; void** trampoline; U8 saved[12]; bool installed; };
// Offline-generated stock signatures from the SHA-verified game executable.
static const U8 kSigGlobalCreate[] = {0x8D, 0x8C, 0x8E, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigGlobalLast[] = {0x8B, 0x8C, 0x8E, 0x2C, 0x56, 0x00, 0x00};
static const U8 kSigInitKind[] = {0x8D, 0x04, 0x8F, 0x85, 0xC0, 0x74, 0x0F};
static const U8 kSigLoopGlobalA[] = {0x8D, 0xB1, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigLoopGlobalB[] = {0x8D, 0xB1, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigFindGlobal[] = {0x81, 0xC1, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigDestroyGlobalA[] = {0x8D, 0x9E, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigDestroyGlobalB[] = {0x8D, 0x86, 0x30, 0x56, 0x00, 0x00};
static const U8 kSigPopKind[] = {0x8B, 0x8C, 0x88, 0x30, 0x57, 0x00, 0x00};
static const U8 kSigAppendKind[] = {0x8D, 0x0C, 0x88, 0x5E, 0x85, 0xC9};
static const U8 kSigRequeueKindA[] = {0x8D, 0x0C, 0x88, 0x85, 0xC9, 0x74, 0x06};
static const U8 kSigRequeueKindB[] = {0x8D, 0x0C, 0x88, 0x85, 0xC9, 0x74, 0x02};
static const U8 kSigRequeueKindC[] = {0x8D, 0x14, 0x90, 0x85, 0xD2, 0x74, 0x04};
static const U8 kSigRequestWrite[] = {0x56, 0x8B, 0x74, 0x24, 0x0C};
static const U8 kSigRequestRead[] = {0x8B, 0x86, 0x34, 0x03, 0x00, 0x00, 0x0F, 0xB7, 0x3C, 0xAE};
static const U8 kSigMapInsert[] = {0x83, 0x79, 0x04, 0x00, 0x8B, 0x11};
static const U8 kSigEfCtor[] = {0x8B,0xC1,0x8D,0x48,0x10};
static const U8 kSigEfRemove[] = {0x8B,0x44,0x24,0x04,0x83,0xEC,0x08};
static const U8 kSigEfDelete[] = {0xF6,0x44,0x24,0x04,0x01};
static const U8 kSigEfFree[] = {0x8B,0xFF,0x55,0x8B,0xEC};
static const U8 kSigEfOwnerRemove[] = {0x8B,0x81,0x30,0x02,0x00,0x00};
static const U8 kSigNodeInsert[] = {0x83, 0x79, 0x04, 0x00, 0x8B, 0x01};
static const U8 kSigThirdInsert[] = {0x51, 0x56, 0x57, 0x8D, 0x44, 0x24, 0x08};

static const U8 kSigArrayCtor[] = {0x8B, 0xC1, 0xC7, 0x00, 0x84, 0x81, 0xE1, 0x00};
static const U8 kSigArrayReset[] = {0xB8, 0x01, 0x01, 0x01, 0x01};
static const U8 kSigArrayDtor[] = {0x53, 0x56, 0x8B, 0xD9, 0x57};
static const U8 kSigArrayInsert[] = {0x51, 0x56, 0x57, 0x8D, 0x44, 0x24, 0x08};
static const U8 kSigArrayRemove[] = {0x8B, 0x81, 0xB8, 0x00, 0x00, 0x00};
static const U8 kSigArrayFind[] = {0x8B, 0x81, 0xB8, 0x00, 0x00, 0x00};
static const U8 kSigClass16A[] = {0x6A, 0x2E, 0x6A, 0x16, 0x89, 0x86, 0x3C, 0x02, 0x00, 0x00};
static const U8 kSigClass16B[] = {0x6A, 0x2E, 0x6A, 0x16, 0x89, 0x4E, 0x08, 0x89, 0x4E, 0x0C};
static PatchSite g_sites[] = {
  {0x5A79F0u,kSigRequestWrite,5u,reinterpret_cast<void*>(&RequestWriteBridge),nullptr,nullptr,{},false},
  {0x5A7520u,kSigRequestRead,10u,reinterpret_cast<void*>(&HookRequestRead),&g_retRequestRead,nullptr,{},false},
  {0x5A14A1u,kSigGlobalCreate,7u,reinterpret_cast<void*>(&HookGlobalCreate),&g_retGlobalCreate,nullptr,{},false},
  {0x5A14CBu,kSigGlobalLast,7u,reinterpret_cast<void*>(&HookGlobalLast),&g_retGlobalLast,nullptr,{},false},
  {0x5A14BEu,kSigInitKind,7u,reinterpret_cast<void*>(&HookInitKind),&g_retInitKind,nullptr,{},false},
  {0x5A0CF8u,kSigLoopGlobalA,6u,reinterpret_cast<void*>(&HookLoopGlobalA),&g_retLoopGlobalA,nullptr,{},false},
  {0x5A0D39u,kSigLoopGlobalB,6u,reinterpret_cast<void*>(&HookLoopGlobalB),&g_retLoopGlobalB,nullptr,{},false},
  {0x5A0DA6u,kSigFindGlobal,6u,reinterpret_cast<void*>(&HookFindGlobal),&g_retFindGlobal,nullptr,{},false},
  {0x5A250Au,kSigDestroyGlobalA,6u,reinterpret_cast<void*>(&HookDestroyGlobalA),&g_retDestroyGlobalA,nullptr,{},false},
  {0x5A2572u,kSigDestroyGlobalB,6u,reinterpret_cast<void*>(&HookDestroyGlobalB),&g_retDestroyGlobalB,nullptr,{},false},
  {0x5A0CABu,kSigPopKind,7u,reinterpret_cast<void*>(&HookPopKind),&g_retPopKind,nullptr,{},false},
  {0x5A0F41u,kSigAppendKind,6u,reinterpret_cast<void*>(&HookAppendKind),&g_retAppendKind,nullptr,{},false},
  {0x5A1E2Du,kSigRequeueKindA,7u,reinterpret_cast<void*>(&HookRequeueKindA),&g_retRequeueKindA,nullptr,{},false},
  {0x5A2155u,kSigRequeueKindB,7u,reinterpret_cast<void*>(&HookRequeueKindB),&g_retRequeueKindB,nullptr,{},false},
  {0x5A2338u,kSigRequeueKindC,7u,reinterpret_cast<void*>(&HookRequeueKindC),&g_retRequeueKindC,nullptr,{},false},
  {0x6AE790u,kSigArrayCtor,8u,reinterpret_cast<void*>(&ArrayCtorBridge),nullptr,reinterpret_cast<void**>(&g_origArrayCtor),{},false},
  {0x6AE140u,kSigArrayReset,5u,reinterpret_cast<void*>(&ArrayResetBridge),nullptr,reinterpret_cast<void**>(&g_origArrayReset),{},false},
  {0x6AE8D0u,kSigArrayDtor,5u,reinterpret_cast<void*>(&ArrayDtorBridge),nullptr,reinterpret_cast<void**>(&g_origArrayDtor),{},false},
  {0x6AE160u,kSigArrayInsert,7u,reinterpret_cast<void*>(&ArrayInsertBridge),nullptr,reinterpret_cast<void**>(&g_origArrayInsert),{},false},
  {0x6AE1B0u,kSigArrayRemove,6u,reinterpret_cast<void*>(&ArrayRemoveBridge),nullptr,reinterpret_cast<void**>(&g_origArrayRemove),{},false},
  {0x6ADF90u,kSigArrayFind,6u,reinterpret_cast<void*>(&ArrayFindBridge),nullptr,reinterpret_cast<void**>(&g_origArrayFind),{},false},
  {0x48F85Du,kSigClass16A,10u,reinterpret_cast<void*>(&HookClass16A),&g_retClass16A,nullptr,{},false},
  {0x48FBB8u,kSigClass16B,10u,reinterpret_cast<void*>(&HookClass16B),&g_retClass16B,nullptr,{},false},
  {0x7D7270u,kSigMapInsert,6u,reinterpret_cast<void*>(&MapInsertBridge),nullptr,reinterpret_cast<void**>(&g_origMapInsert),{},false},
  {0x86B4A0u,kSigEfCtor,5u,reinterpret_cast<void*>(&EfCtorBridge),nullptr,reinterpret_cast<void**>(&g_origEfCtor),{},false},
  {0x556FB0u,kSigEfRemove,7u,reinterpret_cast<void*>(&EfRemoveBridge),nullptr,reinterpret_cast<void**>(&g_origEfRemove),{},false},
  {0x791990u,kSigEfDelete,5u,reinterpret_cast<void*>(&EfDeleteBridge),nullptr,reinterpret_cast<void**>(&g_origEfDelete),{},false},
  {0xC2875Au,kSigEfFree,5u,reinterpret_cast<void*>(&EfFreeBridge),nullptr,reinterpret_cast<void**>(&g_origEfFree),{},false},
  {0x86B750u,kSigEfOwnerRemove,6u,reinterpret_cast<void*>(&EfOwnerRemoveBridge),nullptr,reinterpret_cast<void**>(&g_origEfOwnerRemove),{},false},
  {0x86B6B0u,kSigNodeInsert,6u,reinterpret_cast<void*>(&NodeInsertBridge),nullptr,reinterpret_cast<void**>(&g_origNodeInsert),{},false},
  {0x48F7D0u,kSigThirdInsert,7u,reinterpret_cast<void*>(&ThirdInsertBridge),nullptr,reinterpret_cast<void**>(&g_origThirdInsert),{},false},
};

static U8* g_codeStubs=nullptr;
static unsigned g_stubUsed=0;
void RestoreExtendedSites(){
    for(unsigned j=sizeof(g_sites)/sizeof(g_sites[0]);j>0;--j){
        PatchSite& p=g_sites[j-1];
        if(!p.installed)continue;
        UPtr va=Address(p.original);
        DWORD prot=0,unused=0;
        if(VirtualProtect(reinterpret_cast<void*>(va),p.size,kPageExecuteReadWrite,&prot)){
            for(unsigned i=0;i<p.size;++i)
                *reinterpret_cast<volatile U8*>(va+i)=p.saved[i];
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(va),p.size);
            VirtualProtect(reinterpret_cast<void*>(va),p.size,prot,&unused);
        }
        p.installed=false;
    }
}

// Expand the request processing function's stack-local 12-byte list from
// 50 to 150 elements. Original frame 0x270 and sentinel at ESP+0x278;
// new frame 0x720 and sentinel at ESP+0x728. The sentinel+4 is at 0x72C.
// Exact sites extracted from the SHA-matched 0x005A7490..0x005A785E routine.
struct RequestFramePatch {
    UPtr at; U8 length; U8 immOffset; U8 stock[7]; U32 replacement; bool done;
};
static RequestFramePatch g_requestFramePatches[] = {
  {0x5A7495u,6u,2u,{0x81,0xEC,0x70,0x02,0x00,0x00},0x720u,false},
  {0x5A74E0u,7u,3u,{0x8D,0x94,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A74F9u,7u,3u,{0x89,0x94,0x24,0x7C,0x02,0x00,0x00},0x72Cu,false},
  {0x5A7500u,7u,3u,{0x89,0x8C,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A75F2u,7u,3u,{0x8B,0x8C,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A75FFu,7u,3u,{0x8D,0x84,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A7609u,7u,3u,{0x8B,0x8C,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A7613u,7u,3u,{0x89,0x94,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A767Au,7u,3u,{0x8B,0x84,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A7687u,7u,3u,{0x8D,0x94,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A7691u,7u,3u,{0x8B,0x84,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A769Bu,7u,3u,{0x89,0x8C,0x24,0x78,0x02,0x00,0x00},0x728u,false},
  {0x5A7858u,6u,2u,{0x81,0xC4,0x70,0x02,0x00,0x00},0x720u,false},
};
void RestoreRequestFramePatches() {
    for (unsigned i=sizeof(g_requestFramePatches)/sizeof(g_requestFramePatches[0]);i>0;--i) {
        RequestFramePatch& p=g_requestFramePatches[i-1];
        if(!p.done)continue;
        DWORD old=0,ignored=0;
        UPtr at=Address(p.at);
        if(VirtualProtect(reinterpret_cast<void*>(at),p.length,kPageExecuteReadWrite,&old)) {
            for(unsigned j=0;j<p.length;++j)
                *reinterpret_cast<volatile U8*>(at+j)=p.stock[j];
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),p.length);
            VirtualProtect(reinterpret_cast<void*>(at),p.length,old,&ignored);
        }
        p.done=false;
    }
    DSFV27B_RequestFramePatchCount=0;
}

// V27b regression fix: only the SECOND merge of 0x005A7490 uses the
// expanded stack-local source list. 0x005A7D50 assumes a 50-element
// source: source+0x264 sentinel, source+0x268 first node.  The widened
// stack source has source+0x714 sentinel and +0x718 first node.
// The FIRST merge at 0x005A775D must retain its original 50-node layout.
// Clone the helper unchanged except for the two SOURCE offsets, and point
// only the 0x005A7769 CALL at the clone.  No relative branches leave the
// helper (verified against target exe); thus whole-function copy is valid.
constexpr UPtr kMergeHelperVA = 0x005A7D50u;
constexpr U32 kMergeHelperLength = 0xDFu; // includes RET 4 at 0x005A7E2C
constexpr UPtr kMergeSecondCallVA = 0x005A7769u;
static const U8 kStockSecondCall[5] = {0xE8u,0xE2u,0x05u,0x00u,0x00u};
static void* g_merge150Clone = nullptr;
static bool g_merge150CallInstalled = false;
extern "C" __declspec(dllexport) volatile long DSFV27B_Merge150Fix = 0;

void RestoreMerge150Fix() {
    if (g_merge150CallInstalled) {
        const UPtr callsite=Address(kMergeSecondCallVA);
        DWORD old=0,unused=0;
        if (VirtualProtect(reinterpret_cast<void*>(callsite),5u,kPageExecuteReadWrite,&old)) {
            for (U32 i=0;i<5u;++i)
                *reinterpret_cast<volatile U8*>(callsite+i)=kStockSecondCall[i];
            FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(callsite),5u);
            VirtualProtect(reinterpret_cast<void*>(callsite),5u,old,&unused);
        }
        g_merge150CallInstalled=false;
    }
    if (g_merge150Clone) {
        VirtualFree(g_merge150Clone,0,kMemRelease);
        g_merge150Clone=nullptr;
    }
    DSFV27B_Merge150Fix=0;
}

bool InstallMerge150Fix() {
    const UPtr helper=Address(kMergeHelperVA);
    const UPtr callsite=Address(kMergeSecondCallVA);
    if (!Readable(helper,kMergeHelperLength) || !Readable(callsite,5u)) return false;
    for (U32 i=0;i<5u;++i)
        if (*reinterpret_cast<volatile const U8*>(callsite+i)!=kStockSecondCall[i]) return false;

    const U8 sourceLoad[] = {0x8Bu,0x96u,0x68u,0x02u,0x00u,0x00u};
    const U8 sourceSentinel[] = {0x8Du,0xAEu,0x64u,0x02u,0x00u,0x00u};
    for (U32 i=0;i<6u;++i) {
        if (*reinterpret_cast<volatile const U8*>(Address(0x005A7D5Eu)+i)!=sourceLoad[i] ||
            *reinterpret_cast<volatile const U8*>(Address(0x005A7D64u)+i)!=sourceSentinel[i]) return false;
    }
    const U8 retPattern[]={0xC2u,0x04u,0x00u};
    for (U32 i=0;i<3u;++i)
        if (*reinterpret_cast<volatile const U8*>(Address(0x005A7E2Cu)+i)!=retPattern[i]) return false;

    U8* clone=reinterpret_cast<U8*>(VirtualAlloc(nullptr,0x1000u,kMemCommitReserve,kPageExecuteReadWrite));
    if (!clone) return false;
    for (U32 i=0;i<kMergeHelperLength;++i)
        clone[i]=*reinterpret_cast<volatile const U8*>(helper+i);
    // Only SOURCE displacements, NOT the destination+0x264/+0x268 fields.
    *reinterpret_cast<U32*>(clone+(0x005A7D60u-kMergeHelperVA))=0x718u;
    *reinterpret_cast<U32*>(clone+(0x005A7D66u-kMergeHelperVA))=0x714u;
    if (!FlushInstructionCache(GetCurrentProcess(),clone,kMergeHelperLength)) {
        VirtualFree(clone,0,kMemRelease);
        return false;
    }
    DWORD old=0,unused=0;
    if (!VirtualProtect(reinterpret_cast<void*>(callsite),5u,kPageExecuteReadWrite,&old)) {
        VirtualFree(clone,0,kMemRelease);
        return false;
    }
    // Rel32 CALL. All addresses are 32-bit in the target process.
    U32 target=reinterpret_cast<UPtr>(clone);
    *reinterpret_cast<volatile U8*>(callsite)=0xE8u;
    Write32(callsite+1u,target-static_cast<U32>(callsite+5u));
    const bool flush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(callsite),5u)!=0;
    const bool restored=VirtualProtect(reinterpret_cast<void*>(callsite),5u,old,&unused)!=0;
    g_merge150Clone=clone;
    g_merge150CallInstalled=true;
    if (!flush || !restored) {RestoreMerge150Fix();return false;}
    DSFV27B_Merge150Fix=1;
    Log("DSFV29b: second temporary-list merge redirected to 150-node helper (50-node first caller untouched).\n");
    return true;
}

bool InstallRequestFramePatches() {
    for(auto& p:g_requestFramePatches){
        UPtr at=Address(p.at);
        if(!Readable(at,p.length))return false;
        for(unsigned j=0;j<p.length;++j)
            if(*reinterpret_cast<volatile const U8*>(at+j)!=p.stock[j])return false;
    }
    for(auto& p:g_requestFramePatches){
        UPtr at=Address(p.at);
        DWORD old=0,ignored=0;
        if(!VirtualProtect(reinterpret_cast<void*>(at),p.length,kPageExecuteReadWrite,&old)){
            RestoreRequestFramePatches();return false;
        }
        Write32(at+p.immOffset,p.replacement);
        bool exact=true;
        for(unsigned j=0;j<p.length;++j){
            const U8 wanted=(j>=p.immOffset && j<p.immOffset+4u) ?
                static_cast<U8>(p.replacement>>(8u*(j-p.immOffset))) : p.stock[j];
            if(*reinterpret_cast<volatile const U8*>(at+j)!=wanted)exact=false;
        }
        p.done=true;
        const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),p.length)!=0;
        const bool restored=VirtualProtect(reinterpret_cast<void*>(at),p.length,old,&ignored)!=0;
        if(!exact||!flushed||!restored){RestoreRequestFramePatches();return false;}
        ++DSFV27B_RequestFramePatchCount;
    }
    Log("DSFV29b: request frame 50 -> 150 nodes, 13 exact instruction patches applied.\n");
    return true;
}

bool InstallExtendedSites(){
    for(auto& p:g_sites){
        UPtr at=Address(p.original);
        if(!Readable(at,p.size))return false;
        for(unsigned i=0;i<p.size;++i){
            if(*reinterpret_cast<const U8*>(at+i)!=p.expected[i])return false;
        }
    }
    g_codeStubs=reinterpret_cast<U8*>(VirtualAlloc(nullptr,0x1000u,kMemCommitReserve,kPageExecuteReadWrite));
    if(!g_codeStubs)return false;
    for(auto& p:g_sites){
        const UPtr at=Address(p.original);
        for(unsigned i=0;i<p.size;++i)p.saved[i]=p.expected[i];
        if(p.resume) *p.resume=at+p.size;
        if(p.trampoline){
            U8* stub=g_codeStubs+g_stubUsed;
            g_stubUsed+=32u;
            for(unsigned i=0;i<p.size;++i)stub[i]=p.expected[i];
            stub[p.size]=0xE9;
            Write32(reinterpret_cast<UPtr>(stub+p.size+1u),
                static_cast<U32>(at+p.size-(reinterpret_cast<UPtr>(stub)+p.size+5u)));
            *p.trampoline=stub;
        }
        DWORD old=0;
        if(!VirtualProtect(reinterpret_cast<void*>(at),p.size,kPageExecuteReadWrite,&old)) {
            RestoreExtendedSites();return false;
        }
        U8* dest=reinterpret_cast<U8*>(at);
        dest[0]=0xE9;
        Write32(at+1u,static_cast<U32>(reinterpret_cast<UPtr>(p.bridge)-(at+5u)));
        for(unsigned i=5u;i<p.size;++i)dest[i]=0x90;
        DWORD unused=0;
        const bool ok=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),p.size) &&
            VirtualProtect(reinterpret_cast<void*>(at),p.size,old,&unused);
        p.installed=true;
        if(!ok){RestoreExtendedSites();return false;}
    }
    DSFV27B_NativeHooks=sizeof(g_sites)/sizeof(g_sites[0]);
    Log("DSFV29b: native handler/map/selector/class16 hooks installed (experimental).\n");
    return true;
}

UPtr __fastcall HandlerInitBridge(void* self, void*) {
    ++DSFV27B_HandlerCalls;
    if (!g_originalHandlerInit) return 0u; // unreachable after successful installation
    const UPtr rootAddr = Address(kRootPtr);
    if (!Readable(rootAddr, 4)) return g_originalHandlerInit(self);
    const UPtr root = Read32(rootAddr);
    if (!Readable(root + 0x204u, 4)) return g_originalHandlerInit(self);
    const UPtr manager = Read32(root + 0x204u);
    const UPtr package = manager + 0xAF68u;
    if (!manager || reinterpret_cast<UPtr>(self) != package)
        return g_originalHandlerInit(self);
    // Package zero ONLY: refuse if the metadata or neighboring counters are
    // not readable or the package has already initialized its handler arrays.
    if (!Readable(package + 0x72F8u, 4) || !Readable(package + 0x5A40u, 4)) {
        SetAdmissionError(-6, "DSFV29b: pkg0 metadata/count fields unreadable; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    const UPtr config = Read32(package + 0x72F8u);
    if (!Readable(config + 0x18u, 3)) {
        SetAdmissionError(-6, "DSFV29b: configuration pointer unreadable; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    const U8* values = reinterpret_cast<const U8*>(config + 0x18u);
    DSFV27B_AdmissionBefore = values[0];
    if (Read32(package + 0x5730u) != 0u) {
        SetAdmissionError(-5, "DSFV29b: handler package already initialized; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    if (values[0] != kOldAdmission || values[1] != 2u || values[2] != 2u) {
        SetAdmissionError(-1, "DSFV29b: metadata is not [35,2,2]; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    if(DSFV27B_PoolStatus!=1 && DSFV27B_PoolStatus!=2) {
        SetAdmissionError(-7,"DSFV29b: pool512 hooks absent; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    if (!GraphicsPoolReady()) {
        SetAdmissionError(-2, "DSFV29b: handler initialized before graphics pool 150 ready; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    if (!WriteMetadataByte(config + 0x18u, static_cast<U8>(kAdmissionStep))) {
        SetAdmissionError(-3, "DSFV29b: metadata protection/write failed; admission unchanged.\n");
        return g_originalHandlerInit(self);
    }
    g_pkg0=reinterpret_cast<UPtr>(self);
    for(U32 i=0;i<kHandlerArraySidecar;++i){
        g_globalHandlers[i]=0; g_kind0Handlers[i]=0;
    }
    DSFV27B_MapExternal=0;
    DSFV27B_MapMaxActive=0;
    DSFV27B_AdmissionResult = 1;
    Log(DSF_GRAPHICS_ADMISSION_TARGET == 160
        ? "DSFV29b: package0 metadata 35 -> 160 before handler initialization; first-pass native redirects active.\n"
        : "DSFV29b: package0 metadata 35 -> 150 before handler initialization; first-pass native redirects active.\n");
    UPtr result = g_originalHandlerInit(self);
    DSFV27B_AdmissionAfter = *reinterpret_cast<const U8*>(config + 0x18u);
    DSFV27B_InitHandlerCount = static_cast<long>(Read32(package + 0x5730u));
    DSFV27B_InitKind0Count = static_cast<long>(Read32(package + 0x5834u));
    if (DSFV27B_AdmissionAfter == static_cast<long>(kAdmissionStep) &&
        DSFV27B_InitHandlerCount == static_cast<long>(kAdmissionStep + 4u) &&
        DSFV27B_InitKind0Count == static_cast<long>(kAdmissionStep) && Read32(package + 0x5938u) == 2u &&
        Read32(package + 0x5A3Cu) == 2u) {
        DSFV27B_AdmissionResult = 2;
        Log(DSF_GRAPHICS_ADMISSION_TARGET == 160
            ? "DSFV29b: pkg0 admission 160 verified (handlers=164, kind0=160, kind1=2, kind2=2).\n"
            : "DSFV29b: pkg0 admission 150 verified (handlers=154, kind0=150, kind1=2, kind2=2).\n");
    } else {
        DSFV27B_AdmissionResult = -4;
        Log("DSFV29b: WARNING pkg0 post-initialization counts unexpected; stop testing.\n");
    }
    return result;
}

bool InstallHandlerHook() {
    const UPtr target = Address(kHandlerInit);
    const U8 original[5] = {0xA1, 0x28, 0x72, 0xFE, 0x00};
    if (!Readable(target, 5)) return false;
    for (unsigned i=0; i<5; ++i)
        if (*reinterpret_cast<const U8*>(target+i) != original[i]) return false;
    void* mem = VirtualAlloc(nullptr, 0x1000u, kMemCommitReserve, kPageExecuteReadWrite);
    if (!mem) return false;
    U8* trampoline = reinterpret_cast<U8*>(mem);
    for (unsigned i=0; i<5; ++i) trampoline[i]=original[i];
    trampoline[5]=0xE9;
    const UPtr resume = target+5u;
    *reinterpret_cast<U32*>(trampoline+6) = resume - (reinterpret_cast<UPtr>(trampoline)+10u);
    if (!FlushInstructionCache(GetCurrentProcess(), mem, 10u)) {
        VirtualFree(mem, 0, kMemRelease);
        return false;
    }
    DWORD oldProt=0;
    if (!VirtualProtect(reinterpret_cast<void*>(target), 5u, kPageExecuteReadWrite, &oldProt)) {
        VirtualFree(mem, 0, kMemRelease);
        return false;
    }
    // Uncontended startup only: not safe to install a 5-byte detour after
    // other threads may already be running the native initialization path.
    U8* at = reinterpret_cast<U8*>(target);
    at[0]=0xE9;
    *reinterpret_cast<U32*>(at+1)=reinterpret_cast<UPtr>(&HandlerInitBridge)-(target+5u);
    const BOOL flushed = FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(target), 5u);
    DWORD unused=0;
    const BOOL restored = VirtualProtect(reinterpret_cast<void*>(target), 5u, oldProt, &unused);
    if (!flushed || !restored) {
        // Best effort undo if code cache/protection could not be restored.
        DWORD scratch=0;
        if (VirtualProtect(reinterpret_cast<void*>(target),5u,kPageExecuteReadWrite,&scratch)) {
            for (unsigned i=0; i<5; ++i) at[i]=original[i];
            FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(target),5u);
            DWORD unused2=0; VirtualProtect(reinterpret_cast<void*>(target),5u,oldProt,&unused2);
        }
        VirtualFree(mem,0,kMemRelease);
        return false;
    }
    g_handlerTrampoline=mem;
    g_originalHandlerInit=reinterpret_cast<HandlerInitFn>(mem);
    DSFV27B_HookInstalled=1;
    return true;
}

void RollbackGraphicsConfig() {
    const UPtr start = Address(kGraphicsCount);
    const U32 length = static_cast<U32>(Address(kRecordCount)+4u-start);
    DWORD oldProt=0;
    if (!VirtualProtect(reinterpret_cast<void*>(start),length,kPageReadWrite,&oldProt)) return;
    Write32(Address(kGraphicsCount),kOldSlots);
    Write32(Address(kBudgetA),0x2000u);
    Write32(Address(kBudgetB),0x10000u);
    Write32(Address(kRecordCount),kOldSlots);
    Write32(Address(kCityRegionBudgetA),kOldCityBudgetA);
    Write32(Address(kCityRegionBudgetB),kOldCityBudgetB);
    DWORD unused=0; VirtualProtect(reinterpret_cast<void*>(start),length,oldProt,&unused);
}

void AppendText(char*& cursor, const char* text, const char* end) {
    while (*text && cursor < end) *cursor++ = *text++;
}
void AppendNumber(char*& cursor, long value, const char* end) {
    if (cursor >= end) return;
    if (value < 0) {
        *cursor++ = '-';
        if (value == -2147483647 - 1) { AppendText(cursor, "2147483648", end); return; }
        value = -value;
    }
    char reversed[12] = {};
    unsigned n = 0;
    do { reversed[n++] = static_cast<char>('0' + value % 10); value /= 10; }
    while (value && n < 11);
    while (n && cursor < end) *cursor++ = reversed[--n];
}
void AppendHex32(char*& cursor, U32 value, const char* end) {
    const char digits[] = "0123456789ABCDEF";
    for (int shift=28; shift>=0 && cursor<end; shift-=4)
        *cursor++ = digits[(value>>shift)&0xFu];
}
void LogTelemetry() {
    char message[736] = {};
    char* cursor = message;
    const char* end = message + sizeof(message) - 1;
    AppendText(cursor, "DSFV29b: gfx=", end);
    AppendNumber(cursor, DSF150_GraphicsSlots, end);
    AppendText(cursor, " citySlots=", end);
    AppendNumber(cursor, DSFV28_CityRegionSlots, end);
    AppendText(cursor, " globalCitySlots=", end);
    AppendNumber(cursor, DSFV28_CityGlobalSlots, end);
    AppendText(cursor, " pool=", end);
    AppendNumber(cursor, DSFV27B_PoolLiveRecordCount, end);
    AppendText(cursor, " free=", end);
    AppendNumber(cursor, DSFV27B_PoolFreeCount, end);
    AppendText(cursor, " minFreeObserved=", end);
    AppendNumber(cursor, DSFV27B_PoolMinFreeObserved, end);
    AppendText(cursor, " map=", end);
    AppendNumber(cursor, DSF150_CompletedActive, end);
    AppendText(cursor, " allocated=", end);
    AppendNumber(cursor, DSF150_CompletedAllocated, end);
    AppendText(cursor, " handlers=", end);
    AppendNumber(cursor, DSF150_GlobalHandlerCount, end);
    AppendText(cursor, " kind0=", end);
    AppendNumber(cursor, DSF150_Kind0HandlerCount, end);
    AppendText(cursor, " highMap=", end);
    AppendNumber(cursor, DSF150_MapHighWater, end);
    AppendText(cursor, " highHandlers=", end);
    AppendNumber(cursor, DSF150_HandlerHighWater, end);
    AppendText(cursor, " admission=", end);
    AppendNumber(cursor, DSFV27B_LiveAdmission, end);
    AppendText(cursor, " init=", end);
    AppendNumber(cursor, DSFV27B_AdmissionResult, end);
    AppendText(cursor, " extMap=", end);
    AppendNumber(cursor, DSFV27B_MapExternal, end);
    AppendText(cursor, " map16ext=", end);
    AppendNumber(cursor, DSFV29i_Map16ExtraInserts, end);
    AppendText(cursor, " map16hi=", end);
    AppendNumber(cursor, DSFV29i_Map16ActiveHighWater, end);
    AppendText(cursor, " map16guard=", end);
    AppendNumber(cursor, DSFV29i_Map16Guard, end);
    AppendText(cursor, " selOverflow=", end);
    AppendNumber(cursor, DSFV27B_SelectorOverflow, end);
    AppendText(cursor, " node3Overflow=", end);
    AppendNumber(cursor, DSFV27B_ThirdOverflow, end);
    AppendText(cursor, " arrayOverflow=", end);
    AppendNumber(cursor, DSFV27B_ArrayOverflowInsert, end);
    AppendText(cursor, " merge150=", end);
    AppendNumber(cursor, DSFV27B_Merge150Fix, end);
    AppendText(cursor, " c7target=", end);
    AppendNumber(cursor, DSF_Chapter7PreloadConfiguredCount, end);
    AppendText(cursor, " c7pre=", end);
    AppendNumber(cursor, DSF_Chapter7PreloadHits, end);
    AppendText(cursor, " c7seen18=", end);
    AppendNumber(cursor, DSF_Chapter7PreloadSeen18, end);
    AppendText(cursor, " c7match=", end);
    AppendNumber(cursor, DSF_Chapter7PreloadExact18Matches, end);
    AppendText(cursor, " c7gate=", end);
    AppendNumber(cursor, DSF_Chapter7PreloadGate, end);
    AppendText(cursor, " c7hash=", end);
    AppendHex32(cursor, static_cast<U32>(DSF_Chapter7PreloadLastHash), end);
    AppendText(cursor, " req=", end);
    AppendNumber(cursor, DSFV27B_RequestLastCount, end);
    AppendText(cursor, " reqHigh=", end);
    AppendNumber(cursor, DSFV27B_RequestHighWater, end);
    AppendText(cursor, " reqExtra=", end);
    AppendNumber(cursor, DSFV27B_RequestExtraReads, end);
    AppendText(cursor, " reqFault=", end);
    AppendNumber(cursor, DSFV27B_RequestFault, end);
    AppendText(cursor, " boundary=", end);
    AppendNumber(cursor, DSFV27B_UnsafeBoundary, end);
    AppendText(cursor, " hashHook=", end);
    AppendNumber(cursor, DSFV29_HashHook, end);
    AppendText(cursor, " hashNative=", end);
    AppendNumber(cursor, DSFV29_HashNativeGranted, end);
    AppendText(cursor, " hashRemaining=", end);
    AppendNumber(cursor, DSFV29_HashNativeRemaining, end);
    AppendText(cursor, " hashExtra=", end);
    AppendNumber(cursor, DSFV29_HashExtraGranted, end);
    AppendText(cursor, " hashHead=", end);
    AppendNumber(cursor, DSFV29_HashHeadNonzero, end);
    AppendText(cursor, " hashFault=", end);
    AppendNumber(cursor, DSFV29_HashFaults, end);
    if (cursor < end) *cursor++ = '\n';
    *cursor = 0;
    Log(message);
}

#include "V29cDiagnostics.inc"
static void EfDrainEvents() {
    static long drained=0;
    static U32 nextStatus=0;
    const long total=g_efTotal;
    const long oldest=total>static_cast<long>(kEfEventSlots)?total-static_cast<long>(kEfEventSlots):0;
    if(drained<oldest) {
        DSFV29m_EfEventDrops+=oldest-drained;
        drained=oldest;
    }
    U32 taken=0;
    while(drained<total && taken<96u) {
        const long seq=drained+1;
        const EfEvent& e=g_efEvents[static_cast<U32>(seq-1)%kEfEventSlots];
        if(e.sequence!=seq)break;
        const U32 tick=e.tick,tid=e.tid,op=e.op,owner=e.owner,gen=e.generation;
        const U32 caller=e.caller,map=e.map,cursor=e.cursor,freeCount=e.freeCount;
        const U32 active=e.active,node=e.node,key=e.key,handler=e.handler,flags=e.flags;
        if(e.sequence!=seq)break;
        char line[420]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29m_EF tick=",end);DiagAppendU32(p,tick,end);
        AppendText(p," tid=",end);DiagAppendU32(p,tid,end);
        AppendText(p," op=",end);
        static const char* names[]={"?","CREATE","INSERT_PRE","INSERT_POST","REMOVE_PRE",
            "REMOVE_POST","DELETE_PRE","DELETE_POST","FREE_PRE","FREE_POST",
            "OWNER_REMOVE_PRE","OWNER_REMOVE_POST"};
        AppendText(p,op<sizeof(names)/sizeof(names[0])?names[op]:"?",end);
        AppendText(p," owner=",end);AppendHex32(p,owner,end);
        AppendText(p," gen=",end);DiagAppendU32(p,gen,end);
        AppendText(p," caller=",end);AppendHex32(p,caller,end);
        AppendText(p," map=",end);AppendHex32(p,map,end);
        AppendText(p," cursor=",end);AppendHex32(p,cursor,end);
        AppendText(p," free=",end);DiagAppendU32(p,freeCount,end);
        AppendText(p," active=",end);DiagAppendU32(p,active,end);
        AppendText(p," node=",end);AppendHex32(p,node,end);
        AppendText(p," arg0=",end);AppendHex32(p,key,end);
        AppendText(p," arg1=",end);AppendHex32(p,handler,end);
        AppendText(p," flags=",end);AppendHex32(p,flags,end);
        AppendText(p,"\n",end);*p=0;
        DiagPrint(line);
        ++drained;++taken;
    }
    const U32 now=GetTickCount();
    if(now>=nextStatus) {
        nextStatus=now+5000u;
        char line[320]={};char* p=line;const char* end=line+sizeof(line)-1;
        AppendText(p,"DSFV29m_STATUS total=",end);DiagAppendU32(p,static_cast<U32>(g_efTotal),end);
        AppendText(p," drained=",end);DiagAppendU32(p,static_cast<U32>(drained),end);
        AppendText(p," eventDrops=",end);DiagAppendU32(p,static_cast<U32>(DSFV29m_EfEventDrops),end);
        AppendText(p," trackDrops=",end);DiagAppendU32(p,static_cast<U32>(DSFV29m_EfTrackDrops),end);
        AppendText(p," efExtra=",end);DiagAppendU32(p,static_cast<U32>(DSFV29n_EFExtraInserts),end);
        AppendText(p," efActiveHigh=",end);DiagAppendU32(p,static_cast<U32>(DSFV29n_EFActiveHighWater),end);
        AppendText(p," efResets=",end);DiagAppendU32(p,static_cast<U32>(DSFV29n_EFSidecarResets),end);
        AppendText(p," efReleases=",end);DiagAppendU32(p,static_cast<U32>(DSFV29n_EFSidecarReleases),end);
        AppendText(p," efGuard=",end);DiagAppendU32(p,static_cast<U32>(DSFV29n_EFGuard),end);
        AppendText(p,"\n",end);*p=0;DiagPrint(line);
    }
}
#include "V29fMemoryDiagnostics.inc"
#include "V29gCityDiagnostics.inc"
#include "V29hGraphicsDiagnostics.inc"
#if defined(DSF_V29S_NATIVE_TRACE)
#include "V29sNativeLoadingTrace.inc"
#if defined(DSF_V29S_VISUAL_TRACE)
#include "V29sVisualTrace.inc"
#endif
#if defined(DSF_V29S_SUBMISSION_TRACE)
#include "V29sSubmissionTrace.inc"
#endif
#if defined(DSF_V29S_DRAW_QUEUE_TRACE)
#include "V29sDrawQueueTrace.inc"
#endif
#if defined(DSF_V29S_HANDLE_CHAIN_TRACE)
#include "V29sHandleChainTrace.inc"
#endif
#if defined(DSF_V29S_HASH_HEAD_TRACE)
#include "V29sHashHeadTrace.inc"
#endif
#if defined(DSF_V29S_AUDIO_TRACE)
#include "V29sAudioBankTrace.inc"
#endif
#if defined(DSF_V29S_CAPACITY_TRACE)
#if defined(DSF_V29S_INDEXED_262144)
#include "V29sIndexedCapacity.inc"
#endif
#include "V29sCapacityTrace.inc"
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
#include "V29sAudioDefaultPoolPatch.inc"
#endif
#if defined(DSF_V29S_AUDIO_PROBE)
#include "V29sAudioProbe.inc"
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
#include "V29sAudioDefaultPool.inc"
#endif
#if defined(DSF_V29S_AUDIO_SLOTS_2048)
#include "V29sAudioSlots.inc"
#endif
#if defined(DSF_V29S_AUDIO_REASON_PROBE)
#include "V29sAudioReason.inc"
#endif
#if defined(DSF_V29S_AUDIO_MODEL192)
#include "V29sAudioModelRegistry.inc"
#endif
#if defined(DSF_V29S_CLEANUP_262144)
#include "V29sCleanupQueue.inc"
#endif
#if defined(DSF_V29S_PENDING512)
#include "V29sPendingMap.inc"
#endif
#if defined(DSF_V29S_CHAPTERS)
#include "V29sChapterRuntime.inc"
#endif
#if defined(DSF_V29S_CALLBACK512)
#if !defined(DSF_V29S_CHAPTERS)
#error Callback extension requires the chapter-independent parent
#endif
#include "V29sCallback.inc"
#endif
#if defined(DSF_V29S_ARENA_REUSE)
#if !defined(DSF_V29S_CALLBACK512)
#error Arena lifetime repair requires the callback/chapter parent
#endif
#include "V29sArenaReuse.inc"
#if defined(DSF_V29S_BASELINE150)
#include "V29sBaseline150.inc"
#if defined(DSF_V29S_NOTIFY150)
#include "V29sNotify150.inc"
#if defined(DSF_V29S_PARKED150)
#include "V29sParked150.inc"
#if defined(DSF_V29S_REPLAY150)
#include "V29sReplay150.inc"
#endif
#endif
#endif
#endif
#endif
#if defined(DSF_V29S_AUDIO_STATIC_64M)
#include "V29sAudioStatic64.inc"
#endif
#endif
#endif
#elif defined(DSF_V29R_CALLERS)
#include "V29rCallerDiagnostics.inc"
#elif defined(DSF_V29Q_TRAVERSAL)
#include "V29qTraversalDiagnostics.inc"
#endif

// A very low-frequency read-only monitor. This never modifies resource lists,
// admits extra IDs, or blocks their normal release paths.
#if defined(DSF_V29S_PERF_CONTROL)
DWORD __stdcall Monitor(void*) {
#if defined(DSF_V29S_CAPACITY_TRACE)
    CapacityStartLine();
#endif
    for (;;) {
        const UPtr pool9 = Address(kGraphicsManager) + 0x348u;
        if (DSF150_Status == 1 && Readable(pool9, 8u) && Read32(pool9) != 0u) {
            DSF150_GraphicsSlots = static_cast<long>(Read32(pool9 + 4u));
            DSF150_Status = DSF150_GraphicsSlots == static_cast<long>(kNewSlots) ? 2 : -7;
        }
        const UPtr rootLocation = Address(kRootPtr);
        if (Readable(rootLocation, 4u)) {
            const UPtr root = Read32(rootLocation);
            if (Readable(root + 0x194u, 4u)) {
                const UPtr pool = Read32(root + 0x194u);
                if (Readable(pool + 0x1814u, 4u) && Read32(pool) == Address(0x00DDA2C8u)) {
                    const U32 count = Read32(pool + 0x1010u);
                    const U32 free = Read32(pool + 0x1814u);
                    if (DSFV27B_PoolStatus == 1 && count == 512u && free <= 512u)
                        DSFV27B_PoolStatus = 2;
                    if ((count != 0u && count != 512u) || free > 512u) {
                        DSFV27B_PoolStatus = -1;
                        DSFV27B_UnsafeBoundary = 20;
                    }
                }
            }
        }
#if defined(DSF_V29S_CAPACITY_TRACE)
        CapacityPoll();
#if defined(DSF_V29S_BASELINE150)
        BaselinePoll();
#if defined(DSF_V29S_NOTIFY150)
        NotifyPoll();
#if defined(DSF_V29S_PARKED150)
        ParkPoll();
#if defined(DSF_V29S_REPLAY150)
        ReplayPoll();
#endif
#endif
#endif
#endif
#if defined(DSF_V29S_CHAPTERS)
        ChapterPoll();
#endif
#if defined(DSF_V29S_CALLBACK512)
        CallbackPoll();
#endif
#if defined(DSF_V29S_ARENA_REUSE)
        ArenaReusePoll();
#endif
#if defined(DSF_V29S_AUDIO_PROBE)
        AudioProbePoll();
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
        AudioDefaultPoolPoll();
#endif
#if defined(DSF_V29S_AUDIO_SLOTS_2048)
        AudioSlotsPoll();
#endif
#if defined(DSF_V29S_AUDIO_REASON_PROBE)
        AudioReasonPoll();
#endif
#if defined(DSF_V29S_AUDIO_MODEL192)
        AudioModelPoll();
#endif
#if defined(DSF_V29S_CLEANUP_262144)
        CleanupPoll();
#endif
#if defined(DSF_V29S_PENDING512)
        PendingPoll();
#endif
#if defined(DSF_V29S_AUDIO_STATIC_64M)
        AudioStaticPoolPoll();
#endif
#endif
#endif
        Sleep(200u);
    }
}
#else
DWORD __stdcall Monitor(void*) {
    U32 loops = 0;
    long previousMap = -1;
    long previousSlots = -1;
    long previousRegionSlots = -2;
    bool reached64Logged = false;
    bool badBoundaryLogged = false;
    long previousHashRemaining = -999;
    long previousHashExtra = -1;
    long previousChapter7Matches = -1;
    long previousChapter7Gate = -1;
    DiagOpenFile();
#if DSF_CHAPTER7_PRELOAD_COUNT == 45
    DiagPrint("DSFV29n_START request=45 map16=ED+EF inline=45 extraPerOwner=256 owners=96 ctor=0086B4A0 insert=007D7270 remove=00556FB0 delete=00791990 free=00C2875A ring=4096\n");
#else
    DiagPrint("DSFV29n_START request=150 map16=ED+EF inline=45 extraPerOwner=256 owners=96 ctor=0086B4A0 insert=007D7270 remove=00556FB0 delete=00791990 free=00C2875A ring=4096\n");
#endif
    DiagPrint("DSFV29g_START city7SampleMs=200 maxRows=8192 fields=raw14,raw1C,secondRaw baseSampler=1\n");
#if defined(DSF_V29S_NATIVE_TRACE)
    DiagPrint(DSFV29s_Hooks==5 ? "DSFV29s_START hooks=5 reconcile=005A7490 gate=0059DB40 dispatch=0059D740 pass=0059D778 transfer=0059BAD0 handlerWords=16 handlerState=00859000 handlerSampleMs=5000 noGameDataWrites=1\n" : "DSFV29s_START hooks=-1 signatureFailed=1 v29pRetained=1\n");
    DiagPrint(DSFV29s_WaitHooks==2 ? "DSFV29s_WAIT_START hooks=2 failA=005A65DF failB=005A665A predicate=005A63B0 noGameDataWrites=1\n" : "DSFV29s_WAIT_START hooks=-1 signatureFailed=1\n");
#if defined(DSF_V29S_POOL_DIAG)
    DiagPrint(g_sPoolHooks==2 ? "DSFV29s_POOL_START hooks=2 site0=005A7DD1 site1=0083B170 site2=reconcile site3=waitA site4=waitB ring=8192 registers=all pools=CC,338,5A4,9A0 snapshotMs=1000 noPoolWrites=1\n" : "DSFV29s_POOL_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_SPOOL0_EXPANDED)
    DiagPrint(g_sScratchHook==2 ? "DSFV29s_SPOOL0_START pool=owner+CC inline=50 capacity=256 reconcileRedirect=1 scratchHooks=005A75DD,005A7665 scratchNodes=256\n" : "DSFV29s_SPOOL0_START scratchHooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_CLONE_EXPANDED)
    DiagPrint(g_sCloneHook==1 ? "DSFV29s_CLONE_START hook=005A6960 caller=008093D1 inline=50 capacity=256 arenas=32 reuseByStackHeader=1\n" : "DSFV29s_CLONE_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_BUCKET_DIAG)
    DiagPrint(g_sBucketHook==1 ? "DSFV29s_BUCKET_START hook=00809443 buckets=25 inlinePerBucket=50 readOnly=1\n" : "DSFV29s_BUCKET_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_MODEL_DIAG)
    DiagPrint(g_sModelHook==1 ? "DSFV29s_MODEL_START hook=00890949 vehicleDefinitionId=EDI descriptor=EAX readOnly=1\n" : "DSFV29s_MODEL_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_LIVE_MODEL_DIAG)
    DiagPrint(g_sLiveModelHooks==2 ? "DSFV29s_LIVE_MODEL_START hooks=2 sites=008DDFC4,008DA1EA vehicleDefinitionId=object+4850->14 readOnly=1\n" : "DSFV29s_LIVE_MODEL_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
    DiagPrint(g_sRegHooks==3 ? "DSFV29s_REG_START hooks=3 entry=0086BCF0,0086BD40 create=0086BAB0 batches=RequestWriteBridge readOnly=1\n" : "DSFV29s_REG_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
    DiagPrint(g_sModelPoolPatched && g_sRegMapHook==1 ? "DSFV29s_MODEL_POOL_START source=0088A4F3 stock=45 target=160 allocationType=F8 mapHook=0086BE5F inlineNodes=45 externalNodes=512\n" : "DSFV29s_MODEL_POOL_START patch=-1 preinitFailed=1\n");
#endif
#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
    DiagPrint("DSFV29s_RESOURCE_COLLECTION_START count=0188A6B8 pointerArray=0185A6B8 pairArray=0186A6B8 stock=16384 target=131072 patchSites=31\n");
#endif
#if defined(DSF_V29S_ZAP_TARGET_DIAG)
    DiagPrint("DSFV29s_ZAP_START source=GetTargetedGameVehicle player=0 selector=manager+338 fields=+1C,+18 nativeVehicleVptr=00E3A74C captureBytes=1536 modelLink=vehicle+5B4 modelVptr=00E39EAC pollMs=200 retainMs=10000 readOnly=1\n");
#endif
#if defined(DSF_V29S_EXTERIOR_TRACE)
    DiagPrint("DSFV29s_EXT_START slotSite=00858860 drawEntry=008A8DA0 drawMask=008A8E53 arg0=raw slot=esp+8 sceneModelSlot=+564 events=2048 recentModels=4 readOnly=1\n");
#endif
#if defined(DSF_V29S_LOD_TRACE)
    DiagPrint("DSFV29s_LOD_START requestSite=005A7FC6 base=owner+8 interior=owner+10 exterior=owner+12 extLoadedFlag=owner+1D drawId=base+2000 readOnly=1\n");
    DiagPrint("DSFV29s_DRAW_SAMPLE_START entry=008A8DA0 mask=008A8E53 ring=2048 maxObjects=64 modelId=E39EAC:+4850->14 sceneSlot=E37118:+564 readOnly=1\n");
    DiagPrint(g_sBodyHook==1 ? "DSFV29s_BODY_START hook=1 entry=008D56A0 callsDraw=008D590C model=vehicle+5B4 ring=2048 readOnly=1\n" : "DSFV29s_BODY_START hook=-1 signatureFailed=1\n");
    DiagPrint(g_sCommonHooks==2 ? "DSFV29s_COMMON_START hooks=2 merge=008A8ECA walk=008A7D10 model=drawObject ring=2048 readOnly=1\n" : "DSFV29s_COMMON_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_VISUAL_TRACE)
    DiagPrint(g_vVisualHooks==2 ? "DSFV29s_VIS_START hooks=2 decision=008A8D05 candidate=008A8D12 ring=16384 model=currentShiftTarget caller716C=00716C0E caller8D59=008D5911 readOnly=1\n" : "DSFV29s_VIS_START hooks=-1 signatureFailed=1\n");
    DiagPrint(g_vContextHook==1 ? "DSFV29s_CONTEXT_START hook=00716C3A rawContext=EBX rawField38=EBX+38 filtered=0 returnPath=00716C0E readOnly=1\n" : "DSFV29s_CONTEXT_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_SUBMISSION_TRACE)
    DiagPrint(g_vSubmitHook==1 ? "DSFV29s_SUBMIT_START hooks=008A8B99,008A8D6F after=00697F10 selectedModel=dynamic resourceFields=48,4C budget=1 ring=8192 readOnly=1\n" : "DSFV29s_SUBMIT_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_DRAW_QUEUE_TRACE)
    DiagPrint(g_vDrawQueueHook==3 ? "DSFV29s_DRAW_QUEUE_START hooks=3 flip=00BB5780 jobCheck=006C21F9 trapCheck=00BB5A8C capacity=16384 frameRing=8192 readOnly=1\n" : "DSFV29s_DRAW_QUEUE_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_HANDLE_CHAIN_TRACE)
    DiagPrint(g_vHandleHooks==8 ? "DSFV29s_HANDLE_START hooks=8 firstLink=00BAE8B3 secondLink=00BAE8C4 unlink=00BAEA80 alloc=00BAE888 reject=00BAE8A4 free=00BAEBCC freeAlt=00BAED21 unlinkNext=00BAEB8B indexedSlots=131072 nextProvenance=1 readOnly=1\n" : "DSFV29s_HANDLE_START hooks=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_HASH_HEAD_TRACE)
#if DSF_CHAPTER7_PRELOAD_COUNT == 45
    DiagPrint(g_vHashHeadHooks==4 ? "DSFV29s_HASH_HEAD_START hooks=4 insert=00BAE8BE unlink=00BAEBB7 alternate=00BAED29 lookup=00BAEB4F ledger=131072 request=45 modelSlots=160 logFilter=1 readOnly=1\n" : "DSFV29s_HASH_HEAD_START hooks=-1 signatureFailed=1\n");
#else
    DiagPrint(g_vHashHeadHooks==4 ? "DSFV29s_HASH_HEAD_START hooks=4 insert=00BAE8BE unlink=00BAEBB7 alternate=00BAED29 lookup=00BAEB4F ledger=131072 request=150 modelSlots=160 logFilter=1 readOnly=1\n" : "DSFV29s_HASH_HEAD_START hooks=-1 signatureFailed=1\n");
#endif
#endif
#if defined(DSF_V29S_AUDIO_TRACE)
    DiagPrint(g_audioHooks==5 ? "DSFV29s_AUDIO_START hooks=5 submit=0084BED2 callback=0084BE74 open=008FC54E lookup=0048F780 matched=0048F7B0 statusMs=5000 readOnly=1\n" : "DSFV29s_AUDIO_START hooks=-1 signatureFailed=1\n");
#endif
    DiagPrint("DSFV29s_WAIT_IDS_START scope=observedFailures recentMs=5000 zeroMeansUnobserved noGameDataWrites=1\n");
    DiagPrint(g_sMemberDetailSignature==1 ? "DSFV29s_MEMBER_DETAIL_START ids=126,231 kind=0 include=005A10C0 exclude=005A0F60 global=0059E4E0 sampledOnFailure=1 noGameDataWrites=1\n" : "DSFV29s_MEMBER_DETAIL_START signatureFailed=1\n");
#if defined(DSF_V29S_INCLUDE_EXPANDED)
    DiagPrint(g_sMutationHooks==3 ? "DSFV29s_MUTATION_START hooks=3 ids=126,231 insertA=005A2740 insertB=005A2790 remove=005A26A0 op=0,1,2 buffer=4096 includeCursorRedirect=1\n" : "DSFV29s_MUTATION_START hooks=-1 signatureFailed=1\n");
    DiagPrint(g_sGuardHook==1 ? "DSFV29s_GUARD_START hook=005A200E ids=126,231 count=section+6694 expandedCapacity=256 includeCursorRedirect=1\n" : "DSFV29s_GUARD_START hooks=-1 signatureFailed=1\n");
#if defined(DSF_V29S_LISTA_EXPANDED)
    DiagPrint(g_sListAHook==1 ? "DSFV29s_LISTA_START hook=005A7DD1 inline=50 capacity=512 stride=12 owner=spooler+338 redirect=1\n" : "DSFV29s_LISTA_START hook=-1 signatureFailed=1\n");
#endif
#if defined(DSF_V29S_LISTB_EXPANDED)
    DiagPrint(g_sListBHook==1 ? "DSFV29s_LISTB_START hook=0083B170 inline=50 capacity=512 stride=20 owner=spooler+5A4 redirect=1\n" : "DSFV29s_LISTB_START hook=-1 signatureFailed=1\n");
#endif
#else
    DiagPrint(g_sMutationHooks==3 ? "DSFV29s_MUTATION_START hooks=3 ids=126,231 insertA=005A2740 insertB=005A2790 remove=005A26A0 op=0,1,2 buffer=4096 noGameDataWrites=1\n" : "DSFV29s_MUTATION_START hooks=-1 signatureFailed=1\n");
    DiagPrint(g_sGuardHook==1 ? "DSFV29s_GUARD_START hook=005A200E ids=126,231 count=section+6694 skipWhen=64 noGameDataWrites=1\n" : "DSFV29s_GUARD_START hooks=-1 signatureFailed=1\n");
#endif
#if DSF_CHAPTER7_PRELOAD_COUNT == 45
    DiagPrint("DSFV29s_REQ_START inline=50 target=45 indices=0,44,49,50,51,52,100,101,102,149 noGameDataWrites=1\n");
#else
    DiagPrint("DSFV29s_REQ_START inline=50 target=150 indices=0,44,49,50,51,52,100,101,102,149 noGameDataWrites=1\n");
#endif
#elif defined(DSF_V29R_CALLERS)
    DiagPrint(DSFV29r_Hooks==2 ? "DSFV29r_START hooks=2 entry=00BAE790 return=00BAE921 callerAttribution=1 stepHook=0 diagNoGameDataWrites=1\n" : "DSFV29r_START hooks=-1 signatureFailed=1 v29pRetained=1\n");
#elif defined(DSF_V29Q_TRAVERSAL)
    DiagPrint(DSFV29q_Hooks==3 ? "DSFV29q_START hooks=3 entry=00BAE790 step=00BAE81D return=00BAE921 steps=approx noGameWrites=1\n" : "DSFV29q_START hooks=-1 signaturesUnavailable=1 v29pRetained=1\n");
#endif
    DiagPrint(DSFV29h_Hooks==2 ? "DSFV29h_START hooks=2 ring=256 noSuppression=1\n" : "DSFV29h_START hooks=-1 graphics hooks unavailable; V29g sampling active\n");
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
    DiagPrint("DSFV29s_VPOOL_START pools=8,9,10,11 rawOffsets=14,1C,40,48 changeLimit=30000 eventRing=4096 eventKind=1:free,2:reject cadenceMs=200 noGameDataWrites=1\n");
#endif
    for (;;) {
        // Two-pass snapshots run on the existing monitoring thread; no disk I/O
        // or heap allocations run in the game's callback/handler hot paths.
        DiagSnapshot(loops);
        DiagV29dPoll();
        DiagDrainCallbacks();
        EfDrainEvents();
        DiagFMemorySample(loops);
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
        DiagFVehiclePoolSample();
#endif
        DiagGCitySample();
        DiagHDrain();
        DiagHPrintStatus(loops);
#if defined(DSF_V29S_VEHICLE_POOL_TRACE)
        DiagHVehicleDrain();
        DiagHVehicleStatus(loops);
#endif
#if defined(DSF_V29S_NATIVE_TRACE)
        DiagSPrintStatus(loops);
        DiagSPrintWait(loops);
        DiagSPrintWaitIds(loops);
        DiagSPrintMemberDetail(loops);
        DiagSPrintMutationEvents(loops);
        DiagSPrintGuard(loops);
#if defined(DSF_V29S_SPOOL0_EXPANDED)
        DiagSPrintSpool0(loops);
#endif
#if defined(DSF_V29S_CLONE_EXPANDED)
        DiagSPrintClone(loops);
#endif
#if defined(DSF_V29S_BUCKET_DIAG)
        DiagSPrintBucket(loops);
#endif
#if defined(DSF_V29S_MODEL_DIAG)
        DiagSPrintModel(loops);
#endif
#if defined(DSF_V29S_LIVE_MODEL_DIAG)
        DiagSPrintLiveModel(loops);
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
        DiagSPrintRegSource(loops);
#endif
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
        DiagSPrintModelPool(loops);
#endif
#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
        DiagSPrintResourceCollection(loops);
#endif
#if defined(DSF_V29S_ZAP_TARGET_DIAG)
        DiagSPrintZapTarget();
#endif
#if defined(DSF_V29S_EXTERIOR_TRACE)
        DiagSPrintExterior(loops);
#endif
#if defined(DSF_V29S_LOD_TRACE)
        DiagSPrintLod(loops);
#endif
#if defined(DSF_V29S_VISUAL_TRACE)
        DiagVVisualPrint(loops);
        DiagVContextPrint(loops);
#endif
#if defined(DSF_V29S_SUBMISSION_TRACE)
        DiagVSubmissionPrint(loops);
        DiagVGraphicsBudgetPrint(loops);
#endif
#if defined(DSF_V29S_DRAW_QUEUE_TRACE)
        DiagVDrawQueuePrint(loops);
#endif
#if defined(DSF_V29S_HANDLE_CHAIN_TRACE)
        DiagVHandlePrint(loops);
#endif
#if defined(DSF_V29S_HASH_HEAD_TRACE)
        DiagVHashHeadPrintStatus(loops);
#endif
#if defined(DSF_V29S_AUDIO_TRACE)
        DiagAudioPrintStatus(loops);
#endif
#if defined(DSF_V29S_POOL_DIAG)
        DiagSPrintPoolEvents();
        DiagSPrintPoolState(loops);
#endif
        DiagSPrintRequestProbe(loops);
        DiagSPrintHandlerWords(loops);
#elif defined(DSF_V29R_CALLERS)
        DiagRPrintStatus(loops);
#elif defined(DSF_V29Q_TRAVERSAL)
        DiagQPrintStatus(loops);
#endif
        if (g_hashFirstOwner && Readable(g_hashFirstOwner+kHashCursorOffset,4u)) {
            const UPtr owner=g_hashFirstOwner;
            const UPtr cursor=Read32(owner+kHashCursorOffset);
            const UPtr start=owner+kHashStartOffset;
            const UPtr limit=owner+kHashMetadataOffset;
            if (cursor>=start && cursor<=limit && ((cursor-start)%kHashNodeStride)==0u) {
                DSFV29_HashNativeGranted=static_cast<long>((cursor-start)/kHashNodeStride);
                DSFV29_HashNativeRemaining=static_cast<long>((limit-cursor)/kHashNodeStride);
            }
            else if (DSFV29_HashFaults == 0) {
                DSFV29_HashFaults=1;
                Log("DSFV29b: FATAL hash cursor invalid during monitoring; stop test.\n");
            }
            DSFV29_HashHeadNonzero=(Read32(owner+kHashMetadataOffset)!=0u)?1:0;
        }
        // 21 named pools are at manager+0x0C+index*0x5C.
        // We only inspect [pool+4] slot count after the descriptor is readable.
        const UPtr regionPool = Address(kGraphicsManager) + 0x290u; // index 7
        const UPtr globalCityPool = Address(kGraphicsManager) + 0x234u; // index 6
        if (Readable(regionPool,8u)) DSFV28_CityRegionSlots=static_cast<long>(Read32(regionPool+4u));
        if (Readable(globalCityPool,8u)) DSFV28_CityGlobalSlots=static_cast<long>(Read32(globalCityPool+4u));
        const UPtr pool9 = Address(kGraphicsManager) + 0x348u;
        if (Readable(pool9, 8) && Read32(pool9) != 0) {
            DSF150_GraphicsSlots = static_cast<long>(Read32(pool9 + 4));
            if (DSF150_Status == 1) {
                if (DSF150_GraphicsSlots == static_cast<long>(kNewSlots)) {
                    DSF150_Status = 2;
                    Log(DSF_GRAPHICS_ADMISSION_TARGET == 160
                        ? "DSFV29b: graphics pool9 allocated 160 slots (confirmed).\n"
                        : "DSFV29b: graphics pool9 allocated 150 slots (confirmed).\n");
                } else {
                    DSF150_Status = -7;
                    Log("DSFV29b: graphics pool9 count mismatch.\n");
                }
            }
        }
        const UPtr rootLocation = Address(kRootPtr);
        if (Readable(rootLocation, 4)) {
            UPtr root = Read32(rootLocation);
            if (Readable(root+0x194u,4u)) {
                const UPtr pool=Read32(root+0x194u);
                if (Readable(pool+0x1818u-4u,4u) && Read32(pool)==Address(0x00DDA2C8u)) {
                    const U32 count=Read32(pool+0x1010u);
                    const U32 free=Read32(pool+0x1814u);
                    DSFV27B_PoolLiveRecordCount=static_cast<long>(count);
                    DSFV27B_PoolFreeCount=static_cast<long>(free);
                    if (count == 512u && free <= 512u &&
                        (DSFV27B_PoolMinFreeObserved < 0 ||
                         static_cast<long>(free) < DSFV27B_PoolMinFreeObserved))
                        DSFV27B_PoolMinFreeObserved = static_cast<long>(free);
                    if(DSFV27B_PoolStatus==1 && count==512u && free<=512u) {
                        DSFV27B_PoolStatus=2;
                        Log("DSFV29b: object pool runtime verified: count=512 and free counter <=512.\n");
                    }
                    // A newly constructed pool has count=0 until its initializer
                    // runs, so don't misclassify that brief startup state.
                    if ((count!=0u && count!=512u) || free>512u) {
                        DSFV27B_PoolStatus=-1;
                        DSFV27B_UnsafeBoundary=20;
                        Log("DSFV29b: FATAL object pool metadata/counter mismatch; STOP.\n");
                    }
                }
            }
            if (Readable(root + 0x204u, 4)) {
                UPtr mgr = Read32(root + 0x204u);
                if (Readable(mgr + 0xAF68u + 0x5B50u, 4)) {
                    const UPtr package = mgr + 0xAF68u;
                    // This is package 0 only. Counts are not proof of READY/drawable models.
                    const U32 active = Read32(package + 0x5B4Cu);
                    const U32 allocatedCursor = Read32(package + 0x5B44u);
                    const UPtr first = package + 0x5B50u;
                    if (active <= 100000u) {
                        DSF150_CompletedActive = static_cast<long>(active);
                        if (static_cast<long>(active) > DSF150_MapHighWater)
                            DSF150_MapHighWater = static_cast<long>(active);
                    }
                    if (allocatedCursor >= first && allocatedCursor <= first + 0x400u &&
                        ((allocatedCursor - first) % 16) == 0)
                        DSF150_CompletedAllocated = static_cast<long>((allocatedCursor - first) / 16);
                    else {
                        // V26 telemetry froze at 64 when external arena was entered.
                        // This measures high-water cursor allocations, NOT live residency.
                        const UPtr extraLo=reinterpret_cast<UPtr>(&g_mapExtra[0]);
                        const UPtr extraHi=reinterpret_cast<UPtr>(&g_mapExtra[kMapExtraNodes]);
                        if (allocatedCursor >= extraLo && allocatedCursor <= extraHi &&
                            ((allocatedCursor-extraLo) % sizeof(Node16)) == 0)
                            DSF150_CompletedAllocated = static_cast<long>(64u +
                                (allocatedCursor-extraLo) / sizeof(Node16));
                    }
                    const U32 metadata = Read32(package + 0x72F8u);
                    if (Readable(metadata + 0x18u, 1u))
                        DSFV27B_LiveAdmission = static_cast<long>(
                            *reinterpret_cast<volatile const U8*>(metadata + 0x18u));
                    const U32 handlers = Read32(package + 0x5730u);
                    const U32 kind0 = Read32(package + 0x5834u);
                    if (handlers <= 10000u) {
                        DSF150_GlobalHandlerCount = static_cast<long>(handlers);
                        if (static_cast<long>(handlers) > DSF150_HandlerHighWater)
                            DSF150_HandlerHighWater = static_cast<long>(handlers);
                    }
                    if (kind0 <= 10000u) DSF150_Kind0HandlerCount = static_cast<long>(kind0);
                    if (active >= 64u || handlers >= 64u || kind0 >= 64u) {
                        DSF150_AtOrOver64 = 1;
                        if (!reached64Logged) {
                            Log("DSFV29b: 64-entry occupancy reached; experimental redirection must be monitored.\n");
                            reached64Logged = true;
                        }
                    }
                }
            }
        }
        if (DSFV27B_UnsafeBoundary && !badBoundaryLogged) {
            Log("DSFV29b: FATAL boundary/consistency guard triggered; STOP game and capture logs.\n");
            badBoundaryLogged=true;
        }
        if ((loops % 25u) == 0u &&
            (previousMap != DSF150_CompletedActive || previousSlots != DSF150_GraphicsSlots ||
             previousRegionSlots != DSFV28_CityRegionSlots ||
             previousChapter7Matches != DSF_Chapter7PreloadExact18Matches ||
             previousChapter7Gate != DSF_Chapter7PreloadGate ||
             (DSFV29_HashNativeRemaining>=0 &&
              (DSFV29_HashNativeRemaining/1000)!=(previousHashRemaining/1000)) ||
             (DSFV29_HashExtraGranted/1000)!=(previousHashExtra/1000))) {
            LogTelemetry();
            previousMap = DSF150_CompletedActive;
            previousSlots = DSF150_GraphicsSlots;
            previousRegionSlots = DSFV28_CityRegionSlots;
            previousChapter7Matches = DSF_Chapter7PreloadExact18Matches;
            previousChapter7Gate = DSF_Chapter7PreloadGate;
            previousHashRemaining = DSFV29_HashNativeRemaining;
            previousHashExtra = DSFV29_HashExtraGranted;
        }
        ++loops;
        Sleep(200u);
    }
}
#endif
} // namespace

extern "C" __declspec(dllexport) BOOL __stdcall DllMain(HMODULE module, DWORD reason, void*) {
    if (reason == 1u) { // DLL_PROCESS_ATTACH
        DisableThreadLibraryCalls(module);
        g_base = reinterpret_cast<UPtr>(GetModuleHandleW(nullptr));
#if defined(DSF_V29S_INDEXED_262144)
        CapacityOpen(); // Also record baseline/startup refusal before Monitor exists.
#endif
#if defined(DSF_V29S_CHAPTERS)
        if(!ChapterPreflight()){ChapterStartLine();ExitProcess(0x29F1u);return 0;}
#endif
        ApplyPreinitGraphicsExpansion();
        if (DSF150_Status == 1 && !InstallPool512Patches()) {
            RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status=-11;
            Log("DSFV29b: pool512 signature/write preflight failed. NO PATCHES RETAINED.\n");
        }
        if (DSF150_Status == 1 && !InstallRequestFramePatches()) {
            RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status=-12;
            Log("DSFV29b: request-frame instruction preflight failed. ALL PATCHES ROLLED BACK.\n");
        }
        if (DSF150_Status == 1 && !InstallMerge150Fix()) {
            RestoreMerge150Fix(); RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status=-13;
            Log("DSFV29b: merge150 helper preflight/install failed. ALL PATCHES ROLLED BACK.\n");
        }
        if (DSF150_Status == 1 && (!InstallExtendedSites())) {
            RestoreExtendedSites(); RestoreMerge150Fix(); RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status = -10;
            Log("DSFV29b: native signature/hook preflight failed. ALL INITIAL PATCHES ROLLED BACK.\n");
        }
        if (DSF150_Status == 1 && !InstallHashArenaHook()) {
            RemoveHashArenaHook(); RestoreExtendedSites(); RestoreMerge150Fix();
            RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status = -14;
            Log("DSFV29b: hash arena preflight/allocation failed; ALL INITIAL PATCHES ROLLED BACK.\n");
        }
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
        if (DSF150_Status == 1) {
            g_sRegMapHook=DiagSRegMapInstall()?1:-1;
            if(g_sRegMapHook!=1) {
                RemoveHashArenaHook(); RestoreExtendedSites(); RestoreMerge150Fix();
                RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
                DSF150_Status=-16;
                Log("DSFV29s: vehicle-definition map redirect preflight failed; initial patches rolled back.\n");
            }
        }
        if (DSF150_Status == 1 && !InstallModelPoolCapacity()) {
            RestoreModelPoolCapacity(); DiagSRegMapRestore(); RemoveHashArenaHook(); RestoreExtendedSites(); RestoreMerge150Fix();
            RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status = -15;
            Log("DSFV29s: vehicle-definition pool preinit patch failed; initial patches rolled back.\n");
        }
#endif
#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
        if (DSF150_Status == 1 && !InstallResourceCollection()) {
            RestoreResourceCollection();
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
            RestoreModelPoolCapacity(); DiagSRegMapRestore();
#endif
            RemoveHashArenaHook(); RestoreExtendedSites(); RestoreMerge150Fix();
            RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status=-17;
            Log("DSFV29s: resource collection preflight/redirect failed; initial patches rolled back.\n");
        }
#endif
        if (DSF150_Status == 1 && !InstallHandlerHook()) {
#if defined(DSF_V29S_RESOURCE_COLLECTION_EXPANDED)
            RestoreResourceCollection();
#endif
#if defined(DSF_V29S_MODEL_POOL_EXPANDED)
            RestoreModelPoolCapacity();
            DiagSRegMapRestore();
#endif
            RemoveHashArenaHook(); RestoreExtendedSites(); RestoreMerge150Fix(); RestoreRequestFramePatches(); RestorePoolPatches(); RollbackGraphicsConfig();
            DSF150_Status = -9;
            Log("DSFV29b: handler detour failed; all experimental patches rolled back.\n");
        }
        if (DSF150_Status == 1) {
#if !defined(DSF_V29S_PERF_CONTROL)
            // Two extra passive callback sites, exact bytes verified offline.
            // An optional hook install failure keeps original V29b hooks active.
            InstallV29cCallbackHooks();
            DSFV29f_StreamHooks=DiagFInstallStreamHooks()?2:-1;
            DSFV29h_Hooks=DiagHInstallHooks()?2:-1;
#endif
#if defined(DSF_V29S_NATIVE_TRACE)
#if defined(DSF_V29S_PERF_CONTROL)
            DSFV29s_Hooks=DiagSInstall()?5:-1;
            g_sMutationHooks=DSFV29s_Hooks==5 && DiagSMutationInstall()?3:-1;
            g_sGuardHook=DSFV29s_Hooks==5 && g_sMutationHooks==3 && DiagSGuardInstall()?1:-1;
#else
            DSFV29s_Hooks=DiagSInstall()?5:-1;
            Log(DSFV29s_Hooks==5 ? "DSFV29s: five passive native dispatch/lifecycle hooks armed.\n" : "DSFV29s: preflight failed; V29p retained without V29s probes.\n");
            g_sMutationHooks=DSFV29s_Hooks==5 && DiagSMutationInstall()?3:-1;
            g_sGuardHook=DSFV29s_Hooks==5 && g_sMutationHooks==3 && DiagSGuardInstall()?1:-1;
#endif
#if defined(DSF_V29S_SPOOL0_EXPANDED)
            g_sScratchHook=g_sGuardHook==1 && DiagSScratchInstall()?2:-1;
#endif
#if defined(DSF_V29S_CLONE_EXPANDED)
            g_sCloneHook=g_sScratchHook==2 && DiagSCloneInstall()?1:-1;
#endif
#if defined(DSF_V29S_BUCKET_DIAG)
            g_sBucketHook=g_sCloneHook==1 && DiagSBucketInstall()?1:-1;
#endif
#if defined(DSF_V29S_MODEL_DIAG)
            g_sModelHook=g_sCloneHook==1 && DiagSModelInstall()?1:-1;
#endif
#if defined(DSF_V29S_LIVE_MODEL_DIAG)
            g_sLiveModelHooks=g_sCloneHook==1 && DiagSLiveModelInstall()?2:-1;
#endif
#if defined(DSF_V29S_REG_SOURCE_DIAG)
            g_sRegHooks=g_sCloneHook==1 && DiagSRegInstall()?3:-1;
#endif
#if defined(DSF_V29S_EXTERIOR_TRACE)
            g_sExteriorHook=g_sCloneHook==1 && DiagSExteriorInstall()?1:-1;
            g_sDrawHooks=g_sExteriorHook==1 && DiagSDrawInstall()?2:-1;
#endif
#if defined(DSF_V29S_LOD_TRACE)
            g_sLodHook=g_sDrawHooks==2 && DiagSLodInstall()?1:-1;
            g_sBodyHook=g_sLodHook==1 && DiagSBodyInstall()?1:-1;
            g_sCommonHooks=g_sDrawHooks==2 && DiagSCommonInstall()?2:-1;
#endif
#if defined(DSF_V29S_VISUAL_TRACE)
            g_vVisualHooks=g_sCommonHooks==2 && DiagVVisualInstall()?2:-1;
            g_vContextHook=g_vVisualHooks==2 && DiagVContextInstall()?1:-1;
#endif
#if defined(DSF_V29S_SUBMISSION_TRACE)
            g_vSubmitHook=g_sCommonHooks==2 && DiagVSubmissionInstall()?1:-1;
#endif
#if defined(DSF_V29S_DRAW_QUEUE_TRACE)
            g_vDrawQueueHook=DiagVDrawQueueInstall()?3:-1;
#endif
#if defined(DSF_V29S_HANDLE_CHAIN_TRACE)
            g_vHandleHooks=DiagVHandleInstall()?8:-1;
#endif
#if defined(DSF_V29S_HASH_HEAD_TRACE)
            g_vHashHeadHooks=DiagVHashHeadInstall()?4:-1;
#endif
#if defined(DSF_V29S_AUDIO_TRACE)
            g_audioHooks=DiagAudioInstall()?5:-1;
#endif
#if defined(DSF_V29S_POOL_DIAG)
            g_sPoolHooks=g_sGuardHook==1 && DiagSPoolInstall()?2:-1;
#endif
#if defined(DSF_V29S_LISTA_EXPANDED)
            g_sListAHook=g_sGuardHook==1 && DiagSListAInstall()?1:-1;
#endif
#if defined(DSF_V29S_LISTB_EXPANDED)
            g_sListBHook=g_sGuardHook==1 && DiagSListBInstall()?1:-1;
#endif
#if defined(DSF_V29S_CAPACITY_TRACE)
#if !defined(DSF_V29S_INDEXED_262144)
            CapacityOpen();
#endif
            g_capacityHooks=DSFV29s_Hooks==5 && g_sGuardHook==1 &&
                g_sScratchHook==2 && g_sCloneHook==1 && CapacityInstall()?3:-1;
#if defined(DSF_V29S_INDEXED_262144)
            if(g_capacityHooks==3) IndexedInstall();
            else {g_indexedStatus=-8;g_indexedReason="baselineHooksUnavailable";}
            IndexedStartLine();
#if defined(DSF_V29S_AUDIO_PROBE)
            AudioProbeInstall();
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
            AudioDefaultPoolInstall();
            AudioDefaultPoolStartLine();
#endif
#if defined(DSF_V29S_AUDIO_SLOTS_2048)
            AudioSlotsInstall();
            AudioSlotsStartLine();
#endif
#if defined(DSF_V29S_AUDIO_REASON_PROBE)
            AudioReasonInstall();
            AudioReasonStartLine();
#endif
#if defined(DSF_V29S_AUDIO_MODEL192)
            AudioModelInstall();
            AudioModelStartLine();
#endif
#if defined(DSF_V29S_CLEANUP_262144)
            CleanupInstall();CleanupStartLine();
            if(g_cleanupStatus!=1)CleanupFatal(g_cleanupReason);
#endif
#if defined(DSF_V29S_PENDING512)
            PendingInstall();PendingStartLine();
            if(g_pendingStatus!=1)PendingFatal(g_pendingReason,0u);
#endif
#if defined(DSF_V29S_CHAPTERS)
            ChapterStartLine();
#endif
#if defined(DSF_V29S_CALLBACK512)
            CallbackInstall();CallbackStartLine();
            if(g_callbackStatus!=1)CallbackFatal(g_callbackReason,0u);
#endif
#if defined(DSF_V29S_ARENA_REUSE)
            ArenaReuseStartLine();
#if defined(DSF_V29S_BASELINE150)
            BaselineInstall();
#if defined(DSF_V29S_NOTIFY150)
            NotifyInstall();
#if defined(DSF_V29S_PARKED150)
            ParkInstall();
#if defined(DSF_V29S_REPLAY150)
            ReplayInstall();
#endif
#endif
#endif
#endif
#endif
#if defined(DSF_V29S_AUDIO_STATIC_64M)
            AudioStaticPoolInstall();
            AudioStaticPoolStartLine();
#endif
            AudioProbeStartLine();
#endif
#endif
#endif
#if !defined(DSF_V29S_PERF_CONTROL)
            DSFV29s_WaitHooks=DSFV29s_Hooks==5 && DiagSWaitInstall()?2:-1;
            Log(DSFV29s_WaitHooks==2 ? "DSFV29s: two passive traffic-wait failure hooks armed.\n" : "DSFV29s: traffic-wait hooks unavailable.\n");
            g_sMemberDetailSignature=DSFV29s_WaitHooks==2 && DiagSMemberDetailCheck()?1:-1;
#endif
#elif defined(DSF_V29R_CALLERS)
            DSFV29r_Hooks=DiagRInstallHooks()?2:-1;
            Log(DSFV29r_Hooks==2 ? "DSFV29r: two passive caller/return hooks armed; no step hook.\n" : "DSFV29r: hook preflight failed; V29p active without V29r probes.\n");
#elif defined(DSF_V29Q_TRAVERSAL)
            DSFV29q_Hooks=DiagQInstallHooks()?3:-1;
            Log(DSFV29q_Hooks==3 ? "DSFV29q: three passive traversal hooks armed.\n" : "DSFV29q: hook preflight failed; base V29p active without traversal probes.\n");
#endif
#if !defined(DSF_V29S_PERF_CONTROL)
            Log(DSFV29f_StreamHooks==2 ? "DSFV29f: stream dispatch/residency counters active.\n" : "DSFV29f: stream signature mismatch; pool/memory snapshots remain active.\n");
#endif
            Log("DSFV29b: 150 admission + pool512 + request150/frame150 armed; awaiting pkg0 initializer.\n");
            HANDLE worker = CreateThread(nullptr, 0, &Monitor, nullptr, 0, nullptr);
            if (worker) CloseHandle(worker);
        }
#if defined(DSF_V29S_INDEXED_262144)
        if(g_indexedStatus==0) {
            g_indexedStatus=-8;g_indexedReason="baselineStartupRefused";
            IndexedStartLine();
#if defined(DSF_V29S_AUDIO_PROBE)
            g_audioProbeStatus=-1;g_audioProbeReason="baselineStartupRefused";
#if defined(DSF_V29S_AUDIO_DEFAULT_16M) || defined(DSF_V29S_AUDIO_DEFAULT_64M)
            g_audioDefaultStatus=-1;g_audioDefaultReason="baselineStartupRefused";
            AudioDefaultPoolStartLine();
#endif
#if defined(DSF_V29S_AUDIO_SLOTS_2048)
            g_audioSlotsStatus=-1;g_audioSlotsReason="baselineStartupRefused";
            AudioSlotsStartLine();
#endif
#if defined(DSF_V29S_AUDIO_REASON_PROBE)
            g_audioReasonStatus=-1;g_audioReasonReason="baselineStartupRefused";
            AudioReasonStartLine();
#endif
#if defined(DSF_V29S_AUDIO_MODEL192)
            g_audioModelStatus=-1;g_audioModelReason="baselineStartupRefused";
            AudioModelStartLine();
#endif
#if defined(DSF_V29S_CLEANUP_262144)
            g_cleanupStatus=-1;g_cleanupReason="baselineStartupRefused";CleanupStartLine();
#endif
#if defined(DSF_V29S_PENDING512)
            g_pendingStatus=-1;g_pendingReason="baselineStartupRefused";PendingStartLine();
#endif
#if defined(DSF_V29S_CALLBACK512)
            g_callbackStatus=-1;g_callbackReason="baselineStartupRefused";CallbackStartLine();CallbackFatal(g_callbackReason,0u);
#endif
#if defined(DSF_V29S_AUDIO_STATIC_64M)
            g_audioStaticStatus=-1;g_audioStaticReason="baselineStartupRefused";
            AudioStaticPoolStartLine();
#endif
            AudioProbeStartLine();
#endif
        }
#endif
    }
    return 1;
}

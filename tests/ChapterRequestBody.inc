// Exact production request bridge body.
extern "C" void __fastcall RequestWriteBridge(void* self, void*, const U32* list, int requested) {
    const UPtr ctx=reinterpret_cast<UPtr>(self);
#if defined(DSF_V29S_REG_SOURCE_DIAG)
    const U32* const originalList=list;
    const int originalRequested=requested;
#endif
    if (!Readable(ctx, 0xDB0u) || requested > static_cast<int>(kReqCapacity) ||
        (requested > 0 && (!list || !Readable(reinterpret_cast<UPtr>(list),
                                                  static_cast<U32>(requested)*4u)))) {
        ++DSFV27B_RequestRejected;
        Log("DSFV29b: REJECT request with bad count/context/source, previous request retained.\n");
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
        return;
    }
    for (U32 i=0;i<count;++i) {
        const U32 id=list[i];
        if (id > 0xFFFFu) {
            RequestUnlock();
            ++DSFV27B_RequestRejected;
            Log("DSFV29b: REJECT request: vehicle ID exceeds 16 bits.\n");
            return;
        }
    }
    // Copy the full list BEFORE advertising a new count to the game.
    for (U32 i=0;i<count;++i) slot->ids[i]=list[i];
    slot->count=count;
    for (U32 i=0;i<count && i<kReqInline;++i) Write32(ctx+i*4u,slot->ids[i]);
    Write32(ctx+0xC8u,count);
    RequestUnlock();
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

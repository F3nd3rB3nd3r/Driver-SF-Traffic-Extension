extern "C" __declspec(noinline) void __cdecl LegacyPrepareScratch(U32 site,UPtr frame) {
#if defined(DSF_V29S_ARENA_REUSE)
    const char* why="scratchIntegrity";while(InterlockedExchange(&g_reuseLock,1)){}
    const bool okay=ReusePrepareScratch(site,frame,why);
    const bool saved=Readable(frame,36u);const UPtr esp=saved?Read32(frame+12u)+4u:0u;
    const bool readable=esp && Readable(esp+0x14u,12u);
    const U32 active=readable?Read32(esp+0x1Cu):0u,free=readable?Read32(esp+0x18u):0u,cursor=saved?Read32(frame+28u):0u;
    InterlockedExchange(&g_reuseLock,0);
    if(!okay){InterlockedIncrement(&g_sScratchFault);LegacyExit(0x2Du,why,esp,cursor,active,free);}
#else
    InterlockedIncrement(&g_sScratchCalls);
    const UPtr originalEsp=Read32(frame+12u)+4u;
    const UPtr cursor=Read32(frame+28u); // EAX at 005A75DD
    const UPtr sentinel=originalEsp+0x278u;
    const U32 freeCount=Read32(originalEsp+0x18u);
    if(freeCount)return; // native recycled-node path does not use EAX as fresh cursor
    if(cursor>=originalEsp+0x20u && cursor<sentinel &&
       ((cursor-(originalEsp+0x20u))%12u)==0u)return;
    while(InterlockedExchange(&g_sScratchLock,1)){}
    bool okay=false;
    for(U32 i=0;i<static_cast<U32>(g_sScratchUsed);++i) {
        const DiagSScratchArena& a=g_sScratchArenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSScratchNodes]);
        if(a.sentinel==sentinel && cursor>=lo && cursor<hi &&
           ((cursor-lo)%12u)==0u) {okay=true;break;}
    }
    if(!okay && cursor==sentinel &&
       static_cast<U32>(g_sScratchUsed)<kSScratchArenas) {
        DiagSScratchArena& a=g_sScratchArenas[g_sScratchUsed++];
        a.sentinel=sentinel;
        const UPtr replacement=reinterpret_cast<UPtr>(&a.nodes[0]);
        Write32(frame+28u,replacement); // EAX: fresh cursor
        if(site==0u)Write32(frame+20u,replacement); // EDX at 005A75C4
        else Write32(frame+24u,replacement); // ECX at 005A764C
        Write32(originalEsp+0x14u,replacement); // cursor reloaded at 005A75E7
        InterlockedIncrement(&g_sScratchRedirects);
        okay=true;
    }
    InterlockedExchange(&g_sScratchLock,0);
    if(!okay) {
        InterlockedIncrement(&g_sScratchFault);
        OutputDebugStringA("DSFV29s: reconcile scratch arena unavailable before insert.\n");
        LegacyExit(0x2Du,"scratchStorage",sentinel,cursor,Read32(originalEsp+0x1Cu),freeCount);
    }
#endif
}

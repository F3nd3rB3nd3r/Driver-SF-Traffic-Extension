extern "C" __declspec(noinline) void __cdecl DiagSPrepareSpool0(UPtr owner) {
#if defined(DSF_V29S_ARENA_REUSE)
    const char* why="spoolIntegrity";while(InterlockedExchange(&g_reuseLock,1)){}
    const bool okay=ReusePrepareSpool(owner,why);
    const bool readable=owner && Readable(owner+0xCCu,12u);
    const U32 cursor=readable?Read32(owner+0xCCu):0u,active=readable?Read32(owner+0xD4u):0u,free=readable?Read32(owner+0xD0u):0u;
    InterlockedExchange(&g_reuseLock,0);
    if(!okay){InterlockedIncrement(&g_sSpool0Fault);LegacyExit(0x2Cu,why,owner,cursor,active,free);}
#else
    if(g_sScratchHook!=2 || !owner || !Readable(owner+0xCCu,12u))return;
    while(InterlockedExchange(&g_sSpool0Lock,1)){}
    const UPtr header=owner+0xCCu,cursor=Read32(header);
    const U32 freeCount=Read32(header+4u),active=Read32(header+8u);
    if(active>static_cast<U32>(g_sSpool0High))g_sSpool0High=static_cast<long>(active);
    bool okay=freeCount!=0u;
    for(U32 i=0;i<static_cast<U32>(g_sSpool0Used);++i) {
        const DiagSSpool0Arena& a=g_sSpool0Arenas[i];
        const UPtr lo=reinterpret_cast<UPtr>(&a.nodes[0]);
        const UPtr hi=reinterpret_cast<UPtr>(&a.nodes[kSSpool0Nodes]);
        if(a.owner==owner && cursor>=lo && cursor<hi && ((cursor-lo)%12u)==0u)
            okay=true;
    }
    if(!okay && freeCount==0u && active<=50u &&
       cursor>=owner+0xD8u && cursor<=owner+0x330u &&
       ((cursor-(owner+0xD8u))%12u)==0u &&
       static_cast<U32>(g_sSpool0Used)<kSSpool0Arenas) {
        DiagSSpool0Arena& a=g_sSpool0Arenas[g_sSpool0Used++];
        a.owner=owner;
        Write32(header,reinterpret_cast<UPtr>(&a.nodes[0]));
        InterlockedIncrement(&g_sSpool0Redirects);
        okay=true;
    }
    InterlockedExchange(&g_sSpool0Lock,0);
    if(!okay) {
        InterlockedIncrement(&g_sSpool0Fault);
        OutputDebugStringA("DSFV29s: first spooler pool unavailable before reconcile.\n");
        LegacyExit(0x2Cu,"spoolStorage",owner,cursor,active,freeCount);
    }
#endif
}
extern "C" __declspec(noinline) void __cdecl DiagSPrepareScratch(U32 site,UPtr frame) {
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

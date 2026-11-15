static bool WideStartupStorage(){
    if(!g_limits.valid)return false;g_wideFailureSink=&WideFailureLine;
    // Allocate every shared pointer backing before any native hook can use it.
    g_globalHandlers=static_cast<U32*>(WideAllocate(kHandlerArraySidecar*4u));
    g_kind0Handlers=static_cast<U32*>(WideAllocate(kHandlerArraySidecar*4u));
    g_mapExtra=static_cast<Node16*>(WideAllocate(kMapExtraNodes*16u));
    g_sResourcePointers=static_cast<U32*>(WideAllocate(kResourceCollectionCapacity*4u));
    g_sResourcePairs=static_cast<U32*>(WideAllocate(kResourceCollectionCapacity*8u));
    WideBaselineInit(g_baseline);WideBaselineInit(g_baselinePrint);WideBaselineInit(g_baselineEventPrint.state);
    for(U32 i=0;i<kBaselineEvents;++i)WideBaselineInit(g_baselineEvents[i].state);
    g_wideClass16=WideClass16Nodes();g_wideRequestFrame=12u*GenericRequestStorage()+24u;
    // Reject impossible stack geometry before installing any patch.
    UPtr current=reinterpret_cast<UPtr>(&current);WideCheckStack(current,WideMax(g_wideRequestFrame,GenericParkFrame()));
    DSFV29p_ConfigTarget=g_limits.models;DSFV29p_ConfigBudgetA=WideBudget(g_limits.models,8192u);DSFV29p_ConfigBudgetB=WideBudget(g_limits.models,65536u);
    return true;
}

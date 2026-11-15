#include "V29sTrafficQueuesCore.inc"
static void TrafficQueueRequire(U32 kind,UPtr owner){
    const char* why=nullptr;if(TrafficQueuePrepare(kind,owner,why))return;
    char line[700]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_TRAFFIC_QUEUE_FATAL reason=",end);AppendText(p,why?why:"unknown",end);
    AppendText(p," queue=",end);AppendText(p,kCapacityNames[kind+1u],end);
    DiagFAppendKeyHex(p," owner=",owner,end);DiagFAppendKeyU(p," capacity=",TrafficQueueCapacity(),end);
    AppendText(p," beforeNativeWrite=1\n",end);*p=0;CapacityWrite(line);
    WideAllocationFailure("Unlimiter: traffic queue invalid or configured capacity exhausted.\n");
}
static void TrafficQueuesLog(bool startup,U32 tick){
    for(U32 kind=0;kind<3u;++kind){
        const TrafficQueueStats& s=g_trafficQueueStats[kind];char line[1000]={};char* p=line;const char* end=line+sizeof(line)-1u;
        AppendText(p,startup?"DSFV29s_TRAFFIC_QUEUE_START revision=1":"DSFV29s_TRAFFIC_QUEUE_STATUS",end);
        if(startup)DiagFAppendKeyU(p," status=",g_capacityHooks==3?1u:0u,end);
        AppendText(p," queue=",end);AppendText(p,kCapacityNames[kind+1u],end);
        DiagFAppendKeyU(p," tick=",tick,end);DiagFAppendKeyU(p," capacity=",TrafficQueueCapacity(),end);
        DiagFAppendKeyU(p," stride=",kTrafficQueueStrides[kind],end);
        DiagFAppendKeyU(p," checks=",static_cast<U32>(s.checks),end);DiagFAppendKeyU(p," generations=",static_cast<U32>(s.generations),end);
        DiagFAppendKeyU(p," externalBuffers=",static_cast<U32>(s.buffers),end);DiagFAppendKeyU(p," redirects=",static_cast<U32>(s.redirects),end);
        DiagFAppendKeyU(p," recycles=",static_cast<U32>(s.recycles),end);DiagFAppendKeyU(p," resets=",static_cast<U32>(s.resets),end);
        DiagFAppendKeyU(p," failures=",static_cast<U32>(s.failures),end);DiagFAppendKeyU(p," guardBad=",static_cast<U32>(s.guardBad),end);
        DiagFAppendKeyU(p," projectedPeak=",static_cast<U32>(s.peak),end);DiagFAppendKeyHex(p," lastOwner=",s.owner,end);
        DiagFAppendKeyU(p," activeBeforeLast=",s.active,end);DiagFAppendKeyU(p," recycledBeforeLast=",s.recycled,end);
        DiagFAppendKeyU(p," issuedBeforeLast=",s.issued,end);
        AppendText(p," nativeCapacity=50 processLifetime=1 coldReset=newGeneration beforeNativeWrite=1\n",end);*p=0;CapacityWrite(line);
    }
}

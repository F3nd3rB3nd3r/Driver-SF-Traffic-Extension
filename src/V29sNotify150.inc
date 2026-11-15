#include "V29sNotify150Native.inc"
#include "V29sNotify150Layout.inc"
#include "V29sNotify150Core.inc"
static long g_notifyStatus=0;
static void NotifyFatal(const char* reason,UPtr source,U32 bytes) {
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_NOTIFY150_FATAL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyHex(p," source=",source,end);DiagFAppendKeyU(p," bytes=",bytes,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);ExitProcess(0x29F2u);
}
static void NotifyEvent(UPtr source,U32 count,U32 hash,U32 first,U32 last,bool sidecar) {
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_NOTIFY150_COPY",end);DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);
    DiagFAppendKeyHex(p," source=",source,end);DiagFAppendKeyU(p," count=",count,end);
    AppendText(p,sidecar?" storage=sidecar":" storage=nativeInline",end);
    DiagFAppendKeyHex(p," rosterHash=",hash,end);DiagFAppendKeyU(p," first=",first,end);DiagFAppendKeyU(p," last=",last,end);
    AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void NotifyInstall() {
    const UPtr callback=Address(0x00754C70u),copy=reinterpret_cast<UPtr>(&NotifyCopy);
    bool ok=g_baselineInstalled && ArenaReuseStartup() && !g_pkg0 && VerifyExecutableIdentity() &&
        Readable(callback,sizeof(kNotifyNative)) && NotifyBytesMatch(callback,copy,false);
    DWORD old=0,discarded=0;const UPtr page=Address(0x00754000u);
    if(ok)ok=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&old)!=0;
    if(ok) {
        if(g_pkg0 || !NotifyBytesMatch(callback,copy,false)) {
            const BOOL restored=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded);
            g_notifyStatus=-1;if(!restored)NotifyFatal("precommitProtection",callback,0u);
        }else {
            NotifyApply(callback,copy,true);
            const bool exact=NotifyBytesMatch(callback,copy,true);
            const BOOL flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
            const BOOL protectedAgain=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded);
            if(exact && flushed && protectedAgain)g_notifyStatus=1;
            else {
                DWORD retry=0;if(!VirtualProtect(reinterpret_cast<void*>(page),0x1000u,kPageExecuteReadWrite,&retry))NotifyFatal("rollbackUnlock",callback,0u);
                NotifyApply(callback,copy,false);const bool restored=NotifyBytesMatch(callback,copy,false);
                const BOOL rollbackFlush=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(page),0x1000u);
                const BOOL rollbackProtect=VirtualProtect(reinterpret_cast<void*>(page),0x1000u,old,&discarded);
                if(!restored || !rollbackFlush || !rollbackProtect)NotifyFatal("rollbackIncomplete",callback,0u);
                g_notifyStatus=-1;
            }
        }
    }
#if defined(DSF_V29S_WIDE_LIMITS)
    if(g_notifyStatus==1 && !WideInstallNotifyProbe(callback))g_notifyStatus=-1;
#endif
    if(g_notifyStatus!=1) {
        CapacityWrite("DSFV29s_NOTIFY150_START revision=1 status=-1 reason=parentSignatureOrCommit\n");
        NotifyFatal("startupPatchRefused",callback,0u);return;
    }
#if defined(DSF_V29S_GENERIC_LIMITS)
    char line[600]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_NOTIFY150_START revision=1 status=1 patches=5 nativeCapacity=50",end);
    DiagFAppendKeyU(p," capacity=",g_limits.requests,end);DiagFAppendKeyU(p," frameBytes=",GenericNotifyFrame(),end);
    AppendText(p," source=matchedRequestSidecar memcpyScope=00754CD0 requests=150 graphics=160 otherCapacityChanges=0\n",end);*p=0;CapacityWrite(line);
#else
    CapacityWrite("DSFV29s_NOTIFY150_START revision=1 status=1 patches=5 nativeCapacity=50 capacity=150 frameBytes=608 source=matchedRequestSidecar memcpyScope=00754CD0 requests=150 graphics=160 otherCapacityChanges=0\n");
#endif
}
static void NotifyPoll() {
    static U32 last=0;const U32 now=GetTickCount()-g_capacityStart;if(now-last<1000u)return;last=now;
    char line[500]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_NOTIFY150_STATUS",end);DiagFAppendKeyU(p," tick=",now,end);
    AppendText(p," status=",end);AppendNumber(p,g_notifyStatus,end);
    DiagFAppendKeyU(p," copies=",static_cast<U32>(g_notifyCopies),end);DiagFAppendKeyU(p," inlineCopies=",static_cast<U32>(g_notifyStock),end);
    DiagFAppendKeyU(p," sidecarCopies=",static_cast<U32>(g_notifySidecar),end);DiagFAppendKeyU(p," peakCount=",static_cast<U32>(g_notifyPeak),end);
    DiagFAppendKeyU(p," invalid=",static_cast<U32>(g_notifyInvalid),end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

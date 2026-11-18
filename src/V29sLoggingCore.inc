// Output selection only. Every safety hook, counter and native decision stays
// active. Full tracing takes a byte-preserving fast path before filtering.
struct LoggingStream {U32 key,tick;bool used;};
static LoggingStream g_loggingStreams[128]={};
static volatile long g_loggingLock=0,g_loggingAttempted=0,g_loggingAccepted=0,g_loggingSuppressed=0;
static U32 g_loggingReadySeq=0;
static U32 g_loggingRosterSeq[2]={};
static const U32 kLoggingSummaryMs=30000u;
static bool LoggingEqual(const char* p,const char* e,const char* word){while(p<e && *word && *p==*word){++p;++word;}return p==e && !*word;}
static bool LoggingContains(const char* p,const char* e,const char* word){
    for(;p<e;++p){const char* a=p;const char* b=word;while(a<e && *b && *a==*b){++a;++b;}if(!*b)return true;}return false;
}
static bool LoggingField(const char* line,const char* key,U32& value){
    const char* p=line;while(*p && *p!=' ' && *p!='\n' && *p!='\r')++p;
    while(*p){while(*p==' ' || *p=='\t')++p;const char* a=p;while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r')++p;const char* e=p;
        const char* eq=a;while(eq<e && *eq!='=')++eq;
        if(eq<e && LoggingEqual(a,eq,key)){value=0;++eq;if(eq==e)return false;for(;eq<e;++eq){if(*eq<'0' || *eq>'9')return false;U32 d=*eq-'0';if(value>(0xFFFFFFFFu-d)/10u)return false;value=value*10u+d;}return true;}
        if(*p=='\n' || *p=='\r')break;
    }return false;
}
static bool LoggingFieldIs(const char* line,const char* key,const char* word){
    for(const char* p=line;*p;){while(*p==' ' || *p=='\t')++p;const char* a=p;while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r')++p;
        const char* eq=a;while(eq<p && *eq!='=')++eq;if(eq<p && LoggingEqual(a,eq,key) && LoggingEqual(eq+1,p,word))return true;
        if(*p=='\n' || *p=='\r')break;
    }return false;
}
static bool LoggingEssential(const char* line,const char* end){
    if(LoggingContains(line,end,"FATAL") || LoggingContains(line,end,"EXCEPTION") || LoggingContains(line,end,"_ERROR") ||
       LoggingContains(line,end,"_REFUSED") || LoggingContains(line,end,"_BOUNDARY") || LoggingFieldIs(line,"valid","0"))return true;
    for(const char* p=end;*p;){while(*p==' ' || *p=='\t')++p;const char* a=p;while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r')++p;
        const char* eq=a;while(eq<p && *eq!='=')++eq;
        if(eq+1<p && eq[1]=='-' && (LoggingEqual(a,eq,"status") || LoggingEqual(a,eq,"startupStatus") ||
            LoggingEqual(a,eq,"indexedStatus") || LoggingEqual(a,eq,"baselineStatus")))return true;
        if(*p=='\n' || *p=='\r')break;}
    const char* errors[]={"failures","guardBad","invalid","full","allocationFailed","faults","drainMismatch","readFailures","clockFailures",
        "signatureFailed","attachRefused","instanceOverflow","keyOverflow","eventDrops","memorySnapshotFailures","detailDropped","threadDrops","dropped",
        "unknown","concurrentSkips","concurrent","fileStatus","ceilingHits","invalidCounts","createFailed","refused","defaultProfileMissing","modelProfileMissing",
        "bankNotReady","vehicleDisabled","invalidMode","modeInitializer","vehicleState","snapshotGetter","snapshotFlag"};
    U32 v=0;for(const char* key:errors)if(LoggingField(line,key,v) && v)return true;
    // Native fallbacks of the mission query are expected. Reader fallbacks
    // indicate that the configured reader backing was not used as intended.
    if(LoggingEqual(line,end,"DSFV29s_REPLAY_READ_STATUS") &&
       ((LoggingField(line,"fallbacks",v) && v) || (LoggingField(line,"unmanagedCalls",v) && v)))return true;
    // A preview rejection is native policy; retain its cumulative summary.
    // Other reason records are actionable context even with LogLevel=0.
    if(LoggingContains(line,end,"_AUDIO_SETUP_DETAIL") && !LoggingFieldIs(line,"reason","previewSuppressed"))return true;
    return false;
}
static bool LoggingPeriodic(const char* line,const char* end,U32 now){
    U32 hash=2166136261u;
    for(const char* p=line;p<end;++p)hash=(hash^static_cast<U8>(*p))*16777619u;
    // Preserve separate streams for list/queue/owner counters.
    for(const char* p=end;*p;){while(*p==' ' || *p=='\t')++p;const char* a=p;while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r')++p;
        const char* eq=a;while(eq<p && *eq!='=')++eq;
        if(eq<p && (LoggingEqual(a,eq,"list") || LoggingEqual(a,eq,"queue") || LoggingEqual(a,eq,"owner") || LoggingEqual(a,eq,"name")))
            for(const char* v=a;v<p;++v)hash=(hash^static_cast<U8>(*v))*16777619u;
        if(*p=='\n' || *p=='\r')break;
    }
    if(InterlockedCompareExchange(&g_loggingLock,1,0))return true; // No blocking or lost faults.
    U32 index=hash&127u;
    for(U32 i=0;i<128u;++i){const U32 at=(hash+i)&127u;if(!g_loggingStreams[at].used || g_loggingStreams[at].key==hash){index=at;break;}}
    // If diagnostic streams outlive the bounded directory, replace one slot.
    // This can emit extra summaries, never disable safety/failure reporting.
    LoggingStream& stream=g_loggingStreams[index];
    const bool emit=!stream.used || stream.key!=hash || now-stream.tick>=kLoggingSummaryMs;
    if(emit){stream.used=true;stream.key=hash;stream.tick=now;}
    InterlockedExchange(&g_loggingLock,0);return emit;
}
static bool LoggingSelect(const char* line,U32 now){
    if(g_diagnosticLevel>=2u)return true;
    const char* end=line;while(*end && *end!=' ' && *end!='\n' && *end!='\r')++end;
    if(LoggingContains(line,end,"_START") || LoggingContains(line,end,"_LOGGING_STATUS") || LoggingEssential(line,end))return true;
    if(g_diagnosticLevel==0u)return false;
    if(LoggingEqual(line,end,"DSFV29s_GENERIC_EVENT") || LoggingEqual(line,end,"DSFV29s_REPLAY150_EVENT"))return true;
    if(LoggingEqual(line,end,"DSFV29s_BASELINE_PHASE")){
        if(LoggingFieldIs(line,"kind","accepted"))return true;
        U32 ready=0,count=0,checks=0,seq=0;
        if(LoggingField(line,"ready",ready) && LoggingField(line,"count",count) && ready==count && LoggingField(line,"checkpoints",checks) && checks && LoggingField(line,"seq",seq)){
            if(InterlockedCompareExchange(&g_loggingLock,1,0))return true;
            const bool emit=seq!=g_loggingReadySeq;if(emit)g_loggingReadySeq=seq;InterlockedExchange(&g_loggingLock,0);return emit;}
        return false;
    }
    if(LoggingEqual(line,end,"DSFV29s_BASELINE_ROSTER")){
        U32 seq=0;const U32 kind=LoggingFieldIs(line,"kind","effective")?1u:0u;
        if(!LoggingField(line,"seq",seq) || !seq)return true;
        if(InterlockedCompareExchange(&g_loggingLock,1,0))return true;
        const bool emit=g_loggingRosterSeq[kind]!=seq;if(emit)g_loggingRosterSeq[kind]=seq;
        InterlockedExchange(&g_loggingLock,0);return emit;
    }
    if(LoggingContains(line,end,"_STATUS") || LoggingEqual(line,end,"DSFV29s_AUDIO_MODEL_REGISTRY") ||
       LoggingEqual(line,end,"DSFV29s_AUDIO_SLOTS") || LoggingEqual(line,end,"DSFV29s_AUDIO_PRESSURE") ||
       LoggingEqual(line,end,"DSFV29s_BASELINE_METRIC"))return LoggingPeriodic(line,end,now);
    return false;
}
static bool LoggingAllow(const char* line,U32 now){
    InterlockedIncrement(&g_loggingAttempted);const bool allow=LoggingSelect(line,now);
    InterlockedIncrement(allow?&g_loggingAccepted:&g_loggingSuppressed);return allow;
}

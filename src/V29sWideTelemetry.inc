static bool WideContains(const char* text,const char* needle){
    for(;*text;++text){const char* a=text;const char* b=needle;while(*a && *b && *a==*b){++a;++b;}if(!*b)return true;}return false;
}
// Preserve the inherited record schema while reporting configured values.
// Only startup records are rewritten; model rosters and runtime observations
// pass through verbatim, regardless of their length.
static const char* WideStartupLine(const char* source,char* out,U32 bytes){
    if(!WideContains(source,"_START "))return source;
    const bool pending=WideContains(source,"PENDING_START"),callback=WideContains(source,"CALLBACK_START");
    const char* keys[]={" requests=150"," graphics=160"," privateNodes=256"," testCapacity=512"," extraNodes=448"," externalNodes=512"," generationBytes=7232"," scratchPolicy=expandedFrame150"," newHooks=0"," graphicsSlots=160"," indexedSlots=262144"," request=150"," slots=160"," target=160"};
    const U32 values[]={g_limits.requests,g_limits.models,WideMax(256u,g_limits.requests),pending?g_limits.pending:g_limits.callbacks,
        (pending?g_limits.pending:g_limits.callbacks)-64u,g_limits.definitions,72u+(g_limits.pending-64u)*16u,0,1,g_limits.models,g_limits.indexed,g_limits.requests,g_limits.models,g_limits.models};
    char* p=out;const char* end=out+bytes-1u;
    for(const char* s=source;*s;){bool replaced=false;
        for(U32 i=0;i<14u;++i){if((i==3u || i==4u) && !pending && !callback)continue;
            const char* a=s;const char* b=keys[i];while(*a && *b && *a==*b){++a;++b;}
            if(*b || (*a && *a!=' ' && *a!='\n' && *a!='\r'))continue;
            if(i==7u)AppendText(p," scratchPolicy=expandedFrame",end);
            else {const char* k=keys[i];while(*k && *k!='='){if(p<end)*p++=*k;++k;}if(p<end)*p++='=';DiagFAppendU(p,values[i],end);}
            s=a;replaced=true;break;
        }
        if(!replaced){if(p<end)*p++=*s;++s;}
    }
    *p=0;return out;
}
static void WideFailureLine(const char* reason){
    char line[1200]={};char* p=line;const char* end=line+sizeof(line)-1u;
    AppendText(p,"DSFV29s_USERCAP_FATAL",end);DiagFAppendKeyU(p," configuredRequests=",g_limits.requests,end);
    AppendText(p," detail=",end);AppendText(p,reason,end);if(p==line || p[-1]!='\n')AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

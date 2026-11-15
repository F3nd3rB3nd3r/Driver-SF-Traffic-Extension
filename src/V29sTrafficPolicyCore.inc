// Startup-only policy parsing. No CRT, allocation, engine access or capacity edits.
enum TrafficRosterMode {TrafficNative,TrafficPreset150};
struct TrafficPolicy {TrafficRosterMode mode;const char* reason;};
static TrafficPolicy g_trafficPolicy={TrafficNative,"missingFile"};
static char TrafficLower(char c){return c>='A' && c<='Z'?static_cast<char>(c+('a'-'A')):c;}
static bool TrafficEqual(const char* first,const char* last,const char* text) {
    while(first<last && *text)if(TrafficLower(*first++)!=TrafficLower(*text++))return false;
    return first==last && !*text;
}
static void TrafficTrim(const char*& first,const char*& last) {
    while(first<last && (*first==' ' || *first=='\t' || *first=='\r'))++first;
    while(last>first && (last[-1]==' ' || last[-1]=='\t' || last[-1]=='\r'))--last;
}
static TrafficPolicy TrafficParse(const char* bytes,U32 size) {
    TrafficPolicy result={TrafficNative,"noSetting"};
    if(size>4096u)return {TrafficNative,"fileTooLarge"};
    U32 offset=0;
    if(size>=3u && static_cast<U8>(bytes[0])==0xEFu && static_cast<U8>(bytes[1])==0xBBu && static_cast<U8>(bytes[2])==0xBFu)offset=3u;
    // ASCII settings, optionally UTF-8 BOM. Reject binary/UTF-16/control data.
    for(U32 i=offset;i<size;++i)if(static_cast<U8>(bytes[i])<32u && bytes[i]!='\r' && bytes[i]!='\n' && bytes[i]!='\t')return {TrafficNative,"invalidEncoding"};
    bool traffic=false,seen=false;
    while(offset<size) {
        const U32 start=offset;while(offset<size && bytes[offset]!='\n')++offset;
        const char* first=bytes+start;const char* last=bytes+offset;if(offset<size)++offset;
        for(const char* c=first;c<last;++c)if(*c==';' || *c=='#'){last=c;break;}
        TrafficTrim(first,last);if(first==last)continue;
        if(*first=='[') {
            if(last[-1]!=']')return {TrafficNative,"invalidSection"};
            ++first;--last;TrafficTrim(first,last);traffic=TrafficEqual(first,last,"Traffic");continue;
        }
        if(!traffic)continue;
        const char* equal=first;while(equal<last && *equal!='=')++equal;
        const char* keyEnd=equal;TrafficTrim(first,keyEnd);
        if(!TrafficEqual(first,keyEnd,"RosterMode"))continue;
        if(seen)return {TrafficNative,"duplicateSetting"};
        seen=true;if(equal==last)return {TrafficNative,"invalidMode"};
        first=equal+1;TrafficTrim(first,last);
        if(TrafficEqual(first,last,"Native"))result={TrafficNative,"configured"};
        else if(TrafficEqual(first,last,"Preset150"))result={TrafficPreset150,"configured"};
        else return {TrafficNative,"invalidMode"};
    }
    return result;
}
static const char* TrafficModeName(){return g_trafficPolicy.mode==TrafficPreset150?"Preset150":"Native";}

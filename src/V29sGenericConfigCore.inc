#if defined(DSF_V29S_WIDE_LIMITS)
#include "V29sWideConfigCore.inc"
#else
// First runtime-capacity milestone. No model IDs or chapter/profile tables.
struct GenericLimits {
    U32 requests,audioModels,audioSlots,audioMiB,replayMiB,retained,assetWeights;
    bool valid;const char* reason;
};
static GenericLimits g_limits={150u,192u,2048u,64u,1u,256u,1u,true,"missingFile"};
static char GenericLower(char c){return c>='A' && c<='Z'?static_cast<char>(c+32):c;}
static bool GenericEqual(const char* first,const char* last,const char* text){
    while(first<last && *text)if(GenericLower(*first++)!=GenericLower(*text++))return false;
    return first==last && !*text;
}
static void GenericTrim(const char*& first,const char*& last){
    while(first<last && (*first==' ' || *first=='\t' || *first=='\r'))++first;
    while(last>first && (last[-1]==' ' || last[-1]=='\t' || last[-1]=='\r'))--last;
}
static bool GenericNumber(const char* first,const char* last,U32& value){
    value=0;if(first==last)return false;
    for(;first<last;++first){if(*first<'0' || *first>'9')return false;const U32 digit=*first-'0';
        if(value>(0xFFFFFFFFu-digit)/10u)return false;value=value*10u+digit;}
    return true;
}
static GenericLimits GenericReject(GenericLimits v,const char* why){v.valid=false;v.reason=why;return v;}
static GenericLimits GenericValidate(GenericLimits v){
    // Above150 needs the separate wide-index/stack-probe migration, not wrapping.
    if(!v.requests || v.requests>150u)return GenericReject(v,"unsupportedRequestCapacity_max150");
    if(v.audioModels<45u || v.audioModels<v.requests || v.audioModels>512u)return GenericReject(v,"audioModelEntries_range45to512_andAtLeastRequests");
    if(v.audioSlots<255u || v.audioSlots>8192u)return GenericReject(v,"globalAudioEntries_range255to8192");
    if(v.audioMiB<16u || v.audioMiB>256u)return GenericReject(v,"defaultAudioMiB_range16to256");
    if(!v.replayMiB || v.replayMiB>16u || (v.replayMiB&(v.replayMiB-1u)))return GenericReject(v,"replayReaderMiB_powerOfTwo1to16");
    if(v.retained<50u || v.retained<v.requests || v.retained>4096u)return GenericReject(v,"retainedReferences_range50to4096_andAtLeastRequests");
    return v;
}
static GenericLimits GenericParse(const char* bytes,U32 size){
    GenericLimits v={150u,192u,2048u,64u,1u,256u,1u,true,"configured"};
    if(size>4096u)return GenericReject(v,"fileTooLarge");
    U32 offset=0,seen=0,section=0;
    if(size>=3u && static_cast<U8>(bytes[0])==0xEFu && static_cast<U8>(bytes[1])==0xBBu && static_cast<U8>(bytes[2])==0xBFu)offset=3;
    for(U32 i=offset;i<size;++i)if(static_cast<U8>(bytes[i])<32u && bytes[i]!='\r' && bytes[i]!='\n' && bytes[i]!='\t')return GenericReject(v,"invalidEncoding");
    while(offset<size){const U32 start=offset;while(offset<size && bytes[offset]!='\n')++offset;
        const char* first=bytes+start;const char* last=bytes+offset;if(offset<size)++offset;
        for(const char* c=first;c<last;++c)if(*c==';' || *c=='#'){last=c;break;}
        GenericTrim(first,last);if(first==last)continue;
        if(*first=='['){if(last[-1]!=']')return GenericReject(v,"invalidSection");++first;--last;GenericTrim(first,last);
            section=GenericEqual(first,last,"Limits")?1u:GenericEqual(first,last,"Traffic")?2u:0u;
            if(!section)return GenericReject(v,"unknownSection");continue;}
        if(!section)return GenericReject(v,"settingOutsideSection");
        const char* equal=first;while(equal<last && *equal!='=')++equal;
        if(equal==last)return GenericReject(v,"missingEquals");
        const char* keyEnd=equal;GenericTrim(first,keyEnd);U32 key=0;U32* destination=nullptr;
        if(section==1u){
            if(GenericEqual(first,keyEnd,"RequestCapacity")){key=1u;destination=&v.requests;}
            else if(GenericEqual(first,keyEnd,"AudioModelEntries")){key=2u;destination=&v.audioModels;}
            else if(GenericEqual(first,keyEnd,"GlobalAudioEntries")){key=3u;destination=&v.audioSlots;}
            else if(GenericEqual(first,keyEnd,"DefaultAudioMiB")){key=4u;destination=&v.audioMiB;}
            else if(GenericEqual(first,keyEnd,"ReplayReaderMiB")){key=5u;destination=&v.replayMiB;}
            else if(GenericEqual(first,keyEnd,"ReplayRetainedReferences")){key=6u;destination=&v.retained;}
        }else if(GenericEqual(first,keyEnd,"RosterSource"))key=7u;
        if(!key)return GenericReject(v,"unknownKey");
        if(seen&(1u<<key))return GenericReject(v,"duplicateKey");seen|=1u<<key;
        first=equal+1;GenericTrim(first,last);
        if(key==7u){if(GenericEqual(first,last,"Native"))v.assetWeights=0;
            else if(GenericEqual(first,last,"AssetWeights"))v.assetWeights=1;
            else return GenericReject(v,"unknownRosterSource");}
        else if(!GenericNumber(first,last,*destination))return GenericReject(v,"invalidInteger");
    }
    return GenericValidate(v);
}
static U32 GenericRequestStorage(){return g_limits.requests<50u?50u:g_limits.requests;}
static U32 GenericParkStride(){return GenericRequestStorage()*4u+4u;}
static U32 GenericParkFrame(){return GenericParkStride()*25u+632u;}
static U32 GenericNotifyFrame(){return GenericRequestStorage()*4u+8u;}
static U32 GenericReplayBytes(){return (g_limits.requests*4u+g_limits.retained*8u+12u);}
#endif

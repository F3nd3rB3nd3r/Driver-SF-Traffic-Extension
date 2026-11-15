// User capacities. Bounds below come from signed native arithmetic, layouts,
// alignment and identifier formats, never from the largest live-tested roster.
struct GenericLimits {
    U32 requests,audioModels,audioSlots,audioMiB,replayMiB,retained,assetWeights;
    bool valid;const char* reason;
    U32 models,objects,definitions,pending,callbacks,cleanup,collection,indexed,hashExtra;
};
static GenericLimits g_limits={150,192,2048,64,1,256,1,true,"missingFile",160,512,512,512,512,262144,131072,262144,45000};
static bool GenericEqual(const char* p,const char* e,const char* s){
    while(p<e && *s){char a=*p++,b=*s++;if(a>='A' && a<='Z')a+=32;if(b>='A' && b<='Z')b+=32;if(a!=b)return false;}return p==e && !*s;
}
static void GenericTrim(const char*& p,const char*& e){while(p<e && (*p==' ' || *p=='\t' || *p=='\r'))++p;while(e>p && (e[-1]==' ' || e[-1]=='\t' || e[-1]=='\r'))--e;}
static bool GenericNumber(const char* p,const char* e,U32& n){n=0;if(p==e)return false;while(p<e){if(*p<'0' || *p>'9')return false;U32 d=*p++-'0';if(n>(0xFFFFFFFFu-d)/10u)return false;n=n*10u+d;}return true;}
static GenericLimits GenericReject(GenericLimits v,const char* why){v.valid=false;v.reason=why;return v;}
static U32 WideMax(U32 a,U32 b){return a>b?a:b;}
static bool WideSize(U32 n,U32 stride,U32 header=0u){return stride && header<=0x7FFFFFFFu && n<=(0x7FFFFFFFu-header)/stride;}
static U32 WideBudget(U32 n,U32 unit){
    const U32 q=n/35u,r=n%35u;if(!WideSize(q,unit,1023u+(r*unit+34u)/35u))return 0;
    return (q*unit+(r*unit+34u)/35u+1023u)&~1023u;
}
static U32 WideWorkNodes(){return WideMax(512u,g_limits.models+g_limits.retained);}
static U32 WideCloneNodes(){return WideMax(256u,g_limits.requests+g_limits.retained);}
static U32 WideSidecarNodes(){return WideMax(256u,g_limits.models+4u);}
static U32 WideClass16Nodes(){return WideMax(192u,g_limits.models+32u);}
static GenericLimits GenericValidate(GenericLimits v){
    if(!v.requests || !WideSize(v.requests,112u,65536u))return GenericReject(v,"requestLayoutOverflow");
    if(!v.models)v.models=WideMax(160u,v.requests+10u);
    if(!v.objects)v.objects=WideMax(512u,v.models+4u);
    if(!v.definitions)v.definitions=WideMax(512u,v.models);
    if(!v.pending)v.pending=WideMax(512u,v.models+4u);
    if(!v.callbacks)v.callbacks=WideMax(512u,v.models+4u);
    if(!v.cleanup)v.cleanup=262144u;if(!v.collection)v.collection=131072u;
    if(!v.indexed)v.indexed=262144u;if(!v.hashExtra)v.hashExtra=45000u;
    if(v.models<v.requests || !WideSize(v.models,256u,65536u) || !WideBudget(v.models,8192u) || !WideBudget(v.models,65536u))return GenericReject(v,"modelLayoutOrGraphicsBudget");
    if(v.audioModels<45u || v.audioModels<v.requests || !WideSize(v.audioModels,256u,65536u))return GenericReject(v,"audioModelLayoutOrRequestDependency");
    if(v.audioSlots<255u || !WideSize(v.audioSlots,4u,65536u))return GenericReject(v,"globalAudioSignedVectorLayout");
    if(v.audioMiB<16u || !WideSize(v.audioMiB,1048576u,65536u))return GenericReject(v,"defaultAudioByteOverflow");
    if(!v.replayMiB || (v.replayMiB&(v.replayMiB-1u)) || !WideSize(v.replayMiB,1048576u,0x20000u))return GenericReject(v,"readerGeometryOrByteOverflow");
    if(v.retained<50u || v.retained<v.requests || !WideSize(v.retained,24u,65536u+v.requests*12u+v.models*12u))return GenericReject(v,"retainedLayoutOrRequestDependency");
#if defined(DSF_V29S_TRAFFIC_QUEUES)
    if(!WideSize(WideMax(512u,v.models+v.retained),20u,64u))return GenericReject(v,"trafficQueueLayout");
#if defined(DSF_V29S_STREAM_HANDLERS)
    if(!WideSize(WideMax(512u,v.models+v.retained),184u,560u))return GenericReject(v,"streamHandlerSchedulerLayout");
#endif
#endif
    if(v.objects<512u || !WideSize(v.objects,12u,24u))return GenericReject(v,"objectPoolLayout");
    if(v.definitions<45u || !WideSize(v.definitions,24u,65536u))return GenericReject(v,"definitionMapLayout");
    if(v.pending<64u || !WideSize(v.pending,16u,65536u))return GenericReject(v,"pendingMapLayout");
    if(v.callbacks<64u || !WideSize(v.callbacks,12u*21u,65536u))return GenericReject(v,"callbackGroupLayout");
    if(!v.cleanup || !WideSize(v.cleanup,4u,65536u))return GenericReject(v,"cleanupQueueLayout");
    if(!v.collection || !WideSize(v.collection,12u,0x27AC8u))return GenericReject(v,"collectionLayout");
    if(v.indexed<65536u || !WideSize(v.indexed,8u,65536u))return GenericReject(v,"indexedHandleLayout");
    if(!WideSize(v.hashExtra,24u,65536u))return GenericReject(v,"hashArenaLayout");
    return v;
}
static GenericLimits GenericParse(const char* bytes,U32 size){
    GenericLimits v={150,192,2048,64,1,256,1,true,"configured",0,0,0,0,0,0,0,0,0};
    if(size>4096u)return GenericReject(v,"fileTooLarge");U32 at=0,section=0,seen=0;
    if(size>=3u && static_cast<U8>(bytes[0])==0xEFu && static_cast<U8>(bytes[1])==0xBBu && static_cast<U8>(bytes[2])==0xBFu)at=3;
    for(U32 i=at;i<size;++i)if(static_cast<U8>(bytes[i])<32u && bytes[i]!='\n' && bytes[i]!='\r' && bytes[i]!='\t')return GenericReject(v,"invalidEncoding");
    while(at<size){U32 start=at;while(at<size && bytes[at]!='\n')++at;const char* p=bytes+start;const char* e=bytes+at;if(at<size)++at;
        for(const char* c=p;c<e;++c)if(*c==';' || *c=='#'){e=c;break;}GenericTrim(p,e);if(p==e)continue;
        if(*p=='['){if(e[-1]!=']')return GenericReject(v,"invalidSection");++p;--e;GenericTrim(p,e);section=GenericEqual(p,e,"Limits")?1u:GenericEqual(p,e,"Traffic")?2u:0u;if(!section)return GenericReject(v,"unknownSection");continue;}
        if(!section)return GenericReject(v,"settingOutsideSection");const char* eq=p;while(eq<e && *eq!='=')++eq;if(eq==e)return GenericReject(v,"missingEquals");
        const char* ke=eq;GenericTrim(p,ke);U32 key=0;U32* dest=nullptr;
        const char* names[]={"RequestCapacity","AudioModelEntries","GlobalAudioEntries","DefaultAudioMiB","ReplayReaderMiB","ReplayRetainedReferences","ModelSlots","ObjectEntries","DefinitionEntries","PendingEntries","CallbackEntriesPerGroup","CleanupEntries","ResourceCollectionEntries","IndexedReferenceEntries","HashExtraEntries"};
        U32* fields[]={&v.requests,&v.audioModels,&v.audioSlots,&v.audioMiB,&v.replayMiB,&v.retained,&v.models,&v.objects,&v.definitions,&v.pending,&v.callbacks,&v.cleanup,&v.collection,&v.indexed,&v.hashExtra};
        if(section==1u){for(U32 i=0;i<15u;++i)if(GenericEqual(p,ke,names[i])){key=i+1u;dest=fields[i];break;}}
        else if(GenericEqual(p,ke,"RosterSource"))key=16u;
        if(!key)return GenericReject(v,"unknownKey");if(seen&(1u<<key))return GenericReject(v,"duplicateKey");seen|=1u<<key;p=eq+1;GenericTrim(p,e);
        if(key==16u){if(GenericEqual(p,e,"Native"))v.assetWeights=0;else if(GenericEqual(p,e,"AssetWeights"))v.assetWeights=1;else return GenericReject(v,"unknownRosterSource");}
        else if(!GenericNumber(p,e,*dest) || !*dest)return GenericReject(v,"invalidInteger");
    }
    return GenericValidate(v);
}
static U32 GenericRequestStorage(){return WideMax(50u,g_limits.requests);}
static U32 GenericParkStride(){return GenericRequestStorage()*4u+4u;}
static U32 GenericParkFrame(){return GenericParkStride()*25u+632u;}
static U32 GenericNotifyFrame(){return GenericRequestStorage()*4u+8u;}
static U32 GenericReplayBytes(){return g_limits.requests*4u+g_limits.retained*8u+12u;}

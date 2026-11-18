// Production parser/output policy in a fixture; never starts or attaches to game.
#include "WideLimitsTest.cpp"
static void ResetLogging(U32 level){
    memset(g_loggingStreams,0,sizeof(g_loggingStreams));g_loggingReadySeq=0;g_loggingRosterSeq[0]=g_loggingRosterSeq[1]=0;
    g_loggingLock=0;g_loggingAttempted=g_loggingAccepted=g_loggingSuppressed=0;g_diagnosticLevel=level;
}
static U32 Fingerprint(const GenericLimits& v){
    const U32 fields[]={v.requests,v.models,v.objects,v.definitions,v.pending,v.callbacks,v.cleanup,v.collection,v.indexed,v.hashExtra,v.audioModels,v.audioSlots,v.audioMiB,v.replayMiB,v.retained,v.assetWeights};
    U32 h=2166136261u;for(U32 n:fields)h=(h^n)*16777619u;return h;
}
static GenericLimits Parse(const char* s){return GenericParse(s,DiagLength(s));}
static void Parser(){
    CHECK(g_diagnosticLevel==1u);
    const char* configs[]={"[Diagnostics]\nLogLevel=0\n","[Diagnostics]\nLogLevel=1\n","[Diagnostics]\nLogLevel=2\n"};
    U32 defaults=Fingerprint(Parse(""));
    for(U32 i=0;i<3;++i){GenericLimits v=Parse(configs[i]);CHECK(v.valid && g_diagnosticLevel==i && Fingerprint(v)==defaults);}
    CHECK(Parse("[diagnostics]\nloglevel=0\n").valid && g_diagnosticLevel==0u);
    CHECK(Parse("").valid && g_diagnosticLevel==1u);
    const char* bad[]={"[Diagnostics]\nLogLevel=3\n","[Diagnostics]\nLogLevel=-1\n","[Diagnostics]\nLogLevel=full\n",
        "[Diagnostics]\nLogLevel=4294967296\n","[Diagnostics]\nLogLevel=\n","[Diagnostics]\nLogLevel=1\nLogLevel=2\n",
        "[Limits]\nLogLevel=0\n","[Traffic]\nLogLevel=0\n","[Diagnostics]\nRosterSource=Native\n","[Diagnostics]\nRequestCapacity=320\n",
        "[Diagnostics]\nLogLevel=0\n[Limits]\nAudioModelEntries=44\n"};
    for(const char* s:bad){CHECK(Parse(configs[2]).valid);CHECK(!Parse(s).valid && g_diagnosticLevel==2u);}
    const char* paths[]={"config/DriverSF_Unlimiter.ini","config/profiles/Expanded280.ini","config/profiles/Headroom320.ini"};
    const U32 req[]={150,280,320},models[]={160,290,330},work[]={512,802,1098};
    for(U32 i=0;i<3;++i){static char bytes[4097]={};U32 read=0;
        HANDLE f=CreateFileA(paths[i],0x80000000u,1u,nullptr,3u,0u,nullptr);CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
        CHECK(ReadFile(f,bytes,sizeof(bytes),&read,nullptr));CloseHandle(f);GenericLimits v=GenericParse(bytes,read);
        CHECK(v.valid && v.requests==req[i] && v.models==models[i] && WideMax(512u,v.models+v.retained)==work[i] && g_diagnosticLevel==1u);
    }
}
static void Policy(){
    const char* essential[]={"DSFV29s_USERCAP_FATAL detail=allocation\n","DSFV29s_LEGACY_EXIT_FATAL reason=hashCursor\n",
        "DSFV29s_STREAM_HANDLER_FATAL reason=full\n","DSFV29s_MANAGER_KEY_FATAL reason=invalid\n","DSFV29s_TRAFFIC_QUEUE_FATAL reason=full\n",
        "DSFV29s_GENERIC_START valid=0 reason=invalidInteger\n","DSFV29s_PENDING_START status=-1\n","DSFV29s_CAP_BOUNDARY activeBefore=50\n",
        "DSFV29s_SOMETHING_STATUS status=-99\n","DSFV29s_CAP_STATUS startupStatus=-6\n","DSFV29s_PENDING_STATUS full=1\n",
        "DSFV29s_MANAGER_KEY_STATUS guardBad=1\n","DSFV29s_REPLAY_READ_STATUS allocationFailed=1\n","DSFV29s_REPLAY_CLIENT_STATUS readFailures=1\n",
        "DSFV29s_REPLAY_READ_STATUS fallbacks=1\n","DSFV29s_REPLAY_READ_STATUS unmanagedCalls=1\n",
        "DSFV29s_BASELINE_STATUS eventDrops=1\n","DSFV29s_AUDIO_SETUP_STATUS detailDropped=1\n","DSFV29s_CAP_STATUS fileStatus=2\n",
        "DSFV29s_AUDIO_SETUP_DETAIL reason=defaultProfileMissing model=191\n","DSFV29s_AUDIO_SETUP_DETAIL reason=vehicleState model=191\n",
        "DSFV29s_AUDIO_SETUP_STATUS previewSuppressed=48 defaultProfileMissing=1\n","DSFV29s_TEST_EXCEPTION code=1\n"};
    const char* detail="DSFV29s_BASELINE_MODEL model=191 ready=1\n";
    for(U32 level=0;level<2;++level){ResetLogging(level);for(const char* s:essential)CHECK(LoggingAllow(s,1000u));
        CHECK(!LoggingAllow(detail,1000u));CHECK(!LoggingAllow("DSFV29s_AUDIO_SETUP_DETAIL reason=previewSuppressed model=191\n",1000u));}
    ResetLogging(1u);const char* status="DSFV29s_AUDIO_MODEL_STATUS tick=0 invalid=0 keyOverflow=0 setupFail=48\n";
    CHECK(LoggingAllow(status,0));CHECK(!LoggingAllow(status,1));CHECK(!LoggingAllow(status,29999));CHECK(LoggingAllow(status,30000));
    CHECK(LoggingAllow("DSFV29s_AUDIO_SETUP_STATUS previewSuppressed=48 modeInitializer=0\n",1));
    CHECK(!LoggingAllow("DSFV29s_BASELINE_MODEL ready=0 invalidModelId=12\n",1));
    CHECK(LoggingAllow("DSFV29s_BASELINE_PHASE kind=accepted seq=1 count=280 ready=0 checkpoints=0\n",100));
    CHECK(!LoggingAllow("DSFV29s_BASELINE_PHASE kind=sample seq=1 count=280 ready=200 checkpoints=1\n",101));
    CHECK(LoggingAllow("DSFV29s_BASELINE_PHASE kind=sample seq=1 count=280 ready=280 checkpoints=2\n",102));
    CHECK(!LoggingAllow("DSFV29s_BASELINE_PHASE kind=sample seq=1 count=280 ready=280 checkpoints=3\n",103));
    CHECK(LoggingAllow("DSFV29s_BASELINE_PHASE kind=sample seq=2 count=18 ready=18 checkpoints=1\n",104));
    CHECK(LoggingAllow("DSFV29s_BASELINE_ROSTER seq=1 kind=original ids=1,2,3\n",100));
    CHECK(LoggingAllow("DSFV29s_BASELINE_ROSTER seq=1 kind=effective ids=1,2,3\n",100));
    CHECK(!LoggingAllow("DSFV29s_BASELINE_ROSTER seq=1 kind=effective ids=1,2,3\n",101));
    CHECK(LoggingAllow("DSFV29s_BASELINE_ROSTER seq=2 kind=effective ids=1,2,3\n",102));
    CHECK(LoggingAllow("DSFV29s_AUDIO_SLOTS list=A0 invalid=0\n",100));CHECK(LoggingAllow("DSFV29s_AUDIO_SLOTS list=A1 invalid=0\n",100));
    ResetLogging(1u);CHECK(LoggingAllow(status,0xFFFFFFF0u));CHECK(!LoggingAllow(status,0x10u));CHECK(LoggingAllow(status,29984u));
    g_loggingLock=1;CHECK(LoggingAllow(status,29985u));g_loggingLock=0;
    ResetLogging(2u);for(U32 i=0;i<100;++i)CHECK(LoggingAllow(detail,i));
    CHECK(g_loggingAttempted==100 && g_loggingAccepted==100 && g_loggingSuppressed==0);
    for(const char* s:essential)CHECK(LoggingAllow(s,0));
    ResetLogging(0);CHECK(!LoggingAllow(status,1000));CHECK(LoggingAllow("DSFV29s_LOGGING_START level=0 fullTrace=0\n",0));
}
static void Backend(){
    ResetLogging(0);const char* file="analysis/logging_r10_2026-11-18/backend_capture.log";
    HANDLE f=CreateFileA(file,0xC0000000u,3u,nullptr,2u,0u,nullptr);CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    g_capacityFile=f;g_capacityFileStatus=0;
    CapacityWrite("DSFV29s_BASELINE_MODEL hidden=1\n");const char* essential="DSFV29s_USERCAP_FATAL reason=fixtureOnly\n";CapacityWrite(essential);
    CHECK(GetFileSize(f,nullptr)==DiagLength(essential) && g_capacityFileStatus==0u);CloseHandle(f);g_capacityFile=nullptr;
    f=CreateFileA(file,0x80000000u,1u,nullptr,3u,0u,nullptr);CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    char bytes[256]={};U32 read=0;CHECK(ReadFile(f,bytes,sizeof(bytes),&read,nullptr));CloseHandle(f);
    CHECK(read==DiagLength(essential) && LoggingEqual(bytes,bytes+read,essential));
    ResetLogging(1);f=CreateFileA(file,0xC0000000u,3u,nullptr,2u,0u,nullptr);CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    g_capacityFile=f;LoggingStartLine();LoggingPoll();CloseHandle(f);g_capacityFile=nullptr;
    f=CreateFileA(file,0x80000000u,1u,nullptr,3u,0u,nullptr);CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    static char meta[1024]={};CHECK(ReadFile(f,meta,sizeof(meta),&read,nullptr));CloseHandle(f);
    CHECK(LoggingContains(meta,meta+read,"DSFV29s_LOGGING_START") && LoggingContains(meta,meta+read,"DSFV29s_LOGGING_STATUS") && LoggingContains(meta,meta+read,"level=1"));
}
static U8 input[8192]={};static char line[16384]={};
static void ReplayInput(U32 level){
    ResetLogging(level);HANDLE f=CreateFileA("dist/USERCAP_R9/DSFV29s_usercap_r9_7044.log",0x80000000u,1u,nullptr,3u,0u,nullptr);
    CHECK(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    const char* paths[]={"analysis/logging_r10_2026-11-18/filtered_minimal.log","analysis/logging_r10_2026-11-18/filtered_summary.log","analysis/logging_r10_2026-11-18/filtered_full.log"};
    HANDLE out=CreateFileA(paths[level],0x40000000u,1u,nullptr,2u,0u,nullptr);CHECK(out!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
    U32 size=0,length=0,bytes=0,keptBytes=0,lines=0,kept=0,now=0;
    while(ReadFile(f,input,sizeof(input),&size,nullptr) && size){for(U32 i=0;i<size;++i){CHECK(length+1u<sizeof(line));line[length++]=static_cast<char>(input[i]);++bytes;
        if(input[i]=='\n'){line[length]=0;++lines;U32 tick=0;if(LoggingField(line,"tick",tick) && tick>now)now=tick;
            if(LoggingAllow(line,now)){U32 written=0;CHECK(WriteFile(out,line,length,&written,nullptr) && written==length);++kept;keptBytes+=length;}length=0;}}}
    CHECK(size==0u && length==0u);CloseHandle(f);CloseHandle(out);
    Say("PASS filter level=");Number(level);Say(" inputLines=");Number(lines);Say(" keptLines=");Number(kept);Say(" inputBytes=");Number(bytes);Say(" keptBytes=");Number(keptBytes);Say("\n");
    if(level==2u)CHECK(bytes==keptBytes && lines==kept);
}
extern "C" void LoggingTestMain(){
    SetErrorMode(3u);
    if(WideContains(GetCommandLineA()," replay")){const char* cmd=GetCommandLineA();U32 level=2;
        if(WideContains(cmd," minimal"))level=0;if(WideContains(cmd," summary"))level=1;ReplayInput(level);}
    else {Parser();Policy();Backend();Say("PASS parser/policy/backend checks=");Number(g_checks);Say("\n");}
    ExitProcess(0);
}

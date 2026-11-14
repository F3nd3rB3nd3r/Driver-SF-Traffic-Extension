static void RegMapAppend(char*& p,const char* end,const RegMapDetail& d){
    DiagFAppendKeyHex(p," map=",d.map,end);DiagFAppendKeyHex(p," cursor=",d.cursor,end);
    DiagFAppendKeyHex(p," start=",d.start,end);DiagFAppendKeyHex(p," end=",d.end,end);
    DiagFAppendKeyU(p," active=",d.active,end);DiagFAppendKeyU(p," recycled=",d.recycled,end);
    DiagFAppendKeyU(p," generation=",d.generation,end);DiagFAppendKeyU(p," inlineIssued=",d.inlineIssued,end);
    DiagFAppendKeyU(p," externalIssued=",d.externalIssued,end);DiagFAppendKeyU(p," attached=",d.attached,end);
    DiagFAppendKeyU(p," owners=",static_cast<U32>(g_regMapOwners),end);
}
static bool RegMapInstallRefused(const char* reason,UPtr site){
    char line[400]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REGMAP_INSTALL_FAIL reason=",end);AppendText(p,reason,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);DiagFAppendKeyHex(p," site=",site,end);
    AppendText(p," startupCheckRevision=2\n",end);*p=0;CapacityWrite(line);return false;
}
static void RegMapFatal(const RegMapDetail& d){
    char line[900]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REGMAP_FATAL reason=",end);AppendText(p,d.reason?d.reason:"unknown",end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);RegMapAppend(p,end,d);
    AppendText(p," exitCode=00000031\n",end);*p=0;CapacityWrite(line);
    LegacyExit(0x31u,"vehicleDefinitionMap",d.map,d.cursor,d.active,d.recycled);
}
static void RegMapEvent(const char* kind,const RegMapDetail& d){
    char line[800]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REGMAP_EVENT kind=",end);AppendText(p,kind,end);
    DiagFAppendKeyU(p," tick=",GetTickCount()-g_capacityStart,end);RegMapAppend(p,end,d);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}
static void RegMapStartLine(){CapacityWrite(g_sRegMapHook==1?
    "DSFV29s_REGMAP_START revision=1 status=1 hooks=2 insert=0086BE5F constructor=0086BFA0 fixedOwnerLimit=0 inlineNodes=45 externalNodes=512 nodeBytes=24 processLifetime=1 constructorGeneration=1 requests=150 graphics=160 startupCheckRevision=2\n":
    "DSFV29s_REGMAP_START revision=1 status=-1\n");}
static void RegMapPoll(){
    static U32 last=0;const U32 tick=GetTickCount()-g_capacityStart;if(tick-last<1000u)return;last=tick;
    char line[650]={};char* p=line;const char* end=line+sizeof(line)-1;
    AppendText(p,"DSFV29s_REGMAP_STATUS",end);DiagFAppendKeyU(p," tick=",tick,end);AppendText(p," status=",end);AppendNumber(p,g_sRegMapHook,end);
    const char* keys[]={" owners="," constructs="," reconstructs="," checks="," redirects="," reattaches="," recycled="," externalHigh="," faults="," allocationFailed="};
    const long values[]={g_regMapOwners,g_regMapConstructs,g_regMapReconstructs,g_regMapChecks,g_regMapRedirects,g_regMapReattaches,g_regMapRecycled,g_regMapExternalHigh,g_regMapFaults,g_regMapAllocFailed};
    for(U32 i=0;i<10u;++i)DiagFAppendKeyU(p,keys[i],static_cast<U32>(values[i]),end);AppendText(p,"\n",end);*p=0;CapacityWrite(line);
}

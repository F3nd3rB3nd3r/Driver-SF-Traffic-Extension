#include "V29sTrafficPolicyCore.inc"
// Read one bounded local file before publishing hooks. No profile APIs, current
// directory lookup, worker race, config writes or live reload under the loader.
static void TrafficLoadPolicy(HMODULE module) {
    static wchar_t path[1024]={};
    const U32 length=GetModuleFileNameW(module,path,1024u);
    if(!length || length>=1024u){g_trafficPolicy={TrafficNative,"modulePathFailed"};return;}
    U32 directory=length;while(directory && path[directory-1u]!=L'\\' && path[directory-1u]!=L'/')--directory;
    const wchar_t name[]=L"DriverSF_Traffic.ini";
    if(!directory || directory+sizeof(name)/sizeof(name[0])>1024u){g_trafficPolicy={TrafficNative,"modulePathFailed"};return;}
    for(U32 i=0;i<sizeof(name)/sizeof(name[0]);++i)path[directory+i]=name[i];
    const HANDLE file=CreateFileW(path,0x80000000u,1u,nullptr,3u,0x80u,nullptr);
    if(file==reinterpret_cast<HANDLE>(0xFFFFFFFFu)) {
        const U32 error=GetLastError();
        g_trafficPolicy={TrafficNative,error==2u || error==3u?"missingFile":"openFailed"};return;
    }
    static char bytes[4097]={};U32 read=0;
    const BOOL ok=ReadFile(file,bytes,sizeof(bytes),&read,nullptr);CloseHandle(file);
    g_trafficPolicy=ok?TrafficParse(bytes,read):TrafficPolicy{TrafficNative,"readFailed"};
}

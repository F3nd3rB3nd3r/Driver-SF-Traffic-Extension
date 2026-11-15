#include "V29sGenericConfigCore.inc"
static void GenericLoadConfig(HMODULE module){
    static wchar_t path[1024]={};const U32 length=GetModuleFileNameW(module,path,1024u);
    if(!length || length>=1024u){g_limits=GenericReject(g_limits,"modulePathFailed");return;}
    U32 directory=length;while(directory && path[directory-1]!=L'\\' && path[directory-1]!=L'/')--directory;
    const wchar_t name[]=L"DriverSF_Unlimiter.ini";
    if(!directory || directory+sizeof(name)/2u>1024u){g_limits=GenericReject(g_limits,"modulePathFailed");return;}
    for(U32 i=0;i<sizeof(name)/2u;++i)path[directory+i]=name[i];
    HANDLE file=CreateFileW(path,0x80000000u,1u,nullptr,3u,0x80u,nullptr);
    if(file==reinterpret_cast<HANDLE>(0xFFFFFFFFu)){const U32 error=GetLastError();if(error!=2u && error!=3u)g_limits=GenericReject(g_limits,"openFailed");return;}
    static char bytes[4097]={};U32 read=0;const BOOL okay=ReadFile(file,bytes,sizeof(bytes),&read,nullptr);CloseHandle(file);
    g_limits=okay?GenericParse(bytes,read):GenericReject(g_limits,"readFailed");
}

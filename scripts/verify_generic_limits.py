"""Isolated Win32 fixtures and PE checks for the generic150 milestone; no game."""
import argparse, hashlib, json, re, struct, subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from verify_replay_read1m import sdk_lib
from verify_traffic_policy import function

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/generic_limits_r1_2026-11-14'
BUILD=ROOT/'build/generic_limits_fixtures'
ASI=ROOT/'build/v29s_generic_limits_r1/DriverSF_Unlimiter_GENERIC_R1.asi'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def compile_run(label,source,entry):
    dependencies={}
    def visit(path):
        path=path.resolve()
        if str(path) in dependencies:return
        dependencies[str(path)]=digest(path)
        for name in re.findall(r'^\s*#include "([^\"]+)"',path.read_text(),re.M):
            choices=[Path(name),path.parent/name,BUILD/name,ROOT/'tests'/name]
            target=next((p for p in choices if p.is_file()),None)
            assert target is not None,(path,name)
            visit(target)
    visit(source)
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
         '/I'+str(BUILD),'/I'+str(ROOT/'tests'),'/Fo'+str(BUILD/(label+'.obj')),str(source),'/link','/NODEFAULTLIB',
         '/ENTRY:'+entry+'@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/(label+'.exe')),str(sdk_lib())]
    fingerprint=hashlib.sha256(json.dumps(dict(command=cmd,dependencies=dependencies),sort_keys=True).encode()).hexdigest()
    cache=OUT/(label+'_cache.json');exe=BUILD/(label+'.exe')
    if cache.exists() and exe.exists():
        previous=json.loads(cache.read_text())
        if previous['fingerprint']==fingerprint and previous['binary_sha256']==digest(exe):
            return dict(name=label,result='passed',cached=True,output=(OUT/(label+'_run.log')).read_text().strip())
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/(label+'_build.log')).write_text(r.stdout+r.stderr)
    assert r.returncode==0,(label,r.stdout,r.stderr)
    r=subprocess.run([str(BUILD/(label+'.exe'))],capture_output=True,text=True,timeout=45)
    (OUT/(label+'_run.log')).write_text(r.stdout+r.stderr);assert r.returncode==0,(label,r.returncode,r.stdout,r.stderr)
    cache.write_text(json.dumps(dict(fingerprint=fingerprint,binary_sha256=digest(exe),dependencies=dependencies),indent=2)+'\n')
    return dict(name=label,result='passed',output=r.stdout.strip())
def generated(source,settings=''):
    t=(ROOT/'tests'/source).read_text()
    t=re.sub(r'#include "\.\./src/([^\"]+)"',lambda m:'#include "'+(ROOT/'src'/m[1]).as_posix()+'"',t)
    first=t.index('\n',t.index('typedef unsigned int UPtr'))+1 if 'typedef unsigned int UPtr' in t else t.index('\n',t.index('typedef unsigned int   UPtr'))+1
    prefix='#define DSF_V29S_GENERIC_LIMITS 1\n#include "'+(ROOT/'src/V29sGenericConfigCore.inc').as_posix()+'"\n'
    t=t[:first]+prefix+t[first:]
    # Fixture-only initialization, before any module storage is constructed.
    if settings:
        t=re.sub(r'(extern "C" void __stdcall \w+Main\(\)\s*\{)',lambda m:m[1]+'\n'+settings+'\n',t,count=1)
    return t
def fixtures():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    main=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    support=(ROOT/'tests/TrafficPolicyTest.cpp').read_text().split('#include "../src/V29sTrafficPolicyConfig.inc"')[0]
    support=support.replace('DriverSF_Traffic.ini','DriverSF_Unlimiter.ini')
    (BUILD/'GenericTestSupport.inc').write_text(support)
    types=re.search(r'#define kReqCapacity.*?static RequestSidecar\* g_reqSidecars=nullptr;',main,re.S)[0]
    (BUILD/'GenericRequestTypes.inc').write_text(types+'\n')
    for name,signature in [('Slot','RequestSidecar* RequestSlot('),('Body','extern "C" void __fastcall RequestWriteBridge('),('Read','extern "C" __declspec(noinline) void __cdecl RequestReadBridge(')]:
        body=function(main,signature).replace('_ReturnAddress()','FixtureCaller()')
        (BUILD/('GenericRequest'+name+'.inc')).write_text(body+'\n')
    results=[compile_run('generic',ROOT/'tests/GenericLimitsTest.cpp','GenericTestMain')]
    t=generated('Parked150Test.cpp');p=BUILD/'parked.cpp';p.write_text(t);results.append(compile_run('parked',p,'ParkedTestMain'))
    # Run the exact relocated builder with a smaller runtime category geometry.
    start=t.index('extern "C" void __stdcall ParkedTestMain(){');stop=t.index('    for(U32 n=18;n<=50;',start)
    smaller=t[:stop]+'''    g_limits.requests=100;
    for(U32 n=0;n<=100;++n){CheckFull(n,false);CheckFull(n,true);}
    Check(!fatals);Say("parked100 runtime geometry passed\\n");ExitProcess(0);
}
'''
    p=BUILD/'parked100.cpp';p.write_text(smaller);results.append(compile_run('parked100',p,'ParkedTestMain'))
    smaller=smaller.replace('g_limits.requests=100','g_limits.requests=18').replace('n<=100','n<=18').replace('parked100','parked18')
    p=BUILD/'parked18.cpp';p.write_text(smaller);results.append(compile_run('parked18',p,'ParkedTestMain'))
    t=generated('Replay150Test.cpp')
    t=t.replace('ReplayData d={};Check(ReplayCollectRetained','ReplayData d={};ReplayStorageInit(d,ReplayAllocate(ReplayStorageBytes()));Check(ReplayCollectRetained')
    old='ReplayTemp* held=ReplayBegin(0x123456u,1u);Check(held);ReplayCapture(reinterpret_cast<void*>(testOwner),nullptr,guarded);Check(fatals==oldFatals+2u);ReplayFinish(held);'
    new='ReplayTemp* held[8]={};const U32 reserved=static_cast<U32>(g_replayTempsAllocated);Check(reserved<=8);for(U32 i=0;i<reserved;++i){held[i]=ReplayBegin(0x123456u+i*16u,1u);Check(held[i]);}ReplayCapture(reinterpret_cast<void*>(testOwner),nullptr,guarded);Check(fatals==oldFatals+2u);for(U32 i=0;i<reserved;++i)ReplayFinish(held[i]);'
    assert old in t;t=t.replace(old,new)
    p=BUILD/'snapshot.cpp';p.write_text(t);results.append(compile_run('snapshot',p,'ReplayTestMain'))
    stop=t.index('    for(U32 n=0;n<=150u;++n)RoundTrip')
    smaller=t[:stop]+'''    for(U32 n=0;n<=100u;++n)RoundTrip(n,400u);
    Check(!fatals && !g_replayActive);Say("snapshot100/retained400 dynamic payload passed\\n");ExitProcess(0);
}
'''
    smaller=smaller.replace('extern "C" void __stdcall ReplayTestMain(){','extern "C" void __stdcall ReplayTestMain(){\ng_limits.requests=100;g_limits.retained=512;')
    smaller=smaller.replace('retainedNodes[257]','retainedNodes[513]').replace('pendingNodes[257]','pendingNodes[513]').replace('wire[3000]','wire[10000]')
    p=BUILD/'snapshot100.cpp';p.write_text(smaller);results.append(compile_run('snapshot100',p,'ReplayTestMain'))
    for mib in (1,4):
        t=generated('ReplayReadTest.cpp',f'g_limits.replayMiB={mib};')
        t=t.replace('1MiB',str(mib)+'MiB')
        mark='#include "'+(ROOT/'src/V29sReplayReadLayout.inc').as_posix()+'"'
        t=t.replace(mark,'static void DiagFAppendKeyU(char*& p,const char* key,U32 n,const char* end){AppendText(p,key,end);AppendNumber(p,n,end);}\n'+mark)
        p=BUILD/('reader'+str(mib)+'.cpp');p.write_text(t);results.append(compile_run('reader'+str(mib),p,'ReadTestMain'))
    t=generated('AudioModelRegistryTest.cpp','g_limits.audioModels=256;')
    values={'191':'255','192':'256','193':'257','147':'211','189':'253'}
    t=re.sub(r'\b(191|192|193|147|189)(u?)\b',lambda m:values[m[1]]+m[2],t)
    p=BUILD/'audio256.cpp';p.write_text(t);results.append(compile_run('audio256',p,'AudioModelTestMain'))
    t=generated('AudioSlotsTest.cpp','g_limits.audioSlots=4096;')
    values={'2047':'4095','2048':'4096','2049':'4097'}
    t=re.sub(r'\b(2047|2048|2049)(u?)\b',lambda m:values[m[1]]+m[2],t)
    p=BUILD/'slots4096.cpp';p.write_text(t);results.append(compile_run('slots4096',p,'AudioSlotsTestMain'))
    t=generated('AudioDefaultPoolTest.cpp','g_limits.audioMiB=128;')
    p=BUILD/'default128.cpp';p.write_text(t);results.append(compile_run('default128',p,'AudioDefaultTestMain'))
    # Actual original/modified data, but only production helpers execute in
    # fixture-owned memory. On-disk asset bytes are never modified.
    audits=[json.loads((OUT/name).read_text()) for name in ('original_asset.json','modified_asset.json')]
    for audit in audits:assert digest(Path(audit['file']))==audit['sha256']
    support=(BUILD/'GenericTestSupport.inc').read_text().split('static U32 ioError=')[0]
    support+='''extern "C" {
__declspec(dllimport) U32 __stdcall GetModuleFileNameW(HMODULE,wchar_t*,U32);
__declspec(dllimport) U32 __stdcall GetLastError();
__declspec(dllimport) HANDLE __stdcall CreateFileW(LPCWSTR,U32,U32,void*,U32,U32,HANDLE);
__declspec(dllimport) BOOL __stdcall ReadFile(HANDLE,void*,U32,U32*,void*);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE);
}
'''
    (BUILD/'GenericAssetSupport.inc').write_text(support)
    t=(ROOT/'tests/GenericLimitsTest.cpp').read_text().split('extern "C" void __stdcall GenericTestMain(){')[0]
    t=t.replace('GenericTestSupport.inc','GenericAssetSupport.inc')
    t=re.sub(r'#include "\.\./src/([^\"]+)"',lambda m:'#include "'+(ROOT/'src'/m[1]).as_posix()+'"',t)
    t+='\nstruct ExpectedProfile {U32 asset,hash,frequency,count,ids[150];};\nstatic const ExpectedProfile expected[]={\n'
    for number,audit in enumerate(audits):
        t+='// Asset SHA256 '+audit['sha256']+'\n'
        for profile in audit['profiles']:
            ids=profile['native'] if not profile['missing_from_native'] else sorted(set(profile['native'])|set(profile['missing_from_native']))
            t+='{'+f'{number},0x{profile["hash"]}u,{profile["frequency"]},{len(ids)},'+'{'+','.join(map(str,ids))+'}},\n'
    t+='};\nstatic const wchar_t* assetPaths[]={'+','.join('L"'+Path(a['file']).as_posix()+'"' for a in audits)+'};\n'
    t+='''static UPtr LoadFixtureAsset(LPCWSTR path){HANDLE f=CreateFileW(path,0x80000000u,1u,nullptr,3u,0x80u,nullptr);Check(f!=reinterpret_cast<HANDLE>(0xFFFFFFFFu));
UPtr data=Allocate(4403200);U32 read=0;Check(ReadFile(f,reinterpret_cast<void*>(data),4403200,&read,nullptr) && read==4403200);CloseHandle(f);
UPtr value=data+0x1080;const U32 fields[]={0xEC,0x11C,0x124,0x12C,0x15C,0x164};for(U32 field:fields)Write32(value+field,value+Read32(value+field));return value;}
extern "C" void __stdcall GenericAssetMain(){Configure(150);g_base=Allocate(0xC00000);memcpy(reinterpret_cast<void*>(Address(0x76FDF0)),kGenericSetterBytes,sizeof(kGenericSetterBytes));Check(ChapterPreflight());
root=Allocate(0x1000);context=Allocate(0x5CB000);Write32(Address(kRootPtr),root);Write32(root+0x758,context);caller=Address(0x76FE9A);UPtr owner=Owner();U32 checked=0;
for(U32 number=0;number<2;++number){ai=LoadFixtureAsset(assetPaths[number]);Write32(Address(0xFE7AC0),ai);settings=Read32(ai+0x11C);lists=Read32(ai+0x124);
for(const auto& p:expected)if(p.asset==number){U32 record=0;for(U32 i=0;i<Read32(ai+0x118);++i)if(Read32(settings+i*28)==p.hash)record=settings+i*28;Check(record);
U32 n=Read16(record+14+p.frequency*2),start=Read16(record+4+p.frequency*2);U32 ids[150]={};Check(n<=150);for(U32 i=0;i<n;++i)ids[i]=Read16(lists+(start+i)*2);
Write32(context+0x5CA840,p.hash);Write32(context+0x5CA844,p.frequency);Submit(owner,ids,n);Expect(owner,p.ids,p.count);++checked;}}
Check(checked==160 && !fatalCount && !DSFV27B_RequestRejected);const char result[]="all160 original/modified asset profiles match production generic rosters\\n";U32 written=0;WriteFile(GetStdHandle(static_cast<U32>(-11)),result,sizeof(result)-1,&written,nullptr);ExitProcess(0);}
'''
    p=BUILD/'assets.cpp';p.write_text(t);results.append(compile_run('assets',p,'GenericAssetMain'))
    result=dict(result='passed',fixtures=results,scope='Isolated native slices/production helpers with fixture services; not gameplay')
    (OUT/'fixtures.json').write_text(json.dumps(result,indent=2)+'\n');return result

def source_hashes():
    found={}
    def visit(path):
        path=path.resolve()
        if str(path.relative_to(ROOT)) in found:return
        found[str(path.relative_to(ROOT))]=digest(path)
        for include in re.findall(r'^\s*#include "([^\"]+)"',path.read_text(),re.M):visit(path.parent/include)
    visit(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp')
    for path in [ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj',Path(__file__),ROOT/'scripts/analyze_generic_limits_log.py',ROOT/'dist/MAIN_150/DriverSF_Unlimiter.ini']:
        found[str(path.relative_to(ROOT))]=digest(path)
    tested=json.loads((OUT/'fixtures.json').read_text())
    for result in tested['fixtures']:
        cache=json.loads((OUT/(result['name']+'_cache.json')).read_text())
        for filename in cache['dependencies']:
            p=Path(filename);found[str(p.relative_to(ROOT))]=digest(p)
    return found
def verify_build():
    tested=json.loads((OUT/'fixtures.json').read_text());assert tested['result']=='passed'
    for result in tested['fixtures']:
        cache=json.loads((OUT/(result['name']+'_cache.json')).read_text())
        assert all(digest(Path(name))==value for name,value in cache['dependencies'].items()),('stale fixture',result['name'])
    pe=PE(ASI);old_path=ROOT/'build/v29s_traffic_policy_r1/DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1.asi';old=PE(old_path);native=PE()
    assert digest(old_path)=='bb7411bad4e1532c11789ba6966ec01e27e197ddd6a68b870895526bdc292a2d'
    assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    assert struct.unpack_from('<H',pe.data,pe.pe+4)[0]==0x14C and pe.characteristics&0x2000
    def symbols(path):return {m[1]:int(m[2],16) for line in path.read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    before,after=symbols(old_path.with_suffix('.map')),symbols(ASI.with_suffix('.map'))
    def at(table,prefix):return next(value for name,value in table.items() if name.startswith(prefix))
    def resolve(image,address):
        for _ in range(6):
            b=image.read(address,5)
            if b[0]!=0xE9:return address
            address+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('thunk chain')
    def same_target(a,b):
        old_names={k for k,v in before.items() if v==a};new_names={k for k,v in after.items() if v==b}
        if not old_names:old_names={k for k,v in before.items() if v==resolve(old,a)}
        if not new_names:new_names={k for k,v in after.items() if v==resolve(pe,b)}
        assert old_names&new_names,(hex(a),hex(b))
    assert not any(any(token in name for token in ('kChapter7','kChapterProfiles','kChapterRowIds','kChapterRowWeights')) for name in after)
    assert b'Preset150' not in pe.data and b'canonicalListAndUniform150' not in pe.data
    defaults=struct.unpack('<7I',pe.read(at(after,'?g_limits@'),28));assert defaults==(150,192,2048,64,1,256,1)
    assert pe.read(at(after,'?g_limits@')+28,1)==b'\x01'
    assert pe.read(at(after,'?kGenericSetterBytes@'),198)==native.read(0x76FDF0,198)
    imports=subprocess.run([str(DUMPBIN),'/nologo','/imports',str(ASI)],capture_output=True,text=True,check=True).stdout
    (OUT/'imports.txt').write_text(imports);dlls={s.lower() for s in re.findall(r'^\s+(\S+\.dll)\s*$',imports,re.M|re.I)}
    assert dlls=={'kernel32.dll','psapi.dll'}
    for prefix,label in [('@RequestWriteBridge@16','request'),('_DllMain@12','entry'),('?ChapterSelectRequest@','asset_roster'),('@ReplayRestore@','replay_restore')]:
        start=at(after,prefix);end=min(v for v in after.values() if v>start)
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{end-1:X}',str(ASI)],capture_output=True,text=True,check=True).stdout
        (OUT/(label+'.asm.txt')).write_text(asm)
        if label=='request':assert re.search(r'\bret\s+8\b',asm,re.I)
        if label=='entry':assert re.search(r'\bret\s+0Ch\b',asm,re.I)
    tables={}
    for prefix,count,stride in [('?kAudioModelSites@',8,24),('?kAudioSlotsSites@',9,24),('?kPendingSites@',2,20),('?kCallbackSites@',3,20),('?kCleanupSites@',6,24)]:
        a,b=at(before,prefix),at(after,prefix)
        for i in range(count):
            x,y=old.read(a+i*stride,stride),pe.read(b+i*stride,stride);assert x[:-4]==y[:-4]
            site,n=struct.unpack_from('<2I',y);assert y[8:8+n]==native.read(site,n)
            same_target(struct.unpack_from('<I',x,stride-4)[0],struct.unpack_from('<I',y,stride-4)[0])
        tables[prefix]=count
    for prefix,count,stride in [('?kRFPatches@',8,20),('?kSCPatches@',3,20),('?kReplayPatches@',7,24),('?g_regMapPatches@',2,32)]:
        a,b=at(before,prefix),at(after,prefix)
        for i in range(count):
            x,y=old.read(a+i*stride,stride),pe.read(b+i*stride,stride);assert x[:12]==y[:12]
            site,n,size,stock,target=struct.unpack_from('<5I',y);old_stock,old_target=struct.unpack_from('<2I',x,12)
            assert pe.read(stock,size)==old.read(old_stock,size)==native.read(site,size);same_target(old_target,target)
        tables[prefix]=count
    a,b=at(before,'?g_sites@'),at(after,'?g_sites@')
    for i in range(31):
        x,y=old.read(a+i*40,40),pe.read(b+i*40,40);site,stock,n,target,resume,tramp=struct.unpack_from('<6I',y)
        old_site,old_stock,old_n,old_target,old_resume,old_tramp=struct.unpack_from('<6I',x)
        assert (site,n)==(old_site,old_n) and pe.read(stock,n)==old.read(old_stock,n)==native.read(site,n)
        for v,w in ((old_target,target),(old_resume,resume),(old_tramp,tramp)):
            if v or w:same_target(v,w)
    tables['?g_sites@']=31
    for prefix,size in [('?g_requestFramePatches@',13*24),('?g_poolPatches@',18*24),('?g_indexedPatches@',13*16)]:
        assert old.read(at(before,prefix),size)==pe.read(at(after,prefix),size)
    for token in (b'DSFV29s_GENERIC_START',b'DSFV29s_GENERIC_EVENT',b'DSFV29s_generic_limits_',b'compiledCarLists=0',b'auditedRequestCeiling=150'):
        assert token in pe.data
    result=dict(result='passed',name=ASI.name,bytes=len(pe.data),sha256=digest(ASI),defaults=list(defaults),
        request_abi='fastcall RET8',imports=sorted(dlls),compiled_car_lists=False,retained_hook_tables=tables,
        source_sha256=source_hashes(),fixtures=tested,game_run=False,installed=False)
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');return result
def package():
    r=json.loads((OUT/'verification.json').read_text());assert r['result']=='passed' and r['source_sha256']==source_hashes() and digest(ASI)==r['sha256']
    dest=ROOT/'dist/MAIN_150'/ASI.name
    if dest.exists() and dest.read_bytes()!=ASI.read_bytes():
        previous=json.loads((OUT/'package.json').read_text())
        assert previous['path']==str(dest.relative_to(ROOT)) and digest(dest)==previous['sha256'],'refuse unrelated overwrite'
        saved=OUT/'preliminary_stage'/(previous['sha256']+'.asi');saved.parent.mkdir(parents=True,exist_ok=True)
        assert not saved.exists() or saved.read_bytes()==dest.read_bytes()
        saved.write_bytes(dest.read_bytes());assert digest(saved)==previous['sha256']
        saved.with_suffix('.json').write_text(json.dumps(dict(original_path=previous['path'],bytes=previous['bytes'],sha256=previous['sha256'],reason='Own preliminary build replaced before final delivery'),indent=2)+'\n')
    dest.write_bytes(ASI.read_bytes());assert digest(dest)==r['sha256']
    result=dict(path=str(dest.relative_to(ROOT)),bytes=r['bytes'],sha256=r['sha256'],game_run=False,installed=False)
    (OUT/'package.json').write_text(json.dumps(result,indent=2)+'\n');return result

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--fixtures',action='store_true');parser.add_argument('--verify-build',action='store_true');parser.add_argument('--package',action='store_true');args=parser.parse_args()
    if args.fixtures:print(json.dumps(fixtures(),indent=2))
    elif args.verify_build:print(json.dumps({k:v for k,v in verify_build().items() if k not in ('source_sha256','fixtures')},indent=2))
    elif args.package:print(json.dumps(package(),indent=2))

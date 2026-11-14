"""Verify narrow client probe, inherited R2, and staged artifact; never run Driver.exe."""
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from contextlib import redirect_stdout
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_regmap_reuse as parent
from analyze_replay_client_log import analyze
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/replay_client_r1_2026-11-14';BUILD=ROOT/'build/v29s_replay_client_r1'
NAME='DriverSF_V29s_160Capacity_150Request_REPLAY_CLIENT_R1'
PREFIX=b'DSFV29s_replay_client_'
SPANS=[('Samples',0x7549D0,0x16,5),('Vehicle',0x755DF0,0x58,7),('Player',0x7523B0,9,5)]
SOURCES=['src/V29sReplayClient'+s+'.inc' for s in ('','Core','Data','Native','Bridges','Layout')]+['tests/ReplayClientTest.cpp','src/DriverSF_V29b_NativeFastPath.cpp','src/V29sCapacityTrace.inc','DriverSF_V29s_ARENA_REUSE.vcxproj']
def hashes():return {s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}
def configure_parent():
    parent.OUT=OUT/'parent';parent.BUILD=BUILD;parent.NAME=NAME;parent.PREFIX=PREFIX
    parent.parent.FLAGS=(parent.parent.FLAGS-{'DSF_V29S_REPLAY_HITCH=1'})|{'DSF_V29S_REPLAY_CLIENT=1'}
def parser_fixture():
    sample=OUT/'parser_fixture.log'
    start='DSFV29s_REPLAY_CLIENT_START revision=1 status=1 gateCaller=007379F1 qpcFrequencyLo=10000000 qpcFrequencyHi=0 hooks=3 clients=4 broadHooks=0 boundaryOnly=1 clearFastPath=1 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n'
    status='DSFV29s_REPLAY_CLIENT_STATUS status=1 dropped=0 threadDrops=0 unknown=0 readFailures=0 clockFailures=0\n'
    state=' client=3 sequence=1 thread=7 caller=007379F1 owner=10000000 self=1004E388 recording=20000000 clockBits=40000000 requestedBits=3F800000 phase=0 transition=0 loading=0 curPtr=20000100 curValid=1 nextPtr=00000000 nextValid=0 futurePtr=00000000 recordingValid=1 ringValid=1 capacity=262144 freeBytes=16384 unparsedBytes=0 inFlightBytes=32768 endOfData=0 captureTicks=1000'
    begin='DSFV29s_REPLAY_CLIENT_EVENT kind=busy tick=1000 elapsedTicks=0'+state+'\n'
    end='DSFV29s_REPLAY_CLIENT_EVENT kind=clear tick=1025 elapsedTicks=250000'+state.replace('nextPtr=00000000 nextValid=0','nextPtr=20000200 nextValid=1')+'\n'
    good=start+begin+end+status;sample.write_text(good);r=analyze(sample)
    assert r['structural_pass'] and r['intervals'][0]['duration_ms']==25 and r['intervals'][0]['observed_begin_conditions']==['missing_next'] and not r['root_cause_certified']
    sample.write_text(good.replace('elapsedTicks=250000','elapsedTicks=-1'));assert analyze(sample)['intervals'][0]['duration_ms']==429496.7295
    for bad,key in [(good.replace('broadHooks=0','broadHooks=5'),'startup'),(good.replace('dropped=0','dropped=1'),'telemetry_integrity'),(good.replace('readFailures=0 ',''),'telemetry_integrity'),(start+end+status,'event_pairs'),(good.rstrip('\n'),'records_complete'),(good.replace('client=3','client=9'),'event_schema')]:
        sample.write_text(bad);assert not analyze(sample)['checks'][key],key
    sample.write_text(start+status);r=analyze(sample);assert r['structural_pass'] and not r['busy_starts_by_client'] and not r['root_cause_certified']
    sample.write_text(good);return {'result':'passed','checks':['client/pair/condition/QPC parsing','bad startup, missing counters, drops, incomplete lines and wrong client rejection','no activity/root-cause certification inferred']}
def fixture():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays={n:bytes(int(x,16) for x in b.split(',')) for n,b in re.findall(r'kSC(\w+)Native\[\d+\]=\{(.*?)\};',(ROOT/'src/V29sReplayClientNative.inc').read_text())}
    for n,at,size,length in SPANS:assert arrays[n]==native.read(at,size),n
    assert arrays['Gate']==native.read(0x7379D0,62)
    production=(ROOT/'src/V29sRegMapLayout.inc').read_text();test=(ROOT/'tests/ReplayClientTest.cpp').read_text()
    assert re.search(r'static bool RegMapReadableSpan.*?\n}',production,re.S).group(0) in test
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4','/Fo'+str(BUILD/'ReplayClientTest.obj'),str(ROOT/'tests/ReplayClientTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:ClientTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'ReplayClientTest.exe'),str(lib)]
    r=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'ReplayClientTest.exe')],capture_output=True,text=True,timeout=30);(OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('replay client checks passed\n')
    result={'result':'passed','source_sha256':hashes(),'checks':r.stdout.splitlines(),'parser':parser_fixture()};(OUT/'fixture.json').write_text(json.dumps(result,indent=2)+'\n');print(r.stdout)
    configure_parent();parent.fixture()
def verify():
    tested=json.loads((OUT/'fixture.json').read_text());assert tested['source_sha256']==hashes(),'fixture stale'
    configure_parent();data=(BUILD/(NAME+'.asi')).read_bytes();digest=hashlib.sha256(data).hexdigest()
    path=parent.OUT/'verification.json';cached=json.loads(path.read_text()) if path.exists() else {}
    if cached.get('result')!='passed' or cached.get('sha256')!=digest or cached.get('source_sha256')!=parent.hashes():
        console=io.StringIO()
        with redirect_stdout(console):parent.verify_build()
        (OUT/'parent_console.log').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def resolve(at):
        for _ in range(6):
            b=pe.read(at,5)
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('thunk loop')
    table=symbol('?kSCPatches@');targets=symbol('_g_scTargets')
    for i,(name,at,size,length) in enumerate(SPANS):
        actual,n,total,stock,target=struct.unpack('<5I',pe.read(table+i*20,20));assert (actual,n,total)==(at,length,size) and pe.read(stock,size)==PE().read(at,size)
        prefix='_SCBridge'+name;assert resolve(target)==symbol(prefix)
        start=symbol(prefix);end=min(v for v in symbols.values() if v>start);code=pe.read(start,end-start)
        assert code.startswith(b'\x51\xFF\x15'+struct.pack('<I',targets+i*4))
        assert code.count(b'\xFF\x15'+struct.pack('<I',targets+i*4))==1
        for pattern in ('9c50','60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be561','9d8d642404c3'):assert bytes.fromhex(pattern) in code,(prefix,pattern)
        assert code.count(bytes.fromhex('0fae0424'))==code.count(bytes.fromhex('0fae0c24'))==1
        assert b'\xF0\xFF\x05'+struct.pack('<I',symbol('?g_scFastClear@')) in code
        r=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True);(OUT/(prefix+'.txt')).write_text(r.stdout)
    assert not any(k.startswith('_RHBridge') or 'RHScope' in k for k in symbols)
    assert b'DSFV29s_REPLAY_HITCH_START' not in pe.data and b'DSFV29s_REPLAY_HITCH_EVENT' not in pe.data
    for marker in (b'DSFV29s_REPLAY_CLIENT_START revision=1 status=',b'hooks=3 clients=4 broadHooks=0',b'boundaryOnly=1 clearFastPath=1 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160',b'DSFV29s_REPLAY_CLIENT_EVENT kind=',b'DSFV29s_REPLAY_CLIENT_STATUS',PREFIX):assert marker in pe.data,marker
    core=(ROOT/'src/V29sReplayClientCore.inc').read_text();assert all(s not in core for s in ('Write32(', 'WriteFile(', 'CapacityWrite(', 'VirtualAlloc(', 'Sleep(', 'Readable('))
    for name,digest_old in [('REGMAP_REUSE_R2','8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa'),('REPLAY_HITCH_R1','758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8')]:
        old=ROOT/'dist/MAIN_150'/('DriverSF_V29s_160Capacity_150Request_'+name+'.asi');assert hashlib.sha256(old.read_bytes()).hexdigest()==digest_old
    result=tested|{'name':NAME,'bytes':len(data),'sha256':digest,'inherited_R2_verified':True,'requests':150,'graphics':160,'native_hooks':3,'broad_hooks':0,'installed':False,'game_run':False,'parser':parser_fixture()}
    result['tool_sha256']={s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in ('scripts/verify_replay_client.py','scripts/analyze_replay_client_log.py')}
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k not in ('source_sha256','tool_sha256')},indent=2))
def package():
    result=json.loads((OUT/'verification.json').read_text());assert result['result']=='passed' and result['source_sha256']==hashes()
    assert all(hashlib.sha256((ROOT/s).read_bytes()).hexdigest()==h for s,h in result['tool_sha256'].items())
    data=(BUILD/(NAME+'.asi')).read_bytes();assert hashlib.sha256(data).hexdigest()==result['sha256']
    dest=ROOT/'dist/MAIN_150'/(NAME+'.asi');assert not dest.exists() or dest.read_bytes()==data,'refusing to overwrite another staged artifact'
    dest.write_bytes(data);assert dest.read_bytes()==data
    (OUT/'package.json').write_text(json.dumps({'path':str(dest.relative_to(ROOT)),'bytes':len(data),'sha256':result['sha256'],'installed':False,'game_run':False},indent=2)+'\n');print(str(dest))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--fixture-only',action='store_true');p.add_argument('--verify-build',action='store_true');p.add_argument('--package',action='store_true');a=p.parse_args()
    if a.package:package()
    else:
        if not a.verify_build:fixture()
        if not a.fixture_only:verify()

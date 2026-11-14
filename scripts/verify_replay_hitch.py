"""Execute timing/ABI/installer fixtures, verify inherited R2 and stage the probe."""
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from contextlib import redirect_stdout
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_regmap_reuse as parent
from analyze_replay_hitch_log import analyze
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/replay_hitch_r1_2026-11-14';BUILD=ROOT/'build/v29s_replay_hitch_r1'
NAME='DriverSF_V29s_160Capacity_150Request_REPLAY_HITCH_R1'
SPANS=[('Gate20',0x7379D0,62,11),('Gate24',0x737A10,62,11),('Step',0x738F20,0x214,5),('Seek',0x739F60,0x101,10),('Stream',0x746000,0x99,6)]
SOURCES=['src/V29sReplayHitch'+s+'.inc' for s in ('','Core','Layout','Bridges','Native')]+['tests/ReplayHitchTest.cpp','src/V29sReplay150.inc','src/V29sReplay150Core.inc','src/V29sReplayPause.inc','src/V29sCapacityTrace.inc','src/DriverSF_V29b_NativeFastPath.cpp','DriverSF_V29s_ARENA_REUSE.vcxproj']
def hashes():return {s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}
def configure_parent():
    parent.OUT=OUT/'parent';parent.BUILD=BUILD;parent.NAME=NAME;parent.PREFIX=b'DSFV29s_replay_hitch_'
    parent.parent.FLAGS|={'DSF_V29S_REPLAY_HITCH=1'}
def parser_fixture():
    sample=OUT/'parser_fixture.log'
    start='DSFV29s_REPLAY_HITCH_START revision=1 status=1 qpcFrequencyLo=10000000 qpcFrequencyHi=0 hooks=5 nativeGates=1 duplicateCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n'
    status='DSFV29s_REPLAY_HITCH_STATUS tick=2000 status=1 dropped=0 threadDrops=0 depthDrops=0 mismatches=0 clockFailures=0\n'
    event='DSFV29s_REPLAY_HITCH_EVENT kind=call op=stream duration=100000 tick=1000\n'
    gate='DSFV29s_REPLAY_HITCH_EVENT kind=gateWait op=gate24 duration=500000 tick=1010\n'
    good=start+event+gate+status;sample.write_text(good);r=analyze(sample);assert r['structural_pass'] and r['operations']['stream']['maximum_ms']==10 and r['gate_waits'][0]['duration_ms']==50 and not r['playback_smoothness_certified']
    for bad,key in [(good.replace('duplicateCalls=0','duplicateCalls=1'),'startup'),(good.replace('dropped=0','dropped=1'),'telemetry_integrity'),(good.replace('mismatches=0 ',''),'telemetry_integrity'),(start+event,'status_present'),(good.rstrip('\n'),'records_complete')]:
        sample.write_text(bad);assert not analyze(sample)['checks'][key],key
    sample.write_text(good);return {'result':'passed','checks':['duration units and incomplete/bad/drop/startup rejection','no smoothness certification from structural checks']}
def fixture():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays={n:bytes(int(x,16) for x in b.split(',')) for n,b in re.findall(r'kRH(\w+)Native\[\d+\]=\{(.*?)\};',(ROOT/'src/V29sReplayHitchNative.inc').read_text())}
    for n,at,size,length in SPANS:assert arrays[n]==native.read(at,size),n
    # Use the same page-aware readability predicate exercised by R2.
    text=(ROOT/'src/V29sRegMapLayout.inc').read_text();test=(ROOT/'tests/ReplayHitchTest.cpp').read_text()
    body=re.search(r'static bool RegMapReadableSpan.*?\n}',text,re.S).group(0);assert body in test
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4','/Fo'+str(BUILD/'ReplayHitchTest.obj'),str(ROOT/'tests/ReplayHitchTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:HitchTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'ReplayHitchTest.exe'),str(lib)]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'ReplayHitchTest.exe')],capture_output=True,text=True,timeout=30);(OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('replay hitch checks passed\n')
    result={'result':'passed','checks':r.stdout.splitlines(),'source_sha256':hashes(),'parser':parser_fixture()}
    (OUT/'fixture.json').write_text(json.dumps(result,indent=2)+'\n');print(r.stdout)
    configure_parent();parent.fixture()
def verify():
    result=json.loads((OUT/'fixture.json').read_text());assert result['source_sha256']==hashes(),'fixture stale'
    configure_parent()
    existing=parent.OUT/'verification.json'
    digest=hashlib.sha256((BUILD/(NAME+'.asi')).read_bytes()).hexdigest()
    old=json.loads(existing.read_text()) if existing.exists() else {}
    if old.get('result')!='passed' or old.get('sha256')!=digest or old.get('source_sha256')!=parent.hashes():
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
    table=symbol('?kRHPatches@')
    for i,(name,at,size,length) in enumerate(SPANS):
        actual,n,total,stock,target=struct.unpack('<5I',pe.read(table+i*20,20));assert (actual,n,total)==(at,length,size)
        assert pe.read(stock,size)==PE().read(at,size)
        prefix='_RHBridge'+('20' if i==0 else '24' if i==1 else name)
        assert resolve(target)==symbol(prefix)
        start=symbol(prefix);end=min(v for v in symbols.values() if v>start);code=pe.read(start,end-start)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be5619d'):assert code.count(bytes.fromhex(pattern))==2,(prefix,pattern)
        assert code.rstrip(b'\xCC').endswith(bytes.fromhex('c20800' if i==3 else 'c20400' if i==4 else 'c3'))
        r=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True)
        (OUT/(prefix+'.txt')).write_text(r.stdout)
    for marker in (b'DSFV29s_REPLAY_HITCH_START revision=1 status=',b'duplicateCalls=0 gameThreadFileWrites=0 requests=150 graphics=160',b'DSFV29s_REPLAY_HITCH_EVENT kind=',b'DSFV29s_REPLAY_HITCH_STATUS',b'DSFV29s_replay_hitch_'):assert marker in pe.data,marker
    core=(ROOT/'src/V29sReplayHitchCore.inc').read_text();assert all(x not in core for x in ('Write32(', 'WriteFile(', 'CapacityWrite(', 'VirtualAlloc(', 'Sleep('))
    # Parent adapters only acquire a local observer scope; existing native order remains.
    assert 'RHScope timing(RHUpdate,owner)' in (ROOT/'src/V29sReplayPause.inc').read_text()
    for name in ('RHImport','RHRestore'):assert f'RHScope timing({name},' in (ROOT/'src/V29sReplay150Core.inc').read_text()
    assert 'RHScope timing(RHRequest,owner,0u,count)' in (ROOT/'src/V29sReplay150.inc').read_text()
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.asi';assert hashlib.sha256(old.read_bytes()).hexdigest()=='8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa'
    result['parser']=parser_fixture()
    result['tool_sha256']={s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in ('scripts/verify_replay_hitch.py','scripts/analyze_replay_hitch_log.py')}
    result|={'name':NAME,'bytes':len(pe.data),'sha256':hashlib.sha256(pe.data).hexdigest(),'inherited_R2_verified':True,'requests':150,'graphics':160,'native_entry_hooks':5,'native_calls_duplicated':False,'game_thread_probe_file_writes':False,'installed':False,'game_run':False}
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k!='source_sha256'},indent=2))
def package():
    result=json.loads((OUT/'verification.json').read_text());assert result['result']=='passed' and result['source_sha256']==hashes()
    assert all(hashlib.sha256((ROOT/s).read_bytes()).hexdigest()==digest for s,digest in result['tool_sha256'].items())
    data=(BUILD/(NAME+'.asi')).read_bytes();assert hashlib.sha256(data).hexdigest()==result['sha256']
    dest=ROOT/'dist/MAIN_150'/(NAME+'.asi');assert not dest.exists() or dest.read_bytes()==data,'refusing to overwrite another staged artifact'
    dest.write_bytes(data);assert dest.read_bytes()==data
    (OUT/'package.json').write_text(json.dumps({'path':str(dest.relative_to(ROOT)),'sha256':result['sha256'],'bytes':len(data),'installed':False,'game_run':False},indent=2)+'\n')
    print(str(dest))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--fixture-only',action='store_true');p.add_argument('--verify-build',action='store_true');p.add_argument('--package',action='store_true');a=p.parse_args()
    if a.package:package()
    else:
        if not a.verify_build:fixture()
        if not a.fixture_only:verify()

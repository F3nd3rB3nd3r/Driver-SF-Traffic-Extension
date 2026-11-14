"""Execute native definition-map lifetime fixtures and verify the current ASI."""
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from contextlib import redirect_stdout
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_replay_pause as parent
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/regmap_reuse_r2_2026-11-14';BUILD=ROOT/'build/v29s_regmap_reuse_r2'
NAME='DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2'
PREFIX=b'DSFV29s_regmap_reuse_'
SOURCES=['src/V29sRegMap'+s+'.inc' for s in ('','Core','Layout','Bridges','Native','Runtime')]+['tests/RegMapReuseTest.cpp']
SPANS=[('Ctor',0x86BFA0,0x71),('Insert',0x86BE10,0x74),('Lookup',0x86BDC0,0x49),('Remove',0x86BE90,0x4C),('Allocate',0x8CCAA0,0x65),('Destructor',0x86C040,0x2A)]
def hashes():return {s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}
def verify_parser():
    from analyze_baseline150_log import analyze
    from analyze_callback_log import early_startup
    early=early_startup([{'type':'DSFV29s_INDEXED_START','status':'-8','baselineStatus':'-16'},
        {'type':'DSFV29s_CALLBACK_FATAL','reason':'baselineStartupRefused'}],'startup_fixture')
    assert early['startup_only'] and early['baseline_status']==-16 and not early['recorded_integrity_pass'] and early['audio_setup_failures'] is None
    assert early_startup([{'type':'DSFV29s_CALLBACK_STATUS'},{'type':'DSFV29s_CALLBACK_FATAL'}],'runtime_fixture') is None
    OUT.mkdir(parents=True,exist_ok=True);sample=OUT/'parser_fixture.log'
    start='DSFV29s_REGMAP_START revision=1 status=1 hooks=2 insert=0086BE5F constructor=0086BFA0 fixedOwnerLimit=0 inlineNodes=45 externalNodes=512 nodeBytes=24 processLifetime=1 constructorGeneration=1 requests=150 graphics=160\n'
    constructed='DSFV29s_REGMAP_EVENT kind=constructed generation=2 active=0 recycled=0 inlineIssued=0 externalIssued=0 attached=0 owners=5\n'
    redirect='DSFV29s_REGMAP_EVENT kind=redirect generation=2 active=45 recycled=0 inlineIssued=45 externalIssued=1 attached=1 owners=5\n'
    status='DSFV29s_REGMAP_STATUS status=1 owners=5 constructs=6 reconstructs=1 redirects=6 reattaches=1 externalHigh=105 faults=0 allocationFailed=0\n'
    good=start+constructed+redirect+status;sample.write_text(good);r=analyze(sample)
    keys=['regmap_startup','regmap_status_present','regmap_no_fatal_or_fault','regmap_bounds']
    assert all(r['checks'][k] for k in keys)
    assert all(r['regmap'][k] for k in ('beyond_four_owners_observed','reconstruction_observed','reattachment_observed'))
    assert not r['regmap']['lifecycle_stability_certified'] and not r['baseline_certified']
    for text,key in [(good.replace('fixedOwnerLimit=0','fixedOwnerLimit=4'),'regmap_startup'),
        (good.replace('externalHigh=105','externalHigh=513'),'regmap_bounds'),
        (good.replace('faults=0','faults=1'),'regmap_no_fatal_or_fault'),
        (good+'DSFV29s_REGMAP_FATAL reason=externalEnd\n','regmap_no_fatal_or_fault'),
        (good+'DSFV29s_LEGACY_FATAL reason=vehicleDefinitionMap exitCode=00000031\n','regmap_no_fatal_or_fault'),
        (good+'DSFV29s_REGMAP_INSTALL_FAIL reason=precommitRecheck startupCheckRevision=2\n','regmap_no_fatal_or_fault'),
        (good.replace('kind=constructed generation=2 active=0','kind=constructed generation=2 active=1'),'regmap_bounds')]:
        sample.write_text(text);assert not analyze(sample)['checks'][key],key
    sample.write_text(start+status.replace('owners=5','owners=1').replace('reconstructs=1','reconstructs=0').replace('reattaches=1','reattaches=0'))
    r=analyze(sample);assert not any(r['regmap'][k] for k in ('beyond_four_owners_observed','reconstruction_observed','reattachment_observed'))
    sample.write_text(good)
    result=dict(result='passed',checks=['startup and observed owner/reconstruction/reattachment parsing','bad startup/count/state/fatal rejection','no lifecycle certification or unexercised paths inferred'])
    (OUT/'parser.json').write_text(json.dumps(result,indent=2)+'\n');return result
def parent_call(method,**kwargs):
    saved=(parent.OUT,parent.BUILD,parent.NAME,parent.FLAGS)
    parent.OUT=OUT/'parent';parent.BUILD=BUILD;parent.NAME=NAME;parent.FLAGS=saved[3]|{'DSF_V29S_REGMAP_REUSE=1'}
    console=io.StringIO()
    try:
        with redirect_stdout(console):getattr(parent,method)(**kwargs)
    finally:parent.OUT,parent.BUILD,parent.NAME,parent.FLAGS=saved
    (OUT/(method+'_parent_console.txt')).write_text(console.getvalue())
def fixture():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    def readable_body(path):
        text=path.read_text();at=text.index('bool Readable(');start=text.index('{',at);end=start+1;depth=1
        while depth:
            depth+=(text[end]=='{')-(text[end]=='}');end+=1
        return text[start:end]
    assert readable_body(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp')==readable_body(ROOT/'tests/RegMapReuseTest.cpp'),'fixture must use exact production Readable body'
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    source=(ROOT/'src/V29sRegMapNative.inc').read_text()
    arrays={n:bytes(int(x,16) for x in b.split(',')) for n,b in re.findall(r'kRegMap(\w+)Native\[\d+\]=\{(.*?)\};',source)}
    for n,at,size in SPANS:assert arrays[n]==native.read(at,size),n
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4',
        '/Fo'+str(BUILD/'RegMapReuseTest.obj'),str(ROOT/'tests/RegMapReuseTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:RegMapTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'RegMapReuseTest.exe'),str(lib)]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'RegMapReuseTest.exe')],capture_output=True,text=True,timeout=45)
    (OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('regmap reuse checks passed\n')
    result=dict(result='passed',source_sha256=hashes(),checks=r.stdout.splitlines(),native_spans=SPANS)
    (OUT/'fixture.json').write_text(json.dumps(result,indent=2)+'\n');print(r.stdout)
    parent_call('fixture')
def verify_build(resume=False):
    tested=json.loads((OUT/'fixture.json').read_text());assert tested['source_sha256']==hashes(),'fixture stale'
    parent_call('verify_build',resume=resume,log_prefix=PREFIX)
    parser_result=verify_parser()
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def resolve(at):
        for _ in range(5):
            b=pe.read(at,5)
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('thunk chain')
    assert not any(k.startswith('?g_sRegMapArenas@') for k in symbols),'old fixed owner directory still compiled'
    at=symbol('?g_regMapPatches@')
    for i,(site,length,size,prefix) in enumerate([(0x86BFA0,5,0x71,'@RegMapConstruct@'),(0x86BE5F,8,8,'_DiagSRegMapBridge')]):
        # U32*3, pointer, target, trampoline, protection, bool =32 bytes.
        actual,n,total,stock,target=struct.unpack('<5I',pe.read(at+i*32,20))
        assert (actual,n,total)==(site,length,size)
        assert pe.read(stock,size)==PE().read(site,size)
        assert resolve(target)==symbol(prefix)
    bridge=symbol('_DiagSRegMapBridge');end=min(v for v in symbols.values() if v>bridge);code=pe.read(bridge,end-bridge)
    for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be5619d'):assert bytes.fromhex(pattern) in code,pattern
    assert b'\x9D\xFF\x25'+struct.pack('<I',symbol('_g_regMapInsertTrampoline')) in code
    for marker in (b'DSFV29s_REGMAP_START revision=1 status=1 hooks=2',b'fixedOwnerLimit=0 inlineNodes=45 externalNodes=512',b'constructorGeneration=1 requests=150 graphics=160',b'DSFV29s_REGMAP_FATAL reason=',b'DSFV29s_REGMAP_EVENT kind=',b'DSFV29s_REGMAP_STATUS',PREFIX):assert marker in pe.data,marker
    for prefix in ('@RegMapConstruct@','_DiagSRegMapBridge','_DiagSPrepareRegMap','?DiagSRegMapInstall@','?RegMapPoll@'):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at)
        r=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True)
        (OUT/(prefix.lstrip('@_?').split('@')[0]+'.txt')).write_text(r.stdout)
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_PAUSE_R1.asi'
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='2847467ed26424b6c17773b2c0bd179b6218fbca1f019dc97a50feb86e2c083f'
    assert hashlib.sha256((ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R1.asi').read_bytes()).hexdigest()=='7bec339a6bc22d3ba599fd8533863604d20da23a4ed82add357e59e724b93f9b'
    for marker in (b'startupCheckRevision=2',b'DSFV29s_REGMAP_INSTALL_FAIL reason='):assert marker in pe.data
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),source_sha256=hashes(),
        requests=150,graphics=160,inline_nodes=45,external_nodes=512,fixed_owner_limit=0,constructor_generation=True,
        insertion_hook_preserved='0086BE5F',new_constructor_hook='0086BFA0',fixture_checks=tested['checks'],
        inherited_replay_pause_and_capacities_verified=True,parser_checks=parser_result,game_run=False,installed=False,
        limitations=['External constructor/destructor allocation services are stubbed in native fixture.',
            'Stable storage is process-lived per distinct address; memory remains finite.',
            'Exact run19868 fault branch remains unknown; live repeated transitions need validation.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
def package():
    result=json.loads((OUT/'verification.json').read_text())
    tested=json.loads((OUT/'fixture.json').read_text())
    assert result['result']=='passed' and result['source_sha256']==hashes()==tested['source_sha256']
    parser_result=json.loads((OUT/'parser.json').read_text());assert parser_result['result']=='passed'
    result['parser_checks']=parser_result
    result['tools_sha256']={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ('scripts/verify_regmap_reuse.py','scripts/verify_replay_pause.py','scripts/analyze_baseline150_log.py','scripts/analyze_callback_log.py')}
    data=(BUILD/(NAME+'.asi')).read_bytes();assert hashlib.sha256(data).hexdigest()==result['sha256']
    dest=ROOT/'dist/MAIN_150'/(NAME+'.asi')
    if dest.exists():assert dest.read_bytes()==data,'refusing to overwrite unrelated staged artifact'
    else:dest.write_bytes(data)
    assert dest.read_bytes()==data
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    evidence=dict(result='passed',path=str(dest.relative_to(ROOT)),bytes=len(data),sha256=result['sha256'],configuration='Release',platform='Win32',installed=False,game_run=False)
    (OUT/'package.json').write_text(json.dumps(evidence,indent=2)+'\n');print(json.dumps(evidence,indent=2))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--fixture-only',action='store_true');p.add_argument('--verify-build',action='store_true');p.add_argument('--resume',action='store_true');p.add_argument('--parser-only',action='store_true');p.add_argument('--package',action='store_true');a=p.parse_args()
    if a.package:package();raise SystemExit(0)
    if a.parser_only:print(json.dumps(verify_parser(),indent=2));raise SystemExit(0)
    if not a.verify_build:fixture()
    if not a.fixture_only:verify_build(a.resume)

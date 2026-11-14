"""Offline verification of the combined replay ring experiment. Never runs Driver.exe."""
import argparse, hashlib, io, json, re, struct, subprocess, winreg
from contextlib import redirect_stdout
from pathlib import Path
from static_streaming_audit import PE, DUMPBIN
import verify_replay_client as parent
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/replay_read1m_r1_2026-11-14';BUILD=ROOT/'build/v29s_replay_read1m_r1'
NAME='DriverSF_V29s_160Capacity_150Request_REPLAY_READ1M_R1'
PARTS=('','Core','Bridges','Native','Layout')
SOURCES=['src/V29sReplayRead'+s+'.inc' for s in PARTS]+['tests/ReplayReadTest.cpp']
TOOLS=('scripts/verify_replay_read1m.py','scripts/analyze_replay_read1m_log.py','scripts/verify_replay_client.py','scripts/analyze_replay_client_log.py')
def native():
    p=PE();assert hashlib.sha256(p.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays=[('Ctor',0x73FAB0,0x67),('Dtor',0x73FB40,0x2B),('Reader',0x746940,0x2A7),('Dispatch',0x746CA0,0x53)]
    text='// Exact supported native functions; reader branches are relocated explicitly.\n'
    for name,at,size in arrays:
        text+=f'static const U8 kRF{name}Native[{size}]={{'+','.join(f'0x{b:02X}' for b in p.read(at,size))+'};\n'
    dest=ROOT/'src/V29sReplayReadNative.inc'
    if dest.exists():assert dest.read_text()==text,'native source differs; inspect instead of overwriting'
    else:dest.write_text(text)
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'native.json').write_text(json.dumps({'result':'passed','spans':arrays,'exe_sha256':hashlib.sha256(p.data).hexdigest()},indent=2)+'\n')
    print('Supported native arrays checked/generated')
def configure_parent():
    parent.OUT=OUT/'client';parent.BUILD=BUILD;parent.NAME=NAME;parent.PREFIX=b'DSFV29s_replay_read1m_'
    parent.SOURCES=parent.SOURCES+ [s for s in SOURCES if s not in parent.SOURCES]
    parent.parent.parent.FLAGS=(parent.parent.parent.FLAGS-{'DSF_V29S_REPLAY_HITCH=1'})|{'DSF_V29S_REPLAY_CLIENT=1','DSF_V29S_REPLAY_READ1M=1'}
def sdk_lib():
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    return max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
def fixture():
    native();BUILD.mkdir(parents=True,exist_ok=True)
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4',
         '/Fo'+str(BUILD/'ReplayReadTest.obj'),str(ROOT/'tests/ReplayReadTest.cpp'),'/link','/NODEFAULTLIB',
         '/ENTRY:ReadTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'ReplayReadTest.exe'),str(sdk_lib())]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'ReplayReadTest.exe')],capture_output=True,text=True,timeout=45);(OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('replay read checks passed\n');print(r.stdout)
    configure_parent();console=io.StringIO()
    with redirect_stdout(console):parent.fixture()
    (OUT/'client_fixture_console.log').write_text(console.getvalue())
    (OUT/'fixture.json').write_text(json.dumps({'result':'passed','checks':r.stdout.splitlines(),'source_sha256':parent.hashes()},indent=2)+'\n')
def select_project():
    p=ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj';t=p.read_text()
    old='<PreprocessorDefinitions>DSF_V29S_REPLAY_CLIENT=1;'
    new='<PreprocessorDefinitions>DSF_V29S_REPLAY_READ1M=1;DSF_V29S_REPLAY_CLIENT=1;'
    if old in t:t=t.replace(old,new,1)
    assert new in t
    mark='  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />'
    if 'ClInclude Include="src\\V29sReplayRead.inc"' not in t:
        assert t.count(mark)==1
        group='  <ItemGroup>'+''.join('<ClInclude Include="src\\V29sReplayRead'+s+'.inc" />' for s in PARTS)+'</ItemGroup>\n'
        t=t.replace(mark,group+mark)
    p.write_text(t)
    print('Active single project selects READ1M with CLIENT')
def parser_fixture():
    from analyze_replay_read1m_log import analyze
    path=OUT/'parser_fixture.log'
    client=(OUT/'client/parser_fixture.log').read_text()
    start='DSFV29s_REPLAY_READ_START revision=1 status=1 hooks=8 readerBytes=1048576 blockBytes=32768 readerOnly=1 writerChanged=0 separateBacking=1 guardBytes=65536 processLifetime=1 reusable=1 fixedOwnerLimit=0 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160\n'
    status='DSFV29s_REPLAY_READ_STATUS status=1 owners=1 constructs=2 destructs=1 reuses=1 writerBypass=1 allocationFailed=0 fallbacks=0 faults=0 unmanagedCalls=0 retainedBackingBytes=1048576 processLifetime=1\n'
    from analyze_replay_read1m_log import FIELDS
    values={k:'0' for k in FIELDS};values.update(rfPresent='1',rfGeneration='2',rfState='1',rfBuffer='20000000',rfCapacity='1048576',rfPolls='10',rfIssues='2',rfIssueBytes='65536')
    owner='DSFV29s_REPLAY_READ_OWNER owner=10000000 originalBuffer=30000000 originalCapacity=262144 '+' '.join(k+'='+v for k,v in values.items())+'\n'
    # Parent fixture has no backend/active fields, so it is not a managed reader
    # boundary. Add a dedicated file-reader boundary fixture below.
    good=start+client+owner+status;path.write_text(good);r=analyze(path);assert r['structural_pass'] and r['reader_exercised'] and r['reuse_observed']
    for bad,key in [(good.replace('readerBytes=1048576','readerBytes=262144'),'startup'),(good.replace('fallbacks=0','fallbacks=1'),'integrity'),(good.replace('faults=0','faults=1'),'integrity'),(good.replace('rfIssueRejected=0','rfIssueRejected=1'),'owner_and_boundary_schema'),(good.replace('rfCapacity=1048576','rfCapacity=262144'),'owner_and_boundary_schema'),(good.rstrip('\n'),'complete_records')]:
        path.write_text(bad);assert not analyze(path)['checks'][key],key
    altered=client.replace(' captureTicks=', ' backend=1 ioMode=0 active=1 capacity=1048576 base=20000000 '+' '.join(k+'='+v for k,v in values.items())+' captureTicks=')
    path.write_text(start+altered+owner+status);assert analyze(path)['structural_pass']
    path.write_text((start+altered+owner+status).replace('base=20000000','base=30000000'));assert not analyze(path)['checks']['owner_and_boundary_schema']
    path.write_text(start+client+status);r=analyze(path);assert r['structural_pass'] and not r['reader_exercised']
    path.write_text(good)
    return {'result':'passed','checks':['startup/fallback/fault/rejection/buffer/schema/completeness rejection','boundary backing/capacity match','no exercise/smoothness certification inferred']}
def verify():
    configure_parent();tested=json.loads((OUT/'fixture.json').read_text());assert tested['source_sha256']==parent.hashes(),'fixture stale'
    console=io.StringIO()
    with redirect_stdout(console):parent.verify()
    (OUT/'client_verify_console.log').write_text(console.getvalue())
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
    table=symbol('?kRFPatches@')
    sites=[(0x73FAB0,8,0x67,'@RFConstruct@'),(0x73FB40,9,0x2B,'@RFDestruct@'),
           (0x74694A,8,8,'_RFBridgePoll'),(0x74698C,6,6,'_RFBridgeCommit'),(0x746AD9,8,8,'_RFBridgeWrap'),
           (0x746B62,7,7,'_RFBridgeSpace'),(0x746BD7,6,6,'_RFBridgeIssue'),(0x746B2B,13,13,'_RFBridgeEnd')]
    native_pe=PE()
    for i,(at,n,size,prefix) in enumerate(sites):
        actual,length,total,stock,target=struct.unpack('<5I',pe.read(table+i*20,20));assert (actual,length,total)==(at,n,size)
        assert pe.read(stock,size)==native_pe.read(at,size) and resolve(target)==symbol(prefix)
        begin=symbol(prefix);end=min(v for v in symbols.values() if v>begin);code=pe.read(begin,end-begin)
        if i>=2:
            for pattern in ('9c60','8bdc','83e4f0','0fae0424','fc','0fae0c24','8be3619d'):assert bytes.fromhex(pattern) in code,(prefix,pattern)
            assert code.count(bytes.fromhex('0fae0424'))==code.count(bytes.fromhex('0fae0c24'))==1
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{begin:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.strip('@_')+'.txt')).write_text(asm)
    assert pe.read(symbol('?kRFReaderNative@'),0x2A7)==native_pe.read(0x746940,0x2A7)
    assert not any(k.startswith('_RHBridge') for k in symbols)
    for marker in (b'DSFV29s_REPLAY_READ_START revision=1 status=',b'fixedOwnerLimit=0 extraEngineCalls=0 gameThreadFileWrites=0 requests=150 graphics=160',b'DSFV29s_REPLAY_READ_STATUS',b'DSFV29s_REPLAY_READ_OWNER',b'rfLastStopRequired=',b'DSFV29s_replay_read1m_'):assert marker in pe.data,marker
    core=(ROOT/'src/V29sReplayReadCore.inc').read_text();observer=core[core.index('extern "C" __declspec(noinline) void __cdecl RFObserve'):core.index('static RFView RFCapture')]
    for forbidden in ('Readable(', 'VirtualQuery(', 'RFAllocate(', 'CapacityWrite(', 'WriteFile(', 'RFAddress(', 'QueryPerformanceCounter(', 'Sleep('):assert forbidden not in observer,forbidden
    for name,digest in [('REGMAP_REUSE_R2','8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa'),('REPLAY_HITCH_R1','758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8'),('REPLAY_CLIENT_R1','d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3')]:
        old=ROOT/'dist/MAIN_150'/('DriverSF_V29s_160Capacity_150Request_'+name+'.asi');assert hashlib.sha256(old.read_bytes()).hexdigest()==digest
    result={'result':'passed','name':NAME,'bytes':len(pe.data),'sha256':hashlib.sha256(pe.data).hexdigest(),
            'source_sha256':parent.hashes(),'tool_sha256':{s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in TOOLS},
            'fixture_checks':tested['checks'],'parser':parser_fixture(),'inherited_client_R2_verified':True,'reader_hooks':8,'readerBytes':1048576,'blockBytes':32768,'requests':150,'graphics':160,'installed':False,'game_run':False}
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('source_sha256','tool_sha256')},indent=2))
def package():
    configure_parent();r=json.loads((OUT/'verification.json').read_text());assert r['result']=='passed' and r['source_sha256']==parent.hashes()
    assert all(hashlib.sha256((ROOT/s).read_bytes()).hexdigest()==v for s,v in r['tool_sha256'].items())
    data=(BUILD/(NAME+'.asi')).read_bytes();assert hashlib.sha256(data).hexdigest()==r['sha256']
    dest=ROOT/'dist/MAIN_150'/(NAME+'.asi');assert not dest.exists() or dest.read_bytes()==data,'refuse overwrite'
    dest.write_bytes(data);assert dest.read_bytes()==data
    (OUT/'package.json').write_text(json.dumps({'path':str(dest.relative_to(ROOT)),'bytes':len(data),'sha256':r['sha256'],'installed':False,'game_run':False},indent=2)+'\n');print(str(dest))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--native-only',action='store_true');p.add_argument('--fixture-only',action='store_true');p.add_argument('--select-project',action='store_true');p.add_argument('--verify-build',action='store_true');p.add_argument('--package',action='store_true');a=p.parse_args()
    if a.select_project:select_project()
    elif a.native_only:native()
    elif a.package:package()
    else:
        if not a.verify_build:fixture()
        if not a.fixture_only:verify()

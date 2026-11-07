"""Run real native replay-boundary slices and verify REPLAY150_R1 plus its parents."""
from contextlib import redirect_stdout
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_parked150
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/replay150_r1_2026-11-07';BUILD=ROOT/'build/v29s_replay150_r1'
NAME='DriverSF_V29s_160Capacity_150Request_REPLAY150_R1'
SOURCES=['src/V29sReplay150.inc','src/V29sReplay150Core.inc','src/V29sReplay150Layout.inc','src/V29sReplay150Native.inc','tests/Replay150Test.cpp']
SPANS=[('ReplayCapture',0x74B7A0,0x170),('ReplayRestore',0x74BA20,0x120),('ReplayImport',0x74BB80,0x70),
       ('ReplaySaveTraffic',0x74C57D,5),('ReplaySaveRetained',0x74C59F,5),('ReplayLoadTraffic',0x74DF09,5),('ReplayLoadRetained',0x74DF4A,5),
       ('ReplaySaveSlice',0x74C53E,0x66),('ReplayLoadSlice',0x74DEDC,0xAD),('ReplayRetainedClone',0x5A69B0,0x142),('ReplayNativeRefs',0x5A6B00,0x80),('ReplayTick',0x74BB40,0x33)]

def hashes():return {s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}

def fixture():
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays={n:bytes(int(v,16) for v in b.split(',')) for n,b in re.findall(r'k(\w+)Native\[\d+\]=\{(.*?)\};',(ROOT/SOURCES[3]).read_text())}
    for name,at,n in SPANS:assert arrays[name]==native.read(at,n),name
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[p/'um/x86/kernel32.lib' for p in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',p.name)]
    lib=max((p for p in libs if p.is_file()),key=lambda p:tuple(map(int,p.parents[2].name.split('.'))))
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4',
        '/Fo'+str(BUILD/'Replay150Test.obj'),str(ROOT/'tests/Replay150Test.cpp'),'/link','/NODEFAULTLIB','/ENTRY:ReplayTestMain@0',
        '/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'Replay150Test.exe'),str(lib)]
    r=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr)
    assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'Replay150Test.exe')],capture_output=True,text=True,timeout=45)
    (OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('replay150 checks passed\n')
    result=dict(result='passed',source_sha256=hashes(),checks=r.stdout.splitlines(),spans=SPANS)
    (OUT/'fixture.json').write_text(json.dumps(result,indent=2)+'\n');print(r.stdout)

def verify_build(resume=False):
    tested=json.loads((OUT/'fixture.json').read_text());assert tested['result']=='passed' and tested['source_sha256']==hashes(),'fixture stale'
    if resume:
        parent=json.loads((OUT/'parent/verification.json').read_text())
        assert parent['result']=='passed' and parent['sha256']==hashlib.sha256((BUILD/(NAME+'.asi')).read_bytes()).hexdigest()
        baseline=json.loads((OUT/'parent/parent/parent/verification.json').read_text())
        for name,digest in baseline['source_sha256'].items():assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest
    else:
        console=io.StringIO()
        with redirect_stdout(console):verify_parked150.main(name=NAME,build=BUILD,output=OUT/'parent',extra_flags={'DSF_V29S_REPLAY150=1'},log_prefix=b'DSFV29s_replay150_')
        (OUT/'parent_console.txt').write_text(console.getvalue())
    from analyze_baseline150_log import analyze
    sample=OUT/'replay_parser_fixture.log'
    header=(ROOT/'src/Chapter7Preload.h').read_text()
    ids=sorted(map(int,re.findall(r'(\d+)u',re.search(r'kChapter7AllPositiveIds\[150\] = \{(.*?)\};',header,re.S).group(1))))
    h=2166136261
    for model in ids:h=((h^model)*16777619)&0xffffffff
    good='DSFV29s_REPLAY150_START revision=1 status=1 hooks=7 trafficCapacity=150 retainedCapacity=256 payloadBytes=2660 nativeViewBytes=612 nativeObjectBytes=624 nativeRecordFormat=1\n'
    good+=f'DSFV29s_REPLAY150_EVENT kind=restore count=150 retained=0 rosterHash={h:08X} id50=181 has181=1\n'
    good+='DSFV29s_REPLAY150_STATUS status=1 invalid=0 peakTraffic=150 peakRetained=0 activeTemps=0 loading=0\n'
    sample.write_text(good);parsed=analyze(sample)
    assert parsed['checks']['replay_startup'] and parsed['checks']['replay_no_fatal_or_invalid'] and parsed['checks']['replay_capacity_bounds']
    assert parsed['replay']['observed_canonical150_restore'] and not parsed['replay']['softlock_resolved'] and not parsed['baseline_certified']
    sample.write_text(good.replace('id50=181','id50=0')+'DSFV29s_REPLAY150_FATAL reason=fixture\n');parsed=analyze(sample)
    assert not parsed['checks']['replay_capacity_bounds'] and not parsed['checks']['replay_no_fatal_or_invalid'] and not parsed['replay']['observed_canonical150_restore']
    sample.write_text(good)
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    table=symbol('?kReplayPatches@')
    for i,(name,at,n) in enumerate(SPANS[:7]):
        actual,length,native_length,native_ptr,target,opcode=struct.unpack('<5IB3x',pe.read(table+i*24,24))
        assert (actual,length,native_length,opcode)==(at,6 if i<3 else 5,n,0xE9 if i<3 else 0xE8),(name,actual,length,native_length,opcode)
        native=PE();assert pe.read(native_ptr,n)==native.read(at,n)
        prefix=('@'+name+'@') if i<5 else ('_'+name)
        # MSVC's incremental-link thunk keeps the same ABI and tail-jumps to
        # the mapped function. Resolve it explicitly rather than accepting any target.
        resolved=target
        if pe.read(resolved,1)==b'\xE9':resolved+=5+struct.unpack('<i',pe.read(resolved+1,4))[0]
        assert resolved==symbol(prefix),(name,hex(target),hex(resolved))
    for name,_,_ in SPANS[:7]:
        prefix=('@' if name not in ('ReplayLoadTraffic','ReplayLoadRetained') else '_')+name
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at)
        code=pe.read(at,end-at)
        if name in ('ReplayCapture','ReplayImport'):assert b'\xC2\x04\x00' in code,name
        if name in ('ReplaySaveTraffic','ReplaySaveRetained'):assert b'\xC2\x08\x00' in code,name
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(name+'.txt')).write_text(asm)
    for marker in (b'DSFV29s_REPLAY150_START revision=1 status=1 hooks=7',b'trafficCapacity=150 retainedCapacity=256 payloadBytes=2660',b'nativeObjectBytes=624 nativeRecordFormat=1',b'DSFV29s_REPLAY150_EVENT',b'DSFV29s_REPLAY150_STATUS',b'DSFV29s_REPLAY150_FATAL',b'DSFV29s_replay150_'):assert marker in pe.data,marker
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_PARKED150_R1.asi'
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='6235a3cb814c8d5adf748766d50d06f7fb5d4cfbf7fb85eb59dc5db9aa2c1a36'
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),requests=150,graphics_slots=160,
        traffic_snapshot_capacity=150,retained_snapshot_capacity=256,payload_bytes=2660,native_object_bytes=624,native_view_bytes=612,
        game_patch_boundaries=7,native_record_format_preserved=True,other_capacity_settings_preserved=True,
        fixture_checks=tested['checks'],source_sha256=hashes(),
        limitations=['No game run or installation. Replay -> Story -> Replay and menu transitions require user testing.',
            'Full native save/load boundary instruction slices, old producer, retained clone, refcount lookup and phase tick execute in fixtures; external engine services are stubbed.',
            'Corrects snapshot field overlap; does not establish the sole cause of the supplied loading softlock.',
            'Earlier null simulation-manager Lua pause crash is not bypassed or attributed to this defect.',
            'Legacy recordings with already-corrupted vehicle zero cannot recover missing IDs automatically.',
            'Snapshot reference storage256 does not change the actual game retained-vehicle pool capacity.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--fixture-only',action='store_true');parser.add_argument('--verify-build',action='store_true');parser.add_argument('--resume',action='store_true');args=parser.parse_args()
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    if not args.verify_build:fixture()
    if not args.fixture_only:verify_build(args.resume)

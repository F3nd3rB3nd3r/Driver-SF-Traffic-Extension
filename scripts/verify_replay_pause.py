"""Verify the pause-safe native replay completion bridge and inherited150 build."""
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from contextlib import redirect_stdout
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_replay150
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/replay_pause_r1_2026-11-11';BUILD=ROOT/'build/v29s_replay_pause_r1'
NAME='DriverSF_V29s_160Capacity_150Request_REPLAY_PAUSE_R1'
SOURCES=['src/V29sReplayPause.inc','src/V29sReplayPauseCore.inc','src/V29sReplayPauseBridge.inc','src/V29sReplayPauseLayout.inc','src/V29sReplayPauseNative.inc','tests/ReplayPauseTest.cpp']
SPANS=[('PauseUpdate',0x73A090,0x68E),('PauseComplete',0x7390BE,0x76),('PauseReady',0x738D20,0x80),('PauseClockGate',0x85D8D4,0x16)]
FLAGS={'DSF_V29S_REPLAY_PAUSE=1'}
def hashes():return {s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}
def verify_parser():
    from analyze_baseline150_log import analyze
    OUT.mkdir(parents=True,exist_ok=True);sample=OUT/'parser_fixture.log'
    start='DSFV29s_REPLAY_PAUSE_START revision=1 status=1 hooks=1 site=0073A090 completion=007390BE importTag=1 sameThread=1 nativeReadyGate=1 forceUnpause=0 monitorEngineCalls=0 requests=150 graphics=160\n'
    held='DSFV29s_REPLAY_PAUSE_EVENT kind=nativeHeld phase=0 loadingBefore=1 pausedBefore=1 rateBefore=00000000 importEpoch=1 loadingAfter=1 clockBefore=3F644445 clockAfter=3F644445\n'
    completed='DSFV29s_REPLAY_PAUSE_EVENT kind=completed phase=0 loadingBefore=1 pausedBefore=1 rateBefore=00000000 importEpoch=1 loadingAfter=0 clockBefore=3F644445 clockAfter=3F644445\n'
    status='DSFV29s_REPLAY_PAUSE_STATUS status=1 updates=100 imports=1 candidates=2 attempts=2 completed=1 nativeHeld=1\n'
    good=start+held+completed+status;sample.write_text(good);r=analyze(sample)
    assert all(r['checks'][k] for k in ['replay_pause_startup','replay_pause_status_present','replay_pause_no_fatal','replay_pause_event_guards'])
    assert r['replay_pause']['completion_service_exercised'] and r['replay_pause']['native_readiness_held_observed']
    assert r['replay_pause']['completed_events_with_clock_unchanged']==1 and not r['replay_pause']['user_playback_controls_verified'] and not r['baseline_certified']
    for bad,key in [(good.replace('forceUnpause=0','forceUnpause=1'),'replay_pause_startup'),
                    (good.replace('phase=0','phase=1'),'replay_pause_event_guards'),
                    (start+completed.replace('loadingAfter=0','loadingAfter=1')+status,'replay_pause_event_guards'),
                    (good+'DSFV29s_REPLAY_PAUSE_FATAL reason=fixture\n','replay_pause_no_fatal')]:
        sample.write_text(bad);assert not analyze(sample)['checks'][key],key
    sample.write_text(start+status);assert not analyze(sample)['replay_pause']['completion_service_exercised']
    sample.write_text(good)
    result=dict(result='passed',checks=['guarded completion and native-held event parsing','bad startup/phase/completion/fatal rejection','no exercise or visible/control certification inferred from startup alone'])
    (OUT/'parser.json').write_text(json.dumps(result,indent=2)+'\n');return result
def fixture():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays={n:bytes(int(v,16) for v in b.split(',')) for n,b in re.findall(r'k(\w+)Native\[\d+\]=\{(.*?)\};',(ROOT/SOURCES[4]).read_text())}
    for name,at,n in SPANS:assert arrays[name]==native.read(at,n),name
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[p/'um/x86/kernel32.lib' for p in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',p.name)]
    lib=max((p for p in libs if p.is_file()),key=lambda p:tuple(map(int,p.parents[2].name.split('.'))))
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/W4',
         '/Fo'+str(BUILD/'ReplayPauseTest.obj'),str(ROOT/'tests/ReplayPauseTest.cpp'),'/link','/NODEFAULTLIB',
         '/ENTRY:PauseTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'ReplayPauseTest.exe'),str(lib)]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    r=subprocess.run([str(BUILD/'ReplayPauseTest.exe')],capture_output=True,text=True,timeout=30)
    (OUT/'fixture.log').write_text(r.stdout+r.stderr);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
    assert r.stdout.endswith('replay pause checks passed\n')
    result=dict(result='passed',source_sha256=hashes(),checks=r.stdout.splitlines());(OUT/'fixture.json').write_text(json.dumps(result,indent=2)+'\n');print(r.stdout)
    console=io.StringIO()
    with redirect_stdout(console):verify_replay150.fixture(build=BUILD,output=OUT/'snapshot',extra_flags=FLAGS)
    (OUT/'snapshot_console.txt').write_text(console.getvalue())
def verify_build(resume=False,log_prefix=b'DSFV29s_replay_pause_'):
    tested=json.loads((OUT/'fixture.json').read_text());assert tested['result']=='passed' and tested['source_sha256']==hashes(),'fixture stale'
    console=io.StringIO()
    with redirect_stdout(console):verify_replay150.verify_build(resume=resume,name=NAME,build=BUILD,output=OUT/'snapshot',extra_flags=FLAGS,log_prefix=log_prefix)
    (OUT/'parent_console.txt').write_text(console.getvalue())
    parser_result=verify_parser()
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(prefix):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at);return at,pe.read(at,end-at)
    for prefix in ('@PauseUpdate@','@PauseCompleteNative@','_PauseOnImport','?PauseInstall@','?PausePoll@'):
        at,code=body(prefix)
        if prefix=='@PauseCompleteNative@':
            assert code.startswith(bytes.fromhex('51568bf157ff25'))
            assert code[7:11]==struct.pack('<I',symbol('_g_pauseCompletionTarget'))
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{at+len(code)-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('@_?').split('@')[0]+'.txt')).write_text(asm)
    for name,at,n in SPANS[:3]:assert pe.read(symbol('?k'+name+'Native@'),n)==PE().read(at,n),name
    # Verify compiled parent import calls the additive observer (possibly via ILT).
    at,code=body('@ReplayImport@');observer=symbol('_PauseOnImport');calls=[]
    for i in range(len(code)-4):
        if code[i]!=0xE8:continue
        target=at+i+5+struct.unpack_from('<i',code,i+1)[0]
        try:
            if pe.read(target,1)==b'\xE9':target+=5+struct.unpack('<i',pe.read(target+1,4))[0]
        except Exception:continue
        calls.append(target)
    assert observer in calls,'import observer absent from compiled parent'
    for marker in (b'DSFV29s_REPLAY_PAUSE_START revision=1 status=1 hooks=1',b'completion=007390BE importTag=1 sameThread=1 nativeReadyGate=1 forceUnpause=0',b'DSFV29s_REPLAY_PAUSE_EVENT',b'DSFV29s_REPLAY_PAUSE_STATUS',b'DSFV29s_REPLAY_PAUSE_FATAL',log_prefix):assert marker in pe.data,marker
    main=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text();assert main.index('ReplayInstall();')<main.index('PauseInstall();')<main.index('CreateThread(nullptr, 0, &Monitor')
    runtime=(ROOT/SOURCES[0]).read_text();poll=runtime[runtime.index('static void PausePoll()'):];assert 'PauseRunCompletion' not in poll and 'PauseRunOriginal' not in poll
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY150_R1.asi';assert hashlib.sha256(old.read_bytes()).hexdigest()=='2ab7f763f216abfaf5091308f0adb3e428512038d2f0384432faa140aae12dd9'
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),requests=150,graphics_slots=160,
        hook_site='0073A090',native_completion='007390BE',native_readiness='00738D20',new_game_entry_hooks=1,
        simulation_clock_writes=False,force_unpause=False,monitor_engine_calls=False,matching_import_thread_required=True,
        snapshot150_256_preserved=True,all_other_capacities_preserved=True,fixture_checks=tested['checks'],source_sha256=hashes(),parser_checks=parser_result,
        limitations=['No game run or installation; live softlock recovery and user playback controls need validation.',
            'Fixture executes original readiness/completion bytes and production completion bridge; external world/subscriber/notifier/lifecycle services and the original update body are stubbed.',
            'Recovery targets captured mode2/loading1/lifecycle5/transition0/phase0/countdown0 with zero rates and paused simulation.',
            'Initial zero-rate caller/order and the separate null simulation-manager crash remain unproven/unpatched.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--fixture-only',action='store_true');p.add_argument('--verify-build',action='store_true');p.add_argument('--resume',action='store_true');p.add_argument('--parser-only',action='store_true');a=p.parse_args()
    if a.parser_only:
        result=verify_parser();print(json.dumps(result,indent=2));raise SystemExit(0)
    if not a.verify_build:fixture()
    if not a.fixture_only:verify_build(a.resume)

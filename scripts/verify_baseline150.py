"""Build/run the isolated diagnostics fixture and verify the compiled ASI/parent."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_arena_reuse
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/baseline150_r1_2026-11-06'
BUILD=ROOT/'build/v29s_baseline150_r1'
NAME='DriverSF_V29s_160Capacity_150Request_BASELINE150_R1'

def main(name=NAME,build=BUILD,output=OUT,extra_flags=None,log_prefix=b'DSFV29s_baseline150_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    native=PE()
    assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    spans={0x5A7B80:bytes.fromhex('51568bf18b86ac0d0000'),0x8D590C:bytes.fromhex('e88f34fdff'),0x59CFC0:bytes.fromhex('53558b6c240c8bd9')}
    for at,expected in spans.items():assert native.read(at,len(expected))==expected
    assert 0x8D5911+struct.unpack('<i',native.read(0x8D590D,4))[0]==0x8A8DA0
    # Save whole exact readiness predicate and call-site context, not just prologues.
    for lo,hi in ((0x59CFC0,0x59D026),(0x5A7B80,0x5A7CE0),(0x8D58D0,0x8D591C)):
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{lo:X},0x{hi:X}',str(__import__('static_streaming_audit').EXE)],capture_output=True,text=True,check=True).stdout
        (OUT/f'native_{lo:08X}.txt').write_text(asm)
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    exe=BUILD/'Baseline150Test.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'Baseline150Test.obj'),str(ROOT/'tests/Baseline150Test.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:BaselineTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(exe),str(lib)]
    compilation=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(compilation.stdout+compilation.stderr)
    assert compilation.returncode==0,compilation.stdout+compilation.stderr
    fixture=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
    (OUT/'fixture.log').write_text(fixture.stdout+fixture.stderr)
    assert fixture.returncode==0,(fixture.returncode,fixture.stdout[-1000:])
    assert fixture.stdout.endswith('baseline150 checks passed\n')
    for marker in ('kind=accepted','kind=superseded','kind=invalidInput','count=150','count=3','ready=150','readyLosses=1','bodyCallsOriginal=2','rosterHash='):
        assert marker in fixture.stdout,marker
    from analyze_baseline150_log import analyze
    parsed=analyze(OUT/'fixture.log')
    assert not parsed['checks']['startup'] and not parsed['checks']['no_event_drops']
    assert parsed['has_150_ready_sweep'] and not parsed['baseline_certified']
    assert parsed['phases'][0]['audio_setup_fail_observations']['187']==7
    assert 187 in parsed['phases'][0]['models_with_observed_fallback']
    (OUT/'fixture_analysis.json').write_text(json.dumps(parsed,indent=2)+'\n')
    console=io.StringIO()
    with redirect_stdout(console):
        verify_arena_reuse.main(name=NAME,build=BUILD,output=OUT/'parent',extra_flags={'DSF_V29S_BASELINE150=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'parent_console.txt').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    for prefix,target in (('_BaselineUpdateBridge','_g_baselineUpdateTrampoline'),('_BaselineDrawBridge','_g_baselineDrawTarget')):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at);code=pe.read(at,end-at)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be5619d'):
            assert bytes.fromhex(pattern) in code,(prefix,pattern)
        assert b'\x9D\xFF\x25'+struct.pack('<I',symbol(target)) in code
    for marker in (b'DSFV29s_BASELINE_START revision=1 status=1',b'DSFV29s_BASELINE_ROSTER',b'DSFV29s_BASELINE_MODEL',b'DSFV29s_BASELINE_METRIC',b'drawMeansPixels=0',b'GPUBytes=unmeasured'):
        assert marker in pe.data,marker
    baseline=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_ARENA_REUSE_R1.asi'
    assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='879236089985e71ae7e601d74bfc8a9f5570e3679128ec8957b5ccfbb015b5f7'
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),
        new_hooks=2,requests=150,graphics=160,capacity_changes=0,
        fixture_checks=['150-ID roster and exact original/effective storage','all150 READY in one checkpoint','readiness loss/recovery',
            'direct3-ID request preserved','rejection retains epoch','duplicates reported','queue overflow explicitly counted',
            'contention skips explicitly counted','original/resolved draw identities','per-phase sampled peak',
            'both bridges preserve GP/flags/DF/x87/SSE and original stack'],
        source_sha256={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ('src/V29sBaseline150.inc','tests/Baseline150Test.cpp','src/DriverSF_V29b_NativeFastPath.cpp','src/V29sCapacityTrace.inc')},
        limitations=['Native readiness service is stubbed in isolated fixture; real bytes/ABI reviewed statically.',
            'Draw call is not visible pixels; zero coverage can mean a model was never exercised.',
            'Audio counters are cumulative observations, not decoded failure reasons or audible success.',
            'Metrics are sampled; geometry/texture/GPU budgets remain unmeasured.','Not game-tested or installed.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()

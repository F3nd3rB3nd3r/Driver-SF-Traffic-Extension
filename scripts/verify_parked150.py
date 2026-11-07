"""Verify the parked150 native builder/picker and compiled ASI with parent checks."""
from contextlib import redirect_stdout
import argparse,hashlib,io,json,re,struct,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_notify150
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/parked150_r1_2026-11-06';BUILD=ROOT/'build/v29s_parked150_r1'
NAME='DriverSF_V29s_160Capacity_150Request_PARKED150_R1'

def main(fixture_only=False,name=NAME,build=BUILD,output=OUT,extra_flags=None,log_prefix=b'DSFV29s_parked150_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    arrays={name:bytes(int(v,16) for v in content.split(',')) for name,content in re.findall(r'k(\w+)Native\[\d+\]=\{(.*?)\};',(ROOT/'src/V29sParked150Native.inc').read_text())}
    for name,at,n in [('ParkBuild',0x809370,0x165),('ParkPick',0x808610,0x63),('ParkCopy',0x809F30,0x55)]:
        assert arrays[name]==native.read(at,n),name
    operands=[tuple(int(v.rstrip('u'),0) for v in row.split(',')) for row in re.findall(r'\{([^{}]+)\}',(ROOT/'src/V29sParked150Layout.inc').read_text().split('kParkOperands[]={',1)[1].split('};',1)[0])]
    for offset,stock,replacement in operands:assert struct.unpack_from('<I',arrays['ParkBuild'],offset)[0]==stock,(offset,stock)
    production=(ROOT/'src/V29sParked150.inc').read_text();test=(ROOT/'tests/Parked150Test.cpp').read_text()
    bridge=re.search(r'void ParkBucketBridge\(\)\{__asm \{(.*?)\}\}',production,re.S).group(1)
    fixture_bridge=re.search(r'void TestBucketBridge\(\)\{__asm \{(.*?)\}\}',test,re.S).group(1)
    assert re.sub(r'\s+','',bridge)==re.sub(r'\s+','',fixture_bridge.replace('g_testTrampoline','g_parkBucketTrampoline'))
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[p/'um/x86/kernel32.lib' for p in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',p.name)]
    lib=max((p for p in libs if p.is_file()),key=lambda p:tuple(map(int,p.parents[2].name.split('.'))))
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'Parked150Test.obj'),str(ROOT/'tests/Parked150Test.cpp'),'/link','/NODEFAULTLIB','/ENTRY:ParkedTestMain@0',
        '/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'Parked150Test.exe'),str(lib)]
    compiled=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(compiled.stdout+compiled.stderr)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    fixture=subprocess.run([str(BUILD/'Parked150Test.exe')],capture_output=True,text=True,timeout=45)
    (OUT/'fixture.log').write_text(fixture.stdout+fixture.stderr);assert fixture.returncode==0,(fixture.returncode,fixture.stdout,fixture.stderr)
    assert fixture.stdout.endswith('parked150 native checks passed\n')
    if fixture_only:print(fixture.stdout);return
    from analyze_baseline150_log import analyze
    header=(ROOT/'src/Chapter7Preload.h').read_text()
    ids=sorted(map(int,re.findall(r'(\d+)u',re.search(r'kChapter7AllPositiveIds\[150\] = \{(.*?)\};',header,re.S).group(1))))
    parser_sample=OUT/'parked_parser_fixture.log'
    good='DSFV29s_PARKED150_START revision=1 status=1 hooks=2 nativeCapacity=50 capacity=150 categories=25 perCategory=150 frameBytes=15732 parkedPopulationChanged=0 refreshTimingChanged=0 weightsChanged=0\n'
    good+='DSFV29s_PARKED150_BUILD count=150 ids='+','.join(map(str,ids))+'\n'
    good+='DSFV29s_PARKED150_STATUS status=1 invalid=0 guardBad=0 peakCandidates=150 peakBucket=150 peakUniqueSelections=150 extraSelections=1\n'
    parser_sample.write_text(good);parsed=analyze(parser_sample)
    assert parsed['checks']['parked_startup'] and parsed['checks']['parked_no_fatal_or_invalid'] and parsed['checks']['parked_build_rosters_complete']
    assert parsed['parked']['observed_150_candidate_build'] and parsed['parked']['observed_extended_picker_use']
    assert not parsed['baseline_certified'] and not parsed['parked']['visible_fallback_resolved']
    parser_sample.write_text(good.replace('peakBucket=150','peakBucket=151')+'DSFV29s_PARKED150_FATAL reason=test\n')
    parsed=analyze(parser_sample);assert not parsed['checks']['parked_capacity_bounds'] and not parsed['checks']['parked_no_fatal_or_invalid']
    parser_sample.write_text(good.replace('count=150 ids=','count=149 ids='));parsed=analyze(parser_sample);assert not parsed['checks']['parked_build_rosters_complete']
    parser_sample.write_text(good)
    console=io.StringIO()
    with redirect_stdout(console):verify_notify150.main(name=NAME,build=BUILD,output=OUT/'parent',extra_flags={'DSF_V29S_PARKED150=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'parent_console.txt').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    for prefix in ('_ParkBucketBridge','@ParkPick@','@ParkRebuild@','@ParkDescribe@','?ParkInstall@'):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at);code=pe.read(at,end-at)
        if prefix=='_ParkBucketBridge':
            for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be5619d'):assert bytes.fromhex(pattern) in code,pattern
            assert b'\x9D\xFF\x25'+struct.pack('<I',symbol('_g_parkBucketTrampoline')) in code
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('_?@').split('@')[0]+'.txt')).write_text(asm)
    table=symbol('?kParkOperands@')
    for i,row in enumerate(operands):assert struct.unpack('<III',pe.read(table+i*12,12))==row
    for marker in (b'DSFV29s_PARKED150_START revision=1 status=1 hooks=2',b'categories=25 perCategory=150 frameBytes=15732',
        b'parkedPopulationChanged=0 refreshTimingChanged=0 weightsChanged=0',b'DSFV29s_PARKED150_BUILD',b'DSFV29s_PARKED150_FATAL',log_prefix):
        assert marker in pe.data,marker
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_NOTIFY150_R1.asi'
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='d9be572176410e41fdbfe54a923ffc1759042a3ef80217f59742ee4f87e482e5'
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),requests=150,graphics_slots=160,
        parked_candidates=150,categories=25,parked_per_category=150,parked_frame_bytes=15732,game_entry_hooks=2,
        parked_population_changed=False,refresh_interval_changed=False,weights_changed=False,parent_notify150_preserved=True,
        source_sha256={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in ('src/V29sParked150.inc','src/V29sParked150Core.inc','src/V29sParked150Layout.inc','src/V29sParked150Native.inc','tests/Parked150Test.cpp')},
        fixture_checks=fixture.stdout.splitlines(),
        limitations=['Native clone/descriptor/root services and stack probing are stubbed in the isolated fixture; complete native builder/copy/picker bytes execute.',
            'No game run or installation. Live parked identity/fallback and visual outcome await user testing.',
            'Private cache storage persists per distinct manager address until process exit; no fixed owner limit.',
            'Separate full traffic snapshots and global audio-return interpretation remain outside this correction.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--fixture-only',action='store_true');main(parser.parse_args().fixture_only)

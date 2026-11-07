"""Execute complete native notification/writer/ring/replay fixture; verify ASI."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_baseline150
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/notify150_r1_2026-11-06';BUILD=ROOT/'build/v29s_notify150_r1'
NAME='DriverSF_V29s_160Capacity_150Request_NOTIFY150_R1'

def main(name=NAME,build=BUILD,output=OUT,extra_flags=None,log_prefix=b'DSFV29s_notify150_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    source=(ROOT/'src/V29sNotify150Native.inc').read_text()
    arrays={name:bytes(int(v,16) for v in content.split(',')) for name,content in re.findall(r'k(\w+)Native\[\d+\]=\{(.*?)\};',source)}
    blocks={'Notify':(0x754C70,0x9D),'Record':(0x7467D0,0x5E),'Ring':(0x746650,0x120),'Replay':(0x754AC0,0xE4)}
    for name,(at,n) in blocks.items():assert arrays[name]==native.read(at,n),name
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[p/'um/x86/kernel32.lib' for p in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',p.name)]
    lib=max((p for p in libs if p.is_file()),key=lambda p:tuple(map(int,p.parents[2].name.split('.'))))
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'Notify150Test.obj'),str(ROOT/'tests/Notify150Test.cpp'),'/link','/NODEFAULTLIB','/ENTRY:NotifyTestMain@0',
        '/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(BUILD/'Notify150Test.exe'),str(lib)]
    compiled=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(compiled.stdout+compiled.stderr)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    fixture=subprocess.run([str(BUILD/'Notify150Test.exe')],capture_output=True,text=True,timeout=30)
    (OUT/'fixture.log').write_text(fixture.stdout+fixture.stderr);assert fixture.returncode==0,(fixture.returncode,fixture.stdout,fixture.stderr)
    assert fixture.stdout.endswith('notify150 native checks passed\n')
    console=io.StringIO()
    with redirect_stdout(console):verify_baseline150.main(name=NAME,build=BUILD,output=OUT/'parent',extra_flags={'DSF_V29S_NOTIFY150=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'parent_console.txt').write_text(console.getvalue())
    # Verify the new actual record spellings, including fail-closed startup,
    # without mislabelling an unexercised callback as a successful live test.
    from analyze_baseline150_log import analyze
    parser_sample=OUT/'notification_parser_fixture.log'
    parser_sample.write_text('DSFV29s_NOTIFY150_START revision=1 status=1 patches=5 nativeCapacity=50 capacity=150 frameBytes=608\n'
        'DSFV29s_NOTIFY150_COPY count=150 storage=sidecar rosterHash=5CBDFFEA\n'
        'DSFV29s_NOTIFY150_STATUS status=1 invalid=0 copies=1 sidecarCopies=1\n')
    parsed=analyze(parser_sample);assert parsed['checks']['notification_startup'] and parsed['notification']['observed_150_sidecar_copy']
    assert not parsed['telemetry_checks_pass'] and not parsed['baseline_certified']
    parser_sample.write_text('DSFV29s_NOTIFY150_START revision=1 status=-1\nDSFV29s_NOTIFY150_FATAL reason=startupPatchRefused\n')
    parsed=analyze(parser_sample);assert not parsed['checks']['notification_startup'] and not parsed['checks']['notification_no_fatal_or_invalid']
    parser_sample.write_text('DSFV29s_NOTIFY150_START revision=1 status=1 patches=5 nativeCapacity=50 capacity=150 frameBytes=608\n'
        'DSFV29s_NOTIFY150_COPY count=150 storage=sidecar rosterHash=5CBDFFEA\nDSFV29s_NOTIFY150_STATUS status=1 invalid=0 copies=1 sidecarCopies=1\n')
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    expected=[(7,0xD0,0x260),(55,0xE4,0x274),(78,0xE0,0x270),(97,0x4D3B4B,0),(150,0xD0,0x260)]
    table=symbol('?kNotifyOperands@')
    for i,row in enumerate(expected):assert struct.unpack('<III',pe.read(table+12*i,12))==row
    assert pe.read(symbol('?kNotifyNative@'),0x9D)==arrays['Notify']
    for marker in (b'DSFV29s_NOTIFY150_START revision=1 status=1 patches=5',b'capacity=150 frameBytes=608',b'DSFV29s_NOTIFY150_COPY',
        b'missingOrStaleRequestSidecar',b'DSFV29s_NOTIFY150_FATAL',log_prefix):
        assert marker in pe.data,marker
    for prefix in ('_NotifyCopy','?NotifyInstall@'):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at)
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('_?').split('@')[0]+'.txt')).write_text(asm)
    old=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_BASELINE150_R1.asi'
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='bfbb726524bff658166fbb0dbeb968f94fa3f567f50f120b7e514ab99202247f'
    result=dict(result='passed',name=NAME,bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),
        requests=150,graphics_slots=160,notification_payload_ids=150,notification_frame_bytes=608,patches=5,
        all_other_capacity_settings_preserved=True,full_native_blocks=list(blocks),
        fixture_checks=['native stock51/150 overflow return value reproduced with test-harness return repair',
            'frame-only fix produces wrong ID51','all counts0..150 through complete native callback/writer/ring',
            'original event header/type/timestamp/byte length','contiguous and wrapping native ring writes with guards',
            '150 IDs round-trip through native replay dispatcher','replay readiness gate refusal',
            'both callback gates retain native behavior','callee-saved registers,RET8 stack and x87 stack balance',
            'oversize/missing/stale/mismatched-prefix sources refused before writes'],
        source_sha256={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
            ('src/V29sNotify150.inc','src/V29sNotify150Core.inc','src/V29sNotify150Layout.inc','src/V29sNotify150Native.inc','tests/Notify150Test.cpp')},
        limitations=['External factory/timestamp and replay request service are stubbed; actual native callback/writer/ring/dispatcher bytes execute.',
            'Recorded traffic events tested; separate fixed-size full traffic snapshot format is not expanded.',
            'No game run or installation; no universal pool/readiness/rendering/audio guarantee.'])
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()

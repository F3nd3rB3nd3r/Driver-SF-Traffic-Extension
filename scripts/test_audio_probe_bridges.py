"""Build/run only the synthetic Win32 ABI harness, never the game or ASI."""
import hashlib
import json
import re
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_probe_r1_2026-11-05'
BUILD=ROOT/'build/audio_probe_bridge_test'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    candidates=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    library=max((v for v in candidates if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    program=BUILD/'AudioProbeBridgeTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'AudioProbeBridgeTest.obj'),str(ROOT/'tests/AudioProbeBridgeTest.cpp'),
        '/link','/NODEFAULTLIB','/ENTRY:AudioBridgeTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86',
        '/OUT:'+str(program),str(library)]
    compile_result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'bridge_test_build.log').write_text(compile_result.stdout+compile_result.stderr)
    assert compile_result.returncode==0,compile_result.stdout+compile_result.stderr
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=20)
    assert run.returncode==0,f'Bridge test failed at check {run.returncode}: {run.stdout} {run.stderr}'
    assert run.stdout.strip()=='audio bridge checks passed',repr(run.stdout)
    result=dict(result='passed',architecture='Win32',synthetic_invocations=10,production_bridges=6,
        production_bridges_sha256=hashlib.sha256((ROOT/'src/V29sAudioProbeBridges.inc').read_bytes()).hexdigest(),
        checks=['callback result vs pool ID','all argument offsets','both event epilogues',
            'allocation success bypasses helpers','native return values and stack cleanup',
            'DF/CF and ECX/EDX restored','helpers see DF cleared','full x87/SSE FXSAVE image restored'],
        limitations='Synthetic native frames with production bridges; does not load ASI, execute the game, or test OS hook installation.')
    (OUT/'bridge_tests.json').write_text(json.dumps(result,indent=2))
    print(json.dumps(result,indent=2))

if __name__=='__main__':main()

"""Build/run only the isolated production static-bank installer fixture."""
import hashlib
import json
import re
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_static64m_r1_2026-11-05'
BUILD=ROOT/'build/audio_static64_pool_test'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    candidates=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    library=max((v for v in candidates if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    program=BUILD/'AudioStatic64PoolTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'AudioStatic64PoolTest.obj'),str(ROOT/'tests/AudioStatic64PoolTest.cpp'),
        '/link','/NODEFAULTLIB','/ENTRY:AudioStaticTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86',
        '/OUT:'+str(program),str(library)]
    result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'static64_test_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=20)
    assert run.returncode==0,f'Check/exit {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip()=='static64 scenarios passed: 29',repr(run.stdout)
    summary=dict(result='passed',architecture='Win32',scenarios=29,
        production_static_patch_sha256=hashlib.sha256((ROOT/'src/V29sAudioStatic64Patch.inc').read_bytes()).hexdigest(),
        production_default_patch_sha256=hashlib.sha256((ROOT/'src/V29sAudioDefaultPoolPatch.inc').read_bytes()).hexdigest(),
        checks=['default16 production installation precedes static patch','sole four-byte static operand changes',
            'prior baseline/identity/hooks validated','engine root and audio memory manager must remain cold',
            'unknown code/context refused','late cold/code mutations refused','protection/cache rollback',
            'incomplete rollback explicitly reported','default16 operand survives static rollback'],
        limitations='Fake image and fault-injected OS calls execute production installer logic. Does not exercise real game allocation, full DllMain, ASI loading or playback.')
    (OUT/'static_pool_tests.json').write_text(json.dumps(summary,indent=2));print(json.dumps(summary,indent=2))

if __name__=='__main__':main()

"""Compile/run only a synthetic Win32 fixture, never Driver.exe or an ASI."""
import hashlib
import json
import re
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_slots2048_r1_2026-11-05'
BUILD=ROOT/'build/audio_slots_fixture'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    candidates=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    library=max((v for v in candidates if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    program=BUILD/'AudioSlotsTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'AudioSlotsTest.obj'),str(ROOT/'tests/AudioSlotsTest.cpp'),
        '/link','/NODEFAULTLIB','/ENTRY:AudioSlotsTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86',
        '/OUT:'+str(program),str(library)]
    result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=20)
    assert run.returncode==0,f'Fixture failed at check {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip()=='audio slots checks passed',repr(run.stdout)
    result=dict(result='passed',architecture='Win32',bridge_invocations=9,
        production_sha256={name:hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()
            for name in ('V29sAudioSlotsCore.inc','V29sAudioSlotsBridges.inc','V29sAudioSlotsPatch.inc')},
        checks=['cold startup and baseline prerequisites','exact signatures for all nine hooks and paired settings',
            'trampoline bytes and return targets','all allocation/preprotect failures leave stock code',
            'late signature/phase change refused','cache/protection commit failure rollback',
            'rollback failure explicitly marked','counts above255 and at2048 with separate A0/A1/B lists',
            'actual insert peaks, ceiling and selected reuse counters','native update counts falling to zero',
            'separate8/255-user refusal counters','invalid headers rejected',
            'nine bridge arguments, all general registers, flags/DF and defined x87/SSE image preserved'],
        limitations='Fake native memory and OS fault injection; synthetic bridge frames. Does not execute Driver.exe, load an ASI or establish runtime audio benefit.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

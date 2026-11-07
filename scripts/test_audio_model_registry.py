"""Compile/run only a synthetic Win32 fixture, never Driver.exe or an ASI."""
import hashlib
import json
import re
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_model192_r1_2026-11-05'
BUILD=ROOT/'build/audio_model_registry_fixture'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    candidates=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    library=max((v for v in candidates if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    program=BUILD/'AudioModelRegistryTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'AudioModelRegistryTest.obj'),str(ROOT/'tests/AudioModelRegistryTest.cpp'),
        '/link','/NODEFAULTLIB','/ENTRY:AudioModelTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86',
        '/OUT:'+str(program),str(library)]
    result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=20)
    assert run.returncode==0,f'Fixture failed at check {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip()=='audio model registry checks passed',repr(run.stdout)
    result=dict(result='passed',architecture='Win32',bridge_invocations=8,
        production_sha256={name:hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()
            for name in ('V29sAudioModelRegistryCore.inc','V29sAudioModelRegistryBridges.inc','V29sAudioModelRegistryPatch.inc')},
        checks=['cold owned baseline and all signatures','seven storage/trampoline allocation failure cases',
            'page protection/cache rollback and explicit failure','only detour spans changed',
            'native constructor/register/find/insert/lookup/remove executed in guarded fake objects',
            '192 actual native registrations and lookups','193rd registration refused safely',
            'duplicate update atfull/belowfull','native removal/reuse of external nodes',
            'sentinel/default/provider bounds and external canaries preserved','constructor address reuse reattaches',
            'nonempty stock45 map refuses migration','setup bank/return/fallback frame outcomes',
            'eight bridge ABI invocations and CMP/TEST flags, ignored-ESP continuation, GP/x87/SSE preservation'],
        limitations='Selected native map instructions execute with rebased test pointers/calls and guarded fake objects. Synthetic setup frames and OS fault injection; no Driver.exe execution or ASI loading.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

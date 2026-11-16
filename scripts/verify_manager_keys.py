"""R8 native manager-key list regression; no game entry point or live process."""
import hashlib,json,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN,EXE
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/manager_keys_r8_2026-11-16';BUILD=ROOT/'build/manager_keys_r8_fixtures'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    OUT.mkdir(exist_ok=True);BUILD.mkdir(exist_ok=True)
    assert digest(EXE)=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    defs=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj').find('.//{*}PreprocessorDefinitions').text.split(';')
    assert 'DSF_V29S_MANAGER_KEYS=1' in defs
    lib=sdk_lib();cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']
    cmd+=['/D'+d for d in defs if not d.startswith('%')]
    cmd+=['/Fo'+str(BUILD/'keys.obj'),str(ROOT/'tests/ManagerKeyTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:ManagerKeyTestMain','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/'keys.exe'),str(lib),str(lib.with_name('psapi.lib'))]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr)
    assert r.returncode==0,r.stdout+r.stderr
    cases=[(280,'legacy',0),(280,'captured-hang','timeout'),(150,'',0),(280,'',0),(700,'',0),(1000,'',0),(280,'signature',0),(280,'full',0x29F3),(280,'guard',0x29F3),(280,'cycle',0x29F3)]
    runs=[]
    for n,mode,expected in cases:
        args=[str(BUILD/'keys.exe'),str(n)]+([mode] if mode else [])
        try:
            r=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=2 if expected=='timeout' else 60)
            row=dict(requests=n,mode=mode,exit=r.returncode,stdout=r.stdout,stderr=r.stderr)
        except subprocess.TimeoutExpired as e:
            assert expected=='timeout'
            row=dict(requests=n,mode=mode,exit='timeout',stdout=(e.stdout or b'').decode() if isinstance(e.stdout,bytes) else e.stdout,stderr='Expected isolated native-loop reproduction terminated by subprocess timeout')
        runs.append(row);(OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        print(dict(requests=n,mode=mode,exit=row['exit'],last=(row['stdout'] or '').splitlines()[-2:]),flush=True)
        assert row['exit']==expected,row
    dependencies={}
    def visit(p):
        p=p.resolve();name=str(p.relative_to(ROOT))
        if name in dependencies:return
        dependencies[name]=digest(p)
        for name in re.findall(r'^\s*#include "([^"]+)"',p.read_text(),re.M):visit(p.parent/name)
    visit(ROOT/'tests/ManagerKeyTest.cpp')
    for p in [Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj']:dependencies[str(p.relative_to(ROOT))]=digest(p)
    (OUT/'fixture_manifest.json').write_text(json.dumps(dict(result='passed',processes=len(runs),fixture_sha256=digest(BUILD/'keys.exe'),dependencies=dependencies,scope='retail manager header initialization, sorted key/refcount access, search/insert/remove/recycle and production hooks; mocked unrelated manager factories; no gameplay'),indent=2)+'\n')
if __name__=='__main__':main()

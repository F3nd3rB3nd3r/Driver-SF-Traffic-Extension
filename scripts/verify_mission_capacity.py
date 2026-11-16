"""R8 isolated retail mission admission regression. Never launches the game."""
import hashlib,json,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN,EXE,PE
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/mission_capacity_r8_2026-11-16'
BUILD=ROOT/'build/mission_capacity_r8_fixtures'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    BUILD.mkdir(exist_ok=True);OUT.mkdir(exist_ok=True)
    assert digest(EXE)=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    native=PE(EXE)
    assert native.read(0x5A0AD0,18)==bytes.fromhex('8b81f87200008b4c24040fb6440818c20400')
    assert native.refs(0x5A0AD0,True)==['0059C5DF']
    definitions=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj').find('.//{*}PreprocessorDefinitions').text.split(';')
    lib=sdk_lib();cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']
    cmd+=['/D'+d for d in definitions if not d.startswith('%')]
    cmd+=['/Fo'+str(BUILD/'mission.obj'),str(ROOT/'tests/MissionCapacityTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:MissionTestMain','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/'mission.exe'),str(lib),str(lib.with_name('psapi.lib'))]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(r.stdout+r.stderr)
    assert r.returncode==0,r.stdout+r.stderr
    runs=[]
    for n,mode in [(18,''),(150,''),(245,''),(246,''),(255,''),(256,''),(280,''),(700,''),(1000,''),(280,'signature')]:
        r=subprocess.run([str(BUILD/'mission.exe'),str(n)]+([mode] if mode else []),cwd=ROOT,capture_output=True,text=True,timeout=45)
        row=dict(requests=n,mode=mode,exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row)
        (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        print(dict(requests=n,mode=mode,exit=r.returncode,last=r.stdout.splitlines()[-2:]),flush=True)
        assert r.returncode==0,row
    dependencies={}
    def visit(path):
        path=path.resolve();name=str(path.relative_to(ROOT))
        if name in dependencies:return
        dependencies[name]=digest(path)
        for name in re.findall(r'^\s*#include "([^"]+)"',path.read_text(),re.M):visit(path.parent/name)
    visit(ROOT/'tests/MissionCapacityTest.cpp')
    for p in [Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj']:dependencies[str(p.relative_to(ROOT))]=digest(p)
    (OUT/'fixture_manifest.json').write_text(json.dumps(dict(result='passed',processes=len(runs),fixture_sha256=digest(BUILD/'mission.exe'),dependencies=dependencies,native_sha256=digest(EXE),scope='isolated retail catalog/admission/getter/queue and initializer slices, synthetic owner data, no asynchronous loading/gameplay'),indent=2)+'\n')
if __name__=='__main__':main()

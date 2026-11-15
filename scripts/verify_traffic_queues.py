"""Offline R7 traffic queue/native-boundary regression; never starts the game."""
import hashlib,json,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN,PE,EXE
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/traffic_queues_r7_2026-11-15'
BUILD=ROOT/'build/traffic_queues_r7_fixtures'

def main():
    OUT.mkdir(exist_ok=True);BUILD.mkdir(exist_ok=True)
    assert hashlib.sha256(EXE.read_bytes()).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    defs=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj').find('.//{*}PreprocessorDefinitions').text.split(';')
    assert 'DSF_V29S_TRAFFIC_QUEUES=1' in defs
    lib=sdk_lib();cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']
    cmd+=['/D'+d for d in defs if not d.startswith('%')]
    cmd+=['/Fo'+str(BUILD/'traffic.obj'),str(ROOT/'tests/TrafficQueueTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:TrafficTestMain','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/'traffic.exe'),str(lib),str(lib.with_name('psapi.lib'))]
    result=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    runs=[]
    for n,mode,expected in [(150,'',0),(160,'',0),(500,'',0),(1000,'',0),(160,'fatal',0x29F3),(160,'signature',0)]:
        args=[str(BUILD/'traffic.exe'),str(n)]+([mode] if mode else [])
        r=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=45)
        runs.append(dict(requests=n,mode=mode,exit=r.returncode,stdout=r.stdout,stderr=r.stderr))
        (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        print(dict(requests=n,mode=mode,exit=r.returncode,last=r.stdout.splitlines()[-2:]),flush=True)
        assert r.returncode==expected,r.stdout+r.stderr
    files={}
    def visit(path):
        path=path.resolve();key=str(path.relative_to(ROOT))
        if key in files:return
        files[key]=hashlib.sha256(path.read_bytes()).hexdigest()
        for name in re.findall(r'^\s*#include "([^"]+)"',path.read_text(),re.M):visit(path.parent/name)
    visit(ROOT/'tests/TrafficQueueTest.cpp')
    for p in [Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj']:files[str(p.relative_to(ROOT))]=hashlib.sha256(p.read_bytes()).hexdigest()
    (OUT/'fixture_manifest.json').write_text(json.dumps(dict(dependencies=files,fixture_sha256=hashlib.sha256((BUILD/'traffic.exe').read_bytes()).hexdigest(),processes=len(runs),result='passed',scope='native mapped slices, queue guard helpers and existing detours; no game entry point or gameplay'),indent=2)+'\n')
if __name__=='__main__':main()

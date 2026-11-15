"""Offline USER CAP R3 fixtures. Never starts the game or its entry point."""
import hashlib,json,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/wide_limits_r3_2026-11-15'
BUILD=ROOT/'build/wide_limits_r3_fixtures'
def main():
    OUT.mkdir(exist_ok=True);BUILD.mkdir(exist_ok=True)
    project=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj')
    definitions=project.find('.//{*}PreprocessorDefinitions').text.split(';')
    definitions=[d for d in definitions if not d.startswith('%')]
    lib=sdk_lib()
    cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']
    cmd+=['/D'+d for d in definitions]
    cmd+=['/Fo'+str(BUILD/'wide.obj'),str(ROOT/'tests/WideLimitsTest.cpp'),'/link','/NODEFAULTLIB','/ENTRY:WideTestMain','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/'wide.exe'),str(lib),str(lib.with_name('psapi.lib'))]
    r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(r.stdout+r.stderr)
    assert r.returncode==0,r.stdout+r.stderr
    # Keep a preprocessed copy of the exact active macro configuration for
    # review of inactive legacy arrays versus the candidate's actual bounds.
    pre=cmd[:cmd.index('/link')]
    pre=[v for v in pre if not v.startswith('/Fo')]
    pre+=['/P','/Fi'+str(OUT/'active_preprocessed.cpp')]
    r=subprocess.run(pre,cwd=BUILD,capture_output=True,text=True)
    assert r.returncode==0,r.stdout+r.stderr
    runs=[]
    for n in (18,150,245,246,255,256,500,700,1000):
        r=subprocess.run([str(BUILD/'wide.exe'),str(n)],cwd=ROOT,capture_output=True,text=True,timeout=45)
        row=dict(capacity=n,exit=r.returncode,stdout=r.stdout,stderr=r.stderr)
        runs.append(row);(OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        print(row,flush=True);assert r.returncode==0,row
    for n in (150,700,1000):
        r=subprocess.run([str(BUILD/'wide.exe'),str(n),'startup'],cwd=ROOT,capture_output=True,text=True,timeout=45)
        row=dict(capacity=n,mode='startup',exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row)
        (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        (OUT/('startup_'+str(n)+'.log')).write_text(r.stdout)
        print(dict(capacity=n,mode='startup',exit=r.returncode,last=r.stdout.splitlines()[-1:]),flush=True);assert r.returncode==0,row
    r=subprocess.run([str(BUILD/'wide.exe'),'700','startup','customized'],cwd=ROOT,capture_output=True,text=True,timeout=45)
    row=dict(capacity=700,mode='customized-startup',exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row)
    (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n');(OUT/'startup_customized.log').write_text(r.stdout)
    print(dict(mode='customized-startup',exit=r.returncode,last=r.stdout.splitlines()[-1:]),flush=True);assert r.returncode==0,row
    for mode in ('collection-full','allocation-failure','byte-overflow','completed-full','completed-misaligned','completed-foreign'):

        r=subprocess.run([str(BUILD/'wide.exe'),'150',mode],cwd=ROOT,capture_output=True,text=True,timeout=45)
        row=dict(mode=mode,exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row)
        (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        print(row,flush=True);assert r.returncode==0x29F3,row
    dependencies={}
    def visit(path):
        path=path.resolve();name=str(path.relative_to(ROOT))
        if name in dependencies:return
        dependencies[name]=hashlib.sha256(path.read_bytes()).hexdigest()
        for match in re.findall(r'^\s*#include "([^"]+)"',path.read_text(),re.M):visit(path.parent/match)
    visit(ROOT/'tests/WideLimitsTest.cpp')
    for path in (Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj',ROOT/'dist/USERCAP_R3/DriverSF_Unlimiter.ini',ROOT/'scripts/analyze_generic_limits_log.py',ROOT/'scripts/verify_wide_limits_build.py'):
        dependencies[str(path.relative_to(ROOT))]=hashlib.sha256(path.read_bytes()).hexdigest()
    (OUT/'fixture_manifest.json').write_text(json.dumps(dict(dependencies=dependencies,fixture_sha256=hashlib.sha256((BUILD/'wide.exe').read_bytes()).hexdigest(),scope='isolated native slices and production helpers; no game entry point or gameplay'),indent=2)+'\n')
if __name__=='__main__':main()


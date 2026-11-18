"""R9 diagnostic and configuration fixtures; no game entry point or ASI loading."""
import hashlib,json,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN,PE,EXE
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_setup_r9_2026-11-17'
BUILD=ROOT/'build/audio_setup_r9_fixtures'

def main():
    OUT.mkdir(exist_ok=True);BUILD.mkdir(exist_ok=True)
    pe=PE(EXE);assert hashlib.sha256(pe.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    project=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj')
    defines=[v for v in project.find('.//{*}PreprocessorDefinitions').text.split(';') if not v.startswith('%')]
    assert 'DSF_V29S_AUDIO_SETUP_TRACE=1' in defines
    lib=sdk_lib();runs=[]
    for name,source,entry in [('trace','AudioSetupTraceTest.cpp','AudioSetupTestMain'),('wide','WideLimitsTest.cpp','WideTestMain')]:
        cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']
        cmd+=['/D'+v for v in defines]
        cmd+=['/Fo'+str(BUILD/(name+'.obj')),str(ROOT/'tests'/source),'/link','/NODEFAULTLIB','/ENTRY:'+entry,'/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/(name+'.exe')),str(lib),str(lib.with_name('psapi.lib'))]
        r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/(name+'_build.log')).write_text(r.stdout+r.stderr)
        assert r.returncode==0,r.stdout+r.stderr
    cases=[('trace',[])]
    cases += [('wide',[str(n)]) for n in [18,150,255,256,280,500,700,1000]]
    cases += [('wide',[str(n),'startup']) for n in [18,150,280,700,1000]]
    cases += [('wide',['700','startup','customized'])]
    for program,args in cases:
        r=subprocess.run([str(BUILD/(program+'.exe')),*args],cwd=ROOT,capture_output=True,text=True,timeout=45)
        row=dict(program=program,args=args,exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row)
        (OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        if 'startup' in args:(OUT/('startup_'+('_'.join(args))+'.log')).write_text(r.stdout)
        print(dict(program=program,args=args,exit=r.returncode,last=r.stdout.splitlines()[-1:]),flush=True)
        assert r.returncode==0,row
    dependencies={}
    def visit(p):
        import re
        p=p.resolve();name=str(p.relative_to(ROOT))
        if name in dependencies:return
        dependencies[name]=hashlib.sha256(p.read_bytes()).hexdigest()
        for m in re.findall(r'^\s*#include "([^"]+)"',p.read_text(),re.M):visit(p.parent/m)
    visit(ROOT/'tests/AudioSetupTraceTest.cpp')
    for p in [Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj',ROOT/'scripts/analyze_generic_limits_log.py',
              ROOT/'dist/USERCAP_R9/DriverSF_Unlimiter.ini',ROOT/'dist/USERCAP_R9/profiles/Baseline150.ini',ROOT/'dist/USERCAP_R9/profiles/Headroom320.ini']:
        dependencies[str(p.relative_to(ROOT))]=hashlib.sha256(p.read_bytes()).hexdigest()
    (OUT/'fixture_manifest.json').write_text(json.dumps(dict(result='passed',processes=len(runs),dependencies=dependencies,scope='offline decisions, ABI, copied-image native storage and cold startup; no gameplay'),indent=2)+'\n')
if __name__=='__main__':main()

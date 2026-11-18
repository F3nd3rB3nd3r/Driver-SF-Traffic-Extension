"""R10 output-policy/configuration tests; no game entry point or ASI loading."""
import hashlib,json,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import DUMPBIN,PE,EXE
from verify_replay_read1m import sdk_lib
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/logging_r10_2026-11-18';BUILD=ROOT/'build/logging_r10_fixtures'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    OUT.mkdir(exist_ok=True);BUILD.mkdir(exist_ok=True)
    native=PE(EXE);assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    project=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj');defines=[v for v in project.find('.//{*}PreprocessorDefinitions').text.split(';') if not v.startswith('%')]
    assert 'DSF_V29S_LOGGING=1' in defines
    lib=sdk_lib();runs=[]
    for name,source,entry in [('logging','LoggingPolicyTest.cpp','LoggingTestMain'),('trace','AudioSetupTraceTest.cpp','AudioSetupTestMain'),('wide','WideLimitsTest.cpp','WideTestMain')]:
        cmd=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-','/std:c++17','/W4']+['/D'+v for v in defines]
        cmd+=['/Fo'+str(BUILD/(name+'.obj')),str(ROOT/'tests'/source),'/link','/NODEFAULTLIB','/ENTRY:'+entry,'/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x50000000','/DYNAMICBASE:NO','/STACK:0x100000','/OUT:'+str(BUILD/(name+'.exe')),str(lib),str(lib.with_name('psapi.lib'))]
        r=subprocess.run(cmd,cwd=BUILD,capture_output=True,text=True);(OUT/(name+'_build.log')).write_text(r.stdout+r.stderr);assert r.returncode==0,r.stdout+r.stderr
    cases=[('logging',[]),('trace',[])]+[('logging',['replay',v]) for v in ['minimal','summary','full']]
    cases += [('wide',[str(n)]) for n in [18,150,255,256,280,320,500,700,1000]]
    cases += [('wide',[str(n),'startup','profile'+str(n)]) for n in [150,280,320]]
    cases += [('wide',['150','startup','log-minimal']),('wide',['280','startup','log-full']),('wide',['700','startup','customized'])]
    for program,args in cases:
        r=subprocess.run([str(BUILD/(program+'.exe')),*args],cwd=ROOT,capture_output=True,text=True,timeout=60)
        row=dict(program=program,args=args,exit=r.returncode,stdout=r.stdout,stderr=r.stderr);runs.append(row);(OUT/'fixtures.json').write_text(json.dumps(runs,indent=2)+'\n')
        if 'startup' in args:(OUT/('startup_'+'_'.join(args)+'.log')).write_text(r.stdout)
        print(dict(program=program,args=args,exit=r.returncode,last=r.stdout.splitlines()[-1:]),flush=True);assert r.returncode==0,row
    source=ROOT/'dist/USERCAP_R9/DSFV29s_usercap_r9_7044.log'
    assert digest(OUT/'filtered_full.log')==digest(source)
    assert (OUT/'filtered_minimal.log').stat().st_size<(OUT/'filtered_summary.log').stat().st_size<source.stat().st_size//5
    deps={}
    def visit(p):
        p=p.resolve();name=str(p.relative_to(ROOT))
        if name in deps:return
        deps[name]=digest(p)
        for m in re.findall(r'^\s*#include "([^"]+)"',p.read_text(),re.M):visit(p.parent/m)
    for name in ['LoggingPolicyTest.cpp','AudioSetupTraceTest.cpp']:visit(ROOT/'tests'/name)
    for p in [Path(__file__),ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj',ROOT/'scripts/analyze_generic_limits_log.py',ROOT/'config/DriverSF_Unlimiter.ini',ROOT/'config/profiles/Expanded280.ini',ROOT/'config/profiles/Headroom320.ini']:deps[str(p.relative_to(ROOT))]=digest(p)
    report=dict(result='passed',processes=len(runs),dependencies=deps,fixture_hashes={n:digest(BUILD/(n+'.exe')) for n in ['logging','trace','wide']},
        filter_input=dict(path=str(source.relative_to(ROOT)),bytes=source.stat().st_size,sha256=digest(source)),filtered_bytes={v:(OUT/('filtered_'+v+'.log')).stat().st_size for v in ['minimal','summary','full']},scope='output selection, parser, ABI and copied-image storage/startup; no game, no speed benchmark')
    (OUT/'fixture_manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k not in ['dependencies','fixture_hashes']},indent=2))
if __name__=='__main__':main()

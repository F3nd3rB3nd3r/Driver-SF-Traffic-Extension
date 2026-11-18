"""Verify the local R10 build and reduced/full log semantics; no packaging."""
import hashlib,json,re,struct,subprocess,tempfile
from pathlib import Path
from static_streaming_audit import PE,EXE,DUMPBIN
from analyze_generic_limits_log import analyze
from test_generic_limits_log import run as legacy_parser_tests
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/logging_r10_2026-11-18';BUILD=ROOT/'build/logging_r10_fixtures'
ASI=ROOT/'build/v29s_usercap_r10/DriverSF_Unlimiter_USERCAP_R10.asi'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    manifest=json.loads((OUT/'fixture_manifest.json').read_text());assert manifest['result']=='passed' and manifest['processes']==20
    for name,h in manifest['dependencies'].items():assert digest(ROOT/name)==h,('stale fixture',name)
    for name,h in manifest['fixture_hashes'].items():assert digest(BUILD/(name+'.exe'))==h
    p=PE(ASI);native=PE(EXE);assert struct.unpack_from('<H',p.data,p.pe+4)[0]==0x14c
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(a):return p.read(a,min(v for v in symbols.values() if v>a)-a)
    def resolve(a):
        for _ in range(8):
            try:b=p.read(a,5)
            except ValueError:return a
            if b[0]!=0xE9:return a
            a=(a+5+struct.unpack_from('<i',b,1)[0])&0xFFFFFFFF
        raise AssertionError('jump chain')
    def calls(a,target):
        code=body(a)
        return any(code[i]==0xE8 and resolve((a+i+5+struct.unpack_from('<i',code,i+1)[0])&0xFFFFFFFF)==target for i in range(len(code)-4))
    assert struct.unpack('<I',p.read(at('?g_diagnosticLevel@'),4))[0]==1
    assert calls(at('?CapacityWrite@'),at('?LoggingSelect@'))
    assert calls(at('?Monitor@'),at('?LoggingPoll@'))
    assert calls(at('?LoggingSelect@'),at('?LoggingEssential@'))
    assert calls(at('?LoggingPoll@'),at('?CapacityWrite@'))
    assert calls(at('?LoggingStartLine@'),at('?CapacityWrite@'))
    for token in [b'DSFV29s_usercap_r10_',b'DSFV29s_LOGGING_START',b'DSFV29s_LOGGING_STATUS',b'diagnosticLogLevel',b'Diagnostics',b'LogLevel',b'DSFV29s_MANAGER_KEY_FATAL',b'DSFV29s_STREAM_HANDLER_FATAL',b'DSFV29s_LEGACY_FATAL']:assert token in p.data,token
    # All inherited audio hook signatures/bridges are retained; output policy
    # introduces no new native detour sites.
    native_tables=[]
    for prefix,count,stride,offset in [('?kAudioSetupSites@',10,28,24),('?kAudioModelSites@',8,24,20)]:
        table=at(prefix)
        for i in range(count):
            va,n=struct.unpack('<II',p.read(table+stride*i,8));raw=p.read(table+stride*i+8,n)
            assert raw==native.read(va,n)
            bridge=resolve(struct.unpack('<I',p.read(table+stride*i+offset,4))[0]);code=body(bridge)
            for pattern in ['9c60','0fae0424','0fae0c24','fc','8be5619d']:assert bytes.fromhex(pattern) in code,(prefix,i,pattern)
            if prefix.startswith('?kAudioSetup'):
                assert b'\xff\x64\x24\xe8' in code and calls(bridge,symbols['_AudioSetupEntry'])
            native_tables.append(dict(table=prefix,index=i,site=f'{va:08X}',bytes=raw.hex()))
    imports=subprocess.run([str(DUMPBIN),'/nologo','/imports',str(ASI)],capture_output=True,text=True,check=True).stdout
    (OUT/'imports.txt').write_text(imports);libraries=re.findall(r'^\s+([\w.-]+\.dll)\s*$',imports,re.I|re.M)
    assert {v.lower() for v in libraries}=={'kernel32.dll','psapi.dll'},libraries
    assert digest(ROOT/'dist/USERCAP_R9/DriverSF_Unlimiter_USERCAP_R9.asi')=='26f738c61ccb48150a71e3e843a3b5d001aadf91bb49dd38ec3787137c28409a'
    old=analyze(ROOT/'dist/USERCAP_R9/DSFV29s_usercap_r9_7044.log');assert {k for k,v in old['checks'].items() if not v}=={'audio_setup'}
    assert old['logging']['full_trace_available'] and len(old['checks'])==47
    assert legacy_parser_tests()['result']=='passed'
    # Synthetic metadata is added only to parser-test copies of filtered R9
    # evidence. These copies are not actual R10 gameplay logs.
    parser_cases=[]
    def header(level):return f'DSFV29s_LOGGING_START revision=1 level={level} fullTrace={int(level==2)} summaryMs=30000 checksDisabled=0 outputOnly=1 loggingChangesLimits=0 finalExitSnapshot=0\n'
    def status(level):return f'DSFV29s_LOGGING_STATUS tick=533765 level={level} attempted=51333 accepted=100 suppressed=51233 fileStatus=0 countsBeforeThisLine=1\n'
    with tempfile.TemporaryDirectory(prefix='dsf_logging_parser_') as t:
        file=Path(t)/'DSFV29s_usercap_r10_fixture.log'
        for level,name in enumerate(['minimal','summary','full']):
            data=(OUT/('filtered_'+name+'.log')).read_text();data=re.sub(r'(DSFV29s_GENERIC_START[^\n]*)',lambda m:m[1]+f' logLevel={level}',data)
            file.write_text(header(level)+data+(status(level) if level else ''))
            r=analyze(file);assert r['checks']['logging_configuration'] and r['checks']['logging_status']
            assert r['logging']['full_trace_available']==(level==2) and r['checks']['logging_full_trace']==(level==2)
            assert not r['recorded_integrity_pass'] # Full data still has48 real preview returns.
            if level==1:assert len(r['phases'])==9 and all(v['all_ready_observed'] for v in r['phases'])
            parser_cases.append(dict(level=level,full_trace=level==2,not_misreported_as_clean=True))
        raw=(OUT/'filtered_full.log').read_text()
        clean=re.sub(r'\bsetupFail=\d+','setupFail=0',raw);clean=re.sub(r'\bpreviewSuppressed=\d+','previewSuppressed=0',clean)
        clean=re.sub(r'\bouterTotal=\d+','outerTotal=0',clean);clean=re.sub(r'\bsetupFailSample=\d+','setupFailSample=0',clean)
        clean='\n'.join(v for v in clean.splitlines() if not v.startswith('DSFV29s_AUDIO_SETUP_DETAIL'))+'\n'
        file.write_text(header(2)+clean+status(2));assert analyze(file)['recorded_integrity_pass']
        for before,after,key in [('fullTrace=1','fullTrace=0','logging_configuration'),('checksDisabled=0','checksDisabled=1','logging_configuration'),
            ('fileStatus=0 countsBeforeThisLine','fileStatus=1 countsBeforeThisLine','logging_status')]:
            file.write_text((header(2)+clean+status(2)).replace(before,after));assert not analyze(file)['checks'][key]
        file.write_text(clean+status(2));assert not analyze(file)['checks']['logging_configuration']
        file.write_text(header(1)+clean+status(1));assert not analyze(file)['recorded_integrity_pass']
        file.write_text(header(2)+header(2)+clean+status(2));assert not analyze(file)['checks']['logging_configuration']
    for name in ['startup_150_startup_profile150.log','startup_280_startup_profile280.log','startup_320_startup_profile320.log','startup_150_startup_log-minimal.log','startup_280_startup_log-full.log']:
        r=analyze(OUT/name);assert r['checks']['logging_configuration'] and not r['recorded_integrity_pass']
    result=dict(result='passed',asi=str(ASI.relative_to(ROOT)),bytes=ASI.stat().st_size,sha256=digest(ASI),fixture_processes=20,logging_default=1,
        inherited_audio_sites_checked=len(native_tables),native_tables=native_tables,imports=libraries,parser_levels=parser_cases,legacy_parser_negative_cases=13,r9_preserved=True,live_r10_gameplay=False,packaged=False,published=False,
        filtered_bytes=manifest['filtered_bytes'])
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k!='native_tables'},indent=2))
if __name__=='__main__':main()

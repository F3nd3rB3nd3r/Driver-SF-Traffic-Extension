"""Pin R8's inherited capacity getter to native regression and the compiled ASI."""
import hashlib,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from analyze_generic_limits_log import analyze
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/mission_capacity_r8_2026-11-16'
ASI=ROOT/'build/v29s_usercap_r8/DriverSF_Unlimiter_USERCAP_R8.asi'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    fixture=json.loads((OUT/'fixture_manifest.json').read_text())
    assert fixture['result']=='passed' and fixture['processes']==10
    for name,h in fixture['dependencies'].items():assert digest(ROOT/name)==h,('stale fixture',name)
    assert digest(ROOT/'build/mission_capacity_r8_fixtures/mission.exe')==fixture['fixture_sha256']
    runs=json.loads((OUT/'fixtures.json').read_text());assert len(runs)==10 and all(r['exit']==0 for r in runs)
    repro=next(r for r in runs if r['requests']==280 and not r['mode'])
    assert '280 traffic + mission123 rejected against byte255' in repro['stdout'] and 'models=290' in repro['stdout']
    inherited=0
    for name,count in [('wide_limits',19),('traffic_queues',6),('stream_handlers',11)]:
        parent=json.loads((ROOT/f'analysis/{name}_r8_2026-11-16/build_verification.json').read_text())
        assert parent['result']=='passed' and parent['sha256']==digest(ASI)
        inherited+=count
    pe=PE(ASI)
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines()
             if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    target=at('?WideCapacityQuery@');end=min(v for v in symbols.values() if v>target);code=pe.read(target,end-target)
    (OUT/'WideCapacityQuery.compiled.asm.txt').write_text(subprocess.run([str(DUMPBIN),'/nologo','/disasm:bytes',f'/range:0x{target:X},0x{end-1:X}',str(ASI)],capture_output=True,text=True,check=True).stdout)
    sig=bytes.fromhex('8b81f87200008b4c24040fb6440818c20400')
    assert pe.read(at('?kWideCapacitySig@'),18)==sig
    sites=struct.unpack('<6I',pe.read(at('?g_sites@'),24))
    assert sites==(0x5A0AD0,at('?kWideCapacitySig@'),6,target,0,at('?g_wideCapacityOriginal@'))
    assert b'\x3b\x35'+struct.pack('<I',at('?g_pkg0@')) in code,'active package identity check'
    assert b'\x83\x7d\x08\x00' in code,'subtype0 check'
    assert b'\xa1'+struct.pack('<I',at('?g_limits@@')+36) in code,'full-width ModelSlots load'
    assert b'\xa1'+struct.pack('<I',at('?g_wideCapacityOriginal@')) in code and b'\xff\xd0' in code,'native trampoline fallback'
    assert code.count(b'\xc2\x04\x00')==2,'both paths preserve native callee cleanup'
    for marker in [b'DSFV29s_usercap_r8_',b'DSFV29s_CAPACITY_QUERY_START',b'DSFV29s_CAPACITY_QUERY_STATUS',b'admissionCompare=native']:
        assert marker in pe.data
    startups=[]
    for label in ['150','700','1000','customized']:
        sample=ROOT/f'analysis/wide_limits_r8_2026-11-16/startup_{label}.log'
        result=analyze(sample)
        assert result['checks']['capacity_query_start'] and not result['checks']['capacity_query_status'] and not result['recorded_integrity_pass']
        startups.append(dict(label=label,query_hook=True,gameplay=False))
    # Production-named logs fail closed even if the new record is absent.
    sample=ROOT/'analysis/wide_limits_r8_2026-11-16/startup_150.log'
    missing=OUT/'DSFV29s_usercap_r8_missing_capacity.log'
    missing.write_text('\n'.join(l for l in sample.read_text().splitlines() if not l.startswith('DSFV29s_CAPACITY_QUERY_START'))+'\n')
    assert not analyze(missing)['checks']['capacity_query_start']
    broken=OUT/'DSFV29s_usercap_r8_bad_capacity.log'
    broken.write_text(sample.read_text().replace('CAPACITY_QUERY_START revision=1 tick=0 status=1 modelSlots=160','CAPACITY_QUERY_START revision=1 tick=0 status=1 modelSlots=255'))
    assert not analyze(broken)['checks']['capacity_query_start']
    # Historical R6 logs remain in their schema; no new record required.
    previous=analyze(ROOT/'dist/USERCAP_R6/DSFV29s_usercap_r6_36080.log')
    assert previous['recorded_integrity_pass'] and 'capacity_query_start' not in previous['checks']
    before=json.loads((ROOT/'analysis/mission_capacity_r7_2026-11-15/before_r7_manifest.json').read_text())
    for r in before['preserved_artifacts']:assert digest(ROOT/r['path'])==r['sha256'],('R6 artifact changed',r['path'])
    result=dict(result='passed',asi=str(ASI.relative_to(ROOT)),bytes=ASI.stat().st_size,sha256=digest(ASI),
        focused_processes=len(runs),inherited_processes=inherited,total_processes=len(runs)+inherited,
        native_getter='005A0AD0',stolen_bytes=6,validated_signature_bytes=18,full_width_ModelSlots=True,
        abi='fastcall wrapper for thiscall ret4, nonvolatile registers and stack verified by native fixture',
        startups=startups,historical_R6_unchanged=True,scope='offline native slices and compiled hooks; live loading/rendering/audio pending')
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()

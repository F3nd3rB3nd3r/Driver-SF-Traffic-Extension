"""Pin the R8 manager-key correction to its native tests and compiled PE32."""
import hashlib,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,EXE,DUMPBIN
from analyze_generic_limits_log import analyze
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/manager_keys_r8_2026-11-16'
ASI=ROOT/'build/v29s_usercap_r8/DriverSF_Unlimiter_USERCAP_R8.asi'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    fixture=json.loads((OUT/'fixture_manifest.json').read_text())
    assert fixture['result']=='passed' and fixture['processes']==10
    for name,h in fixture['dependencies'].items():assert digest(ROOT/name)==h,('stale fixture',name)
    assert digest(ROOT/'build/manager_keys_r8_fixtures/keys.exe')==fixture['fixture_sha256']
    runs=json.loads((OUT/'fixtures.json').read_text());assert len(runs)==10
    for r in runs:
        expected='timeout' if r['mode']=='captured-hang' else 0x29F3 if r['mode'] in ('full','guard','cycle') else 0
        assert r['exit']==expected
    assert 'overwrite256-node sentinel' in runs[0]['stdout']
    parent=json.loads((ROOT/'analysis/mission_capacity_r8_2026-11-16/build_verification.json').read_text())
    assert parent['result']=='passed' and parent['total_processes']==46 and parent['sha256']==digest(ASI)
    pe=PE(ASI);native=PE(EXE)
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(address):return pe.read(address,min(v for v in symbols.values() if v>address)-address)
    def resolve(a):
        for _ in range(8):
            try:opcode=pe.read(a,1)
            except ValueError:return a # raw E8 may be an immediate byte, not a call.
            if opcode!=b'\xe9':return a
            a=(a+5+struct.unpack('<i',pe.read(a+1,4))[0])&0xFFFFFFFF
        raise AssertionError('jump chain')
    def calls(a,target):
        code=body(a)
        return any(code[i]==0xE8 and resolve((a+i+5+struct.unpack_from('<i',code,i+1)[0])&0xFFFFFFFF)==target for i in range(len(code)-4))
    contracts=json.loads((OUT/'native_contracts.json').read_text());table=at('?kManagerKeyContracts@')
    for i,r in enumerate(contracts):
        assert struct.unpack('<3I',pe.read(table+12*i,12))==(r['address'],r['length'],r['fnv'])
        h=2166136261
        for v in native.read(r['address'],r['length']):h=((h^v)*16777619)&0xFFFFFFFF
        assert h==r['fnv']
    bridges=[]
    for index,label in enumerate(['Insert','Lookup','Destroy']):
        a=symbols['_ManagerKey'+label+'Bridge'];code=body(a)
        for pattern in ['9c60','0fae0424','0fae0c24','fc','8be5619d']:assert bytes.fromhex(pattern) in code,(label,pattern)
        assert calls(a,symbols['_ManagerKey'+label+'Entry'])
        assert b'\xff\x25'+struct.pack('<I',at('?g_managerKeyTramps@')+4*index) in code
        bridges.append(dict(name=label,flags_registers_DF_FX_preserved=True))
    install=body(at('?ManagerKeyInstall@'))
    for target in [0x59E120,0x59E0D0,0x59D566]:assert struct.pack('<I',target) in install
    assert calls(at('?ManagerKeyInstall@'),at('?DiagFInstallDetour@')) and calls(at('?ManagerKeyInstall@'),at('?DiagFRestoreDetour@'))
    construct=body(at('?StreamConstruct@'))
    for prefix in ['?g_managerKeyStatus@','?g_managerKeyOwners@','?g_managerKeyGenerations@']:assert struct.pack('<I',at(prefix)) in construct
    for marker in [b'DSFV29s_usercap_r8_',b'DSFV29s_MANAGER_KEY_START',b'DSFV29s_MANAGER_KEY_STATUS',b'DSFV29s_MANAGER_KEY_FATAL',b'activeCycleOrForeignNode',b'nativeSentinels=1',b'managerKeyEntries=']:
        assert marker in pe.data,marker
    startups=[]
    for label in ['150','700','1000','customized']:
        result=analyze(ROOT/f'analysis/wide_limits_r8_2026-11-16/startup_{label}.log')
        assert result['checks']['manager_key_start'] and not result['checks']['manager_key_status'] and not result['recorded_integrity_pass']
        startups.append(dict(label=label,manager_keys_installed=True,gameplay=False))
    sample=(ROOT/'analysis/wide_limits_r8_2026-11-16/startup_150.log').read_text()
    missing=OUT/'DSFV29s_usercap_r8_missing_key.log';missing.write_text('\n'.join(l for l in sample.splitlines() if not l.startswith('DSFV29s_MANAGER_KEY_START'))+'\n')
    assert not analyze(missing)['checks']['manager_key_start']
    before=json.loads((OUT/'before_r8_manifest.json').read_text())
    assert digest(ROOT/'dist/USERCAP_R7/DriverSF_Unlimiter_USERCAP_R7.asi')==before['r7_asi_sha256']
    result=dict(result='passed',asi=str(ASI.relative_to(ROOT)),bytes=ASI.stat().st_size,sha256=digest(ASI),focused_processes=10,inherited_processes=46,total_expected_outcomes=56,
        native_contracts=len(contracts),bridges=bridges,lists_per_manager=14,native_capacity=256,
        capacity_formula='max(512, ModelSlots + ReplayRetainedReferences)',example280_capacity=802,
        startups=startups,r7_artifact_preserved=True,scope='offline native list/hook/lifetime tests and compiled cold startup; live Getaway1 return and performance pending')
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()

"""Check USER CAP R8 PE/defaults/fixtures and preserve the exact R1 fallback."""
import hashlib,json,re,struct,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN,EXE
from analyze_generic_limits_log import analyze
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/wide_limits_r8_2026-11-16'
ASI=ROOT/'build/v29s_usercap_r8/DriverSF_Unlimiter_USERCAP_R8.asi'
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
    fixture=json.loads((OUT/'fixture_manifest.json').read_text())
    for name,value in fixture['dependencies'].items():assert digest(ROOT/name)==value,('stale fixture',name)
    assert digest(ROOT/'build/wide_limits_r8_fixtures/wide.exe')==fixture['fixture_sha256']
    runs=json.loads((OUT/'fixtures.json').read_text());assert len(runs)==19
    assert all(r['exit']==(0x29F3 if r.get('mode') in ('collection-full','allocation-failure','byte-overflow','completed-full','completed-misaligned','completed-foreign') else 0) for r in runs)
    baseline=ROOT/'analysis/wide_limits_r2_2026-11-15/baseline';manifest=json.loads((baseline/'manifest.json').read_text())
    for item in manifest:
        p=baseline/item['path'];assert p.stat().st_size==item['bytes'] and digest(p)==item['sha256'],item['path']
    for name,expected in [('DriverSF_Unlimiter_GENERIC_R1.asi','a8c2915aad6c3791410891be78d9066f68d35b5ab8e5a32185b2bf055c05722b'),('DriverSF_Unlimiter.ini','2a0851edff6aa31dee19cfbc230319b345b27eabefb4e7cf0358cc51fd5315f4')]:
        assert digest(ROOT/'dist/MAIN_150'/name)==expected,('fallback changed',name)
    r2_manifest=json.loads((ROOT/'dist/USERCAP_R2/manifest.json').read_text())
    for item in r2_manifest['files']:assert digest(ROOT/item['path'])==item['sha256'],('R2 artifact changed',item['path'])
    assert digest(EXE)=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    pe=PE(ASI);assert pe.characteristics&0x2000 and struct.unpack_from('<H',pe.data,pe.pe+4)[0]==0x14C
    project=ET.parse(ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj');defs=project.find('.//{*}PreprocessorDefinitions').text
    assert 'DSF_V29S_WIDE_LIMITS=1' in defs and 'NATIVE_FIXTURE' not in defs
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    at=lambda prefix:next(v for k,v in symbols.items() if k.startswith(prefix))
    assert not any('kChapter7AllPositiveIds' in k or 'WideTestMain' in k or 'FixtureFactory' in k for k in symbols)
    defaults=struct.unpack('<7IB3xI9I',pe.read(at('?g_limits@@'),72))
    assert defaults[:8]==(150,192,2048,64,1,256,1,1)
    assert defaults[9:]==(160,512,512,512,512,262144,131072,262144,45000)
    identity=pe.read(at('?VerifyExecutableIdentity@'),0x100)
    assert struct.pack('<I',0x400000) in identity and struct.pack('<I',0x20000000) not in identity,'fixture identity escaped into ASI'
    entry=struct.unpack_from('<I',pe.data,pe.pe+24+16)[0]+pe.base
    resolved=entry+5+struct.unpack('<i',pe.read(entry+1,4))[0] if pe.read(entry,1)==b'\xe9' else entry
    assert resolved==symbols['_DllMain@12']
    hooks={}
    for name,size in [('_WideAdmissionBridge',32),('_WideRequestFrameBridge',32),('_WideNotifyFrameBridge',32),('_WideProbeStack',96),('_WideCollectionBridge',48)]:
        address=symbols[name];data=pe.read(address,size);hooks[name]=dict(address=hex(address),bytes=data.hex())
        r=subprocess.run([str(DUMPBIN),'/nologo','/disasm:bytes',f'/range:0x{address:X},0x{address+size-1:X}',str(ASI)],capture_output=True,text=True,check=True)
        (OUT/(name.strip('_')+'.compiled.asm.txt')).write_text(r.stdout)
    assert bytes.fromhex(hooks['_WideProbeStack']['bytes']).startswith(b'\x9c\x60')
    for key in ('_WideRequestFrameBridge','_WideNotifyFrameBridge'):
        data=bytes.fromhex(hooks[key]['bytes']);assert data[0]==0x50 and b'\x58\x2b\x25' in data and b'\xff\x25' in data
    assert bytes.fromhex(hooks['_WideCollectionBridge']['bytes'])[0]==0x9C
    imports=subprocess.run([str(DUMPBIN),'/nologo','/imports',str(ASI)],capture_output=True,text=True,check=True).stdout
    (OUT/'imports.txt').write_text(imports)
    libraries=re.findall(r'^\s+([\w.-]+\.dll)\s*$',imports,re.I|re.M)
    assert {v.lower() for v in libraries}=={'kernel32.dll','psapi.dll'},libraries
    # The old supplied live log must still pass the R1 analyzer branch.
    previous=analyze(ROOT/'dist/MAIN_150/DSFV29s_generic_limits_10556.log')
    assert previous['recorded_integrity_pass'],previous['checks']
    startups=[]
    start_keys=('audio_model_start','audio_slots_start','audio_limit_start','replay_read_start','notify150_start','parked150_start','replay150_start','pending_start','callback_start','regmap_start','indexed_start','cleanup_start')
    for label in ('150','700','1000','customized'):
        result=analyze(OUT/('startup_'+label+'.log'))
        for key in ('configuration','layout','configured_queue_geometry','no_fatal',*start_keys):assert result['checks'][key],(label,key,result['checks'])
        assert not result['recorded_integrity_pass'],'startup-only fixture must never masquerade as gameplay'
        startups.append(dict(label=label,checks={k:result['checks'][k] for k in ('configuration','layout','configured_queue_geometry',*start_keys)}))
    result=dict(result='passed',asi=str(ASI.relative_to(ROOT)),bytes=ASI.stat().st_size,sha256=digest(ASI),fixture_processes=len(runs),baseline_files_verified=len(manifest),native_sha256=digest(EXE),imports=libraries,compiled_hooks=hooks,old_log_analyzer_pass=True,startup_analyzer_checks=startups,scope='offline native slices/storage/startup; no gameplay')
    result['source_sha256']=fixture['dependencies']
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('result','bytes','sha256','fixture_processes','baseline_files_verified','imports','old_log_analyzer_pass')},indent=2))
if __name__=='__main__':main()

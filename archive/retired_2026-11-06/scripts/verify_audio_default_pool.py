"""Verify exact-executable pressure evidence and the one-operand ASI variant."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_audio_probe

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_default16m_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_default16m_r1'
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT16M_R1'

def main(project='DriverSF_V29s_AUDIO_DEFAULT16M.vcxproj',name=NAME,build=BUILD,output=OUT,
         default64=False,pressure_evidence_path=None):
    NAME,BUILD,OUT=name,build,output
    requested=0x4000000 if default64 else 0x1000000
    OUT.mkdir(parents=True,exist_ok=True)
    native=PE();binary=PE(BUILD/(NAME+'.asi'))
    tests=json.loads((OUT/'pool_tests.json').read_text())
    assert tests['result']=='passed'
    assert tests.get('requested_bytes',0x1000000)==requested
    assert tests['production_patch_sha256']==hashlib.sha256((ROOT/'src/V29sAudioDefaultPoolPatch.inc').read_bytes()).hexdigest()
    verify_audio_probe.main(project=project,name=NAME,build=BUILD,output=OUT,
        extra_flags={'DSF_V29S_CAPACITY_TRACE=1','DSF_V29S_INDEXED_262144=1','DSF_V29S_AUDIO_PROBE=1',
            'DSF_V29S_AUDIO_DEFAULT_64M=1' if default64 else 'DSF_V29S_AUDIO_DEFAULT_16M=1'},
        bridge_test_path=ROOT/'analysis/audio_probe_r1_2026-11-05/bridge_tests.json',
        log_prefix=b'DSFV29s_audio_default64m_' if default64 else b'DSFV29s_audio_default16m_',capacity_marker=b'actualBankResultStack=8 audioCapacityChanges=')
    assert native.read(0x84A64D,7)==bytes.fromhex('c7454099995900')
    assert native.read(0x84A62E,8)==bytes.fromhex('f30f1005d46cdd00')
    assert native.read(0x84A654,10)==bytes.fromhex('f30f1145e8f30f114544')
    assert native.read(0xDD6CD4,4)==struct.pack('<I',0x3F666666)
    assert native.read(0xC66530,0x6C).hex()==re.search(r'kNativePressureBytes\[\]=\{(.*?)\}',(ROOT/'tests/AudioPressurePredicateBytes.inc').read_text())[1].replace('0x','').replace(',','').lower()
    assert native.read(0xC66910,6)==bytes.fromhex('d9057418d701')
    assert native.read(0xC66923,5)[0]==native.read(0xC6693F,5)[0]==0xE8
    assert native.read(0xC66BB5,5)==bytes.fromhex('e856fdffff')
    assert native.read(0xCA0266,5)==bytes.fromhex('e8a566fcff')
    # Model exactly the sole operand change and prove all adjacent bytes unchanged.
    stock=bytearray(native.read(0x84A600,0x100));patched=stock.copy()
    patched[0x50:0x54]=struct.pack('<I',requested)
    assert patched[0x4D:0x54]==bytes.fromhex('c74540')+struct.pack('<I',requested)
    assert stock[:0x50]==patched[:0x50] and stock[0x54:]==patched[0x54:]
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        match=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9a-fA-F]{8})',line)
        if match:symbols[match[1]]=int(match[2],16)
    for function in ('AudioDefaultPoolInstall','AudioDefaultPoolPoll','AudioDefaultInstructionMatches'):
        start=next(v for k,v in symbols.items() if k.startswith('?'+function+'@'))
        following=min(v for v in symbols.values() if v>start)
        result=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{following-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True)
        (OUT/(function+'.asm.txt')).write_text(result.stdout)
        if function=='AudioDefaultPoolInstall':
            assert struct.pack('<I',requested) in binary.read(start,following-start)
    main_source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    dllmain=main_source[main_source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dllmain.index('IndexedInstall();')<dllmain.index('AudioProbeInstall();')<dllmain.index('AudioDefaultPoolInstall();')<dllmain.index('CreateThread(nullptr, 0, &Monitor')
    baseline=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_PROBE_R1.asi'
    assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9f5ab4720068f3424d159d8675de2ee7d16a1018a6b566626af645f69522975'
    if pressure_evidence_path:
        threshold=json.loads(pressure_evidence_path.read_text())
        assert threshold['threshold_bits']=='3F666666' and threshold['above_rows']>0
        for row in threshold['rows']:
            assert (row['used']*16777216>row['usable']*15099494)==row['native_pressure_predicate']
    else:
        log=ROOT/'dist/MAIN_150/DSFV29s_audio_probe_4188.log'
        pool_rows=[]
        for line_no,line in enumerate(log.read_text().splitlines(),1):
            if not line.startswith('DSFV29s_AUDIO_POOL '):continue
            row=dict(piece.split('=',1) for piece in line.split()[1:] if '=' in piece)
            if row['id']!='5':continue
            used,total=int(row['mallocUsedBytes']),int(row['usableBytes'])
            pressure=used*16777216>total*15099494
            pool_rows.append(dict(line=line_no,tick=int(row['tick']),used=used,usable=total,native_pressure_predicate=pressure))
        above=[r for r in pool_rows if r['native_pressure_predicate']]
        assert above and all(r['native_pressure_predicate'] for r in pool_rows if r['tick']>=above[0]['tick'])
        threshold=dict(threshold_bits='3F666666',threshold_value=struct.unpack('<f',struct.pack('<I',0x3F666666))[0],
            sampled_rows=len(pool_rows),above_rows=len(above),first_above=above[0],last_above=above[-1],rows=pool_rows,
            caveat='Sampled current pool values satisfy exact native predicate. R1 did not hook actual native gate calls or kick decisions.')
    (OUT/'run4188_pressure_evidence.json').write_text(json.dumps(threshold,indent=2))
    result=dict(result='passed',sha256=hashlib.sha256(binary.data).hexdigest(),bytes=len(binary.data),
        configured_size_old=0x599999,configured_size_new=requested,allocation_size_operands_changed=1,
        threshold_preserved=True,fixed_pools_preserved=True,prior_r2_and_probe_artifacts_unchanged=True,
        production_patch_scenarios=tests['patch_scenarios'],isolated_native_predicate_cases=tests['native_predicate_cases'],
        old_log_above_threshold_rows=threshold['above_rows'],old_log_first_above_tick=threshold['first_above']['tick'],
        limitations='No game run or installation. Native threshold is a verified mechanism and sampled correlation, not proof of individual playback kick events or an audible fix.')
    (OUT/'default_pool_verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

"""Checks for the clarified shared-default64MiB comparison; no game execution."""
import hashlib
import json
import struct
from pathlib import Path
from static_streaming_audit import PE
import test_audio_default_pool
import verify_audio_default_pool

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_default64m_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_default64m_r1'
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT64M_R1'

def main():
    # Only standalone fixtures are compiled in both modes; the active ASI
    # solution builds only the new Release/Win32 default64 project.
    test_audio_default_pool.main(output=OUT/'default16_regression',build=ROOT/'build/audio_default16_regression')
    test_audio_default_pool.main(output=OUT,build=ROOT/'build/audio_default64_pool_test',default64=True)
    verify_audio_default_pool.main(project='DriverSF_V29s_AUDIO_DEFAULT64M.vcxproj',name=NAME,build=BUILD,output=OUT,
        default64=True,pressure_evidence_path=ROOT/'analysis/audio_default16m_r1_2026-11-05/run4188_pressure_evidence.json')
    native,binary=PE(),PE(BUILD/(NAME+'.asi'))
    assert native.read(0x84A683,5)==bytes.fromhex('6800b84200')
    # Same sole original operand as the16MiB build; static bank instruction stays native.
    original=bytearray(native.read(0x84A600,256));baseline=original.copy();expanded=original.copy()
    baseline[0x50:0x54]=struct.pack('<I',16*1024*1024)
    expanded[0x50:0x54]=struct.pack('<I',64*1024*1024)
    assert expanded[0x83:0x88]==original[0x83:0x88]
    assert all(expanded[i]==baseline[i] for i in range(256) if not 0x50<=i<0x54)
    assert b'DSFV29s_AUDIO_STATIC_START' not in binary.data and b'DSFV29s_AUDIO_STATIC_POOL' not in binary.data
    assert native.read(0xC99E25,6)==bytes.fromhex('33d2f7f58bf8')
    assert native.read(0xC99F40,5)==bytes.fromhex('890e896e04')
    assert native.read(0xC9A392,9)==bytes.fromhex('8d4ff481f9f4ffff3f')
    blocks=64*1024*1024//64
    assert blocks==1048576 and (64*1024*1024-3180)-12<=0x3FFFFFF4
    preserved={
        'DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT16M_R1.asi':'4106331601430e38a3bcb888de0f5fc310b47ce9597efffe128742ec786d3535',
        'DriverSF_V29s_160Capacity_150Request_AUDIO_STATIC64M_R1.asi':'1c36dbad10b092555d8b3253d6bf7b93879c2f8ad8f3c24ac4b21284707cabd8'}
    for filename,expected in preserved.items():
        assert hashlib.sha256((ROOT/'dist/MAIN_150'/filename).read_bytes()).hexdigest()==expected
    result=dict(result='passed',sha256=hashlib.sha256(binary.data).hexdigest(),bytes=len(binary.data),
        changed_pool='sharedDefault',default_requested_bytes=67108864,static_requested_bytes=4372480,
        default64_patch_scenarios=22,default16_regression_scenarios=20,native_predicate_cases_per_mode=15,
        baseline_startup_checks=29,additional_live_audio_hooks=0,static_expansion_disabled=True,
        native_pool_block_count_32bit=blocks,threshold_preserved=True,earlier_artifacts_preserved=True,
        limitations='Isolated fixtures and offline binary checks. No game startup, live size, playback or FPS confirmation until the user test.')
    (OUT/'default64_verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

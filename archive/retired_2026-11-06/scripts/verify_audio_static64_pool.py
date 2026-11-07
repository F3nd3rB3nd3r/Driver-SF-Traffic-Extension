"""Verify the requested StaticBanksPool64MiB comparison and its baseline."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_audio_probe

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_static64m_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_static64m_r1'
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_STATIC64M_R1'

def main():
    native=PE();binary=PE(BUILD/(NAME+'.asi'))
    tests=json.loads((OUT/'static_pool_tests.json').read_text())
    assert tests['result']=='passed' and tests['scenarios']==29
    assert tests['production_static_patch_sha256']==hashlib.sha256((ROOT/'src/V29sAudioStatic64Patch.inc').read_bytes()).hexdigest()
    assert tests['production_default_patch_sha256']==hashlib.sha256((ROOT/'src/V29sAudioDefaultPoolPatch.inc').read_bytes()).hexdigest()
    default_tests=json.loads((ROOT/'analysis/audio_default16m_r1_2026-11-05/pool_tests.json').read_text())
    assert default_tests['result']=='passed' and default_tests['production_patch_sha256']==tests['production_default_patch_sha256']
    verify_audio_probe.main(project='DriverSF_V29s_AUDIO_STATIC64M.vcxproj',name=NAME,build=BUILD,output=OUT,
        extra_flags={'DSF_V29S_CAPACITY_TRACE=1','DSF_V29S_INDEXED_262144=1','DSF_V29S_AUDIO_PROBE=1','DSF_V29S_AUDIO_DEFAULT_16M=1','DSF_V29S_AUDIO_STATIC_64M=1'},
        bridge_test_path=ROOT/'analysis/audio_probe_r1_2026-11-05/bridge_tests.json',
        log_prefix=b'DSFV29s_audio_static64m_',capacity_marker=b'actualBankResultStack=8 audioCapacityChanges=')
    assert native.read(0x84A683,5)==bytes.fromhex('6800b84200')
    assert native.read(0x84A67B,8)==bytes.fromhex('680004000083cbff')
    assert native.read(0x84A696,5)==bytes.fromhex('e8f5f64400')
    assert native.read(0x84A64D,7)==bytes.fromhex('c7454099995900')
    assert native.read(0xDD6CD4,4)==struct.pack('<I',0x3F666666)
    stock=bytearray(native.read(0x84A600,256));baseline=stock.copy();baseline[0x50:0x54]=struct.pack('<I',0x1000000)
    experiment=baseline.copy();experiment[0x84:0x88]=struct.pack('<I',0x4000000)
    assert experiment[0x83:0x88]==bytes.fromhex('6800000004')
    assert all(experiment[i]==baseline[i] for i in range(256) if not 0x84<=i<0x88)
    # Dynamic CreatePool divides requested bytes using full-width arithmetic,
    # passes rounded bytes into the heap, and retains a DWORD block count.
    assert native.read(0xC99E25,6)==bytes.fromhex('33d2f7f58bf8')
    assert native.read(0xC99F40,5)==bytes.fromhex('890e896e04')
    assert native.read(0xC9A385,6)==bytes.fromhex('81c794f3ffff')
    assert native.read(0xC9A392,9)==bytes.fromhex('8d4ff481f9f4ffff3f')
    requested=64*1024*1024;block_bytes=1024;blocks=requested//block_bytes
    assert blocks==65536 and blocks*block_bytes==requested
    heap_bytes=(requested-3180)&~3
    assert heap_bytes-12<=0x3FFFFFF4 # Native heap-size acceptance check, not an allocation guarantee.
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        match=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9a-fA-F]{8})',line)
        if match:symbols[match[1]]=int(match[2],16)
    for function in ('AudioStaticPoolInstall','AudioStaticPoolPoll','AudioStaticInstructionMatches','AudioDefaultPoolInstall'):
        start=next(v for k,v in symbols.items() if k.startswith('?'+function+'@'))
        following=min(v for v in symbols.values() if v>start)
        result=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{following-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True)
        (OUT/(function+'.asm.txt')).write_text(result.stdout)
        if function.endswith('Install'):
            expected=0x4000000 if function.startswith('AudioStatic') else 0x1000000
            assert struct.pack('<I',expected) in binary.read(start,following-start)
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    dllmain=source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dllmain.index('IndexedInstall();')<dllmain.index('AudioProbeInstall();')<dllmain.index('AudioDefaultPoolInstall();')<dllmain.index('AudioStaticPoolInstall();')<dllmain.index('CreateThread(nullptr, 0, &Monitor')
    baseline_asi=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT16M_R1.asi'
    assert hashlib.sha256(baseline_asi.read_bytes()).hexdigest()=='4106331601430e38a3bcb888de0f5fc310b47ce9597efffe128742ec786d3535'
    assert b'DSFV29s_AUDIO_STATIC_START' in binary.data and b'playbackGateAppliesToThisPool=0' in binary.data
    summary=dict(result='passed',sha256=hashlib.sha256(binary.data).hexdigest(),bytes=len(binary.data),
        comparison_baseline='run6736 default16MiB',static_requested_old=4372480,static_requested_new=requested,
        additional_operands_changed=1,additional_requested_bytes=requested-4372480,
        default_requested_preserved=16777216,threshold_preserved=True,fixed_object_limits_preserved=True,
        block_count_32bit=blocks,native_heap_size_check_passes=True,
        startup_order_verified=True,production_static_patch_scenarios=29,baseline_asi_unchanged=True,
        limitations='Offline compiled checks and production fake-image installer tests only. Real allocation/startup, audio behavior and FPS require the user comparison. StaticBanksPool is outside the verified90% playback gate.')
    (OUT/'static64_verification.json').write_text(json.dumps(summary,indent=2));print(json.dumps(summary,indent=2))

if __name__=='__main__':main()

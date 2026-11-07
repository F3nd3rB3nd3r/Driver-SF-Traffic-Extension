"""Focused offline validation of the built probe and its preserved R2 baseline."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN,EXE
import verify_indexed_capacity
import test_startup_checks
from workspace_paths import historical_path

ROOT=Path(__file__).resolve().parents[1]
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_PROBE_R1'
BUILD=ROOT/'build/v29s_audio_probe_r1'
OUT=ROOT/'analysis/audio_probe_r1_2026-11-05'
PROJECT='DriverSF_V29s_AUDIO_PROBE.vcxproj'
SITES=[('Bank',0x84BE74,'c7401404000000','ff7534ff7530ff752cff7528',16),
    ('Emitter',0x84AA29,'83bea00e00005a','ff7504',4),
    ('Malloc',0xC99AA5,'5e5b8bc75f','6a00ff7530ff7538ff75346a00',20),
    ('Malign',0xC99B04,'5e5b8bc75f','ff753cff7530ff7538ff75346a01',20),
    ('Post',0xC54E17,'8bc65e5f5b83c410','ff7540ff7548ff75446a00',16),
    ('PostEarly',0xC54DD3,'5f5b83c410','ff753cff7544ff75406a01',16)]

def main(project=PROJECT,name=NAME,build=BUILD,output=OUT,extra_flags=None,
         bridge_test_path=None,log_prefix=b'DSFV29s_audio_probe_',capacity_marker=b'audioCapacityChanges=0'):
    PROJECT,NAME,BUILD,OUT=project,name,build,output
    OUT.mkdir(parents=True,exist_ok=True)
    pe,native=PE(BUILD/(NAME+'.asi')),PE()
    tests=json.loads((bridge_test_path or OUT/'bridge_tests.json').read_text())
    assert tests['result']=='passed' and tests['production_bridges_sha256']==hashlib.sha256((ROOT/'src/V29sAudioProbeBridges.inc').read_bytes()).hexdigest()
    test_startup_checks.main(output=OUT,build=ROOT/'build/audio_probe_startup_checks')
    verify_indexed_capacity.main(project=PROJECT,name=NAME,build=BUILD,output=OUT,
        extra_flags=extra_flags or {'DSF_V29S_CAPACITY_TRACE=1','DSF_V29S_INDEXED_262144=1','DSF_V29S_AUDIO_PROBE=1'})
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        match=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9a-fA-F]{8})',line)
        if match:symbols[match[1]]=int(match[2],16)
    tramps=symbols['_g_audioProbeTramps']
    def resolve(at):
        for _ in range(4):
            try:b=pe.read(at,5)
            except ValueError:return None
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('thunk chain')
    inventory=next(v for k,v in symbols.items() if k.startswith('?kAudioProbeSites@'))
    for i,(name,at,signature,_,_) in enumerate(SITES):
        row=pe.read(inventory+i*20,20)
        target,length=struct.unpack_from('<II',row)
        assert target==at and length==len(bytes.fromhex(signature))
        assert row[8:8+length]==bytes.fromhex(signature)
        assert resolve(struct.unpack_from('<I',row,16)[0])==symbols[f'_AudioProbe{name}Bridge']
    bridges=[]
    for index,(name,at,signature,args,cleanup) in enumerate(SITES):
        stock=bytes.fromhex(signature);assert native.read(at,len(stock))==stock
        start=symbols[f'_AudioProbe{name}Bridge']
        following=min(v for v in symbols.values() if v>start)
        code=pe.read(start,following-start)
        assert code[0]==0x9C
        for pattern in ('0fae0424','0fae0c24','83e4f0','619d'):
            assert bytes.fromhex(pattern) in code,(name,pattern)
        assert bytes.fromhex(args) in code,(name,'argument offsets')
        assert b'\x83\xC4'+bytes([cleanup])+bytes.fromhex('0fae0c24') in code
        assert b'\x9d\xff\x25'+struct.pack('<I',tramps+4*index) in code
        helper_name='Alloc' if name in ('Malloc','Malign') else 'Post' if name=='PostEarly' else name
        target=symbols[f'_AudioProbe{helper_name}Entry']
        calls=[]
        for offset in range(len(code)-4):
            if code[offset]==0xE8 and resolve(start+offset+5+struct.unpack_from('<i',code,offset+1)[0])==target:
                calls.append(offset)
        assert len(calls)==1,(name,calls)
        if name in ('Malloc','Malign'):
            # Successful pointer paths skip all pushad/FXSAVE/C++ work.
            assert code[:4]==bytes.fromhex('9c85ff75')
            destination=5+struct.unpack('b',code[4:5])[0]
            assert code[destination:destination+3]==bytes.fromhex('9dff25')
        result=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{following-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True)
        (OUT/(name.lower()+'_audio_bridge.asm.txt')).write_text(result.stdout)
        bridges.append(dict(name=name,site=f'{at:08X}',bytes=signature,compiled_arguments_verified=True,helper_calls=len(calls)))
    # Near control transfers anywhere in .text, plus all candidate short branches
    # within reach. Byte scanning overapproximates instructions; review any hits.
    text=next(s for s in native.sections if s['name']=='.text')
    block=native.data[text['offset']:text['offset']+text['size']]
    candidates=[]
    for i in range(len(block)-6):
        op=block[i];pc=text['va']+i
        if op in (0xE8,0xE9):dest=pc+5+struct.unpack_from('<i',block,i+1)[0];kind='near'
        elif op==0x0F and 0x80<=block[i+1]<=0x8F:dest=pc+6+struct.unpack_from('<i',block,i+2)[0];kind='near'
        elif op==0xEB or 0x70<=op<=0x7F or 0xE0<=op<=0xE3:dest=pc+2+struct.unpack_from('b',block,i+1)[0];kind='short'
        else:continue
        for name,at,signature,_,_ in SITES:
            if at<dest<at+len(bytes.fromhex(signature)):
                candidates.append(dict(source=f'{pc:08X}',destination=f'{dest:08X}',kind=kind,site=name))
    (OUT/'incoming_branch_candidates.json').write_text(json.dumps(candidates,indent=2))
    # Decode each complete containing native function from its known entry.
    decoded={}
    for first,last in ((0x84BE70,0x84BE7C),(0x84AA20,0x84AAC8),(0xC99A40,0xC99AAA),(0xC99AB0,0xC99B09),(0xC54DA0,0xC54E20)):
        result=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{first:X},0x{last:X}',str(EXE)],capture_output=True,text=True,check=True)
        (OUT/f'native_{first:08X}.asm.txt').write_text(result.stdout)
        for line in result.stdout.splitlines():
            match=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
            if match:decoded[int(match[1],16)]=match[3]
    unresolved=[];false_positives=[]
    for row in candidates:
        source=int(row['source'],16)
        if source in decoded and not re.match(r'(?:j\w+|call|loop\w*)\s',decoded[source]):false_positives.append(row)
        elif source not in decoded and any(a<=source<=b for a,b in ((0x84BE70,0x84BE7C),(0x84AA20,0x84AAC8),(0xC99A40,0xC99AAA),(0xC99AB0,0xC99B09),(0xC54DA0,0xC54E20))):false_positives.append(row)
        else:unresolved.append(row)
    assert not unresolved,f'Review incoming branches: {unresolved}'
    main=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    dllmain=main[main.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dllmain.index('IndexedInstall();')<dllmain.index('AudioProbeInstall();')<dllmain.index('CreateThread(nullptr, 0, &Monitor')
    assert log_prefix in pe.data and capacity_marker in pe.data
    baseline=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_INDEXED_262144_R2.asi')
    assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='55eda80bf5972cf0333de3bf631ac6d145f529bc92026bdaa2789328d4398a46'
    result=dict(result='passed',sha256=hashlib.sha256(pe.data).hexdigest(),bytes=len(pe.data),bridges=bridges,
        native_branch_interior_hits=0,byte_scan_false_positives=false_positives,
        startup_regression_checks=29,synthetic_bridge_invocations=10,working_r2_binary_unchanged=True,
        limitations='Offline and isolated harness checks. No game execution, installation, live hook validation, audible-output or performance claim. Indirect entry aliases and OS patch failure paths are not exhaustively proven.')
    (OUT/'audio_verification.json').write_text(json.dumps(result,indent=2))
    print(json.dumps(result,indent=2))

if __name__=='__main__':main()

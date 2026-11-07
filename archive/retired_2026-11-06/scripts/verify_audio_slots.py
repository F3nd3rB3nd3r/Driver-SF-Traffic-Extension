"""Focused offline checks for the combined2048 test; never executes the game."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from audio_static_audit import EXPECTED
import verify_audio_probe

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_slots2048_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_slots2048_r1'
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_SLOTS2048_R1'
NAMES=('AGate','BGate','AInsert','BInsert','AReuse','Eight','Byte','AUpdate','BUpdate')

def main():
    native,binary=PE(),PE(BUILD/(NAME+'.asi'))
    assert hashlib.sha256(native.data).hexdigest()==EXPECTED
    fixture=json.loads((OUT/'fixture_results.json').read_text());assert fixture['result']=='passed'
    for name,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    verify_audio_probe.main(project='DriverSF_V29s_AUDIO_SLOTS2048.vcxproj',name=NAME,build=BUILD,output=OUT/'baseline',
        extra_flags={'DSF_V29S_CAPACITY_TRACE=1','DSF_V29S_INDEXED_262144=1','DSF_V29S_AUDIO_PROBE=1',
            'DSF_V29S_AUDIO_DEFAULT_64M=1','DSF_V29S_AUDIO_SLOTS_2048=1'},
        bridge_test_path=ROOT/'analysis/audio_probe_r1_2026-11-05/bridge_tests.json',
        log_prefix=b'DSFV29s_audio_slots2048_',capacity_marker=b'actualBankResultStack=8 audioCapacityChanges=')
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9a-fA-F]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def resolve(at):
        for _ in range(4):
            code=binary.read(at,5)
            if code[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',code,1)[0]
        raise AssertionError('Thunk chain')
    inventory=next(v for k,v in symbols.items() if k.startswith('?kAudioSlotsSites@'))
    decoded={}
    for filename in ('00C50F00-00C50FA0.asm.txt','00C6B160-00C6BB80.asm.txt'):
        for line in (OUT/filename).read_text().splitlines():
            m=re.match(r'\s*([0-9A-F]{8}):\s+((?:[0-9A-F]{2} )+)(.*)',line)
            if m:decoded[int(m[1],16)]=m[3]
    spans=[]
    for i,name in enumerate(NAMES):
        row=binary.read(inventory+i*24,24);site,length=struct.unpack_from('<II',row)
        assert native.read(site,length)==row[8:8+length]
        assert site in decoded and site+length in decoded
        assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',text) for at,text in decoded.items() if site<=at<site+length)
        address=symbols[f'_AudioSlots{name}Bridge'];assert resolve(struct.unpack_from('<I',row,20)[0])==address
        following=min(v for v in symbols.values() if v>address);code=binary.read(address,following-address)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','83c4080fae0c24','8be5619d'):
            assert bytes.fromhex(pattern) in code,(name,pattern)
        assert bytes([0x53 if i==5 else 0x57,0x6A,i]) in code,(name,'pointer register/kind')
        assert bytes.fromhex('ff25')+struct.pack('<I',symbols['_g_audioSlotsTramps']+4*i) in code
        calls=[j for j in range(len(code)-4) if code[j]==0xE8 and
            resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==symbols['_AudioSlotsEntry']]
        assert len(calls)==1
        spans.append(dict(name=name,site=f'{site:08X}',length=length,signature=row[8:8+length].hex()))
    # Overapproximating raw branch scan, resolved with instruction boundaries in
    # the complete containing native functions. Any outside candidate must be reviewed.
    text=next(s for s in native.sections if s['name']=='.text');block=native.data[text['offset']:text['offset']+text['size']]
    candidates=[];unresolved=[]
    for i in range(len(block)-6):
        op=block[i];pc=text['va']+i
        if op in (0xE8,0xE9):dest=pc+5+struct.unpack_from('<i',block,i+1)[0]
        elif op==0x0F and 0x80<=block[i+1]<=0x8F:dest=pc+6+struct.unpack_from('<i',block,i+2)[0]
        elif op==0xEB or 0x70<=op<=0x7F or 0xE0<=op<=0xE3:dest=pc+2+struct.unpack_from('b',block,i+1)[0]
        else:continue
        for span in spans+[dict(site='00C50F0C',length=5,name='Settings')]:
            at=int(span['site'],16)
            if at<dest<at+span['length']:
                r=dict(source=f'{pc:08X}',destination=f'{dest:08X}',site=span['name']);candidates.append(r)
                if pc not in decoded and not any(a<=pc<=b for a,b in ((0xC50F00,0xC50FA0),(0xC6B160,0xC6BB80))):unresolved.append(r)
                elif pc in decoded and re.match(r'(?:j\w+|call|loop\w*)\s',decoded[pc]):unresolved.append(r)
    (OUT/'slot_branch_candidates.json').write_text(json.dumps(dict(candidates=candidates,unresolved=unresolved),indent=2))
    assert not unresolved,unresolved
    assert native.read(0xC50F0C,5)==bytes.fromhex('b9ff000000')
    assert native.read(0xC50F17,6)==bytes.fromhex('894804894808')
    assert native.read(0x84A683,5)==bytes.fromhex('6800b84200')
    assert b'DSFV29s_AUDIO_STATIC_START' not in binary.data
    install=next(v for k,v in symbols.items() if k.startswith('?AudioSlotsInstall@'))
    end=min(v for v in symbols.values() if v>install)
    assert struct.pack('<I',2048) in binary.read(install,end-install)
    default=next(v for k,v in symbols.items() if k.startswith('?AudioDefaultPoolInstall@'))
    end=min(v for v in symbols.values() if v>default)
    assert struct.pack('<I',67108864) in binary.read(default,end-default)
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    dll=source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('AudioProbeInstall();')<dll.index('AudioDefaultPoolInstall();')<dll.index('AudioSlotsInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    previous=ROOT/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT64M_R1.asi'
    assert hashlib.sha256(previous.read_bytes()).hexdigest()=='6ec8ca1ffe69452dfc3e69e9c1e158d1f70109f0cfbfefcaf5c601c9770b1167'
    result=dict(result='passed',bytes=len(binary.data),sha256=hashlib.sha256(binary.data).hexdigest(),
        requested_global_max=2048,managers=2,global_pointer_arrays=3,additional_hooks=spans,
        default_bytes=67108864,per_record_limits_unchanged=[8,255],native_branch_interior_hits=0,
        preserved_default64_artifact=True,baseline_startup_checks=29,
        limitations='Offline binary checks and synthetic fixtures. Runtime initialization, counts, audio and FPS remain to be tested.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

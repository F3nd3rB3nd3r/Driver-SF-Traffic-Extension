"""Offline compiled/native verification; never loads an ASI or runs Driver.exe."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN,EXE
from audio_static_audit import EXPECTED
import verify_audio_probe
from workspace_paths import historical_path

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_reason_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_reason_r1'
NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_REASON_R1'
RANGES=((0xC53FB0,0xC54090),(0x84C2F0,0x84C330),(0x84C920,0x84C9E0),(0x84CC90,0x84CE70),(0xC6B160,0xC6BB80),(0xC50F00,0xC50F40))

def main(project='DriverSF_V29s_AUDIO_REASON.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_audio_reason_',fixture_path=None):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    pe,native=PE(BUILD/(NAME+'.asi')),PE();assert hashlib.sha256(native.data).hexdigest()==EXPECTED
    fixture=json.loads((fixture_path or OUT/'fixture_results.json').read_text());assert fixture['result']=='passed'
    for name,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    previous_fixture=json.loads((ROOT/'analysis/audio_slots2048_r1_2026-11-05/fixture_results.json').read_text())
    for name,digest in previous_fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    verify_audio_probe.main(project=project,name=NAME,build=BUILD,output=OUT/'baseline',
        extra_flags={'DSF_V29S_CAPACITY_TRACE=1','DSF_V29S_INDEXED_262144=1','DSF_V29S_AUDIO_PROBE=1',
            'DSF_V29S_AUDIO_DEFAULT_64M=1','DSF_V29S_AUDIO_SLOTS_2048=1','DSF_V29S_AUDIO_REASON_PROBE=1'} | (extra_flags or set()),
        bridge_test_path=ROOT/'analysis/audio_probe_r1_2026-11-05/bridge_tests.json',
        log_prefix=log_prefix,capacity_marker=b'actualBankResultStack=8 audioCapacityChanges=')
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def resolve(at):
        for _ in range(4):
            try:code=pe.read(at,5)
            except ValueError:return None # Raw E8 may be an immediate/padding, not a call.
            if code[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',code,1)[0]
        raise AssertionError('Thunk chain')
    decoded={}
    for first,last in RANGES:
        r=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{first:X},0x{last:X}',str(EXE)],capture_output=True,text=True,check=True)
        (OUT/f'native_{first:08X}.asm.txt').write_text(r.stdout)
        for line in r.stdout.splitlines():
            m=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
            if m:decoded[int(m[1],16)]=m[3]
    inventories=(('Reason',('Lookup','Create','Gate','Result','Delivered','Handled','Remove','Name','NameDefault','Register')),
        ('Slots',('AGate','BGate','AInsert','BInsert','AReuse','Eight','Byte','AUpdate','BUpdate')))
    spans=[]
    for prefix,names in inventories:
        table=next(v for k,v in symbols.items() if k.startswith('?kAudio'+prefix+'Sites@'))
        tramps=symbols['_g_audio'+prefix+'Tramps']
        for i,name in enumerate(names):
            row=pe.read(table+i*24,24);site,length=struct.unpack_from('<II',row)
            assert native.read(site,length)==row[8:8+length]
            assert site in decoded and site+length in decoded
            assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',text) for at,text in decoded.items() if site<=at<site+length)
            address=symbols[f'_Audio{prefix}{name}Bridge'];assert resolve(struct.unpack_from('<I',row,20)[0])==address
            following=min(v for v in symbols.values() if v>address);code=pe.read(address,following-address)
            for pattern in ('9c60','8bec','83e4f0','0fae0424','fc','83c4080fae0c24','8be5619d'):
                assert bytes.fromhex(pattern) in code,(prefix,name,pattern)
            assert bytes([0x55 if prefix=='Reason' else 0x53 if i==5 else 0x57,0x6A,i]) in code
            assert b'\xFF\x25'+struct.pack('<I',tramps+4*i) in code
            helper=symbols['_Audio'+prefix+'Entry']
            calls=[j for j in range(len(code)-4) if code[j]==0xE8 and resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==helper]
            assert len(calls)==1
            spans.append(dict(name=prefix+name,site=f'{site:08X}',length=length,signature=row[8:8+length].hex()))
    for i,a in enumerate(spans):
        for b in spans[i+1:]:assert int(a['site'],16)+a['length']<=int(b['site'],16) or int(b['site'],16)+b['length']<=int(a['site'],16)
    text=next(s for s in native.sections if s['name']=='.text');block=native.data[text['offset']:text['offset']+text['size']]
    candidates=[];unresolved=[]
    for i in range(len(block)-6):
        op=block[i];pc=text['va']+i
        if op in (0xE8,0xE9):dest=pc+5+struct.unpack_from('<i',block,i+1)[0]
        elif op==0x0F and 0x80<=block[i+1]<=0x8F:dest=pc+6+struct.unpack_from('<i',block,i+2)[0]
        elif op==0xEB or 0x70<=op<=0x7F or 0xE0<=op<=0xE3:dest=pc+2+struct.unpack_from('b',block,i+1)[0]
        else:continue
        for s in spans:
            at=int(s['site'],16)
            if at<dest<at+s['length']:
                record=dict(source=f'{pc:08X}',destination=f'{dest:08X}',site=s['name']);candidates.append(record)
                if pc in decoded:
                    if re.match(r'(?:j\w+|call|loop\w*)\s',decoded[pc]):unresolved.append(record)
                elif not any(a<=pc<=b for a,b in RANGES):unresolved.append(record)
    (OUT/'incoming_branch_candidates.json').write_text(json.dumps(dict(candidates=candidates,unresolved=unresolved),indent=2))
    assert not unresolved,unresolved
    for prefix,value in (('AudioDefaultPoolInstall',67108864),('AudioSlotsInstall',2048)):
        start=next(v for k,v in symbols.items() if k.startswith('?'+prefix+'@'));end=min(v for v in symbols.values() if v>start)
        assert struct.pack('<I',value) in pe.read(start,end-start)
    assert native.read(0x84CC98,7)==bytes.fromhex('83be6c01000010')
    assert native.read(0x84A683,5)==bytes.fromhex('6800b84200') and b'DSFV29s_AUDIO_STATIC_START' not in pe.data
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text();dll=source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('AudioDefaultPoolInstall();')<dll.index('AudioSlotsInstall();')<dll.index('AudioReasonInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    old=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_SLOTS2048_R1.asi')
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='c0210bf47a3c61b7b9a547d0ab30a359d9d991d6bbd153f96f35952a3abfefcc'
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),
        new_hooks=10,preserved_slot_hooks=9,compiled_sites=spans,new_capacity_changes=0,
        object_event_capacity=16,shared_default_bytes=67108864,global_audio_max=2048,
        native_branch_interior_hits=0,baseline_startup_checks=29,prior_slots_artifact_preserved=True,
        limitations='Synthetic fixtures and offline compiled checks. No live hook installation, audible-output or FPS result until the user test.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

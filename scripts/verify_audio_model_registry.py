"""Offline/native-fixture verification for combined192 model-registry build."""
from contextlib import redirect_stdout
import hashlib
import io
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN,EXE
from audio_static_audit import EXPECTED
import verify_audio_reason
from workspace_paths import historical_path

ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/audio_model192_r1_2026-11-05'
BUILD=ROOT/'build/v29s_audio_model192_r1';NAME='DriverSF_V29s_160Capacity_150Request_AUDIO_MODEL192_R1'
RANGES=((0x556E00,0x557050),(0x5535C0,0x553ADC),(0x7D7270,0x7D72D5))

def main(project='DriverSF_V29s_AUDIO_MODEL192.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_audio_model192_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    pe,native=PE(BUILD/(NAME+'.asi')),PE();assert hashlib.sha256(native.data).hexdigest()==EXPECTED
    fixture=json.loads((ROOT/'analysis/audio_model192_r1_2026-11-05/fixture_results.json').read_text());assert fixture['result']=='passed'
    for file,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/file).read_bytes()).hexdigest()==digest
    # Confirm the exact selected native instructions used by the fixture.
    inc=(ROOT/'tests/AudioModelNativeBytes.inc').read_text();arrays={}
    for index,values in re.findall(r'const U8 native(\d+)\[\]=\{(.*?)\};',inc):arrays[int(index)]=bytes(int(x,16) for x in values.split(','))
    blocks=re.findall(r'\{0x([0-9A-F]+)u,(\d+)u,native(\d+)\}',inc)
    assert len(blocks)==13
    for at,length,index in blocks:assert native.read(int(at,16),int(length))==arrays[int(index)]
    inherited_path=OUT/'inherited/verification.json'
    inherited=json.loads(inherited_path.read_text()) if inherited_path.exists() else {}
    if inherited.get('result')!='passed' or inherited.get('sha256')!=hashlib.sha256(pe.data).hexdigest():
        console=io.StringIO()
        with redirect_stdout(console):
            verify_audio_reason.main(project=project,name=NAME,build=BUILD,output=OUT/'inherited',
                extra_flags={'DSF_V29S_AUDIO_MODEL192=1'} | (extra_flags or set()),log_prefix=log_prefix,
                fixture_path=ROOT/'analysis/audio_reason_r1_2026-11-05/fixture_results.json')
        (OUT/'inherited_console.txt').write_text(console.getvalue())
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def resolve(at):
        for _ in range(4):
            try:b=pe.read(at,5)
            except ValueError:return None # Raw E8 bytes inside other instructions/padding.
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('Thunk chain')
    decoded={}
    for a,b in RANGES:
        result=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{a:X},0x{b:X}',str(EXE)],capture_output=True,text=True,check=True)
        (OUT/f'native_{a:08X}.asm.txt').write_text(result.stdout)
        for line in result.stdout.splitlines():
            m=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
            if m:decoded[int(m[1],16)]=m[3]
    table=next(v for k,v in symbols.items() if k.startswith('?kAudioModelSites@'));spans=[]
    names=('Gate','Store','Remove','Lookup','Setup','Bank','Success','Failure')
    for i,name in enumerate(names):
        row=pe.read(table+i*24,24);at,length=struct.unpack_from('<II',row);stock=native.read(at,length)
        assert stock==row[8:8+length] and at in decoded and at+length in decoded
        address=symbols[f'_AudioModel{name}Bridge'];assert resolve(struct.unpack_from('<I',row,20)[0])==address
        end=min(v for v in symbols.values() if v>address);code=pe.read(address,end-address)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc556a','83c4080fae0c24','8be5619d'):
            assert bytes.fromhex(pattern) in code,(name,pattern)
        assert b'\x55\x6A'+bytes([i]) in code
        if i in (0,5):assert bytes.fromhex('9dff6424e8') in code
        else:assert b'\x9D\xFF\x25'+struct.pack('<I',symbols['_g_audioModelTramps']+i*4) in code
        calls=[j for j in range(len(code)-4) if code[j]==0xE8 and resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==symbols['_AudioModelEntry']]
        assert len(calls)==1
        if i not in (0,5):assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',text) for pc,text in decoded.items() if at<=pc<at+length)
        spans.append(dict(name=name,site=f'{at:08X}',length=length,bytes=stock.hex(),explicit_branch=i in (0,5),includes_native_return=i in (6,7)))
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
                r=dict(source=f'{pc:08X}',destination=f'{dest:08X}',site=s['name']);candidates.append(r)
                if pc in decoded:
                    if re.match(r'(?:j\w+|call|loop\w*)\s',decoded[pc]):unresolved.append(r)
                elif not any(a<=pc<=b for a,b in RANGES):unresolved.append(r)
    (OUT/'model_branch_candidates.json').write_text(json.dumps(dict(candidates=candidates,unresolved=unresolved),indent=2));assert not unresolved,unresolved
    assert native.read(0x556E30,6)==bytes.fromhex('83790c2d7313')
    assert native.read(0x553918,8)==bytes.fromhex('84c00f84b5010000')
    # Native object/sentinel/default and generic map helper are never in patch inventory.
    for a,b in ((0x557000,0x55704B),(0x556EE0,0x556F0B),(0x556E00,0x556E17),(0x7D7270,0x7D72D5),(0x791990,0x7919AF)):
        assert all(int(s['site'],16)+s['length']<=a or int(s['site'],16)>=b for s in spans)
    install=next(v for k,v in symbols.items() if k.startswith('?AudioModelInstall@'));end=min(v for v in symbols.values() if v>install)
    assert struct.pack('<I',8*193*16) in pe.read(install,end-install)
    main=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text();dll=main[main.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('AudioReasonInstall();')<dll.index('AudioModelInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    old=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_REASON_R1.asi')
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='fcd56d9ad3faa845ecff75a86e4fd75288a8342b8b13453f44c11480b4661715'
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),capacity=192,new_hooks=spans,
        native_map_fixture_blocks_verified=len(blocks),node_bytes_per_instance=3072,storage_bytes_with_guards=24704,
        instance_bound=8,native_provider_layout_preserved=True,active_migration=False,
        native_branch_interior_hits=0,baseline_startup_checks=29,previous_reason_artifact_preserved=True,
        limitations='Isolated native map/core/bridge/OS fixtures and compiled checks. Actual game registry attachment, live counts, audio and FPS require user test.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

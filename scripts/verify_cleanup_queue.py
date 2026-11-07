"""Verify the compiled cleanup test and all inherited model192 checks, offline."""
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
import verify_audio_model_registry
from workspace_paths import historical_path

ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/cleanup262144_r1_2026-11-05'
BUILD=ROOT/'build/v29s_cleanup262144_r1';NAME='DriverSF_V29s_160Capacity_150Request_CLEANUP262144_R1'

def main(project='DriverSF_V29s_CLEANUP262144.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_cleanup262144_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    pe,native=PE(BUILD/(NAME+'.asi')),PE();assert hashlib.sha256(native.data).hexdigest()==EXPECTED
    fixture=json.loads((ROOT/'analysis/cleanup262144_r1_2026-11-05/fixture_results.json').read_text());assert fixture['result']=='passed'
    for name,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    inc=(ROOT/'tests/CleanupNativeBytes.inc').read_text()
    arrays={int(i):bytes(int(x,16) for x in s.split(',')) for i,s in re.findall(r'const U8 native(\d+)\[\]=\{(.*?)\};',inc)}
    for at,length,index in re.findall(r'\{0x([0-9A-F]+)u,(\d+)u,native(\d+)\}',inc):
        assert native.read(int(at,16),int(length))==arrays[int(index)]
    parent=OUT/'inherited/verification.json'
    previous=json.loads(parent.read_text()) if parent.exists() else {}
    if previous.get('result')!='passed' or previous.get('sha256')!=hashlib.sha256(pe.data).hexdigest():
        console=io.StringIO()
        with redirect_stdout(console):
            verify_audio_model_registry.main(project=project,name=NAME,build=BUILD,
                output=OUT/'inherited',extra_flags={'DSF_V29S_CLEANUP_262144=1'} | (extra_flags or set()),log_prefix=log_prefix)
        (OUT/'inherited_console.txt').write_text(console.getvalue())
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def resolve(at):
        for _ in range(5):
            try:b=pe.read(at,5)
            except ValueError:return None
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('Thunk chain')
    disasm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes','/range:0xBAE670,0xBAEF08',str(EXE)],capture_output=True,text=True,check=True).stdout
    (OUT/'native_queue.asm.txt').write_text(disasm)
    decoded={}
    for line in disasm.splitlines():
        m=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
        if m:decoded[int(m[1],16)]=m[3]
    table=symbol('?kCleanupSites@');spans=[]
    expected=(0xBAEA66,0xBAEC1F,0xBAED3A,0xBAE6B0,0xBAE753,0xBAE761)
    for i,name in enumerate(('Direct','Replace','Release','Begin','End','Empty')):
        row=pe.read(table+i*24,24);at,length=struct.unpack_from('<II',row)
        assert at==expected[i] and row[8:8+length]==native.read(at,length)
        assert at in decoded and at+length in decoded
        address=symbols[f'_Cleanup{name}Bridge'];assert resolve(struct.unpack_from('<I',row,20)[0])==address
        end=min(v for v in symbols.values() if v>address);code=pe.read(address,end-address)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc556a','83c4080fae0c24','8be5619d'):
            assert bytes.fromhex(pattern) in code,(name,pattern)
        assert b'\x55\x6A'+bytes([i]) in code
        assert b'\x9D\xFF\x25'+struct.pack('<I',symbols['_g_cleanupTramps']+i*4) in code
        calls=[j for j in range(len(code)-4) if code[j]==0xE8 and resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==symbols['_CleanupEntry']]
        assert len(calls)==1
        assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',s) for pc,s in decoded.items() if at<=pc<at+length)
        spans.append(dict(name=name,site=f'{at:08X}',length=length,stock=row[8:8+length].hex(),lea_relocated_in_trampoline=i<3))
    operands=symbol('?kCleanupOperands@');immediates=[]
    for i in range(2):
        row=pe.read(operands+i*16,16);at,offset=struct.unpack_from('<II',row)
        assert at==(0xBAE6D3 if i==0 else 0xBAE6E4) and offset==2 and row[8:14]==native.read(at,6)
        immediates.append(dict(site=f'{at:08X}',offset=offset,stock=row[8:14].hex()))
    interior=[]
    for pc,s in decoded.items():
        m=re.match(r'(?:j\w+|call|loop\w*)\s+([0-9A-Fa-f]{8})\b',s)
        if m:
            target=int(m[1],16)
            for site in spans:
                at=int(site['site'],16)
                if at<target<at+site['length']:interior.append(dict(source=hex(pc),target=hex(target)))
    assert not interior,interior
    install=symbol('?CleanupInstall@');end=min(v for v in symbols.values() if v>install)
    assert struct.pack('<I',1048608) in pe.read(install,end-install)
    for marker in (b'DSFV29s_CLEANUP_FATAL',b'DSFV29s_CLEANUP_START revision=1 status=',b'testCapacity=262144',b'nativeOrderPreserved=1',log_prefix):
        assert marker in pe.data
    main=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text();dll=main[main.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('AudioModelInstall();')<dll.index('CleanupInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    assert 'if(g_cleanupStatus!=1)CleanupFatal(g_cleanupReason);' in dll
    old=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_MODEL192_R1.asi')
    assert hashlib.sha256(old.read_bytes()).hexdigest()=='5bbb884f4c20ad24b099f012b2a4a9f036fe593446b9b8500b3b7ec6d6cb2fd1'
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),capacity=262144,
        hooks=spans,operands=immediates,native_blocks_verified=len(arrays),branch_interior_hits=0,
        baseline_startup_checks=29,audio_model_capacity=192,requests=150,graphics_slots=160,
        indexed_slots=262144,previous_model192_asi_preserved=True,
        limitations='Offline native fixture and compiled checks passed; game/main-menu/exit behavior requires user run.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

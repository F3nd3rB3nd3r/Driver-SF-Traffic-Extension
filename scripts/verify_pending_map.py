"""Compiled pending-map integration and inherited audio/cleanup checks, offline."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN,EXE
from audio_static_audit import EXPECTED
import verify_cleanup_queue
from workspace_paths import historical_path
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/pending512_r1_2026-11-05';BUILD=ROOT/'build/v29s_pending512_r1'
NAME='DriverSF_V29s_160Capacity_150Request_PENDING512_R1'
def main(project='DriverSF_V29s_PENDING512.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_pending512_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    pe,native=PE(BUILD/(NAME+'.asi')),PE();assert hashlib.sha256(native.data).hexdigest()==EXPECTED
    fixture=json.loads((ROOT/'analysis/pending512_r1_2026-11-05/fixture_results.json').read_text());assert fixture['result']=='passed'
    for n,d in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest()==d
    inc=(ROOT/'tests/PendingNativeBytes.inc').read_text();arrays={int(i):bytes(int(x,16) for x in s.split(',')) for i,s in re.findall(r'const U8 native(\d+)\[\]=\{(.*?)\};',inc)}
    for at,n,i in re.findall(r'\{0x([0-9A-F]+)u,(\d+)u,native(\d+)\}',inc):assert native.read(int(at,16),int(n))==arrays[int(i)]
    inherited_path=OUT/'inherited/verification.json';old=json.loads(inherited_path.read_text()) if inherited_path.exists() else {}
    if old.get('result')!='passed' or old.get('sha256')!=hashlib.sha256(pe.data).hexdigest():
        console=io.StringIO()
        with redirect_stdout(console):verify_cleanup_queue.main(project=project,name=NAME,build=BUILD,
            output=OUT/'inherited',extra_flags={'DSF_V29S_PENDING512=1'} | (extra_flags or set()),log_prefix=log_prefix)
        (OUT/'inherited_console.txt').write_text(console.getvalue())
    symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def function(prefix):
        a=symbol(prefix);b=min(v for v in symbols.values() if v>a);return a,pe.read(a,b-a)
    def resolve(at):
        for _ in range(5):
            try:b=pe.read(at,5)
            except ValueError:return None
            if b[0]!=0xE9:return at
            at+=5+struct.unpack_from('<i',b,1)[0]
        raise AssertionError('Thunk chain')
    asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes','/range:0x5A1A80,0x5A27E0',str(EXE)],capture_output=True,text=True,check=True).stdout
    (OUT/'native_pending_map.asm.txt').write_text(asm);decoded={}
    for line in asm.splitlines():
        m=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
        if m:decoded[int(m[1],16)]=m[3]
    sites=[];table=symbol('?kPendingSites@')
    for i,name in enumerate(('Begin','End')):
        row=pe.read(table+i*20,20);at,n=struct.unpack_from('<II',row)
        assert at==(0x5A2230 if i==0 else 0x5A2466) and n==6 and row[8:14]==native.read(at,n)
        assert at in decoded and at+n in decoded
        address=symbols[f'_Pending{name}Bridge'];assert resolve(struct.unpack_from('<I',row,16)[0])==address
        end=min(v for v in symbols.values() if v>address);code=pe.read(address,end-address)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','fc556a','83c4080fae0c24','8be5619d'):assert bytes.fromhex(pattern) in code,(name,pattern)
        assert b'\x55\x6A'+bytes([i]) in code and b'\x9D\xFF\x25'+struct.pack('<I',symbols['_g_pendingTramps']+i*4) in code
        assert any(code[j]==0xE8 and resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==symbols['_PendingEntry'] for j in range(len(code)-4))
        assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',s) for pc,s in decoded.items() if at<=pc<at+n)
        sites.append(dict(name=name,site=f'{at:08X}',length=n,stock=row[8:14].hex()))
    interior=[]
    for pc,s in decoded.items():
        m=re.match(r'(?:j\w+|call|loop\w*)\s+([0-9A-Fa-f]{8})\b',s)
        if m:
            target=int(m[1],16)
            for site in sites:
                at=int(site['site'],16)
                if at<target<at+site['length']:interior.append(dict(source=hex(pc),target=hex(target)))
    assert not interior,interior
    address,code=function('?MapInsertBridge@');insert=symbol('?PendingInsert@')
    assert any(code[j] in (0xE8,0xE9) and resolve(address+j+5+struct.unpack_from('<i',code,j+1)[0])==insert for j in range(len(code)-4))
    _,prepare=function('?PendingPrepare@');assert struct.pack('<I',7232) in prepare
    _,fatal=function('?PendingFatal@');assert struct.pack('<I',0x29E1) in fatal
    for marker in (b'DSFV29s_PENDING_START revision=1 status=',b'testCapacity=512 inlineNodes=64 extraNodes=448',b'DSFV29s_PENDING_FATAL',log_prefix):assert marker in pe.data
    main_source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    bridge=main_source[main_source.index('UPtr __fastcall MapInsertBridge('):main_source.index('// Selector node sidecar,')]
    assert bridge.index('if(PendingOwns(map))return PendingInsert(map,a,b,c);')<bridge.index('IsResourceMap16Variant')
    dll=main_source[main_source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('CleanupInstall();')<dll.index('PendingInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    assert 'if(g_pendingStatus!=1)PendingFatal(g_pendingReason,0u);' in dll
    previous=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_CLEANUP262144_R1.asi')
    assert hashlib.sha256(previous.read_bytes()).hexdigest()=='ee5bb02f2ddc8a5bb1074a001a622ce2567ccb54124cb092bb5fd7a35aef332b'
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),capacity=512,
        inline_nodes=64,external_nodes_per_generation=448,arena_bytes=7232,new_traversal_hooks=sites,
        native_insert_shared_hook='007D7270',native_fixture_blocks=len(arrays),branch_interior_hits=0,
        baseline_startup_checks=29,audio_model_capacity=192,cleanup_capacity=262144,requests=150,graphics_slots=160,
        previous_cleanup_asi_preserved=True,limitations='Native-byte fixture and compiled checks; actual menu/reload/exit still needs user run.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
if __name__=='__main__':main()

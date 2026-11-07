"""Verify compiled callback extension and all inherited chapter/pending/audio checks."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_chapter_requests
from workspace_paths import historical_path
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/callback512_r1_2026-11-06'
BUILD=ROOT/'build/v29s_callback512_r1';NAME='DriverSF_V29s_160Capacity_150Request_CALLBACK512_R1'

def main(project='DriverSF_V29s_CALLBACK512.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_callback512_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    fixture=json.loads((ROOT/'analysis/callback512_r1_2026-11-06/fixture_results.json').read_text());assert fixture['result']=='passed'
    for file,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/file).read_bytes()).hexdigest()==digest
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    inc=(ROOT/'tests/CallbackNativeBytes.inc').read_text()
    arrays={int(i):bytes(int(x,16) for x in s.split(',')) for i,s in re.findall(r'const U8 native(\d+)\[\]=\{(.*?)\};',inc)}
    for at,n,i in re.findall(r'\{0x([0-9A-F]+)u,(\d+)u,native(\d+)\}',inc):assert native.read(int(at,16),int(n))==arrays[int(i)]
    console=io.StringIO()
    with redirect_stdout(console):verify_chapter_requests.main(project=project,name=NAME,build=BUILD,
        output=OUT/'inherited',extra_flags={'DSF_V29S_CALLBACK512=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'inherited_console.txt').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
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
        raise AssertionError('thunk chain')
    decoded={}
    for a,b in ((0x5567D0,0x556821),(0x858F80,0x859244)):
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{a:X},0x{b:X}',str(__import__('static_streaming_audit').EXE)],capture_output=True,text=True,check=True).stdout
        (OUT/f'native_{a:08X}.asm.txt').write_text(asm)
        for line in asm.splitlines():
            m=re.match(r'\s*([0-9A-Fa-f]{8}):\s+((?:[0-9A-Fa-f]{2} )+)(.*)',line)
            if m:decoded[int(m[1],16)]=m[3]
    sites=[];table=symbol('?kCallbackSites@')
    for i,(at,length,prefix) in enumerate(((0x5567D0,6,'?CallbackInsertBridge@'),(0x858FB5,6,'_CallbackWriterBridge'),(0x859000,5,'?CallbackPollBridge@'))):
        row=pe.read(table+i*20,20);assert struct.unpack_from('<II',row)==(at,length)
        assert row[8:8+length]==native.read(at,length) and at in decoded and at+length in decoded
        assert resolve(struct.unpack_from('<I',row,16)[0])==symbol(prefix)
        assert all(not re.match(r'(?:j\w+|call|loop\w*)\s',text) for pc,text in decoded.items() if at<=pc<at+length)
        sites.append(dict(address=f'{at:08X}',length=length,stock=row[8:8+length].hex(),bridge=prefix))
    interior=[]
    for pc,text in decoded.items():
        m=re.match(r'(?:j\w+|call|loop\w*)\s+([0-9A-Fa-f]{8})\b',text)
        if m:
            for s in sites:
                if int(s['address'],16)<int(m[1],16)<int(s['address'],16)+s['length']:interior.append((pc,m[1]))
    assert not interior
    writer=symbol('_CallbackWriterBridge');stop=min(v for v in symbols.values() if v>writer);code=pe.read(writer,stop-writer)
    for pattern in ('9c60','8bec','83e4f0','0fae0424','fcff7504','83c4040fae0c24','8be5619d'):
        assert bytes.fromhex(pattern) in code,pattern
    assert b'\x9D\xFF\x25'+struct.pack('<I',symbols['_g_callbackTramps']+4) in code
    for marker in (b'DSFV29s_CALLBACK_START revision=1 status=',b'groups=21 stockCapacity=64 testCapacity=512',
        b'DSFV29s_CALLBACK_FATAL reason=',b'DSFV29s_CALLBACK_GROUP',b'processLifetime=1 lazyRedirect=1',log_prefix):
        assert marker in pe.data,marker
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text();dll=source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('PendingInstall();')<dll.index('CallbackInstall();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    assert 'if(g_callbackStatus!=1)CallbackFatal(g_callbackReason,0u);' in dll
    fatal=symbol('?CallbackFatal@');end=min(v for v in symbols.values() if v>fatal)
    assert struct.pack('<I',0x29C1) in pe.read(fatal,end-fatal)
    parent=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_CHAPTERS_R1.asi')
    assert hashlib.sha256(parent.read_bytes()).hexdigest()=='602aa57d7f5464faa3456499ac3009a07e854e76fbc2b8d6264c2ccd1b229442'
    for prefix in ('?CallbackInsertBridge@','?CallbackPollBridge@','_CallbackWriterBridge','?CallbackInstall@'):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at)
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{end-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('?_').split('@')[0]+'.asm.txt')).write_text(asm)
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),groups=21,capacity=512,
        native_inline=64,private_extra_per_group=448,node_bytes=12,private_node_bytes=112896,hooks=sites,
        requests=150,graphics_slots=160,chapter_parent_preserved=True,inherited_startup_checks=29,
        production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in
            ('V29sCallbackCore.inc','V29sCallbackBridges.inc','V29sCallbackPatch.inc','V29sCallback.inc')},
        limitations='Native fixture/compiled verification only; live challenge transitions, callback counts and stability await user run.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()

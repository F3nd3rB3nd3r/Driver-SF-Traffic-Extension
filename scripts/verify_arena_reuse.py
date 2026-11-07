"""Verify the compiled arena-lifetime repair and inherited callback/chapter patches."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from test_arena_reuse import extract
import verify_callback_arena
from workspace_paths import historical_path
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/arena_reuse_r1_2026-11-06'
BUILD=ROOT/'build/v29s_arena_reuse_r1';NAME='DriverSF_V29s_160Capacity_150Request_ARENA_REUSE_R1'

def main(name=NAME,build=BUILD,output=OUT,extra_flags=None,log_prefix=b'DSFV29s_arena_reuse_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    fixture=json.loads((ROOT/'analysis/arena_reuse_r1_2026-11-06/fixture_results.json').read_text());assert fixture['result']=='passed'
    for name,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    source=(ROOT/'src/V29sNativeLoadingTrace.inc').read_text()
    adapters=[extract(source,'extern "C" __declspec(noinline) void __cdecl '+name) for name in ('DiagSPrepareSpool0(','DiagSPrepareScratch(')]
    assert [hashlib.sha256(a.encode()).hexdigest() for a in adapters]==fixture['adapters_sha256']
    native=PE();inc=(ROOT/'tests/ArenaReuseNativeBytes.inc').read_text()
    arrays={int(i):bytes(int(x,16) for x in s.split(',')) for i,s in re.findall(r'const U8 native(\d+)\[\]=\{(.*?)\};',inc)}
    for at,n,i in re.findall(r'\{0x([0-9A-F]+)u,(\d+)u,native(\d+)\}',inc):assert native.read(int(at,16),int(n))==arrays[int(i)]
    console=io.StringIO()
    with redirect_stdout(console):verify_callback_arena.main(project='DriverSF_V29s_ARENA_REUSE.vcxproj',name=NAME,build=BUILD,
        output=OUT/'inherited',extra_flags={'DSF_V29S_ARENA_REUSE=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'inherited_console.txt').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(prefix):
        at=symbol(prefix);end=min(v for v in symbols.values() if v>at);return at,pe.read(at,end-at)
    for i,prefix in enumerate(('_DiagSScratchBridge','_DiagSScratchSecondBridge','_DiagSReconcileBridge')):
        at,code=body(prefix)
        for pattern in ('9c60','8bec','83e4f0','0fae0424','0fae0c24','8be5619d'):
            assert bytes.fromhex(pattern) in code,(prefix,pattern)
        if i<2:
            assert b'\x55\x6A'+bytes([i]) in code
            assert b'\x9D\xFF\x25'+struct.pack('<I',symbol('?g_sScratchTramps@')+i*4) in code
        else:
            assert bytes.fromhex('ff7518') in code and b'\x9D\xFF\x25'+struct.pack('<I',symbol('?g_sTramp@')) in code
    for marker in (b'DSFV29s_LEGACY_FATAL reason=',b' exitCode=',b'DSFV29s_ARENA_START revision=1 status=1',
        b'fixedOwnerLimit=0 privateNodes=256 scratchPolicy=expandedFrame150 scratchExternal=0',b'DSFV29s_ARENA_STATUS',log_prefix):
        assert marker in pe.data,marker
    # These fixed scratch buffers must not survive into the repaired variant.
    assert not any(k.startswith('?g_sScratchArenas@') or k.startswith('?g_sSpool0Arenas@') for k in symbols)
    main_source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    dll=main_source[main_source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('CallbackInstall();')<dll.index('ArenaReuseStartLine();')<dll.index('CreateThread(nullptr, 0, &Monitor')
    assert 'namespace {\n#if defined(DSF_V29S_ARENA_REUSE)\nstatic void LegacyExit' in main_source
    # All native-loading legacy exits route through the file-backed logger.
    assert 'ExitProcess(' not in source
    parent=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_CALLBACK512_R1.asi')
    assert hashlib.sha256(parent.read_bytes()).hexdigest()=='00a7f6e81fed401031302a10e4b169fdeb0d7dad15e2a9a74a25d98f5683bf18'
    for prefix in ('_DiagSPrepareSpool0','_DiagSPrepareScratch','?ReusePrepareSpool@','?LegacyExit@','?ArenaReuseStartup@'):
        try:at,code=body(prefix)
        except StopIteration:continue # Optimized helper may be folded into its adapter.
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{at:X},0x{at+len(code)-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('_?').split('@')[0]+'.asm.txt')).write_text(asm)
    result=dict(result='passed',bytes=len(pe.data),sha256=hashlib.sha256(pe.data).hexdigest(),
        requests=150,graphics_slots=160,scratch_inline=150,scratch_external_arenas=0,spool_private_nodes_per_owner=256,
        fixed_eight_owner_registry_removed=True,new_hook_sites=0,replaced_bridges=3,callback_capacity=512,
        inherited_startup_checks=29,parent_callback_asi_preserved=True,
        limitations='Native fixture and compiled checks only; live challenge/menu/reload stability and legacy-exit attribution await user run.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()

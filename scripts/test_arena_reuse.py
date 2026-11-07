"""Production adapters/core/bridges, actual150-frame append and merge native fixture."""
import hashlib,json,re,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/arena_reuse_r1_2026-11-06';BUILD=ROOT/'build/arena_reuse_fixture'

def extract(source,signature):
    start=source.index(signature);at=source.index('{',start)+1;depth=1
    while depth:
        if source[at]=='{':depth+=1
        elif source[at]=='}':depth-=1
        at+=1
    return source[start:at]

def main():
    OUT.mkdir(exist_ok=True,parents=True);BUILD.mkdir(exist_ok=True,parents=True);pe=PE()
    assert hashlib.sha256(pe.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    blocks=[(0x5A75DD,0x3D),(0x5A7665,0x3D),(0x5A7D50,0xDF)]
    lines=[]
    for i,(at,n) in enumerate(blocks):lines.append(f'static const U8 native{i}[]={{'+','.join(f'0x{x:02X}' for x in pe.read(at,n))+'};')
    lines+=['struct NativeBlock{U32 at,length;const U8* bytes;};','static const NativeBlock nativeBlocks[]={'+','.join(f'{{0x{at:X}u,{n}u,native{i}}}' for i,(at,n) in enumerate(blocks))+'};']
    operands=[[0x5A75F5,0x5A7602,0x5A760C,0x5A7616],[0x5A767D,0x5A768A,0x5A7694,0x5A769E]]
    for row in operands:
        for at in row:assert pe.read(at,4)==bytes.fromhex('78020000')
    lines.append('static const U32 scratchOperands[2][4]={'+','.join('{'+','.join(f'{at-blocks[i][0]}u' for at in row)+'}' for i,row in enumerate(operands))+'};')
    (ROOT/'tests/ArenaReuseNativeBytes.inc').write_text('\n'.join(lines)+'\n')
    source=(ROOT/'src/V29sNativeLoadingTrace.inc').read_text()
    adapters=[extract(source,'extern "C" __declspec(noinline) void __cdecl '+name) for name in ('DiagSPrepareSpool0(','DiagSPrepareScratch(')]
    (ROOT/'tests/ArenaReuseAdapters.inc').write_text('\n'.join(adapters)+'\n')
    (ROOT/'tests/LegacyScratchAdapter.inc').write_text(adapters[1].replace('DiagSPrepareScratch(', 'LegacyPrepareScratch(')+'\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    exe=BUILD/'ArenaReuseTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'ArenaReuseTest.obj'),str(ROOT/'tests/ArenaReuseTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:ArenaReuseTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(exe),str(lib)]
    build=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(build.stdout+build.stderr)
    assert build.returncode==0,build.stdout+build.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=40);assert run.returncode==0,(run.returncode,run.stdout,run.stderr)
    assert run.stdout.strip()=='arena reuse checks passed'
    result=dict(result='passed',architecture='Win32',native_blocks=blocks,
        production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in ('V29sArenaReuseCore.inc','V29sArenaReuseBridges.inc','V29sArenaReuse.inc')},
        adapters_sha256=[hashlib.sha256(a.encode()).hexdigest() for a in adapters],
        checks=['old scratch adapter actually exhausts8 arenas on ninth50-boundary reset',
            '64 sequential150-entry local frames with both real native append fragments',
            'same production150-source-offset merge clone copies values into persistent storage',
            'local frame overwrite does not change destination values','16 genuine owner-header resets reuse one arena',
            '16 distinct owners retain distinct process-lived backing beyond8','native shrink/recycled-node refill and inline free-tail attachment',
            '151st fresh scratch write refused','bad cursor/count/guard/allocation refusal writes reason and code',
            'frame/merge startup mutation refusal','all3 changed bridge GP/flags/DF/x87/SSE preservation'],
        limitations='Selected native append fragments have eight frame offsets rebased to the already installed728 sentinel; merge has the same two source-offset changes as production. Native services/whole reconcile/state machines/game are not executed.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()

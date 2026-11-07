"""Production pending-map core/installer and native-byte fixture, no game/ASI load."""
import hashlib,json,re,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from audio_static_audit import EXPECTED
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/pending512_r1_2026-11-05';BUILD=ROOT/'build/pending_map_fixture'
def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True);pe=PE();assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    blocks=[(0x7D7270,0x65),(0x5A1BD0,0x26),(0x5A242C,0x16),(0x5A2230,0x16),(0x5A2466,0xA)]
    lines=['// Selected exact native bytes; fixture substitutes only the work between traversal entry/end.']
    for i,(at,n) in enumerate(blocks):lines.append(f'static const U8 native{i}[]={{'+','.join(f'0x{x:02X}' for x in pe.read(at,n))+'};')
    lines+=['struct NativeBlock{U32 at,length;const U8* bytes;};','static const NativeBlock nativeBlocks[]={'+','.join(f'{{0x{at:X}u,{n}u,native{i}}}' for i,(at,n) in enumerate(blocks))+'};']
    (ROOT/'tests/PendingNativeBytes.inc').write_text('\n'.join(lines)+'\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    exe=BUILD/'PendingMapTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'PendingMapTest.obj'),str(ROOT/'tests/PendingMapTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:PendingTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(exe),str(lib)]
    result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);assert run.returncode==0,f'Fixture check {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip()=='pending map checks passed'
    result=dict(result='passed',architecture='Win32',capacity=512,arena_bytes=7232,native_blocks=blocks,
        production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in
            ('V29sPendingMapCore.inc','V29sPendingMapBridges.inc','V29sPendingMapPatch.inc')},
        checks=['stock65th insertion reproduces sentinel/neighbor overwrite','512 actual native inserts and linked traversal',
            '513th refused before write','native inline removal/recycling256 nodes and reinsertion',
            'native traversal entry/return with production bridges and actual native removal fragment',
            'empty checks and nonempty/incomplete/completed traversal','constructor reset uses new arena and retains old external bytes',
            'two owners and completed/pending maps simultaneously distinct','neighbor headers and guards preserved',
            'allocation/identity/parent/cold/signature/protection/cache/rollback failures',
            'cycle/free-chain/header/guard rejection','two bridge GP/flags/DF/x87/SSE ABI checks'],
        limitations='Only selected native instructions run in fake packages. Traversal body is a fixture drain using the native removal fragment. No actual game/ASI execution.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
if __name__=='__main__':main()

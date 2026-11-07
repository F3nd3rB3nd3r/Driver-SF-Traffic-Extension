"""Run production callback logic and selected native instructions in an isolated Win32 fixture."""
import hashlib,json,re,subprocess,winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/callback512_r1_2026-11-06';BUILD=ROOT/'build/callback_arena_fixture'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    pe=PE();assert hashlib.sha256(pe.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    blocks=[(0x5567D0,0x51),(0x858FB5,0x1F),(0x859000,0x73),(0xDB40A0,0x3C)]
    lines=[]
    for i,(at,n) in enumerate(blocks):lines.append(f'static const U8 native{i}[]={{'+','.join(f'0x{x:02X}' for x in pe.read(at,n))+'};')
    lines+=['struct NativeBlock{U32 at,length;const U8* bytes;};','static const NativeBlock nativeBlocks[]={'+','.join(f'{{0x{at:X}u,{n}u,native{i}}}' for i,(at,n) in enumerate(blocks))+'};']
    (ROOT/'tests/CallbackNativeBytes.inc').write_text('\n'.join(lines)+'\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    exe=BUILD/'CallbackArenaTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'CallbackArenaTest.obj'),str(ROOT/'tests/CallbackArenaTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:CallbackTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(exe),str(lib)]
    result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(result.stdout+result.stderr)
    assert result.returncode==0,result.stdout+result.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=45)
    assert run.returncode==0,f'Fixture check {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip().endswith('callback arena checks passed')
    result=dict(result='passed',architecture='Win32',native_blocks=blocks,groups=21,capacity=512,
        production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in
            ('V29sCallbackCore.inc','V29sCallbackBridges.inc','V29sCallbackPatch.inc')},
        checks=['stock65th native insertion overwrites adjacent header','all21 groups at512 with original sentinels',
            '513th refused before native write','actual native callback poll/remove/recycle/partial and complete drain',
            'empty refill retains process storage','same-thread registration during callback',
            'exact registration fragment and selector guard','non-target helper passthrough',
            'cycles/freechain/reset/guards/selector/concurrent/recursive-poll refusals',
            'parent/cold/allocation/signature/protection/cache/rollback failure handling',
            'mid-function bridge GP/flags/DF/x87/SSE preservation'],
        limitations='Fixture rebases six native absolute global operands and registration CALL. Native poll prefix runs real callbacks/removal; remaining resource state machine is replaced by a return stub. No game or ASI load.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()

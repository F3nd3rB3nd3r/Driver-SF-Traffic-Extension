"""Compile/run isolated native queue fixtures. No game/ASI execution."""
import hashlib
import json
import re
import struct
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
from audio_static_audit import EXPECTED

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/cleanup262144_r1_2026-11-05'
BUILD=ROOT/'build/cleanup_queue_fixture'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    pe=PE();assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    blocks=[(0xBAE670,0x2B),(0xBAE6B0,0xB9),(0xBAEA60,0x20),(0xBAEBE0,0xB1),(0xBAECA0,0xE8)]
    absolute=[0x16F49A4,0x16F49A0,0x1732BE8,0x1732BD0,0x1732BD4,0x1732BE0,0x1732BC8,
              0x1732BCC,0x1732BE4,0x1732BD8,0x1732BDC,0x19B3BE0,0xDC915C,0xDC9168]
    lines=['// Exact selected native bytes; fixture-only address fixups below.']
    fixups=[]
    for i,(at,size) in enumerate(blocks):
        b=pe.read(at,size);lines.append(f'static const U8 native{i}[]={{'+','.join(f'0x{x:02X}' for x in b)+'};')
        for v in absolute:
            needle=struct.pack('<I',v);off=b.find(needle)
            while off>=0:
                fixups.append((at+off,v));off=b.find(needle,off+1)
    lines+=['struct NativeBlock{U32 at,length;const U8* bytes;};','static const NativeBlock nativeBlocks[]={'+
        ','.join(f'{{0x{at:X}u,{n}u,native{i}}}' for i,(at,n) in enumerate(blocks))+'};',
        'struct NativeFixup{U32 at,value;};','static const NativeFixup nativeFixups[]={'+
        ','.join(f'{{0x{a:X}u,0x{v:X}u}}' for a,v in fixups)+'};']
    (ROOT/'tests/CleanupNativeBytes.inc').write_text('\n'.join(lines)+'\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libraries=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    library=max((v for v in libraries if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    program=BUILD/'CleanupQueueTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'CleanupQueueTest.obj'),str(ROOT/'tests/CleanupQueueTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:CleanupTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/OUT:'+str(program),str(library)]
    compiled=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/'fixture_build.log').write_text(compiled.stdout+compiled.stderr)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=30)
    assert run.returncode==0,f'Fixture failed at check {run.returncode}: {run.stdout!r} {run.stderr!r}'
    assert run.stdout.strip()=='cleanup queue checks passed',repr(run.stdout)
    result=dict(result='passed',architecture='Win32',capacity=262144,native_blocks=blocks,
        production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in
            ('V29sCleanupQueueCore.inc','V29sCleanupQueueBridges.inc','V29sCleanupQueuePatch.inc')},
        checks=['262144 native appends through production detours','actual native drain and virtual destruction/wrapper-release order',
            'empty drain and reuse','native replacement publishes users after enqueue and transfers reference count',
            'native zero-ref release only enqueues on final reference and unlinks',
            'capacity boundary/guard/owner/reentrancy refusal and late-enqueue mismatch',
            'seven allocation failures, cold identity/signature refusal, protection/cache/rollback failures',
            'unrelated code preserved','six ABI bridges preserve GP/flags/DF/x87/SSE'],
        limitations='Native selected bytes execute in a fake manager with fixture-only imported/global/call fixups. No actual game/ASI load; user menu transition still required.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

if __name__=='__main__':main()

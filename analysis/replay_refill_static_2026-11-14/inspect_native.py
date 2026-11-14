"""Scoped replay refill audit. Reads supported PE; never executes/attaches to it."""
import hashlib, json, struct, subprocess, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'scripts'))
from static_streaming_audit import PE, DUMPBIN, EXE
p = PE()
digest = hashlib.sha256(p.data).hexdigest()
assert digest == 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
mode = sys.argv[1]
if mode == 'disasm':
    for span in sys.argv[2:]:
        lo, hi = (int(x, 16) for x in span.split(':'))
        r = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
                            f'/range:0x{lo:X},0x{hi:X}', str(EXE)],
                           capture_output=True, text=True, check=True)
        f = OUT / f'{lo:08X}-{hi:08X}.asm.txt'
        f.write_text(r.stdout)
        print(f.name, len(r.stdout.splitlines()), 'lines')
elif mode == 'refs':
    targets = {int(x, 16): [] for x in sys.argv[2:]}
    for s in p.sections:
        if not s['flags'] & 0x20000000:
            continue
        b = p.data[s['offset']:s['offset'] + s['size']]
        for i in range(len(b) - 4):
            if b[i] not in (0xE8, 0xE9):
                continue
            t = (s['va'] + i + 5 + struct.unpack_from('<i', b, i + 1)[0]) & 0xffffffff
            if t in targets:
                targets[t].append(dict(at=f'{s["va"]+i:08X}', opcode=f'{b[i]:02X}'))
    result = {f'{t:08X}': dict(relative_candidates=x, absolute_candidates=p.refs(t, False))
              for t, x in targets.items()}
    f = OUT / ('refs_' + '_'.join(f'{t:08X}' for t in targets) + '.json')
    f.write_text(json.dumps(dict(exe_sha256=digest, results=result), indent=2) + '\n')
    print(json.dumps(result, indent=2))
elif mode == 'vtable':
    result = {}
    for arg in sys.argv[2:]:
        address = int(arg, 16)
        result[arg] = {f'{i*4:02X}': f'{v:08X}' for i, v in enumerate(struct.unpack('<10I', p.read(address, 40)))}
    (OUT / 'vtables.json').write_text(json.dumps(dict(exe_sha256=digest, vtables=result), indent=2) + '\n')
    print(json.dumps(result, indent=2))
elif mode == 'read':
    for arg in sys.argv[2:]:
        at, size = (int(x, 16) for x in arg.split(':'))
        print(arg, p.read(at, size).hex(' '))
else:
    raise SystemExit('disasm LO:HI ... | refs TARGET ... | vtable ADDRESS ... | read ADDRESS:SIZE ...')

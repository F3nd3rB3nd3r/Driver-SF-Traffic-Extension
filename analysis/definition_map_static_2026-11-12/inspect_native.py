"""Scoped read-only Driver.exe analysis; writes only adjacent evidence files."""
import hashlib
import json
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from static_streaming_audit import PE, EXE, DUMPBIN

OUT = Path(__file__).resolve().parent
pe = PE()
digest = hashlib.sha256(pe.data).hexdigest()
assert digest == 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
if sys.argv[1] == 'disasm':
    for pair in sys.argv[2:]:
        lo, hi = (int(x, 16) for x in pair.split(':'))
        result = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
            f'/range:0x{lo:X},0x{hi:X}', str(EXE)], capture_output=True, text=True, check=True)
        dest = OUT / f'{lo:08X}-{hi:08X}.asm.txt'
        dest.write_text(result.stdout, encoding='utf-8')
        print(dest.name)
elif sys.argv[1] == 'refs':
    targets = {int(x, 16): [] for x in sys.argv[2:]}
    for section in pe.sections:
        if not section['flags'] & 0x20000000:
            continue
        block = pe.data[section['offset']:section['offset'] + section['size']]
        for i in range(len(block) - 4):
            if block[i] not in (0xE8, 0xE9):
                continue
            target = (section['va'] + i + 5 + struct.unpack_from('<i', block, i + 1)[0]) & 0xFFFFFFFF
            if target in targets:
                targets[target].append(dict(site=f'{section["va"] + i:08X}', opcode=f'{block[i]:02X}'))
    results = {f'{target:08X}': dict(relative_candidates=sites,
        absolute_candidates=pe.refs(target, False)) for target, sites in targets.items()}
    dest = OUT / ('refs_' + '_'.join(f'{x:08X}' for x in targets) + '.json')
    dest.write_text(json.dumps(dict(exe_sha256=digest, results=results), indent=2) + '\n')
    print(json.dumps(results, indent=2))
else:
    raise SystemExit('Use disasm LO:HI ... or refs TARGET ...')

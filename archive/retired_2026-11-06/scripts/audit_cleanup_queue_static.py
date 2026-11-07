"""Focused Driver.exe deferred-cleanup audit. No execution or process attachment."""
import hashlib
import json
import re
from pathlib import Path
import struct
import subprocess

from static_streaming_audit import PE, DUMPBIN, EXE
from audio_static_audit import EXPECTED

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'analysis/cleanup_queue_static_2026-11-05'
TARGETS = {'drain': 0xBAE6B0, 'append_direct': 0xBAEA60,
           'remove_object': 0xBAEBE0, 'remove_key': 0xBAECA0, 'constructor': 0xBAE930}


def function_start(pe, at):
    lo = max(0x401000, at-0x1800)
    block = pe.read(lo, at-lo)
    # MSVC function padding is a useful boundary candidate, not unwind metadata.
    i = block.rfind(b'\xcc\xcc')
    if i < 0:
        return None
    while i < len(block) and block[i] == 0xCC:
        i += 1
    return lo+i


def main():
    pe = PE()
    assert hashlib.sha256(pe.data).hexdigest() == EXPECTED
    OUT.mkdir(parents=True, exist_ok=True)
    incoming = {name: [] for name in TARGETS}
    target_names = {v: k for k, v in TARGETS.items()}
    for section in pe.sections:
        if not section['flags'] & 0x20000000:
            continue
        block = pe.data[section['offset']:section['offset']+section['size']]
        for i in range(len(block)-4):
            if block[i] not in (0xE8, 0xE9):
                continue
            site = section['va']+i
            target = (site+5+struct.unpack_from('<i', block, i+1)[0]) & 0xFFFFFFFF
            if target in target_names:
                incoming[target_names[target]].append(dict(site=f'{site:08X}',
                    opcode='call' if block[i] == 0xE8 else 'jmp',
                    function_start_candidate=f'{function_start(pe,site):08X}' if function_start(pe,site) else None))
    ranges = {(0xBAE670, 0xBAEE50), (0xBAE930, 0xBAE9AD),
        (0xDC7F30,0xDC7F50), (0xBB4EE0,0xBB4FC0), (0xBAEE50,0xBAEF60),
        (0xBB81D0,0xBB8320), (0xBA9600,0xBA96B0), (0xC28590,0xC285E0),
        (0xBABC10,0xBABD20), (0xBAD790,0xBAD910)}
    ranges.add((0xBB6490,0xBB6680))
    for candidates in incoming.values():
        for c in candidates:
            site = int(c['site'], 16)
            start = int(c['function_start_candidate'], 16) if c['function_start_candidate'] else site-64
            ranges.add((start, site+80))
    for lo, hi in sorted(ranges):
        result = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
            f'/range:0x{lo:X},0x{hi:X}', str(EXE)], capture_output=True, text=True, check=True)
        (OUT/f'{lo:08X}-{hi:08X}.asm.txt').write_text(result.stdout, encoding='utf-8')
    # Resolve only imported operations used in the inspected queue/caller paths.
    wanted = {0xDC915C,0xDC9160,0xDC9168,0xDC91F4,0xDC9288,
              0xDC9200,0xDC91FC,0xDC9204}
    imports = {}
    def cstring(at):
        value = bytearray()
        for i in range(256):
            b = pe.read(at+i,1)[0]
            if not b:
                return value.decode('ascii')
            value.append(b)
        raise ValueError('Long import name')
    irva, _ = struct.unpack_from('<2I',pe.data,pe.pe+24+104)
    for d in range(128):
        original, stamp, chain, name, first = struct.unpack('<5I',pe.read(pe.base+irva+d*20,20))
        if not any((original,stamp,chain,name,first)):
            break
        for n in range(4096):
            thunk, = struct.unpack('<I',pe.read(pe.base+(original or first)+n*4,4))
            if not thunk:
                break
            iat = pe.base+first+n*4
            if iat in wanted:
                imports[f'{iat:08X}'] = dict(dll=cstring(pe.base+name),
                    name=f'ordinal{thunk&0xFFFF}' if thunk&0x80000000 else cstring(pe.base+thunk+2))
    result = dict(result='passed',executable=str(EXE), sha256=EXPECTED, imports=imports,
        targets={k: f'{v:08X}' for k, v in TARGETS.items()},
        incoming_candidates=incoming,
        data_pointer_candidates={k: pe.refs(v, False) for k, v in TARGETS.items()},
        count_offset_candidates=pe.refs(0x17E090, False),
        array_offset_candidates=pe.refs(0x15E090, False),
        absolute_count_candidates=pe.refs(0x18B0C80, False),
        absolute_array_candidates=pe.refs(0x1890C80, False),
        disassembly_ranges=[dict(start=f'{a:08X}', end=f'{b:08X}') for a, b in sorted(ranges)],
        limitations=['Byte-reference candidates require confirmation at decoded instruction boundaries.',
            'Padding-derived starts are candidates, not debug symbols.',
            'Does not enumerate arbitrary computed addresses or indirect calls without a resolved table.'])
    decoded = {}
    for lo, hi in ranges:
        for line in (OUT/f'{lo:08X}-{hi:08X}.asm.txt').read_text(encoding='utf-8').splitlines():
            m = re.match(r'\s+([0-9A-F]{8}): (?:[0-9A-F]{2} )+\s+(call|jmp)\s+([0-9A-F]{8})\s*$',line)
            if m:
                decoded[int(m[1],16)] = (m[2],int(m[3],16))
    confirmed = []
    for name, candidates in incoming.items():
        for c in candidates:
            site = int(c['site'],16)
            assert decoded.get(site) == (c['opcode'], TARGETS[name]), c
            confirmed.append(dict(target=name,site=c['site'],opcode=c['opcode']))
    result['confirmed_incoming'] = confirmed
    spans = {
        0xBAE6D3: '8db590e01500', 0xBAE6E4: '8b8e00000200',
        0xBAE6EB: '8d3c8e', 0xBAE731: '8b068b088b118b026a00ffd0',
        0xBAE73D: '8b0e51b9e03b9b01e896670000',
        0xBAE74A: '83c6043bf775a1', 0xBAE753: 'c78590e0170000000000',
        0xBAE761: '898590e01700', 0xBAE939: 'c78690e0170000000000',
        0xBAEA60: '8b8190e017008d848190e01500',
        0xBAEA75: '8910ff8190e01700',
        0xBAEC17: '8b8790e017008b168d848790e01500',
        0xBAEC2A: '8910ff8790e01700',
        0xBAEC3E: 'ff156891dc00', 0xBAEC44: '8b4424108906',
        0xBAECDC: 'ff155c91dc0085c00f8596000000',
        0xBAED34: '8b8790e017008d848790e01500',
        0xBAED45: '89308b54241cff8790e01700',
        0xBAED7B: 'e820010000',
        0xDC3F50: 'b9f02b7301e8d6a9deff68307fdc00',
        0xDC7F30: 'c3',
    }
    for at, signature in spans.items():
        assert pe.read(at,len(bytes.fromhex(signature))) == bytes.fromhex(signature),hex(at)
    result['verified_spans'] = {f'{a:08X}':b for a,b in spans.items()}
    result['relocation_inventory'] = [
        dict(instruction='00BAE6D3',role='drain base',immediate_offset=2),
        dict(instruction='00BAEA66',role='direct append base',immediate_offset=3),
        dict(instruction='00BAEC1F',role='replacement append base',immediate_offset=3),
        dict(instruction='00BAED3A',role='zero-reference append base',immediate_offset=3),
        dict(instruction='00BAE6E4',role='drain indirect count reader',immediate_offset=2)]
    assert result['array_offset_candidates'] == ['00BAE6D5','00BAEA69','00BAEC22','00BAED3D']
    assert result['count_offset_candidates'] == ['00BAE6BB','00BAE755','00BAE763','00BAE93B',
        '00BAEA62','00BAEA79','00BAEC19','00BAEC2E','00BAED36','00BAED4D']
    assert imports['00DC915C']['name'] == 'InterlockedDecrement'
    assert imports['00DC9168']['name'] == 'InterlockedExchange'
    result['layout'] = dict(owner='01732BF0',array_offset='0015E090',count_offset='0017E090',
        capacity=32768,entry_bytes=4,count_bits=32,indirect_count_read='00BAE6E4',
        constructor='00BAE930',registered_exit_callback='00DC7F30',exit_callback_bytes='C3')
    result['native_order'] = dict(key_removal=[
        'Lookup keyed map value, obtain wrapper and pointee.',
        'InterlockedDecrement pointee+8; return if nonzero.',
        'Rewrite linked handles/recycle indexed slots.',
        'Append wrapper to deferred list, increment count.',
        'Optional short-key recycling, then unlink/recycle hash node.'],
        replacement=[
        'Lookup/match keyed record.', 'Append old wrapper and increment count.',
        'InterlockedExchange new pointee+8 with old reference count+1.',
        'Publish replacement wrapper and rewrite linked users.'],
        drain=[
        'Snapshot count and end pointer once.',
        'For each queued wrapper, call pointee virtual slot0 with argument0.',
        'Return wrapper to native allocator00BB4EE0.',
        'After full snapshot, reset native count to0.'])
    (OUT/'inventory.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(dict(result='passed',confirmed_incoming=len(confirmed),verified_spans=len(spans),
        incoming_counts={k:len(v) for k,v in incoming.items()}, imports=imports,
        relocation_sites=result['relocation_inventory'],disassembly_ranges=len(ranges)), indent=2))


if __name__ == '__main__':
    main()

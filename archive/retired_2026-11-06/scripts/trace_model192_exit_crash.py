"""Bounded static/dump follow-up to the supplied run260 exit fault."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from static_streaming_audit import PE, DUMPBIN, EXE
from audio_static_audit import EXPECTED

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'analysis/audio_model192_run260_exit_2026-11-05'


def main():
    summary = json.loads((OUT/'dump_summary.json').read_text())
    data = (ROOT/summary['dump']).read_bytes()
    pe = PE()
    assert hashlib.sha256(pe.data).hexdigest() == EXPECTED
    def read(address, size=4):
        for m in summary['memory_ranges']:
            start = int(m['start'], 16)
            if start <= address and address+size <= start+m['size']:
                off = m['rva']+address-start
                return data[off:off+size]
        return None
    def u32(address):
        b = read(address)
        return None if b is None else struct.unpack('<I', b)[0]
    r = {k: int(v, 16) for k, v in summary['exception']['registers'].items()}
    owner = r['edi']
    probes = {}
    addresses = {
        'resource_pending_count': owner+0x157AC8,
        'retirement_count': owner+0x17E090,
        'retirement_first': owner+0x15E090,
        'retirement_last': owner+0x17E08C,
        'indexed_free_head': owner+0x27E094,
        'retiring_object': r['esi'],
        'adjacent_previous_object': r['esi']-4,
        'retiring_hash_node': r['ebx'],
        'caller_handle_list': 0x1B04E958,
        'audio_provider': 0x1AA94158,
        'audio_buffer': 0x04EA0000,
        'audio_buffer_guard': 0x04EA0000+192*16,
    }
    for label, a in addresses.items():
        b = read(a, 16)
        probes[label] = dict(address=hex(a), value=None if u32(a) is None else hex(u32(a)),
                            bytes=None if b is None else b.hex(' '))
    # LEA truncates the scaled index to32 bits; four possible DWORD counts fit
    # a given address. Prefer a captured native count when present.
    residual = (r['eax']-owner-0x15E090) & 0xFFFFFFFF
    count_candidates = [residual//4+i*0x40000000 for i in range(4)] if residual%4 == 0 else []
    count = u32(owner+0x17E090)
    if count is not None:
        assert (owner+0x15E090+count*4) & 0xFFFFFFFF == r['eax']
    assert pe.read(0xBAED34, 19).hex() == '8b8790e017008d848790e0150085c074028930'
    evidence = {
        0xBAE6D3: '8db590e01500',  # drain base
        0xBAE6E4: '8b8e00000200',  # count relative to OLD inline base
        0xBAE753: 'c78590e0170000000000',
        0xBAEA60: '8b8190e017008d848190e01500',
        0xBAEA75: '8910ff8190e01700',
        0xBAEC17: '8b8790e017008b168d848790e01500',
        0xBAEC2A: '8910ff8790e01700',
        0xBAED45: '89308b54241cff8790e01700',
    }
    for address, signature in evidence.items():
        assert pe.read(address, len(bytes.fromhex(signature))) == bytes.fromhex(signature), hex(address)
    actual = read(0xBAED34, 19)
    if actual is not None:
        assert actual == pe.read(0xBAED34, 19)
    spans = [(0xBAE670,0xBAE780), (0xBAEA10,0xBAEAB0), (0xBAEBE0,0xBAED90),
             (0xBBA760,0xBBA7C0), (0xBB9290,0xBB96F0),
             (0xBAE930,0xBAE9AD), (0x8581E0,0x8582A9),
             (0x599900,0x599941), (0x599AB0,0x599ADC),
             (0x5A4AE0,0x5A4B4E), (0x5A529C,0x5A52AC)]
    for lo, hi in spans:
        result = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
            f'/range:0x{lo:X},0x{hi:X}', str(EXE)], capture_output=True, text=True, check=True)
        (OUT/f'{lo:08X}-{hi:08X}.asm.txt').write_text(result.stdout, encoding='utf-8')
    stack = int(summary['exception']['registers']['esp'], 16)
    # BAECA0 has pushed ECX/EBX/EBP/EDI/ESI at this point: saved registers
    # occupy20 bytes; the following DWORD is the actual immediate return.
    immediate_return = u32(stack+20)
    assert immediate_return == 0xBBA791
    assert pe.read(immediate_return-5, 5).hex() == 'e80f45ffff'
    # The two saved loop registers describe the remaining native handle batch.
    remaining_handles = (u32(stack+4)-u32(stack))//8
    # Demonstrate the native append's exact alias at index32768. This is a
    # small arithmetic replay, not evidence that the first overflow was captured.
    prior_pointer = r['esi']-4
    overwritten_count = (prior_pointer+1) & 0xFFFFFFFF
    next_destination = (owner+0x15E090+overwritten_count*4) & 0xFFFFFFFF
    saved = {name: hex(u32(stack+delta)) for name, delta in
             [('esi',0), ('edi',4), ('ebp',8), ('ebx',12)]}
    detail = dict(executable_sha256=EXPECTED, fault_address=hex(r['eip']),
        instruction='mov DWORD PTR [eax],esi', write_target=hex(r['eax']), written_pointer=hex(r['esi']),
        resource_owner=hex(owner), retirement_array_offset=hex(0x15E090),
        retirement_count_offset=hex(0x17E090), inline_pointer_capacity=(0x17E090-0x15E090)//4,
        captured_count=None if count is None else hex(count), count_candidates=[hex(c) for c in count_candidates],
        minimum_possible_count=min(count_candidates), probes=probes,
        remaining_handles_in_caller_batch=remaining_handles,
        boundary_replay=dict(index=32768, destination=hex(owner+0x15E090+32768*4),
            count_address=hex(owner+0x17E090), aliases_count=True,
            hypothetical_previous_pointer=hex(prior_pointer),
            count_after_native_store_and_increment=hex(overwritten_count),
            next_destination=hex(next_destination), matches_fault=next_destination==r['eax'],
            interpretation='Consistent with the immediately preceding4-byte object pointer overwriting the count; not a captured prior write.'),
        immediate_caller_return=hex(immediate_return), immediate_caller_call=hex(immediate_return-5),
        saved_caller_registers=saved,
        later_stack_candidates=[w for w in summary['exception']['stack_words'][10:40] if w['module']],
        fault_bytes_match_stock=actual is not None,
        count_displacement_references=pe.refs(0x17E090, False),
        array_displacement_references=pe.refs(0x15E090, False),
        verified_native_spans={hex(a): b for a, b in evidence.items()},
        limitations=['The minidump may omit the count/array and their prior writers.',
          'Out-of-range count at the failing append does not identify its first corrupting write.',
          'Only the immediate return is validated using the exact faulting function stack layout.'])
    (OUT/'crash_trace.json').write_text(json.dumps(detail, indent=2), encoding='utf-8')
    print(json.dumps({k: detail[k] for k in ('fault_address', 'instruction', 'write_target',
        'inline_pointer_capacity', 'captured_count', 'count_candidates', 'boundary_replay',
        'immediate_caller_return', 'remaining_handles_in_caller_batch', 'fault_bytes_match_stock')}, indent=2))


if __name__ == '__main__':
    main()

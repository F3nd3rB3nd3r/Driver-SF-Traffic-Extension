"""Static callback storage audit and algorithm replay; never runs Driver.exe."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from static_streaming_audit import PE, DUMPBIN, EXE

ORIGIN, STRIDE, SENTINEL, GROUPS = 0x14427EC, 0x314, 0x30C, 21
SHA256 = 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
ARENA_INSTRUCTIONS = [0x858A4A, 0x858FC2, 0x859013, 0x85901D,
                      0x859023, 0x859053, 0x859088, 0x8590EF, 0xDB40A5]
SPANS = [(0x5567D0, 0x556821), (0x5568F0, 0x5569C2),
         (0x556C66, 0x556C88), (0x556D5C, 0x556D7E), (0x5A3870, 0x5A3A32),
         (0x8CCE80, 0x8CCF66), (0x8589C0, 0x858A5E),
         (0x858B60, 0x858BB4), (0x858C90, 0x858CD0),
         (0x858F80, 0x858FF6), (0x859000, 0x859244),
         (0x859260, 0x85936E), (0x5A20C0, 0x5A21CE),
         (0xDB40A0, 0xDB40DC)]


class Replay:
    """Decoded insert/unlink memory operations, with a proposed allocation guard.

    The guard is a design hypothesis, not compiled production code. Native
    payload copies and pointer manipulation below follow the audited functions.
    """
    def __init__(self):
        self.memory = {}
        self.issued = [0] * GROUPS
        self.redirects = [0] * GROUPS
        for group in range(GROUPS):
            h = ORIGIN + group * STRIDE
            s = h + SENTINEL
            for at, value in ((h, h+12), (h+4, 0), (h+8, 0), (s, s), (s+4, s)):
                self.write(at, value)
            first, end = self.extra(group)
            self.write(first-4, 0xC0FFEE01)
            self.write(end, 0xC0FFEE02)

    def extra(self, group):
        first = 0x30000000 + group * 0x2000
        return first, first + 448 * 12

    def read(self, at):
        return self.memory.get(at, 0)

    def write(self, at, value):
        self.memory[at] = value

    def insert(self, group, payload):
        h, fresh = ORIGIN + group * STRIDE, False
        s = h + SENTINEL
        if not self.read(h+4):
            fresh = True
            if self.read(h) == s:
                assert self.issued[group] == 64
                self.write(h, self.extra(group)[0])
                self.redirects[group] += 1
            if self.issued[group] == 512:
                raise OverflowError('hypothetical guard refuses before native overwrite')
        # Native 005567D0, including recycled tail retaining the fresh cursor.
        node = self.read(h)
        following = self.read(node)
        self.write(node+8, payload)
        if self.read(h+4):
            self.write(h+4, self.read(h+4)-1)
            self.write(h, following)
        else:
            self.write(h, self.read(h)+12)
        self.write(h+8, self.read(h+8)+1)
        previous = self.read(s)
        self.write(node, previous)
        self.write(node+4, s)
        self.write(self.read(s)+4, node)
        self.write(s, node)
        self.issued[group] += int(fresh)
        return node

    def remove(self, group, node):
        # Native 0085903F..66: unlink, put on recycled chain, retain next iterator.
        h = ORIGIN + group * STRIDE
        previous, following = self.read(node), self.read(node+4)
        self.write(previous+4, following)
        self.write(following, previous)
        self.write(node, self.read(h))
        self.write(h+4, self.read(h+4)+1)
        self.write(h+8, self.read(h+8)-1)
        self.write(h, node)
        return following

    def validate(self, group):
        h = ORIGIN + group * STRIDE
        s = h + SENTINEL
        first, end = self.extra(group)
        def node_valid(node):
            return ((h+12 <= node < s and (node-h-12) % 12 == 0) or
                    (first <= node < end and (node-first) % 12 == 0))
        nodes, current, previous = [], self.read(s+4), s
        while current != s:
            assert len(nodes) < 512 and current not in nodes and node_valid(current)
            assert self.read(current) == previous
            nodes.append(current)
            previous, current = current, self.read(current+4)
        assert self.read(s) == previous and len(nodes) == self.read(h+8)
        free, current = [], self.read(h)
        for _ in range(self.read(h+4)):
            assert node_valid(current) and current not in nodes and current not in free
            free.append(current)
            current = self.read(current)
        assert len(nodes)+len(free) == self.issued[group]
        expected = h+12+self.issued[group]*12 if self.issued[group] <= 64 else first+(self.issued[group]-64)*12
        assert current == expected
        assert self.read(first-4) == 0xC0FFEE01 and self.read(end) == 0xC0FFEE02
        return nodes


def replay():
    r = Replay()
    # Every group at once: native headers/sentinels and adjacent groups retained.
    for group in range(GROUPS):
        for ordinal in range(512):
            r.insert(group, group*10000+ordinal+1)
        r.validate(group)
        before = r.memory.copy()
        try:
            r.insert(group, 999999)
            raise AssertionError('513th insertion was not refused')
        except OverflowError:
            assert r.memory == before
    for group in range(GROUPS):
        original = r.validate(group)
        for node in original[::2]:
            r.remove(group, node)
        r.validate(group)
        for _ in range(256):
            r.insert(group, 0xABCDEF)
        assert r.issued[group] == 512
        for node in r.validate(group):
            r.remove(group, node)
        assert r.validate(group) == []
        for _ in range(512):
            r.insert(group, 0x123456)
        r.validate(group)
        assert r.redirects[group] == 1
    # Recycling before crossing64 must preserve the unissued native cursor.
    partial = Replay()
    for _ in range(40):
        partial.insert(9, 1)
    for node in partial.validate(9)[::2]:
        partial.remove(9, node)
    for _ in range(130):
        partial.insert(9, 2)
    assert len(partial.validate(9)) == 150 and partial.issued[9] == 150
    partial.validate(10)
    return dict(result='passed', kind='Python decoded-operation design replay, not native fixture',
        groups=21, proposed_capacity=512, native_inline=64, private_extra=448,
        private_node_bytes_total=21*448*12, all_groups_full_and_adjacent_headers_valid=True,
        refused_513_before_mutation=True, alternating_removal_and_full_reuse=True,
        empty_refill_preserves_fresh_cursor=True, partial_recycle_before_inline_boundary=True,
        limitations='No production hook, native execution, callback side effects, concurrency or live demand validation.')


def census(pe):
    process = subprocess.Popen([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes', str(EXE)],
                               stdout=subprocess.PIPE, text=True)
    arena, stride, calls = [], [], []
    for line in process.stdout:
        match = re.match(r'\s*([0-9A-F]{8}):\s+((?:[0-9A-F]{2} )+)\s*(.*)', line)
        if not match:
            continue
        at, operand = int(match[1], 16), match[3].strip()
        values = [int(s, 16) for s in re.findall(r'\b([0-9A-F]+)h\b', operand)]
        row = dict(address=f'{at:08X}', text=line.strip())
        if any(ORIGIN <= x < ORIGIN+GROUPS*STRIDE for x in values):
            arena.append(row)
        if STRIDE in values and operand.startswith(('imul', 'add')):
            stride.append(row)
        if operand.startswith('call') and '005567D0' in operand:
            calls.append(row)
    assert process.wait() == 0
    assert [int(r['address'], 16) for r in arena] == ARENA_INSTRUCTIONS
    assert [int(r['address'], 16) for r in calls] == [0x55690F, 0x5A399A, 0x858FC8, 0x8CCF4C]
    data_candidates = []
    for section in pe.sections:
        if section['flags'] & 0x20000000:
            continue
        block = pe.data[section['offset']:section['offset']+section['size']]
        at = block.find(bytes.fromhex('4401'))
        while at >= 0:
            if at >= 2:
                value = struct.unpack_from('<I', block, at-2)[0]
                if ORIGIN <= value < ORIGIN+GROUPS*STRIDE:
                    data_candidates.append(dict(section=section['name'],
                        address=f'{section["va"]+at-2:08X}', value=f'{value:08X}'))
            at = block.find(bytes.fromhex('4401'), at+1)
    return dict(arena_operand_instructions=arena, stride314_arithmetic_instructions=stride,
        helper_direct_calls=calls, helper_absolute_byte_refs=pe.refs(0x5567D0, False),
        constructor_absolute_byte_refs=pe.refs(0xDB40A0, False),
        non_executable_raw_byte_candidates=data_candidates,
        limitations='Linear disassembly and literal-reference census; computed/indirect references are not exhaustively disproved.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--census', action='store_true', help='Also decode the EXE for literal reference candidates')
    args = parser.parse_args()
    pe = PE()
    assert hashlib.sha256(pe.data).hexdigest() == SHA256
    signatures = {0x5567D0:'837904008b01568d5008',
        0x5567FF:'83010c', 0x556805:'8b910c03000081c10c030000',
        0x858FB5:'8b8e6005000069c9140300005781c1ec274401',
        0x859031:'8b4f088b018b500cffd2',
        0x85905A:'8b108917ff4004ff480889388bf9',
        0xDB40A0:'ba14000000b8f0274401'}
    for at, expected in signatures.items():
        assert pe.read(at, len(bytes.fromhex(expected))) == bytes.fromhex(expected), hex(at)
    assert struct.unpack('<I', pe.read(0xE37118+0x24, 4))[0] == 0x859000
    args.output.mkdir(parents=True, exist_ok=True)
    for start, stop in SPANS:
        result = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
            f'/range:0x{start:X},0x{stop:X}', str(EXE)], capture_output=True, text=True, check=True)
        (args.output / f'{start:08X}.verified.asm.txt').write_text(result.stdout, encoding='utf-8')
    result = dict(result='passed', exe=str(EXE), sha256=SHA256,
        verified_signatures={f'{k:08X}':v for k,v in signatures.items()},
        geometry=dict(origin=hex(ORIGIN), exclusive_end=hex(ORIGIN+GROUPS*STRIDE),
            groups=GROUPS, stride=STRIDE, node_bytes=12, inline_capacity=(SENTINEL-12)//12),
        replay=replay())
    if args.census:
        result['reference_census'] = census(pe)
    (args.output / 'audit_summary.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps({k:v for k,v in result.items() if k != 'reference_census'}, indent=2))


if __name__ == '__main__':
    main()

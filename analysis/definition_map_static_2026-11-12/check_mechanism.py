"""Arithmetic/branch model of the inspected guard, not native execution."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
source = ROOT / 'src/V29sNativeLoadingTrace.inc'
text = source.read_text()
assert 'kSRegMapArenas=4u,kSRegMapNodes=512u' in text
assert 'if(Read32(map+4u)!=0u)return;' in text
assert 'if(cursor==inlineEnd)' in text
assert 'else if(arena)' in text

def guard(map_address, cursor, free_count, arenas):
    # Direct transcription of the relevant branch conditions; not an emulator.
    if free_count:
        return 'recycledBypass'
    arena = arenas.get(map_address)
    if cursor == map_address + 0x444:
        if arena is None and len(arenas) == 4:
            return 'ownerSlotsFull'
        return 'redirect'
    if arena is not None:
        if arena <= cursor <= arena + 512 * 24 and (cursor - arena) % 24 == 0:
            return 'externalEnd' if cursor == arena + 512 * 24 else 'externalOkay'
        return 'unexpectedCursor'
    return 'uncheckedInlineOrOther'

owners = {0x10000000 + i * 0x10000: 0x30000000 + i * 0x10000 for i in range(4)}
owner = next(iter(owners))
checks = {
    'inline_nodes': (0x444 - 0xC) // 24,
    'four_owner_fifth_crossing': guard(0x10040000, 0x10040444, 0, owners),
    'same_address_constructor_first_insert': guard(owner, owner + 0xC, 0, owners),
    'same_address_constructor_inline_crossing': guard(owner, owner + 0x444, 0, owners),
    'ordinary_external_insert': guard(owner, owners[owner] + 105 * 24, 0, owners),
    'external_capacity_guard_retained': guard(owner, owners[owner] + 512 * 24, 0, owners),
    'recycled_path_skipped': guard(owner, owner + 0xC, 1, owners),
    'backing_bytes_per_owner': 512 * 24,
    'existing_total_node_storage_per_owner': 45 + 512,
}
assert checks['inline_nodes'] == 45
assert checks['four_owner_fifth_crossing'] == 'ownerSlotsFull'
assert checks['same_address_constructor_first_insert'] == 'unexpectedCursor'
assert checks['same_address_constructor_inline_crossing'] == 'redirect'
assert checks['ordinary_external_insert'] == 'externalOkay'
assert checks['external_capacity_guard_retained'] == 'externalEnd'
output = dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
    scope='Branch/arithmetic model only; no Driver.exe execution or live branch attribution.',
    checks=checks)
Path(__file__).with_name('mechanism.json').write_text(json.dumps(output, indent=2) + '\n')
print(json.dumps(output, indent=2))

"""Focused byte/data checks for the static audit, with no build or game run."""
import hashlib, json, struct, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'scripts'))
from static_streaming_audit import PE, EXE
p = PE()
digest = hashlib.sha256(p.data).hexdigest()
assert digest == 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
checks = []
def exact(at, data, label):
    b = bytes.fromhex(data)
    assert p.read(at, len(b)) == b, (hex(at), label)
    checks.append(dict(address=f'{at:08X}', check=label, bytes=b.hex(' ')))
exact(0x746940, '56 8b f1 8b 06 8b 50 0c ff d2 84 c0', 'reader polls native backend before commit/parse')
exact(0x746A0B, '83 c0 10 3b f8 72 67', 'parser waits for entire header plus payload')
exact(0x746ADB, '0f 82 02 01 00 00', 'insufficient wrap assembly space returns before issuing read')
exact(0x746B62, '3d 00 80 00 00 76 7a', 'ordinary refill requires more than32KiB free')
exact(0x746BCD, '68 00 80 00 00 51 8b ce ff d2', 'ordinary I/O request is32KiB')
exact(0x746BF1, '8b 99 6c 85 00 00', 'retention walk starts from parse cursor')
exact(0x746C87, '89 99 64 85 00 00', 'retention walk updates oldest retained pointer')
exact(0x746DE2, '8b 40 0c 89 46 0c', 'per-client dequeue advances chain head')
exact(0x73A453, 'e8 18 c9 00 00', 'updater reconciles per-client chains before tail refill')
exact(0x73A714, 'e9 87 c5 00 00', 'updater tail dispatches recording service')
exact(0x7546E0, '8b d0 89 46 04 83 7a 0c 00', 'client3 advances its current sample before requiring successor')
exact(0x7549D0, '8b 41 04 85 c0 74 09 83 78 0c 00 74 03', 'client3 busy tests current and successor presence')
exact(0x745EA0, '8b 91 5c 85 00 00', 'cursor helper uses capacity field, not256KiB mask')
exact(0x73F941, 'e8 6a 7a 11 00', 'file reader delegates to8573B0')
exact(0x73FB37, 'e8 a4 79 11 00', 'file busy virtual delegates to8574E0')
exact(0x8573F0, 'e8 1b 39 00 00', 'file read delegates to85AD10')
exact(0x85AD70, 'e8 bb fd ff ff', 'read schedules native I/O through85AB30')
exact(0x736D79, '8d 81 00 00 04 00', 'shared second buffer starts at service+40000')
exact(0x736E56, '8d 81 00 00 08 00', 'independent next region starts at service+80000')
exact(0x73991D, 'e8 de d4 00 00', 'failure path drains recording before destructor/provider release')
exact(0x73935A, 'e8 a1 da 00 00', 'background completion drains recording before destructor/provider release')
exact(0x73AD4C, 'e8 af c0 00 00', 'mode teardown drains recording before destructor/provider release')
constructors = []
for at, mode, capacity, provider, mode_at, capacity_at, provider_at in [
        (0x739705,0,0x40000,0x736CB0,0x7396EF,0x7396F1,0x7396F8),
        (0x739770,0,0x40000,0x736C00,0x739755,0x739757,0x73975E),
        (0x7397AC,0,0x40000,0x736C00,0x739796,0x739798,0x73979F),
        (0x739AB9,1,0x80000,0x736C00,0x739AA5,0x739AA7,0x739AAC),
        (0x73B048,1,0x40000,0x736D60,0x73B02F,0x73B031,0x73B038)]:
    assert p.read(at, 1) == b'\xe8'
    assert at + 5 + struct.unpack('<i', p.read(at + 1, 4))[0] == 0x73FAB0
    assert p.read(mode_at, 2) == bytes([0x6A, mode])
    assert p.read(capacity_at, 5) == b'\x68' + struct.pack('<I', capacity)
    assert p.read(provider_at, 1) == b'\xe8'
    assert provider_at + 5 + struct.unpack('<i', p.read(provider_at + 1, 4))[0] == provider
    constructors.append(dict(call=f'{at:08X}', mode=mode, capacity=capacity, provider=f'{provider:08X}'))
vt = struct.unpack('<10I', p.read(0xE2A088,40))
assert vt[3] == 0x73FB30 and vt[5] == 0x73F930 and vt[8] == 0x73F9F0
spans = json.loads((OUT/'boundary_spans.json').read_text())
assert len(spans['pairs']) == 7 and spans['adjacent_sample_spans'][1]['summed_ring_distances'] == 345212
artifacts = {}
for name, expected in [('REGMAP_REUSE_R2','8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa'),
                       ('REPLAY_HITCH_R1','758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8'),
                       ('REPLAY_CLIENT_R1','d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3')]:
    path = ROOT/'dist/MAIN_150'/f'DriverSF_V29s_160Capacity_150Request_{name}.asi'
    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    assert actual == expected, name
    artifacts[str(path.relative_to(ROOT))] = actual
result = dict(result='passed', executable=str(EXE), exe_sha256=digest, checks=checks,
              file_constructor_paths=constructors, staged_artifact_sha256=artifacts,
              boundary_span_checks='passed', build=False, game_run=False, debugger=False)
(OUT/'audit_checks.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k not in ('checks','staged_artifact_sha256')},indent=2))
print('native byte checks:',len(checks),'; staged ASIs unchanged:',len(artifacts))

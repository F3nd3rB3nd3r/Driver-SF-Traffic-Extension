"""Read only run30212 and derive ring spans; never interpret spans as disk bytes."""
import hashlib, json, struct
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
log = ROOT / 'dist/MAIN_150/DSFV29s_replay_client_30212.log'
raw = log.read_bytes()
assert hashlib.sha256(raw).hexdigest() == '3ce160af071cfbc49a769d093728d6dc60017de82cc1b08f07bb880b87e40954'
HEX = {'clockBits', 'curTimeBits', 'nextTimeBits', 'curPtr', 'nextPtr', 'base', 'retained', 'filled', 'parse'}
def value(r, k):
    return int(r[k], 16 if k in HEX else 10)
def time(r, k):
    return struct.unpack('<f', struct.pack('<I', value(r, k)))[0]
events = []
for line in raw.decode().splitlines():
    if not line.startswith('DSFV29s_REPLAY_CLIENT_EVENT '):
        continue
    r = dict(x.split('=', 1) for x in line.split()[1:] if '=' in x)
    if r['client'] == '3' and r['phase'] == r['transition'] == r['loading'] == '0':
        events.append(r)
starts = {r['sequence']: r for r in events if r['kind'] == 'busy'}
pairs = []
for r in events:
    if r['kind'] != 'clear':
        continue
    begin = starts[r['sequence']]
    capacity = value(r, 'capacity')
    distance = lambda a, b: (b - a) % capacity
    retained, cur, nxt, filled = [value(r, k) for k in ('retained', 'curPtr', 'nextPtr', 'filled')]
    used = capacity - value(r, 'freeBytes')
    assert distance(retained, cur) + 40 <= used
    assert distance(retained, nxt) + 40 <= used
    assert value(begin, 'freeBytes') > 32768
    assert value(begin, 'inFlightBytes') == 0
    until_end = value(begin, 'base') + capacity - value(begin, 'parse')
    unparsed = value(begin, 'unparsedBytes')
    # If fewer than16 bytes are available, header assembly (when needed) only
    # asks for32KiB free; all such observed starts have more than that.
    header_only_fits = unparsed < 16 and value(begin, 'freeBytes') >= 32768
    pairs.append(dict(sequence=int(r['sequence']), clock=time(begin, 'clockBits'),
                      current_time=time(r, 'curTimeBits'), next_time=time(r, 'nextTimeBits'),
                      current=f'{cur:08X}', next=f'{nxt:08X}', capacity=capacity,
                      current_to_next_ring_distance=distance(cur, nxt),
                      retained_before_current=distance(retained, cur),
                      clear_used_bytes=used, begin_unparsed=unparsed,
                      begin_bytes_to_ring_end=until_end,
                      header_only_assembly_has_space=header_only_fits,
                      begin_free=value(begin, 'freeBytes'), clear_free=value(r, 'freeBytes')))
adjacent = []
for a, b in zip(pairs, pairs[1:]):
    if a['next'] != b['current'] or a['next_time'] != b['current_time']:
        continue
    total = a['current_to_next_ring_distance'] + b['current_to_next_ring_distance']
    adjacent.append(dict(sequences=[a['sequence'], b['sequence']],
                         times=[a['current_time'], a['next_time'], b['next_time']],
                         summed_ring_distances=total, capacity=a['capacity'],
                         exceeds_capacity=total > a['capacity']))
result = dict(log_sha256=hashlib.sha256(raw).hexdigest(), pairs=pairs,
              adjacent_sample_spans=adjacent,
              caveat='Modulo physical ring distances include layout/relocation gaps; they are not exact serialized byte sizes, simultaneous occupancy, or proof of a prior capacity stall.')
(OUT / 'boundary_spans.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))

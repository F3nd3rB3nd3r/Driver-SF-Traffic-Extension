"""Generate exact native traffic-profile fingerprints; read-only game inputs."""
import hashlib
import json
from pathlib import Path
from static_streaming_audit import PE
from audit_traffic_chapter_assets import decode, INSTALLED_SHA, ORIGINAL_SHA

ROOT = Path(__file__).resolve().parents[1]
MEDIA = Path(r'E:\Games\Driver San Francisco Save Bug Test\media')
OUT = ROOT / 'analysis/chapters_r1_2026-11-06'


def main():
    summary = json.loads((ROOT / 'analysis/traffic_docs_chapter_audit_2026-11-06/summary.json').read_text())
    names = {int(r['hash'], 16): r['name'] for r in summary['records'] if r['name']}
    raw = (MEDIA / 'san_franciscoAI.fchunk').read_bytes()
    assert hashlib.sha256(raw).hexdigest() == INSTALLED_SHA
    original_raw = (MEDIA / 'san_franciscoAI_original.fchunk').read_bytes()
    assert hashlib.sha256(original_raw).hexdigest() == ORIGINAL_SHA
    current, original = decode(raw, names), decode(original_raw, names)
    roster = summary['uniform_roster']
    assert len(roster) == 150
    pe = PE()
    assert hashlib.sha256(pe.data).hexdigest() == 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    setter = pe.read(0x76FDF0, 0xC6)
    assert setter[-3:] == bytes.fromhex('c20800')
    assert pe.read(0x76FE95, 5) == bytes.fromhex('e8567be3ff')
    profiles = []
    for record in current['records']:
        assert record['slot'] == record['selector']
        for f in record['frequencies']:
            expand = all(current['tables'][i]['positive_ids'] == roster and
                         current['tables'][i]['positive_weights'] == [1.0] * 150 and
                         current['tables'][i]['total'] == 150.0 for i in f['tables'])
            assert len(f['request_ids']) <= 27 and len(f['tables']) <= 17
            if expand:
                assert set(f['request_ids']) <= set(roster)
            profiles.append(dict(slot=record['slot'], hash=record['hash'], name=record['name'],
                                 frequency=f['frequency'], ids=f['request_ids'], tables=f['tables'], expand=expand))
    assert len(profiles) == 80 and sum(p['expand'] for p in profiles) == 55
    assert sum(p['expand'] and p['name'] is not None for p in profiles) == 51
    row_ids = []
    import struct
    for row in range(191):
        row_ids.append(struct.unpack_from('<I', raw, 0x2DE440 + row * 8)[0])
    lines = ['#pragma once', '// Generated from installed AI SHA256 ' + INSTALLED_SHA + '.',
             '// All 80 active native profiles; expand only the 55 already using uniform150.',
             'struct ChapterProfile {U32 hash,frequency,slot,count,tableCount,expand;U16 ids[27];U8 tables[17];};',
             'static const U32 kChapterProfileCount=80u,kChapterExpandedProfiles=55u;',
             'static const ChapterProfile kChapterProfiles[80]={']
    for p in profiles:
        ids = ','.join(str(n)+'u' for n in p['ids'])
        tables = ','.join(str(n)+'u' for n in p['tables'])
        lines.append('    {0x'+p['hash']+'u,'+','.join(str(n)+'u' for n in
                     (p['frequency'], p['slot'], len(p['ids']), len(p['tables']), int(p['expand'])))+
                     ',{'+ids+'},{'+tables+'}},')
    lines += ['};', 'static const U32 kChapterRowIds[191]={'+','.join(str(n)+'u' for n in row_ids)+'};',
              'static const U32 kChapterRowWeights[191]={'+','.join('0x3F800000u' if n in roster else '0u' for n in row_ids)+'};',
              'static const U8 kChapterSetterBytes[198]={'+','.join('0x%02Xu' % n for n in setter)+'};']
    target = ROOT / 'src/V29sChapterProfiles.h'
    target.write_text('\n'.join(lines)+'\n', encoding='utf-8')
    OUT.mkdir(parents=True, exist_ok=True)
    evidence = dict(result='passed', installed_sha256=INSTALLED_SHA, original_sha256=ORIGINAL_SHA,
                    profiles=profiles, roster=roster, header_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
                    native_setter_sha256=hashlib.sha256(setter).hexdigest(),
                    native_setter_bytes=setter.hex(), native_call='0076FE95', return_address='0076FE9A',
                    other_direct_calls=['0066E5F3','0074BA58'])
    (OUT/'profiles.json').write_text(json.dumps(evidence, indent=2)+'\n', encoding='utf-8')
    print('Generated80 profiles:51 named +4 unnamed uniform150 targets;25 native profiles preserved.')


if __name__ == '__main__':
    main()

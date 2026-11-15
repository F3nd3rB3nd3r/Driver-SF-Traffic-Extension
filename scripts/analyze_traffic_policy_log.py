"""Check roster policy in one named log; run inherited analyzers separately."""
import argparse
import json
import re
from pathlib import Path
from compare_audio_slots_logs import rows


def analyze(path):
    path = Path(path)
    records = rows(path)
    of = lambda name: [r for r in records if r['type'] == 'DSFV29s_' + name]
    starts, chapters = of('TRAFFIC_POLICY_START'), of('CHAPTER_START')
    mode = starts[0].get('mode') if len(starts) == 1 else None
    valid_reasons = ('configured', 'missingFile', 'noSetting')
    checks = dict(
        policy_startup=len(starts) == 1 and mode in ('Native', 'Preset150') and all(
            starts[0].get(k) == v for k, v in dict(revision='1', requestCapacity='150',
                graphics='160', capacityChanges='0', liveReload='0',
                file='DriverSF_Traffic.ini', location='asiDirectory').items()),
        configuration_valid=len(starts) == 1 and starts[0].get('reason') in valid_reasons and
            (mode != 'Preset150' or starts[0].get('reason') == 'configured'),
        chapter_startup=len(chapters) == 1 and chapters[0].get('status') == '1' and
            chapters[0].get('policy') == mode and
            chapters[0].get('assetCheckedOnRequest') == ('1' if mode == 'Preset150' else '0'),
    )
    header = (Path(__file__).resolve().parents[1] / 'src/Chapter7Preload.h').read_text()
    preset = sorted(map(int, re.findall(r'(\d+)u', re.search(
        r'kChapter7AllPositiveIds\[150\] = \{(.*?)\};', header, re.S)[1])))
    roster_rows = of('BASELINE_ROSTER')
    parsed = []
    for event in of('BASELINE_PHASE'):
        if event.get('kind') != 'accepted':
            continue
        def roster(kind):
            matches = [r for r in roster_rows if r.get('seq') == event['seq'] and r.get('kind') == kind]
            return [int(n) for n in matches[0].get('ids', '').split(',') if n] if len(matches) == 1 else None
        original, effective = roster('original'), roster('effective')
        count, original_count = int(event['count']), int(event['originalCount'])
        complete = (effective is not None and 0 <= count <= 150 and len(effective) == count and
            all(0 <= n <= 65535 for n in effective) and original is not None and
            len(original) == max(0, original_count))
        preserved = complete and count == max(0, original_count) and original == effective
        preset_applied = complete and effective == preset and event.get('caller') == '0076FE9A'
        parsed.append(dict(seq=event['seq'], count=count, original_count=original_count,
            complete=complete, preserved=preserved, preset_applied=preset_applied))
    events = of('CHAPTER_EVENT')
    allowed = {'expanded', 'otherCaller', 'empty', 'contextUnavailable', 'unknownProfile',
        'nativeProfile', 'ownerMismatch', 'sourceMismatch', 'assetMismatch',
        'graphicsNotReady', 'admissionMismatch', 'nativeRoster'}
    checks['accepted_rosters_complete'] = bool(parsed) and all(r['complete'] for r in parsed)
    checks['accepted_rosters_follow_policy'] = bool(parsed) and all(
        r['preserved'] or (mode == 'Preset150' and r['preset_applied']) for r in parsed)
    checks['events_follow_policy'] = bool(events) and all(
        e.get('reason') in allowed and
        (e.get('reason') != 'expanded' or (mode == 'Preset150' and
            e.get('effectiveCount') == '150' and e.get('caller') == '0076FE9A')) and
        (e.get('reason') == 'expanded' or
            int(e['effectiveCount']) == max(0, int(e['nativeCount']))) and
        (mode != 'Native' or e.get('reason') in ('nativeRoster', 'otherCaller', 'empty', 'contextUnavailable'))
        for e in events)
    checks['complete_records'] = path.read_bytes().endswith(b'\n')
    return dict(log=str(path), mode=mode, checks=checks, policy_checks_pass=all(checks.values()),
        requests=parsed, events=events,
        rejected_requests=[r for r in of('BASELINE_PHASE') if r.get('kind') not in ('accepted', 'sample', 'superseded')],
        limitations=['Policy checks do not replace baseline/callback/READ1M integrity checks.',
            'Logged accepted lists are not proof of READY/drawable residency or audible playback.',
            'Native mode cannot repair modified assets whose positive weights exceed their preload list.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = analyze(args.log)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(mode=result['mode'], checks=result['checks'], policy_checks_pass=result['policy_checks_pass']), indent=2))


if __name__ == '__main__':
    main()

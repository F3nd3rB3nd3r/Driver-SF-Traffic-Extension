"""Analyze exactly two supplied chapter logs; no discovery or target execution."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

from compare_audio_slots_logs import rows
from compare_cleanup_logs import analyze

NAMES = {'AE88E83B': 'Chapter1', '47EB4D0E': 'Chapter7',
         'D433D631': 'Challenge', 'EF19694A': 'Exposition'}


def summarize(path):
    data = path.read_bytes()
    records = rows(path)
    def of(kind):
        return [r for r in records if r['type'] == 'DSFV29s_' + kind]
    inherited = analyze(path)
    model = inherited['model']
    events = of('CHAPTER_EVENT')
    audio = of('AUDIO_MODEL_STATUS')
    cap = of('CAP_STATUS')
    transitions = []
    for i, event in enumerate(events):
        tick = int(event['tick'])
        next_tick = int(events[i+1]['tick']) if i+1 < len(events) else None
        before = [r for r in audio if int(r['tick']) < tick]
        after = [r for r in audio if int(r['tick']) >= tick and
                 (next_tick is None or int(r['tick']) < next_tick)]
        first_fail = next((r for r in after if int(r['setupFail']) >
                          int(before[-1]['setupFail'] if before else 0)), None)
        transitions.append(dict(event, setting=NAMES.get(event['hash'], 'unmapped'),
            post_event_cap_samples=sum(int(r['tick']) >= tick and
                (next_tick is None or int(r['tick']) < next_tick) for r in cap),
            setup_failure_delta=int(after[-1]['setupFail']) -
                int(before[-1]['setupFail'] if before else 0) if after else None,
            first_increased_failure_status=first_fail))
    chapter_start = of('CHAPTER_START')
    chapter_status = of('CHAPTER_STATUS')
    checks = dict(startup=len(chapter_start) == 1 and chapter_start[0]['status'] == '1',
        all_recorded_requests_expanded=bool(events) and all(
            r['reason'] == 'expanded' and r['effectiveCount'] == '150' and
            r['caller'] == '0076FE9A' for r in events),
        no_sampled_refusals=bool(chapter_status) and all(
            r[k] == '0' for r in chapter_status for k in
            ('otherCaller', 'empty', 'unknown', 'nativeProfile', 'refused')))
    pending = of('PENDING_STATUS')
    pending_errors = ('invalid', 'full', 'allocationFailed', 'guardBad')
    return dict(log=str(path), bytes=len(data), sha256=hashlib.sha256(data).hexdigest(),
        lines=len(data.splitlines()), complete_final_newline=data.endswith(b'\n'),
        final_line=data.decode('utf-8-sig').splitlines()[-1],
        last_recorded_tick=max(int(r['tick']) for r in records if 'tick' in r),
        fatal_records=[r for r in records if 'FATAL' in r['type']],
        chapter_checks=checks, chapter_start=chapter_start, chapter_final=chapter_status[-1],
        event_count=len(events), event_reasons=dict(Counter(r['reason'] for r in events)),
        owners=sorted({r['owner'] for r in events}), transitions=transitions,
        final_cap=cap[-1],
        inherited_structural_checks_pass=inherited['valid'],
        inherited_startup_checks=model['baseline']['startup_checks'],
        cleanup=dict(checks=inherited['checks'], final=inherited['final'],
            attempts_minus_drained=inherited['attempts_minus_drained'],
            final_drain_balance=inherited['final_drain_balance']),
        pending=dict(start=of('PENDING_START'), final=pending[-1],
            exercised=any(int(r['attempts']) or int(r['walkStarts']) for r in pending),
            sampled_errors={k:max(int(r[k]) for r in pending) for k in pending_errors}),
        audio=dict(totals=model['model_totals'], final=model['final_status'],
            registries=model['registries'], models_with_failures=model['model_failures'],
            slots=model['slots'], pressure=model['baseline']['default_pressure']),
        limitations=[
            'Chapter event is emitted before native request processing; admission is not completion.',
            'Inherited structural validity does not imply audible playback or crash-free execution.',
            'Setup failures are native return outcomes, not a decoded failure reason or audible-loss proof.',
            'No exception context or callback-arena occupancy is recorded by these logs.',
            'Traffic-setting names do not prove actual chapter/save-manager coverage.',
            'Request and registry counts do not establish simultaneous drawable vehicles.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('logs', nargs=2, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    results = []
    for path in args.logs:
        result = summarize(path)
        (args.output / (path.stem + '.json')).write_text(json.dumps(result, indent=2), encoding='utf-8')
        results.append({k: result[k] for k in ('log', 'sha256', 'event_count', 'chapter_checks',
            'last_recorded_tick', 'final_line', 'inherited_structural_checks_pass')})
        results[-1]['setup_failures'] = result['audio']['totals']['setupFail']
        results[-1]['pending_exercised'] = result['pending']['exercised']
    (args.output / 'comparison.json').write_text(json.dumps(results, indent=2), encoding='utf-8')
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()

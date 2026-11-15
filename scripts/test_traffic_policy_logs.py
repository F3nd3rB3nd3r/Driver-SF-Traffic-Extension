"""Synthetic log checks only: no compiler, game, native code or old logs."""
import json
import re
import tempfile
from pathlib import Path
from analyze_traffic_policy_log import analyze


def run():
    with tempfile.TemporaryDirectory(prefix='dsf_policy_logs_') as directory:
        path = Path(directory) / 'fixture.log'
        def check(text, passed, key=None):
            path.write_text(text, encoding='utf-8')
            result = analyze(path)
            assert result['policy_checks_pass'] == passed, result
            if key:
                assert not result['checks'][key], key
            return result
        native = ('DSFV29s_TRAFFIC_POLICY_START revision=1 mode=Native reason=configured '
            'file=DriverSF_Traffic.ini location=asiDirectory requestCapacity=150 graphics=160 capacityChanges=0 liveReload=0\n'
            'DSFV29s_CHAPTER_START status=1 policy=Native assetCheckedOnRequest=0\n'
            'DSFV29s_CHAPTER_EVENT reason=nativeRoster caller=0076FE9A nativeCount=3 effectiveCount=3\n'
            'DSFV29s_BASELINE_PHASE kind=accepted seq=1 caller=0076FE9A originalCount=3 count=3\n'
            'DSFV29s_BASELINE_ROSTER seq=1 kind=original ids=130,62,123\n'
            'DSFV29s_BASELINE_ROSTER seq=1 kind=effective ids=130,62,123\n')
        check(native, True)
        check(native.replace('reason=configured', 'reason=missingFile'), True)
        check(native.replace('reason=configured', 'reason=invalidMode'), False, 'configuration_valid')
        check(native.replace('kind=effective ids=130,62,123', 'kind=effective ids=62,123,130'), False, 'accepted_rosters_follow_policy')
        check(native.replace('kind=effective ids=130,62,123', 'kind=effective ids=130,62'), False, 'accepted_rosters_complete')
        check(native.replace('effectiveCount=3', 'effectiveCount=150').replace('reason=nativeRoster', 'reason=expanded'), False, 'events_follow_policy')
        check(native.replace('requestCapacity=150', 'requestCapacity=700'), False, 'policy_startup')
        check(native.rstrip('\n'), False, 'complete_records')
        check(native.replace('status=1 policy=', 'status=-1 policy='), False, 'chapter_startup')
        check(native + native.splitlines()[0] + '\n', False, 'policy_startup')
        # A valid noncanonical150 list is accepted and order is preserved.
        custom = ','.join(str(n) for n in range(65000, 65150))
        check(native.replace('nativeCount=3 effectiveCount=3', 'nativeCount=150 effectiveCount=150')
            .replace('originalCount=3 count=3', 'originalCount=150 count=150').replace('130,62,123', custom), True)
        # Empty and negative native requests retain native clear semantics.
        for count in (0, -1):
            check(native.replace('reason=nativeRoster', 'reason=empty')
                .replace('nativeCount=3 effectiveCount=3', f'nativeCount={count} effectiveCount=0')
                .replace('originalCount=3 count=3', f'originalCount={count} count=0').replace('130,62,123', ''), True)
        preset = native.replace('mode=Native', 'mode=Preset150').replace('policy=Native', 'policy=Preset150')
        preset = preset.replace('assetCheckedOnRequest=0', 'assetCheckedOnRequest=1').replace('reason=nativeRoster', 'reason=otherCaller')
        check(preset, True)
        check(preset.replace('reason=configured', 'reason=missingFile'), False, 'configuration_valid')
        header = (Path(__file__).resolve().parents[1] / 'src/Chapter7Preload.h').read_text()
        canonical = ','.join(map(str, sorted(map(int, re.findall(r'(\d+)u', re.search(
            r'kChapter7AllPositiveIds\[150\] = \{(.*?)\};', header, re.S)[1])))))
        expanded = preset.replace('reason=otherCaller', 'reason=expanded').replace('effectiveCount=3', 'effectiveCount=150')
        expanded = expanded.replace('originalCount=3 count=3', 'originalCount=3 count=150').replace(
            'kind=effective ids=130,62,123', 'kind=effective ids=' + canonical)
        check(expanded, True)
        check(expanded.replace('0076FE9A', '0066E5F8'), False, 'accepted_rosters_follow_policy')
        check(expanded.replace('kind=effective ids=' + canonical, 'kind=effective ids=' + custom),
            False, 'accepted_rosters_follow_policy')
    return dict(result='passed', checks=['native IDs and order preserved', 'noncanonical150 preserved',
        'empty/negative clear semantics', 'configuration failure surfaced', 'missing file default',
        'wrong mode/capacity/roster/truncation/preflight rejected', 'preset must be explicitly configured',
        'canonical preset expansion and caller/roster guards'],
        scope='Python log parser only; production C++ not executed')


if __name__ == '__main__':
    result = run()
    output = Path(__file__).resolve().parents[1] / 'analysis/traffic_policy_r1_2026-11-14/parser_tests.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result, indent=2))

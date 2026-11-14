"""Reproduce run28136 timing comparison without modifying the supplied log."""
from pathlib import Path
from collections import Counter
import hashlib,json,statistics,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts'))
from analyze_replay_hitch_log import number as n,floating as f
OUT=Path(__file__).resolve().parent
LOG=ROOT/'dist/MAIN_150/DSFV29s_replay_hitch_28136.log'
raw=LOG.read_bytes();records=[]
for line_no,line in enumerate(raw.decode(errors='replace').splitlines(),1):
    if not line.startswith('DSFV29s_REPLAY_HITCH_EVENT'):continue
    r=dict(x.split('=',1) for x in line.split()[1:] if '=' in x);r['line']=line_no;records.append(r)
windows=[r for r in records if r['kind']=='window']
waits=[r for r in records if r['kind']=='gateWait']
calls=[r for r in records if r['kind']=='call']
def compare(lo,hi):
    rows=[r for r in windows if lo<=n(r,'tick')<=hi]
    observed=[r for r in waits if lo<=n(r,'tick')<=hi]
    operation_calls=[r for r in calls if lo<=n(r,'tick')<=hi]
    sums=('spanMs','stepCalls','advanced','unchanged','busy20','busy24','subscriberOnlyBusy','snapshotPendingAtBusy','trafficPendingAtBusy')
    result={'range_ms':[lo,hi],'windows':len(rows),'totals':{k:sum(n(r,k) for r in rows) for k in sums},
            'clock_first':f(n(rows[0],'clockBits')) if rows else None,'clock_last':f(n(rows[-1],'clockBits')) if rows else None,
            'observer_body_ms':sum(n(r,'observerBodyTicks') for r in rows)/10000,
            'waits':len(observed),'wait_duration_ms':[n(r,'duration')/10000 for r in observed],
            'operations':dict(Counter(r['op'] for r in operation_calls))}
    if observed:
        ticks=[n(r,'tick') for r in observed];durations=result['wait_duration_ms']
        result['wait_summary']={'minimum_ms':min(durations),'median_ms':statistics.median(durations),'maximum_ms':max(durations),
                                'median_end_spacing_ms':statistics.median(b-a for a,b in zip(ticks,ticks[1:])) if len(ticks)>1 else None,
                                'busy_intervals_with_idle_fallback':sum(n(r,'phaseBefore')==n(r,'phase')==0 and n(r,'pendingTrafficBefore')==n(r,'pendingTraffic')==0 and n(r,'spoolStateBefore')!=1 and n(r,'spoolState')!=1 for r in observed)}
    return result
result={'file':{'bytes':len(raw),'lines':raw.count(b'\n'),'sha256':hashlib.sha256(raw).hexdigest(),'complete_final_newline':raw.endswith(b'\n')},
        'pre_chop_10_windows':compare(402718,411718),
        'late_chop_8_windows':compare(412718,419718),
        'late_gate_waits_all':compare(412000,419812),
        'first_playback':compare(255062,425250),
        'rewind_segment':compare(426375,431671),
        'resumed_earlier_playback':compare(431718,439718)}
states=[r for r in records if r['kind']=='state']
result['rate_changes']=[{'tick':n(r,'tick'),'clock':f(n(r,'clockBits')),'requested':f(n(r,'requestedBits')),'applied':f(n(r,'appliedBits')),'paused':n(r,'paused')} for r in states if r['requestedBits']!=r['requestedBeforeBits'] or r['appliedBits']!=r['appliedBeforeBits']]
baseline=json.loads((OUT/'baseline.json').read_text());callback=json.loads((OUT/'callback.json').read_text());hitch=json.loads((OUT/'hitch.json').read_text())
result['integrity']={'baseline_checks':all(baseline['checks'].values()),'callback':callback['recorded_integrity_pass'],'hitch':hitch['structural_pass']}
result['rosters']={'accepted_phases':len(baseline['phases']),'ready_sweeps':sum(p.get('all_ready_in_recorded_sweep',False) for p in baseline['phases'])}
result['caveats']=['Window counters can include activity before their ending timestamp; gateWait endpoints give finer event timing.',
                   'No caller/subscriber identity is recorded for gate waits; direct caller paths are established separately by static analysis.',
                   'Observer body time is elapsed wall time, not exclusive CPU usage or a complete overhead benchmark.']
(OUT/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))

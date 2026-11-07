"""Summarize one explicitly named BASELINE150 log; never certify visible/audible success."""
import argparse,json,re
from pathlib import Path
from compare_audio_slots_logs import rows

def analyze(path):
    header=(Path(__file__).resolve().parents[1]/'src/Chapter7Preload.h').read_text()
    expected=set(map(int,re.findall(r'(\d+)u',re.search(r'kChapter7AllPositiveIds\[150\] = \{(.*?)\};',header,re.S).group(1))))
    assert len(expected)==150
    records=rows(Path(path))
    starts=[r for r in records if r['type']=='DSFV29s_BASELINE_START']
    status=[r for r in records if r['type']=='DSFV29s_BASELINE_STATUS']
    phases=[r for r in records if r['type']=='DSFV29s_BASELINE_PHASE']
    rosters=[r for r in records if r['type']=='DSFV29s_BASELINE_ROSTER']
    model_rows=[r for r in records if r['type']=='DSFV29s_BASELINE_MODEL']
    def peak(key):return max((int(r.get(key,0)) for r in status),default=0)
    summaries=[]
    for event in phases:
        if event.get('kind')!='accepted':continue
        seq=event['seq'];samples=[r for r in phases if r.get('seq')==seq]
        roster=next((r for r in rosters if r.get('seq')==seq and r.get('kind')=='effective'),None)
        ids=[int(v) for v in roster.get('ids','').split(',') if v] if roster else []
        models={int(r['model']):r for r in model_rows if r.get('seq')==seq}
        ready=[r for r in samples if int(r.get('checkpoints',0)) and int(r.get('ready',0))==int(event['count'])]
        metric_rows=[r for r in records if r['type']=='DSFV29s_BASELINE_METRIC' and r.get('seq')==seq]
        metric_peaks={}
        for r in metric_rows:metric_peaks[r['name']]=max(metric_peaks.get(r['name'],0),int(r['peakSampled']))
        summaries.append(dict(seq=int(seq),caller=event['caller'],count=int(event['count']),ids=ids,
            exact_roster_recorded=len(ids)==int(event['count']) and len(set(ids))==len(ids),
            matches_vanilla150=len(ids)==150 and set(ids)==expected,
            all_ready_in_recorded_sweep=bool(ready),first_all_ready_tick=min((int(r['allReadyTick']) for r in ready),default=None),
            last_ready_count=int(samples[-1].get('ready',0)),last_checkpoint_age_ms=int(samples[-1].get('checkpointAgeMs',0)),
            models_seen_in_original_body_call=sorted(i for i,r in models.items() if int(r.get('bodyCallsOriginal',0))),
            models_seen_as_resolved_body=sorted(i for i,r in models.items() if int(r.get('bodyCallsResolved',0))),
            models_with_observed_fallback=sorted(i for i,r in models.items() if int(r.get('bodyFallbackCalls',0))),
            models_without_ready_observation=[i for i in ids if not int(models.get(i,{}).get('firstReady',0))],
            audio_setup_fail_observations={str(i):int(r.get('setupFailObserved',0)) for i,r in models.items() if int(r.get('setupFailObserved',0))},
            metric_peaks=metric_peaks))
    checks=dict(startup=len(starts)==1 and starts[0].get('status')=='1',has_status=bool(status),
        no_event_drops=peak('eventDrops')==0,no_memory_snapshot_failures=peak('memorySnapshotFailures')==0,
        accepted_rosters_complete=bool(summaries) and all(s['exact_roster_recorded'] for s in summaries))
    notify_start=[r for r in records if r['type']=='DSFV29s_NOTIFY150_START']
    notify_status=[r for r in records if r['type']=='DSFV29s_NOTIFY150_STATUS']
    notify_copies=[r for r in records if r['type']=='DSFV29s_NOTIFY150_COPY']
    notify_fatal=[r for r in records if r['type']=='DSFV29s_NOTIFY150_FATAL']
    if notify_start or notify_status or notify_copies or notify_fatal:
        checks['notification_startup']=len(notify_start)==1 and all(notify_start[0].get(k)==v for k,v in
            {'status':'1','patches':'5','nativeCapacity':'50','capacity':'150','frameBytes':'608'}.items())
        checks['notification_no_fatal_or_invalid']=not notify_fatal and all(r.get('status')=='1' and r.get('invalid')=='0' for r in notify_status)
        checks['notification_copy_bounds']=all(0<=int(r['count'])<=150 for r in notify_copies)
    parked_start=[r for r in records if r['type']=='DSFV29s_PARKED150_START']
    parked_status=[r for r in records if r['type']=='DSFV29s_PARKED150_STATUS']
    parked_builds=[r for r in records if r['type']=='DSFV29s_PARKED150_BUILD']
    parked_fatals=[r for r in records if r['type']=='DSFV29s_PARKED150_FATAL']
    parsed_builds=[]
    for r in parked_builds:
        ids=[int(v) for v in r.get('ids','').split(',') if v];n=int(r.get('count',-1))
        parsed_builds.append(dict(record=r,ids=ids,valid=0<=n<=150 and len(ids)==n and len(set(ids))==n,
            matches_vanilla150=n==150 and len(ids)==150 and set(ids)==expected))
    if parked_start or parked_status or parked_builds or parked_fatals:
        checks['parked_startup']=len(parked_start)==1 and all(parked_start[0].get(k)==v for k,v in
            {'status':'1','hooks':'2','nativeCapacity':'50','capacity':'150','categories':'25','perCategory':'150',
             'frameBytes':'15732','parkedPopulationChanged':'0','refreshTimingChanged':'0','weightsChanged':'0'}.items())
        checks['parked_status_present']=bool(parked_status)
        checks['parked_no_fatal_or_invalid']=not parked_fatals and all(r.get('status')=='1' and r.get('invalid')=='0' and r.get('guardBad')=='0' for r in parked_status)
        checks['parked_capacity_bounds']=all(0<=int(r.get(k,0))<=150 for r in parked_status for k in ('peakCandidates','peakBucket','peakUniqueSelections'))
        checks['parked_build_rosters_complete']=all(b['valid'] for b in parsed_builds)
    replay_start=[r for r in records if r['type']=='DSFV29s_REPLAY150_START']
    replay_status=[r for r in records if r['type']=='DSFV29s_REPLAY150_STATUS']
    replay_events=[r for r in records if r['type']=='DSFV29s_REPLAY150_EVENT']
    replay_fatals=[r for r in records if r['type']=='DSFV29s_REPLAY150_FATAL']
    canonical_hash=2166136261
    for model in sorted(expected):canonical_hash=((canonical_hash^model)*16777619)&0xffffffff
    def canonical_snapshot(r):
        return int(r.get('count',0))==150 and int(r.get('rosterHash','0'),16)==canonical_hash and r.get('id50')=='181' and r.get('has181')=='1'
    if replay_start or replay_status or replay_events or replay_fatals:
        checks['replay_startup']=len(replay_start)==1 and all(replay_start[0].get(k)==v for k,v in
            {'status':'1','hooks':'7','trafficCapacity':'150','retainedCapacity':'256','payloadBytes':'2660',
             'nativeViewBytes':'612','nativeObjectBytes':'624','nativeRecordFormat':'1'}.items())
        checks['replay_status_present']=bool(replay_status)
        checks['replay_no_fatal_or_invalid']=not replay_fatals and all(r.get('status')=='1' and r.get('invalid')=='0' for r in replay_status)
        checks['replay_capacity_bounds']=all(0<=int(r.get('count',-1))<=150 and 0<=int(r.get('retained',-1))<=256 and
            (int(r.get('count',0))<=50 or 0<int(r.get('id50',0))<=65535) for r in replay_events) and all(
            0<=int(r.get('peakTraffic',0))<=150 and 0<=int(r.get('peakRetained',0))<=256 for r in replay_status)
    return dict(log=str(path),checks=checks,telemetry_checks_pass=all(checks.values()),phases=summaries,
        lock_skips=peak('lockSkips'),obsolete_checks=peak('obsoleteChecks'),unmapped_body_calls=peak('bodyUnmapped'),
        rejected_requests=[r for r in phases if r.get('kind') not in ('accepted','sample','superseded')],
        notification=dict(start=notify_start,final=notify_status[-1] if notify_status else None,copies=notify_copies,fatals=notify_fatal,
            observed_150_sidecar_copy=any(r.get('storage')=='sidecar' and r.get('count')=='150' for r in notify_copies)),
        parked=dict(start=parked_start,final=parked_status[-1] if parked_status else None,builds=parsed_builds,fatals=parked_fatals,
            observed_150_candidate_build=any(b['valid'] and b['matches_vanilla150'] for b in parsed_builds),
            observed_extended_picker_use=any(int(r.get('extraSelections',0))>0 for r in parked_status),
            visible_fallback_resolved=False),
        replay=dict(start=replay_start,final=replay_status[-1] if replay_status else None,events=replay_events,fatals=replay_fatals,
            observed_canonical150_save=any(r.get('kind')=='save' and canonical_snapshot(r) for r in replay_events),
            observed_canonical150_restore=any(r.get('kind')=='restore' and canonical_snapshot(r) for r in replay_events),
            loading_samples=[r for r in replay_status if int(r.get('loading',0))],
            softlock_resolved=False),
        has_150_ready_sweep=any(s['matches_vanilla150'] and s['exact_roster_recorded'] and s['all_ready_in_recorded_sweep'] for s in summaries),
        baseline_certified=False,
        limitations=['Run analyze_callback_log.py separately for inherited queue/arena/audio checks.',
            'READY is a sequential native-predicate sweep, not an atomic worker-thread or drawable snapshot.',
            'Body-call coverage is not visible pixels. Missing coverage can mean an unexercised model.',
            'Audio counters are cumulative observations; compare audioAtStart for phase deltas.',
            'Trailer/special models do not necessarily have engine/horn audio.',
            'Lock skips can omit observations; memory/queue peaks are sampled. No final-exit snapshot.',
            'A 150-model preset still needs user-visible/audible results and lifecycle coverage.'])

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('log',type=Path);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();result=analyze(args.log);args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('phases','limitations','rejected_requests')},indent=2))
if __name__=='__main__':main()

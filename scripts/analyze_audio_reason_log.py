"""Analyze one explicitly supplied reason-probe log; no game/file discovery."""
import argparse
from collections import Counter,defaultdict
import json
from pathlib import Path
from analyze_audio_probe_log import analyze
from compare_audio_slots_logs import rows,slot_summary

def main():
    parser=argparse.ArgumentParser();parser.add_argument('log',type=Path);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    records=rows(args.log);baseline=analyze(args.log);slots=slot_summary(records)
    def of(kind):return [r for r in records if r['type']=='DSFV29s_AUDIO_REASON_'+kind]
    start,status,owner_rows,event_rows,details=of('START'),of('STATUS'),of('OWNER'),of('EVENT'),of('DETAIL')
    assert status,'No reason status coverage';last=status[-1]
    counters=tuple(k for k in last if k not in ('type','line','tick'))
    checks=dict(reason_start=len(start)==1 and all(start[0].get(k)==v for k,v in {
        'revision':'1','status':'1','hooks':'10','expectedHooks':'10','changedLimits':'0','objectEventLimit':'16','globalMax':'2048','defaultBytes':'67108864'}.items()),
        counters_monotonic=all(int(b[k])>=int(a[k]) for a,b in zip(status,status[1:]) for k in counters),
        no_invalid=all(r['invalid']=='0' for r in status),no_owner_overflow=all(r['ownerOverflow']=='0' for r in status),
        no_event_overflow=all(r['eventOverflow']=='0' for r in status))
    owners={};events={}
    for r in owner_rows:owners[r['owner']]=r
    for r in event_rows:events[r['event']]=r
    def integer_values(row,skip):return {k:int(v) for k,v in row.items() if k not in skip}
    owner_summary=[dict(owner=key,**integer_values(r,('type','line','owner'))) for key,r in owners.items()]
    event_summary=[dict(event=key,**integer_values(r,('type','line','event','firstOwner','lastOwner','firstCaller','lastCaller')),
        **{k:r[k] for k in ('firstOwner','lastOwner','firstCaller','lastCaller')}) for key,r in events.items()]
    detail_groups=defaultdict(list)
    for r in details:detail_groups[r['kind']].append(r)
    missing_groups=defaultdict(list)
    for r in details:
        if r['kind'] in ('lookupMissing','noneLookupMissing'):missing_groups[(r['a'],r['owner'])].append(r)
    missing_summary=[dict(event=e,object=o,retained=len(rs),first_tick=int(rs[0]['tick']),last_tick=int(rs[-1]['tick']),
        returns=sorted({r['c'] for r in rs})) for (e,o),rs in missing_groups.items()]
    names=[]
    for r in detail_groups['inputName']:
        names.append(dict(event=r['a'],owner=r['owner'],caller=r['c'],flags=int(r['flags']),tick=int(r['tick']),
            name=bytes.fromhex(r.get('nameHex','')).decode('ascii',errors='backslashreplace')))
    first_positive={}
    for r in event_rows:
        if int(r['lookupHit'])>0 and r['event'] not in first_positive:first_positive[r['event']]=r
    final={k:int(last[k]) for k in counters}
    totals={k:sum(r[k] for r in owner_summary) for k in ('requests','refused','accepted','zero','callbacks','ends','unmatched','removes','registrations','lookupHit','lookupMiss')}
    result=dict(log=str(args.log),variant='reason2048_default64m',baseline=baseline,slots=slots,reason_checks=checks,
        valid=baseline['valid_probe'] and slots['valid'] and all(checks.values()),last_reason_tick=int(last['tick']),reason_final=final,
        reason_status_rows=len(status),owner_rows=len(owner_rows),event_rows=len(event_rows),distinct_owner_addresses=len(owners),distinct_event_ids=len(events),
        last_aggregate_tick=max((int(r['tick']) for r in owner_rows+event_rows),default=None),owners=owner_summary,events=event_summary,
        owner_aggregate_totals=totals,retained_details={k:len(v) for k,v in detail_groups.items()},missing_groups=missing_summary,names=names,
        first_positive_event_snapshots={k:first_positive[k] for k in events if k in first_positive and int(events[k]['lookupMiss'])>0},
        retained_wrapper_zero_events=dict(Counter(r['a'] for r in detail_groups['wrapperPostZero'])),
        diagnostics=dict(lookup_balance=final['lookupCalls']-final['lookupHit']-final['lookupMiss'],
            wrapper_return_balance=final['wrapperReturns']-final['wrapperAccepted']-final['wrapperZero'],
            wrapper_request_balance=final['wrapperRequests']-final['wrapperReturns']-final['full16'],
            completion_minus_removal=final['endsHandled']-final['removes']),
        limitations=['Native counters are independent snapshots; small in-flight imbalances are possible.',
            'Owner/event aggregate rows are emitted on changes about every5s and may trail final status.',
            'Owner addresses can be reused; rows aggregate lifetimes and are not vehicle identities.',
            'Projected accepted peak is pre-insertion count+1; observed peak reads the native field.',
            'Admitted events/lookup hits do not establish downstream voice/media/mixer success.',
            'Input names cover only the two instrumented string wrappers, with bounded details.'])
    (args.output/'summary.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(dict(valid=result['valid'],seconds=result['last_reason_tick']/1000,checks=checks,
        final=final,diagnostics=result['diagnostics'],owner_addresses=len(owners),event_ids=len(events),owner_totals=totals,
        maximum_owner_observed=max((r['peakObserved'] for r in owner_summary),default=None),
        maximum_owner_projected=max((r['acceptedPeakProjected'] for r in owner_summary),default=None),
        owners_by_peak=sorted(owner_summary,key=lambda r:r['peakObserved'],reverse=True)[:6],
        failed_events=[r for r in event_summary if r['lookupMiss'] or r['createFailed'] or r['refused'] or r['zero']],
        names=names,retained_details=result['retained_details'],baseline_audio=baseline['audio_final'],
        baseline_pressure=baseline.get('default_pressure'),visual=baseline['visual_baseline'],
        slot_peaks={k:v['peak_observed_count'] for k,v in slots['lists'].items()},users=slots['users_final']),indent=2))

if __name__=='__main__':main()

"""Compare two supplied vehicle runs with approximate load/end-relative windows."""
import argparse
from collections import Counter
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import sys
import analyze_audio_reason_log
from compare_audio_slots_logs import rows

EVENT_COUNTERS=('lookupHit','lookupMiss','createFailed','requests','refused','accepted','zero')
OWNER_COUNTERS=('registrations','requests','refused','accepted','zero','noneRequests','callbacks','ends','unmatched','removes','lookupHit','lookupMiss')

def last_by(records,key,end):
    selected={}
    for r in records:
        if int(r['tick'])<=end:selected[r[key]]=r
    return selected

def window(records,start,end,kind,key,counters):
    selected=[r for r in records if r['type']==kind]
    before=last_by(selected,key,start);after=last_by(selected,key,end);result=[]
    for name,r in after.items():
        old=before.get(name,{})
        delta={k:int(r[k])-int(old.get(k,0)) for k in counters}
        if not any(delta.values()):continue
        item=dict(identity=name,**delta,last_observed_tick=int(r['tick']))
        for k in ('firstOwner','lastOwner','firstCaller','lastCaller','peakObserved','acceptedPeakProjected','lastCount'):
            if k in r:item[k]=r[k]
        result.append(item)
    return result

def main():
    parser=argparse.ArgumentParser();parser.add_argument('working',type=Path);parser.add_argument('silent',type=Path)
    parser.add_argument('--output',type=Path,required=True);parser.add_argument('--load-seconds',type=int,default=40)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    runs={}
    for label,path in (('crown_victoria',args.working),('cadillac_escalade',args.silent)):
        argv=sys.argv
        try:
            sys.argv=['analyze_audio_reason_log.py',str(path),'--output',str(args.output/label)]
            console=io.StringIO()
            with redirect_stdout(console):analyze_audio_reason_log.main()
        finally:sys.argv=argv
        summary=json.loads((args.output/label/'summary.json').read_text());records=rows(path)
        (args.output/label/'analyzer_output.json').write_text(console.getvalue())
        end=summary['last_reason_tick'];start=args.load_seconds*1000
        profiles={}
        for title,lo in (('after_approx_load',start),('last30s',max(start,end-30000)),('last15s',max(start,end-15000))):
            profiles[title]=dict(start_tick=lo,end_tick=end,
                events=window(records,lo,end,'DSFV29s_AUDIO_REASON_EVENT','event',EVENT_COUNTERS),
                owners=window(records,lo,end,'DSFV29s_AUDIO_REASON_OWNER','owner',OWNER_COUNTERS))
        banks=[r for r in records if r['type']=='DSFV29s_AUDIO_DETAIL' and r.get('kind')=='bank']
        runs[label]=dict(valid=summary['valid'],seconds=end/1000,reason=summary['reason_final'],diagnostics=summary['diagnostics'],
            bank_histogram=dict(Counter(r['bank'] for r in banks)),baseline_audio=summary['baseline']['audio_final'],
            pool_thresholds=summary['baseline']['pool_thresholds'],visual=summary['baseline']['visual_baseline'],
            slot_peaks={k:v['peak_observed_count'] for k,v in summary['slots']['lists'].items()},profiles=profiles)
    a,b=runs['crown_victoria'],runs['cadillac_escalade']
    comparisons={}
    for title in a['profiles']:
        ar={r['identity']:r for r in a['profiles'][title]['events']};br={r['identity']:r for r in b['profiles'][title]['events']}
        accepted_a={k for k,r in ar.items() if r['accepted']>0};accepted_b={k for k,r in br.items() if r['accepted']>0}
        comparisons[title]=dict(accepted_events_only_crown=sorted(accepted_a-accepted_b),
            accepted_events_only_escalade=sorted(accepted_b-accepted_a),accepted_events_shared=sorted(accepted_a&accepted_b),
            events=[dict(event=k,crown=ar.get(k),escalade=br.get(k)) for k in sorted(set(ar)|set(br))])
    result=dict(runs=runs,comparison=comparisons,
        bank_id_sets_equal=set(a['bank_histogram'])==set(b['bank_histogram']),bank_callback_histograms_equal=a['bank_histogram']==b['bank_histogram'],
        bank_ids_only_crown=sorted(set(a['bank_histogram'])-set(b['bank_histogram'])),bank_ids_only_escalade=sorted(set(b['bank_histogram'])-set(a['bank_histogram'])),
        limitations=['40s in-game start is user estimate, not a logged vehicle-entry marker.',
            'Event/owner aggregates are change snapshots emitted about every5s; window deltas can include activity just before the chosen boundary or omit the last few seconds.',
            'Alt+F4 means no intentional final menu sequence, but background traffic/ambient activity remains and exit has no guaranteed final flush.',
            'Owner addresses are per-process and reused; never match a numerical address to a car across runs.',
            'Candidate event differences are not proof of the driven vehicle; no full event-object-time matrix or car model tag is logged.'])
    (args.output/'comparison.json').write_text(json.dumps(result,indent=2))
    for label,run in runs.items():
        print(json.dumps(dict(run=label,valid=run['valid'],seconds=run['seconds'],reason=run['reason'],
            bank_ids=len(run['bank_histogram']),baseline_audio=run['baseline_audio'],pools=run['pool_thresholds'],
            slots=run['slot_peaks'],tail30_accepted=sum(e['accepted'] for e in run['profiles']['last30s']['events']),
            tail30_lookup_misses=sum(e['lookupMiss'] for e in run['profiles']['last30s']['events']),
            tail30_event_ids=len(run['profiles']['last30s']['events']),
            after40_failed=[r for r in run['profiles']['after_approx_load']['events'] if r['lookupMiss'] or r['zero'] or r['refused']]),indent=2))
    print(json.dumps(dict(bank_id_sets_equal=result['bank_id_sets_equal'],bank_callback_histograms_equal=result['bank_callback_histograms_equal'],
        tail30_only_crown=comparisons['last30s']['accepted_events_only_crown'],tail30_only_escalade=comparisons['last30s']['accepted_events_only_escalade']),indent=2))

if __name__=='__main__':main()

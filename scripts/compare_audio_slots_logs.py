"""Compare two explicitly supplied logs; no game execution or file discovery."""
import argparse
from collections import Counter
import json
from pathlib import Path
from analyze_audio_probe_log import analyze

def rows(path):
    result=[]
    for number,line in enumerate(path.read_text(encoding='utf-8-sig').splitlines(),1):
        parts=line.split()
        if parts:
            fields=dict(p.split('=',1) for p in parts[1:] if '=' in p)
            # Payload fields must not collide with the parser's record metadata.
            for reserved in ('type','line'):
                if reserved in fields:fields['field_'+reserved]=fields.pop(reserved)
            result.append(dict(type=parts[0],line=number,**fields))
    return result

def slot_summary(records):
    start=[r for r in records if r['type']=='DSFV29s_AUDIO_SLOTS_START']
    counts=[r for r in records if r['type']=='DSFV29s_AUDIO_SLOTS']
    users=[r for r in records if r['type']=='DSFV29s_AUDIO_SLOT_USERS']
    checks=dict(startup=len(start)==1 and all(start[0].get(k)==v for k,v in {
        'revision':'1','status':'1','patches':'1','hooks':'9','expectedHooks':'9',
        'stockMax':'255','testMax':'2048','managers':'2','globalLists':'3','perRecordLimits':'8,255'}.items()),
        user_coverage=bool(users))
    lists={}
    monotonic=('updatePeak','updateSamples','attempts','peakBefore','inserts','peakInserted','ceilingHits','selectedReuses','invalid')
    for name in ('A0','A1','B'):
        selected=[r for r in counts if r['list']==name]
        observed=[r for r in selected if int(r['owner'],16)!=0]
        checks[name+'_live_capacity']=bool(observed) and all(r['configuredMax']==r['allocatedSlots']=='2048' for r in observed)
        checks[name+'_counters_monotonic']=all(int(b[k])>=int(a[k]) for a,b in zip(selected,selected[1:]) for k in monotonic)
        checks[name+'_headers_valid']=bool(selected) and all(r['invalid']=='0' for r in selected)
        if not selected:continue
        final=selected[-1]
        peak=max(int(r[k]) for r in selected for k in ('updatePeak','peakBefore','peakInserted'))
        lists[name]=dict(rows=len(selected),observed_rows=len(observed),first_observed_tick=int(observed[0]['tick']) if observed else None,
            final={k:int(final[k]) for k in ('configuredMax','allocatedSlots','nativeUpdateCount')+monotonic},
            peak_observed_count=peak,exceeded_stock255=peak>255,
            owner_variants=sorted({r['owner'] for r in observed}),last_tick=int(final['tick']))
    keys=('eightAttempts','eightPeakBefore','eightFull','byteAttempts','bytePeakBefore','byteFull','invalid')
    checks['user_counters_monotonic']=all(int(b[k])>=int(a[k]) for a,b in zip(users,users[1:]) for k in keys)
    checks['user_headers_valid']=bool(users) and all(r['invalid']=='0' for r in users)
    return dict(start=start,checks=checks,valid=all(checks.values()),lists=lists,
        users_final={k:int(users[-1][k]) for k in keys} if users else None,
        users_last_tick=int(users[-1]['tick']) if users else None,
        limitations=['Counts and peaks are observed at the installed native paths; gauge fields are last-update snapshots.',
            'An unexercised attachment path has no measured demand; zero attempts is not a saturated list.',
            'Selected reuse does not prove successful reinsertion or audible playback.'])

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('default64',type=Path);parser.add_argument('slots2048',type=Path)
    parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    summaries={}
    for name,path in (('default64',args.default64),('slots2048',args.slots2048)):
        result=analyze(path);records=rows(path)
        if name=='slots2048':
            result['slots2048']=slot_summary(records)
            result['variant']='slots2048_default64m'
            result['valid_probe']=result['valid_probe'] and result['slots2048']['valid']
        banks=[r for r in records if r['type']=='DSFV29s_AUDIO_DETAIL' and r.get('kind')=='bank']
        result['retained_bank_ids']=sorted({r['bank'] for r in banks})
        result['retained_event_zero_counts']=dict(Counter(r['event'] for r in records if r['type']=='DSFV29s_AUDIO_DETAIL' and r.get('kind')=='postZero'))
        (args.output/(name+'_summary.json')).write_text(json.dumps(result,indent=2),encoding='utf-8')
        summaries[name]=result
    left,right=summaries['default64'],summaries['slots2048']
    comparison=dict(default64_valid=left['valid_probe'],slots2048_valid=right['valid_probe'],
        bank_ids_only_default64=sorted(set(left['retained_bank_ids'])-set(right['retained_bank_ids'])),
        bank_ids_only_slots2048=sorted(set(right['retained_bank_ids'])-set(left['retained_bank_ids'])),
        limitations='Different run durations/routes; event totals cannot establish an improvement or regression. Bank callback success and admitted event posts do not establish audible output.')
    (args.output/'comparison.json').write_text(json.dumps(comparison,indent=2),encoding='utf-8')
    for name,result in summaries.items():
        print(json.dumps(dict(run=name,valid=result['valid_probe'],seconds=result['last_tick']/1000,
            startup=result['startup_checks'],audio=result['audio_final'],default_pressure=result.get('default_pressure'),
            pools=[{k:p[k] for k in ('id','usable_bytes','sampled_peak_malloc_bytes','sampled_peak_percent','sampled_min_free_blocks','blocks','block_bytes')} for p in result['pools']],
            bank_results=result['bank_result_histogram'],retained_details=result['retained_details'],
            event_zeros=result['retained_event_zero_counts'],visual=result['visual_baseline'],
            slots=result.get('slots2048')),indent=2))
    print(json.dumps(comparison,indent=2))

if __name__=='__main__':main()

"""Analyze one explicitly supplied pending512 run; no discovery or target execution."""
import argparse
import json
from pathlib import Path
from collections import defaultdict
from compare_audio_slots_logs import rows
from compare_cleanup_logs import analyze as analyze_cleanup


def main():
    p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    records=rows(a.log);inherited=analyze_cleanup(a.log)
    def of(kind):return [r for r in records if r['type']=='DSFV29s_PENDING_'+kind]
    start,status,events=map(of,('START','STATUS','EVENT'));assert len(start)==1 and status
    final=status[-1];fatals=[r for r in records if r['type'].endswith('_FATAL')]
    cumulative=('generations','resets','attempts','stores','redirects','recycledInserts','activePeak','issuedPeak',
        'walkStarts','walkEnds','emptyChecks','invalid','full','allocationFailed','guardBad')
    checks=dict(startup=all(start[0].get(k)==v for k,v in {
        'revision':'1','status':'1','stockCapacity':'64','testCapacity':'512','inlineNodes':'64','extraNodes':'448',
        'nodeBytes':'16','generationBytes':'7232','traversalHooks':'2','sharedInsertHook':'007D7270',
        'nativeSentinelPreserved':'1','activeMigration':'0'}.items()),
        no_fatal=not fatals,
        no_invalid=all(r['invalid']=='0' for r in status),no_capacity_refusal=all(r['full']=='0' for r in status),
        no_allocation_failure=all(r['allocationFailed']=='0' for r in status),
        guards_healthy=all(r['guardBad']=='0' for r in status),
        counters_monotonic=all(int(b[k])>=int(x[k]) for x,b in zip(status,status[1:]) for k in cumulative),
        final_insert_balance=final['attempts']==final['stores'],
        final_walk_balance=final['walkStarts']==final['walkEnds'],
        final_empty=final['current']=='0' and final['depth']=='0')
    grouped=defaultdict(list)
    for r in events:grouped[int(r['generation'])].append(r)
    generations=[]
    for generation,rs in sorted(grouped.items()):
        beginnings=[r for r in rs if r['kind']=='generation'];redirects=[r for r in rs if r['kind']=='redirect']
        completed=[r for r in rs if r['kind']=='walkEnd' and r['active']=='0']
        generations.append(dict(generation=generation,maps=sorted({r['map'] for r in rs}),
            generation_events=len(beginnings),redirects=len(redirects),
            creation_tick=int(beginnings[0]['tick']) if beginnings else None,
            max_retained_issued=max(int(r['issued']) for r in rs),
            max_retained_active=max(int(r['active']) for r in rs),
            retained_empty_completion_ticks=[int(r['tick']) for r in completed]))
    checks['generation_census']=len(generations)==int(final['generations']) and all(g['generation_events']==1 for g in generations)
    checks['expanded_path_exercised']=int(final['issuedPeak'])>64 and int(final['redirects'])>0
    model=inherited['model'];baseline=model['baseline']
    result=dict(log=str(a.log),bytes=inherited['bytes'],sha256=inherited['sha256'],
        valid=all(checks.values()) and inherited['valid'],checks=checks,
        duration_seconds=max(int(r['tick']) for r in records if r.get('tick','').isdigit())/1000,
        final_pending=final,generations=generations,retained_pending_events=len(events),
        final_cleanup=inherited['final'],cleanup_checks=inherited['checks'],
        cleanup_attempts_minus_drained=inherited['attempts_minus_drained'],
        cleanup_final_drain_balance=inherited['final_drain_balance'],
        cleanup_large_drains=[r for r in inherited['retained_drains'] if r['begin_count']>=32768],
        final_model=model['final_status'],model_checks=model['checks'],model_totals=model['model_totals'],
        baseline=baseline,slots=model['slots'],reason_final=model['reason_last'],
        last_capacity=next(r for r in reversed(records) if r['type']=='DSFV29s_CAP_STATUS'),
        limitations=['User reports successful menu/reload/exit; telemetry does not record the final OS exit status.',
            'Three different owners do not exercise constructor address reuse in this live run; that remains fixture-tested.',
            'Fields are independent snapshots and low-active walk event logging is bounded.',
            'Audio setup success is not a per-category audibility measurement; request/slot counts do not prove simultaneous drawable vehicles.'])
    (a.output/'summary.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps({k:result[k] for k in ('valid','checks','duration_seconds','final_pending','generations',
        'final_cleanup','cleanup_attempts_minus_drained','cleanup_final_drain_balance','final_model','last_capacity')},indent=2))


if __name__=='__main__':main()

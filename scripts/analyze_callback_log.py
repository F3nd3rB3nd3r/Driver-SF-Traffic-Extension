"""Analyze one explicitly supplied callback512 log without discovering other files."""
import argparse,json
from pathlib import Path
from compare_audio_slots_logs import rows
from compare_chapter_logs import summarize

def early_startup(records,path):
    fatal=[r for r in records if 'FATAL' in r['type']]
    if any(r['type']=='DSFV29s_CALLBACK_STATUS' for r in records) or not fatal:return None
    starts=[r for r in records if r['type'].endswith('_START')]
    return dict(log=str(path),checks=dict(startup=False,no_fatal=False,has_status=False),recorded_integrity_pass=False,
        startup_only=True,startup_markers=starts,fatals=fatal,final_status=None,groups={},extension_exercised=False,
        baseline_status=next((int(r['baselineStatus']) for r in records if 'baselineStatus' in r),None),
        inherited=None,audio_setup_failures=None,chapter_events=sum(r['type']=='DSFV29s_CHAPTER_EVENT' for r in records),
        limitations=['Fatal startup log has no periodic samples; runtime inherited analysis is unavailable.',
            'A downstream baselineStartupRefused fatal does not itself identify the original preflight predicate.'])

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('log',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    records=rows(a.log);early=early_startup(records,a.log)
    if early is not None:
        a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(early,indent=2)+'\n')
        print(json.dumps(early,indent=2));return
    base=summarize(a.log)
    start=[r for r in records if r['type']=='DSFV29s_CALLBACK_START']
    status=[r for r in records if r['type']=='DSFV29s_CALLBACK_STATUS']
    groups=[r for r in records if r['type']=='DSFV29s_CALLBACK_GROUP']
    fatal=[r for r in records if r['type']=='DSFV29s_CALLBACK_FATAL']
    legacy=[r for r in records if r['type']=='DSFV29s_LEGACY_FATAL']
    arena_start=[r for r in records if r['type']=='DSFV29s_ARENA_START']
    arena_status=[r for r in records if r['type']=='DSFV29s_ARENA_STATUS']
    checks=dict(startup=len(start)==1 and all(start[0].get(k)==v for k,v in
        {'revision':'1','status':'1','groups':'21','stockCapacity':'64','testCapacity':'512','hooks':'3','sentinelPreserved':'1','processLifetime':'1'}.items()),
        no_fatal=not fatal and not legacy,has_status=bool(status),no_reported_errors=all(r[k]=='0' for r in status for k in ('invalid','full','guardBad','concurrent')),
        valid_group_indices=all(0<=int(r['group'])<21 for r in groups),
        capacity_respected=all(int(r[k])<=512 for r in groups for k in ('peak','issuedPeak','current','recycled')))
    if arena_start:
        checks['arena_startup']=len(arena_start)==1 and all(arena_start[0].get(k)==v for k,v in
            {'status':'1','spoolPolicy':'ownerReuse','fixedOwnerLimit':'0','scratchPolicy':'expandedFrame150','scratchExternal':'0'}.items())
        checks['arena_status_present']=bool(arena_status)
        checks['arena_integrity']=all(r[k]=='0' for r in arena_status for k in ('invalid','guardBad','allocationFailed','scratchArenas'))
    summaries={}
    counters=('attempts','stores','redirects','recycledInserts','peak','issuedPeak','polls','removed')
    for i in sorted({int(r['group']) for r in groups}):
        selected=[r for r in groups if int(r['group'])==i]
        monotonic=all(int(y[k])>=int(x[k]) for x,y in zip(selected,selected[1:]) for k in counters)
        checks[f'group{i}_monotonic']=monotonic
        final=selected[-1]
        summaries[str(i)]=dict(samples=len(selected),final=final,observed_beyond64=any(int(r['peak'])>64 or int(r['redirects']) for r in selected),
            sampled_stores_minus_removed_minus_current=int(final['stores'])-int(final['removed'])-int(final['current']))
    result=dict(log=str(a.log),checks=checks,recorded_integrity_pass=all(checks.values()),start=start,fatals=fatal,
        final_status=status[-1] if status else None,groups=summaries,legacy_fatals=legacy,
        arena_start=arena_start,arena_final=arena_status[-1] if arena_status else None,
        extension_exercised=any(r['observed_beyond64'] for r in summaries.values()),
        audio_setup_failures=base['audio']['totals']['setupFail'],chapter_events=base['event_count'],inherited=base,
        limitations=['Gauges are independent in-flight snapshots; balance/depth differences are not alone corruption.',
            'No callback expansion observed means this workload did not validate the extension.',
            'No fatal record is not proof of normal OS exit or successful challenge completion.',
            'Audio native setup return failures need separate interpretation; structural pass is not audible-playback proof.'])
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('inherited','limitations')},indent=2))
if __name__=='__main__':main()

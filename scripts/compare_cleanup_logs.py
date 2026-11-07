"""Compare only explicitly supplied cleanup262144 logs. No target execution."""
import argparse
import hashlib
import json
from pathlib import Path

from analyze_audio_model192_exit_dump import analyze_log
from compare_audio_slots_logs import rows


def analyze(path):
    records=rows(path)
    def of(kind):return [r for r in records if r['type']=='DSFV29s_'+kind]
    model=analyze_log(path)
    starts,status,events,fatals=map(of,('CLEANUP_START','CLEANUP_STATUS','CLEANUP_EVENT','CLEANUP_FATAL'))
    assert len(starts)==1 and status
    checks=dict(startup=all(starts[0].get(k)==v for k,v in {
        'revision':'1','status':'1','hooks':'6','expectedHooks':'6','stockCapacity':'32768',
        'testCapacity':'262144','nativeCountPreserved':'1','nativeOrderPreserved':'1','activeMigration':'0'}.items()),
        no_fatal=not fatals,no_invalid=all(r['invalid']=='0' for r in status),
        no_full=all(r['full']=='0' for r in status),guards_healthy=all(r['guardBad']=='0' for r in status),
        no_append_during_drain=all(r['duringDrain']=='0' for r in status),
        no_drain_mismatch=all(r['drainMismatch']=='0' for r in status))
    counters=('direct','replacement','release','peak','beyondStock','oldCrossings','full','invalid',
              'drainStarts','drainEnds','nonempty','drained','duringDrain','drainMismatch')
    checks['counters_monotonic']=all(int(b[k])>=int(a[k]) for a,b in zip(status,status[1:]) for k in counters)
    final=status[-1]
    def stable_suffix(records,keys):
        if not records:return None
        end=records[-1];i=len(records)-1
        while i>0 and all(records[i-1].get(k)==end.get(k) for k in keys):i-=1
        return dict(first_tick=int(records[i]['tick']),last_tick=int(end['tick']),
            observed_seconds=(int(end['tick'])-int(records[i]['tick']))/1000,
            samples=len(records)-i,final={k:end.get(k) for k in keys})
    progress=('direct','replacement','release','drainStarts','drainEnds','drained','current','depth')
    plateau=stable_suffix(status,progress)
    big=[r for r in events if int(r['count'])>=32768]
    pending=None;drains=[]
    for r in events:
        if r['kind']=='drainBegin':pending=r
        elif r['kind']=='drainEnd' and pending:
            drains.append(dict(begin_tick=int(pending['tick']),end_tick=int(r['tick']),
                begin_count=int(pending['count']),end_count=int(r['count']),
                processed=int(r['value'],16),milliseconds=int(r['tick'])-int(pending['tick']),mismatch=int(r['mismatch'])))
            pending=None
    groups={kind:of(kind) for kind in ('CAP_STATUS','AUDIO_STATUS','AUDIO_REASON_STATUS','AUDIO_MODEL_STATUS','AUDIO_SLOT_USERS')}
    series_keys={
        'CAP_STATUS':('pending','indexedFreeHead'),
        'AUDIO_STATUS':('bankCallbacks','emitterAttempts','postNormalReturns','postZero'),
        'AUDIO_REASON_STATUS':('lookupCalls','lookupHit','lookupMiss','noneMissing','otherMissing','removes','registrations'),
        'AUDIO_MODEL_STATUS':('attempts','stores','removes','lookups','setupOk','setupFail')}
    suffixes={k:stable_suffix(groups[k],v) for k,v in series_keys.items()}
    slot_updates={}
    for name in ('A0','A1','B'):
        rs=[r for r in of('AUDIO_SLOTS') if r['list']==name]
        slot_updates[name]=stable_suffix(rs,('updateSamples','attempts','inserts','nativeUpdateCount'))
    plateau_activity={}
    for kind,keys in series_keys.items():
        rs=[r for r in groups[kind] if int(r['tick'])>=plateau['first_tick']]
        if not rs:continue
        plateau_activity[kind]=dict(first_tick=int(rs[0]['tick']),last_tick=int(rs[-1]['tick']),
            delta={k:int(rs[-1][k])-int(rs[0][k]) for k in keys if rs[-1].get(k,'').isdigit() and rs[0].get(k,'').isdigit()})
    native_audio_activity={}
    for name in ('A0','A1','B'):
        rs=[r for r in of('AUDIO_SLOTS') if r['list']==name and int(r['tick'])>=plateau['first_tick']]
        if rs:
            elapsed=(int(rs[-1]['tick'])-int(rs[0]['tick']))/1000
            delta=int(rs[-1]['updateSamples'])-int(rs[0]['updateSamples'])
            native_audio_activity[name]=dict(first_tick=int(rs[0]['tick']),last_tick=int(rs[-1]['tick']),
                native_update_calls=delta,observed_seconds=elapsed,calls_per_second=delta/elapsed if elapsed else None,
                insert_delta=int(rs[-1]['inserts'])-int(rs[0]['inserts']))
    return dict(log=str(path),bytes=path.stat().st_size,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        checks=checks,valid=all(checks.values()) and model['valid'] and model['baseline']['valid_probe'] and model['slots']['valid'],
        final=final,attempts_minus_drained=sum(int(final[k]) for k in ('direct','replacement','release'))-int(final['drained']),
        final_drain_balance=int(final['drainStarts'])-int(final['drainEnds']),cleanup_progress_plateau=plateau,
        big_events=big,retained_drains=drains,unfinished_retained_begin=pending,
        status_timeline=[r for i,r in enumerate(status) if i==0 or any(r[k]!=status[i-1][k] for k in progress)][-12:],
        other_progress_suffixes=suffixes,slot_updates_suffixes=slot_updates,activity_during_cleanup_plateau=plateau_activity,
        native_audio_activity_during_cleanup_plateau=native_audio_activity,
        model=model,limitations=['Status fields are independent in-flight samples.',
            'A flat counter does not identify the blocked instruction or owning thread.',
            'Only bounded nonempty-drain event lines are retained; counters cover later drains.',
            'Continuing monitor output does not establish main-thread or menu progress.'])


def main():
    p=argparse.ArgumentParser();p.add_argument('logs',nargs=2,type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);summary=[]
    for path in a.logs:
        result=analyze(path);(a.output/(path.stem+'.json')).write_text(json.dumps(result,indent=2),encoding='utf-8')
        summary.append({k:result[k] for k in ('log','valid','checks','final','attempts_minus_drained','final_drain_balance',
            'cleanup_progress_plateau','big_events','slot_updates_suffixes','activity_during_cleanup_plateau',
            'native_audio_activity_during_cleanup_plateau')})
    (a.output/'comparison.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
    for r in summary:
        print(json.dumps({k:r[k] for k in ('log','valid','final','attempts_minus_drained','final_drain_balance',
            'cleanup_progress_plateau','big_events','native_audio_activity_during_cleanup_plateau')},indent=2))


if __name__=='__main__':main()

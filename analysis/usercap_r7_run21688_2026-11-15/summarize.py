"""Analyze only the user-supplied R7 run; no game/source changes."""
import hashlib,json,sys
from collections import Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'scripts'))
from compare_audio_slots_logs import rows
path=ROOT/'dist/USERCAP_R7/DSFV29s_usercap_r7_21688.log'
records=rows(path);of=lambda name:[r for r in records if r['type']=='DSFV29s_'+name]
def number(r,k):
    try:return int(r.get(k,0))
    except ValueError:return 0
def peak(name,keys):return {k:max((number(r,k) for r in of(name)),default=0) for k in keys}
phases=[]
for start in of('BASELINE_PHASE'):
    if start.get('kind')!='accepted':continue
    sample=[r for r in of('BASELINE_PHASE') if r.get('seq')==start['seq']]
    ready=[r for r in sample if number(r,'ready')==number(start,'count') and number(r,'checkpoints')>0]
    phases.append(dict(seq=start['seq'],count=start['count'],started=start['started'],ready=bool(ready),first_ready=ready[0] if ready else None,last=sample[-1]))
keys={r['model']:r for r in of('AUDIO_MODEL_KEY')}
failures=[r for r in keys.values() if number(r,'setupFail')]
increments=[];last=0
for r in of('AUDIO_MODEL_STATUS'):
    if number(r,'setupFail')!=last:
        increments.append(dict(tick=r['tick'],setupFail=r['setupFail'],setupOk=r['setupOk'],delta=number(r,'setupFail')-last))
        last=number(r,'setupFail')
selected=['CAPACITY_QUERY_STATUS','STREAM_HANDLER_STATUS','AUDIO_MODEL_STATUS','AUDIO_MODEL_REGISTRY','REPLAY_READ_STATUS','REPLAY_CLIENT_STATUS','REGMAP_STATUS','PARKED150_STATUS','REPLAY150_STATUS','GENERIC_STATUS','BASELINE_METRIC','AUDIO_PRESSURE']
summary=dict(input=dict(path=str(path.relative_to(ROOT)),bytes=path.stat().st_size,sha256=hashlib.sha256(path.read_bytes()).hexdigest()),
    record_counts=dict(Counter(r['type'] for r in records)),last_tick=max(number(r,'tick') for r in records),phases=phases,
    final={name:of(name)[-1] if of(name) else None for name in selected},
    peaks=dict(stream=peak('STREAM_HANDLER_STATUS',['currentSampled','peakSampled','queue0Peak','queue1Peak','queue2Peak','queue3Peak']),
               baseline={name:max((number(r,'peakSampled') for r in of('BASELINE_METRIC') if r.get('name')==name),default=0) for name in ['hashExtraLifetimeIssued','processPrivateBytes','processWorkingBytes']},
               audio=peak('AUDIO_PRESSURE',['usedBytes','aboveThresholdSampled','pressureSamples'])),
    audio_failed_models=failures,audio_failure_increments=increments,
    failed_setup_details=[r for r in of('AUDIO_MODEL_DETAIL') if r.get('kind')=='setupResult' and int(r.get('a','0'),16)==0],
    capacity_query_changes=[],audio_recovery=[],fatal_records=[r for r in records if 'FATAL' in r['type'] or 'EXCEPTION' in r['type']])
for final in failures:
    samples=[r for r in of('AUDIO_MODEL_KEY') if r['model']==final['model']]
    failed=[r for r in samples if number(r,'setupFail')]
    before=[r for r in samples if number(r,'tick')<429390]
    after=[r for r in samples if number(r,'tick')>432484 and number(r,'setupOk')>number(failed[-1] if not before else before[-1],'setupOk')]
    summary['audio_recovery'].append(dict(model=final['model'],before_transition=before[-1] if before else None,first_failed=failed[0],first_later_success=after[0] if after else None,final=final))
previous=None
for r in of('CAPACITY_QUERY_STATUS'):
    key=(r.get('overrides'),r.get('fallbacks'))
    if key!=previous:summary['capacity_query_changes'].append(r);previous=key
(OUT/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(dict(last_tick=summary['last_tick'],peaks=summary['peaks'],failed_models=[r['model'] for r in failures],audio_failure_increments=increments,
    recovery=[dict(model=r['model'],before_ok=r['before_transition']['setupOk'] if r['before_transition'] else None,later_success_tick=r['first_later_success']['tick'] if r['first_later_success'] else None,final_ok=r['final']['setupOk'],final_fail=r['final']['setupFail']) for r in summary['audio_recovery']],
    capacity_query_final=summary['final']['CAPACITY_QUERY_STATUS'],replay=summary['final']['REPLAY_READ_STATUS'],fatals=summary['fatal_records']),indent=2))
print('PHASES',json.dumps([dict(seq=p['seq'],count=p['count'],started=p['started'],ready=p['ready'],first_ready_tick=p['first_ready']['tick'] if p['ready'] else None,last_tick=p['last']['tick']) for p in phases]))

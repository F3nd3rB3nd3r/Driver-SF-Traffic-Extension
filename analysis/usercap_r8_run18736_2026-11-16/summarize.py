"""Streaming summary of the single supplied89MB R8 log; no game/source changes."""
import collections,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
path=ROOT/'dist/USERCAP_R8/DSFV29s_usercap_r8_18736.log'
names={'GENERIC_START','GENERIC_EVENT','BASELINE_PHASE','BASELINE_METRIC','MANAGER_KEY_STATUS','STREAM_HANDLER_STATUS','CAPACITY_QUERY_STATUS',
 'AUDIO_MODEL_STATUS','AUDIO_MODEL_KEY','AUDIO_MODEL_REGISTRY','AUDIO_PRESSURE','REPLAY_READ_STATUS','REPLAY150_STATUS','REPLAY_PAUSE_STATUS','REGMAP_STATUS','PARKED150_STATUS'}
counts=collections.Counter();final={};phases={};events=[];keys={};audio_changes=[];last_fail=0;metrics={};key_changes=[];prior_key=None;key_peaks={};latest_tick=0;fatal=[];per_key={}
def n(r,k):return int(r.get(k,'0'))
with path.open(encoding='utf-8-sig') as f:
    for lineno,line in enumerate(f,1):
        parts=line.split()
        if not parts:continue
        kind=parts[0];counts[kind]+=1;name=kind.removeprefix('DSFV29s_')
        if name not in names and not ('FATAL' in kind or 'EXCEPTION' in kind):continue
        row=dict(type=kind,line=lineno,**dict(p.split('=',1) for p in parts[1:] if '=' in p))
        # Selected records have no payload field named type or line.
        final[name]=row;latest_tick=max(latest_tick,n(row,'tick'))
        if 'FATAL' in kind or 'EXCEPTION' in kind:fatal.append(row)
        if name=='GENERIC_EVENT':events.append(row)
        elif name=='BASELINE_PHASE':
            seq=row['seq'];p=phases.setdefault(seq,dict(seq=seq,accepted=None,first_ready=None,last=row))
            if row['kind']=='accepted':p['accepted']=row
            if n(row,'checkpoints')>0 and row.get('ready')==row.get('count') and p['first_ready'] is None:p['first_ready']=row
            p['last']=row
        elif name=='BASELINE_METRIC':metrics[row['name']]=max(metrics.get(row['name'],0),n(row,'peakSampled'))
        elif name=='AUDIO_MODEL_STATUS' and n(row,'setupFail')!=last_fail:
            previous=events[-1] if events else None
            audio_changes.append(dict(tick=n(row,'tick'),delta=n(row,'setupFail')-last_fail,total=n(row,'setupFail'),preceding_traffic=previous,
                ms_after_traffic=n(row,'tick')-n(previous,'tick') if previous else None));last_fail=n(row,'setupFail')
        elif name=='AUDIO_MODEL_KEY':
            model=row['model'];old=keys.get(model,{});state=per_key.setdefault(model,dict(last_failure_tick=None,success_after_last_failure=None))
            if n(row,'setupFail')>n(old,'setupFail'):
                state['last_failure_tick']=n(row,'tick');state['success_after_last_failure']=None
            if n(row,'setupOk')>n(old,'setupOk') and state['last_failure_tick'] is not None and n(row,'tick')>state['last_failure_tick']:
                state['success_after_last_failure']=state['success_after_last_failure'] or n(row,'tick')
            keys[model]=row
        elif name=='MANAGER_KEY_STATUS':
            for k in ['peakPerListProjected','activeSampled','buffers','redirects','generations','destructions','alive']:key_peaks[k]=max(key_peaks.get(k,0),n(row,k))
            stamp=tuple(row.get(k) for k in ['generations','destructions','alive','buffers','redirects','peakPerListProjected'])
            if stamp!=prior_key:key_changes.append(row);prior_key=stamp
summary=dict(input=dict(path=str(path.relative_to(ROOT)),bytes=path.stat().st_size,sha256=hashlib.sha256(path.read_bytes()).hexdigest()),
 latest_tick=latest_tick,record_counts=dict(counts),final=final,phases=list(phases.values()),metrics=metrics,manager_key_peaks=key_peaks,manager_key_changes=key_changes,
 audio_failure_changes=audio_changes,audio_failed_models=[dict(final=r,**per_key[model]) for model,r in keys.items() if n(r,'setupFail')],fatal_records=fatal)
(OUT/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(dict(input=summary['input'],duration_ms=latest_tick,manager_key_final=final['MANAGER_KEY_STATUS'],manager_key_peaks=key_peaks,metrics=metrics,
 phases=[dict(seq=p['seq'],count=p['accepted']['count'] if p['accepted'] else None,started=p['accepted']['started'] if p['accepted'] else None,ready=p['first_ready']['tick'] if p['first_ready'] else None,last_ready=p['last'].get('ready')) for p in phases.values()],
 audio_failures=audio_changes,audio_failed_models=[dict(model=x['final']['model'],ok=x['final']['setupOk'],fail=x['final']['setupFail'],last_failure_tick=x['last_failure_tick'],success_after_last_failure=x['success_after_last_failure']) for x in summary['audio_failed_models']],
 audio_final=final['AUDIO_MODEL_STATUS'],replay=final['REPLAY_READ_STATUS'],fatal_count=len(fatal)),indent=2))

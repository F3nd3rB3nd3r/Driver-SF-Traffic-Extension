"""Focused analysis of the supplied R7 replay/Story lifecycle log only."""
import hashlib,json,sys
from collections import Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'scripts'))
from compare_audio_slots_logs import rows
from analyze_replay_client_log import analyze as client_analyze
from analyze_replay_read1m_log import FIELDS,HEX
path=ROOT/'dist/USERCAP_R7/DSFV29s_usercap_r7_11636.log'
records=rows(path);of=lambda name:[r for r in records if r['type']=='DSFV29s_'+name]
def n(r,k):return int(r.get(k,'0'),16 if k in HEX else 10)&0xFFFFFFFF
cfg=of('GENERIC_START')[0];capacity=n(cfg,'replayReaderMiB')*1048576
final_names=['CAPACITY_QUERY_STATUS','STREAM_HANDLER_STATUS','AUDIO_MODEL_STATUS','AUDIO_MODEL_REGISTRY','REPLAY_READ_STATUS','REPLAY_CLIENT_STATUS','REPLAY_PAUSE_STATUS','REPLAY150_STATUS','REGMAP_STATUS','PARKED150_STATUS','GENERIC_STATUS']
summary=dict(input=dict(path=str(path.relative_to(ROOT)),bytes=path.stat().st_size,sha256=hashlib.sha256(path.read_bytes()).hexdigest()),
    last_tick=max(n(r,'tick') for r in records),configuration=cfg,counts=dict(Counter(r['type'] for r in records)),
    final={name:of(name)[-1] if of(name) else None for name in final_names},phases=[],
    replay_events=of('REPLAY150_EVENT'),pause_events=of('REPLAY_PAUSE_EVENT'),
    fatals=[r for r in records if 'FATAL' in r['type'] or 'EXCEPTION' in r['type']])
for start in of('BASELINE_PHASE'):
    if start.get('kind')!='accepted':continue
    samples=[r for r in of('BASELINE_PHASE') if r.get('seq')==start['seq']]
    ready=[r for r in samples if n(r,'ready')==n(start,'count') and n(r,'checkpoints')]
    summary['phases'].append(dict(seq=start['seq'],count=start['count'],start=start['started'],all_ready=bool(ready),first_ready_tick=ready[0]['tick'] if ready else None,last=samples[-1]))
summary['metric_peaks']={name:max((n(r,'peakSampled') for r in of('BASELINE_METRIC') if r.get('name')==name),default=0) for name in ['hashExtraLifetimeIssued','processPrivateBytes','processWorkingBytes']}
summary['stream_peaks']={k:max(n(r,k) for r in of('STREAM_HANDLER_STATUS')) for k in ['currentSampled','queue0Peak','queue1Peak','queue2Peak','queue3Peak']}
owners=of('REPLAY_READ_OWNER');latest={r['owner']:r for r in owners}
boundaries=[r for r in of('REPLAY_CLIENT_EVENT') if n(r,'backend')==1 and n(r,'ioMode')==0 and n(r,'active')==1]
errors=[]
for r in owners:
    if not set(FIELDS).issubset(r):errors.append(dict(line=r['line'],reason='owner_missing_fields'));continue
    if n(r,'rfPresent')!=1 or n(r,'rfState') not in (0,1,2) or n(r,'rfCapacity')!=capacity or not n(r,'rfBuffer') or n(r,'rfBuffer')&0xFFFF:errors.append(dict(line=r['line'],reason='owner_backing_or_state'))
    if n(r,'rfState')!=2 and n(r,'rfGeneration') and n(r,'originalCapacity')!=262144:errors.append(dict(line=r['line'],reason='native_capacity'))
    if n(r,'rfIssueRejected') or n(r,'rfMinFree')>capacity or n(r,'rfLastStop')>4:errors.append(dict(line=r['line'],reason='reader_counter_bounds_or_rejected'))
for r in boundaries:
    if not set(FIELDS).issubset(r) or n(r,'rfPresent')!=1 or n(r,'rfGeneration')==0 or n(r,'rfState')!=1 or n(r,'rfCapacity')!=capacity or n(r,'capacity')!=capacity or n(r,'rfBuffer')!=n(r,'base') or n(r,'rfIssueRejected'):
        errors.append(dict(line=r['line'],reason='managed_boundary'))
client=client_analyze(path)
legacy_start=client['checks'].pop('startup');client['legacy_fixed150_startup_check']=legacy_start
start=of('REPLAY_CLIENT_START')[0]
client['checks']['configured_startup']=len(of('REPLAY_CLIENT_START'))==1 and all(n(start,k)==v for k,v in dict(revision=1,status=1,hooks=3,clients=4,broadHooks=0,boundaryOnly=1,clearFastPath=1,extraEngineCalls=0,gameThreadFileWrites=0,requests=n(cfg,'requestCapacity'),graphics=n(cfg,'modelSlots')).items())
client['structural_pass']=all(client['checks'].values())
summary['replay_reader']=dict(configured_bytes=capacity,owners=latest,owner_count=len(latest),owner_rows=len(owners),managed_boundaries=len(boundaries),errors=errors,
    counters={k:max((n(r,k) for r in owners),default=0) for k in ['rfPolls','rfIssues','rfIssueBytes','rfIssueRejected','rfBackendBusy','rfSpaceStops','rfWrapStops','rfEndStops']})
summary['client']=client
(OUT/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(dict(last_tick=summary['last_tick'],final=summary['final'],metric_peaks=summary['metric_peaks'],stream_peaks=summary['stream_peaks'],phases=[{k:v for k,v in p.items() if k!='last'} for p in summary['phases']],
    reader={k:v for k,v in summary['replay_reader'].items() if k!='owners'},client_checks=client['checks'],client_final=client['final_status'],client_conditions=client['observed_conditions'],interval_count=len(client['intervals']),interval_examples=client['intervals'][:3],unfinished=len(client['unfinished']),fatal_count=len(summary['fatals'])),indent=2))

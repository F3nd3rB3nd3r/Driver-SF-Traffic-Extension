"""Reproduce run30212 client3 boundary comparison; source log is read-only."""
from pathlib import Path
import hashlib,json,statistics,sys
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'scripts'))
from analyze_replay_client_log import number as n,floating as f
path=ROOT/'dist/MAIN_150/DSFV29s_replay_client_30212.log';raw=path.read_bytes();events=[];statuses=[]
for line_no,line in enumerate(raw.decode(errors='replace').splitlines(),1):
    if not line.startswith(('DSFV29s_REPLAY_CLIENT_EVENT','DSFV29s_REPLAY150_STATUS')):continue
    r=dict(x.split('=',1) for x in line.split()[1:] if '=' in x);r['line']=line_no
    if line.startswith('DSFV29s_REPLAY_CLIENT_EVENT'):events.append(r)
    elif r.get('mode')=='2':statuses.append(r)
pending={};pairs=[]
for r in events:
    key=(r['thread'],r['sequence'])
    if r['kind']=='busy':pending[key]=r
    elif r['kind'] in ('clear','contextEnd'):
        begin=pending.pop(key);pairs.append((begin,r))
normal=[(a,b) for a,b in pairs if a['client']=='3' and b['kind']=='clear' and all(n(a,k)==0 for k in ('phase','transition','loading')) and f(n(a,'requestedBits'))==1]
late=[(a,b) for a,b in normal if (f(n(a,'clockBits')) or 0)>=130]
def simplify(a,b):
    return {'begin_tick':n(a,'tick'),'end_tick':n(b,'tick'),'begin_line':a['line'],'end_line':b['line'],
        'clock_before':f(n(a,'clockBits')),'clock_after':f(n(b,'clockBits')),'current_time':f(n(a,'curTimeBits')),
        'next_time_before':f(n(a,'nextTimeBits')) if n(a,'nextValid') else None,'next_time_after':f(n(b,'nextTimeBits')) if n(b,'nextValid') else None,
        'duration_ms':n(b,'elapsedTicks')/10000,'current_bytes':n(a,'curBytes'),
        'backend':n(a,'backend'),'io_mode':n(a,'ioMode'),'capacity':n(a,'capacity'),'free_before':n(a,'freeBytes'),'free_after':n(b,'freeBytes'),
        'unparsed_before':n(a,'unparsedBytes'),'unparsed_after':n(b,'unparsedBytes'),'in_flight_before':n(a,'inFlightBytes'),'in_flight_after':n(b,'inFlightBytes'),
        'end_flag_before':n(a,'endOfData'),'end_flag_after':n(b,'endOfData'),'position_delta':n(b,'filePosition')-n(a,'filePosition'),
        'capture_before_ms':n(a,'captureTicks')/10000,'capture_after_ms':n(b,'captureTicks')/10000}
b=json.loads((OUT/'baseline.json').read_text());c=json.loads((OUT/'callback.json').read_text());probe=json.loads((OUT/'client.json').read_text())
late_rows=[simplify(a,b) for a,b in late]
result={'file':{'bytes':len(raw),'lines':raw.count(b'\n'),'sha256':hashlib.sha256(raw).hexdigest(),'complete_final_newline':raw.endswith(b'\n')},
    'integrity':{'baseline':all(b['checks'].values()),'callback':c['recorded_integrity_pass'],'client':probe['structural_pass']},
    'normal_client3_intervals':[simplify(a,b) for a,b in normal],'late_client3_intervals':late_rows,
    'late_summary':{'count':len(late_rows),'duration_min_ms':min(r['duration_ms'] for r in late_rows),'duration_max_ms':max(r['duration_ms'] for r in late_rows),
        'free_min_bytes':min(r['free_before'] for r in late_rows),'free_max_bytes':max(r['free_before'] for r in late_rows),
        'all_free_above_one_io_block':all(r['free_before']>32768 for r in late_rows),'all_no_bookkept_read_or_end_flag_at_begin':all(r['in_flight_before']==r['end_flag_before']==0 for r in late_rows),
        'all_read_pending_after_clear':all(r['in_flight_after']==32768 for r in late_rows),'position_delta_bytes':[r['position_delta'] for r in late_rows]},
    'playback_status_range':{'first_tick':n(statuses[0],'tick'),'last_tick':n(statuses[-1],'tick'),'first_clock':f(n(statuses[0],'clockBits')),'last_clock':f(n(statuses[-1],'clockBits'))},
    'probe_final':probe['final_status'],'capture_total_ms':probe['capture_body_total_ms'],
    'roster_phases':len(b['phases']),'native_ready_sweeps':sum(x.get('all_ready_in_recorded_sweep',False) for x in b['phases']),
    'snapshot_events':[{k:r.get(k) for k in ('kind','tick','count','rosterHash')} for r in b['replay']['events'] if r.get('kind') in ('import','restore')],
    'limitations':['Busy snapshots are post-return observations, not a continuous fullness/read-completion trace.',
        'File-position increments reflect native scheduled/progress bookkeeping and do not count synchronous disk reads.',
        'Capture-body time is a partial overhead measure, not total probe CPU/frame cost.']}
(OUT/'comparison.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k not in ('probe_final','normal_client3_intervals')},indent=2))

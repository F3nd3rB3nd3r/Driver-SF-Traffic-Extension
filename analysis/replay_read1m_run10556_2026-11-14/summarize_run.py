"""Reproduce the supplied run10556 comparison; never modifies runtime/log files."""
import hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
path=ROOT/'dist/MAIN_150/DSFV29s_replay_read1m_10556.log';raw=path.read_bytes()
read=json.loads((OUT/'read.json').read_text());baseline=json.loads((OUT/'baseline.json').read_text());callback=json.loads((OUT/'callback.json').read_text())
previous=json.loads((ROOT/'analysis/replay_client_run30212_2026-11-14/comparison.json').read_text())
def clock(r):return struct.unpack('<f',struct.pack('<I',int(r['clockBits'],16)))[0]
owners={};segments=[];segment=None;minimums={};failures=[]
for line_no,line in enumerate(raw.decode(errors='replace').splitlines(),1):
    if not line.startswith(('DSFV29s_REPLAY_READ_OWNER ','DSFV29s_REPLAY150_STATUS ')):continue
    row=dict(x.split('=',1) for x in line.split()[1:] if '=' in x)
    if line.startswith('DSFV29s_REPLAY_READ_OWNER '):
        owners[row['owner']]=row
        if int(row['rfGeneration'])>0:
            key=(row['owner'],int(row['rfGeneration']))
            minimums[key]=min(minimums.get(key,1048576),int(row['rfMinFree']))
        continue
    if row.get('mode')!='2':segment=None;continue
    tick=int(row['tick']);time=clock(row);owner=row['replay'];generation=int(owners.get(owner,{}).get('rfGeneration',0))
    # OWNER rows are periodic and can lag this status by one sample. A changed
    # generation hint is not independently a new playback/seek boundary.
    if segment is None or owner!=segment['owner'] or time<segment['last_clock']-0.1:
        segment={'owner':owner,'first_generation_hint':generation,'last_generation_hint':generation,'first_tick':tick,'last_tick':tick,'first_clock':time,'last_clock':time,'minimum_clock':time,'maximum_clock':time,'samples':0,'forward_samples':0,'steady_samples':0,'near_1x_intervals':0,'samples_130_to_140':0}
        segments.append(segment)
    elif tick>segment['last_tick']:
        dt=time-segment['last_clock'];wall=(tick-segment['last_tick'])/1000
        if dt>0.001:segment['forward_samples']+=1
        else:segment['steady_samples']+=1
        if abs(dt-wall)<0.15:segment['near_1x_intervals']+=1
    segment['samples']+=1;segment['last_tick']=tick;segment['last_clock']=time;segment['last_generation_hint']=generation
    segment['minimum_clock']=min(segment['minimum_clock'],time);segment['maximum_clock']=max(segment['maximum_clock'],time)
    segment['samples_130_to_140']+=int(130<=time<=140)
totals={}
for row in read['latest_owners'].values():
    for k in ('rfPolls','rfBackendBusy','rfCommits','rfCommittedBytes','rfWrapChecks','rfWrapStops','rfSpaceChecks','rfSpaceStops','rfIssues','rfIssueBytes','rfIssueRejected','rfEndChecks','rfEndStops'):
        totals[k]=totals.get(k,0)+int(row[k])
normal=[r for r in read['client']['intervals'] if r['phase']==r['transition']==r['loading']==0 and r['requested']==1]
inherited=callback['inherited']
result={'file':{'bytes':len(raw),'lines':raw.count(b'\n'),'sha256':hashlib.sha256(raw).hexdigest(),'complete_final_newline':raw.endswith(b'\n'),'last_recorded_tick':inherited['last_recorded_tick']},
        'integrity':{'read':read['structural_pass'],'client':read['client']['structural_pass'],'baseline':all(baseline['checks'].values()),'callback':callback['recorded_integrity_pass'],'inherited':inherited['inherited_structural_checks_pass']},
        'user_observation':{'bursts':'User did not recall seeing bursting indicator/choppy motion.','sequence':'Main menu -> Replay -> full viewing -> main menu -> Story -> full replay viewing -> main menu -> normal quit.','render_audio_fps':'No explicit assessment supplied in this reply.'},
        'previous_30212':{'normal_client3_waits':len(previous['normal_client3_intervals']),'late_client3_waits':previous['late_summary']['count'],'late_duration_ms':[previous['late_summary']['duration_min_ms'],previous['late_summary']['duration_max_ms']]},
        'current_10556':{'client3_busy_results':int(read['client']['final_status']['busy3']),'normal_waits':normal,'all_intervals':read['client']['intervals'],'final_client_status':read['client']['final_status'],
                         'reader_final':read['final_status'],'reader_totals':totals,'latest_owners':read['latest_owners'],'owner_generation_minimum_free':[{'owner':key[0],'generation':key[1],'minimum_free':v} for key,v in minimums.items()],
                         'sampled_playback_segments':segments,'segments_covering_130_to_140':sum(s['samples_130_to_140']>0 for s in segments)},
        'inherited':{'roster_phases':len(baseline['phases']),'all_ready_sequential_sweeps':sum(p.get('all_ready_in_recorded_sweep',False) for p in baseline['phases']),
                     'replay_final':baseline['replay']['final'],'pause_final':baseline['replay_pause']['final'],'parked_final':baseline['parked']['final'],'regmap_final':baseline['regmap']['final'],
                     'audio_totals':inherited['audio']['totals'],'cleanup':inherited['cleanup'],'pending':inherited['pending']},
        'limitations':['Two full viewings reported by user; sampled segments include paused/end/transition state and do not independently certify every frame was normal1x.',
                       'Zero client3 waits is evidence for the instrumented path, not an exhaustive smoothness or full-playthrough guarantee.',
                       'Space/wrap/backend-busy counters can be normal prefetch backpressure; the observed stop counts do not justify another increase.',
                       'No controlled identical recording/hardware/frame-timing A/B comparison or total probe-overhead benchmark.']}
(OUT/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'file':result['file'],'integrity':result['integrity'],'reader_totals':totals,'segments':segments,
                  'roster_phases':result['inherited']['roster_phases'],'all_ready_sweeps':result['inherited']['all_ready_sequential_sweeps']},indent=2))

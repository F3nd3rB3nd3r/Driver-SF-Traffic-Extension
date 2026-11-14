"""Read passive replay timing. Does not certify playback, vehicles or gate causes."""
import argparse,json,math,struct
from collections import Counter
from pathlib import Path

PREFIX='DSFV29s_REPLAY_HITCH_'
def number(row,key,default=0):
    value=row.get(key)
    if value is None:return default
    return int(value,16 if 'Bits' in key or key in ('caller','arg0','arg1','result','owner','recording','ownerBefore','recordingBefore') else 10)
def floating(bits):
    v=struct.unpack('<f',struct.pack('<I',bits))[0]
    return v if math.isfinite(v) else None
def analyze(path):
    starts=[];statuses=[];events=[];malformed=[]
    with Path(path).open(errors='replace') as f:
        for line_no,line in enumerate(f,1):
            if not line.startswith(PREFIX):continue
            parts=line.split();row=dict(part.split('=',1) for part in parts[1:] if '=' in part);row['line']=line_no
            if not line.endswith('\n'):malformed.append(line_no)
            if parts[0]==PREFIX+'START':starts.append(row)
            elif parts[0]==PREFIX+'STATUS':statuses.append(row)
            elif parts[0]==PREFIX+'EVENT':events.append(row)
    frequency=number(starts[-1],'qpcFrequencyLo')+(number(starts[-1],'qpcFrequencyHi')<<32) if starts else 0
    ms=lambda ticks:1000*ticks/frequency if frequency else None
    kinds=Counter(r.get('kind','missing') for r in events)
    windows=[r for r in events if r.get('kind')=='window']
    calls=[r for r in events if r.get('kind')=='call']
    operations={}
    for name in ('seek','stream','import','restore','request'):
        rows=[r for r in calls if r.get('op')==name]
        operations[name]={'count':len(rows),'maximum_ms':max((ms(number(r,'duration')) for r in rows),default=None)}
    holds=[]
    for r in events:
        if r.get('kind') not in ('hold','recovered'):continue
        holds.append({k:number(r,k) for k in ('tick','duration','phase','transition','loading','paused','busy20','busy24','subscriberOnlyBusy','snapshotPendingAtBusy','trafficPendingAtBusy','maxStepGapMs','maxUpdateGapMs')}
                     |{'kind':r['kind'],'duration_unit':'ms','requested':floating(number(r,'requestedBits')),'applied':floating(number(r,'appliedBits'))})
    gate_waits=[{'tick':number(r,'tick'),'op':r.get('op'),'duration_ms':ms(number(r,'duration')),'phase_before':number(r,'phaseBefore'),'phase_after':number(r,'phase'),'traffic_before':number(r,'pendingTrafficBefore'),'traffic_after':number(r,'pendingTraffic')} for r in events if r.get('kind')=='gateWait']
    seeks=[{'tick':number(r,'tick'),'caller':r.get('caller'),'target':floating(number(r,'arg0')),'force':number(r,'arg1')&255,
            'accepted':bool(number(r,'result')&255),'checkpoint':floating(number(r,'selectedBits')),'duration_ms':ms(number(r,'duration'))} for r in calls if r.get('op')=='seek']
    counters=('stepCalls','updateCalls','busy20','busy24','subscriberOnlyBusy','snapshotPendingAtBusy','trafficPendingAtBusy','unknownBusy','advanced','unchanged','backwards')
    checks={
        'startup':bool(starts) and all(number(r,'revision')==1 and number(r,'status')==1 and number(r,'hooks')==5 and number(r,'nativeGates')==1 and number(r,'duplicateCalls')==0 and number(r,'gameThreadFileWrites')==0 and number(r,'requests')==150 and number(r,'graphics')==160 for r in starts) and frequency>0,
        'status_present':bool(statuses),
        'telemetry_integrity':bool(statuses) and all(number(r,'status')==1 and all(k in r and number(r,k)==0 for k in ('dropped','threadDrops','depthDrops','mismatches','clockFailures')) for r in statuses),
        'records_complete':not malformed,
        'events_present':bool(events),
    }
    return {'path':str(path),'checks':checks,'structural_pass':all(checks.values()),'frequency':frequency,'event_counts':dict(kinds),
            'operations':operations,'completed_window_totals':{k:sum(number(r,k) for r in windows) for k in counters},
            'window_max_step_gap_ms':max((number(r,'maxStepGapMs') for r in windows),default=None),
            'window_max_update_gap_ms':max((number(r,'maxUpdateGapMs') for r in windows),default=None),
            'observer_body_ms_in_completed_windows':ms(sum(number(r,'observerBodyTicks') for r in windows)),
            'holds':holds,'gate_waits':gate_waits,'seeks':seeks,'malformed_lines':malformed,
            'playback_smoothness_certified':False,
            'limitations':['Gate pending-state counters can overlap subscriber waits; only subscriberOnlyBusy excludes the fallback gate.',
                           'gateWait measures the observed busy-to-clear polling interval, including scheduling gaps; unfinished or sub-1/1024-second waits are not emitted.',
                           'Call durations include nested probes and inherited synchronous logging; nested durations must not be summed as exclusive work.',
                           'observerBodyTicks excludes assembly/QPC boundary costs and the monitor writer; it is not total instrumentation overhead.',
                           'Windows are game-thread call driven. Final partial windows/queued events can be absent on process exit.',
                           'Clock holds with nonzero requested rate include deliberate pause/transition/rate scenarios; correlate state and user observations.']}

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--output',type=Path);a=p.parse_args()
    result=analyze(a.log);data=json.dumps(result,indent=2)+'\n'
    if a.output:a.output.write_text(data)
    print(data)

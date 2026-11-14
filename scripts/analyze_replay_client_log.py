"""Analyze narrow replay client busy boundaries without certifying a root cause."""
import argparse,json,math,struct
from collections import Counter
from pathlib import Path
PREFIX='DSFV29s_REPLAY_CLIENT_'
HEX={'owner','self','recording','vtable','caller','gateCaller','recordingVtable','base','retained','filled','parse','eventHead'}
def number(row,key,default=0):
    value=row.get(key)
    if value is None:return default
    result=int(value,16 if key in HEX or key.endswith(('Bits','Ptr','TypeFlags','Link')) else 10)
    # The inherited decimal formatter accepts signed long. Restore raw U32
    # QPC counters if a long interval prints in signed decimal notation.
    return result&0xFFFFFFFF if key in ('elapsedTicks','captureTicks') else result
def floating(bits):
    v=struct.unpack('<f',struct.pack('<I',bits))[0]
    return v if math.isfinite(v) else None
def observed_conditions(r):
    reasons=[];client=number(r,'client');cur=number(r,'curPtr');next_=number(r,'nextPtr')
    if not cur:reasons.append('missing_current')
    elif not number(r,'curValid'):reasons.append('unreadable_current')
    elif client in (1,3,7):
        if not next_:reasons.append('missing_next')
        elif not number(r,'nextValid'):reasons.append('unreadable_next')
        elif client==1 and not number(r,'futurePtr'):
            clock=floating(number(r,'clockBits'));time=floating(number(r,'nextTimeBits'))
            if clock is not None and time is not None and clock>time:reasons.append('expired_lookahead')
    if client==1 and number(r,'vehiclePending'):reasons.append('vehicle_pending_flag')
    return reasons or ['no_busy_condition_in_post_return_snapshot']
def analyze(path):
    starts=[];statuses=[];events=[];incomplete=[]
    with Path(path).open(errors='replace') as stream:
        for line_no,line in enumerate(stream,1):
            if not line.startswith(PREFIX):continue
            parts=line.split();row=dict(p.split('=',1) for p in parts[1:] if '=' in p);row['line']=line_no
            if not line.endswith('\n'):incomplete.append(line_no)
            if parts[0]==PREFIX+'START':starts.append(row)
            elif parts[0]==PREFIX+'STATUS':statuses.append(row)
            elif parts[0]==PREFIX+'EVENT':events.append(row)
    start=starts[-1] if starts else {};frequency=number(start,'qpcFrequencyLo')+(number(start,'qpcFrequencyHi')<<32)
    expected={'revision':1,'status':1,'hooks':3,'clients':4,'broadHooks':0,'boundaryOnly':1,'clearFastPath':1,'extraEngineCalls':0,'gameThreadFileWrites':0,'requests':150,'graphics':160}
    counters=('dropped','threadDrops','unknown','readFailures','clockFailures');pending={};intervals=[];pair_errors=[]
    mandatory={'client','sequence','thread','tick','caller','elapsedTicks','captureTicks','owner','self','recording','clockBits','phase','transition','loading','requestedBits','curPtr','curValid','nextPtr','nextValid','futurePtr','recordingValid','ringValid','inFlightBytes','endOfData'}
    valid_events=all(mandatory.issubset(r) and number(r,'client') in (1,2,3,7) and number(r,'caller')==number(start,'gateCaller') for r in events)
    for r in events:
        key=(number(r,'thread'),number(r,'sequence'));kind=r.get('kind')
        if kind=='busy':
            if key in pending:pair_errors.append({'line':r['line'],'reason':'duplicate_begin'})
            pending[key]=r
        elif kind in ('clear','contextEnd'):
            before=pending.pop(key,None)
            if before is None:pair_errors.append({'line':r['line'],'reason':'missing_begin'});continue
            if r.get('client')!=before.get('client') or r.get('owner')!=before.get('owner'):pair_errors.append({'line':r['line'],'reason':'identity_changed'})
            intervals.append({'client':number(r,'client'),'kind':kind,'begin_tick':number(before,'tick'),'end_tick':number(r,'tick'),
                'duration_ms':number(r,'elapsedTicks')*1000/frequency if frequency else None,
                'begin_clock':floating(number(before,'clockBits')),'end_clock':floating(number(r,'clockBits')),
                'observed_begin_conditions':observed_conditions(before),'phase':number(before,'phase'),'transition':number(before,'transition'),'loading':number(before,'loading'),
                'requested':floating(number(before,'requestedBits')),'ring_valid':bool(number(before,'ringValid')),
                'capacity':number(before,'capacity'),'free_bytes':number(before,'freeBytes'),'unparsed_bytes':number(before,'unparsedBytes'),
                'in_flight_bytes':number(before,'inFlightBytes'),'end_flag':number(before,'endOfData'),'begin_line':before['line'],'end_line':r['line']})
        elif kind=='held':
            if key not in pending:pair_errors.append({'line':r['line'],'reason':'held_without_begin'})
        else:pair_errors.append({'line':r['line'],'reason':'unknown_event'})
    checks={'startup':bool(starts) and frequency>0 and number(start,'gateCaller')!=0 and all(k in s and number(s,k)==v for s in starts for k,v in expected.items()),
        'status_present':bool(statuses),'telemetry_integrity':bool(statuses) and all(number(s,'status')==1 and all(k in s and number(s,k)==0 for k in counters) for s in statuses),
        'records_complete':not incomplete,'event_schema':valid_events,'event_pairs':not pair_errors}
    busy=[r for r in events if r.get('kind')=='busy'];conditions=Counter(c for r in busy for c in observed_conditions(r))
    return {'path':str(path),'checks':checks,'structural_pass':all(checks.values()),'frequency':frequency,'startup':start,'final_status':statuses[-1] if statuses else None,
        'events':len(events),'busy_starts_by_client':dict(Counter(number(r,'client') for r in busy)),'observed_conditions':dict(conditions),'intervals':intervals,
        'unfinished':list(pending.values()),'pair_errors':pair_errors,'incomplete_lines':incomplete,
        'capture_body_total_ms':sum(number(r,'captureTicks') for r in events)*1000/frequency if frequency else None,
        'root_cause_certified':False,'playback_smoothness_certified':False,
        'limitations':['Snapshots are after the original native result; conditions are observed state, not an independently observed instruction branch.',
            'Busy intervals include polling/scheduler gaps. contextEnd is not a readiness recovery; final open intervals may lack an end.',
            'freeBytes follows native ring convention and does not include unlogged I/O completion/read status.',
            'inFlightBytes and endOfData are raw bookkeeping; a set end flag does not prove every queued event was consumed.',
            'captureTicks excludes the fast path, bridge/observer framing and monitor output; it is not a full overhead benchmark.',
            'No busy events is a valid unexercised diagnostic result, not proof of smooth playback.']}
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--output',type=Path);a=p.parse_args();r=analyze(a.log);data=json.dumps(r,indent=2)+'\n'
    if a.output:a.output.write_text(data)
    print(data)

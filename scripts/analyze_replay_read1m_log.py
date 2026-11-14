"""Validate combined replay-reader capacity/counter telemetry; never certify smoothness."""
import argparse,json
from pathlib import Path
from analyze_replay_client_log import analyze as client_analyze
HEX={'owner','originalBuffer','rfBuffer','recording','base','recordingVtable'}
FIELDS=('rfPresent','rfGeneration','rfState','rfBuffer','rfCapacity','rfPolls','rfBackendBusy','rfCommits','rfCommittedBytes','rfWrapChecks','rfWrapStops','rfSpaceChecks','rfSpaceStops','rfIssues','rfIssueBytes','rfIssueRejected','rfEndChecks','rfEndStops','rfMinFree','rfMaxRequired','rfLastStop','rfLastStopPoll','rfLastStopFree','rfLastStopRequired')
def n(r,k):return int(r.get(k,'0'),16 if k in HEX else 10)&0xFFFFFFFF
def analyze(path):
    starts=[];statuses=[];owners=[];events=[];incomplete=[]
    with Path(path).open(errors='replace') as stream:
        for line_no,line in enumerate(stream,1):
            if not line.startswith(('DSFV29s_REPLAY_READ_','DSFV29s_REPLAY_CLIENT_EVENT ')):continue
            p=line.split();r=dict(x.split('=',1) for x in p[1:] if '=' in x);r['line']=line_no
            if not line.endswith('\n'):incomplete.append(line_no)
            if p[0]=='DSFV29s_REPLAY_READ_START':starts.append(r)
            elif p[0]=='DSFV29s_REPLAY_READ_STATUS':statuses.append(r)
            elif p[0]=='DSFV29s_REPLAY_READ_OWNER':owners.append(r)
            elif p[0]=='DSFV29s_REPLAY_CLIENT_EVENT':events.append(r)
    expected={'revision':1,'status':1,'hooks':8,'readerBytes':1048576,'blockBytes':32768,'readerOnly':1,'writerChanged':0,'separateBacking':1,'guardBytes':65536,'processLifetime':1,'reusable':1,'fixedOwnerLimit':0,'extraEngineCalls':0,'gameThreadFileWrites':0,'requests':150,'graphics':160}
    required_status=('status','owners','constructs','destructs','reuses','writerBypass','allocationFailed','fallbacks','faults','unmanagedCalls','retainedBackingBytes','processLifetime')
    errors=[];latest={}
    for r in owners:
        if not set(FIELDS).issubset(r) or 'owner' not in r:errors.append({'line':r['line'],'reason':'missing_owner_fields'});continue
        if n(r,'rfPresent')!=1 or n(r,'rfState') not in (0,1,2) or n(r,'rfCapacity')!=1048576 or not n(r,'rfBuffer') or n(r,'rfBuffer')&0xFFFF:errors.append({'line':r['line'],'reason':'owner_backing_or_state'})
        # state2 and generation0 are legal transient construction snapshots.
        if n(r,'rfState')!=2 and n(r,'rfGeneration')>0 and n(r,'originalCapacity')!=262144:errors.append({'line':r['line'],'reason':'original_capacity'})
        if n(r,'rfIssueRejected'):errors.append({'line':r['line'],'reason':'native_read_rejected'})
        if n(r,'rfMinFree')>1048576 or n(r,'rfLastStop')>4:errors.append({'line':r['line'],'reason':'counter_bounds'})
        latest[r['owner']]=r
    managed_events=[r for r in events if n(r,'backend')==1 and n(r,'ioMode')==0 and n(r,'active')==1]
    for r in managed_events:
        if not set(FIELDS).issubset(r):errors.append({'line':r['line'],'reason':'missing_boundary_counters'});continue
        if n(r,'rfPresent')!=1 or n(r,'rfGeneration')==0 or n(r,'rfState')!=1 or n(r,'rfCapacity')!=1048576 or n(r,'capacity')!=1048576 or n(r,'rfBuffer')!=n(r,'base'):errors.append({'line':r['line'],'reason':'boundary_not_using_private_1MiB_ring'})
        if n(r,'rfIssueRejected'):errors.append({'line':r['line'],'reason':'boundary_read_rejected'})
    client=client_analyze(path)
    checks={'startup':bool(starts) and all(k in r and n(r,k)==v for r in starts for k,v in expected.items()),
            'status_present':bool(statuses),
            'integrity':bool(statuses) and all(set(required_status).issubset(r) and n(r,'status')==1 and all(n(r,k)==0 for k in ('allocationFailed','fallbacks','faults')) for r in statuses),
            'complete_records':not incomplete,'owner_and_boundary_schema':not errors,'client_integrity':client['structural_pass']}
    exercise=any(n(r,'rfPolls') and n(r,'rfIssues') for r in owners)
    return {'path':str(path),'checks':checks,'structural_pass':all(checks.values()),'startup':starts[-1] if starts else None,
            'final_status':statuses[-1] if statuses else None,'latest_owners':latest,'managed_client_boundaries':len(managed_events),
            'reader_exercised':exercise,'reuse_observed':any(n(r,'reuses') for r in statuses),'schema_errors':errors,
            'boundary_counters':[{k:r[k] for k in ('kind','client','tick','recording','rfGeneration')+FIELDS if k in r} for r in managed_events],
            'client':client,'root_cause_certified':False,'playback_smoothness_certified':False,
            'limitations':['Space/backend-busy stops can be normal prefetch backpressure; counts alone do not identify a visible hitch.',
                           'Individual owner counters are atomic but periodic monitor rows are not one globally coherent instant.',
                           'Counter totals persist across generations; minFree/maxRequired/lastStop reset on managed construction.',
                           'Private payload/backing is retained per wrapper owner until process exit and reused after destructor; retainedBackingBytes is expected, not an active-I/O count.',
                           'Unmanaged calls can come from unchanged native memory readers; fallbacks/faults/issueRejected are separate test-integrity failures.',
                           'No observed waits or no exercised reader is not proof of smooth playback or lifetime correctness.']}
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--output',type=Path);a=p.parse_args();r=analyze(a.log);s=json.dumps(r,indent=2)+'\n'
    if a.output:a.output.write_text(s)
    print(s)

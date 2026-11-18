"""Validate one named generic-capacity log against its recorded configuration."""
import argparse,json
from pathlib import Path
from compare_audio_slots_logs import rows
def analyze(path):
    path=Path(path);records=rows(path);of=lambda name:[r for r in records if r['type']=='DSFV29s_'+name]
    starts=of('GENERIC_START');cfg=starts[0] if len(starts)==1 else {}
    n=lambda r,key,default=-1:int(r.get(key,default))
    capacity=n(cfg,'requestCapacity');retained=n(cfg,'retainedReferences');storage=max(capacity,50)
    checks=dict(configuration=len(starts)==1 and cfg.get('valid')=='1' and cfg.get('compiledCarLists')=='0' and
        cfg.get('reason') in ('configured','missingFile') and cfg.get('file')=='DriverSF_Unlimiter.ini' and
        cfg.get('rosterSource') in ('Native','AssetWeights') and 1<=capacity<=150 and
        max(45,capacity)<=n(cfg,'audioModelEntries')<=512 and 255<=n(cfg,'globalAudioEntries')<=8192 and
        16<=n(cfg,'defaultAudioMiB')<=256 and n(cfg,'replayReaderMiB') in (1,2,4,8,16) and
        max(50,capacity)<=retained<=4096,
        complete_records=path.read_bytes().endswith(b'\n'),no_fatal=not any('FATAL' in r['type'] for r in records))
    wide=cfg.get('userCapacities')=='1'
    checks['layout']=all(n(cfg,key)==value for key,value in dict(requestStorage=storage,
        parkedStride=4*storage+4,parkedFrame=25*(4*storage+4)+632,notifyFrame=4*storage+8,
        replayPayloadCapacity=4*capacity+8*retained+12,**({} if wide else dict(modelSlots=160,auditedRequestCeiling=150))).items())
    if wide:
        # Mirrors representation/layout conditions, never a tested roster cap.
        size=lambda count,stride,header=0:count>=0 and count*stride+header<=0x7FFFFFFF
        models=n(cfg,'modelSlots');audio=n(cfg,'audioModelEntries');reader=n(cfg,'replayReaderMiB')
        checks['configuration']=len(starts)==1 and cfg.get('valid')=='1' and cfg.get('compiledCarLists')=='0' and cfg.get('reason') in ('configured','missingFile') and cfg.get('file')=='DriverSF_Unlimiter.ini' and cfg.get('rosterSource') in ('Native','AssetWeights') and capacity>0 and size(capacity,112,65536) and models>=capacity and size(models,256,65536) and (models*65536+34)//35+1023<=0x7FFFFFFF and audio>=max(45,capacity) and size(audio,256,65536) and n(cfg,'globalAudioEntries')>=255 and size(n(cfg,'globalAudioEntries'),4,65536) and n(cfg,'defaultAudioMiB')>=16 and size(n(cfg,'defaultAudioMiB'),1048576,65536) and reader>0 and not reader&(reader-1) and size(reader,1048576,0x20000) and retained>=max(50,capacity) and size(retained,24,65536+12*capacity+12*models)
        checks['layout'] &= n(cfg,'requestFramePhysicalCapacity')==storage
        queues=dict(objectEntries=(512,12,24),definitionEntries=(45,24,65536),pendingEntries=(64,16,65536),callbackEntriesPerGroup=(64,252,65536),cleanupEntries=(1,4,65536),resourceCollectionEntries=(1,12,0x27AC8),indexedReferenceEntries=(65536,8,65536),hashExtraEntries=(1,24,65536))
        checks['configured_queue_geometry']=all(n(cfg,k)>=minimum and size(n(cfg,k),stride,header) for k,(minimum,stride,header) in queues.items())
    rosters=of('BASELINE_ROSTER');phases=[]
    for event in of('BASELINE_PHASE'):
        if event.get('kind')!='accepted':continue
        found=[r for r in rosters if r.get('seq')==event['seq'] and r.get('kind')=='effective']
        ids=[int(i) for i in found[0].get('ids','').split(',') if i] if len(found)==1 else []
        samples=[r for r in of('BASELINE_PHASE') if r.get('seq')==event['seq']]
        phases.append(dict(seq=event['seq'],count=n(event,'count'),ids=ids,
            complete=len(found)==1 and len(ids)==n(event,'count') and 0<=len(ids)<=capacity and all(0<i<=65535 for i in ids),
            all_ready_observed=any(n(s,'checkpoints',0)>0 and n(s,'ready')==n(event,'count') for s in samples)))
    checks['request_rosters']=bool(phases) and all(p['complete'] for p in phases)
    errors={
        'GENERIC_STATUS':('failures',),'BASELINE_STATUS':('eventDrops','memorySnapshotFailures'),
        'NOTIFY150_STATUS':('invalid',),'PARKED150_STATUS':('invalid','guardBad','concurrentSkips'),
        'REPLAY150_STATUS':('invalid',),'REGMAP_STATUS':('faults','allocationFailed'),
        'REPLAY_READ_STATUS':('faults','fallbacks','allocationFailed'),
        'CALLBACK_STATUS':('invalid','full','guardBad','concurrent'),
        'ARENA_STATUS':('invalid','guardBad','allocationFailed'),
        'AUDIO_MODEL_STATUS':('invalid','instanceOverflow','keyOverflow'),
        'AUDIO_MODEL_REGISTRY':('full','guardBad'),
        'PENDING_STATUS':('invalid','full','allocationFailed','guardBad'),
        'CLEANUP_STATUS':('invalid','full','guardBad','drainMismatch'),
        'REPLAY_CLIENT_STATUS':('dropped','threadDrops','unknown','readFailures','clockFailures')}
    for name,keys in errors.items():
        selected=of(name);checks[name.lower()]=bool(selected) and all(n(r,k)==0 for r in selected for k in keys)
    expected={
        'AUDIO_MODEL_START':dict(status=1,testCapacity=n(cfg,'audioModelEntries')),
        'AUDIO_SLOTS_START':dict(status=1,testMax=n(cfg,'globalAudioEntries'),defaultBytes=n(cfg,'defaultAudioMiB')*1048576),
        'AUDIO_LIMIT_START':dict(status=1,testRequestedBytes=n(cfg,'defaultAudioMiB')*1048576),
        'REPLAY_READ_START':dict(status=1,readerBytes=n(cfg,'replayReaderMiB')*1048576,blockBytes=32768),
        'NOTIFY150_START':dict(status=1,capacity=capacity,frameBytes=4*storage+8),
        'PARKED150_START':dict(status=1,capacity=capacity,perCategory=capacity,frameBytes=25*(4*storage+4)+632),
        'REPLAY150_START':dict(status=1,trafficCapacity=capacity,retainedCapacity=retained,payloadBytes=4*capacity+8*retained+12)}
    if wide:
        expected.update(PENDING_START=dict(status=1,testCapacity=n(cfg,'pendingEntries'),extraNodes=n(cfg,'pendingEntries')-64),CALLBACK_START=dict(status=1,testCapacity=n(cfg,'callbackEntriesPerGroup'),extraNodes=n(cfg,'callbackEntriesPerGroup')-64),REGMAP_START=dict(status=1,externalNodes=n(cfg,'definitionEntries')),INDEXED_START=dict(status=1,testSlots=n(cfg,'indexedReferenceEntries')),CLEANUP_START=dict(status=1,testCapacity=n(cfg,'cleanupEntries')))
    for name,values in expected.items():
        selected=of(name);checks[name.lower()]=len(selected)==1 and all(n(selected[0],k)==v for k,v in values.items())
    checks['audio_setup']=bool(of('AUDIO_MODEL_STATUS')) and all(n(r,'setupFail')==0 for r in of('AUDIO_MODEL_STATUS'))
    checks['registry_capacity']=bool(of('AUDIO_MODEL_REGISTRY')) and all(n(r,'capacity')==n(cfg,'audioModelEntries') and n(r,'peak')<=n(cfg,'audioModelEntries') for r in of('AUDIO_MODEL_REGISTRY'))
    checks['parked_bounds']=all(n(r,'peakCandidates')<=capacity and n(r,'peakBucket')<=capacity for r in of('PARKED150_STATUS'))
    checks['replay_bounds']=all(n(r,'peakTraffic')<=capacity and n(r,'peakRetained')<=retained for r in of('REPLAY150_STATUS'))
    traffic_starts=of('TRAFFIC_QUEUE_START');traffic_status=of('TRAFFIC_QUEUE_STATUS')
    if traffic_starts or path.name.lower().startswith(('dsfv29s_usercap_r4_','dsfv29s_usercap_r5_','dsfv29s_usercap_r6_','dsfv29s_usercap_r7_','dsfv29s_usercap_r8_','dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):
        required={'retireIds':12,'retainedVehicles':20,'vehicleOperations':20}
        expected_capacity=max(512,n(cfg,'modelSlots')+retained)
        checks['traffic_queue_start']=len(traffic_starts)==3 and {r.get('queue') for r in traffic_starts}==set(required) and all(
            n(r,'status')==1 and n(r,'revision')==1 and n(r,'capacity')==expected_capacity and n(r,'stride')==required.get(r.get('queue')) and n(r,'beforeNativeWrite')==1 for r in traffic_starts)
        checks['traffic_queue_status']=bool(traffic_status) and {r.get('queue') for r in traffic_status}==set(required) and all(
            n(r,'capacity')==expected_capacity and n(r,'failures')==0 and n(r,'guardBad')==0 and n(r,'projectedPeak')<=expected_capacity for r in traffic_status)
    stream_starts=of('STREAM_HANDLER_START');stream_status=of('STREAM_HANDLER_STATUS')
    if stream_starts or 'streamingHandlers' in cfg or path.name.lower().startswith(('dsfv29s_usercap_r5_','dsfv29s_usercap_r6_','dsfv29s_usercap_r7_','dsfv29s_usercap_r8_','dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):
        expected_capacity=max(512,n(cfg,'modelSlots')+retained)
        revision=n(stream_starts[0],'revision',1) if stream_starts else 1
        fourth=revision>=2
        stream_values=dict(status=1,capacity=expected_capacity,schedulerQueueCapacity=expected_capacity,
            membershipCapacityPerKind=expected_capacity,schedulerBytes=(184 if fourth else 180)*expected_capacity+544,
            hooks=8 if fourth else 7,basePatches=11,initializationCalls=2,nativeCapacity=256,stride=80,coldOnly=1,activeMigration=0,stableGenerationSlabs=1)
        if fourth:stream_values.update(queueCount=4,nativeFourthQueueCapacity=512)
        checks['stream_handler_start']=len(stream_starts)==1 and n(cfg,'streamingHandlers')==expected_capacity and all(
            n(stream_starts[0],k)==v for k,v in stream_values.items())
        if path.name.lower().startswith(('dsfv29s_usercap_r6_','dsfv29s_usercap_r7_','dsfv29s_usercap_r8_','dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):checks['stream_handler_start'] &= revision==2
        checks['stream_handler_status']=bool(stream_status) and all(
            all(n(r,k)==v for k,v in stream_values.items()) and n(r,'failures')==0 and n(r,'guardBad')==0 and
            0<=n(r,'currentSampled')<=expected_capacity*n(r,'alive') and
            0<=n(r,'membershipPeak')<=expected_capacity and all(
                0<=n(r,'queue'+str(i))<=expected_capacity*n(r,'schedulerAlive') and
                0<=n(r,'queue'+str(i)+'Peak')<=expected_capacity for i in range(4 if fourth else 3)) for r in stream_status)
    capacity_starts=of('CAPACITY_QUERY_START');capacity_status=of('CAPACITY_QUERY_STATUS')
    if capacity_starts or capacity_status or path.name.lower().startswith(('dsfv29s_usercap_r7_','dsfv29s_usercap_r8_','dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):
        def capacity_valid(r):
            return n(r,'status')==1 and n(r,'modelSlots')==n(cfg,'modelSlots') and n(r,'vehicleSubtype')==0 and all(
                r.get(k)=='native' for k in ('otherPackages','otherSubtypes','managerStateGate','admissionCompare')) and n(r,'noLoadedInference')==1
        checks['capacity_query_start']=len(capacity_starts)==1 and n(capacity_starts[0],'revision')==1 and capacity_valid(capacity_starts[0])
        checks['capacity_query_status']=bool(capacity_status) and all(capacity_valid(r) for r in capacity_status)
        # An override counter is evidence of a query, never mission completion.
    key_starts=of('MANAGER_KEY_START');key_status=of('MANAGER_KEY_STATUS')
    if key_starts or key_status or 'managerKeyEntries' in cfg or path.name.lower().startswith(('dsfv29s_usercap_r8_','dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):
        expected_capacity=max(512,n(cfg,'modelSlots')+retained)
        values=dict(status=1,capacity=expected_capacity,lists=14,nativeCapacity=256,stride=12,hooks=3,preserveFX=1,nativeSentinels=1,stableGenerationNodes=1,beforeNativeSearchOrWrite=1)
        checks['manager_key_start']=len(key_starts)==1 and n(key_starts[0],'revision')==1 and n(cfg,'managerKeyEntries')==expected_capacity and all(n(key_starts[0],k)==v for k,v in values.items())
        checks['manager_key_status']=bool(key_status) and all(all(n(r,k)==v for k,v in values.items()) and n(r,'failures')==0 and n(r,'guardBad')==0 and 0<=n(r,'peakPerListProjected')<=expected_capacity and 0<=n(r,'activeSampled')<=14*expected_capacity*n(r,'alive') for r in key_status)
    trace_start=of('AUDIO_SETUP_START');trace_status=of('AUDIO_SETUP_STATUS')
    trace_names=('vehicleDisabled','bankNotReady','modelProfileMissing','defaultProfileMissing','previewSuppressed','invalidMode','modeInitializer','vehicleState','snapshotGetter','snapshotFlag')
    if trace_start or trace_status or path.name.lower().startswith(('dsfv29s_usercap_r9_','dsfv29s_usercap_r10_')):
        required=dict(revision=1,status=1,hooks=10,expectedHooks=10,outerReasons=7,subReasons=3,ringCapacity=1024,nativeOutcomes=1,capacitiesChanged=0)
        checks['audio_setup_trace_start']=len(trace_start)==1 and trace_start[0].get('bankHook')=='reused' and all(n(trace_start[0],k)==v for k,v in required.items())
        checks['audio_setup_trace_status']=bool(trace_status) and all(all(n(r,k)>=0 for k in trace_names) and n(r,'outerTotal')==sum(n(r,k) for k in trace_names[:7]) for r in trace_status)
        checks['audio_setup_trace_details']=bool(trace_status) and all(n(r,'detailDropped')==0 for r in trace_status)
    log_starts=of('LOGGING_START');log_status=of('LOGGING_STATUS');level=None
    if log_starts or path.name.lower().startswith('dsfv29s_usercap_r10_'):
        level=n(log_starts[0],'level') if log_starts else -1
        required=dict(revision=1,summaryMs=30000,checksDisabled=0,outputOnly=1,loggingChangesLimits=0,finalExitSnapshot=0)
        checks['logging_configuration']=len(log_starts)==1 and level in (0,1,2) and all(n(log_starts[0],k)==v for k,v in required.items()) and n(log_starts[0],'fullTrace')==int(level==2)
        if 'logLevel' in cfg:checks['logging_configuration'] &= n(cfg,'logLevel')==level
        checks['logging_status']=(bool(log_status) or level==0) and all(n(r,'level')==level and n(r,'fileStatus')==0 for r in log_status)
        checks['logging_full_trace']=level==2 and checks['logging_configuration']
    full_trace=level is None or (level==2 and checks.get('logging_configuration',False))
    return dict(log=str(path),configuration=cfg,checks=checks,recorded_integrity_pass=all(checks.values()),phases=phases,
        logging=dict(level=level,full_trace_available=full_trace,scope='full' if full_trace else 'reduced',final=log_status[-1] if log_status else None),
        audio_setup_trace_final=trace_status[-1] if trace_status else None,
        audio_setup_trace_details=of('AUDIO_SETUP_DETAIL'),
        manager_key_final=key_status[-1] if key_status else None,
        capacity_query_final=capacity_status[-1] if capacity_status else None,
        events=of('GENERIC_EVENT'),reader_exercised=any(n(r,'constructs',0)>0 for r in of('REPLAY_READ_STATUS')),
        audio_final=of('AUDIO_MODEL_STATUS')[-1] if of('AUDIO_MODEL_STATUS') else None,
        stream_final=stream_status[-1] if stream_status else None,
        limitations=['READY checks are sequential and do not certify drawable residency.',
            'No reader exercise does not certify replay playback.', 'No fatal record is not proof of normal exit.']+
            (['Reduced logging omits full diagnostic evidence; restart with Diagnostics LogLevel=2 for full-trace validation.'] if not full_trace else []))
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('log',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();r=analyze(a.log)
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(dict(checks=r['checks'],recorded_integrity_pass=r['recorded_integrity_pass']),indent=2))

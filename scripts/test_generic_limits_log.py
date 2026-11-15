"""Independent synthetic generic-log acceptance/mutation cases; no native execution."""
import json,tempfile
from pathlib import Path
from analyze_generic_limits_log import analyze
def run():
    good='''DSFV29s_GENERIC_START valid=1 reason=configured compiledCarLists=0 file=DriverSF_Unlimiter.ini rosterSource=AssetWeights requestCapacity=100 audioModelEntries=256 globalAudioEntries=4096 defaultAudioMiB=128 replayReaderMiB=4 retainedReferences=512 requestStorage=100 parkedStride=404 parkedFrame=10732 notifyFrame=408 replayPayloadCapacity=4508 modelSlots=160 auditedRequestCeiling=150
DSFV29s_GENERIC_STATUS failures=0
DSFV29s_BASELINE_PHASE kind=accepted seq=1 count=3
DSFV29s_BASELINE_PHASE kind=sample seq=1 count=3 ready=3 checkpoints=1
DSFV29s_BASELINE_ROSTER seq=1 kind=effective ids=42,65000,300
DSFV29s_BASELINE_STATUS eventDrops=0 memorySnapshotFailures=0
DSFV29s_NOTIFY150_START status=1 capacity=100 frameBytes=408
DSFV29s_NOTIFY150_STATUS invalid=0
DSFV29s_PARKED150_START status=1 capacity=100 perCategory=100 frameBytes=10732
DSFV29s_PARKED150_STATUS invalid=0 guardBad=0 concurrentSkips=0 peakCandidates=100 peakBucket=40
DSFV29s_REPLAY150_START status=1 trafficCapacity=100 retainedCapacity=512 payloadBytes=4508
DSFV29s_REPLAY150_STATUS invalid=0 peakTraffic=100 peakRetained=400
DSFV29s_REGMAP_STATUS faults=0 allocationFailed=0
DSFV29s_REPLAY_READ_START status=1 readerBytes=4194304 blockBytes=32768
DSFV29s_REPLAY_READ_STATUS constructs=1 faults=0 fallbacks=0 allocationFailed=0
DSFV29s_CALLBACK_STATUS invalid=0 full=0 guardBad=0 concurrent=0
DSFV29s_ARENA_STATUS invalid=0 guardBad=0 allocationFailed=0
DSFV29s_AUDIO_MODEL_START status=1 testCapacity=256
DSFV29s_AUDIO_MODEL_STATUS invalid=0 instanceOverflow=0 keyOverflow=0 setupFail=0
DSFV29s_AUDIO_MODEL_REGISTRY full=0 guardBad=0 capacity=256 peak=200
DSFV29s_AUDIO_SLOTS_START status=1 testMax=4096 defaultBytes=134217728
DSFV29s_AUDIO_LIMIT_START status=1 testRequestedBytes=134217728
DSFV29s_PENDING_STATUS invalid=0 full=0 allocationFailed=0 guardBad=0
DSFV29s_CLEANUP_STATUS invalid=0 full=0 guardBad=0 drainMismatch=0
DSFV29s_REPLAY_CLIENT_STATUS dropped=0 threadDrops=0 unknown=0 readFailures=0 clockFailures=0
'''
    mutations=[('valid=1','valid=0','configuration'),('requestCapacity=100','requestCapacity=700','configuration'),
        ('parkedFrame=10732','parkedFrame=15732','layout'),('testCapacity=256','testCapacity=192','audio_model_start'),
        ('peak=200','peak=257','registry_capacity'),('frameBytes=408','frameBytes=608','notify150_start'),
        ('capacity=100 perCategory=100','capacity=150 perCategory=150','parked150_start'),
        ('readerBytes=4194304','readerBytes=1048576','replay_read_start'),
        ('fallbacks=0','fallbacks=1','replay_read_status'),('setupFail=0','setupFail=1','audio_setup'),
        ('ids=42,65000,300','ids=42,65000','request_rosters'),('failures=0','failures=1','generic_status')]
    with tempfile.TemporaryDirectory(prefix='dsf_generic_log_') as directory:
        p=Path(directory)/'fixture.log';p.write_text(good);r=analyze(p);assert r['recorded_integrity_pass'],r
        assert r['reader_exercised'] and r['phases'][0]['all_ready_observed']
        for before,after,key in mutations:
            p.write_text(good.replace(before,after));r=analyze(p);assert not r['checks'][key],key
        p.write_text(good.rstrip('\n'));assert not analyze(p)['checks']['complete_records']
    return dict(result='passed',valid_configuration='100/256/4096/128MiB/4MiB/512',negative_cases=len(mutations)+1)
if __name__=='__main__':
    r=run();out=Path(__file__).resolve().parents[1]/'analysis/generic_limits_r1_2026-11-14/parser_tests.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))

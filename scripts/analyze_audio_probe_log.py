"""Analyze one supplied audio R1/default16/default64/static64 log."""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path


def analyze(path):
    data=path.read_bytes()
    rows=[]
    for line_number,line in enumerate(data.decode('utf-8-sig').splitlines(),1):
        parts=line.split()
        if not parts:continue
        row={'type':parts[0],'line':line_number}
        row.update(piece.split('=',1) for piece in parts[1:] if '=' in piece)
        rows.append(row)
    def of(kind):return [r for r in rows if r['type']=='DSFV29s_'+kind]
    def n(row,key):return int(row[key],10)
    def maximum(records,key):return max((n(r,key) for r in records if key in r),default=None)
    audio,capacity,pools,states=of('AUDIO_STATUS'),of('CAP_STATUS'),of('AUDIO_POOL'),of('AUDIO_STATE')
    details=of('AUDIO_DETAIL')
    groups={kind:[r for r in details if r['kind']==kind] for kind in
        ('bank','emitterRefused','mallocFailed','malignFailed','postZero','postEarlyZero')}
    assert audio and capacity,'No monitor coverage'
    indexed_start,audio_start,cap_start=of('INDEXED_START'),of('AUDIO_START'),of('CAP_START')
    assert len(indexed_start)==len(audio_start)==len(cap_start)==1,'Expected exactly one process startup'
    indexed_start,audio_start,cap_start=indexed_start[0],audio_start[0],cap_start[0]
    limits=of('AUDIO_LIMIT_START');pressure=of('AUDIO_PRESSURE')
    static_limits=of('AUDIO_STATIC_START');static_live=of('AUDIO_STATIC_POOL')
    default16m=bool(limits)
    default64=bool(limits) and limits[0].get('testRequestedBytes')=='67108864'
    default_bytes='67108864' if default64 else '16777216'
    checks={
        'indexed_start':all(indexed_start.get(k)==v for k,v in {
            'status':'1','startupCheckRevision':'2','stockPreflight':'1','ownedHookVerified':'1',
            'patches':'13','pendingRedirectPatches':'31','testSlots':'262144'}.items()),
        'audio_start':all(audio_start.get(k)==v for k,v in {
            'revision':'1','status':'1','hooks':'6','expectedHooks':'6','audioCapacityChanges':'1' if default16m else '0'}.items()),
        'capacity_start':all(cap_start.get(k)==v for k,v in {'hooks':'3','request':'150','slots':'160','ED_EF':'1','capacityChanges':'1'}.items()),
        'later_baseline':any(all(r.get(k)==v for k,v in {'startupStatus':'2','graphicsSlots':'160','requestLast':'150','indexedStatus':'2','indexedSlots':'262144'}.items()) for r in capacity),
        'all_file_status_zero':all(r['fileStatus']=='0' for r in capacity),
        'all_heads_valid':all(r['headValid']=='1' for r in capacity),
    }
    counters=('bankCallbacks','bankResult1','emitterAttempts','emitterPeakBefore','emitterRefused',
        'mallocFailed','malignFailed','postZero','postEarlyZero','postNormalReturns')
    checks['audio_counters_monotonic']=all(n(b,k)>=n(a,k) for a,b in zip(audio,audio[1:]) for k in counters)
    if default16m:
        checks['default64_start' if default64 else 'default16m_start']=len(limits)==1 and all(limits[0].get(k)==v for k,v in {
            'revision':'1','status':'1','patches':'1','stockRequestedBytes':'5872025',
            'testRequestedBytes':default_bytes,'thresholdBits':'3F666666','thresholdChanged':'0'}.items())
        checks['default64_live' if default64 else 'default16m_live']=bool(pressure) and all(all(r.get(k)==v for k,v in {
            'patchStatus':'1','configuredBytes':default_bytes,'thresholdBits':'3F666666'}.items()) for r in pressure)
        checks['pressure_consistency']=all((n(r,'usedBytes')*16777216>n(r,'usableBytes')*15099494)==bool(n(r,'aboveThresholdSampled')) for r in pressure)
    if static_limits:
        checks['static64_start']=len(static_limits)==1 and all(static_limits[0].get(k)==v for k,v in {
            'revision':'1','status':'1','patches':'1','stockRequestedBytes':'4372480',
            'testRequestedBytes':'67108864','defaultRequestedBytes':'16777216','thresholdChanged':'0'}.items())
        checks['static64_live']=default16m and not default64 and bool(static_live) and all(
            all(r.get(k)==v for k,v in {'patchStatus':'1','requestedBytes':'67108864',
                'blocks':'65536','blockBytes':'1024','playbackGateAppliesToThisPool':'0'}.items())
            and 60*1024*1024<n(r,'usableBytes')<=64*1024*1024 for r in static_live)
    final=audio[-1]
    bank=groups['bank'];zero=groups['postZero']
    pool_groups=defaultdict(list)
    for r in pools:pool_groups[(n(r,'id'),n(r,'generation'),r['table'],r['backing'])].append(r)
    pool_summary=[]
    for (pool_id,generation,table,backing),records in sorted(pool_groups.items()):
        last=records[-1];usable=n(last,'usableBytes');peak=maximum(records,'sampledPeakMallocUsed')
        pool_summary.append(dict(id=pool_id,generation=generation,table=table,backing=backing,
            attrs=last['attrs'],fixed_blocks=last['fixedBlocks']=='1',usable_bytes=usable,
            final_malloc_bytes=n(last,'mallocUsedBytes'),sampled_peak_malloc_bytes=peak,
            sampled_peak_percent=round(peak/usable*100,3) if usable else None,
            sampled_headroom_bytes=usable-peak,blocks=n(last,'blocks'),block_bytes=n(last,'blockBytes'),
            sampled_min_free_blocks=min(n(r,'sampledMinFree') for r in records),
            first_line=records[0]['line'],last_line=last['line']))
    per_event=defaultdict(list)
    for r in zero:per_event[(r['event'],r['caller'])].append(r)
    failure_summary=[dict(event=e,caller=c,retained_count=len(records),first_tick=n(records[0],'tick'),
        last_tick=n(records[-1],'tick'),first_line=records[0]['line'])
        for (e,c),records in sorted(per_event.items(),key=lambda x:-len(x[1]))]
    timeline=[];previous=None
    for end in range(10000,n(final,'tick')+10000,10000):
        selected=[r for r in audio if end-10000<n(r,'tick')<=end]
        if not selected:continue
        last=selected[-1]
        timeline.append(dict(tick=n(last,'tick'),bank_callbacks=n(last,'bankCallbacks'),
            posts=n(last,'postNormalReturns'),zeros=n(last,'postZero'),
            delta_posts=n(last,'postNormalReturns')-(n(previous,'postNormalReturns') if previous else 0),
            delta_zeros=n(last,'postZero')-(n(previous,'postZero') if previous else 0)))
        previous=last
    q=[r for r in of('CAP_QUEUE') if r['queue']=='audioBanks']
    result=dict(log=str(path),sha256=hashlib.sha256(data).hexdigest(),bytes=len(data),lines=len(rows),
        complete_final_newline=data.endswith(b'\n'),startup_checks=checks,valid_probe=all(checks.values()),
        last_tick=n(final,'tick'),row_counts=dict(Counter(r['type'] for r in rows)),
        audio_final={k:n(final,k) for k in counters},
        admitted_posts=n(final,'postNormalReturns')-n(final,'postZero'),
        normal_post_zero_percent=round(n(final,'postZero')/n(final,'postNormalReturns')*100,4),
        max_sampled_emitter_active=maximum(audio,'emitterActive'),
        max_sampled_cleanup_active=maximum(audio,'cleanupActive'),pool_count_peak=maximum(audio,'poolCountPeak'),
        distinct_readable_owners=len({r['owner'] for r in audio if r.get('ownerReadable')=='1'}),
        initialized_flags=sorted({(r.get('managerByte30'),r.get('soundInit'),r.get('musicInit')) for r in audio if r.get('ownerReadable')=='1'}),
        raw_volume_state_variants=dict(Counter(r['raw50to90'] for r in states)),
        named_pool_id_variants={k:sorted({r[k] for r in states}) for k in ('filePoolId','preparePoolId','staticBankPoolId','defaultPoolId','platformPoolId')},
        retained_details={k:len(v) for k,v in groups.items()},
        bank_result_histogram=dict(Counter(r['result'] for r in bank)),bank_pool_histogram=dict(Counter(r['pool'] for r in bank)),
        bank_first_tick=min((n(r,'tick') for r in bank),default=None),bank_last_tick=max((n(r,'tick') for r in bank),default=None),
        post_zero_first_tick=n(zero[0],'tick') if zero else None,
        post_zero_last_retained_tick=n(zero[-1],'tick') if zero else None,
        post_zero_detail_truncated=n(final,'postZero')>len(zero),post_zero_by_event_and_caller=failure_summary,
        first_150_request_sample=next((n(r,'tick') for r in capacity if r['requestLast']=='150'),None),
        pools=pool_summary,timeline=timeline,
        visual_baseline=dict(initialized_samples=n(capacity[-1],'samples'),empty_samples=n(capacity[-1],'emptySamples'),
            pending_peak=maximum(capacity,'pendingPeakSampled'),pending_final=n(capacity[-1],'pending'),
            audio_queue_projected_peak=maximum(q,'peakAfterInsertProjected'),audio_queue_boundary_attempts=maximum(q,'boundaryAttempts'),
            audio_queue_final_inserts=n(q[-1],'inserts')),
        limitations=['Only the instrumented bank callback/API paths are counted.',
            'Numeric event return IDs do not prove audible voices; successful event IDs were not retained.',
            'Only first 128 normal zero-result details retained; counters cover later failures.',
            'Pool peaks are sampled; occupancy does not measure fragmentation or prove no GetBlock/CreatePool failure.',
            'Loading completion and audible-loss timestamps were not explicitly marked.'])
    if default16m:
        result['variant']='default64m' if default64 else 'default16m'
        result['default_pressure']=dict(rows=len(pressure),last_tick=n(pressure[-1],'tick'),
            samples=n(pressure[-1],'samples'),above_samples=n(pressure[-1],'pressureSamples'),
            maximum_logged_used_bytes=maximum(pressure,'usedBytes'),usable_bytes=n(pressure[-1],'usableBytes'),
            first_line=pressure[0]['line'],last_line=pressure[-1]['line'])
        result['pool_thresholds']=[dict(id=p['id'],sampled_peak_bytes=p['sampled_peak_malloc_bytes'],
            usable_bytes=p['usable_bytes'],sampled_peak_percent=p['sampled_peak_percent'],
            sampled_peak_above_90=p['sampled_peak_malloc_bytes']*16777216>p['usable_bytes']*15099494)
            for p in pool_summary if p['id'] in (5,6)]
        if n(final,'postZero')<=len(zero):
            result['limitations']=[s for s in result['limitations'] if not s.startswith('Only first 128')]
    if static_limits:
        result['variant']='static64m_default16m'
        result['static_pool']=dict(rows=len(static_live),start=static_limits[0],
            last=static_live[-1] if static_live else None,maximum_logged_used_bytes=maximum(static_live,'usedBytes'))
    return result


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('log',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    result=analyze(args.log)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result,indent=2))


if __name__=='__main__':main()

"""Summarize supplied run24888; telemetry is not a frame-time trace."""
import collections,json,struct,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'scripts'))
from compare_audio_slots_logs import rows
r=rows(ROOT/'dist/MAIN_150/DSFV29s_regmap_reuse_24888.log')
replay=[x for x in r if x['type']=='DSFV29s_REPLAY150_STATUS']
events=[x for x in r if x['type']=='DSFV29s_REPLAY150_EVENT']
restores=[x for x in events if x['kind']=='restore']
def floating(bits):return struct.unpack('<f',struct.pack('<I',int(bits,16)))[0]
segments=[];last_key=None
for s in replay:
    key=(s.get('replay'),s.get('mode'),s.get('loading'),s.get('lifecycle'),s.get('snapshotState'))
    if key!=last_key:
        segments.append(dict(replay=key[0],mode=key[1],loading=key[2],lifecycle=key[3],snapshot=key[4],
            first_tick=int(s['tick']),last_tick=int(s['tick']),samples=1,
            first_clock=floating(s['clockBits']) if 'clockBits' in s else None,
            last_clock=floating(s['clockBits']) if 'clockBits' in s else None))
        last_key=key
    else:
        seg=segments[-1];seg['last_tick']=int(s['tick']);seg['samples']+=1
        if 'clockBits' in s:seg['last_clock']=floating(s['clockBits'])
windows=[]
for first in range(min(int(s['tick']) for s in restores),max(int(s['tick']) for s in restores)+1,5000):
    selected=[x for x in restores if first<=int(x['tick'])<first+5000]
    if selected:windows.append(dict(first_tick=first,last_tick=first+4999,restore_count=len(selected)))
restored=[dict(tick=int(x['tick']),owner=x['owner'],hash=x['rosterHash'],count=int(x['count'])) for x in restores]
timeline=[]
for s in replay:
    if s.get('mode')=='2':
        timeline.append(dict(tick=int(s['tick']),replay=s['replay'],clock=floating(s['clockBits']),loading=int(s['loading']),
            lifecycle=int(s['lifecycle']),snapshot=int(s['snapshotState']),imports=int(s['imports']),restores=int(s['restores']),
            pendingTraffic=int(s['pendingTraffic']),pendingRetained=int(s['pendingRetained']),spoolState=int(s['spoolState'])))
result=dict(event_counts=dict(collections.Counter(x['kind'] for x in events)),
    snapshot_rosters=sorted({(x['count'],x['retained'],x['rosterHash'],x['id50'],x['has181']) for x in events}),
    segments=segments,restores=restored,restore_windows_5sec=windows,replay_samples=timeline,
    limitations=['One-second status sampling misses subsecond waits.','Restores are internal events, not user replay entries.',
        'Clock pauses can be user pause/seek, loading, playback rate or scheduler behavior; no frame-time/rate trace exists.'])
(OUT/'replay_timing.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(dict(event_counts=result['event_counts'],snapshot_rosters=result['snapshot_rosters'],
    replay_segments=[s for s in segments if s['mode']=='2'],restore_windows_5sec=windows),indent=2))

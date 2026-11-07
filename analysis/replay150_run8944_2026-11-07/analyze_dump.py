import mmap,struct,bisect,json
from pathlib import Path
dump_path=Path('dist/MAIN_150/Driver.DMP')
dump_file=dump_path.open('rb'); data=mmap.mmap(dump_file.fileno(),0,access=mmap.ACCESS_READ)
def unpack(fmt,off):return struct.unpack_from('<'+fmt,data,off)
signature,version,stream_count,directory,checksum,dump_timestamp,dump_flags=unpack('6IQ',0)
assert signature==0x504D444D
streams={kind:(size,rva) for kind,size,rva in (unpack('3I',directory+12*i) for i in range(stream_count))}
def mstring(rva):
 n=unpack('I',rva)[0];return data[rva+4:rva+4+n].decode('utf-16le',errors='replace')
modules=[]
if 4 in streams:
 off=streams[4][1];n=unpack('I',off)[0]
 for i in range(n):
  base,size,check,stamp,name=unpack('Q4I',off+4+i*108)
  modules.append(dict(base=base,end=base+size,size=size,timestamp=stamp,name=mstring(name)))
def module_name(a):
 m=next((m for m in modules if m['base']<=a<m['end']),None)
 return None if m is None else Path(m['name']).name+'+'+hex(a-m['base'])
ranges=[]
if 9 in streams:
 off=streams[9][1];n,rva=unpack('2Q',off)
 for i in range(n):
  start,size=unpack('2Q',off+16+16*i);ranges.append((start,start+size,rva));rva+=size
if 5 in streams:
 off=streams[5][1];n=unpack('I',off)[0]
 for i in range(n):
  start,size,rva=unpack('Q2I',off+4+16*i)
  if rva and size:ranges.append((start,start+size,rva))
ranges.sort();range_starts=[r[0] for r in ranges]
def read(a,n):
 idx=bisect.bisect_right(range_starts,a)-1
 if idx>=0:
  start,end,rva=ranges[idx]
  if a+n<=end:return data[rva+a-start:rva+a-start+n]
 return None
def word(a):
 b=read(a,4);return struct.unpack('<I',b)[0] if b and len(b)==4 else None
def words(a,n):
 b=read(a,n*4);return list(struct.unpack('<'+'I'*n,b)) if b and len(b)==n*4 else None
threads=[]
if 3 in streams:
 off=streams[3][1];n=unpack('I',off)[0]
 for i in range(n):
  tid,suspend,pc,priority,teb,stack,size,rva,ctxsize,ctxrva=unpack('4I2Q4I',off+4+i*48)
  t=dict(id=tid,suspend=suspend,teb=teb,stack=stack,stack_size=size,context_size=ctxsize,context_rva=ctxrva)
  if ctxsize>=204:
   t['context_flags']=unpack('I',ctxrva)[0]
   if t['context_flags']&0x10000:
    names=['edi','esi','ebx','edx','ecx','eax','ebp','eip','cs','eflags','esp','ss'];t['regs']=dict(zip(names,unpack('12I',ctxrva+156)))
  threads.append(t)
if 17 in streams:
 off=streams[17][1];head,entry,n=unpack('3I',off)
 for i in range(n):
  fields=unpack('4I6Q',off+head+i*entry);tid=fields[0]
  t=next((t for t in threads if t['id']==tid),None)
  if t:t.update(kernel_seconds=fields[6]/1e7,user_seconds=fields[7]/1e7,start=fields[8])
pid=None
if 15 in streams:
 off=streams[15][1];size,flags=unpack('2I',off)
 if flags&1:pid=unpack('I',off+8)[0]

for t in threads:
 if t.get('regs'):continue
 raw=read(t['teb']+0x1488,8)
 if not raw:continue
 reserved=struct.unpack('<Q',raw)[0];ctx=read(reserved+4,204)
 if ctx and struct.unpack_from('<I',ctx,0)[0]&0x10000:
  names=['edi','esi','ebx','edx','ecx','eax','ebp','eip','cs','eflags','esp','ss']
  regs=dict(zip(names,struct.unpack_from('<12I',ctx,156)))
  if regs['cs']==0x23 and regs['ss']==0x2b:
   t['regs']=regs;t['wow64_context']=reserved+4
def frame_chain(regs,limit=18):
 frames=[];fp=regs['ebp']
 for _ in range(limit):
  pair=words(fp,2)
  if not pair:break
  nxt,ret=pair;frames.append(dict(fp=hex(fp),next=hex(nxt),ret=hex(ret),module=module_name(ret)))
  if nxt<=fp or nxt-fp>0x100000:break
  fp=nxt
 return frames

# Focused offline analysis of the explicitly supplied PID8944 Task Manager dump.
import sys,re,hashlib
sys.path.insert(0,'scripts')
from static_streaming_audit import PE
out=Path('analysis/replay150_run8944_2026-11-07')
assert unpack('I',streams[15][1]+8)[0]==8944
native=PE()
assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
code_spans=[(0x738F20,0x214),(0x7379D0,0x80),(0x738D20,0x80),(0x73A090,0x650),
    (0x53A9D0,0x240),(0x755000,0x140),(0x755DF0,0x60),(0x85D820,0x190),
    (0x53A7E0,0x12A),(0x79DAE0,0x30)]
code_checks={f'{lo:08X}':read(lo,n)==native.read(lo,n) for lo,n in code_spans}
assert all(code_checks.values()),code_checks
root=word(0xFE7228);replay=word(root+0x22C);spool=word(root+0x214)
sim=word(root+0xCC);ctrl=sim+0x168;scheduler=word(root+0x2C)
assert word(ctrl)==0xE2CA70 and word(word(ctrl)+4)==0x53AA20
def floating(a):return struct.unpack('<f',read(a,4))[0]
clocks=[];node=word(scheduler);seen=set()
while node!=scheduler:
    assert node and node not in seen and len(seen)<64
    seen.add(node);obj=node-4
    clocks.append(dict(object=f'{obj:08X}',vtable=f'{word(obj):08X}',
        paused=bool(read(obj+0x20,1)[0]),is_simulation=obj==ctrl))
    node=word(node)
assert any(c['is_simulation'] for c in clocks)
subscribers=[];node=word(replay+0x4E580);seen=set()
while node:
    assert node not in seen and len(seen)<32
    seen.add(node);obj=word(node+0xC);fn=word(word(obj)+0x20)
    detail={}
    if fn==0x8203B0:busy=False
    elif fn==0x7523B0:busy=word(obj+4)==0
    elif fn==0x7549D0:
        rec=word(obj+4);busy=not rec or not word(rec+0xC)
    elif fn==0x755DF0:
        rec=word(obj+4);nxt=word(rec+0xC) if rec else 0
        exhausted=not nxt or (not word(nxt+0xC) and floating(replay+0x4E9A0)>floating(nxt+4))
        busy=exhausted or bool(read(obj+0x34,1)[0])
        detail=dict(record=f'{rec:08X}',next=f'{nxt:08X}',flag34=read(obj+0x34,1)[0],exhausted=exhausted)
    else:raise AssertionError(f'unhandled predicate {fn:08X}')
    subscribers.append(dict(module=word(node+8),object=f'{obj:08X}',predicate=f'{fn:08X}',busy=bool(busy),detail=detail))
    node=word(node+4)
# Evaluate native 0074B760, 007379D0 and 00738D20 from captured state.
native_spool_idle=word(spool+0xDAC)!=1 and word(spool+0xC8)==0
snapshot_busy=word(replay+0x438EC)!=0 or not native_spool_idle
subscriber_busy=word(replay+0x4E588)==2 and any(s['busy'] for s in subscribers)
replay_busy=subscriber_busy or snapshot_busy
world=word(root+0x204)
assert word(world+0x6FD90)==2 # native0059C690 returns +74DA8 for state2
world_result=word(world+0x74DA8)
recording=word(replay+0x4E58C);transition=word(replay+0x4E9F4)
loading_busy=(not recording or (replay_busy and
    (not read(recording+0x858C,1)[0] or word(recording+0x8584)!=0)) or
    world_result==0 or transition not in (0,3) or (transition==3 and word(replay+0x4E380)!=0))
assert not replay_busy and not loading_busy
asi=next(m for m in modules if m['name'].endswith('REPLAY150_R1.asi'))
build=Path('build/v29s_replay150_r1');name='DriverSF_V29s_160Capacity_150Request_REPLAY150_R1'
symbols={}
for line in (build/(name+'.map')).read_text().splitlines():
    match=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
    if match:symbols[match[1]]=int(match[2],16)
def symbol(prefix):return asi['base']+next(v for k,v in symbols.items() if k.startswith(prefix))-0x10000000
slot=word(symbol('?g_replaySlots@'));assert word(slot+4)==replay+0x438E4
assert word(slot+8)==word(slot+0xA74)==0x29150ACD and word(slot+0xC)==1
count=word(slot+0x10);ids=words(slot+0x14,count);h=2166136261
for model in ids:h=((h^model)*16777619)&0xffffffff
assert count==150 and len(set(ids))==150 and 0 not in ids and ids[50]==181 and h==0x5CBDFFEA
locks={p:word(symbol('?'+p+'@')) for p in ['g_replayLock','g_replayActive','g_replayInvalid','g_parkLock','g_reqLock','g_baselineLock']}
assert not any(locks.values())
current=word(replay+0x8554);end_index=word(replay+0x4EA0C)
# This group's identity is recovered from the saved EDI/ESI/EBX triple on
# main-thread stack19FB30..38, not inferred from arbitrary stack pointers.
main=next(t for t in threads if t['id']==4936)
group=word(main['regs']['esp']+0x14)
assert word(main['regs']['esp']+0x10)==group+0x28 and word(main['regs']['esp']+0x18)==group+0x2C
with dump_path.open('rb') as file:dump_hash=hashlib.file_digest(file,'sha256').hexdigest()
result=dict(dump=str(dump_path),bytes=len(data),sha256=dump_hash,pid=8944,timestamp=dump_timestamp,
    exception_stream=6 in streams,threads=len(threads),memory_ranges=len(ranges),
    root=f'{root:08X}',replay=f'{replay:08X}',simulation=f'{sim:08X}',spooler=f'{spool:08X}',
    traffic_asi=asi,other_game_plugin_present='DriverNGHook.asi',
    cache=dict(address=f'{slot:08X}',guards_valid=True,count=count,unique=len(set(ids)),
        rosterHash=f'{h:08X}',id50=ids[50],has181=True,hasZero=False,retained=word(slot+0x26C)),
    locks=locks,code_matches_supported_exe=code_checks,
    replay_state=dict(mode=word(replay+0x4E588),loading=read(replay+0x4E9EC,1)[0],
        transition=transition,lifecycle=word(replay+0x4EA14),snapshot_phase=word(replay+0x438EC),
        clock=floating(replay+0x4E9A0),cursor=f'{current:08X}',cursor_time=floating(current+4),
        record_end_time=floating(recording+0x28+end_index*8),requested_playback_rate=floating(replay+0x4E99C)),
    clock_controller=dict(address=f'{ctrl:08X}',scheduler=f'{scheduler:08X}',
        paused=bool(read(ctrl+0x20,1)[0]),rate=floating(ctrl+0x28),scale=floating(ctrl+0x2C),
        global_requested_rate=floating(0xF2D3F0),step_callback='0053AA20',all_clocks=clocks),
    gates=dict(snapshot_busy=snapshot_busy,subscriber_busy=subscriber_busy,replay_busy=replay_busy,
        loading_busy=bool(loading_busy),world_result=world_result,
        traffic_pending=word(spool+0xC8),retained_pending=word(spool+0x9A8),spool_state=word(spool+0xDAC)),
    subscribers=subscribers,main_thread=dict(id=4936,frames=frame_chain(main['regs']),
        task_group=f'{group:08X}',submitted=word(group+0x28),completed=word(group+0x2C)),
    finding='Paused simulation clock prevents dispatch of the replay-step/loading-completion callback despite captured readiness gates being clear.',
    limitations=['Dump captures one instant; the log supplies the persistent loading/clock plateau.',
        'Exact caller/order that first set zero playback rate is not recorded.',
        'A safe correction must preserve user pause and native readiness/lifecycle checks; no unconditional unpause is justified.',
        'Older recording origin is not established from this dump; correct150-ID cache is directly verified.',
        'Presence of DriverNGHook is not evidence it caused the stall.'])
(out/'dump_analysis.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:result[k] for k in ['pid','sha256','cache','replay_state','gates','clock_controller','finding']},indent=2))


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


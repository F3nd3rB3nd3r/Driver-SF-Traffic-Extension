"""Memory-mapped, read-only minidump analysis. No debugger or process attachment."""
import argparse
from bisect import bisect_right
import hashlib
import json
import mmap
from pathlib import Path
import struct


class Dump:
    def __init__(self,path):
        self.path=Path(path);self.file=self.path.open('rb');self.data=mmap.mmap(self.file.fileno(),0,access=mmap.ACCESS_READ)
        signature,version,n,directory,checksum,self.timestamp,self.flags=self.unpack('6IQ',0)
        assert signature==0x504D444D
        self.streams={k:(s,r) for k,s,r in (self.unpack('3I',directory+i*12) for i in range(n))}
        self.modules=[];self.ranges=[]
        if 4 in self.streams:
            _,off=self.streams[4];n,=self.unpack('I',off)
            for i in range(n):
                base,size,check,stamp,name=self.unpack('Q4I',off+4+i*108)
                self.modules.append(dict(base=base,end=base+size,size=size,timestamp=stamp,name=self.string(name)))
        if 9 in self.streams:
            _,off=self.streams[9];n,rva=self.unpack('2Q',off)
            for i in range(n):
                start,size=self.unpack('2Q',off+16+i*16);self.ranges.append((start,size,rva));rva+=size
        elif 5 in self.streams:
            _,off=self.streams[5];n,=self.unpack('I',off)
            for i in range(n):self.ranges.append(self.unpack('Q2I',off+4+i*16))
        self.ranges.sort();self.starts=[r[0] for r in self.ranges]
    def close(self):self.data.close();self.file.close()
    def unpack(self,fmt,off):return struct.unpack_from('<'+fmt,self.data,off)
    def string(self,rva):
        n,=self.unpack('I',rva);return self.data[rva+4:rva+4+n].decode('utf-16le',errors='replace')
    def read(self,at,size):
        out=bytearray()
        while size:
            i=bisect_right(self.starts,at)-1
            if i<0:return None
            start,length,rva=self.ranges[i]
            if not start<=at<start+length:return None
            part=min(size,start+length-at);off=rva+at-start
            out.extend(self.data[off:off+part]);at+=part;size-=part
        return bytes(out)
    def u32(self,at):
        b=self.read(at,4);return None if b is None else struct.unpack('<I',b)[0]
    def u64(self,at):
        b=self.read(at,8);return None if b is None else struct.unpack('<Q',b)[0]
    def module(self,at):
        m=next((m for m in self.modules if m['base']<=at<m['end']),None)
        return None if m is None else dict(name=m['name'],base=hex(m['base']),rva=hex(at-m['base']))
    def x86_context(self,off,size):
        if size<204:return None
        flags,=self.unpack('I',off)
        if flags&0xFFFF0000!=0x10000:return None
        names=('edi','esi','ebx','edx','ecx','eax','ebp','eip','cs','eflags','esp','ss')
        return dict(zip(names,self.unpack('12I',off+156)))
    def threads(self):
        info={}
        if 17 in self.streams:
            _,off=self.streams[17];header,stride,n=self.unpack('3I',off)
            for i in range(n):
                tid,dumpflags,error,status,create,exit,kernel,user,start,affinity=self.unpack('4I6Q',off+header+i*stride)
                info[tid]=dict(create_time=create,kernel_seconds=kernel/1e7,user_seconds=user/1e7,
                    start_address=hex(start),start_module=self.module(start),dump_flags=dumpflags)
        results=[]
        if 3 not in self.streams:return results
        _,off=self.streams[3];n,=self.unpack('I',off)
        for i in range(n):
            tid,suspend,priority_class,priority,teb,stack,size,rva,ctxsize,ctxrva=self.unpack('4I2Q4I',off+4+i*48)
            regs=self.x86_context(ctxrva,ctxsize)
            source='native_x86_context' if regs else None
            native64=None;saved_wow=None;teb32=None
            if not regs and ctxsize>=256 and self.unpack('I',ctxrva+48)[0]&0xFFFF0000==0x100000:
                names=('rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi','r8','r9','r10','r11','r12','r13','r14','r15','rip')
                native64=dict(zip(names,self.unpack('17Q',ctxrva+120)))
                cs,=self.unpack('H',ctxrva+56)
                native64['cs']=cs
                # Validate the WOW64 CPU block present in THIS dump, rather
                # than silently treating a1232-byte AMD64 CONTEXT as x86.
                cpu=self.u64(teb+0x1488)
                block=self.read(cpu,208) if cpu else None
                if block and struct.unpack_from('<H',block,2)[0]==0x14C and struct.unpack_from('<I',block,4)[0]&0xFFFF0000==0x10000:
                    saved_wow=dict(zip(('edi','esi','ebx','edx','ecx','eax','ebp','eip','cs','eflags','esp','ss'),struct.unpack_from('<12I',block,160)))
                if cs==0x23 and native64['rip']<0x100000000:
                    regs={r:native64['r'+r[1:]]&0xFFFFFFFF for r in ('edi','esi','ebx','edx','ecx','eax','ebp','eip','esp')}
                    regs.update(cs=cs,eflags=self.unpack('I',ctxrva+68)[0],ss=self.unpack('H',ctxrva+66)[0]);source='live_amd64_context_in_cs23'
                elif saved_wow:
                    regs=saved_wow;source='validated_saved_wow64_cpu_context'
                candidate=self.u64(teb)
                if candidate and candidate<0x100000000 and self.u32(candidate+0x18)==candidate and self.u32(candidate+0x24)==tid:
                    teb32=candidate;stack=self.u32(candidate+8);size=self.u32(candidate+4)-stack
            result=dict(id=tid,teb=hex(teb),stack_start=hex(stack),stack_size=size,
                context_size=ctxsize,context_flags=hex(self.unpack('I',ctxrva)[0]),
                context_prefix=self.data[ctxrva:ctxrva+64].hex(),info=info.get(tid),registers=None)
            result.update(context_source=source,teb32=None if teb32 is None else hex(teb32),
                native64=None if native64 is None else {k:hex(v) for k,v in native64.items()},
                saved_wow64=None if saved_wow is None else {k:hex(v) for k,v in saved_wow.items()})
            if regs:
                result['registers']={k:hex(v) for k,v in regs.items()};result['instruction_module']=self.module(regs['eip'])
                code=self.read(regs['eip']-16,64);result['instruction_bytes']=None if code is None else code.hex(' ')
                words=[];candidates=[]
                end=min(stack+size,regs['esp']+4096)
                for address in range(regs['esp'],end,4):
                    value=self.u32(address)
                    if value is None:break
                    m=self.module(value);item=dict(address=hex(address),value=hex(value),module=m)
                    if address<regs['esp']+128:words.append(item)
                    if m:candidates.append(item)
                result['top_stack_words']=words;result['stack_module_candidates']=candidates
                chain=[];fp=regs['ebp']
                for _ in range(64):
                    if not stack<=fp<stack+size-8:break
                    nxt,ret=self.u32(fp),self.u32(fp+4)
                    if nxt is None or ret is None:break
                    chain.append(dict(ebp=hex(fp),next=hex(nxt),return_address=hex(ret),module=self.module(ret)))
                    if nxt<=fp or nxt>=stack+size:break
                    fp=nxt
                result['ebp_chain']=chain
            results.append(result)
        return results


def main():
    p=argparse.ArgumentParser();p.add_argument('dump',type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);d=Dump(a.dump)
    try:
        pid=None
        if 15 in d.streams:
            _,off=d.streams[15];size,flags=d.unpack('2I',off)
            if flags&1:pid,=d.unpack('I',off+8)
        system=None
        if 7 in d.streams:
            _,off=d.streams[7];arch,level,revision,n,product,major,minor,build,platform,csd=d.unpack('3H2B5I',off)
            system=dict(architecture=arch,processors=n,major=major,minor=minor,build=build)
        digest=hashlib.sha256()
        for off in range(0,len(d.data),4*1024*1024):digest.update(d.data[off:off+4*1024*1024])
        threads=d.threads()
        result=dict(dump=str(a.dump),bytes=len(d.data),sha256=digest.hexdigest(),pid=pid,timestamp=d.timestamp,
            flags=hex(d.flags),system=system,streams={str(k):dict(size=s,rva=r) for k,(s,r) in d.streams.items()},
            modules=[dict(m,base=hex(m['base']),end=hex(m['end'])) for m in d.modules],threads=threads,
            memory_range_count=len(d.ranges),memory_bytes=sum(s for _,s,_ in d.ranges),
            limitations=['Stack module values are candidates, not an unwind.',
                'EBP chains are heuristic; x86 optimized code may use EBP as a general register.',
                'A single snapshot does not by itself distinguish a persistent wait from transient execution.'])
        (a.output/'dump_summary.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
        print(json.dumps(dict(pid=pid,bytes=len(d.data),system=system,flags=hex(d.flags),
            memory_ranges=len(d.ranges),threads=[dict(id=t['id'],eip=(t['registers'] or {}).get('eip'),
                module=None if not t.get('instruction_module') else Path(t['instruction_module']['name']).name,
                user_seconds=(t['info'] or {}).get('user_seconds'),source=t['context_source'],
                driver_stack=[w['value'] for w in t.get('stack_module_candidates',[]) if w['module']['name'].lower().endswith('driver.exe')][:6]) for t in threads]),indent=2))
    finally:d.close()


if __name__=='__main__':main()

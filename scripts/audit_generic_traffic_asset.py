"""Read-only roster/weight audit of one explicitly named AI chunk; no game."""
import argparse,hashlib,json,math,struct
from pathlib import Path
def audit(path,capacity=150,base=0x1080):
    data=Path(path).read_bytes()
    def span(at,n):
        if at<0 or n<0 or at+n>len(data):raise ValueError('Chunk range outside file')
        return at
    def u32(at):return struct.unpack_from('<I',data,span(at,4))[0]
    def u16(at):return struct.unpack_from('<H',data,span(at,2))[0]
    def array(count_field,pointer_field,stride):
        n=u32(base+count_field);at=base+u32(base+pointer_field);span(at,n*stride);return at,n
    settings,sn=array(0x118,0x11C,28);lists,ln=array(0x120,0x124,2)
    groups,gn=array(0xE8,0xEC,12);routes,rn=array(0x128,0x12C,20)
    metadata,tn=array(0x158,0x15C,12);weights,wn=array(0x160,0x164,8)
    profiles=[]
    for slot in range(sn):
        record=settings+slot*28;model_slot=data[span(record+24,1)]
        for frequency in range(5):
            count=u16(record+14+frequency*2)
            if not count:continue
            start=u16(record+4+frequency*2)
            if start>ln or count>ln-start:raise ValueError('Invalid native request range')
            native=[u16(lists+(start+i)*2) for i in range(count)];positive=set();tables=set()
            for g in range(gn):
                route=u16(groups+g*12+8)+model_slot
                if route>=rn:raise ValueError('Invalid route')
                table=data[span(routes+route*20+13+frequency,1)]
                if table>=tn:raise ValueError('Invalid table')
                tables.add(table)
            for table in tables:
                first,n=u32(metadata+table*12),u32(metadata+table*12+4)
                if first>wn or n>wn-first:raise ValueError('Invalid weight range')
                for row in range(n):
                    at=weights+(first+row)*8;weight=struct.unpack_from('<f',data,at+4)[0]
                    if not math.isfinite(weight):raise ValueError('Nonfinite weight')
                    if weight>0:positive.add(u32(at))
            union=set(native)|positive
            profiles.append(dict(hash=f'{u32(record):08X}',frequency=frequency,slot=slot,
                native=native,native_count=count,positive_count=len(positive),union_count=len(union),
                missing_from_native=sorted(positive-set(native)),within_capacity=len(union)<=capacity,
                valid_ids=all(0<n<=65535 for n in union)))
    return dict(file=str(path),sha256=hashlib.sha256(data).hexdigest(),bytes=len(data),capacity=capacity,
        profiles=profiles,all_within_capacity=all(p['within_capacity'] for p in profiles),
        all_ids_valid=all(p['valid_ids'] for p in profiles),
        settings_requiring_augmentation=sum(bool(p['missing_from_native']) for p in profiles),
        maximum_union=max((p['union_count'] for p in profiles),default=0))
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('asset',type=Path);p.add_argument('--capacity',type=int,default=150);p.add_argument('--ai-offset',type=lambda s:int(s,0),default=0x1080);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    r=audit(a.asset,a.capacity,a.ai_offset);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(r,indent=2)+'\n')
    print(json.dumps({k:v for k,v in r.items() if k!='profiles'},indent=2))

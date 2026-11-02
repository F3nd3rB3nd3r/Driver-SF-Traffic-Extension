#!/usr/bin/env python3
"""V29s passive native request reconciliation/dispatch/transfer diagnostic summary.
Accepts one or two full DSFV29f_worldmem logs (45-success and 150-stall).
Does not interpret completed map as a READY/drawable counter.
"""
import argparse
from pathlib import Path
import re

def parse(path):
    out={'start':None,'status':[],'ids':{},'kinds':{}}
    for line in path.read_text(errors='replace').splitlines():
        if line.startswith('DSFV29s_START '):out['start']=line
        elif line.startswith('DSFV29s_STATUS '):
            out['status'].append(dict((k,int(v)) for k,v in re.findall(r'([A-Za-z]+)=(-?\d+)',line)))
        elif line.startswith('DSFV29s_ID '):
            row=dict((k,int(v)) for k,v in re.findall(r'([A-Za-z]+)=(-?\d+)',line))
            if 'id' in row:out['ids'][row['id']]=row
        elif line.startswith('DSFV29s_KIND '):
            row=dict((k,int(v)) for k,v in re.findall(r'([A-Za-z]+)=(-?\d+)',line))
            if 'kind' in row:out['kinds'][row['kind']]=row
    return out

def summary(p,d):
    print('\n===',p.name,'===')
    print('START:',d['start'] or '(missing V29s start; wrong ASI/log?)')
    ss=d['status']
    if not ss:
        print('NO DSFV29s_STATUS samples; cannot infer activity')
        return
    s=ss[-1]
    for k in ['tick','hooks','reconcile','rosterLast','gate','blocked','dispatch',
              'memberPass','transfers','transferHasHandler','map','reqHigh',
              'hashLeft','efGuard','idOverflow']:
        print(f'{k:>22} = {s.get(k,"unknown")}')
    print('Last 6 five-second status snapshots:')
    for row in ss[-6:]:
        print('  tick={tick} map={map} reqHigh={reqHigh} gate={gate} blocked={blocked} dispatch={dispatch} pass={memberPass} transfers={transfers} rosterLast={rosterLast}'.format(**{k:row.get(k,'?') for k in ['tick','map','reqHigh','gate','blocked','dispatch','memberPass','transfers','rosterLast']}))
    print('Kinds (totals):', ', '.join(f'{i}:gate={row.get("gate")} blocked={row.get("blocked")}' for i,row in sorted(d['kinds'].items())) or '(none)')
    ids=d['ids']
    print('IDs with recorded activity:',len(ids))
    groups={
        'gate-only (no dispatch)':lambda row:row.get('gate',0)>0 and row.get('dispatch',0)==0,
        'dispatched (no membership pass)':lambda row:row.get('dispatch',0)>0 and row.get('pass',0)==0,
        'membership passed (no transfer yet observed)':lambda row:row.get('pass',0)>0 and row.get('transfer',0)==0,
        'transferred':lambda row:row.get('transfer',0)>0,
    }
    for title,pred in groups.items():
        group=[i for i,v in sorted(ids.items()) if pred(v)]
        print(title,len(group),'IDs:',', '.join(map(str,group[:48]))+(' ...' if len(group)>48 else ''))
    print('Reminder: different paths and preexisting objects may be represented; no simple one-to-one completion is guaranteed.')

def compare(a,b):
    print('\n=== Per-ID counter differences: file 1 versus file 2 ===')
    for i in sorted(set(a['ids'])|set(b['ids'])):
        x=a['ids'].get(i,{});y=b['ids'].get(i,{})
        if not x or not y:
            print(f'  id={i:>3} present in {"both" if x and y else "file1" if x else "file2"} only')
            continue
        keys=('gate','blocked','dispatch','pass','transfer')
        if any(x.get(k,0)!=y.get(k,0) for k in keys):
            print(f'  id={i:>3} f1:',*(x.get(k,0) for k in keys),'f2:',*(y.get(k,0) for k in keys))

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('logs',nargs='+',type=Path)
a=p.parse_args()
if len(a.logs)>2:p.error('provide 1 log or 2 for comparison')
data=[]
for path in a.logs:
    if not path.exists():p.error(f'not found: {path}')
    d=parse(path);data.append(d);summary(path,d)
if len(data)==2:compare(*data)

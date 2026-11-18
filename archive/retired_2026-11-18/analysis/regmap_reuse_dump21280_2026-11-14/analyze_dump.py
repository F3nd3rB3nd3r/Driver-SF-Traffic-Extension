"""Read-only supplied PID21280 dump and the two explicitly discussed AI assets."""
import bisect,hashlib,json,re,struct,sys
from pathlib import Path
import dump_reader as d
ROOT=Path(__file__).resolve().parents[2];OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'scripts'))
from static_streaming_audit import PE
assert d.pid==21280
info=[]
off=d.streams[16][1];header,entry,count=d.unpack('2IQ',off)
for i in range(count):
    base,allocation,allocation_protect,_,size,state,protect,kind,_=d.unpack('2Q2IQ4I',off+header+i*entry)
    info.append(dict(base=base,end=base+size,size=size,allocation=allocation,state=state,protect=protect,type=kind))
info.sort(key=lambda x:x['base']);starts=[x['base'] for x in info]
def region(a):
    i=bisect.bisect_right(starts,a)-1
    return info[i] if i>=0 and a<info[i]['end'] else None
def readable(a,n):
    m=region(a)
    return bool(a and 0<=n<=0x100000 and a+n<=0xFFFFFFFF and m and m['state']==0x1000 and not(m['protect']&0x100) and (m['protect']&0xff)!=1 and a+n<=m['end'])
def memory(a,n):
    # Dump capture segments are separate from VirtualQuery protection regions.
    chunks=[]
    while n:
        i=bisect.bisect_right(d.range_starts,a)-1
        assert i>=0,(hex(a),n)
        lo,hi,rva=d.ranges[i];take=min(n,hi-a);assert take>0,(hex(a),n)
        chunks.append(d.data[rva+a-lo:rva+a-lo+take]);a+=take;n-=take
    return b''.join(chunks)
def u16(a):return struct.unpack('<H',memory(a,2))[0]
def u8(a):return memory(a,1)[0]
def row(a):return struct.unpack('<2I',memory(a,8))
root=d.word(0xFE7228);ai=d.word(0xFE7AC0);owner=d.word(root+0x778);m=owner+0x254;sentinel=m+0x444
nodes=[];node=d.word(sentinel+4);seen=set();previous=sentinel
while node!=sentinel:
    assert node not in seen and len(seen)<557 and d.word(node)==previous
    seen.add(node);value=d.word(node+12);definition=d.word(value+0x4850)
    nodes.append(dict(node=node,key=d.word(node+8),value=value,definition=definition,definition_id=d.word(definition+0x14),references=d.word(node+16)))
    previous=node;node=d.word(node+4)
assert d.word(sentinel)==previous and len(nodes)==d.word(m+8)
keys={n['key'] for n in nodes};first_eligible=next(n for n in nodes if struct.unpack('<i',struct.pack('<I',n['references']))[0]<=0)
assert first_eligible['key']==15 and first_eligible['definition_id']==15
fields=[(0x118,59,0x11C,28,'settings'),(0x120,1369,0x124,2,'lists'),(0xE8,319,0xEC,12,'groups'),
    (0x128,18821,0x12C,20,'routes'),(0x158,208,0x15C,12,'metadata'),(0x160,39728,0x164,8,'weights')]
arrays={};checks=[]
for field,expected,pointer,stride,name in fields:
    address=d.word(ai+pointer);actual=d.word(ai+field);assert actual==expected
    arrays[name]=address;checks.append(dict(name=name,count=actual,address=address,bytes=expected*stride,
        production_readable=readable(address,expected*stride),region=region(address)))
assert readable(ai,0x168) and all(c['production_readable'] for c in checks)
profiles=json.loads((ROOT/'analysis/chapters_r1_2026-11-06/profiles.json').read_text())['profiles']
header=(ROOT/'src/V29sChapterProfiles.h').read_text()
expected_ids=[int(x.rstrip('u'),0) for x in re.search(r'kChapterRowIds\[191\]=\{(.*?)\};',header).group(1).split(',')]
expected_weights=[int(x.rstrip('u'),0) for x in re.search(r'kChapterRowWeights\[191\]=\{(.*?)\};',header).group(1).split(',')]
ss_path=Path(r'E:\Games\Driver San Francisco SS Test\media\san_franciscoAI.fchunk');ss=ss_path.read_bytes()
vanilla_path=Path(r'E:\Games\Driver San Francisco Save Bug Test\media\san_franciscoAI.fchunk');vanilla=vanilla_path.read_bytes()
result_profiles=[]
for p in profiles:
    if p['hash'] not in ('AE88E83B','47EB4D0E') or p['frequency']!=0:continue
    record=arrays['settings']+p['slot']*28;start=u16(record+4);native_count=u16(record+14)
    assert d.word(record)==int(p['hash'],16) and u8(record+24)==p['slot'] and native_count==len(p['ids'])
    assert [u16(arrays['lists']+(start+i)*2) for i in range(native_count)]==p['ids']
    selected=sorted({u8(arrays['routes']+(u16(arrays['groups']+i*12+8)+p['slot'])*20+13) for i in range(319)})
    assert selected==p['tables']
    tables=[]
    for t in selected:
        at=arrays['weights']+t*191*8;rows=[row(at+i*8) for i in range(191)]
        meta=d.words(arrays['metadata']+t*12,3);assert meta==[t*191,191,0x43160000]
        positive=[r[0] for r in rows if struct.unpack('<f',struct.pack('<I',r[1]))[0]>0]
        pairs=[(i,w) for i,w in rows]
        weights=[w for _,w in pairs];ids=[i for i,_ in pairs]
        mismatch=[dict(index=i,expected_id=expected_ids[i],actual_id=ids[i],expected_weight_bits=expected_weights[i],actual_weight_bits=weights[i]) for i in range(191) if pairs[i]!=(expected_ids[i],expected_weights[i])]
        offset=0x2DE440+t*191*8;ss_match=memory(at,191*8)==ss[offset:offset+191*8]
        assert ss_match and len(positive)==150 and not keys.intersection(positive)
        tables.append(dict(table=t,positive_count=len(positive),positive_ids=positive,
            registered_positive_ids=sorted(keys.intersection(positive)),row_id_mismatches=sum(i!=e for i,e in zip(ids,expected_ids)),
            row_weight_mismatches=sum(w!=e for w,e in zip(weights,expected_weights)),first_mismatches=mismatch[:5],
            matches_active_ss_disk=ss_match,matches_save_bug_disk=memory(at,191*8)==vanilla[offset:offset+191*8]))
    result_profiles.append(dict(hash=p['hash'],frequency=0,slot=p['slot'],native_request_ids=p['ids'],native_count=native_count,
        header_lists_routes_metadata_match=True,first_failed_predicate='weight bits at row0 of first selected table',tables=tables))
asi=next(x for x in d.modules if x['name'].endswith('REGMAP_REUSE_R2.asi'))
symbols={}
for line in (ROOT/'build/v29s_regmap_reuse_r2/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.map').read_text().splitlines():
    match=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
    if match:symbols[match[1]]=int(match[2],16)
def symbol(prefix):return asi['base']+next(v for k,v in symbols.items() if k.startswith(prefix))-0x10000000
head=d.word(symbol('?g_regMapHead@'));arena=d.words(head,7)
assert arena[1]==m and arena[2]==1 and arena[3]==d.word(m+4)+d.word(m+8) and arena[4:6]==[0,0]
assert arena[6]==d.word(head+0x301C)==0x29DE51AC
counters={n:d.word(symbol('?'+n+'@')) for n in ['g_regMapOwners','g_regMapConstructs','g_regMapReconstructs','g_regMapChecks','g_regMapRedirects','g_regMapReattaches','g_regMapFaults','g_regMapAllocFailed','g_regMapExternalHigh']}
native=PE();code_checks={f'{a:08X}':memory(a,n)==native.read(a,n) for a,n in [(0x86B9E0,0xC2),(0x86BDC0,0x49),(0x86BE90,0x4C),(0x8CCAA0,0x65)]}
assert all(code_checks.values()) and d.word(owner)==0xE395D0 and d.word(0xE395E4)==0x86B9E0
with d.dump_path.open('rb') as f:digest=hashlib.file_digest(f,'sha256').hexdigest()
result=dict(pid=d.pid,dump_bytes=len(d.data),dump_sha256=digest,timestamp=d.dump_timestamp,flags=d.dump_flags,
    exception_stream=6 in d.streams,thread_count=len(d.threads),game_module=next(x for x in d.modules if x['base']==0x400000),
    traffic_module=asi,other_asi_modules=[x for x in d.modules if x['name'].lower().endswith('.asi') and x!=asi],
    root=root,ai=ai,ai_arrays=checks,profiles=result_profiles,
    active_ai_disk=dict(path=str(ss_path),sha256=hashlib.sha256(ss).hexdigest()),
    prior_checked_disk=dict(path=str(vanilla_path),sha256=hashlib.sha256(vanilla).hexdigest()),
    registry=dict(owner=owner,map=m,nodes=nodes,active=d.word(m+8),recycled=d.word(m+4),
        sidecar=head,sidecar_metadata=arena,counters=counters,first_eligible_fallback=first_eligible),
    native_code_matches=code_checks,native_fallback_method='0086B9E0',
    limitations=['Dump is a captured state, not a recording of every historical selection call.',
        'Model15 Mini identification is based on user observation plus profile/definition ID15, not independently decoded display name.',
        'Other ASI module presence alone does not establish modification responsibility.',
        'R2 vanilla lifecycle and Forza compatibility need separate live validation.'])
(OUT/'dump_analysis.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(dict(pid=d.pid,dump_sha256=digest,arrays_readable=True,
    profiles=[dict(hash=p['hash'],native_count=p['native_count'],tables=len(p['tables']),positive_counts=sorted({t['positive_count'] for t in p['tables']}),all_positive_unregistered=all(not t['registered_positive_ids'] for t in p['tables'])) for p in result_profiles],
    registry_keys=sorted(keys),first_eligible_key=first_eligible['key'],regmap_counters=counters,code_checks=code_checks),indent=2))

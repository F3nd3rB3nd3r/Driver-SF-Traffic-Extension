"""Pin the R8 production ASI to the native queue fixtures and inspect its bridges."""
import hashlib,json,re,struct
from pathlib import Path
from static_streaming_audit import PE
from compare_audio_slots_logs import rows
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/traffic_queues_r8_2026-11-16'
ASI=ROOT/'build/v29s_usercap_r8/DriverSF_Unlimiter_USERCAP_R8.asi'

def main():
    f=json.loads((OUT/'fixture_manifest.json').read_text())
    assert f['result']=='passed' and f['processes']==6
    for name,digest in f['dependencies'].items():assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest,('stale fixture',name)
    assert hashlib.sha256((ROOT/'build/traffic_queues_r8_fixtures/traffic.exe').read_bytes()).hexdigest()==f['fixture_sha256']
    parent=json.loads((ROOT/'analysis/wide_limits_r8_2026-11-16/build_verification.json').read_text())
    digest=hashlib.sha256(ASI.read_bytes()).hexdigest();assert parent['result']=='passed' and parent['sha256']==digest
    pe=PE(ASI)
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(address):
        end=min(v for v in symbols.values() if v>address);return pe.read(address,end-address)
    def calls(address,target):
        code=body(address)
        def resolve(destination):
            for _ in range(8):
                if pe.read(destination,1)!=b'\xe9':break
                destination=(destination+5+struct.unpack('<i',pe.read(destination+1,4))[0])&0xffffffff
            return destination
        return any(code[i]==0xE8 and resolve((address+i+5+struct.unpack_from('<i',code,i+1)[0])&0xffffffff)==target for i in range(len(code)-4))
    require=at('?TrafficQueueRequire@');prepare=at('?TrafficQueuePrepare@')
    assert calls(require,prepare)
    hooks=[]
    for name,helper,index in [('Retire','_CapacityRetireEntry',2),('Traffic','_CapacityTrafficEntry',1)]:
        bridge=symbols['_Capacity'+name+'Bridge'];code=body(bridge)
        for pattern in ['9c','60','0fae0424','0fae0c24','8be5619d']:assert bytes.fromhex(pattern) in code,(name,pattern)
        assert calls(bridge,symbols[helper]) and calls(symbols[helper],require)
        assert b'\xff\x25'+struct.pack('<I',at('?g_capacityTramps@')+index*4) in code
        if name=='Traffic':
            for caller in (0x5A68A7,0x5A7E3D,0x5A7E6D):assert struct.pack('<I',caller) in code
        hooks.append(dict(bridge=name,preserve_fx=True,helper_guard_reachable=True,existing_trampoline=index))
    for marker in [b'DSFV29s_usercap_r8_',b'DSFV29s_TRAFFIC_QUEUE_START',b'DSFV29s_TRAFFIC_QUEUE_FATAL',b'coldReset=newGeneration',b'headerUnreadable',b'arenaGuard',b'configuredCapacity',b'Unlimiter: traffic queue hooks unavailable']:
        assert marker in pe.data,marker
    startups=[]
    for label in ['150','700','1000','customized']:
        records=rows(ROOT/'analysis/wide_limits_r8_2026-11-16'/('startup_'+label+'.log'))
        cfg=next(r for r in records if r['type']=='DSFV29s_GENERIC_START');expected=max(512,int(cfg['modelSlots'])+int(cfg['retainedReferences']))
        queues=[r for r in records if r['type']=='DSFV29s_TRAFFIC_QUEUE_START']
        assert len(queues)==3 and {r['queue'] for r in queues}=={'retireIds','retainedVehicles','vehicleOperations'}
        assert all(r['status']=='1' and int(r['capacity'])==expected and r['beforeNativeWrite']=='1' for r in queues)
        startups.append(dict(label=label,capacity=expected,queues=3))
    result=dict(result='passed',asi=str(ASI),sha256=digest,bytes=ASI.stat().st_size,traffic_processes=6,inherited_processes=19,bridges=hooks,startups=startups,scope='offline native slices, full cold installation and compiled production verification; gameplay pending')
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()

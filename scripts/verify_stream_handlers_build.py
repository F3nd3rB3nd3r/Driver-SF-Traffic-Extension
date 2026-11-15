"""Pin R7 production hooks/layout to the isolated native fixtures and audit."""
import hashlib,json,re,struct
from pathlib import Path
from static_streaming_audit import PE,EXE
from analyze_generic_limits_log import analyze
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/stream_handlers_r7_2026-11-15'
ASI=ROOT/'build/v29s_usercap_r7/DriverSF_Unlimiter_USERCAP_R7.asi'
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
    fixture=json.loads((OUT/'fixture_manifest.json').read_text())
    assert fixture['result']=='passed' and fixture['processes']==11
    for name,h in fixture['dependencies'].items():assert digest(ROOT/name)==h,('stale fixture',name)
    assert digest(ROOT/'build/stream_handlers_r7_fixtures/stream.exe')==fixture['fixture_sha256']
    parent=json.loads((ROOT/'analysis/wide_limits_r7_2026-11-15/build_verification.json').read_text())
    queues=json.loads((ROOT/'analysis/traffic_queues_r7_2026-11-15/build_verification.json').read_text())
    assert parent['result']==queues['result']=='passed' and parent['sha256']==queues['sha256']==digest(ASI)
    native=PE(EXE);pe=PE(ASI)
    assert digest(EXE)=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    rows=json.loads((OUT/'scheduler_operands.json').read_text())
    assert len(rows)==362 and len({r['operand'] for r in rows})==362
    for r in rows:
        a=int(r['instruction'],16);b=native.read(a,r['length'])
        assert b.hex()==r['signature'] and native.read(int(r['operand'],16),4)==struct.pack('<I',r['original'])
        h=2166136261
        for c in b:h=((h^c)*16777619)&0xffffffff
        assert h==r['signature_fnv1a']
    base_sites=[0x59c707,0x59c759,0x59c7d9,0x59c85a,0x59c969,0x59c9f9,0x59ca79,0x59d524,0x59d7d9,0x59d91e,0x59d9dc]
    assert {int(a,16)-2 for a in native.refs(0x6fda0,False)}==set(base_sites)|{0x59dceb,0x59de51}
    assert native.refs(0x59e5b0,True)==['0059DCF1','0059DE57']
    assert native.refs(0x16620,False)==['005A5C61']
    assert set(native.refs(0x15d5c,False))=={'005A2D1D','005A4364','005A43B2','005A4D50'}
    assert native.refs(0x5a55a0,True)==['005A438C','005A4D78']
    assert {r['operand'] for r in rows}>={'005A2D1D','005A4364','005A43B2','005A4D50','005A55A2','005A55C7','005A55DE'}
    # Every decoded whole-image memory access to the scheduler tail is present,
    # including non-scheduler gameplay code. Immediate base materializations
    # are captured separately in scheduler_operands.json.
    by_instruction={r['instruction'] for r in rows}
    for line in (OUT/'all_scheduler_tail_refs.txt').read_text().splitlines():
        assert line.strip().split(':')[0] in by_instruction,line
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines()
             if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    table=at('?kStreamLayoutOperands@')
    for i,r in enumerate(rows):
        assert struct.unpack('<5I',pe.read(table+20*i,20))==(int(r['operand'],16),r['original'],int(r['instruction'],16),r['length'],r['signature_fnv1a'])
    def body(a):
        end=min(v for v in symbols.values() if v>a);return pe.read(a,end-a)
    def resolve(a):
        for _ in range(8):
            if pe.read(a,1)!=b'\xe9':return a
            a=(a+5+struct.unpack('<i',pe.read(a+1,4))[0])&0xffffffff
        raise AssertionError('jump chain')
    bridges=[]
    for i,name in enumerate(('Construct','Destroy','Null','SchedulerConstruct','SchedulerDestroy','Vector','Member','FourthVector','Resize')):
        address=symbols['_Stream'+name+'Bridge'];code=body(address)
        for sequence in ('9c60','0fae0424','0fae0c24','8be5619d'):assert bytes.fromhex(sequence) in code,(name,sequence)
        target=symbols['_Stream'+name+'Entry']
        assert any(code[k]==0xe8 and resolve((address+k+5+struct.unpack_from('<i',code,k+1)[0])&0xffffffff)==target for k in range(len(code)-4)),name
        if name=='Resize':assert b'\xc2\x08\x00' in code
        else:assert b'\xff\x25'+struct.pack('<I',at('?g_streamTramps@')+4*i) in code
        bridges.append(dict(name=name,register_flags_fx_preserved=True))
    for marker in [b'DSFV29s_usercap_r7_',b'DSFV29s_STREAM_HANDLER_START',b'DSFV29s_STREAM_HANDLER_STATUS',
                   b'DSFV29s_STREAM_HANDLER_FATAL',b'layoutSignature',b'noFreeDispatchHandler',b'schedulerVectorFullOrGuard',
                   b'membershipFullOrCursor',b'stableGenerationSlabs=1',b'streamHandlerSchedulerLayout']:
        assert marker in pe.data,marker
    startups=[]
    for label in ('150','700','1000','customized'):
        result=analyze(ROOT/'analysis/wide_limits_r7_2026-11-15'/('startup_'+label+'.log'))
        assert result['checks']['stream_handler_start'],(label,result['checks'])
        assert not result['checks']['stream_handler_status'] and not result['recorded_integrity_pass']
        startups.append(dict(label=label,start=True,gameplay=False))
    # A missing R7 startup record must be rejected rather than silently taking
    # the legacy log branch.
    sample=ROOT/'analysis/wide_limits_r7_2026-11-15/startup_150.log'
    missing=OUT/'missing_stream_start.log'
    missing.write_text('\n'.join(l for l in sample.read_text().splitlines() if not l.startswith('DSFV29s_STREAM_HANDLER_START'))+'\n')
    assert not analyze(missing)['checks']['stream_handler_start']
    assert digest(ROOT/'dist/USERCAP_R4/DriverSF_Unlimiter_USERCAP_R4.asi')=='24cac4fdb996f8aa9f75b492508d0ca8df0f3bcde2f2085d1b3fab70ff2e2aed'
    result=dict(result='passed',asi=str(ASI),bytes=ASI.stat().st_size,sha256=digest(ASI),
                focused_processes=11,inherited_processes=25,layout_operands=362,base_patches=11,
                bridges=bridges,startups=startups,scope='offline supported native image/slices;280-model gameplay pending')
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('result','bytes','sha256','focused_processes','inherited_processes','layout_operands')},indent=2))
if __name__=='__main__':main()

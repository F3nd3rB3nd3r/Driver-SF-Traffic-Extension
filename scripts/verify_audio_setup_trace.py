"""Check R9's compiled diagnostic hooks and log parser, preserving R8 artifacts."""
import hashlib,json,re,struct,tempfile
from pathlib import Path
from static_streaming_audit import PE,EXE
from analyze_generic_limits_log import analyze
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/audio_setup_r9_2026-11-17'
ASI=ROOT/'build/v29s_usercap_r9/DriverSF_Unlimiter_USERCAP_R9.asi'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    fixture=json.loads((OUT/'fixture_manifest.json').read_text());assert fixture['result']=='passed'
    for name,h in fixture['dependencies'].items():assert digest(ROOT/name)==h,('stale fixture',name)
    p=PE(ASI);native=PE(EXE);assert struct.unpack_from('<H',p.data,p.pe+4)[0]==0x14c
    symbols={m[1]:int(m[2],16) for line in ASI.with_suffix('.map').read_text().splitlines() if (m:=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line))}
    def at(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    def body(a):return p.read(a,min(v for v in symbols.values() if v>a)-a)
    def resolve(a):
        for _ in range(8):
            try:b=p.read(a,5)
            except ValueError:return a
            if b[0]!=0xE9:return a
            a=(a+5+struct.unpack_from('<i',b,1)[0])&0xFFFFFFFF
        raise AssertionError('jump chain')
    def calls(a,target):
        code=body(a)
        return any(code[i]==0xE8 and resolve((a+i+5+struct.unpack_from('<i',code,i+1)[0])&0xFFFFFFFF)==target for i in range(len(code)-4))
    table=at('?kAudioSetupSites@');spans=[]
    for i in range(10):
        va,n=struct.unpack('<II',p.read(table+28*i,8));raw=p.read(table+28*i+8,n);bridge=struct.unpack('<I',p.read(table+28*i+24,4))[0]
        assert raw==native.read(va,n)
        code=body(resolve(bridge))
        for pattern in ['9c60','0fae0424','0fae0c24','fc','8be5619d','ff6424e8']:assert bytes.fromhex(pattern) in code,(i,pattern)
        assert calls(resolve(bridge),symbols['_AudioSetupEntry'])
        spans.append(dict(address=f'{va:08X}',length=n,bytes=raw.hex(),abi_preserved=True))
    original=at('?kAudioModelSites@')
    inherited=[struct.unpack('<II',p.read(original+24*i,8)) for i in range(8)]
    for r in spans:
        a=int(r['address'],16);b=a+r['length']
        for x,n in inherited:assert b<=x or a>=x+n,('parent overlap',r,x)
    assert calls(symbols['_AudioModelEntry'],at('?AudioSetupRecord@')) or calls(symbols['_AudioModelEntry'],at('?AudioSetupBankFailure@'))
    for token in [b'DSFV29s_usercap_r9_',b'DSFV29s_AUDIO_SETUP_START',b'DSFV29s_AUDIO_SETUP_STATUS',b'DSFV29s_AUDIO_SETUP_DETAIL',b'defaultProfileMissing',b'previewSuppressed',b'snapshotFlag']:assert token in p.data
    # Existing clean R8 evidence retains its result; diagnostics do not waive
    # the strict setupFail criterion or certify startup-only logs as gameplay.
    old=analyze(ROOT/'dist/USERCAP_R8/DSFV29s_usercap_r8_3600.log');assert old['recorded_integrity_pass']
    startup=(OUT/'startup_150_startup.log').read_text()
    names=('vehicleDisabled','bankNotReady','modelProfileMissing','defaultProfileMissing','previewSuppressed','invalidMode','modeInitializer','vehicleState','snapshotGetter','snapshotFlag')
    status='DSFV29s_AUDIO_SETUP_STATUS tick=1000 '+' '.join(k+'=0' for k in names)+' outerTotal=0 setupFailSample=0 detailDropped=0\n'
    with tempfile.TemporaryDirectory(prefix='dsf_audio_trace_parser_') as t:
        f=Path(t)/'DSFV29s_usercap_r9_fixture.log';f.write_text(startup+status)
        r=analyze(f);assert all(r['checks'][k] for k in ('audio_setup_trace_start','audio_setup_trace_status','audio_setup_trace_details'))
        assert not r['recorded_integrity_pass']
        for before,after,key in [('hooks=10','hooks=9','audio_setup_trace_start'),('outerTotal=0','outerTotal=1','audio_setup_trace_status'),('detailDropped=0','detailDropped=1','audio_setup_trace_details')]:
            f.write_text((startup+status).replace(before,after));assert not analyze(f)['checks'][key]
        f.write_text('\n'.join(v for v in (startup+status).splitlines() if not v.startswith('DSFV29s_AUDIO_SETUP_START'))+'\n');assert not analyze(f)['checks']['audio_setup_trace_start']
    r8=ROOT/'dist/USERCAP_R8/DriverSF_Unlimiter_USERCAP_R8.asi';assert digest(r8)=='5a7b62b2170881be116bca1de8bdf6ea2a6f8f9ae71db299256d5bcf0d3838bc'
    result=dict(result='passed',asi=str(ASI.relative_to(ROOT)),bytes=ASI.stat().st_size,sha256=digest(ASI),fixture_processes=fixture['processes'],spans=spans,parent_overlap=False,r8_unchanged=True,parser_cases=6,live_gameplay=False)
    (OUT/'build_verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k!='spans'},indent=2))
if __name__=='__main__':main()

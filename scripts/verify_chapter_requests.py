"""Compiled chapter extension, exact source/data identity and inherited checks."""
from contextlib import redirect_stdout
import hashlib,io,json,re,struct,subprocess
from pathlib import Path
from static_streaming_audit import PE,DUMPBIN
import verify_pending_map
from workspace_paths import historical_path
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/chapters_r1_2026-11-06'
BUILD=ROOT/'build/v29s_chapters_r1';NAME='DriverSF_V29s_160Capacity_150Request_CHAPTERS_R1'

def main(project='DriverSF_V29s_CHAPTERS.vcxproj',name=NAME,build=BUILD,output=OUT,
         extra_flags=None,log_prefix=b'DSFV29s_chapters_'):
    OUT,BUILD,NAME=output,build,name
    OUT.mkdir(parents=True,exist_ok=True)
    chapter_data=ROOT/'analysis/chapters_r1_2026-11-06'
    fixture=json.loads((chapter_data/'fixture_results.json').read_text());assert fixture['result']=='passed'
    for name,digest in fixture['production_sha256'].items():assert hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest()==digest
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    start=source.index('extern "C" void __fastcall RequestWriteBridge(');brace=source.index('{',start);depth=1;end=brace+1
    while depth:
        if source[end]=='{':depth+=1
        elif source[end]=='}':depth-=1
        end+=1
    bridge=source[start:end]
    if extra_flags and 'DSF_V29S_BASELINE150=1' in extra_flags:
        # Only the five explicitly delimited telemetry additions are removed.
        # The inherited request-selection/storage implementation remains hash locked.
        blocks=re.findall(r'#if defined\(DSF_V29S_BASELINE150\)\n(.*?)#endif\n',bridge,re.S)
        assert len(blocks)==5 and all('baseline' in b.lower() for b in blocks)
        bridge=re.sub(r'#if defined\(DSF_V29S_BASELINE150\)\n.*?#endif\n','',bridge,flags=re.S)
    assert hashlib.sha256(bridge.encode()).hexdigest()==fixture['production_bridge_sha256']
    console=io.StringIO()
    with redirect_stdout(console):verify_pending_map.main(project=project,name=NAME,build=BUILD,
        output=OUT/'inherited',extra_flags={'DSF_V29S_CHAPTERS=1'} | (extra_flags or set()),log_prefix=log_prefix)
    (OUT/'inherited_console.txt').write_text(console.getvalue())
    pe=PE(BUILD/(NAME+'.asi'));symbols={}
    for line in (BUILD/(NAME+'.map')).read_text().splitlines():
        m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
        if m:symbols[m[1]]=int(m[2],16)
    def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
    manifest=json.loads((chapter_data/'profiles.json').read_text())
    table=symbol('?kChapterProfiles@')
    for i,p in enumerate(manifest['profiles']):
        data=pe.read(table+i*96,96)
        assert struct.unpack_from('<6I',data)==(int(p['hash'],16),p['frequency'],p['slot'],len(p['ids']),len(p['tables']),p['expand'])
        assert list(struct.unpack_from('<27H',data,24))[:len(p['ids'])]==p['ids']
        assert list(data[78:78+len(p['tables'])])==p['tables']
    assert pe.read(symbol('?kChapterSetterBytes@'),198)==bytes.fromhex(manifest['native_setter_bytes'])
    for marker in (b'DSFV29s_CHAPTER_START revision=1 status=',b'expandedProfiles=55',b'callerReturn=0076FE9A',
                   b'DSFV29s_CHAPTER_EVENT reason=',b'DSFV29s_CHAPTER_STATUS',log_prefix):
        assert marker in pe.data,marker
    dll=source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('ChapterPreflight()')<dll.index('ApplyPreinitGraphicsExpansion();')
    assert dll.index('PendingInstall();')<dll.index('ChapterStartLine();',dll.index('PendingInstall();'))<dll.index('CreateThread(nullptr, 0, &Monitor')
    assert source[start:end].index('ChapterSelectRequest(reinterpret_cast<UPtr>(_ReturnAddress()),ctx,list,requested);')<source[start:end].index('RequestLock();')
    # Decode only the compiled request bridge/policy/preflight for review.
    for prefix in ('@RequestWriteBridge@16','?ChapterDecide@','?ChapterPreflight@'):
        a=symbol(prefix);b=min(v for v in symbols.values() if v>a)
        asm=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{a:X},0x{b-1:X}',str(BUILD/(NAME+'.asi'))],capture_output=True,text=True,check=True).stdout
        (OUT/(prefix.lstrip('@?').split('@')[0]+'.asm.txt')).write_text(asm)
    baseline=historical_path('dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_PENDING512_R1.asi')
    assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='f96fc542561528b8f1cfa3d8f958d88f92a508ee372d5db6f0392f077fc0409b'
    result=dict(result='passed',sha256=hashlib.sha256(pe.data).hexdigest(),bytes=len(pe.data),
                profiles=80,expanded_profiles=55,native_profiles_preserved=25,requests=150,graphics_slots=160,
                new_native_hook_sites=0,parent='PENDING512_R1',baseline_asi_preserved=True,
                limitations='Compiled/offline fixture coverage only; actual chapter loads/mission handling/menu/exit need a user run.')
    (OUT/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

if __name__=='__main__':main()

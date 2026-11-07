"""Run the exact native setter and production request bridge in fake globals."""
import hashlib,json,re,subprocess,winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/chapters_r1_2026-11-06';BUILD=ROOT/'build/chapter_request_fixture'

def main():
    OUT.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True,exist_ok=True)
    for name,digest in [('san_franciscoAI.fchunk','434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13'),
                        ('san_franciscoAI_original.fchunk','da5b1ca3e4e51b6c8d937ed0eb2cbeb2a54048c9aca5a3e5a26f8704b1f0b936')]:
        assert hashlib.sha256((Path(r'E:\Games\Driver San Francisco Save Bug Test\media')/name).read_bytes()).hexdigest()==digest
    source=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    start=source.index('extern "C" void __fastcall RequestWriteBridge(');brace=source.index('{',start);depth=1;end=brace+1
    while depth:
        if source[end]=='{':depth+=1
        elif source[end]=='}':depth-=1
        end+=1
    body=source[start:end]
    (ROOT/'tests/ChapterRequestBody.inc').write_text('// Exact production request bridge body.\n'+body+'\n')
    manifest=json.loads((OUT/'profiles.json').read_text())
    (ROOT/'tests/ChapterRosterValues.inc').write_text(','.join(str(n)+'u' for n in manifest['roster'])+'\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:sdk=Path(winreg.QueryValueEx(key,'KitsRoot10')[0])
    libs=[v/'um/x86/kernel32.lib' for v in (sdk/'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+',v.name)]
    lib=max((v for v in libs if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    exe=BUILD/'ChapterRequestTest.exe'
    command=[str(DUMPBIN.with_name('cl.exe')),'/nologo','/O2','/Oi-','/GS-','/GR-','/Zl','/EHs-c-',
        '/Fo'+str(BUILD/'ChapterRequestTest.obj'),str(ROOT/'tests/ChapterRequestTest.cpp'),'/link','/NODEFAULTLIB',
        '/ENTRY:ChapterTestMain@0','/SUBSYSTEM:CONSOLE','/MACHINE:X86','/BASE:0x10000000','/FIXED','/DYNAMICBASE:NO','/OUT:'+str(exe),str(lib)]
    build=subprocess.run(command,cwd=BUILD,capture_output=True,text=True);(OUT/'fixture_build.log').write_text(build.stdout+build.stderr)
    assert build.returncode==0,build.stdout+build.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
    assert run.returncode==0,(run.returncode,run.stdout,run.stderr)
    assert run.stdout.strip()=='chapter request checks passed'
    result=dict(result='passed',architecture='Win32',profiles=80,expanded=55,native_profiles_preserved=25,
                production_bridge_sha256=hashlib.sha256(body.encode()).hexdigest(),
                production_sha256={n:hashlib.sha256((ROOT/'src'/n).read_bytes()).hexdigest() for n in ('V29sChapterCore.inc','V29sChapterProfiles.h')},
                checks=['actual native setter bytes and compiled production bridge call ABI/RET8',
                    'all55 targets expand to same150;25 other profiles retain native counts',
                    'all55 expansion profiles reject original nonuniform asset',
                    'both other native call-site returns retain original18 IDs',
                    'empty frequency request clears normally',
                    'graphics/admission/context/owner/fullDWORD-source guards',
                    'weights/IDs/totals/routes/counts/pointers mutation refusals',
                    'native processing dispatch preserved',
                    'three repeated55-profile cycles use same request sidecar'],
                limitations='Isolated fixture globals and native lookup/rebuild/processing stubs. No real game, chapter manager, mission, audio or renderer executes.')
    (OUT/'fixture_results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))

if __name__=='__main__':main()

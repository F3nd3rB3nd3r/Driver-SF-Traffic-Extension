"""Check the archive and active paths without builds, fixtures, game or external reads."""
import ast
import hashlib
import importlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

from workspace_paths import ROOT, ARCHIVE_ROOT, historical_path


def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):
            h.update(block)
    return h.hexdigest()


def main():
    manifest=json.loads((ARCHIVE_ROOT/'manifest.json').read_text(encoding='utf-8'))
    assert manifest['status']=='complete'
    archived_files=0;archived_bytes=0
    for action in manifest['actions']:
        old=(ROOT/action['source']).resolve();new=(ROOT/action['destination']).resolve()
        old.relative_to(ROOT);new.relative_to(ARCHIVE_ROOT)
        if action['action']=='move' and action['source']!='scripts/__pycache__':
            assert not old.exists(),str(old)
        for item in action['files']:
            path=(ROOT/item['destination']).resolve();path.relative_to(ARCHIVE_ROOT)
            assert path.is_file() and path.stat().st_size==item['bytes'] and digest(path)==item['sha256'],str(path)
            archived_files+=1;archived_bytes+=item['bytes']
    for item in manifest['protected']:
        path=ROOT/item['path'];assert path.is_file() and digest(path)==item['sha256'],item['path']
    baseline=json.loads((ROOT/'CURRENT_BASELINE.json').read_text(encoding='utf-8'))
    assert digest(ROOT/baseline['asi'])==baseline['sha256']
    assert len(list((ROOT/'dist/MAIN_150').glob('*.asi')))==1
    projects=ET.parse(ROOT/baseline['solution']).getroot().findall('Project')
    assert len(projects)==1 and projects[0].attrib['Path']==baseline['project']
    assert [p.name for p in ROOT.glob('*.vcxproj')]==[baseline['project']]
    project=ET.parse(ROOT/baseline['project'])
    configs=project.findall('.//{*}ProjectConfiguration')
    assert len(configs)==1 and configs[0].attrib['Include']=='Release|Win32'
    for kind in ('ClCompile','ClInclude'):
        for entry in project.findall('.//{*}'+kind):
            if 'Include' in entry.attrib:
                assert (ROOT/entry.attrib['Include']).is_file(),entry.attrib['Include']
    # Import the active command chain, without invoking any main()/native tool.
    for name in ('verify_arena_reuse','analyze_callback_log'):
        importlib.import_module(name)
    parsed=0
    for path in (ROOT/'scripts').glob('*.py'):
        ast.parse(path.read_text(encoding='utf-8-sig'));parsed+=1
    # Exercise every moved baseline through the exact newly added path lookup,
    # comparing it to the verifier's existing expected digest, not a new value.
    baselines=[]
    names=('verify_arena_reuse','verify_callback_arena','verify_chapter_requests',
        'verify_pending_map','verify_cleanup_queue','verify_audio_model_registry',
        'verify_audio_reason','verify_audio_probe')
    for name in names:
        source=(ROOT/'scripts'/(name+'.py')).read_text()
        assignments={}
        for node in ast.walk(ast.parse(source)):
            if isinstance(node,ast.Assign) and isinstance(node.value,ast.Call) and isinstance(node.value.func,ast.Name) and node.value.func.id=='historical_path':
                assignments[node.targets[0].id]=ast.literal_eval(node.value.args[0])
        matches=re.findall(r"assert hashlib\.sha256\((\w+)\.read_bytes\(\)\)\.hexdigest\(\)==['\"]([a-f0-9]{64})['\"]",source)
        for variable,expected in matches:
            if variable not in assignments:
                continue
            path=historical_path(assignments[variable]);assert digest(path)==expected
            path.relative_to(ARCHIVE_ROOT)
            baselines.append(dict(verifier=name,path=path.relative_to(ROOT).as_posix(),sha256=expected))
    assert len(baselines)==8
    capacity=importlib.import_module('verify_capacity_trace')
    quiet=capacity.definitions('DriverSF_V29s_ORDERED_150_PERF.vcxproj')
    active=capacity.definitions(baseline['project'])
    assert quiet<=active and 'DSF_V29S_ARENA_REUSE=1' in active-quiet
    # Check preserved fixture provenance against unchanged production files.
    provenance=0
    for dirname in manifest['kept_analysis_directories']:
        path=ROOT/'analysis'/dirname/'fixture_results.json'
        if not path.is_file():
            continue
        data=json.loads(path.read_text());assert data['result']=='passed'
        for filename,expected in data.get('production_sha256',{}).items():
            assert digest(ROOT/'src'/filename)==expected,(dirname,filename)
            provenance+=1
    report=dict(result='passed',moved_files=sum(len(a['files']) for a in manifest['actions'] if a['action']=='move'),
        moved_bytes=sum(f['bytes'] for a in manifest['actions'] if a['action']=='move' for f in a['files']),
        original_control_documents_saved=sum(a['action']=='copy' for a in manifest['actions']),
        archived_files_verified=archived_files,archived_bytes_verified=archived_bytes,
        protected_files_unchanged=len(manifest['protected']),production_source_files_unchanged=sum(i['path'].startswith('src/') for i in manifest['protected']),
        python_modules_parsed=parsed,current_entry_point_imports=True,historical_baselines=baselines,
        active_project=baseline['project'],active_asi=baseline['asi'],active_asi_sha256=baseline['sha256'],
        fixture_source_digests_verified=provenance,
        game_or_native_fixture_run=False,build_performed=False,external_game_files_read=False,
        limitations='Organization/import/path/hash/provenance checks only; prior gameplay/native verification results were preserved rather than rerun.')
    output=ROOT/'analysis/WORKSPACE_ARCHIVE_2026-11-06.json'
    output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k!='historical_baselines'},indent=2))


if __name__=='__main__':
    main()

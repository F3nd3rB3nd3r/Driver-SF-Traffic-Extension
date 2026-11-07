"""One-time, non-deleting workspace archive with checked paths and SHA256 inventory.

Only explicitly selected workspace areas are inspected. No .git/.vs/reference/
tools/existing archive contents or external paths are traversed. --plan precedes
--apply; original files are renamed on the same volume, never overwritten.
"""
import argparse
import ast
import hashlib
import json
import os
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = ROOT / 'archive/retired_2026-11-06'
CURRENT = 'DriverSF_V29s_160Capacity_150Request_ARENA_REUSE_R1.asi'
CURRENT_SHA = '879236089985e71ae7e601d74bfc8a9f5570e3679128ec8957b5ccfbb015b5f7'
KEEP_ANALYSIS = {
    'arena_reuse_r1_2026-11-06', 'arena_reuse_run20236_2026-11-06',
    'audio_default16m_r1_2026-11-05', 'audio_model192_r1_2026-11-05',
    'audio_probe_r1_2026-11-05', 'audio_reason_r1_2026-11-05',
    'audio_slots2048_r1_2026-11-05', 'callback512_r1_2026-11-06',
    'chapters_r1_2026-11-06', 'cleanup262144_r1_2026-11-05',
    'indexed_capacity_r2_2026-11-05', 'indexed_handles_static_2026-11-05',
    'pending512_r1_2026-11-05', 'traffic_docs_chapter_audit_2026-11-06'}
SCRIPT_ROOTS = {
    'verify_arena_reuse', 'test_arena_reuse', 'analyze_callback_log',
    'test_callback_arena', 'test_chapter_requests', 'test_pending_map',
    'test_cleanup_queue', 'test_audio_model_registry', 'test_audio_reason',
    'test_audio_slots', 'test_audio_default_pool', 'test_audio_probe_bridges',
    'test_startup_checks', 'generate_chapter_profiles', 'audit_traffic_chapter_assets',
    'trace_audio_event_failures', 'audio_static_audit',
    'verify_vehicle_audio_registry_static', 'analyze_audio_reason_log',
    'compare_audio_reason_pair', 'archive_workspace_20261106'}


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024*1024), b''):
            h.update(block)
    return h.hexdigest()


def checked(path, archived=False):
    resolved = path.resolve()
    resolved.relative_to(ROOT)
    if resolved == ROOT:
        raise ValueError('Workspace root is never an archive target')
    if archived:
        resolved.relative_to(ARCHIVE.resolve())
    return resolved


def files(path):
    # Explicit metadata walk, rejecting reparse/junction paths before descent.
    checked(path)
    if path.is_symlink() or (hasattr(path, 'is_junction') and path.is_junction()):
        raise ValueError(f'Reparse point refused: {path}')
    if path.is_file():
        return [path]
    result = []
    for child in sorted(path.iterdir()):
        result.extend(files(child))
    return result


def keep_scripts():
    modules = {p.stem:p for p in (ROOT/'scripts').glob('*.py')}
    keep, todo = set(), list(SCRIPT_ROOTS)
    while todo:
        name = todo.pop()
        if name in keep:
            continue
        keep.add(name)
        tree = ast.parse(modules[name].read_text(encoding='utf-8-sig'))
        for node in ast.walk(tree):
            names = [n.name.split('.')[0] for n in node.names] if isinstance(node,ast.Import) else (
                [node.module.split('.')[0]] if isinstance(node,ast.ImportFrom) and node.module else [])
            todo.extend(n for n in names if n in modules and n not in keep)
    return keep


def plan():
    destination = checked(ARCHIVE/'manifest.json', True)
    if destination.exists():
        raise FileExistsError('Archive manifest already exists; use its recorded plan')
    assert digest(ROOT/'dist/MAIN_150'/CURRENT) == CURRENT_SHA
    actions = []
    def add(path, reason, copy=False):
        source = checked(path)
        relative = source.relative_to(ROOT)
        target = ARCHIVE / ('notes_before_archive' if copy else '') / relative
        checked(target, True)
        if target.exists():
            raise FileExistsError(target)
        inventory = []
        for file in files(source):
            saved = target / file.relative_to(source) if source.is_dir() else target
            inventory.append(dict(source=file.relative_to(ROOT).as_posix(),
                destination=saved.relative_to(ROOT).as_posix(), bytes=file.stat().st_size,
                sha256=digest(file)))
        actions.append(dict(source=relative.as_posix(), destination=target.relative_to(ROOT).as_posix(),
            action='copy' if copy else 'move', reason=reason, files=inventory))
    for name in ('START_HERE.md','NEXT_ACTION.md','README.md','PROMPT_FOR_VISUAL_STUDIO.md','AGENTS.md','.gitignore'):
        add(ROOT/name, 'Original control document preserved before curation', True)
    for path in sorted(ROOT.glob('*.vcxproj*')):
        if path.name not in ('DriverSF_V29s_ARENA_REUSE.vcxproj','DriverSF_V29s_ARENA_REUSE.vcxproj.user'):
            add(path, 'Retired project; restore to original location before building')
    for path in sorted(ROOT.glob('*.csv')):
        add(path, 'Historical reference table; production uses source headers')
    for path in sorted((ROOT/'build').iterdir()):
        if path.name != 'v29s_arena_reuse_r1':
            add(path, 'Historical/generated output; fixture sources remain available')
    for path in sorted((ROOT/'dist/MAIN_150').iterdir()):
        if path.name not in (CURRENT, 'ARENA_REUSE_R1_TEST_2026-11-06.md', 'DSFV29s_arena_reuse_20236.log'):
            add(path, 'Retired test artifact/log; historical baselines remain verifier-readable')
    for name in ('dist/CONTROL_45','dist/forza_package','verification'):
        add(ROOT/name, 'Retired control/deferred optional package/obsolete verification entry point')
    current_reports = {'ARENA_REUSE_R1_BUILD_2026-11-06.md','ARENA_REUSE_RUN20236_RESULTS_2026-11-06.md'}
    for path in sorted((ROOT/'analysis').iterdir()):
        if path.name not in KEEP_ANALYSIS | current_reports:
            add(path, 'Historical experiment interpretation; superseded as current instructions')
    keep = keep_scripts()
    for path in sorted((ROOT/'scripts').iterdir()):
        if path.name == '__pycache__' or (path.is_file() and path.suffix=='.py' and path.stem not in keep) or path.suffix=='.txt':
            add(path, 'Retired standalone investigation/variant or generated cache')
    add(ROOT/'tests/AudioStatic64PoolTest.cpp', 'Retired static-pool variant; shared-default pool regression remains active')
    protected = files(ROOT/'src') + [ROOT/'dist/MAIN_150'/CURRENT,
        ROOT/'dist/MAIN_150/DSFV29s_arena_reuse_20236.log',
        ROOT/'DriverSF_V29s_ARENA_REUSE.vcxproj',ROOT/'DriverSF_V29s_NATIVE_TRACE.slnx']
    payload = dict(status='planned', archive='archive/retired_2026-11-06',
        policy='No deletion or overwrite; preserve relative paths; verify SHA256 after same-volume moves.',
        kept_script_modules=sorted(keep), kept_analysis_directories=sorted(KEEP_ANALYSIS),
        protected=[dict(path=p.relative_to(ROOT).as_posix(),sha256=digest(p)) for p in protected],
        actions=actions)
    ARCHIVE.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(payload,indent=2)+'\n',encoding='utf-8')
    return payload


def apply():
    manifest = checked(ARCHIVE/'manifest.json', True)
    payload = json.loads(manifest.read_text(encoding='utf-8'))
    if payload['status']=='complete':
        raise RuntimeError('This one-time archive is already complete')
    for item in payload['protected']:
        assert digest(checked(ROOT/item['path']))==item['sha256'],item['path']
    # Preflight every source/destination before any directory move.
    for action in payload['actions']:
        source,target=checked(ROOT/action['source']),checked(ROOT/action['destination'], True)
        if source.exists() and target.exists() and action['action']=='move':
            raise FileExistsError(f'Both original and destination exist: {source}')
        for item in action['files']:
            old,new=checked(ROOT/item['source']),checked(ROOT/item['destination'], True)
            candidate=old if old.exists() else new
            assert candidate.is_file() and digest(candidate)==item['sha256'],str(candidate)
    payload['status']='in_progress'
    manifest.write_text(json.dumps(payload,indent=2)+'\n',encoding='utf-8')
    for action in payload['actions']:
        source,target=checked(ROOT/action['source']),checked(ROOT/action['destination'], True)
        if not target.exists():
            target.parent.mkdir(parents=True,exist_ok=True)
            if action['action']=='copy':
                shutil.copy2(source,target)
            else:
                os.rename(source,target)  # Same-volume, fail if destination exists.
        for item in action['files']:
            saved=checked(ROOT/item['destination'], True)
            assert saved.is_file() and digest(saved)==item['sha256'],str(saved)
        if action['action']=='move':
            assert not source.exists(),str(source)
    for item in payload['protected']:
        assert digest(ROOT/item['path'])==item['sha256'],item['path']
    payload['status']='complete'
    manifest.write_text(json.dumps(payload,indent=2)+'\n',encoding='utf-8')
    return payload


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('operation',choices=('plan','apply'))
    args=p.parse_args();payload=plan() if args.operation=='plan' else apply()
    moved=[a for a in payload['actions'] if a['action']=='move']
    print(json.dumps(dict(status=payload['status'],move_actions=len(moved),
        moved_files=sum(len(a['files']) for a in moved),
        moved_bytes=sum(f['bytes'] for a in moved for f in a['files']),
        original_documents_saved=sum(a['action']=='copy' for a in payload['actions']),
        protected_files=len(payload['protected']),manifest=str(ARCHIVE/'manifest.json')),indent=2))


if __name__=='__main__':
    main()

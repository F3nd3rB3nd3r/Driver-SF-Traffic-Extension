"""Plan/archive superseded documentation and closed evidence; never delete data.

Metadata inventory is restricted to analysis/, dist/ and the named controls.
No build, native execution, external files, Git mutation or old archive scan.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / 'archive/retired_2026-11-18'
OUT = ROOT / 'analysis/workspace_cleanup_2026-11-18'
CONTROLS = ['START_HERE.md', 'NEXT_ACTION.md', 'AGENTS.md',
            'CURRENT_BASELINE.json', 'PROMPT_FOR_VISUAL_STUDIO.md',
            'README.md', 'analysis/README.md', 'scripts/README.md',
            'tests/README.md', 'dist/README.md', 'dist/MAIN_150/README.md', 'build/README.md',
            '.gitignore', 'scripts/workspace_paths.py']
CURRENT_REPORTS = {
    'CAPACITY_COST_AUDIT_2026-11-15.md',
    'USERCAP_R9_AUDIO_AND_COVERAGE_2026-11-17.md',
    'USERCAP_R9_RUN26476_RESULTS_2026-11-18.md',
    'USERCAP_R9_RUN7044_RESULTS_2026-11-18.md',
    'USERCAP_R10_LOGGING_CONFIGURATION_2026-11-18.md',
    'USERCAP_R10_RUN20268_RESULTS_2026-11-18.md',
    'USERCAP_R10_RUN22856_REFUSAL_2026-11-18.md',
    'USERCAP_R10_RUN23108_REPLAY_2026-11-18.md', 'README.md'}
CURRENT_DATA = {'usercap_r9_run26476_2026-11-18',
                'usercap_r9_run7044_2026-11-18',
                'usercap_r10_run20268_2026-11-18',
                'usercap_r10_run23108_2026-11-18'}


def checked(path):
    path = path.resolve()
    if not path.is_relative_to(ROOT) or path == ROOT:
        raise ValueError(f'Path outside intended workspace: {path}')
    if path.is_symlink():
        raise ValueError(f'Refusing symbolic link: {path}')
    return path


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(4 * 1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def make_plan():
    # Conservative prerequisite discovery: preserve any filename or directory
    # mentioned by the retained tooling/tests or recorded fixture dependencies.
    code_files = list((ROOT / 'scripts').glob('*.py')) + list((ROOT / 'tests').glob('*.cpp'))
    code = '\n'.join(p.read_text(encoding='utf-8-sig') for p in code_files
                     if p.resolve() != Path(__file__).resolve())
    metadata = []
    for directory in (ROOT / 'analysis').iterdir():
        if directory.is_dir():
            for name in ['fixture_manifest.json', 'build_verification.json']:
                p = directory / name
                if p.is_file():
                    metadata.append(p.read_text(encoding='utf-8-sig'))
    for p in (ROOT / 'dist').glob('*/manifest.json'):
        metadata.append(p.read_text(encoding='utf-8-sig'))
    references = code + '\n' + '\n'.join(metadata)
    def referenced(name):
        return name in references
    moves = {}
    kept = []
    for p in sorted((ROOT / 'analysis').glob('*.md')):
        if p.name in CURRENT_REPORTS or referenced(p.name):
            kept.append(str(p.relative_to(ROOT)).replace('\\', '/'))
        else:
            moves[p] = 'superseded dated report; historical scope preserved'
    for directory in sorted((ROOT / 'analysis').iterdir()):
        if not directory.is_dir() or directory.name in CURRENT_DATA:
            continue
        name = directory.name
        closed = ('_run' in name or name.startswith(('generic_activity', 'generic_pair',
                  'regmap_reuse_dump', 'baseline150_crash_pair', 'notify150_7480',
                  'usercap_r2_story_crashes', 'usercap_r4_crash', 'usercap_r5_crash',
                  'usercap_r6_exit', 'usercap_r6_softlock', 'usercap_r7_getaway')))
        if not closed:
            continue
        files = [p for p in directory.rglob('*') if p.is_file()]
        if referenced(name) or any(referenced(p.name) for p in files):
            kept.append(str(directory.relative_to(ROOT)).replace('\\', '/') + '/')
            continue
        for p in files:
            moves[p] = 'closed investigation evidence; no retained tooling reference found'
    # Keep R9/R10 distributions and every named old prerequisite. Do not touch
    # TrafficEditor distributions or build/IDE/reference/source trees.
    for directory in sorted((ROOT / 'dist').iterdir()):
        if not directory.is_dir() or directory.name in {'USERCAP_R9', 'USERCAP_R10'}:
            continue
        if not (directory.name == 'MAIN_150' or re.fullmatch(r'USERCAP_R[2-8]', directory.name)):
            continue
        for p in directory.rglob('*'):
            if not p.is_file() or p.suffix.lower() not in {'.md', '.log', '.dmp'}:
                continue
            if referenced(p.name):
                kept.append(str(p.relative_to(ROOT)).replace('\\', '/'))
            else:
                moves[p] = 'retired test notes or raw run evidence; no retained tooling reference found'
    rows = []
    for p, reason in sorted(moves.items()):
        checked(p)
        rel = p.relative_to(ROOT).as_posix()
        rows.append(dict(original=rel, archived=(DEST / rel).relative_to(ROOT).as_posix(),
                         bytes=p.stat().st_size, action='move', reason=reason))
    for name in CONTROLS:
        p = checked(ROOT / name)
        if p.is_file():
            rows.append(dict(original=name, archived=(DEST / 'snapshots' / name).relative_to(ROOT).as_posix(),
                             bytes=p.stat().st_size, action='snapshot', reason='original control document before consolidation'))
    protected = set((ROOT / 'src').rglob('*')) | set((ROOT / 'tests').glob('*.cpp'))
    protected.update(ROOT / name for name in ['DriverSF_V29s_NATIVE_TRACE.slnx', 'DriverSF_V29s_ARENA_REUSE.vcxproj'])
    protected.update((ROOT / 'config').rglob('*.ini'))
    for rev in ['R9', 'R10']:
        protected.update((ROOT / ('dist/USERCAP_' + rev)).rglob('*'))
    fixture = json.loads((ROOT / 'analysis/logging_r10_2026-11-18/fixture_manifest.json').read_text())
    protected.update(ROOT / name for name in fixture['dependencies'])
    protected.update(ROOT / 'build/logging_r10_fixtures' / (name + '.exe') for name in fixture['fixture_hashes'])
    protected_rows = []
    for p in sorted(protected):
        if p.is_file():
            checked(p)
            if p in moves:
                raise ValueError(f'Candidate is protected: {p}')
            protected_rows.append(dict(path=p.relative_to(ROOT).as_posix(), bytes=p.stat().st_size, sha256=digest(p)))
    result = dict(state='planned', archive=DEST.relative_to(ROOT).as_posix(), entries=rows,
                  retained_prerequisites=sorted(set(kept)), protected=protected_rows,
                  scope='documentation and closed evidence only; no runtime/build/install/Git changes')
    write_json(OUT / 'plan.json', result)
    print(json.dumps(dict(files=len(rows), moves=sum(r['action']=='move' for r in rows),
                         MiB=round(sum(r['bytes'] for r in rows)/1048576, 2),
                         protected=len(protected_rows), retained=len(set(kept))), indent=2))


def apply_plan():
    plan = json.loads((OUT / 'plan.json').read_text())
    manifest_path = checked(DEST / 'manifest.json')
    if manifest_path.exists():
        raise ValueError('Archive manifest already exists; verify/recover instead of overwriting')
    # Validate every source/target before any move. Rename files individually,
    # never issue a recursive shell move/delete or overwrite an existing target.
    for row in plan['entries']:
        source, target = checked(ROOT / row['original']), checked(ROOT / row['archived'])
        if not target.is_relative_to(checked(DEST)) or target.exists():
            raise ValueError(f'Invalid/existing archive target: {target}')
        if not source.is_file() or source.stat().st_size != row['bytes']:
            raise ValueError(f'Changed archive source: {source}')
        row['sha256'] = digest(source)
    plan['state'] = 'archiving'
    write_json(manifest_path, plan)
    for index, row in enumerate(plan['entries']):
        source, target = checked(ROOT / row['original']), checked(ROOT / row['archived'])
        if digest(source) != row['sha256']:
            raise ValueError(f'Source changed before operation: {source}')
        target.parent.mkdir(parents=True, exist_ok=True)
        if row['action'] == 'snapshot':
            with target.open('xb') as stream:
                stream.write(source.read_bytes())
        else:
            source.rename(target)
        if target.stat().st_size != row['bytes'] or digest(target) != row['sha256']:
            raise ValueError(f'Archive verification failed: {target}')
        row['verified'] = True
        if index % 50 == 0:
            write_json(manifest_path, plan)
            print(f'Archived/verified {index+1}/{len(plan["entries"])}', flush=True)
    plan['state'] = 'archived_verified'
    write_json(manifest_path, plan)
    print('Archive bytes verified; control documents can now be consolidated.', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    apply_plan() if args.apply else make_plan()

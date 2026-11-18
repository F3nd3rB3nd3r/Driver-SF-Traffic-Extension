"""Read-only checks for the 2026-11-18 archive and current R10 entry points.

Hashes only the explicit manifest/protected paths. No compiler, game, native
fixture, debugger, Git mutation or broad historical-directory traversal.
"""
import ast
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from workspace_paths import historical_path

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = ROOT / 'archive/retired_2026-11-18'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(4 * 1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def within(name):
    p = (ROOT / name).resolve()
    assert p.is_relative_to(ROOT) and p != ROOT, name
    return p


def main():
    manifest = json.loads((ARCHIVE / 'manifest.json').read_text())
    assert manifest['state'] == 'archived_verified'
    moved = 0
    for index, row in enumerate(manifest['entries']):
        p = within(row['archived'])
        assert p.is_relative_to(ARCHIVE.resolve())
        assert row['verified'] and p.stat().st_size == row['bytes']
        assert digest(p) == row['sha256'], row['archived']
        if row['action'] == 'move':
            assert not within(row['original']).exists(), row['original']
            assert historical_path(row['original']).resolve() == p
            moved += 1
        if index % 50 == 0:
            print(f'Archive verified {index+1}/{len(manifest["entries"])}', flush=True)
    for row in manifest['protected']:
        p = within(row['path'])
        assert p.stat().st_size == row['bytes'] and digest(p) == row['sha256'], row['path']
    for rev in ['R9', 'R10']:
        base = ROOT / ('dist/USERCAP_' + rev)
        release = json.loads((base / 'manifest.json').read_text())
        for name, record in release['files'].items():
            p = base / name
            assert p.stat().st_size == record['bytes'] and digest(p) == record['sha256'], name
    fixture = json.loads((ROOT / 'analysis/logging_r10_2026-11-18/fixture_manifest.json').read_text())
    for name, expected in fixture['dependencies'].items():
        assert digest(within(name)) == expected, name
    for name, expected in fixture['fixture_hashes'].items():
        assert digest(ROOT / 'build/logging_r10_fixtures' / (name + '.exe')) == expected
    current = json.loads((ROOT / 'CURRENT_BASELINE.json').read_text())
    assert current['name'] == 'USERCAP_R10'
    assert digest(within(current['asi'])) == current['sha256']
    solution = ET.parse(ROOT / current['solution'])
    assert [p.attrib['Path'] for p in solution.findall('Project')] == [current['project']]
    project = ET.parse(ROOT / current['project'])
    assert project.find('.//{*}TargetName').text == 'DriverSF_Unlimiter_USERCAP_R10'
    for include in project.findall('.//{*}ClInclude') + project.findall('.//{*}ClCompile'):
        name = include.attrib.get('Include')
        if name:
            assert within(name).is_file(), name
    documents = ['START_HERE.md', 'NEXT_ACTION.md', 'README.md', 'CONFIGURATION.md',
                 'analysis/README.md', 'scripts/README.md', 'tests/README.md',
                 'dist/README.md', 'dist/USERCAP_R10/README.md']
    for name in documents:
        source = ROOT / name
        for target in re.findall(r'\]\(([^)]+)\)', source.read_text(encoding='utf-8-sig')):
            if not target.startswith(('http:', 'https:', '#')):
                assert (source.parent / target.split('#', 1)[0]).resolve().exists(), (name, target)
    modules = list((ROOT / 'scripts').glob('*.py'))
    for p in modules:
        ast.parse(p.read_text(encoding='utf-8-sig'), filename=str(p))
    for invalid in ['../outside', 'C:/outside']:
        try:
            historical_path(invalid)
        except ValueError:
            pass
        else:
            raise AssertionError(invalid)
    result = dict(result='passed', archived_files=len(manifest['entries']), moved_files=moved,
                  archived_bytes=sum(r['bytes'] for r in manifest['entries']),
                  protected_files=len(manifest['protected']), fixture_dependencies=len(fixture['dependencies']),
                  fixture_binaries=len(fixture['fixture_hashes']), python_modules=len(modules),
                  current_baseline=current['name'], build_or_game_run=False,
                  note='Files archived without deletion; original bytes/paths retained. No disk-space reclamation.')
    output = ROOT / 'analysis/workspace_cleanup_2026-11-18/verification.json'
    output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()

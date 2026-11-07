# Historical archive ? 2026-11-06

This is preserved research/test history, not the current build. Start at the workspace's `START_HERE.md` and `NEXT_ACTION.md` for current conclusions. Earlier reports can contain superseded limits, hypotheses and proposed next steps.

The archive moved 2026 files (273447374 bytes) and copied six original control documents before curation. Nothing was deleted or compressed. Relative paths are preserved below this folder. Existing earlier archives, git/IDE state, reference and tools were untouched.

`manifest.json` records every moved/copied file's original path, archived path, size and SHA256. All archived files were verified after moving; all54 production source files, the working ASI/project/solution and validated user log were verified unchanged.

- Root `.vcxproj` files: retired configurations. Restore to the original workspace path before building because their relative source paths assume that location.
- `dist/`: old ASIs/logs/guides, control45 and deferred Forza package.
- `analysis/`: historical interpretations and evidence not required by the current verifier.
- `build/`: preserved generated outputs; recreatable, but retained here.
- `scripts/`, `tests/`, `verification/`: retired standalone tools/variant fixtures.
- `notes_before_archive/`: verbatim former START_HERE, NEXT_ACTION, README, AGENTS, Visual Studio prompt and.gitignore.

To recover a specific file, use its manifest entry and restore it to the recorded original location only if that location is available. Do not overwrite current source or current handoffs with old versions. Archived scripts/projects are restore-first snapshots, not a self-contained runnable repository. Current verifiers use a narrow archive lookup for their historical baseline hashes and quiet-project comparison.

Keeping this archive preserves recoverability and provenance; it does not reduce total disk usage. Folder sizes in the active work areas are smaller because retired content is now here.

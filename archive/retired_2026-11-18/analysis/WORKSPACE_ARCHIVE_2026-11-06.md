# Workspace archive completed

The active workspace now presents ARENA_REUSE_R1 as its single working baseline. The older test material is preserved under `archive/retired_2026-11-06/`, with original relative paths, byte sizes and SHA256 hashes in `manifest.json`.

## Changes

- Moved 2026 files,273447374 bytes (260.78MiB), in278 file/directory operations. No artifact was deleted or compressed.
- Archived21 retired project definitions and their applicable user files. The active solution/project remain in their original paths.
- Archived61 previous MAIN_150 ASIs,old guides/logs/dump,CONTROL_45,and the deferred Forza package.
- Archived historical build outputs,closed investigation tools,a retired static-pool fixture and superseded analysis reports/evidence.
- Preserved six original control files verbatim before rewriting them: START_HERE,NEXT_ACTION,README,AGENTS,the Visual Studio prompt and.gitignore.
- Reduced the two handoffs from178389 to6736 bytes (96.2%). They now identify the verified build,measured limitations and open audio/parked/chapter questions without an accumulated obsolete task list.
- Added current indexes for analysis,scripts,tests,build and distribution,and a machine-readable `CURRENT_BASELINE.json`.

MAIN_150 now contains one ASI,the current test guide,the validated20236 log and a README. Archived files still occupy disk space inside this workspace.

## Dependency preservation

Retained the source and generated native-byte inputs required by the current fixture chain. Fourteen analysis directories remain because they hold current evidence or regression/profile prerequisites. Their older names are explained in `analysis/README.md`.

Added `scripts/workspace_paths.py`; eight current/shared verifiers now locate their preserved parent ASIs through its original-path/archive fallback. The quiet-project comparison likewise resolves its archived project definition. Expected hashes and validation logic remain intact; no C++ behavior or current ASI changed.

Retired scripts/projects are restore-first snapshots. They were not rewritten into a second runnable checkout. Use the manifest to restore specific historical files when needed; current entry points are listed in `scripts/README.md`.

## Verification

`python scripts/verify_workspace_archive.py` passed:

- Verified all2032 archived files,including six control-document copies,against the manifest.
- Confirmed all54 production source files,the current ASI,active solution/project and validated log are unchanged (58 protected files).
- Parsed38 Python modules and imported the current verifier/analyzer dependency chains.
- Located all eight archived baseline ASIs and matched their existing verifier digests.
- Checked active/quiet project definitions,Release/Win32 selection,project include paths and the single active ASI.
- Matched23 preserved fixture production-source digests against current source.

The checker can regenerate an ignored Python cache; this is distinct from the archived old cache. No build,game/native-fixture run,external game-file read,git history scan or process attachment was performed. Earlier gameplay/compiled/native results were preserved,not rerun as part of organization.

Machine result: `analysis/WORKSPACE_ARCHIVE_2026-11-06.json`. Existing earlier archives,reference,tools,IDE and git data were left in place.

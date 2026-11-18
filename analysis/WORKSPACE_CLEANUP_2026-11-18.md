# Workspace cleanup: current R10 guidance and historical archive

The user requested removal of unnecessarily retained outdated information
from the working project. Active control documents now describe USERCAP_R10.
Their prior accumulated history is preserved byte-for-byte in the archive;
no prior test evidence or uncommitted file was destroyed.

## Scope

- Archived 289 superseded files and snapshotted 14 control documents.
- Preserved 4,374,210,881 bytes with original paths and SHA256 hashes.
- Protected 175 source/configuration/release/fixture files against byte changes.
- Condensed START_HERE/NEXT_ACTION, corrected the stale baseline JSON,
  AGENTS and Visual Studio prompt, and refreshed analysis/tool/test/dist/build
  indexes. Default 150/160 capacities are distinguished from user-selected
  expanded configurations.
- Kept older inputs conservatively when tooling/tests or fixture/distribution
  manifests reference their filenames. Retired packages are labelled historical.
- Left build caches, the supported source/solution/project, existing archives,
  reference/tools/IDE data, TrafficEditor packages and game installations alone.
- Added archive path fallback and a focused read-only cleanup checker.
  No native build/test or game/debugger launch was performed. Repository
  publication is handled separately as described below.

## Recovery and verification

See [archive README](../archive/retired_2026-11-18/README.md) and its manifest.
Copies of the original controls are under snapshots/; moved files retain their
original relative paths inside the archive. Read those only for a specific
historical question. Dated reports retain their original scope, not current
recommendations.

Run `python scripts/verify_workspace_cleanup.py` for archive/protected hashes,
R9/R10 release manifests, all recorded R10 fixture dependencies and executable
hashes, project include/link consistency and Python syntax. Its machine result
is `workspace_cleanup_2026-11-18/verification.json`. This is an organization
check, not a new gameplay or full-trace certification.

## Completed verification

The read-only checker passed: all 303 archived files (289 moves and 14 control
snapshots), 175 protected files, 125 recorded R10 fixture dependencies and three
fixture binaries match their hashes. Both release manifests match. All 75
Python modules parse, active project includes and current documentation links
resolve, and the baseline JSON identifies USERCAP_R10. No native code ran.
The two handoffs shrank from 160132 bytes to roughly 6 KiB; their full originals
remain in snapshots. Archive-local attributes preserve original bytes during
Git publication as well. The user subsequently authorized pushing this cleanup.
The [publication index](../archive/retired_2026-11-18/publication.json) separates
the included relocated reports/snapshots from local-only raw evidence. The
full cleanup checker requires the complete local archive and native fixture
prerequisites; a repository download does not contain all 4.1 GiB.

Archiving clears the active folders but does not free disk space. A later
request for permanent deletion would be a separate operation.

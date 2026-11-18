# Next action: R10 is published; no required runtime patch identified

Read [START_HERE.md](START_HERE.md) for the active build and evidence.
The R10 candidate, configuration guide and profiles were pushed in commit
`9900ef5595fefe6081356d277212a026f076f46d`. The latest Story and Replay smoke
tests support the logging change. Do not repeat completed configuration
comparisons or increase capacities merely to obtain a different analyzer score.

## Current task

The user requested workspace cleanup. Superseded documents and completed
investigation material are archived with original-path/hash records. Current
entry points describe R10; old control documents are preserved as snapshots.
This cleanup makes no C++/ASI/INI capacity, game installation or save changes,
and performs no build/game/debugger run. The user authorized publication of
the cleanup. The repository includes relocated reports, snapshots and compact
records; full raw evidence remains local. The archive publication index lists
the included and local-only entries. No release tag is created by this change.

Verification entry point: `python scripts/verify_workspace_cleanup.py`.
It reads the explicit manifest/protected paths, checks archive integrity,
release/fixture hashes, active project links and Python syntax. It does not
build or execute native fixtures. See
[cleanup results](analysis/WORKSPACE_CLEANUP_2026-11-18.md).

## Useful future work, when requested

- Review a newly supplied log or reproducible symptom before proposing a patch.
- For actual 500+ models, install valid matching assets, populate the AI
  setting's weights and select sufficient INI capacities. Follow the
  [worked example](CONFIGURATION.md#setting-up-a-larger-roster-a-worked-500-model-example);
  then test driving/audio, activities and Replay at that actual demand.
- Other chapters' real mission/progression coverage is distinct from Lua
  traffic-setting changes. Frequency indices beyond the tested path retain
  their separate coverage scope.
- Investigate the local 16-event audio gate only if a reproducible audible
  symptom or stronger diagnostic evidence warrants it. It is not the model
  registry/global audio-list capacity.
- Prepare a GitHub Release only if requested; repository candidate files are
  already available. No new release tag is implied by the cleanup.

## Working boundaries

Use the existing Release/Win32 project only. Keep exactly one enabled traffic
ASI in the game folder. Preserve ED/EF sidecars and the user's selected
capacities; do not replace their 280-model AI's INI with the 150 baseline.

Do not build, launch the game/debugger, install dependencies, browse, benchmark,
deploy or change capacities without an applicable user request. Historical
verifiers remain pinned to their own revisions. Some need the supported local
Driver.exe and retained logs/fixtures; the archive is not another active checkout.
Use `scripts/workspace_paths.py` for historical path resolution.

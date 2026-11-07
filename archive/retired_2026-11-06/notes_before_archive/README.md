# DriverSF V29s research workspace

Start with `START_HERE.md`, then `NEXT_ACTION.md`. This workspace contains the current 150-request experiment, a matched 45-request control, historical diagnostic ASIs and logs, and an archived detailed handoff. It is an active working tree; do not assume untracked files are disposable.

- `DriverSF_V29s_NATIVE_TRACE.slnx` opens the current 150 diagnostic project, `DriverSF_V29s_LOD_TRACE.vcxproj`. Use Release|Win32; see `START_HERE.md` before building.
- Staged 150 and 45 test ASIs: `dist/MAIN_150/` and `dist/CONTROL_45/`. Install exactly one traffic ASI per game run.
- Full historical handoff: `archive/notes/NEXT_ACTION_full_2026-11-04.md`. Older project definitions: `archive/projects/`.
- Optional background: `reference/` and `dist/forza_package/research/`. Read only the specific file needed for a task.

The present question is where an invisible vehicle body waits after its first-stage spool request: queued CHNK work, later graphics resource/upload work, or a world-render decision. No queue or pool limit should be changed without identifying that stage.

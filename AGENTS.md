# Codex/Visual Studio work rules ? DriverSF V29s

Read only `START_HERE.md` and `NEXT_ACTION.md` first. **Do not automatically scan the full workspace, git history, reference/, archive/, old logs or user folders.** A scoped inventory/hash check is appropriate when the user explicitly requests workspace organization.

- Work on the latest concrete task or supplied log. Do not treat an archived experiment's next step as the current task. The current working baseline is ARENA_REUSE_R1; its verification scope and open questions are in the two handoff files.
- The broader goal remains 150 READY/drawable resident vehicles. Preserve 150 requests /160 slots and ED/EF sidecars. Map/request/spool counts do not prove READY/drawable. `0059F3A0` is a spatial query.
- Do not build, run the game, install dependencies, web-search, benchmark, change request counts or launch debugging tools unless specifically asked. Perform focused checks appropriate to an authorized analysis/verification task. Analyze supplied logs before proposing a patch.
- Preserve uncommitted work. Do not replace production source, restore old commits or perform broad cleanup. NEVER run `git clean -fdx` or `git reset --hard`. Authorized archiving must retain original bytes/paths in a verified manifest and avoid overwriting unrelated files.
- Use Release / Win32 only for the active `.slnx`, which contains one ARENA_REUSE project. Do not build retired variants by default. Enable exactly one traffic ASI in the game folder.
- Some historical evidence remains as a dependency of current checks. Use `scripts/workspace_paths.py` to locate archived baseline files. Do not remove prerequisites merely because their names mention an older test.
- On opening, report the current state briefly and wait for a concrete task/log if none was supplied. Historical project paths are restore-first material, not active build entry points.

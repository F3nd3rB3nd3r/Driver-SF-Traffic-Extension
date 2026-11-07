# Codex/Visual Studio work rules — DriverSF V29s

Read only `START_HERE.md` and `NEXT_ACTION.md` first. **Never automatically scan the full workspace, git history, reference/, old logs, or user folders.**

- The goal is 150 READY/drawable resident vehicles; today's task is **compare two V29s logs when the user supplies them**, not building V29t or patching game behavior.
- Do not build, run the game, install dependencies, web-search, benchmark, change request counts, or launch debugging tools unless specifically asked. One focused verification or analyzer command is okay when the user explicitly requests analysis/testing.
- Keep 150 requests / 160 slots, with ED/EF sidecars. Do not infer READY or drawable from map count. `0059F3A0` is a spatial query, not a vehicle-READY marker.
- Do not replace source, restore old commits, or execute cleanup commands. NEVER run `git clean -fdx`, `git reset --hard`, or overwrite a user's uncommitted work.
- On first opening: read `START_HERE.md` and `NEXT_ACTION.md`, report current state briefly, then stop until a concrete task/log arrives. If given logs, analyze before proposing any new probe or patch.
- Use **Release / Win32** only for the active `.slnx`. Do not build every solution or both test variants by default. Exactly one traffic ASI in the game folder at a time.

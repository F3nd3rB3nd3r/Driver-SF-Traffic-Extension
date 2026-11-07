# CALLBACK512_R1: challenge-switch test

Use `DriverSF_V29s_160Capacity_150Request_CALLBACK512_R1.asi` as the **only enabled traffic ASI**. Replace the previous traffic ASI in your usual game setup; keep a backup outside the loader's enabled ASI path. This artifact is staged here; it has not been installed into the game by Codex.

This build includes CHAPTERS_R1 and all inherited150-request/160-slot/ED+EF/audio/indexed/cleanup/pending fixes. The new change expands each of21 native callback lists from64 to512 nodes and records counts/integrity. It does not change traffic weights, vehicle IDs or audio pool sizes.

1. Load your usual save and briefly check vehicle bodies, engine/tire/siren or horn sounds, and approximate driving FPS.
2. Repeat the previously crashing Challenge sequence, including Gone!, President's Run and Tanner's Day Off where available. Note their order and whether each finishes loading with the appropriate traffic and mission vehicles.
3. If transitions work, return to the main menu normally, reload the save and repeat a few transitions. Then exit normally.
4. Copy `%TEMP%\DSFV29s_callback512_<PID>.log` into this `dist/MAIN_150` folder. Report the sequence and any silent crash, diagnostic exit, freeze, audio/body change or FPS change. If a new dump is produced, keep it with the matching log.

Expected startup: `DSFV29s_CALLBACK_START revision=1 status=1`, `groups=21`, `testCapacity=512`, `hooks=3`. `CHAPTER_START status=1` remains present. A `CALLBACK_EVENT kind=redirect` or a group `peak`/`issuedPeak` above64 establishes that the extra storage was exercised. No `CALLBACK_FATAL`, invalid/full/guardBad/concurrent counts are expected. Independent monitor snapshots can catch nonzero depth while work is running; that alone does not indicate failure.

This test deliberately exits with code29C1 and a `CALLBACK_FATAL reason=...` line if it detects unsafe selector, list corruption, capacity exhaustion, concurrent access to the same group, or recursive polling of that group. Same-thread insertion during a callback is supported. Such an exit is useful diagnostic evidence; preserve the log rather than raising another limit. Logging runs once per second for groups that were used, with one redirect event per group, rather than printing every callback.

Native isolated checks and compiled inherited verification passed. Live challenge stability, real demand, performance and normal-exit behavior remain to be established by this run. The earlier CHAPTERS audio setup failures remain separately tracked.

# ARENA_REUSE_R1: repeated Challenge test

Use `DriverSF_V29s_160Capacity_150Request_ARENA_REUSE_R1.asi` as the **only enabled traffic ASI**, replacing CALLBACK512_R1 in your usual game setup. Codex has staged this file here, without installing it into the game.

This build keeps the chapter-independent150-request/160-slot setup, callback512 and all previous audio/indexed/cleanup/pending fixes. It repairs two older allocation policies:

- Scratch work uses the already expanded150-entry native stack frame. It no longer consumes a new external arena at the obsolete50-entry boundary.
- Persistent spooler backing is reused for the same owner; distinct owners receive their own process-lived storage without an eight-owner registry limit. Recycled nodes are preserved.

Test sequence:

1. Load your usual save; check car visibility, engine/tire/siren or horn audio, and approximate FPS.
2. Start at least **six Challenges**, returning to free drive between starts. Include Gone!, President's Run and Tanner's Day Off where available. This should cross the previous ninth-setting-request failure point. Note the order and any incorrect mission vehicles/traffic.
3. If those work, return to the main menu normally, reload the save and try another Challenge. Exit normally afterward.
4. Copy `%TEMP%\DSFV29s_arena_reuse_<PID>.log` into this `dist/MAIN_150` folder. Report whether transitions/menu/reload/exit worked, plus any audio/body/FPS changes. Keep a matching dump if one is produced.

Expected startup includes `ARENA_START revision=1 status=1`, `scratchPolicy=expandedFrame150`, `scratchExternal=0`, plus the existing successful callback/chapter records. `ARENA_STATUS scratchArenas=0` is expected. Spool arenas count distinct owners rather than repeated setting requests. `spoolReuses=0` can be normal when the native list is recycled without a constructor reset. `scratchPeak` is sampled before insertion, so149 is normal for150 writes.

The unsafe-boundary exits remain protective, but the older guard paths now write `DSFV29s_LEGACY_FATAL` with `reason`, `exitCode`, owner/cursor/counts and arena usage into this same file. New callback/pending/cleanup guards retain their own fatal records. Preserve the log if the game disappears; this should make a guard exit distinguishable from an unlogged failure. No fatal marker should appear in a successful run.

The isolated native test reproduced the old ninth-use exhaustion and passed64 repaired150-entry repetitions, owner resets,16 distinct owners, recycling and bridge register-state checks. Live challenge stability and the actual cause of run23636 remain to be verified.

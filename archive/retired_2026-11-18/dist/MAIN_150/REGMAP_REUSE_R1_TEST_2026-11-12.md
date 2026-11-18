# REGMAP_REUSE_R1 live test

**Superseded2026-11-14: R1 blocks startup. Use REGMAP_REUSE_R2 and its
REGMAP_REUSE_R2_TEST_2026-11-14.md guide. This file is historical evidence.**

Use `DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R1.asi` as the **only
enabled traffic ASI**. Replace the previous traffic ASI; do not load both.
The artifact here is staged, not installed by the agent. Keep the same AI
asset/settings and recordings for this comparison.

Release/Win32;252928 bytes. SHA256:
`7bec339a6bc22d3ba599fd8533863604d20da23a4ed82add357e59e724b93f9b`.

This adds stable per-address definition-map storage and observes native
construction generations. It removes the four-owner directory cap and
recognizes same-address reconstruction. It retains512 external map nodes,
the snapshot150/256 and paused replay completion fixes,150 requests/160
graphics slots/ED+EF, and all existing audio/parked/queue capacities.

1. Start a fresh game process and load Story. Check rendering, audio and
   parked variety during a short drive.
2. Enter Replay, try play/pause/resume/seek, then return to Story. Repeat the
   Story/Replay round trip several times with some driving between entries.
3. Return to the main menu, reload Story and repeat. Aim for at least six
   world loads total, including the transition that ended run19868. Challenge
   entry/return can be included in the same sequence.
4. Quit normally. Report the transitions completed, any loading delay or
   exit/freeze, audio/rendering behavior and rough FPS.

Copy `%TEMP%\DSFV29s_regmap_reuse_<PID>.log` into this folder afterward.
If the game exits, retain the full log. If it hangs, a matching Task Manager
dump is useful; a dump is not necessary just to read a new explicit fatal.

Expected startup: `REGMAP_START status=1 hooks=2 fixedOwnerLimit=0`, plus all
inherited startup successes. `REGMAP_STATUS` reports constructs, reconstructs,
owners, redirects, reattaches, recycled uses and externalHigh. A normal run
should have faults0/allocationFailed0 and no REGMAP_FATAL/LEGACY_FATAL.
Owner count depends on heap reuse; six loads need not mean six distinct
addresses. Reconstructs/reattaches prove old addresses were exercised; owners>4
proves the old owner-count cap was exceeded. The log must be interpreted
alongside the observed gameplay result.

The guard still terminates on a real invalid cursor/count/node/guard, backing
exhaustion or allocation failure. A new REGMAP_FATAL prints the exact reason
and actual map/cursor/count/backing/generation values instead of default zeros.
Do not disable that guard if another fault occurs.

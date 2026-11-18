# REPLAY_PAUSE_R1 run19868: explicit vehicle-definition map exit

The user reports that the game closed like a process exit. The supplied
`dist/MAIN_150/DSFV29s_replay_pause_19868.log` confirms an intentional ASI
termination, rather than merely suggesting it from the visible outcome.

Log size: 16173780 bytes. SHA256:
`03e31d31d66c7e5328008faf3c7b0932072e22a93c7c7cc516ee8c2cc7d82cfd`.
Final line48332 is complete:

```text
DSFV29s_LEGACY_FATAL reason=vehicleDefinitionMap exitCode=00000031 tick=462641 owner=2EB9754C cursor=00000000 active=0 recycled=0 spoolArenas=4 scratchArenas=0 cloneArenas=1 invalid=0 guardBad=0 allocationFailed=0
```

## Confirmed termination path

`src/V29sNativeLoadingTrace.inc`, `DiagSPrepareRegMap`, hooked at0086BE5F,
protects the vehicle-definition registry's45 inline24-byte nodes. The
extension has a static registry of **four distinct map addresses**, each
with512 external nodes. The inspected code has no owner removal/recycling.
When its fault counter becomes nonzero it invokes
`LegacyExit(0x31,"vehicleDefinitionMap",map)`. `src/V29sArenaReuse.inc`
writes the observed fatal and calls `ExitProcess(code)`.

Possible fault branches are: no free owner entry at the inline-end redirect;
external cursor reaches the512-node end; or a registered owner's cursor is
outside/misaligned with its external storage. This is a sticky fault counter.
The log does not distinguish those branches, so four-owner exhaustion is a
strong lifecycle candidate, not a proven fifth-owner count.

The fatal's cursor/active/recycled zeros are omitted argument defaults, not
captured map fields. Its spoolArenas4 counts a different registry. Its
invalid/guardBad/allocationFailed fields are arena-reuse counters, not
`g_sRegMapFault`. None rules out this map failure. REG_MAP diagnostics are
absent in this log.

## Replay completion result

The new startup passed. The paused completion service was exercised twice:

| Tick(ms) | Import epoch | Native hold then completion | Clock bits |
|---|---|---|---|
|219078 /219141|1|loading1 ->0, lifecycle5 ->2|3F666667 unchanged|
|419110 /419172|19|loading1 ->0, lifecycle5 ->2|3F644445 unchanged|

Final pause counters:37124 updates,33 imports,13 attempts,2 completions,
11 native readiness holds;0 owner changes/missing imports/snapshot skips/
reentrancy/mark drops. Pause and zero rate remained unchanged in both logged
completion events. These are actual native loading completions; they do not
alone certify user playback controls or every replay transition.

Snapshot counters:49 captures/saves,33 loads/imports/restores, invalid0,
activeTemps0.40 accepted request phases have complete canonical150 rosters;
16 have a recorded all150 native-ready sweep. Sweeps are sequential and do
not prove simultaneous drawable bodies.

At the final periodic sample462469, replay mode3/loading1/transition2/
lifecycle4 was loading a world, rather than the prior mode2 paused replay
stall. Simulation/replay pointers had changed. Pending and cleanup were
empty with balanced walks/drains. Audio setup364 successful,0 failures,
registry full/misses0; no audible outcome supplied. Parked invalid/guard0.

## Next work

Check the vehicle-definition map's native create/reset/destroy and node
consumers. Replace the fixed four-owner bookkeeping with safe per-owner
storage and validated reset/reuse, retaining cursor/node bounds protection.
Add branch-specific fatal telemetry (actual cursor, owner count, used/end,
and fault reason). Do not simply disable the exit or raise unrelated limits.
The exact branch is unproven without those values or matching captured state.
A new dump is not needed to establish this explicit termination path.

Both required analyzers ran. Baseline telemetry checks pass, while the
callback analyzer correctly reports no_fatal=false and overall integrity
false because of LEGACY_FATAL. Do not report this session as a clean pass.
Outputs: `replay_pause_run19868_2026-11-12/baseline.json` and `callback.json`.
Only analysis/handoff files changed; no source, ASI, build, installation or
game execution in this analysis.

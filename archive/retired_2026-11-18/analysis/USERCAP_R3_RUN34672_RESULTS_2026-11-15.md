# USER CAP R3 run34672: saved Replay -> Story -> menu/quit passes

Input: `dist/USERCAP_R3/DSFV29s_usercap_r3_34672.log`, 10991163 bytes,
35187 parsed records; SHA256 `5bac2e5abea2b6aac7b0476bb31e475edf4e5ac7fe9f134b57100ffe0686905f`.
Four analyzer outputs, lifecycle bands, identity and summary are retained in
`analysis/usercap_r3_run34672_2026-11-15/`.

User reports main menu -> Replay -> watch -> main menu -> Story -> one chase ->
main menu -> quit game. Telemetry independently shows Replay playback first,
a cleared frontend interval, a different Story world, then a cleared frontend
at the end. User provides the quit step; the log has no definitive OS-exit marker.

## Conclusion

This supplies the previously requested R3 Replay-to-Story re-entry and reported
normal-quit coverage at the150-car defaults. Combined with36508, the two runs
cover both Story -> Replay -> menu and Replay -> menu -> Story -> menu/quit.
The R2 loading crash does not recur. Keep R3/default INI as the current150-car
baseline; no new ASI or capacity increase is indicated by this test.

Generic configuration/integrity, baseline and reader/client analyzers pass,
including the zero-audio-setup-failure check. No fatal, guard/invalid state,
allocation failure, rejected reader request, dropped baseline/client record or
sampled memory snapshot failure occurs. As in36508, the historical callback
analyzer reports its old `expandedFrame150` versus R3 `expandedFrame` label
mismatch. R3 startup metadata matches the current installer, and arena integrity
checks pass separately; this is a parser-schema difference rather than a new
runtime fault. Original analyzer outputs are preserved unchanged.

## Lifecycle and restored rosters

Periodic bounds (ASI elapsed seconds, not exact UI transition times):

| Phase | First..last sample | Recorded evidence |
|---|---|---|
|Replay loading|43.297..51.344|mode3/loading1, simulation1BECED30, recording3C4CF1D8|
|Replay playback|52.344..287.641|mode2/loading0, advancing replay clock|
|Frontend|289.656..291.656|simulation/recording0|
|Story initialization|305.719..317.781|new simulation3C595880, recording2E0A6BC0|
|Story recording|318.797..392.156|mode1/loading0|
|Final frontend|394.156..398.172|simulation/recording0, worldState2/worldStatus2|

Eleven complete150-ID phases are accepted. Ten reach150 READY; the first Story
loading request (phase10) is superseded at143/150 and the replacement phase11
reaches150/150. No model is left as an unresolved terminal request in the final
phase. Native named traffic hashes AE88E83B and47EB4D0E derive18->150 from the
active asset; replay restore requests preserve the recorded150 IDs.

Replay restores/imports8 snapshots; Story then captures/saves16 snapshots.
These counters are internal operations, not counts of user mode entries.
No invalid/deferred/active temporary state remains. Retained-reference peak0
means nonzero retained-reference restore is still not live-exercised.

The native loading-completion path holds5 times, completes once at52000ms,
and changes loading1->0/lifecycle5->2 while pause1/rate00000000/clock3F666667
remain unchanged. This does not force unpause or playback speed.

## Audio and capacity observations

All550 audio setups succeed, compared with36508's12 recovering failures.
Registry peak150/192,0 profile misses/full/key/instance/guard errors.
Shared default audio peaks at5,479,896 bytes (~5.23 MiB) against64 MiB with0
sampled pressure events. No emitter/slot/16-event refusal or audio allocator
failure is recorded. This is clean telemetry, not an independent listening test.

Pending-map peak154/512 and cleanup peak52,143/262,144 exceed their original
native sizes, so those extensions are exercised. Pending54 walk begin/end
pairs and cleanup36,819 drain begin/end pairs balance; both current counts
finish0 with0 guard/full/invalid/drain errors. Two definition-map owners each
use45 inline+105 external nodes, with0 faults. Neither reconstructs at the same
address; that particular lifetime case remains unexercised here.

Parked storage rebuilds7 times over2 owners, retains150 candidates and completes
4,510 selections (4,165 beyond the original50-entry prefix), without invalid,
guard or concurrent state. The body-render probe records only7 calls, with0
unmapped/mismatch observations. Its narrow call coverage is insufficient to
certify general visibility; READY likewise does not certify drawable pixels.
No separate FPS/render/audio/smoothness observation was supplied for this run.

## Replay waits and comparison with36508

Reader backing constructs/destructs5 times and reuses4 times with0 faults,
fallbacks, unmanaged calls or rejected reads. All replay-client busy intervals
finish: one1.401-second context-ended interval occurs during initial loading,
two further loading intervals clear, and10 playback intervals clear.
Playback intervals total197.959ms across clients, with a maximum42.295ms.
These durations include polling/scheduler gaps and are not a direct FPS metric.

All10 playback intervals occur at the same native clock/client positions as
36508:82.225,84.942,111.117,115.542,118.200,154.850 and157.417 seconds (some
positions involve two clients). Durations differ slightly. That repeatability
is consistent with recorded event/traffic boundaries in the saved replay,
rather than a progressively accumulating shortage. This is an inference;
no branch-level cause or perceptual smoothness is established. Native replay
clock values need not equal displayed timecode. No prolonged unresolved
playback-loading interval is captured.

## Remaining scope

The basic requested default150 lifecycle sequence now has live coverage in both
directions plus the user's normal exit. Keep R3 and current values. Larger or
non-default INI configurations still require matching asset rosters and live
validation; their existing offline fixtures are not gameplay certification.
Same-address reconstruction, nonzero retained pairs and exhaustive chapter/
mission coverage remain separate scopes. The earlier transient audio failures
remain an unexplained historical caveat, not a persistent failure in this run.

Only analysis reports and handoff notes were changed. No production source,
ASI/INI, game/assets/saves, build, debugger or live game action was performed.

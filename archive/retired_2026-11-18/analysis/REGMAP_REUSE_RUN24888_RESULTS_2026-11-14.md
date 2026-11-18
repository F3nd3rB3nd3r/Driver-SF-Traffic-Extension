# REGMAP_REUSE_R2 run24888: lifecycle pass, replay-motion hitching remains

The log verifies both corrected definition-map lifecycle paths in a150-roster
workload. The user confirms correct traffic variety/rendering/audio, repeated
Story/Replay/main-menu transitions and normal exit. The user subsequently
qualifies playback smoothness: during normal uninterrupted playback, loading
indicators appear intermittently, rendering stays around100FPS, but recorded
replay motion feels closer to50FPS/stutters. These FPS figures are user
estimates, not measured frame timings. Do not call replay performance resolved.

User's approximate transition sequence:
Story -> Replay -> main menu -> Replay -> Story -> main menu -> Replay ->
main menu -> Replay -> main menu -> Replay -> main menu, then normal quit.
Do not infer an exact replay-entry count from internal import counts or this
remembered sequence.

## Log identity and overall checks

`dist/MAIN_150/DSFV29s_regmap_reuse_24888.log`:27736174 bytes,80640 lines,
complete final newline. SHA256:
`0f185750f247589fb642a5c9da5b7677a0f6396e4c3511d5e2f76fe86cc4ade2`.
Last tick648563ms =10m48.563s. No fatal/installer refusal is recorded.
All required baseline150 and callback/inherited structural checks pass.
R2 startupCheckRevision2 is present. The final simulation/replay pointers
are null, consistent with menu teardown; normal OS exit is confirmed by the
user, not proved by a final-exit hook.

## Both map-lifetime failure paths exercised live

| Evidence | Result |
|---|---|
|Distinct map addresses|5, exceeding the old4-entry directory.|
|Constructors|6, including one reused-address generation2.|
|First external redirects|6; each crosses the45-node inline boundary.|
|Same-address reattachment|1; stable backing reused for generation2.|
|Insertion checks|900 =6x150 fresh-key checks in this workload.|
|Peak external issued|105, consistent with45 inline +105 external =150.|
|Faults / allocation failures|0 /0.|

The fifth distinct map3D40BAEC is constructed at325813ms and redirects into
its own backing at337532ms. The original four-record directory would have
no unused entry after the four prior distinct owners redirected.

Map31A3249C, previously constructed at222954ms with backing01DD001C, is
reconstructed at473204ms as generation2. It starts with native inline cursor
31A324A8/counts0, reaches the boundary and reattaches **the same** backing
01DD001C at491172ms. This exercises the same-address path the old guard would
reject on its first fresh insertion. It does not merely show a startup marker.

This is live evidence for both specific corrections in the tested sequence,
not proof that every possible owner lifetime or multithreaded case is covered.

## Traffic, replay snapshot and inherited capacities

-8 named setting requests expand to150, with no assetMismatch/refusal.
  Chapter1/frequency0 and Chapter7/frequency0 are exercised. Actual chapter-save
  or mission-required identity coverage is not established by setting names.
-87 accepted request phases have complete canonical150 rosters:8 named and79
  replay-restoration requests.52 phases have a recorded all150 native-ready
  sweep. The others lack such a sweep in their sampled lifetime; they are not
  automatically missing models. READY remains sequential, not atomic drawable
  coverage.
-46 snapshot capture/save pairs and79 import/restores all have150 IDs,
  retained0, hash5CBDFFEA, id50=181 and has181=1. No invalid data; activeTemps0
  at the end.79 restores do not mean79 user replay sessions. Retained>0,
  cached captures and deferred imports remain unexercised in this run.
- Paused native completion service runs four times, with20 held attempts and
  no missing import/owner/cache/reentrancy/mark-drop faults. Each completion
  clears loading1 ->0 and lifecycle5 ->2 while preserving pause/rate/clock.
  The four logged first-held ->completed intervals are62,62,47,47ms; these
  intervals are not total world-load durations or playback-frame timings.
- Parked51 builds, peak150 candidates,2644 successful selections,2611 beyond
  the original prefix;0 empty/invalid/guard/concurrent results. User confirms
  variety; the sampled unique-selection gauge is not150-model visual proof.
- Audio900 stores/900 removes,733 setups successful,0 setup failures/misses/
  registry-full/fallback misses; user confirms audio. No extrapolation to every
  model or environment.
- Cleanup594684 appended/drained,56453 balanced drain starts/ends, final0.
  Pending922 stores, peak154,135 balanced walks, final0. No sampled capacity/
  allocation/guard errors. Callback extra capacity is not shown exercised.

## Remaining replay-motion/loading issue

The user's clarification distinguishes a replay advancement/presentation
issue from general rendering FPS loss. Intermittent loading during normal
playback remains a real unresolved observation despite successful transitions.

Existing telemetry has **one-second** replay status samples, no per-frame
duration, requested playback-rate history, commanded seek target or exact
wait start/end. All sampled mode2 replay statuses have loading0; one sample
at620407ms has snapshotState2. Brief waits between samples are not excluded.
There is no basis to claim the loading indicator never appeared or that all
replay-motion hitches are fixed because these samples look idle.

There are79 full-roster restores, always the same150-ID hash. Some5-second
wall-time windows contain up to8 restores; sampled replay clocks sometimes
jump rather than advance continuously. Other intervals advance near1:1,
e.g.535907..547969ms. Some earlier intervals advance near2x and some jump
backward. Controls/seek target/rate are not logged, so these data cannot
attribute every jump to loading or treat the whole session as constant1x.
The user specifically reports the observed loading hitches during normal
playback, rather than only while operating controls.

ReplayRestore currently issues the recorded traffic request on each restore,
then preserves native reference/extra-state order and enters phase2. Repeated
identical requests are a candidate for timing investigation; **they are not
proof of disk reads, reloads, or redundant work that can safely be skipped**.
The log has no measured duration for the request or its subsequent readiness
waits. No saturated queue or exhausted map/audio capacity appears here.

Next scoped work should trace replay snapshot dispatch and the native wait/
time-advance gates, then add event-based timing if a new probe is requested:
import/restore/request duration; phase/wait transitions; replay clock versus
wall time; requested rate/seek target; resource readiness and pending counts
on the **game thread**. Aggregate timing/log only on boundaries, without
per-frame file writes or monitor-thread engine calls. This should distinguish
snapshot/streaming waits from replay scheduling/rate/interpolation and logging
overhead. Do not raise capacities, bypass readiness or deduplicate restores
based only on equal roster hashes.

No source/ASI/build/game changes in this log-analysis turn. Current R2 is a
successful tested baseline for the specific map exits and this transition/
normal-exit workload; replay-motion smoothness is the open follow-up.

Evidence: `regmap_reuse_run24888_2026-11-14/baseline.json`, `callback.json`,
`replay_timing.json`, analyzer console logs and `summarize_replay.py`.

# GENERIC_R1 run10556: same-process Story/menu/Replay/menu sequence passes

Input: `dist/MAIN_150/DSFV29s_generic_limits_10556.log`,9,043,702 bytes,
29,806 records. Identity, four analyzer outputs and state bands are in
`analysis/generic_lifecycle10556_2026-11-15/`.

User reports **main menu -> Story -> main menu -> Replay -> main menu -> quit**.
The telemetry shows Story recording, destruction of that world, construction
of a different world for Replay, import/restore and playback, then destruction
back to the frontend. This extends24080's fresh-process saved-replay test with
same-process Story-to-Replay lifecycle coverage.

All generic, baseline, callback/inherited and replay reader/client checks pass.
Keep GENERIC_R1/default limits. No new probe, capacity change or ASI is indicated.
Only analysis artifacts and handoff notes changed; no production source/INI/
ASI/asset/save modification, build, game/debugger launch or live attachment.

## Configured dimensions and accepted requests

AssetWeights/default config remains150 requests,160 model/graphics slots,
192 audio-model entries,2048 global audio slots,64MiB shared audio,1MiB replay
reader and256 retained references. No compiled car list.

Four accepted150-ID request phases: three asset-derived18->150 requests and
one preserved150 request from the replay restore path. Initial phase1 is
superseded at142/150; phases2/3/4 each reach all150 READY, with no rejected or
fatal request. READY is a sequential native-predicate sweep, not150 simultaneous
visible/drawable certification.

## Lifecycle evidence

Periodic replay status bands (`lifecycle_bands.json`):

| Phase | ASI elapsed seconds | Observed state |
|---|---:|---|
| Story initialization |24.219–36.282|simulation1B706C60, recording3C09AE40|
| Story recording |37.282–172.985|mode1;29 captures/saves across the Story period|
| Frontend/no world |173.985–197.141|simulation and recording pointers0|
| Replay loading |198.157–206.204|new simulation2F25FE30, recording2DEAFF00|
| Replay playback |207.219–345.125|mode2, loading0, native clock0.8917->116.8333|
| Frontend/no world |346.125–349.141|simulation and recording pointers0|

Times are periodic sample bounds, not exact UI transition times. The native
clock is not necessarily the displayed Replay timecode. The user supplies the
quit step; these logs have no definitive OS-exit event. No separate visual,
audible, FPS, playback-control or full-duration observation was requested or
independently established for10556.

The150-ID snapshot imports/restores at206672ms, with matching hash5CBDFFEA,
ID50=181 and extraFFFFFFFF. Final snapshot invalid/deferred/activeTemps counts0.
Retained-reference count/peak is0, so nonzero retained-reference restoration
remains unexercised by this log.

## Replay readiness-completion fix: exercised successfully

This workload exercises a branch that24080 did not:

- Eight candidates/attempts; native readiness holds seven times, then one
  completion. No owner change, missing import, snapshot skip, reentrancy or
  mark-drop failure.
- `nativeHeld` event at206797ms retains loading1/lifecycle5.
- `completed` event at206860ms changes loading1->0 and lifecycle5->2 while
  **paused1, rate00000000 and clock3F644445 remain unchanged**.

Thus loading completion respects the native ready gate and does not force
unpause, change playback speed or advance the clock in the recorded completion
event. This is direct telemetry for the existing loading-completion fix, not
a claim that manual pause/seek controls were independently tested.

## Replay reader and waits

One reader owner, five constructions/destructions, four backing reuses; final
state inactive. All managed boundaries use the configured1MiB backing.
1,298 read issues and1,295 commits,42,415,088 committed bytes across generations;
0 rejected reads, fallbacks, allocation failures, faults or unmanaged calls.
The difference between lifetime issue/commit counters alone does not establish
lost I/O across teardown/generation changes.

Three observed busy intervals, **all during loading**:

- Client7:1,420.537ms ending with `contextEnd` (context change, not demonstrated
  readiness recovery).
- Client7:38.692ms, clears in the following context.
- Client1:10.107ms, clears before loading completion.

Final client open/held counts0; no playback (`loading=0`) busy interval or event
drop/read-check/pair/schema error. This is stronger than zero observed long
waits, but does not independently certify pixel smoothness or FPS.

Reader space/wrap stops14,366/119 and minFree4 are prefetch/backpressure
observations, not visible-hitch counts or evidence to increase the reader.

## Audio, parked vehicles and teardown

- Audio:348 attempts,348 successes,0 failures.300 registration stores and300
  removals over the two world instances; registry peak150,0 overflow/guard
  faults. Prior22828 transient failures do not recur in this workload.
- Parked: two owners,three builds,150 candidates,4,682 selections, all
  successful;4,674 use the extended range.0 empty/invalid/guard failures.
- Resource registry: two owners/two constructions,300 checks,two redirects,
  externalHigh105,0 faults/allocation failures. **Same-address reconstruction
  is not exercised** (`reconstructs=0`); different owners are not proof of that
  reuse case.
- Pending: two generations/two redirects,308 insertions, peak154, final active0
  and recycled154 in the latest generation.0 invalid/full/allocation/guard faults.
- Cleanup: peak52,141,38,692 insertions beyond stock capacity across two stock-
  boundary crossings;234,253 drained, final active0, drain balance0 and matched
  starts/ends.0 full/guard/invalid/drain-mismatch failures.
- Callback integrity passes; no beyond64 callback extension exercise. No
  notification sidecar-copy exercise. Shared audio has0 sampled pressure events.

## What is covered and next

This passes the exact supplied menu/Story/menu/Replay/menu/quit lifecycle, in
addition to22828's activities and24080's saved-replay loading. It does not
include re-entering Story after Replay or repeated alternation, seeking,
same-address registry reconstruction, nonzero retained-reference restore or
non-default INI dimensions. Those are scope limits, not newly discovered faults.

The next planned configuration milestone is a non-default supported capacity
with an asset roster that fits it. A150-positive-weight asset must not be paired
with RequestCapacity below150: fail-fast is intentional in that case. Plan a
matched test rather than blindly lowering a limit against the current asset.
Widening beyond150 requests/160 physical slots remains separate authorized
static/implementation work. Await the user's next concrete task; preserve the
current candidate.

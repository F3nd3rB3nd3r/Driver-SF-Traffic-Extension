# USER CAP R3 run36508: Story/Replay/menu regression findings

Input: `dist/USERCAP_R3/DSFV29s_usercap_r3_36508.log`, 15366156 bytes,
48988 parsed records, SHA256 `14dff2c82ee046c7b44958e2ca1284b0f9bca21b2c26b868b871fd6d9ad3d2c5`.
Evidence and four original analyzer outputs are in
`analysis/usercap_r3_run36508_2026-11-15/`.

User reports Story with police chase, activities and dares, then Replay accessed
from Story, then return to the main menu. The roughly550-second ASI timeline
supports Story recording, a different Replay world, playback and return to the
frontend. Visual/audio/FPS/replay-smoothness and final quit confirmation were
requested asynchronously and remain pending at this report's creation.

## Main result

The R2 first-Story-load crash does not recur in this R3 session. Startup and
configured geometry match the unchanged150 default:150 requests,160 models,
192 audio-model entries,2048 global audio entries,64 MiB default audio,1 MiB
replay reader and256 retained pairs. No fatal, guard corruption, allocation
failure, invalid pending/callback/spool/definition state, dropped baseline event
or sampled memory-snapshot error is recorded. No new patch or increase is
indicated by these findings.

Baseline and replay-reader/client analyzers pass. The generic analyzer passes
all checks except its strict zero-native-audio-failure rule (12 failures below).
The historical callback analyzer reports only an arena startup-label mismatch:
it expects `scratchPolicy=expandedFrame150`, while R3 legitimately emits
`expandedFrame` with configured geometry and `newHooks=1`. The actual record
matches the current installer, and all arena integrity counters are0. This is
not a runtime arena failure; `r3_arena_schema.json` retains the explicit schema
validation separately rather than altering the historical parser's output.

## Requests, parked vehicles and exercised capacities

Thirteen accepted150-ID phases have complete rosters and no duplicate/malformed
request. Phase1 is superseded at149/150 during loading; phases2..13 each record
all150 READY. This is a sequence of predicate sweeps, not proof of simultaneous
visible/drawable residency.15,094 body calls are observed, with0 unmapped calls
and no reported phase draw mismatch.

Parked storage performs7 rebuilds across2 owners, all150 candidates;11,519
successful picks,10,218 from beyond the native50-slot prefix. No invalid/guard/
concurrent state occurs. The selected-value statistics remain evidence of the
picker, not independent proof that every spawned car was visible.

| Tracked capacity | Observed peak | Configured bound | Meaning |
|---|---:|---:|---|
|Audio model registry|150|192|No registry full/miss/guard or key/manager-directory overflow|
|Pending map|154|512|Beyond native64,2 constructor generations; drains to0|
|Cleanup queue|51361|262144|Beyond native32768; drains to0 with balanced52107 begin/end checks|
|Definition external nodes|105|512|45 inline plus105 external nodes for150 definitions;2 owners, no faults|
|Global audio lists A0/A1/B|97/53/60|2048 each|No ceiling hits; values are maximum observed update/insert counts|
|Hash extra nodes issued|25368|45000|Cumulative/lifetime-issued telemetry, not current live occupancy|
|Callback per group|1|512|No callback expansion beyond native64 exercised in this workload|
|Default audio used bytes|5489320|67108864|Sampled peak5.24 MiB;0 sampled pressure events|

Resource-collection count peaks at1268 and ends0; this is distinct from the
154-entry per-package pending map. Process-private-byte sampled peak is
1,012,645,888 bytes (~965.7 MiB); working-set sampled peak783,384,576 bytes
(~747.1 MiB). These are whole-process observations, not per-car asset costs or
a guarantee of future memory headroom.

## Audio caveat:12 failures recover

919 native setup attempts yield907 successes and12 failures. The failures are
two each for IDs196,197,201,205,226,299, first visible in the142.860-second sample;
the preceding141.860-second sample has0 failures. Exact per-call failure times
and sub-reasons are not recorded by the bounded detail samples.

Every affected model records additional successful setups after the final failed
sample:8,9,4,8,6,15 successes respectively. Final per-model success totals are
8,11,6,10,8,15. Thus this log does not show these models remaining permanently
silent. All six have0 profile misses/bank-not-ready observations, and no audio
registry/global-slot/emitter/16-event refusal, allocator failure or sampled
shared-budget pressure is recorded. Native setup failure is real telemetry,
but no limit-exhaustion explanation is supported here.

Earlier GENERIC run22828 likewise had native setup failures followed by recovery
(123 failures across19 models); this is a similar recovery pattern, not identical
failed model membership or proof of an identical underlying cause. Audible
results still require the user's observations. Preserve the12 failures as an
open diagnostic caveat; do not erase them or increase pools from this count alone.

## Replay and lifecycle

Periodic samples bound Story mode1 at37.250..277.547 seconds. Replay loading is
present around299.672..308.719 seconds; playback mode2/loading0 spans
309.719..542.860. World/recording pointers clear at543.860, then frontend
worldState2/worldStatus2 continues through549.891. These are sampled bounds,
not exact UI timings. There is no definitive OS-exit marker.

53 snapshot captures/saves and8 import/restores complete with150 IDs and no
invalid/deferred/active-temp state. Eight internal restore operations do not
mean eight user Replay entries. Retained-reference peak remains0, so nonzero
retained restoration is not live-exercised.

The readiness-completion path holds6 times and completes once at308781ms.
Loading/lifecycle change1->0 and5->2 while pause1, rate00000000 and clock3F666667
remain unchanged. It respects native readiness without forcing playback speed.

The1 MiB reader constructs/destructs5 times and reuses its backing4 times,
with0 faults/fallbacks/rejected reads. Replay client pairs are complete; no
unfinished busy interval remains. One1.408-second context-ended interval occurs
in initial loading, not normal playback. Two further loading intervals clear.
During playback10 observed waits all clear, lasting0.193..39.437ms each,
227.163ms summed across clients (not a sum of independent FPS loss). Paired
client waits can occur at the same replay position. They cluster around native
replay clocks82.225,84.942,111.117,115.542,118.200,154.850 and157.417 seconds;
these clocks need not equal the displayed timecode. No prolonged unresolved
playback-loading stall is captured. Smoothness/controls still need user context.

## Next scope

Retain R3 and these defaults. This is useful live coverage for the Story-load
fix,150-car resource handling and Story-to-Replay-to-menu teardown. User
rendering/audio/FPS/smoothness/exit confirmation remains pending. Story re-entry
after Replay, same-address registry reconstruction, nonzero retained restore
and non-default larger geometries are not established by this session.
No ASI/INI/source/game/asset/save changes, build, debugger or game run were made
for this analysis; only reports and handoff notes were updated.

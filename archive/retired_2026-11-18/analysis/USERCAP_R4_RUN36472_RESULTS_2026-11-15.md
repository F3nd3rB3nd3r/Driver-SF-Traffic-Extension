# USER CAP R4 run36472: requested Story/menu/Replay/re-entry lifecycle succeeds

Input: `dist/USERCAP_R4/DSFV29s_usercap_r4_36472.log`,12,195,086 bytes,
42,746 parsed records, SHA256
`9b37b1c3d69bad00569608fc6a09687dc04300ab710a06902df2e8c4fc48da3d`.
Last recorded tick532109ms (~8m52s). Evidence and unmodified analyzer outputs:
`analysis/usercap_r4_run36472_2026-11-15/`.

The user states they followed the requested Story/activity -> menu -> Replay
watch -> menu -> Story/activity -> menu -> in-game quit sequence. The sampled
timeline independently supports those mode changes and final frontend teardown.
The final OS exit itself is not an instrumented event.

## Conclusion

The planned basic R4 lifecycle check succeeds for the current152-model expanded
AI with160 request capacity and170 model slots. Story-to-Replay, nonzero retained
reference restoration, Replay-to-Story and final main-menu teardown are exercised
without fatal/queue/guard/allocation errors. Together with20408's repeated
activity transitions, this supports retaining R4/current INI. No new ASI or
capacity increase is indicated. Documentation/release preparation can be the next
user-selected task; this turn does not authorize a Git push.

There are recovering audio setup warnings, so this is targeted lifecycle success,
not a blanket zero-warning result or proof that every asset/effect is correct.

## Recorded lifecycle

Times below are periodic ASI elapsed-time sample bounds, not exact UI clicks.

| Phase | First..last(seconds) | Evidence |
|---|---|---|
|First Story recording|43.343..196.171|mode1/loading0, simulation1BF1EDF0, recording33DBD360|
|Frontend|197.171..213.265|simulation/recording0|
|Replay loading|215.281..222.312|mode3 then2/loading1, simulation1BF1EDD0, recording30285F40|
|Replay playback|223.312..369.000|mode2/loading0|
|Frontend|370.015..386.093|simulation/recording0|
|Second Story recording|398.171..529.093|mode1/loading0, simulation33058D78, recording2FEACFE8|
|Final frontend|530.093..532.109|simulation/recording0, worldState2/worldStatus2|

All13 accepted request phases have complete ID lists and reach full native READY
in a recorded sweep: seven152-ID phases and six18-ID phases. Named requests use
AssetWeights augmentation; replay-origin requests preserve their recorded IDs.
READY is a sequential predicate observation, not simultaneous drawable proof.

## Traffic queues, owner lifetimes and teardown

All three new queues start successfully with capacity512. They each observe
three distinct owner addresses:1B81E1C0,33F9C870 and34030AC8. Retirement allocates
three external buffers and redirects three times, with peak147 nodes and73
recycled insertions. There are512 retirement checks,48 retained-vehicle checks
and94 operation checks. Both20-byte queue peaks are13. All recorded failure and
guard counts remain0.

Generations3 with resets0 describes different owners, not same-address
reconstruction. External storage for the20-byte queues is not needed in this
run; their overflow/reconstruction coverage remains offline. The last detailed
traffic-queue samples are528.078s, just before final frontend teardown. Later
world and subsystem drain records provide teardown evidence; the traffic queue
samplers do not produce an exit snapshot.

The definition map likewise constructs three owners, redirects each once and
records528 recycled insertions, external high120,0 faults/allocation failures.
There is no same-address reconstruction/reattachment event.

Pending storage has three generations, peak163/512,490 recycled insertions,
524 balanced walk starts/ends and final active0. Cleanup peaks47129/262144,
crosses its original32768 capacity three times, drains502196 entries with50105
balanced begin/end pairs, and ends active0/depth0. All invalid/full/guard/drain
errors remain0. Callback, spool arena and parked integrity checks are clean.
Parked has three owners,10 builds,152 peak candidates and708 selections with51
beyond the original50-entry prefix.

## Replay restoration and reader

Replay captures/saves60 snapshots and performs four import/restore pairs:

| Restore tick(ms) | Traffic IDs | Retained pairs | Roster hash |
|---:|---:|---:|---|
|219828|152|7|CA9E3FA6|
|295265|152|6|CA9E3FA6|
|298218|18|12|CBDD7A32|
|354453|152|7|CA9E3FA6|

All imports/restores remain within the configured160/256 bounds. This supplies
live coverage for nonzero retained-reference restoration, previously absent
from20408. Peak retained count is12. Final deferred/active temporary state and
invalid counts are0.

Reader backing constructs/destructs five times and reuses four times, with0
faults/fallbacks/unmanaged calls/allocation failures. Its1MiB backing remains
retained by design. Client telemetry has no drops/read/clock errors and no open
busy interval. Six intervals clear; one initial loading interval ends with its
context, which is not counted as a readiness recovery.

Four loading0 busy intervals clear during playback:31.331ms,70.072ms,0.230ms
and32.230ms, totalling133.862ms across clients. They accompany snapshot/traffic
changes around native clock60.275/63.042/90.850s. These include scheduler/polling
time and do not measure visible FPS. No sustained unresolved loading interval
appears. The assisted pause-completion path records no candidates/attempts in
this run; native loading completion handles this sequence.

## Audio warnings and capacity

1021 setup attempts include937 successes and84 failures. The12 affected model
IDs are127,128,148,151,158,179,187,231,255,274,281,298. Every affected model has
a later sampled increase in successful setups after its last observed failure
increase. Recovery is observed per model; individual emitter recovery is not
established by cumulative counters.

Registry peak165/192,0 full/misses/guard/instance/key failures; global audio-list
peaks137/53/58 against2048,0 ceiling hits; emitter refusal/allocator failure and
object8/255-slot/16-event refusals all0. Shared audio peaks16,986,044 bytes
(16.20MiB) against64MiB with0 sampled pressure crossings. All1029 bank callbacks
report result1. These observations do not support another capacity increase.

The reason probe's eventOverflow2995 is the bounded512-entry diagnostic event
directory, not failed live audio allocations. Its detailed event attribution is
incomplete. Final audio registry current1 is retained as observed; its periodic
sample and cumulative event counters are not an atomic occupancy snapshot.

## Analyzer interpretation and remaining scope

The current generic analyzer passes configuration, geometry, rosters, every
monitored integrity/storage check and both new traffic-queue checks. Its only
false check is the strict zero-cumulative-failures `audio_setup`; the original
`recorded_integrity_pass=false` output is retained unchanged.

Historical BASELINE150/reader/client analyzers retain fixed150/160 expectations
and150-ID bounds. Their startup/bounds false values are schema mismatches for
this160/170 configuration with152-ID rosters. Current-schema checks separately
verify parked list IDs, reader/client configured startup and replay event bounds.
Reader/client runtime integrity, completeness and event pairing checks pass;
the old tools were not changed to force a pass.

This completes the requested basic lifecycle sequence for this configuration.
It does not certify an entire playthrough, every model/sound, larger rosters,
same-address owner reconstruction, expanded20-byte queue exhaustion or prolonged
sessions. No production source/ASI/INI, assets, saves or game installation were
changed; no build/game/debugger launch, commit or push occurred.

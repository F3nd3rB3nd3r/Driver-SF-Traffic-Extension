# REPLAY150_R1 run8944 ? paused replay loading softlock

## Outcome and supplied evidence

The user reports a softlock entering Replay after playing Story once. It does
not require a second Story/Replay cycle. The user also notes that recordings
can survive restarting the game. Recording file origin is not established here;
no saved recordings or assets were read, changed or deleted.

- Log:`dist/MAIN_150/DSFV29s_replay150_8944.log`,4796908 bytes,15793 lines,
  complete final newline, last tick199531ms. SHA256
  `307c131d0e19d72779db8695f2d190791864e1335eae8079ae92d8403207173a`.
- Task Manager dump:`dist/MAIN_150/Driver.DMP`,1161317834 bytes, PID8944,
  timestamp1794077329,50 threads, no exception stream. SHA256
  `231432983fcbf43390ce8f912086d63186e50aa21824eeae28022ad26350dba8`.
- Loaded traffic ASI is REPLAY150_R1 at`68AE0000`; PE timestamp1794075306
  matches the built artifact. DriverNGHook is also present; presence alone
  does not attribute the softlock to it.

Both required log analyzers and inherited structural checks pass, but the
user-visible loading softlock is real. Structural success does not mean
completion of the replay lifecycle.

## Snapshot correction remains intact

The log contains17 capture/save pairs and one import/restore pair, all with
150 IDs, hash`5CBDFFEA`, index50=181, has181=1 and retained count0. Four accepted
traffic requests all retain the canonical150 roster; the last originates from
ASI restore caller`68AE4D31`. Chapter status has3 named expansions, one expected
otherCaller and zero refusals.

The dump directly confirms the full private cache at`3BAD0000` for embedded
snapshot`307A0484`: both guards`29150ACD`, valid1,150 unique nonzero IDs,181 at
index50, hash`5CBDFFEA`. Retained count0. The original inline mirror still has
only50-ID storage by design; it must not be interpreted as the full array.

Final replay counters:17 captures/17 saves, one load/import/restore, invalid0,
activeTemps0, one reused temporary slot. Native snapshot phase is0, indicating
completion of this restore. Final baseline phase reached all150 native READY
predicates and was resampled31ms before the final log gauge. This is a
sequential predicate sweep, not150 simultaneously visible bodies.

## Persistent loading state

Across45 sampled records from152843 to199531ms (46.688seconds), the state stays:

| Field | Value |
|---|---|
|Replay mode|2|
|Loading flag|1|
|Lifecycle|5|
|Transition|0|
|Snapshot phase|0|
|Replay clock|0.8916667104 seconds (`3F644445`)|
|Event cursor|23ABA5F8, next event time0.9583333731 seconds|
|Pending traffic / retained / spool state|0 /0 /0|
|World state / returned status|2 /2|

The recording has data through64.7083358765seconds, so the clock is not at its
recorded end. User wait duration was not specified; the persistent interval
above comes from the log itself.

## Captured blocking mechanism

The simulation manager exists at`1B18FE40`. Its clock controller is
`1B18FFA8` (`manager+168`), registered in the live scheduler`053F89E8`.
It has pause byte`+20=1`, rate`+28=0.0`, scale`+2C=120.0`. Global requested
rate`00F2D3F0` and replay requested speed`replay+4E99C` are both0.
The other six registered clock controllers are not paused.

Static code and the captured data establish this chain:

1. Native`0053A7E0` marks a clock paused when its effective rate is zero.
   `00797520` applies that rate to the simulation controller;
   `0079DAE0` is the replay rate setter. Replay update`0073A090` has an explicit
   zero-rate path at`0073A644`. The dump does not identify the first caller
   that selected zero or its temporal ordering.
2. Scheduler`0085D820` checks paused clocks at`0085D8D4` and skips their virtual
   `+4` step dispatch. The actual dispatch at`0085D967` is therefore not run
   for this paused simulation controller.
3. The controller's native vtable`00E2CA70` has step callback`0053AA20`.
   Its tail at`0053AC03` enters`0073A070`; mode2 routes to`00738F20`.
   This replay step advances playback and contains the loading-completion work.
4. At`007390BE`, loading1 leads to readiness predicate`00738D20`; when it
   returns false,`007390D2` clears loading, then invokes the existing native
   completion work and updates lifecycle when its normal condition permits.
5. Evaluating those predicates from the dump gives **not busy**: all11 replay
   subscribers' virtual`+20` predicates return false; the traffic snapshot is
   phase0; native spooler checks pass; the world readiness function returns2;
   recording exists and transition0 passes. The data therefore permits the
   completion block, but its owning clock is paused and not dispatching it.

This is a concrete circular-wait mechanism for the remaining softlock: loading
completion depends on a replay step stranded behind the paused simulation
clock. The dump establishes the stuck state; the repeated log samples establish
its persistence. The precise origin/order of the pause is not recorded, so do
not attribute it to a particular Lua call, plugin, recording, or instruction
race without additional evidence.

The focused analyzer verifies that ten relevant native code spans in the dump
match the supported Driver.exe exactly, including scheduler, pause setter,
replay step/completion and readiness code. This is not a claim that every
instruction or every plugin hook in the process was audited.

## Other captured checks

Replay/request/baseline/parked locks are0, replay active temp count0. No recorded
fatal/exception/guard/allocation errors. Cleanup current0 with17348/17348
balanced drains and115516 appended/drained records. Pending current0 with
34/34 balanced walks. Audio has130 successful setups,0 failures/misses/full
reports. Stores300/removes150 is consistent with an active150-model session;
this was not a normal teardown and imbalance is not independently a leak.

Main thread4936 was sampled in a1ms job-group yield through`00BC8BB9` /
`00BAD7AD`. The recovered group`053FD770` has submitted1/completed1. This is
not evidence of an outstanding job or a persistent thread deadlock. The
monitor thread is normally sleeping; the render thread is in its presentation
wait. Those stacks do not justify blaming the GPU or a resource worker.

## Next correction, when authorized

Preserve the full snapshot correction and existing capacities. Target a
**pause-safe replay loading-completion service on the game thread**, using
an updater that still runs when simulation time is paused. Retain native
readiness, snapshot phase, owner/lifetime, mode and lifecycle checks. Reuse
native completion behavior only when its conditions pass. Record the paused
controller, readiness result, service invocation and completion result.

Do not force the clock to run, resume user playback, skip pending resources,
or call game services from the monitoring thread. A retained recording may
legitimately open paused; it still needs loading completion. A narrow guarded
completion fix plus telemetry is more appropriate than another capacity
increase. The first zero-rate caller/order can be instrumented if needed;
this dump alone cannot reconstruct that history.

No ASI, game code, project configuration, assets or recordings changed in this
analysis. No debugger, game process, build, dependency installation or web
search was launched. Existing REPLAY150_R1 artifact hash remains
`2ab7f763f216abfaf5091308f0adb3e428512038d2f0384432faa140aae12dd9`.

Evidence: `replay150_run8944_2026-11-07/baseline.json`, `callback.json`,
`summary.json`, `dump_analysis.json`. Reproducible offline dump analysis:
`python -B analysis/replay150_run8944_2026-11-07/analyze_dump.py`.
That script verifies PID8944 and the exact supported native image; it should
not be applied to a newly overwritten Driver.DMP without a fresh audit.

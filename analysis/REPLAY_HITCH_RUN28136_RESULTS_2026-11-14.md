# REPLAY_HITCH_R1 run28136: late subscriber readiness pattern

2026-11-14. Supplied log:
`dist/MAIN_150/DSFV29s_replay_hitch_28136.log`.
User reports the beginning was smooth and choppy motion appeared near the
end. User confirms a fresh replay from the new Story session, normal-speed
viewing until the end, then rewinding to compare earlier playback, and a
normal quit. The late control activity is therefore not an unexplained
automatic restore loop.

## Main finding

At approximately replay time **144.94–151.87 seconds** (2:25–2:32), a
subscriber readiness predicate repeatedly reports busy. There are29
completed observed busy-to-clear intervals, spaced at a median250ms,
lasting11.42–61.68ms, median22.30ms. This is a strong lead for the reported
late choppiness, not proof of the individual responsible subsystem or limit.

The busy predicate is `007379D0`, which calls subscriber virtual **+20**.
The ordinary replay-clock gate `00737A10`, subscriber virtual **+24**,
reports **zero busy results throughout the log**. Replay time continues
advancing during the late +20 intervals. These different checks must not
be collapsed into a claim that the replay was never waiting.

There are **no checkpoint requests, recording repositions, imports,
restores or replay150 traffic requests during the first playing segment**
at ticks255062–425250. Repeated same-roster restores are not the cause of
that segment's late symptom in this capture. No saturated capacity or
allocation/guard failure is demonstrated.

## Log and inherited integrity

-15110606 bytes,45438 lines, complete final newline.
- SHA256 `bf5f7b3d2af45f5105cddac06dbf6cf9526fefcaf6e3e2c4af8591e732c248c7`.
- Last recorded tick445390ms, approximately7m25s.
- HITCH_START status1, five hooks, QPC frequency10000000Hz.
- All HITCH status samples have zero dropped/thread/depth/nesting/QPC
  errors. All baseline150 and callback/inherited structural checks pass.
-29 accepted roster phases,8 recorded all-ready sequential sweeps;
  this is not atomic drawable/visual proof.
-36 captures/saves,26 imports/restores, all canonical150. Final temporary
  count0, retained peak0, invalid0. One paused loading completion succeeds.
- Two map constructions/redirects,300 checks, external peak105, faults0,
  allocationFailed0. No >4-owner or reconstruction exercise in this run.
- Parked632 successful picks,597 beyond original50, empty/invalid/guard0.
- Audio300 stores/removes,504 setups successful, no setup failure/miss/full.
  Audible correctness is not established solely by these counters.
- Cleanup248712 appended/drained,41041 walks balanced, final0.
  Pending308 stores, peak154,69 walks balanced, final0.

The log's clean teardown is consistent with the user's normal-quit report;
log integrity alone would not establish the OS exit reason.

## Compare preceding playback with the late symptom

These are complete one-second windows selected by their ending ticks;
per-window boundaries can include activity preceding the sampled state.

| Measure | Preceding10 windows, ticks402718–411718 | Late8 windows, ticks412718–419718 |
| --- | --- | --- |
| Recorded time at sampled endpoints |135.03–144.03s|145.03–152.03s|
| Native replay steps / clock advances |1200 /1200|960 /960|
| +24 clock-gate busy results |0|0|
| +20 busy results |0|153|
| +20 results with idle snapshot/traffic fallback |0|153|
| Snapshot/traffic pending-at-busy counts |0 /0|0 /0|
| Checkpoint/restore/request calls |0|0|
| Requested / applied rate |1 /1|1 /1|

The exact late wait endpoint range412000–419812 captures29 intervals; the
slightly narrower eight window endpoints capture28. All29 have phase0 and
pending traffic0 before and after. The native fallback gate is idle, so a
true +20 gate result in this state comes from its subscriber loop, rather
than the snapshot-phase/traffic fallback.

For example, the interval ending413390ms lasts61.6755ms while replay time
advances145.6532 ->145.7115. Other waits advance recorded time by one or
several1/120-second steps too. A busy-result count is not a count of skipped
rendered frames: this predicate has multiple callers.

The first playback's170 completed windows contain18378 replay steps, all
advancing time,184 +20 busy calls and0 +24 busy calls. It has38 individual
gateWait records total,29 in the late cluster; earlier events are sparse.

## Why +20 can matter even while the clock advances

Scoped static call-site tracing found four direct callers of `007379D0`:

| Call site | Native context |
| --- | --- |
|0053AB23|Simulation-clock update0053AA20|
|00738D35|Replay loading/readiness query00738D20|
|00738F9A|Replay step's countdown branch|
|00799794|Outer update path after0073A090|

At0053AB23, a true result jumps to0053ABF8. That branch skips the native
service-update block0053AB30–0053ABF7 but still tail-dispatches through
0073A070 at0053AC03, which can invoke the replay step00738F20. The replay
step's distinct +24 gate can remain clear and advance its clock. Thus the
code has a concrete mechanism for **native update work being withheld while
recorded time keeps moving**. At00799794, the +20 result can also skip a
downstream virtual+28 call.

This fits a motion-update/readiness problem better than diagnosing render
FPS from clock samples alone. The exact identities and responsibilities of
the skipped services, individual blocking subscriber and active caller at
each logged busy result remain unresolved. The probe does not log gate
callers/subscriber pointers, so it does not prove153 passes through the
simulation-clock short circuit or identify the loading-indicator UI trigger.

Static evidence is in `replay_hitch_run28136_2026-11-14/`, including
`gate_references.json`,0053AA20–0053AC30 and00799740–00799800 disassembly.
Reference candidates were instruction-checked. A separate raw list-offset
candidate at007C508D decoded as a different displacement and was rejected;
it is not evidence of subscriber-list access. Incomplete range endpoints
are not used as instructions.

## Normal playback versus speed changes and rewind

Requested speed stays1 during the first viewing. The engine applies0.5 and
0.2 in several sections, including near the endpoint. Those rate changes are
separate from the late +20 cluster: the cluster starts while both requested
and applied rates are still1. At419812ms, time152.1349, applied speed becomes
0.5; later it becomes0.2 and reaches the recording end154.1333 at425250ms.
The reason for each native/timeline speed change is not individually traced.
Do not attribute these changes to the user operating playback-speed controls.

The later seek/control segment begins at426375ms, after the end. It has27
seek calls,25 successful import/restore/request sequences and25 recording
repositions in ticks426375–431671. This agrees with the user's rewind
description; it does not explain choppiness that preceded those controls.

Reposition00746000 costs at most0.0021ms in the entire log; import maximum
2.2593ms, restore1.8317ms, request1.5365ms. These are gross instrumented call
durations, not a measurement of every ordinary asynchronous recording read.
Normal sequential replay I/O was not individually hooked, so fast reposition
calls do not rule out a subscriber/streaming-buffer wait.

## Probe overhead and limits

The observer-body elapsed-time estimate is significant:23.65s across189
emitted one-second windows. In the preceding10s it totals1.256s; in the
late8s it totals0.941s, about0.126 versus0.118 seconds per observed second.
This is elapsed wall time accumulated in observer bodies, not CPU utilization
or a complete exclusive overhead benchmark. Assembly boundaries, monitor
writing, parent logging and scheduling effects are not fully accounted for.

The probe can perturb the workload. Its overhead does not show an increase
at this particular onset, but it cannot be dismissed as an influence on wait
durations or readiness behavior. A further live probe should be substantially
narrower and reduce full-view/readability checks on hot paths. Do not use
these timings as uninstrumented performance measurements.

## Next target

Trace the replay subscriber(s) whose virtual+20 readiness method returns
busy and determine what its queue/buffer/completion condition represents.
Keep the simulation-clock short-circuit caller in view; clock advance alone
is insufficient to exonerate native update stalls. Static registration and
vtable analysis is the next focused target. If live identity is needed,
capture the original subscriber object/vtable and result at the existing
virtual call without invoking it again, with a much lighter observer.

No current evidence justifies increasing another pool, changing request
counts, skipping snapshot restores or bypassing either readiness gate.
No new ASI/source/build/game changes were made for this analysis.

Reproduce summaries with
`analysis/replay_hitch_run28136_2026-11-14/summarize_run.py`. Evidence also
includes `baseline.json`, `callback.json`, `hitch.json`, `comparison.json`
and analyzer console outputs. These structural passes do not certify smooth
motion or the root cause of the individual subscriber wait.

# Replay motion / intermittent loading: scoped static analysis

Date: 2026-11-14. Scope: the user's normal-playback hitch report after the
successful REGMAP_REUSE_R2 run24888. No build, game run, debugger attachment,
capacity change or runtime-source patch was performed. Only analysis evidence
and documentation were added/updated.

## Finding

The native replay has explicit ways to hold recorded time while rendering
continues. The most useful next investigation is to measure which of these
paths is active during a hitch. This trace identifies probe sites; it does not
establish the particular cause of the user's intermittent loading indicator.

There is no new demonstrated capacity exhaustion in run24888. Repeated
150-ID snapshot restores do not establish repeated vehicle asset loading or
justify skipping restores. Replay recording buffers, traffic readiness,
subscriber readiness and simulation rate are separate parts of this path.

## Inputs and reproducibility

- Supported `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256
  `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
- Supplied `dist/MAIN_150/DSFV29s_regmap_reuse_24888.log`; measured results
  are in `REGMAP_REUSE_RUN24888_RESULTS_2026-11-14.md` and
  `regmap_reuse_run24888_2026-11-14/replay_timing.json`.
- Current replay adapters `src/V29sReplay150.inc`,
  `src/V29sReplay150Core.inc` and the existing pause/update integration.
- Scoped disassembly is saved in `replay_hitch_static_2026-11-14/`.
  `inspect_native.py` checks the executable hash and supports `disasm LO:HI`
  and `refs TARGET` commands. Reference scans produce byte-pattern
  candidates; call sites used here were checked against disassembly.
  Range endpoints can cut an instruction; incomplete endpoint instructions
  are not evidence.

## 1. Replay time has its own readiness gate

The mode dispatcher at `0073A070` calls `00738F20` in replay mode2. That
step processes due recorded events and, on the ordinary countdown-zero path,
calls `00737A10` at `0073906D` **before** advancing the replay clock.

If the predicate returns busy, execution jumps to `007390BE` and skips the
clock increment. Otherwise the clock at replay-manager `+4E9A0` advances by
`1/120` second, bounded by the recording endpoint. The clock write is at
`007390B6`. This is a simulation step, not a claim about recorded presentation
FPS or how many steps execute per rendered frame.

`00737A10` can report busy for either:

1. A replay subscriber: list at manager `+4E580`, object at node `+0C`,
   virtual method `+24` returns true.
2. The snapshot/traffic gate `0074B760` reports busy.

The related predicate `007379D0` calls subscriber virtual **+20**, then the
same snapshot gate. It is used for other countdown/loading decisions.
These two subscriber checks must not be treated as interchangeable.
The subscriber implementations and which one, if any, blocks this workload
are not yet identified.

`0074B760` reports busy if snapshot phase `+8` is nonzero; otherwise it
consults traffic predicate `005A63B0`. That predicate returns false when
spooler `+DAC == 1` or pending-traffic count `+C8 != 0`.

Consequently, sampled `loading=0` does **not** prove replay time was free to
advance. The current one-second status log neither records these predicate
results nor measures how long they hold the clock.

There are additional early returns in event handling. At `00739006` a
recorded-event handler is called through virtual `+14`; a false result can
return before advancing time. One event-kind branch also zeroes requested
rate `+4E99C`. If readiness timing is clean, event-dispatch holds need to be
distinguished from them rather than labelled streaming by elimination.

## 2. Snapshot completion is stricter than the generic busy test

Snapshot tick `0074BB40` uses `0074B9D0`:

- Phase1 waits for idle, then invokes restore `0074BA20`.
- Phase2 waits for idle, then clears the phase to0.
- Its idle test requires spooler `+DAC == 0`, retained-pending `+9A8 == 0`
  and pending traffic `+C8 == 0`, as well as `005A63B0` succeeding.

The current full-roster import/restore adapter preserves this staged flow.
`ReplayImport` restores immediately only if its three spooler fields are
idle; otherwise it enters phase1. `ReplayRestore` requests the recorded IDs
through `005A79F0`, preserves reference/extra-state restore order, and enters
phase2. Therefore the cost can be in the request itself or in readiness
**after** it returns. Timing only the request function would miss the latter.

Run24888 has79 imports/restores, all preserving the same150-ID roster,
retained0 and hash5CBDFFEA. One status sample sees phase2. There are only900
definition/audio stores across six world constructions, not79x150 stores;
the counters do not support calling every restore a full resource reload.
They also cannot exclude a shorter traffic/readiness wait on each restore.

## 3. Checkpoint restoration repositions the recording stream

`00739F60(targetTime, force)` selects a recorded checkpoint and stores its
pointer in manager `+4E9F8`. It sets the outer transition at `+4E9F4` to1
or2. This is a useful site to log target, caller, force flag, result and
selected checkpoint time.

The outer updater `0073A090` runs a separate transition sequence:

- State1 prepares the transition and enters2.
- State2 waits for a world condition (`005DFCD0`), then calls `00746000`
  on the recording with the checkpoint's stored offset (`0073A240`).
  It writes replay time from the selected checkpoint (`0073A24D`), updates
  subscribers/sequence state, sets countdown50 and enters state3.
- State3 runs settling/completion logic, including the countdown, before
  returning to0. Fifty is a count here, not a demonstrated vehicle limit
  and not necessarily fifty rendered frames.

At `00746000` the recording object repeatedly calls virtual `+0C` until it
returns false. It then splits the requested position into a32KiB-aligned
offset and an in-block offset, invokes virtual `+10` with the block offset,
and resets its buffer/cursor state.

This is an explicit recording-stream reposition path, separate from vehicle
asset streaming. The concrete virtual implementations and their storage
backend are unresolved: the loop is not yet proof of a disk-I/O stall.
Measure its elapsed time before changing buffers or assuming disk speed.

Checkpoint requests are not exclusively explicit user seeks. The outer
updater can call `00739F60` at `0073A51C` when the current time is before
the recording's first indexed time and playback is enabled. Start/load and
other control paths also call it. Thus backward clock jumps/restoration
bursts need caller and target logging before attribution. The trace does
not establish which callers explain the user's uninterrupted-playback hitches.

## 4. Requested speed and native simulation rate are separate

The outer updater also determines simulation pause/rate. It considers
requested rate `+4E99C`, transition/countdown state, an external virtual
override, and timeline/interpolated speed. Timeline helper `0073DAA0` uses
rate values including1,4,2,0.5,0.25 and0.1; another interpolation path uses
`00751870` / `00751960`. The result reaches `0079DAE0`, which controls the
simulation clock. The render loop can continue independently.

Therefore a slow clock interval can be a busy hold, a deliberate rate
change, a checkpoint reset, or another event/transition condition. Current
logs do not record the commanded rate/target and cannot separate these.
No hardcoded50FPS replay cap was established by this trace.

The existing guarded pause-completion service reuses the native completion
tail for the specific paused-loading case; it does not advance replay time
or bypass readiness. Run24888's four successful completions occur in that
case. This is not evidence that they cause the later playback hitches, nor
proof that every remaining wait is resolved.

## Recommended next live evidence: passive timing probe

No new ASI is built by this analysis. If requested, retain REGMAP_REUSE_R2's
capacities, native call order and every readiness decision, and observe:

| Measurement | What it distinguishes |
| --- | --- |
| Actual replay clock advance/hold versus wall time; requested and applied simulation rate | Busy waiting versus intended playback rate or scheduler pause |
| Existing `00737A10` / `007379D0` results and blocking subscriber object/vtable, where practical | Normal advancement gate versus separate countdown/loading gate |
| Snapshot phase transitions and phase1/2 dwell; spooler `+DAC`, `+C8`, `+9A8` | Traffic/snapshot readiness after a fast request |
| Checkpoint request caller, target, force, selected time and transition1/2/3/0 | Automatic reposition versus controls, and which transition repeats |
| Elapsed `00746000`, import, restore and traffic-request calls | Recording reposition cost versus traffic request cost |
| Event-dispatch early returns if the above do not explain holds | Non-readiness event stalls |

Use high-resolution durations, bounded in-memory aggregation and logs on
state changes or long holds. Do not add per-frame synchronous file writes.
Observe calls already made by the game; do not invoke readiness helpers a
second time just to log them. Read native objects only on the owning game
thread, not from the existing monitor thread. Record probe overhead and
separate time spent inside native calls from time between scheduler calls.

A controlled capture should include uninterrupted normal-speed playback
first, then any separate control/transition segment clearly marked. Capture
the user's observed hitch times approximately. This would test whether
loading indicators line up with checkpoint repositions, traffic/subscriber
holds, or another rate/scheduling path. An old retained recording and a
fresh recording may be compared in separate marked segments if needed;
do not delete recordings or change roster/AI inputs for the first capture.

Capacity increases, bypassed gates, forced unpause and hash-only restore
deduplication are not supported by the evidence. A brief-stall timing probe
is more likely to identify the cause than a single untimed memory dump.

## Remaining uncertainty

The responsible branch, blocking subscriber identity, recording virtual
backend, exact loading-indicator trigger and timing cost remain unmeasured.
Logging overhead and recording/presentation quality are also not ruled out.
The static work narrows the live instrumentation target; it is not a
completed playback-smoothness fix.

# Passive replay timing probe: REPLAY_HITCH_R1

2026-11-14. User authorized trying the probe after
`REPLAY_HITCH_STATIC_2026-11-14.md`. Built Release / Win32 using the active
`DriverSF_V29s_NATIVE_TRACE.slnx` and its single ARENA_REUSE project.
Staged only; no game launch, debugger attachment or installation.

## Artifact

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_HITCH_R1.asi`

-270848 bytes.
- SHA256 `758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8`.
- Output `build/v29s_replay_hitch_r1/`.
- Log `%TEMP%\DSFV29s_replay_hitch_<PID>.log`.
- User guide `dist/MAIN_150/REPLAY_HITCH_R1_TEST_2026-11-14.md`.

The tested REGMAP_REUSE_R2 artifact remains byte-identical, SHA256
`8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa`.
No capacities, roster weights, requests, reference order, ED/EF handling,
readiness decisions or completion service behavior were intentionally changed.
All150 requests/160 slots and inherited fixes remain compiled/verified.

## Instrumentation

Five native entry hooks call the displaced native implementation once:

| Entry | Role | Displaced bytes |
| --- | --- | --- |
|007379D0|Subscriber virtual+20 / snapshot-traffic busy check|11|
|00737A10|Subscriber virtual+24 / ordinary clock-advance busy check|11|
|00738F20|Native replay step / clock advancement|5|
|00739F60|Checkpoint request, original caller/target/force/result|10|
|00746000|Recording stream reposition|6|

The copied prefixes contain complete instructions and no relative branches
or calls. Each trampoline executes that prefix and jumps to the native
continuation. Naked bridges save/restore flags, GPRs and FXSAVE state around
both observers. Stack arguments are copied for the native callee and cleaned
separately by callee and bridge, preserving the original calling convention.
Native return registers/flags are restored after observation.

Small conditional RAII scopes observe the existing outer update call,
full-roster import/restore and replay traffic request. Their native call order
and existing error/return paths are retained. Fixtures without the runtime
observer keep exercising the parent behavior; the new fixture separately
exercises the production observer core, bridges and installer.

The shared observer core performs no native-object writes, allocations,
engine calls, sleeps or file writes. It uses16 bounded thread records,
24 nested frames per thread and a512-record queue. Contention or exhaustion
drops telemetry and increments diagnostic counters; it does not spin or
suppress native work. The monitor drains new records every existing200ms
iteration. New high-frequency state reads stay on the intercepted caller,
never in the monitor. Readability checks and all new timing overhead are
included in the observer-body estimate where instrumented.

The installer checks exact supported bytes and the existing executable
identity before allocating executable trampolines and unlocking four code
pages. Page-aware signature matching retains R2's RX/RWX-boundary correction.
It rechecks cold ownership/signatures before writing, verifies exact patches,
flushes instruction caches and restores original page protections. Ordinary
preflight/commit refusal leaves status-1 and the parent baseline after clean
rollback; unrecoverable rollback/protection errors use explicit inherited
fatal diagnostics rather than silently claiming a usable probe.

## Records and interpretation

- `REPLAY_HITCH_START`: installation status, QPC frequency and probe policy.
- `kind=window`: per-operation call counts, gross elapsed/max QPC ticks;
  step advance/unchanged/backward counts, busy counters and scheduling gaps.
  Windows are emitted by active replay calls about once per second.
- `kind=state`: identity, snapshot/transition/loading, requested/applied rate
  and pause changes observed across outer updates.
- `kind=call`: seek/stream/import/restore/request elapsed time and before/after
  state. Seek includes original return address, target, force and result;
  replay request includes the existing request count.
- `kind=gateWait`: observed busy-to-clear readiness interval. Its duration
  uses QPC ticks. The threshold is frequency shifted right10, approximately
  1/1024 second; short intervals are counted without individual records.
- `kind=hold` / `recovered`: replay clock remains unchanged for at least50ms
  with a nonzero requested rate, then moves or the request stops. Here
  `duration` is **milliseconds**, unlike call/gateWait QPC ticks.
- `REPLAY_HITCH_STATUS`: dropped records, thread/depth exhaustion, nesting
  mismatches and QPC failures. These must stay zero for complete telemetry.

Busy counters preserve the two gate identities. `subscriberOnlyBusy` means
the observed true gate result has an idle fallback snapshot/traffic state.
`snapshotPendingAtBusy` and `trafficPendingAtBusy` report pending fallback
state, which can coexist with a subscriber hold; they do not prove the
subscriber loop reached that fallback. This probe does not identify the
individual blocking subscriber object/virtual implementation.

QPC frequency is logged as two decimal32-bit halves. Float values and
addresses are logged as hex bits. Raw32-bit duration counters saturate at
FFFFFFFF; unusually long intervals need that qualification. Elapsed native
calls include nested instrumentation and inherited synchronous logging.
Summing nested operation times would double-count work. `observerBodyTicks`
does not include all assembly/QPC boundary costs or monitor/file overhead;
it is not a full overhead benchmark.

Clock holds can be intentional during pause, speed or lifecycle changes.
Correlate rates, transitions, actual busy checks and the user's symptom time.
Gate waits include polling/scheduler gaps and end at the first observed
clear result, not the exact instant a resource became ready. Context changes
reset scheduling-gap/window accounting so time in menus is not a replay
stall. Final partial windows and open waits can be absent at replay/process
exit; their absence is not proof that no wait occurred. Brief phases entirely
inside an outer update may instead be visible in import/restore call records.

## Verification

`python -B scripts/verify_replay_hitch.py --fixture-only` passes:

- Actual RX pages, real VirtualQuery, RX/RWX signature boundary, precommit
  corruption, partial page-unlock refusal, commit-flush rollback, exact
  restored bytes and page protections, trampoline prefix checks.
- All five production bridges execute a controlled native callee exactly
  once and preserve stack cleanup/arguments, GPRs, flags including DF, x87
  control/status/register data and SSE register/MXCSR state.
- Clock hold/recovery, separate busy predicates, observed gate-wait interval,
  replay exit/reentry accounting and bounded depth/thread/queue pressure.
- Parser frequency/duration conversion and rejection of bad/missing startup,
  dropped records and incomplete lines; no visual smoothness certification.
- Inherited map, snapshot and pause fixtures.

`--verify-build` passes exact compiled targets/signatures, both observer
save/restore sequences and calling conventions, required log markers,
read-only observer/source integration checks, preserved R2 artifact hash
and the complete inherited compiled verification chain. `--package` verifies
fixture/source/build hashes and stages an identical copy without replacing
another artifact.

Evidence: `replay_hitch_r1_2026-11-14/fixture.json`, `fixture.log`,
`fixture_build.log`, `verification.json`, `package.json`, parser fixture,
compiled bridge disassembly and `parent/` inherited evidence. MSBuild log:
`build/replay_hitch_r1_build.log`. Build retains pre-existing signedness,
unused-static and exported-DllMain warnings. MSBuild's file tracker required
an approved run outside the sandbox. An initial no-CRT stack-probe link error
was resolved by keeping the monitor's formatting buffer in static storage;
the final build succeeds without adding a CRT dependency.

## Next live evidence

Follow the user guide with one enabled traffic ASI and the same working
vanilla150 assets. First capture normal1x playback without controls, note
approximate loading-indicator/hitch times, then exit normally after a short
menu wait. Analyze the supplied log with the existing baseline/callback
analyzers and `analyze_replay_hitch_log.py` before proposing a runtime fix.
No live smoothness, overhead or root-cause claim is established by the
offline checks.

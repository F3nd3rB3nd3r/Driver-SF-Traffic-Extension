# REPLAY_CLIENT_R1 narrow busy-subscriber probe

2026-11-14. User authorized the next live-probe preparation after
`REPLAY_SUBSCRIBER_STATIC_2026-11-14.md`. Built using the active single-project
solution, Release / Win32. Offline checks pass; staged only. No installation,
game run, debugger attachment, native capacity increase or asset change.

## Artifact

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_CLIENT_R1.asi`

-265216 bytes.
- SHA256 `d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3`.
- Output `build/v29s_replay_client_r1/`.
- Log `%TEMP%\DSFV29s_replay_client_<PID>.log`.
- Guide `dist/MAIN_150/REPLAY_CLIENT_R1_TEST_2026-11-14.md`.

REGMAP_REUSE_R2 and REPLAY_HITCH_R1 ASIs remain byte-identical. The project
now selects `DSF_V29S_REPLAY_CLIENT`; `DSF_V29S_REPLAY_HITCH` is not enabled.
All R2 capacities/fixes,150 requests/160 slots and ED/EF handling remain.
The existing conditional broad RAII scopes are not compiled without their
runtime definition. Compiled verification checks absence of broad hooks
and HITCH startup/event markers, plus the inherited R2 verification chain.

## Three native hooks, four identities

| Native entry | Native client | Displaced complete prefix |
| --- | --- | --- |
|007549D0|Client3 at manager+4E388 and client7 at+4E428|5 bytes|
|00755DF0|Vehicle client1 at+4E3A8|7 bytes|
|007523B0|Player client2 at+4E3E0|5 bytes|

The original predicate runs once through a trampoline. The observer filters
its return to **007379F1**, the original +20 virtual-call return in
007379D0, and validates the client offset/vtable/method against EDI's manager.
Other uses of a shared method return with their native output untouched.

An important fixture finding: the vehicle predicate's saved-ECX stack slot
is also used for a timestamp local. Some branches return that timestamp in
ECX. The bridge therefore saves **input self separately before the native
call** and preserves the actual native output ECX. It does not assume ECX
still identifies the client after return. The copied native body and terminal
timestamp branch exercise this distinction in the fixture.

All bridges preserve the original result registers, flags/DF, x87 and SSE
state. They retain input self in one extra stack word, then discard it with
LEA after restoring native flags. All copied prefixes contain complete
instructions and no relative calls/branches.

## Lower hot-path cost

If the native result is clear and no tracked wait is open, the assembly
bridge increments a counter and returns. That path performs **no new C++
observer call, QPC query, VirtualQuery/readability scan or FXSAVE**. When a
different client has a tracked wait, clear candidates can enter the observer
but return before state capture/time queries if their own wait is closed.

The only captured engine state is on busy start, clear, context change and
one-second held heartbeat. There are no step, outer-update, seek, recording
reposition, import/restore/request timing hooks in this probe. Existing R2
and low-frequency monitor telemetry remain; no readiness decisions change.

Eight bounded thread records each track four client identities. A512-record
queue uses a try-lock; contention or exhaustion drops telemetry and increments
diagnostics. No spin, allocation, file writes or engine calls occur in the
shared observer core. New event formatting/writing occurs only in SCPoll on
the existing monitor thread. Extra event/recording pointer readability checks
are limited to captures, not normal clear callbacks.

The startup installer checks the supported executable identity, exact
predicate bodies, the original aggregate gate and all four vtable+20 entries.
Three pages are unlocked and rechecked before patching, exact installed bytes
are verified, instruction caches flushed and original protections restored.
Ordinary refusal after a clean rollback leaves diagnostic status-1 with the
R2 parent still active. Unrecoverable rollback/protection errors use explicit
inherited fatal diagnostics (`clientRollback*` / `clientProtectionRestore`).

## Captured data and interpretation

`REPLAY_CLIENT_EVENT` kinds:

- `busy`: first observed true result for this client/context.
- `clear`: first observed false result after its open wait.
- `held`: at most one extra capture per second while still busy.
- `contextEnd`: owner/recording changes or replay clock moves backward while
  the wait is open. Uses the cached old snapshot; it is not a readiness
  recovery and never dereferences an old owner to capture it.

Each record has client ID, sequence/thread identity, original caller,
elapsed QPC ticks, capture-body QPC ticks, replay clock/requested rate,
snapshot/transition/loading state and vehicle pending flag where applicable.
Current/next/following event snapshots include pointer validity, type/flags,
time bits, payload size and chain link.

Recording state includes backend/vtable/mode, capacity/base, retained/filled/
parse cursors, free/used/unparsed bytes under the native circular-buffer
convention, file position, in-flight byte count, end flag and active flag.
Backend1 is the native file wrapper E2A088, backend2 the memory wrapper
E2A33C; other types are recorded as backend0. No native I/O or virtual
readiness function is called again for logging.

Free-space calculations require all cursors inside a non-overflowing ring
range and a bounded capacity. Equal retained/filled pointers use the native
free=capacity convention. Invalid/missing initialization leaves ringValid0;
it is not automatically an engine fault. Extra unreadable non-null pointers
increment readFailures and invalidate that node/snapshot without bypassing
native behavior.

QPC frequency is logged in the startup record; elapsed/capture durations are
raw32-bit QPC ticks, saturating atFFFFFFFF. The inherited decimal formatter
may print the upper half of U32 as a signed
number; the analyzer restores raw unsigned elapsed/capture ticks. Captures
occur **after** the
original predicate returns; observed data conditions are not an independent
trace of its individual branch instruction. Busy spans include polling/
scheduling gaps. The in-flight byte count is raw bookkeeping, not an extra
I/O completion check; the end flag does not prove all queued events were
consumed. Context ends and final open waits must remain distinct from clear.

`REPLAY_CLIENT_STATUS` reports fast-clear/observer/busy/start/end/context/
held/open counters, per-client busy counts and drops/thread/identity/read/
clock errors. Require zero errors for complete telemetry. A clean run with
no busy events is an unexercised path, not smoothness certification.
captureTicks measures snapshot capture only, excluding fast-path assembly,
other observer work and monitor I/O; actual overhead remains unmeasured.

## Verification

`python -B scripts/verify_replay_client.py --fixture-only` passes:

- Actual RX page installation; exact native/vtable/gate checks; partial page
  unlock refusal, precommit corruption and commit-flush rollback; RX restored.
- Copied original predicate bodies, including absent current/next data,
  terminal expired timestamp and vehicle pending-flag branches. Only external
  root/time references are relocated/stubbed in the isolated vehicle fixture.
- Native-versus-bridge output parity for all four clients: stack, all GPRs,
  native ECX scratch output, EDI identity, flags/DF, x87/SSE state. Native
  timestamp query occurs once per vehicle invocation in its exercised branch.
- Normal-clear path incurs zero C++ observer, QPC and readability calls;
  a caller outside the aggregate gate is filtered without observer capture.
- Correct identity/busy-clear pairs, chained data, wrapped/empty/invalid ring
  math, unreadable pointer rejection, held/context changes, bounded thread/
  queue pressure and contention drops.
- Parser pairing, IDs, observed missing-next condition and QPC conversion;
  bad startup, missing counters, drops, incomplete lines and wrong IDs rejected.
- Inherited R2 map/snapshot/pause fixtures.

`--verify-build` passes compiled hook tables/native bytes, one original call
per bridge, one FXSAVE/FXRSTOR observer sequence, saved-self/flags-safe stack
cleanup, atomic clear counter, required log markers, no broad hook symbols/
markers, read-only observer checks, parent artifact hashes and the complete
inherited compiled verification chain. `--package` hash-checks fixture,
source, analysis tools and staged bytes.

Evidence: `replay_client_r1_2026-11-14/fixture.json`, `fixture.log`,
`fixture_build.log`, `verification.json`, `package.json`, parser fixture,
compiled bridge disassembly and `parent/` inherited verification. Build log:
`build/replay_client_r1_build.log`. MSBuild required approved access outside
the sandbox for its file tracker; the final build succeeds with existing
signedness/unused-static/exported-DllMain warnings.

## Next live check

Follow the guide in the same installation/assets/settings as28136, with
exactly one traffic ASI. Make a fresh recording and watch through the final
20–30 seconds at normal speed before operating controls. Retain the new
client log and user symptom timing, then analyze it with baseline/callback
integrity plus `scripts/analyze_replay_client_log.py` before choosing a fix.
No live client identity, buffer-full cause or reduced overhead is certified
by these offline checks.

# REPLAY_READ1M_R1 combined replay ring and refill test

2026-11-14. User authorized the next combined test after the static refill
audit. Implemented, built Release/Win32 through the active single-project
`DriverSF_V29s_NATIVE_TRACE.slnx`, offline verified and staged. No game
installation, game execution, debugger attachment or asset modification.

## Artifact and build

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_READ1M_R1.asi`

- 276480 bytes.
- SHA256 `fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.
- Log `%TEMP%\DSFV29s_replay_read1m_<PID>.log`.
- Guide `dist/MAIN_150/REPLAY_READ1M_R1_TEST_2026-11-14.md`.
- Build `build/v29s_replay_read1m_r1/`; verification evidence
  `analysis/replay_read1m_r1_2026-11-14/`.

The project enables READ1M and the existing CLIENT probe; broad HITCH is absent.
150 requests/160 slots, ED/EF and inherited traffic/audio/parked/snapshot/pause/
definition-map fixes remain. Existing R2, HITCH_R1 and CLIENT_R1 staged ASIs
retain their previous hashes. The current pre-edit bytes of six integration
files are saved under the evidence directory's `prechange/` with a manifest;
pre-existing uncommitted work was preserved.

The first sandboxed MSBuild attempt compiled but FileTracker failed to resolve
paths. The approved retry outside the sandbox completed successfully. Compiler
warnings concern existing signedness/unreferenced functions and the exported
DllMain entry point; no new reader-module warning was reported. No dependencies
were installed.

## Capacity and lifetime

File-wrapper constructor `0073FAB0` receives a different buffer and capacity
only for native mode0 readers with non-null original backing and capacity40000.
The original constructor executes once through its displaced prefix/trampoline.
Writer constructors keep their original arguments. Memory-wrapper paths and
native provider leases/releases are unchanged.

Each distinct wrapper address gets one process-lifetime metadata record and a
private reservation: 1 MiB committed payload aligned to64 KiB, with an uncommitted
64 KiB region on each side. Directory publication uses compare/exchange; records
are never deleted while readers/monitor can find them. There is no fixed owner
directory limit. Allocation occurs only on first managed construction for an
owner, never in a reader-counter hook or client busy callback.

File destructor `0073FB40` executes the original close/drain/destruction before
marking the record closed. It never dereferences the native object afterward,
because the destructor flag can delete that allocation. Private backing remains
allocated until process exit and can be reused by a subsequent construction at
the same wrapper address after the observed destructor. Generation and reuse
counters distinguish those lifetimes. Active reconstruction is refused for
private-buffer reuse and reported as a fault/fallback; active data is not copied
or moved. Native ring pointers are set by construction, before any live records.

Metadata/reservations survive native object deletion safely. Same-address
read/write/read cycles reuse the same private payload without allocating another
MiB on every transition. This is deliberate process-lifetime retention, matching
the diagnostic build's stable-owner approach, rather than per-transition free.
`retainedBackingBytes` reports committed payload only; each owner also has4 KiB
metadata and128 KiB of reserved, uncommitted guard space.

If private metadata/reservation/commit fails, the original constructor receives
its native arguments. allocationFailed/fallbacks make the test invalid in the
analyzer. No new runtime process-exit fallback was added for such failures.

## Eight hooks and native behavior

| Site | Action |
| --- | --- |
| 0073FAB0,8-byte prefix | Reader backing/capacity at construction |
| 0073FB40,9-byte prefix | Mark closed after original destructor |
| 0074694A,8 bytes | Observe the original backend-poll result |
| 0074698C,6 bytes | Count original pending bytes being committed |
| 00746AD9,8 bytes | Count wrap-space check/stop with exact free/required values |
| 00746B62,7 bytes | Count ordinary free-space check/stop |
| 00746BD7,6 bytes | Count original issue and native read return value |
| 00746B2B,13 bytes | Count end-of-data check/stop |

Six internal reader bridges preserve all GPRs, stack, flags/DF and x87/SSE state
around integer-only observations. They then reproduce the displaced native
instruction/comparison/branch and resume at the same success/stop targets.
The native virtual read/poll executes once. There is no extra engine call,
refill loop, forced prefetch, sampler change, reordered delivery, readiness bypass
or recording-format change. Native 32 KiB transfer/assembly/seek units remain.

Startup checks exact full native constructor/destructor/reader bytes, dispatch
bytes, file-vtable entries and the supported executable identity. Reader matching
accounts only for its own installed patches. Two code pages are transactionally
unlocked, rechecked, patched, flushed and restored. A clean refusal/rollback logs
READ status-1 while the parent remains; unrecoverable rollback/protection failures
use explicit inherited PauseFatal diagnostics. The existing client observer has
its own three hooks/checks and must install before this component is enabled.

## Telemetry and interpretation

Reader hooks update counters using already-valid native fields and stable owner
records. They make no new clock, VirtualQuery, allocation, file or engine calls.
The existing monitor formats/writes new READ rows once per second. Individual
counter fields are atomic; periodic multi-field rows are not one globally
coherent instant. CLIENT boundary captures obtain the same counter view on the
existing slow busy/clear/context/held path; normal all-clear CLIENT still avoids
its C++/QPC/readability/FXSAVE observer.

READ_STATUS contains owner/construct/destruct/reuse, writer bypass, allocation/
fallback/fault and unmanaged-call counts. READ_OWNER and CLIENT `rf*` fields show
backend-busy, commit bytes, wrap checks/stops, free-space checks/stops, issue bytes/
rejections, end checks/stops, minimum checked free space, maximum assembly need
and the latest stop's kind/poll ordinal/free/required values. Stop kinds are
1=backend busy,2=wrap-space stop,3=ordinary space stop,4=end-of-data stop.

Counters persist across owner generations. Minimum free/max required/latest stop
reset on managed construction. File-position and in-flight fields retain their
existing raw native meaning. Normal prefetch can reach a space stop or observe
backend busy; those counts alone do not identify a user-visible hitch. Compare
their deltas with the narrow client3 boundaries and replay time. Unmanaged calls
can be native memory-reader paths, which this experiment deliberately preserves.
Fallbacks/faults/issueRejected are separate integrity failures, not benign proof
of capacity pressure. No busy events is not playback-smoothness certification.

## Offline verification

`python -B scripts/verify_replay_read1m.py --fixture-only` passes:

- Actual RX page installation and dependency checks; refusal, partial-unlock,
  precommit corruption and flush-failure rollback with original protections.
- Private alignment/readability and inaccessible guard regions;100 same-address
  generations with stable backing; writer passthrough; more than4 distinct owners.
- Failure at metadata, reservation and payload-commit allocation stages uses
  native constructor arguments once and records explicit fallback counts.
- All six reader bridges preserve native stack/GPR/flag/DF/x87/SSE outcomes for
  taken and untaken branches and the 32 KiB equality boundary.
- The actual copied native reader body executes on the1 MiB ring, including a
  header at the ring end, a payload spanning three reads, wrap relocation,
  async-busy result, commits, ordinary-space/wrap-space/end stops and unchanged
  single32 KiB issue cadence. External backend/link/validation/move services are
  isolated stubs; this does not simulate the full game or storage scheduler.
- Deleting destructor frees its fixture object before returning; the observer
  subsequently updates only retained metadata, with no freed-object read.
- Inherited client native/ABI/installer/parser fixtures and full R2 chain.

`--verify-build` additionally passes inherited compiled checks, exact native
arrays/patch table/bridge framing, absent broad HITCH hooks, expected markers/
prefix and previous artifact hashes. Combined parser fixtures reject wrong
startup/size/backing, fallbacks/faults/rejected reads, missing fields and incomplete
records, and do not infer reader exercise or smoothness from startup alone.
`--package` verifies source/tool hashes and staged byte equality without replacing
another artifact. New analyzer: `scripts/analyze_replay_read1m_log.py`.

## Live scope still open

No live improvement, total probe overhead, real I/O performance or teardown
stability is certified yet. Follow the guide with the known problematic replay
if available, normal1x playback through the end before rewind, then another
Story/Replay/menu cycle and normal exit. Analyze baseline150/callback, CLIENT and
READ integrity before recommending another change. If waits remain with adequate
prior headroom and no space/assembly stops, investigate scheduling/completion
instead of simply enlarging the ring again.

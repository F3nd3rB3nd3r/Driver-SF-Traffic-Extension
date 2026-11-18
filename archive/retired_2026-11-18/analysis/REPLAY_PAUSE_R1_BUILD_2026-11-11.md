# REPLAY_PAUSE_R1 — completion while simulation time is paused

User authorized the ASI adjustment on2026-11-11 after the run8944 log/dump
analysis. The active solution was built once, Release/Win32, and the resulting
ASI is staged only. No game launch, installation, asset/recording edit,
dependency install or web search was performed.

## Problem and scope

Run8944's snapshot was correct:150 unique nonzero IDs including181, matching
cache guards/hash, snapshot phase0 and empty pending queues. All native
loading-readiness predicates evaluated clear in the full dump. Loading1 and
replay clock0.8916667 persisted46.688s in the log. The simulation clock was
paused at rate0, so its scheduler did not dispatch0053AA20 ->0073A070 ->00738F20,
which contains replay loading completion. See
`REPLAY150_RUN8944_SOFTLOCK_2026-11-07.md` for the evidence and limitations.

This correction supplies completion servicing from an updater that runs
outside that paused clock. It does not establish the initial zero-rate caller
or patch the separate null simulation-manager Lua pause crash. Live recovery
is pending user testing.

## Implementation

One new entry hook at0073A090. Its original six-byte prologue is relocated to
an RX trampoline and the complete original updater runs first. After return,
the service revalidates current-root replay/simulation/recording identity and
the native simulation/controller vtables and callback pointers.

Recovery requires all of the following:

- mode2, loading1, lifecycle5, transition0;
- completed snapshot phase0 and countdown0;
- paused simulation controller with zero rate and zero requested playback rate;
- a matching completed import tag for the same replay, simulation, recording
  and importing thread;
- valid REPLAY150 private-cache guards/data and matching native inline mirror.

The parent import path adds only an observer call under the new build flag.
It tags completed imports without changing native request/restore behavior.
The service rechecks identity/state immediately before calling game code.
Reentrancy is guarded; a newer import tag is not consumed by an older completion.

The completion bridge recreates the original saved-register frame and jumps
directly into the unchanged native block007390BE. That block calls the original
busy predicate00738D20, clears loading only when it passes, then performs the
original notification/debug/lifecycle sequence. It does not replay a physics
step, advance events, or copy/reimplement the completion body in production.
Busy readiness keeps loading set and permits retry on later qualifying updates.

The mod does not directly write simulation pause/rate, requested playback rate,
or replay time. Native completion callbacks retain their own behavior. All game
calls occur on the tagged updater/import thread; the monitor prints counters
only. Runtime state/identity/mark/cache misses skip recovery. Startup failures
log a reason and exit code0x29F5 rather than proceeding with a partial hook.

Installation checks the entire1678-byte original updater,118-byte completion
tail,128-byte readiness predicate and native clock step pointer, on the exact
supported Driver.exe SHA256. Commit checks, instruction-cache flush, protection
restoration and exact rollback checks protect the single six-byte patch.

Preserved capacities:150 requests/160 graphics with ED/EF, snapshot150 IDs/256
reference records, parked150 candidates/25x150 categories, audio model192/
slots2048/default64MiB, pending512/cleanup262144 and callback512. No weights,
population, refresh timing, serialized recording format or assets changed.

## Verification

`python -B scripts/verify_replay_pause.py --fixture-only` passed:

- Full updater signature validation, one-site patch and exact rollback.
- Captured paused/loading state completes exactly once through the production
  bridge and real native readiness/completion bytes, with original notifications
  and lifecycle update. Pause/rate/replay clock remain unchanged in the fixture.
- Native busy subscriber/world cases hold loading, then complete on retry.
- Mode/loading/lifecycle/transition/phase/countdown/rate/import/thread/cache/
  lifetime guard cases, including nonzero/NaN rates and absent simulation.
- Import during the original update, debug branch preserving native lifecycle
  behavior, reentrancy and100 repeated qualifying loads.
- Callee-saved registers, stack cleanup and x87 stack balance.

External world/subscriber/notifier/lifecycle services and the original updater
body are fixture stubs. The real native completion and readiness blocks execute;
this is not a live reproduction of the complete engine or scheduler.

The inherited snapshot fixture also runs with the new build flag and verifies
the additive observer matches every import. Its native capture/save/load/tick,
150-ID and256-reference tests remain passing. The compiled import's call to
the actual observer is checked, including MSVC incremental-link thunk resolution.

The active Release/Win32 solution build succeeded. Approved escalation was used
because MSBuild FileTracker previously failed inside the sandbox. Existing
signed/unsigned, unused-function and exported-entry warnings remain; there were
no new replay-pause warnings. No retired project/alternate build was invoked.

`python -B scripts/verify_replay_pause.py --verify-build` passed compiled bridge/
target/native-byte checks and all inherited snapshot, parked, notification,
baseline, callback, arena, chapter and capacity checks. Parent artifacts remain
unchanged. Focused parser checks subsequently passed via`--parser-only`:
completed/native-held events, bad startup/phase/completion/fatal rejection, and
no claim of exercise or visible/control success from startup alone. Parser
results and current verifier hashes are recorded with verification.json.

Evidence directory:`replay_pause_r1_2026-11-11/`: fixture.json/log/build log,
snapshot fixture and inherited verification tree, compiled disassemblies,
parser.json/sample, verification.json and package.json.

## Staged test

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_PAUSE_R1.asi`

244224 bytes, SHA256
`2847467ed26424b6c17773b2c0bd179b6218fbca1f019dc97a50feb86e2c083f`.

Log:`%TEMP%\DSFV29s_replay_pause_<PID>.log`.
Guide:`dist/MAIN_150/REPLAY_PAUSE_R1_TEST_2026-11-11.md`.

Use exactly one traffic ASI. Repeat the previously failing first Story->Replay
entry, test retained and fresh recordings, pause/play/seek, then Story/Replay
and menu reloads with normal exit. A completed recovery event establishes that
the service ran; user-visible loading and playback behavior still need testing.
An ordinary clean run without such an event does not prove the stuck path was
exercised. New logs should be analyzed before any further patch.

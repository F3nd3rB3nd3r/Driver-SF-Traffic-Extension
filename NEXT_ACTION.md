# Next action ? work only on the next concrete request

## Current finding: run8944 paused replay loading softlock

Analyze/report completed; no new ASI built. Read
`analysis/REPLAY150_RUN8944_SOFTLOCK_2026-11-07.md` for the native chain and
`analysis/replay150_run8944_2026-11-07/dump_analysis.json` for captured values.
First Replay entry after Story can freeze. Correct150-ID cache and phase0,
empty queues, all native loading-readiness predicates pass. Simulation clock
paused at rate0 blocks its step callback, which owns replay/loading completion;
loading1 and clock0.8916667 persisted45 samples/46.688s. Initial pause caller
and ordering remain unproven. Do not infer a new capacity limit or blame a
plugin/older recording solely from their presence.

When a fix is requested: preserve snapshot150/256 and parent capacities; add
guarded loading-completion servicing from an updater on the game thread that
runs while simulation is paused, with telemetry and native fixture coverage.
Retain mode/lifetime/readiness/phase checks and user pause. No unconditional
unpause, resource-gate bypass or game calls from the monitor thread.

## Current test: REPLAY150_R1 live replay/lifecycle validation

The authorized full snapshot correction is built, fixture/compiled verified
and staged. Follow `dist/MAIN_150/REPLAY150_R1_TEST_2026-11-07.md`: fresh process,
fresh recording, repeated Story/Replay cycles, menu/Story reload, Challenge,
normal exit. Exactly one traffic ASI. No agent installation or game execution.

First live run10520 passed: user-confirmed Story driving, replay playback,
correct loading and normal exit;84 saves/52 restores preserved150 IDs/hash
and181, with no replay invalid/fatal. Pending/cleanup ended empty. Report:
`analysis/REPLAY150_RUN10520_RESULTS_2026-11-07.md`. That run passed its workload; subsequent run8944 reproduced a separate paused-loading block.
Next targeted coverage is repeated Story -> Replay -> Story -> Replay, then
menu/Story reload and another replay. Do not equate52 internal restores with
52 user sessions. Retained>0/deferred/cached paths remain unexercised live.

Require successful REPLAY150_START status1/hooks7 and all inherited startup
markers. Check capture/save/import/restore counts/hashes, ID50=181 for the
canonical150 roster, no replay fatal/invalid, temporary storage completion and
native restore-state transitions. New STATUS records include loading,
lifecycle, clock, event cursor, simulation pointer and native pending counts.
A sampled phase or nonzero activeTemps is not proof of a persistent stall/leak.

Report: `analysis/REPLAY150_R1_BUILD_2026-11-07.md`.
Evidence: `analysis/replay150_r1_2026-11-07/verification.json`, `fixture.json`,
`package.json` and the inherited verifier tree. Use the top-level
`python -B scripts/verify_replay150.py` for any newly requested verification.

PARKED150_R1 run19620 validated parked variety after toggles/reloads, correct
rendering/audio and about100FPS in a long session. Replay-related failures
remained. The full dump confirms150-ID snapshot overlap:181 became0; native
snapshot phase0 and empty request queues do not establish the exact loading
block. REPLAY150_R1 fixes the overlap; run10520 passed its first live replay workload, while repeated lifecycle coverage remains pending.
The separate null simulation-manager Lua pause crash is not bypassed.

Keep150 requests/160 slots/ED+EF and all inherited capacities. The new256
reference capacity concerns snapshot storage only, not the game's retained
vehicle pool. No unrelated limit increase, INI or larger pack sizing.

## If a new log is supplied

Read that explicitly supplied log before proposing changes. Use `scripts/analyze_callback_log.py`; inspect `LEGACY_FATAL`, arena/callback/pending/cleanup status and user-visible outcome. Nonzero cached gauges or native audio setup returns need interpretation; do not equate them with a proven leak or audible failure.

## Unresolved work, when requested

1. **Audio setup outcomes.** Run29344 recorded1731 successful/18855 failed native setup returns despite the user's correct audio throughout. Registry misses/full and pool-pressure indicators were0. Decode the native return conditions before treating them as audible failures or proposing capacity changes. Final cached registry current1 is not proof of a leak; stores/removals450/450 balanced.
2. **Replay lifecycle.** The full traffic snapshot overlap is corrected in staged REPLAY150_R1. The first Story/replay/normal-exit run10520 passed; repeated Story/Replay/menu validation remains pending. Trace any remaining stall from the new transition telemetry; do not attribute the separate null simulation-manager pause crash without evidence. Parked fallback was resolved in the user's PARKED150_R1 test.
3. **Broader vanilla chapter coverage.** Current tests exercised traffic settings named Chapter1/6/7, Exposition and Challenge frequencies0/1/2. Actual chapter saves, mission-required vehicle identity, longer gameplay and150 simultaneous drawable proof remain broader validation work.

The Forza package/configurable pack sizing remain deferred. Its original files are preserved in `archive/retired_2026-11-06/dist/forza_package/`.

## Resolved or limited findings

- The old ninth-setting silent-exit regression is resolved in the supplied ARENA_REUSE run; the prior process exit code itself was never captured.
- Both current menu unloads drained154 pending nodes and cleanup completed; no need to repeat old cleanup/pending limit experiments by default.
- Callback512 remains installed, but its extra capacity was not exercised by the successful run. An earlier small dump supported a conditional overflow candidate; do not state that this was the captured cause of every Challenge exit.
- Same-address spool reset reattachment is fixture-tested; the live run used two different owners and ordinary retained backing.

Use `START_HERE.md` for current capacities and artifact identity. Read historical material only for a specific question; its old next-step instructions are superseded. Preserve 150 requests /160 slots /ED+EF unless the user explicitly changes scope.

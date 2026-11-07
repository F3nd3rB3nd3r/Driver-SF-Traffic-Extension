# DriverSF V29s ? current baseline

Updated 2026-11-07. This file and `NEXT_ACTION.md` describe the current state.
Historical reports' proposed next steps are superseded by the current request.

## Current requested test: REPLAY150_R1

The full replay traffic snapshot correction is built, fixture/compiled verified
and staged. Seven adapters preserve150 traffic IDs and256 retained-reference
records in separate storage; the original612-byte views/624-byte embedded
object and variable-length recording format remain unchanged. This corrects
native50-entry field overlap (model181 became0 in the supplied dump). It does
not establish that this was the sole loading-softlock cause or bypass the
separate null simulation-manager pause crash.

- Staged: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY150_R1.asi`.
- Size234496 bytes; SHA256 `2ab7f763f216abfaf5091308f0adb3e428512038d2f0384432faa140aae12dd9`.
- Guide: `dist/MAIN_150/REPLAY150_R1_TEST_2026-11-07.md`.
- Log: `%TEMP%\DSFV29s_replay150_<PID>.log`.
- Verify: `python -B scripts/verify_replay150.py`.
- Report: `analysis/REPLAY150_R1_BUILD_2026-11-07.md`.
- The active `.slnx`/project builds REPLAY150_R1 only, Release/Win32.
- Staged only: no game run or installation by the agent.

Use a fresh recording and repeat Story -> Replay -> Story -> Replay, then
menu/Story reload, Challenge and normal exit. Analyze the supplied log with
both `analyze_baseline150_log.py` and `analyze_callback_log.py`. Snapshot events
must preserve count/hash/ID50; sampled loading state alone is not a diagnosis.

## Latest failure: REPLAY150_R1 run8944 ? paused loading completion

User reports softlock on first Replay entry after Story. The log and full
Driver.DMP (PID8944) confirm correct150-ID cache/181/hash, completed snapshot
phase0 and empty pending queues. Loading1/clock0.8916667 persisted46.688s.
The simulation clock is paused at zero rate; native scheduler skips its
0053AA20 step, which routes to replay/loading completion00738F20. Captured
native readiness gates all pass, but the completion callback is stranded.
This supports a paused-clock/loading circular wait; the first pause caller
and ordering remain unknown. No new limit, ASI or game patch made in analysis.

Report: `analysis/REPLAY150_RUN8944_SOFTLOCK_2026-11-07.md`.
Next authorized fix should service native loading completion on an unpaused
game-thread updater with readiness/lifetime guards, preserving user pause.
The user notes recordings can persist across restarts; their origin is not
established, but this dump's full roster is directly verified intact.

## Latest live result: REPLAY150_R1 run10520

User reports about five minutes Story driving, replay playback, correct
loading and normal exit. The12m48.984s log recorded84 snapshot saves and52
restores, all retaining the canonical150 roster/hash and ID181, with replay
invalid0 and activeTemps0. Both required analyzers and inherited structural
checks passed; pending/cleanup finished empty. This is the first live replay
pass, not52 user replay sessions or completed repeated Story/Replay coverage.
Retained counts, cached captures and deferred imports were all0 in this run.
FPS/detailed audio were not supplied. Next: repeat Story -> Replay -> Story ->
Replay and menu/Story reload. See
`analysis/REPLAY150_RUN10520_RESULTS_2026-11-07.md`.

## Live-validated parent: PARKED150_R1, replay stability unresolved

User run19620 covered about47m52s. Parked variety survived toggles and reloads;
the user reported correct audio/rendering and about100FPS. It ended around a
replay-related failure, so it is not a successful normal-exit replay test.
The71.8MB log had30 parked rebuilds, peak150 candidates,168781 selections beyond
the original50 prefix, and no parked invalid/guard failures. Later replay
restore requests contained model0 instead of181; this motivated REPLAY150_R1.
The separate full Task Manager dump captured a loading stall after Story/Replay.

Preserved parent: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_PARKED150_R1.asi`,
218624 bytes, SHA256 `6235a3cb814c8d5adf748766d50d06f7fb5d4cfbf7fb85eb59dc5db9aa2c1a36`.
The parent's build report is `analysis/PARKED150_R1_BUILD_2026-11-06.md`;
its former live-pending statement is superseded by the user result above.

All inherited150 requests/160 graphics slots/ED+EF capacities are preserved.
Exactly one traffic ASI should be enabled in the game. Staging is separate
from installing. Current fixes do not certify150 simultaneous drawable bodies.

## Last live-validated parent: NOTIFY150_R1

- Active solution: `DriverSF_V29s_NATIVE_TRACE.slnx`.
- Its only project: `DriverSF_V29s_ARENA_REUSE.vcxproj`, Release / Win32.
- Parent ASI: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_NOTIFY150_R1.asi`.
- Parent size210432 bytes; SHA256 `d9be572176410e41fdbfe54a923ffc1759042a3ef80217f59742ee4f87e482e5`.
- Parent log prefix: `%TEMP%\DSFV29s_notify150_<PID>.log`.
- Latest parent result: `analysis/NOTIFY150_RUN29344_RESULTS_2026-11-06.md`.
- Older ARENA_REUSE_R1 remains preserved for its historical transition workload;
  older builds lack the direct150 notification correction.

Exactly one traffic ASI should be enabled in the game. Staging a file here is separate from installing it into the game.

## Latest extended parent verification

Run29344 covered31m23.531s,14 expanded requests and three pending unloads. User
confirmed correct rendering/audio, roughly100 FPS, varied Challenges, main-menu
transitions, Story reload, chases and normal exit. Native audio setup failures
were recorded despite correct audibility, so their interpretation remains open.
See [run29344 results](analysis/NOTIFY150_RUN29344_RESULTS_2026-11-06.md).

## Earlier targeted notification verification

Run7480 passed the NOTIFY150_R1 direct request. Its150-ID notification used the
correct sidecar/hash, all150 IDs reached native readiness, and no diagnostic
fatal/guard failures were recorded. Audio model registry peak150;617 setups
succeeded with0 failures. Pending/cleanup finished empty with balanced walks/
drains. User reported everything seemed fine. Only four requested models were
observed in body-draw calls; vanilla selection weights were retained.

This establishes the reported crash fix, not complete150-model visual/audio or
chapter/lifecycle coverage. Exact exit method/FPS were not provided; the final
readiness gauge was cached. See
[run7480 results](analysis/NOTIFY150_RUN7480_RESULTS_2026-11-06.md).

## Previous broad transition verification

Run20236 lasted 26m48.578s. The user completed about eight Challenges through mixed garage/free-drive entry, returned to the main menu, reloaded Story Mode, completed four more, and exited normally.

The log recorded 33 supported traffic-setting requests, 4950 scratch checks, zero external scratch arenas, and two spooler arenas for two owners. Both pending-map unloads completed; cleanup appends and drains balanced. No fatal, invalid, guard, allocation or capacity error was reported. This is the working baseline for that tested sequence.

The obsolete scratch helper redirected at50 entries even though the parent already provided150 inline entries. Its eight-arena registry could terminate on the ninth fresh use. ARENA_REUSE_R1 uses the existing150-entry frame and preserves per-owner spool storage. Older protective exits now write `LEGACY_FATAL` reason/code/state into the same log.

| Current setting | Value / interpretation |
|---|---|
| Vehicle requests / graphics slots |150 /160, with ED/EF sidecars|
| Scratch frame |150 native inline nodes; no external scratch arenas|
| Spool backing |256 private nodes per distinct owner; process-lived; no fixed eight-owner registry|
| Indexed handles / resource collection |262144 /131072|
| Cleanup / pending capacity |262144 /512|
| Audio model registry / global audio slots |192 /2048|
| Shared default audio pool |64MiB|
| Parked model candidates / per-category IDs |150 /150,25 categories; live parked variety validated; nearby population unchanged|
| Full replay snapshot storage |150 traffic IDs /256 retained-reference pairs; test REPLAY150_R1; actual retained game pool unchanged|
| Callback lists |21 groups, 512 nodes each; live peak9, so extra capacity was not exercised|

Request/map/spool counts do not prove 150 simultaneous READY/drawable vehicles. `0059F3A0` is a spatial query. Traffic-setting names do not prove actual chapter-save coverage.

## Current evidence and tooling

- Parked build: [PARKED150_R1_BUILD_2026-11-06.md](analysis/PARKED150_R1_BUILD_2026-11-06.md).
- Latest parent result: [NOTIFY150_RUN29344_RESULTS_2026-11-06.md](analysis/NOTIFY150_RUN29344_RESULTS_2026-11-06.md).
- Targeted live result: [NOTIFY150_RUN7480_RESULTS_2026-11-06.md](analysis/NOTIFY150_RUN7480_RESULTS_2026-11-06.md).
- Previous broad result: [ARENA_REUSE_RUN20236_RESULTS_2026-11-06.md](analysis/ARENA_REUSE_RUN20236_RESULTS_2026-11-06.md).
- Implementation/validation: [ARENA_REUSE_R1_BUILD_2026-11-06.md](analysis/ARENA_REUSE_R1_BUILD_2026-11-06.md).
- Evidence folders: `analysis/arena_reuse_run20236_2026-11-06/` and `analysis/arena_reuse_r1_2026-11-06/`.
- Supplied-log entry point: `python scripts/analyze_callback_log.py <explicit-log> --output <summary.json>`.
- For the current fix, use `python -B scripts/verify_replay150.py`. Parked/notify/arena checks are inherited components. Their historical default output names are not the active build.
- When building is requested: build the active solution once, Release / Win32. Do not build retired variants.

Some historical fixture evidence remains under `analysis/` because current checks require it. See [analysis/README.md](analysis/README.md) and [scripts/README.md](scripts/README.md).

Older projects, ASIs, logs, reports and generated tests are in `archive/retired_2026-11-06/`. Its `manifest.json` records original paths, sizes and SHA256 hashes. The previous handoffs are preserved verbatim under its `notes_before_archive/` folder. C++ source and the working ASI were unchanged by archiving.

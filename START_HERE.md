# DriverSF V29s ? current baseline

Updated 2026-11-14. This file and `NEXT_ACTION.md` describe the current state.
Historical reports' proposed next steps are superseded by the current request.

## Latest live result: READ1M run10556 passes; client3 waits absent

Read `analysis/REPLAY_READ1M_RUN10556_RESULTS_2026-11-14.md`. All READ/CLIENT/
baseline/callback/inherited checks pass across about16 minutes. Private
1 MiB rings active; fallback/fault/rejected-read/drop counters0. Two full
viewings reach201.38s and233.51s and cover the old135–137s problem window.
Client3 busy0 versus seven ordinary waits in30212; all other observed waits
belong to loading transitions. User does not recall the bursting indicator/
choppy motion and reports two full viewings, Story/menu transitions and normal
quit. Rendering/audio/FPS were not explicitly assessed in this reply.

Two backing owners,10 constructions/destructions,8 reuses;both generation5
closed,2 MiB retained payload as designed. Native space/wrap/end stops occur
without successor starvation; do not treat those normal prefetch stops or
low minimum free space as a reason to increase again. Five all-ready sweeps,
audio setup failures0 and final pending/cleanup queues drained/balanced.

Keep READ1M_R1 as the successful replay-test candidate. No new ASI/source/
build/game/debugger changes for log analysis. Longer sessions/other recordings
can broaden confidence; no full-playthrough/seek or universal smoothness
certification. Await concrete next request/log. Older await10556 instructions
below are satisfied.

## Current staged combined test: REPLAY_READ1M_R1; first live pass above

User authorized the combined capacity/counter test. Built Release/Win32 in
the single active project, native/ABI/ownership/parser/compiled/inherited
verified, staged only. Artifact:
`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_READ1M_R1.asi`.
276480 bytes; SHA256
`fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.
Guide `dist/MAIN_150/REPLAY_READ1M_R1_TEST_2026-11-14.md`; report
`analysis/REPLAY_READ1M_R1_BUILD_2026-11-14.md`.

File readers use separate 1 MiB aligned backing, retained per wrapper owner
for the process lifetime and reused only after native destructor/drain.
Writer/memory paths,32 KiB transfers/format/gates/order and all R2 fixes,
150/160/ED/EF remain. Eight targeted hooks count actual reader wait/progress
branches; original narrow CLIENT boundaries include rf* counter views.
No new read/poll call, full-manager hot scan or game-thread log writing.
Three old staged ASIs unchanged. Ordinary allocation failure keeps native
arguments and records an explicit fallback, invalidating this comparison.

Use one traffic ASI. Log `%TEMP%\DSFV29s_replay_read1m_<PID>.log`; analyze
baseline/callback/CLIENT and `scripts/analyze_replay_read1m_log.py` before any
further patch. Test known choppy replay normally to end before rewind,
another Story/Replay/menu cycle, menu wait and normal quit. No agent game
install/run; live smoothness, overhead and lifetime remain unverified.
Active output `build/v29s_replay_read1m_r1/`. Older static-next statements
below are satisfied by this build; await concrete user log/request.

## Latest static audit: replay ring retention/lookahead is the next test target

Read `analysis/REPLAY_REFILL_STATIC_2026-11-14.md`. Reader746940 shares a
256 KiB ring across tracks, retains current samples and reads one 32 KiB
block per invocation. Missing next samples can follow earlier retention
pressure despite later free space. Captured adjacent sample layout spans
total about337 KiB; supporting evidence, not exact file bytes or a prior
full-ring measurement. Actual last reader return reason remains unmeasured.

A controlled 1 MiB file-reader ring with matching separate backing is now
justified as the next experiment, only if requested. Keep native32 KiB I/O,
sampler, gates/order, writer/memory paths,150/160/ED/EF and all R2 fixes.
Some native backing partitions coexist; a size-immediate-only change is
unsafe. Owner/generation/drain/failure/reconstruction handling is required.
Combine narrow client captures with bounded reader stop/progress counters.
No new general identity session is needed first; live validation is required
after a test build. Static byte/data checks pass; three staged ASIs unchanged.
Analysis/docs only, no runtime source/build/game/debugger changes this task.
Older recommendations below are superseded by this audit where applicable.

## Latest live result: run30212 identifies client3 following-sample waits

Read `analysis/REPLAY_CLIENT_RUN30212_RESULTS_2026-11-14.md`. All three
analyzers pass. Five normal-playback waits at135.26–136.95s are client3
current-valid/next-absent,15–23ms each; clear supplies the following sample.
User confirms the2:15 indicator episode cleared around2:17, did not recur,
playback reached the end and the game quit normally. No ordinary-playback
checkpoint/restore or vehicle/player-client starvation explains this cluster.

Actual backend is file reader,256KiB ring,32KiB blocks. Busy snapshots retain
105–168KiB free: no simple full-ring condition there. Raw read bookkeeping
is0 before and32768 after clear; file position progresses2–3 blocks. These
are not disk-latency measurements and do not exclude earlier retention pressure.
Next static target is reader746940 lookahead/refill/whole-record assembly and
updater73A090 ordering. No new identity probe or blind capacity increase is
needed. Any fix needs a later live validation. No new ASI/source/build here;
150/160/ED/EF and R2 fixes remain. Older await-log statements are satisfied.

## Staged diagnostic: REPLAY_CLIENT_R1; run30212 analyzed above

User authorized the next probe. Built Release/Win32, native/data/installer/
parser and compiled/inherited R2 verified. Staged only, no game install/run:
`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_CLIENT_R1.asi`.
265216 bytes; SHA256
`d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3`.
Guide: `dist/MAIN_150/REPLAY_CLIENT_R1_TEST_2026-11-14.md`.
Report: `analysis/REPLAY_CLIENT_R1_BUILD_2026-11-14.md`.

Three hooks observe existing busy predicate returns for clients1/2/3/7.
Normal all-clear path has no new C++/QPC/readability/FXSAVE. Captures occur
at busy/clear/context boundaries and held heartbeats, with client identity,
sample horizon, backend/ring/in-flight/end state; monitor-only new writes.
Input self is separately saved: vehicle predicate can return timestamp
scratch in ECX. Broad HITCH hooks/scopes are not compiled. All R2 fixes,
150/160/ED/EF/capacities retained; old artifacts unchanged.

Use exactly one traffic ASI and the same setup as28136. Fresh Story recording,
normal1x playback through the end before rewind, then normal quit/menu wait.
Log `%TEMP%\DSFV29s_replay_client_<PID>.log` copied to MAIN_150 by user.
Analyze supplied log first. No live client identity/full-buffer cause or
overhead reduction is yet certified. Active output `build/v29s_replay_client_r1/`.

## Latest static follow-up: four built-in +20 candidates identified

Read `analysis/REPLAY_SUBSCRIBER_STATIC_2026-11-14.md`. Registration has11
built-in clients (IDs0..10); seven +20 methods always return false. Candidates:
vehicle1/755DF0 (missing/expired future data or flag34), player2/7523B0
(missing current event), clients3 and7/shared7549D0 (missing current/next).
Client3 has a0.25s recording timer, matching cadence but not proving identity.
Run28136 lacks per-client results; a narrow live identity/horizon/ring-state
capture is needed before choosing a fix. Do not repeat the broad hot-path
full-view probe. No new ASI/source/build in this static task.

File replay paths pass256KiB ring capacity and use32KiB I/O blocks. Fixed
providers return neighboring partitions at base and base+40000; a size-only
increase could overlap them. No actual occupancy/fullness is known from28136.
The13 registry buckets and12 inline nodes are client registry bookkeeping,
not per-vehicle limits to raise to150. All150/160/ED/EF and R2 fixes retained.

## Latest live probe: run28136 late subscriber-readiness cluster

Read `analysis/REPLAY_HITCH_RUN28136_RESULTS_2026-11-14.md`. User confirms
fresh Story replay, normal-speed viewing until the end, then rewind and
normal quit. All baseline/callback/probe integrity checks pass, no drops.
Near replay2:25–2:32,29 observed +20 busy intervals recur about250ms apart,
11–62ms each, with idle snapshot/traffic fallback. Clock gate+24 never busy;
all960 steps in the late8s advance. No seek/restore/replay150 request occurs
during first playback. Later restore bursts match user rewind after the end.

Static caller0053AB23 can skip native service updates on +20 busy while
still dispatching the replay step; steady replay time does not rule this out.
Actual subscriber/caller identity and UI trigger remain unmeasured. Next
target is subscriber virtual+20 registration/readiness, not another limit.
Probe overhead is significant (~0.12s observer-body elapsed per replay sec);
any further live identity probe should be narrower. No new ASI/source/build
for this log analysis. REPLAY_HITCH_R1 remains diagnostic, R2 fixes retained.

## Current staged probe: REPLAY_HITCH_R1; live timing capture pending

User authorized the passive replay timing test. Built Release/Win32 in the
single active ARENA_REUSE project, fixture/compiled/inherited verified and
staged `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_HITCH_R1.asi`.
270848 bytes; SHA256
`758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8`.
Test guide: `dist/MAIN_150/REPLAY_HITCH_R1_TEST_2026-11-14.md`.
Report: `analysis/REPLAY_HITCH_R1_BUILD_2026-11-14.md`.

Five passive native entry hooks measure distinct busy gates, replay steps,
checkpoint requests and recording stream reposition. Conditional scopes time
the existing update/import/restore/request. Bounded queue, monitor-only new
event writes, no duplicated engine calls or readiness bypass. All R2 fixes,
150 requests/160 slots, ED/EF and capacities remain; R2 artifact unchanged.
Probe is staged only, not installed/run. Use exactly one traffic ASI.
Log `%TEMP%\DSFV29s_replay_hitch_<PID>.log`; analyze the next supplied log
before any fix. Current build output `build/v29s_replay_hitch_r1/`.
No live root cause, smoothness improvement or probe overhead is established.

## Latest follow-up: replay hitch static trace complete

Read `analysis/REPLAY_HITCH_STATIC_2026-11-14.md`. Native replay time can be
held by subscriber/traffic/snapshot readiness even when sampled loading=0.
Checkpoint restore also repositions a separate recording stream through
00746000; requested/timeline simulation rates are another independent path.
The responsible branch and wait durations are not identified by the1s log.
No new exhausted capacity or safe restore deduplication was established.
Next useful live evidence is a passive event/duration probe if requested;
do not build automatically. Analysis evidence/docs only; runtime source,
R2 ASI,150 requests/160 slots and all inherited fixes remain unchanged.

## Latest live R2 result: run24888 map/lifecycle pass; replay motion hitches

User confirms correct variety/rendering/audio, repeated Story/Replay/menu
transitions and normal exit. Log10m48.563s: five distinct map owners, six
constructions, one same-address generation2/reattachment, six redirects,
900 checks, external peak105, faults0. Both formerly failing map paths were
actually exercised. Eight settings expanded to150, no asset refusal; all87
accepted rosters canonical150,52 native-ready sweeps. Snapshot46 saves/79
restores preserve150/hash/181. Four paused loading completions succeeded;
final pending/cleanup empty and balanced. Both analyzers pass.

User also reports intermittent loading indicators and replay-motion stutter
during **normal playback**: rendering stays~100FPS, recorded motion feels~50FPS.
Do not claim smooth playback is fixed. One-second telemetry misses short
waits;79 identical-roster restores include bursts but do not prove redundant
loading. Next scope: replay advancement/loading timing and native gates,
without raising limits or skipping restores from hash equality alone.
Report: `analysis/REGMAP_REUSE_RUN24888_RESULTS_2026-11-14.md`.
No new source/ASI/build in this log-analysis task. R2 is the tested map-exit/
transition baseline; broader chapter coverage/Forza remain separate.

## Latest dump finding: Forza setup / vanilla roster mismatch

Supplied Driver.DMP is PID21280 from **Driver San Francisco SS Test**. User
confirmed it is the Forza-car setup. Its AI SHA957d7eb6... differs from the
Save Bug Test fingerprint434e1a51.... Captured rows match the SS Test file;
all AI arrays are readable. Exact ChapterDataMatches failure is row weight/
ID content, so native18 requests are retained. None of the selected tables'
150 positive IDs is in the27-key loaded definition map. The stock native
lookup falls back to first eligible key15, consistent with the observed Mini.
Map guards/counts agree, no external backing/reset/fault is exercised.

Report: `analysis/REGMAP_REUSE_DUMP21280_FORZA_MISMATCH_2026-11-14.md`.
Next: vanilla R2 verification in **Save Bug Test**, using its expected AI
file and one traffic ASI. Forza needs separate roster/profile/ID compatibility
work; do not bypass the fingerprint or increase limits. No source/ASI/build
change in this dump analysis. Driver.DMP now contains PID21280, not8944.
The earlier log-only result below is superseded on the active-installation
and mismatch-cause questions; vanilla R2 lifecycle remains unverified.

## Latest R2 result: startup works; Mini-only civilian traffic reported

Run21280 passes all startup checks, but user reports all moving civilian
traffic became Mini Cooper S on the first Story load, same files as before.
Captured log prefix has Chapter1/7 assetMismatch and only18 requested IDs,
no150 expansion. One map owner/41 checks/13 recycled uses, no external
redirect/reset/fault. Disk AI hash and Chapter1/7 row/route fingerprints match
the expected adjusted asset. The loaded-data refusal predicate and actual
selection/fallback cause are unknown; do not assume changed files or increase
limits. Report: `analysis/REGMAP_REUSE_RUN21280_TRAFFIC_2026-11-14.md`.
Next evidence is a matching full dump while Mini-only traffic is present.
R2 is a diagnostic test with a reported selection regression, not a validated
normal-use baseline. No code/build change in this analysis turn.

## Current staged test: REGMAP_REUSE_R2 (startup first)

R1 is startup-blocked in the user's game. Logs11908/36608 report map installer
refusal/baselineStatus-16 then downstream callback fatal. The real production
Readable predicate rejects its constructor signature across adjacent RX/RWX
regions during the installer's own protection change. The old fixture missed
this. R2 checks page-bounded readable coverage and retains exact byte checks.
The real VirtualQuery/RX fixture reproduces R1 and passes R2, guards/rollback,
all native lifecycle tests and compiled/inherited verification.

- Staged: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.asi`.
-253440 bytes; SHA256 `8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa`.
- Guide: `dist/MAIN_150/REGMAP_REUSE_R2_TEST_2026-11-14.md`.
- Report: `analysis/REGMAP_REUSE_R2_STARTUP_FIX_2026-11-14.md`.
- Active Release/Win32 output: `build/v29s_regmap_reuse_r2/`.
- Verifier: `python -B scripts/verify_regmap_reuse.py`.
- Log remains `%TEMP%\DSFV29s_regmap_reuse_<PID>.log`; startupCheckRevision=2.

Use only R2 as the traffic ASI. First validate main-menu startup, then Story/
Replay/menu/control transitions. All existing fixes/capacities are retained.
Staged only; no agent install/game run. R1 and earlier artifacts are preserved
as evidence, not recommended tests. Earlier sections below are historical.

## Rejected parent: REGMAP_REUSE_R1

User-authorized definition-map lifecycle fix built once Release/Win32,
native-fixture/compiled/inherited verified and staged. Removes the fixed
four-owner directory and recognizes same-address reconstruction through the
native constructor.512 external map nodes and every parent capacity/fix are
preserved. Eight owners/100 reconstruct cycles pass the native fixture;
live repeated Story/Replay/menu transitions and playback controls are pending.

- ASI: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R1.asi`.
-252928 bytes; SHA256 `7bec339a6bc22d3ba599fd8533863604d20da23a4ed82add357e59e724b93f9b`.
- Guide: `dist/MAIN_150/REGMAP_REUSE_R1_TEST_2026-11-12.md`.
- Log: `%TEMP%\DSFV29s_regmap_reuse_<PID>.log`.
- Report: `analysis/REGMAP_REUSE_R1_BUILD_2026-11-12.md`.
- Verifier: `python -B scripts/verify_regmap_reuse.py`.
- Active solution still contains one project; new output under
  `build/v29s_regmap_reuse_r1/`. Exactly one traffic ASI enabled in the game.

Staged only; no agent installation or game run. Check REGMAP_START status1/
hooks2, faults0 and actual owner/reconstruct/reattach exercise. A startup pass
alone does not certify transitions. Earlier analysis/test sections below are
retained context and do not supersede this staged test.

## Prior finding: definition-map static lifetime audit

`analysis/DEFINITION_MAP_STATIC_2026-11-12.md` traces native type0x1DE
create/destroy, constructor0086BFA0, map insert/search/remove and consumers.
Two ASI lifecycle defects are established: four lifetime owner records, and
rejection of valid inline cursors after same-address reconstruction. The
run19868 fatal does not identify which branch fired. A safe correction can
be implemented now: dynamic stable owner records plus explicit constructor
generation handling, retaining512 external nodes and overwrite protection.
No production source/project/ASI/build changed. Live verification follows
implementation; no preliminary live probe/debugger is required.

## Latest supplied result: REPLAY_PAUSE run19868

The game explicitly exited via LEGACY_FATAL vehicleDefinitionMap/code0x31
at462641ms. The older map extension has four lifetime owner slots/512 extra
nodes each; owner exhaustion, node exhaustion or an unexpected cursor share
this fatal. The log does not distinguish them. Do not equate spoolArenas4
or default cursor0 with captured definition-map state. The new pause service
completed loading twice, preserving pause/rate/clock in both events. This run
is not a clean pass. Next focus is definition-map lifetime bookkeeping and
branch telemetry, retaining all current fixes/capacities. No source/build
changes were made for this analysis. See
`analysis/REPLAY_PAUSE_RUN19868_RESULTS_2026-11-12.md`.

## Preserved parent: REPLAY_PAUSE_R1

User authorized the pause-safe completion adjustment. It is built, fixture/
compiled/inherited verified and staged; run19868 exercised two live completions
but ended in the older definition-map guard above. One new hook
at0073A090 runs the original updater first, then services the original native
007390BE completion block only for a matching imported, paused loading state.
Native readiness remains in control. The service does not force unpause,
advance physics/replay time, or call engine functions from the monitor.

- Staged: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY_PAUSE_R1.asi`.
- Size244224 bytes; SHA256 `2847467ed26424b6c17773b2c0bd179b6218fbca1f019dc97a50feb86e2c083f`.
- Guide: `dist/MAIN_150/REPLAY_PAUSE_R1_TEST_2026-11-11.md`.
- Log: `%TEMP%\DSFV29s_replay_pause_<PID>.log`.
- Verify: `python -B scripts/verify_replay_pause.py`.
- Report: `analysis/REPLAY_PAUSE_R1_BUILD_2026-11-11.md`.
- Active `.slnx`/project builds REPLAY_PAUSE_R1 only, Release/Win32.
- Staged only; no installation/game run by the agent.

Preserves REPLAY150_R1's full snapshot correction and every inherited capacity.
Retest the failing first Story->Replay entry, retained/fresh recordings,
pause/play/seek controls, then Story/Replay and menu reloads with normal exit.
Use exactly one traffic ASI. Analyze supplied logs with both baseline150 and
callback analyzers. Completed recovery events show service use; live loading
and playback success still require the user's observations. The original
zero-rate caller/order and separate null simulation-manager pause crash remain
unproven/unpatched.

## Snapshot parent: REPLAY150_R1

Preserved artifact: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REPLAY150_R1.asi`,
234496 bytes, SHA256 `2ab7f763f216abfaf5091308f0adb3e428512038d2f0384432faa140aae12dd9`.
Its seven adapters preserve150 IDs/256 reference records while retaining the
native612-byte views/624-byte embedded object and recording format. Run10520
passed its first replay workload; run8944 exposed the remaining paused-loading
block. Keep the snapshot fix. Build: `analysis/REPLAY150_R1_BUILD_2026-11-07.md`.

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
REPLAY_PAUSE_R1 now stages that guarded game-thread completion service;
live validation of the correction is pending.
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
- For the current fix, use `python -B scripts/verify_replay_pause.py`. Parked/notify/arena checks are inherited components. Their historical default output names are not the active build.
- When building is requested: build the active solution once, Release / Win32. Do not build retired variants.

Some historical fixture evidence remains under `analysis/` because current checks require it. See [analysis/README.md](analysis/README.md) and [scripts/README.md](scripts/README.md).

Older projects, ASIs, logs, reports and generated tests are in `archive/retired_2026-11-06/`. Its `manifest.json` records original paths, sizes and SHA256 hashes. The previous handoffs are preserved verbatim under its `notes_before_archive/` folder. C++ source and the working ASI were unchanged by archiving.

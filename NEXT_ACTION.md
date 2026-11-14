# Next action ? work only on the next concrete request

## READ1M run10556 analyzed: retain candidate, await concrete next request/log

Report `analysis/REPLAY_READ1M_RUN10556_RESULTS_2026-11-14.md`; evidence
`analysis/replay_read1m_run10556_2026-11-14/`. All analyzers pass. Actual
private1 MiB rings, no fallbacks/faults/rejected reads/drops. Two full replay
viewings cover old135–137s range; client3 busy0 and no ordinary-playback
waits. Earlier30212 had seven client3 waits. User does not recall visible
bursts/choppiness; reports menu -> Replay(full) -> menu -> Story -> Replay
(full) -> menu -> normal quit. No explicit current audio/render/FPS assessment.

Backing2 owners,10 constructors/destructors,8 reuses,closed generation5 each;
2 MiB retained as designed. Ordinary space/wrap/end stops are expected
prefetch/backpressure, not a reason to enlarge again. Native queues drain
and inherited counters stay clean. This is a first live pass, not universal
smoothness/full-campaign/rewind certification or a controlled frame-time A/B.

No new ASI/source/build in analysis. Keep READ1M_R1 for continued use/testing;
do not automatically patch scheduling, increase capacity or build another
probe. Additional recordings/longer use can expand confidence if requested.
The supplied-log procedure below is now satisfied for10556.

## REPLAY_READ1M_R1 supplied-log procedure retained for future runs

Guide `dist/MAIN_150/REPLAY_READ1M_R1_TEST_2026-11-14.md`; report
`analysis/REPLAY_READ1M_R1_BUILD_2026-11-14.md`. New ASI276480 bytes, SHA256
`fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.
Only one traffic ASI enabled by the user. Keep current150-car AI/settings;
normal1x playback through end before controls, then another Story/Replay/menu
cycle and normal exit. Log `%TEMP%\DSFV29s_replay_read1m_<PID>.log`.

Analyze baseline150/callback, narrow CLIENT and new READ integrity first:
`python -B scripts/analyze_replay_read1m_log.py <log> --output <json>`.
Require READ_START status1/readerBytes1048576/blockBytes32768 and zero
allocationFailed/fallbacks/faults/issueRejected. Managed file boundaries
must match base==rfBuffer and capacity/rfCapacity1048576. Distinguish native
memory-reader unmanagedCalls from actual fallback/rejection errors.

Compare client3 missing-next waits/replay time with rf* busy/commit/wrap/
space/issue/end deltas and last-stop state. Stops alone can be normal
prefetch behavior. Counters persist per owner; minFree/maxRequired/lastStop
reset on managed construction. Snapshot rows are not a globally coherent
instant. Retained backing per owner is expected until process exit, including
after native destructor marks it closed; reuse/generation must be checked.

Offline native/ABI/installer/failure/guard/100-generation/>4-owner/parser/
compiled/inherited checks pass. No installation/game/debugger run. Old R2/
HITCH/CLIENT artifacts unchanged. Active output `build/v29s_replay_read1m_r1/`.
Do not build or enlarge again automatically. If waits remain despite prior
headroom/no space stops, investigate scheduling/completion. Live benefit,
total observer overhead and real transition stability remain unverified.

## Static refill audit complete; await a concrete next implementation request

Report `analysis/REPLAY_REFILL_STATIC_2026-11-14.md`; new evidence and byte/
data checks in `analysis/replay_refill_static_2026-11-14/`. No ASI/source/build
change. File ring capacity is a viable controlled test: retention can limit
lookahead before client advancement, then leave ample free space at busy.
Adjacent measured sample-layout intervals total337 KiB against256 KiB;
not an exact byte-rate/occupancy measurement or proof of prior space stop.

If requested, prepare 1 MiB reader-only backing plus capacity, retaining
32 KiB transfers/format/gates/update order and narrow client telemetry.
Add bounded backend-busy/free-space/wrap-space/issue/progress counters so a
failed capacity test identifies which mechanism remains. Do not increase
the block size or change sampler/readiness,150/160/ED/EF or inherited fixes.
Five file constructors include three readers and two writers; do not change
all indiscriminately. C00 exclusive and CB0/D60 shared leases have different
ownership. A 1 MiB ring needs separate backing with native leases retained
and correct generation/drain/failure/reconstruction/shutdown management.
Details and teardown addresses are in the report. Do not relocate live data.

No new general live probe is required before implementation; validate the
result later with normal playback and repeated transitions/normal exit.
If ample prior headroom/no space stops still accompany waits, follow refill
scheduling/completion rather than increasing capacity again. Runtime cause
is not certified by static analysis alone. No build/run without request.

## Run30212 analyzed: next target is file-reader lookahead/refill policy

Report `analysis/REPLAY_CLIENT_RUN30212_RESULTS_2026-11-14.md`; evidence in
`analysis/replay_client_run30212_2026-11-14/`. All baseline/callback/client
integrity checks pass. Actual client3 lacks its following recorded sample
in five15–23ms waits at135.26–136.95s. User confirms transient2:15–2:17
indicator episode, no recurrence, playback to end and normal quit.

File reader has256KiB ring with105–168KiB free at busy boundaries, end0,
inFlight0 before/32768 after clear. Do not call this full-pool exhaustion or
measured slow disk; earlier retention/insufficient lookahead remains possible.
Focus a requested static fix-design audit on746940 whole-record assembly,
32KiB issue/completion cadence,746BF0 retention and73A090 client-delivery/
tail-refill ordering. Capture does not identify the partial packet type or
establish a safe scheduling change. Fixed neighboring backing makes a lone
capacity-constant increase unsafe. No readiness bypass/sampler change.

No new ASI/source/build in this analysis. Wait for concrete next work rather
than building automatically. Another broad identity probe is unnecessary;
client3 is now measured. Any runtime fix still requires live validation.
Earlier await-CLIENT-log instructions below are satisfied by30212.

## REPLAY_CLIENT_R1 procedure retained for reference; first log analyzed above

User-authorized narrow probe is built, offline verified and staged only.
Guide `dist/MAIN_150/REPLAY_CLIENT_R1_TEST_2026-11-14.md`; report
`analysis/REPLAY_CLIENT_R1_BUILD_2026-11-14.md`. ASI265216 bytes, SHA
`d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3`.
Use exactly one traffic ASI and retain the setup from28136. Make a fresh
recording, watch normal1x through its ending (particularly last20–30s)
before controls, then menu wait~3s and normal quit.

New log `%TEMP%\DSFV29s_replay_client_<PID>.log`, copied by user to MAIN_150.
Run baseline150/callback integrity and `scripts/analyze_replay_client_log.py`.
Require CLIENT_START status1/hooks3/broadHooks0 and zero dropped/thread/
unknown/read/clock errors. Distinguish busy/clear/held/contextEnd and final
unfinished waits; correlate client ID with current/future sample timestamps,
requested clock, backend, ring free/unparsed bytes, in-flight count and end
flag. These are post-result snapshots; raw I/O fields are not completion calls.

The broad timing hooks/scopes are absent. Original predicates run once;
saved self is separate from native output ECX, which the vehicle predicate
can use as scratch. Normal all-clear path has no observer C++/QPC/VQ/FXSAVE.
No capacities or decisions changed. Live identity, occupancy and overhead
remain unmeasured. Do not raise a buffer/slot limit or bypass readiness based
only on the0.25s cadence. No game install/run by the agent.

Active output `build/v29s_replay_client_r1/`; verifier
`scripts/verify_replay_client.py` supports fixture/build/package phases.
Earlier "no new probe built" sections below are superseded.

## Subscriber static trace complete; targeted live identity is next

Report: `analysis/REPLAY_SUBSCRIBER_STATIC_2026-11-14.md`; evidence in
`analysis/replay_subscriber_static_2026-11-14/`. User confirms rapid bursts
and loading indicator blink together. Four built-in candidates remain:
IDs1(vehicle),2(player),3(two-position track),7(28-byte sampled track).
Their +20 implementations755DF0/7523B0/7549D0 test recorded-data availability;
vehicle1 also returns flag34, but its +24=754FF0 never reported busy in28136.
Client3's0.25s sampler matches the burst cadence; actual live identity is
unproven. Do not equate timing correlation with client3 identification.

If a new probe is requested, make it substantially lighter: observe original
busy callback results/caller/object, event pointer/timestamp horizon and
recording backend/ring occupancy/in-flight/end state on busy boundaries.
Three readiness implementations cover four native candidates. Avoid all
old per-step/update full-view scans, extra native calls and file writes on
the game thread. No probe was built in this static task.

Replay reader746940 uses32KiB blocks; file-wrapper setup passes256KiB ring
capacity. Native providers736C00/CB0/D60 return fixed partitions (D60 at
base+40000) rather than allocating the requested size. Do not increase a
constructor size alone: neighboring backing must also be addressed if
fullness is actually confirmed. Registry13 buckets/12 inline nodes contain
11 built-in clients, not150 vehicle entries. Preserve capacities/gates/order.

## Run28136 analyzed: next focus is subscriber virtual+20 readiness

Report `analysis/REPLAY_HITCH_RUN28136_RESULTS_2026-11-14.md`; evidence in
`analysis/replay_hitch_run28136_2026-11-14/`. User confirms fresh replay,
first normal viewing -> end -> rewind comparison -> normal quit. All three
analyzers pass. First viewing has no checkpoint/restore/request calls.
Late replay144.94–151.87s has29 completed +20 busy intervals (median22.3ms,
spacing250ms), with snapshot/traffic fallback idle. Native +24 clock gate
is never busy; late960/960 replay steps advance.

Static0053AA20 calls+20 at0053AB23; true skips0053AB30–0053ABF7 service
updates but still dispatches the replay step at0053AC03.00799794 can also
skip a downstream virtual call. This is a concrete candidate mechanism,
not live identification of the caller/subscriber or proof of153 skipped
simulation ticks. Next scoped static target: subscriber registration/vtables
and virtual+20 queue/buffer readiness. If identity needs live data, propose
a narrower, lighter passive observer of the existing virtual call.

Current broad probe has significant observer overhead; preserve this caveat
when interpreting absolute durations. No pool exhaustion/repeated restore
cause is shown for first playback. Do not raise capacities, bypass gates,
change150/160/ED/EF, build or launch a new test without a concrete request.
Earlier await-log instructions below are satisfied for28136.

## Await user REPLAY_HITCH_R1 normal-playback log

The user-authorized probe is built, offline verified and staged; no game
install/run. Use only the REPLAY_HITCH_R1 traffic ASI and keep the working
vanilla150 AI/settings. Guide:
`dist/MAIN_150/REPLAY_HITCH_R1_TEST_2026-11-14.md`.
Report: `analysis/REPLAY_HITCH_R1_BUILD_2026-11-14.md`.
First useful capture: Story3–5min -> normal1x Replay2–3min with no seeking/
rewinding, menu wait~3s and normal quit. Existing known-choppy replay is
also useful if explicitly identified; do not delete recordings.

Log `%TEMP%\DSFV29s_replay_hitch_<PID>.log`, copied to MAIN_150 by user.
Analyze baseline150/callback integrity plus
`python -B scripts/analyze_replay_hitch_log.py <log> --output <json>`.
Require HITCH_START status1, QPC frequency>0 and zero drops/thread/depth/
nesting/clock failures. Correlate symptom times with gateWait (QPC ticks),
hold/recovered (milliseconds), checkpoint caller/target, recording-stream
duration, phases, rates and step/update gaps. Pending fallback counters are
not individual subscriber identification; nested times are not additive.
No timing result yet justifies raising limits or skipping equal-hash restores.

Active Release/Win32 output `build/v29s_replay_hitch_r1/`; verifier
`scripts/verify_replay_hitch.py` supports fixture/build/package phases.
All R2 capacities/fixes remain,150 requests/160 slots/ED/EF preserved.
Earlier "probe not built" and static-next-step sections below are superseded.

## Static replay-hitch investigation complete; timing probe not built

Report: `analysis/REPLAY_HITCH_STATIC_2026-11-14.md`; disassembly evidence in
`analysis/replay_hitch_static_2026-11-14/`. 00738F20 gates the replay clock
through00737A10 (subscriber virtual+24, then snapshot/traffic readiness),
independently of the sampled loading flag. 007379D0 uses virtual+20 and is
a different gate. Checkpoint helper00739F60 and transitions in0073A090 can
reposition recording buffers through00746000; not all callers are user seeks.
Requested/timeline speed and simulation pause/rate are separate variables.

No responsible live branch or new capacity exhaustion is proven. If the user
requests a probe, measure existing gate returns, clock/rate/phase dwell,
checkpoint caller/target and native reposition/import/restore/request time.
Use bounded aggregation, game-thread observations, no extra native helper
calls or per-frame file I/O. Preserve all readiness gates/capacities/order;
do not skip restores on equal roster hashes. No ASI/source/build change in
the static investigation; R2 remains the tested transition baseline.

## Current follow-up: replay-motion/loading hitches after run24888 pass

Run24888 verifies R2's fifth owner and same-address reconstruction/reattachment
live; no faults. User confirms variety/audio/rendering, repeated Story/Replay/
menu transitions and normal quit. Report:
`analysis/REGMAP_REUSE_RUN24888_RESULTS_2026-11-14.md`.

User qualifies playback: loading indicators intermittently appear during
normal playback; actual game rendering~100FPS, recorded replay motion feels
~50FPS/stutters. This is the current unresolved issue. Existing1s samples do
not measure short waits/frame times/rates/seek targets.79 restores all preserve
the same150 roster; bursts are candidates to time, not evidence to skip them.
No capacity exhaustion appears. Next scoped static trace should identify
replay time-advance/snapshot/wait gates; event-based timing may follow if a
probe is requested. Do not build automatically, raise limits, unconditionally
unpause/bypass readiness, or deduplicate on roster hash alone.

All existing fixes/capacities remain. No source/ASI/build changes in the latest
analysis. Earlier requests for a vanilla comparison below are now satisfied
for this reported workload; Forza adaptation stays separate.

## Current finding: dump21280 explains Forza Mini fallback

Read `analysis/REGMAP_REUSE_DUMP21280_FORZA_MISMATCH_2026-11-14.md`.
Driver.DMP is PID21280 from SS Test; user confirmed Forza setup. Loaded AI
rows match that installation's different disk asset; single-region Readable
checks pass. Vanilla fingerprint rejects weight/ID content and requests only
18. None of the150 positive Forza IDs is in the27-key definition map; the
unchanged native0086B9E0 fallback returns first eligible key15. Map guards/
issued counts are intact; external backing/reset path remains unexercised.

Recommended next live comparison: R2 in **Save Bug Test**, expected vanilla
AI and exactly one traffic ASI, verify150 expansion/variety then lifecycle
transitions. User offered to return there. Do not begin Forza adaptation or
raise limits without a concrete request; vanilla fingerprints/roster are still
specific to the current150 baseline. No patch/build for this analysis.
Do not reuse the PID8944 dump script on the overwritten Driver.DMP. Earlier
"dump needed" instructions below are superseded; evidence is now analyzed.

## Current issue: R2 Mini-only traffic, dump needed

Read `analysis/REGMAP_REUSE_RUN21280_TRAFFIC_2026-11-14.md`. Startup now
succeeds, but the user reports all moving civilian traffic became Mini Cooper
S on first Story entry, same files. Snapshot21280 records assetMismatch for
Chapter1/7, effective18 and no expanded150 request. On-disk AI hash and relevant
profile/weight/route fingerprints match the expected adjusted asset. The
loaded-data predicate is not logged. One map owner, no external redirect/reset
or fault; this does not exonerate the observer/guard categorically.

Need a matching full dump while Mini-only traffic is present to inspect actual
AI pointers/counts/readability/rows and selection/definition state. Preserve
earlier dumps with a distinct filename. Do not change probabilities, disable
validation, raise limits, or call R2 validated. No new ASI/source/build in this
analysis. The older startup instructions below are superseded by this issue.

## Immediate next live check: REGMAP_REUSE_R2 startup

R1 prevents startup in the user's report. Its map preflight returns false
(baselineStatus-16) before runtime; downstream callback refusal exits.
R2 fixes the reproduced split-region readability bug. Use only
`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.asi`.
Guide: `dist/MAIN_150/REGMAP_REUSE_R2_TEST_2026-11-14.md`; report:
`analysis/REGMAP_REUSE_R2_STARTUP_FIX_2026-11-14.md`. Native fixture, Release/
Win32 build, compiled/inherited checks pass; actual startup remains pending.

First reach the main menu. Analyze the new regmap_reuse log; successful
REGMAP_START should include startupCheckRevision=2. Installer refusals now
write REGMAP_INSTALL_FAIL with the stage/site. Baseline/callback analyzers
support the short fatal-only logs. Only after startup succeeds, proceed with
the repeated Story/Replay/menu/control/normal-exit test. No new limits needed.

Earlier R1 instructions below are historical; do not retry its startup-blocked
binary. No game installation/run by the agent. All capacities/fixes retained.

## Superseded live plan: REGMAP_REUSE_R1

The authorized fix is built/verified/staged; use
`dist/MAIN_150/REGMAP_REUSE_R1_TEST_2026-11-12.md`. No game install/run by the
agent. Retain AI assets/settings and recordings; use exactly one traffic ASI.
Repeat Story/Replay round trips, menu/Story reload and normal exit, ideally
at least six world loads. Include the transition that ended run19868.

New log `%TEMP%\DSFV29s_regmap_reuse_<PID>.log`. Analyze it with baseline150
and callback analyzers; require all startup successes including REGMAP_START
status1/hooks2, faults0/allocationFailed0 and no fatal. Check owners>4 for the
old owner-cap path, reconstructs/reattaches for same-address generations.
Loads do not directly equal distinct owner addresses. Compare observed
rendering/audio, loading/playback controls and exit outcome with telemetry.
Report: `analysis/REGMAP_REUSE_R1_BUILD_2026-11-12.md`; verifier:
`python -B scripts/verify_regmap_reuse.py`. All current capacities, snapshot
and pause completion fixes remain. No need for unrelated limit changes.

Earlier sections below preserve prior reasoning; the implementation is now
complete. Live stability remains pending; exact run19868 branch is unknown.

## Prior static audit (implemented in REGMAP_REUSE_R1)

Current report: `analysis/DEFINITION_MAP_STATIC_2026-11-12.md`; evidence in
`analysis/definition_map_static_2026-11-12/`. Native type0x1DE registries are
constructed/destroyed during world setup/teardown. The ASI's four lifetime
owner records and same-address inline-cursor rejection are separate defects.
Recommend stable dynamically allocated owner records and a post-constructor
generation observer, preserving512 external nodes and corruption protection.
No production patch/build in this static-analysis task. The branch model is
not a native fixture or proof of run19868's exact branch. Implementation
needs native lifecycle/allocator fixtures, inherited verification and then
live repeated transitions. Keep current capacities and replay fixes.

## Latest analysis: run19868 definition-map process exit

Read `analysis/REPLAY_PAUSE_RUN19868_RESULTS_2026-11-12.md`. The supplied
REPLAY_PAUSE log ends in explicit vehicleDefinitionMap ExitProcess(0x31),
not a silent unknown exit. Its older extension has four lifetime map-owner
entries/512 external nodes each. Exact fault branch is not logged; four-owner
exhaustion is a candidate, not established by spoolArenas4. Next scoped work
is native map lifetime/reset/consumers, safe owner storage/reuse and precise
fault telemetry. Retain overwrite protection and current replay/capacity
fixes. No new build/source patch authorized or made in this analysis turn.
The pause completion service completed twice; this does not make the failed
session a clean pass. User transition details are pending.

## Parent test: REPLAY_PAUSE_R1 live completion/playback validation

The user-authorized adjustment is built/staged, with native fixture, compiled
and inherited checks passing. Follow
`dist/MAIN_150/REPLAY_PAUSE_R1_TEST_2026-11-11.md`. Active Release/Win32 output is
REPLAY_PAUSE_R1; use exactly one traffic ASI. No agent install/game execution.

Require REPLAY_PAUSE_START status1/hooks1 and all parent startup markers.
Check recovery EVENT kind=completed, nativeHeld retries, guard skips and
original snapshot150/181/hash preservation. Completed means loading was cleared
by the native block, not that user-visible controls are universally validated.
Retest first Story->Replay entry, retained and fresh recordings, play/pause/
resume/seek, repeated Story/Replay, menu reload and normal exit. Do not delete
recordings or add unrelated limit changes. Analyze the supplied log first.

Guide/report: `analysis/REPLAY_PAUSE_R1_BUILD_2026-11-11.md`.
Evidence: `analysis/replay_pause_r1_2026-11-11/verification.json`, fixture.json,
parser.json, package.json and inherited checks. Current verifier:
`python -B scripts/verify_replay_pause.py`. Snapshot150/256,150 requests,
160 graphics and ED/EF capacities are preserved. No unconditional unpause or
engine calls from the monitor. Initial pause caller and separate null-manager
Lua pause crash remain outside the correction.

## Current finding: run8944 paused replay loading softlock

The analysis was followed by the authorized REPLAY_PAUSE_R1 build above. Read
`analysis/REPLAY150_RUN8944_SOFTLOCK_2026-11-07.md` for the native chain and
`analysis/replay150_run8944_2026-11-07/dump_analysis.json` for captured values.
First Replay entry after Story can freeze. Correct150-ID cache and phase0,
empty queues, all native loading-readiness predicates pass. Simulation clock
paused at rate0 blocks its step callback, which owns replay/loading completion;
loading1 and clock0.8916667 persisted45 samples/46.688s. Initial pause caller
and ordering remain unproven. Do not infer a new capacity limit or blame a
plugin/older recording solely from their presence.

Implemented in staged REPLAY_PAUSE_R1: preserve snapshot150/256 and parent capacities; add
guarded loading-completion servicing from an updater on the game thread that
runs while simulation is paused, with telemetry and native fixture coverage.
Retain mode/lifetime/readiness/phase checks and user pause. No unconditional
unpause, resource-gate bypass or game calls from the monitor thread.

## Snapshot parent: REPLAY150_R1 prior live validation

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

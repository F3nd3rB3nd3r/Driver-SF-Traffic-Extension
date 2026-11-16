## Current R8 run18736: Getaway1 fixed in52-minute Story test; Replay next

Read `analysis/USERCAP_R8_RUN18736_RESULTS_2026-11-16.md`.
User confirms formerly frozen Getaway1 exit worked, no FPS/audio/rendering
issues, Story-only session.22 accepted phases including eleven280 phases all
native READY. Key peak280/802, buffer1/redirection1, generation3/destruction2,
failures0/guardBad0. Native256 threshold actually crossed. Hash55377/180000,
dispatch290/802, audio registry283/384. No fatal/capacity/queue failures.
Analyzer passes43/44: audio_setup fails450 times; all43 affected models later
successfully set up. Most failures near transitions, two clusters during280
phase; native cause unclassified, no user audible issue. Not a clean44/44 pass.
R8 Replay unexercised (reader constructs0), normal exit not separately specified.
Keep R8/current INI/AI; no new build or capacity increase indicated. Next useful
validation: R8 Story/menu/Replay lifecycle and normal quit. Analysis/handoff
only; no production/game/build/deploy/INI/asset/save/commit/push changes.

## R8 candidate implementation and offline verification

Read `analysis/USERCAP_R8_MANAGER_KEY_FIX_2026-11-16.md`.
User authorized audit/fix/build. New dist/USERCAP_R8 contains312832-byte ASI,
SHA2565a7b62b2...d3838bc, same active280/HASH180K INI values and R8_TEST.md.
Fourteen manager-owned key/ref lists retain native C18 layout/sentinels; lazy
external suffix gives max(512,ModelSlots+ReplayRetainedReferences)=802 entries
each for current INI. Scoped native search/insert guards preserve unrelated
callers. Register at cold manager construct; retire after package destruction
at0059D566; stable old generation nodes retained. New MANAGER_KEY telemetry.
All56 expected offline outcomes/compiled checks pass, including original548
overflow and captured native lookup hang, five548-key overlap transitions and
13 manager generations. This is not yet a live Getaway/FPS pass.
Next user test: replace R7 with R8 (one traffic ASI), same AI/assets/save,
exit Getaway Activity1 via prior path, then repeat/transitions if successful.
Return DSFV29s_usercap_r8_<PID>.log; dump if frozen. No game install/INI/assets/
saves, game/debugger launch, commit or push changed. Active project targets R8.
Scoped backup/evidence: analysis/manager_keys_r8_2026-11-16/.

## Previous R7 Getaway Activity1 freeze27864: native256-key list

Read `analysis/USERCAP_R7_GETAWAY27864_FREEZE_2026-11-16.md`.
Supplied full Driver.DMP PID27864, SHA256bde5509d...418923. Actual thread-stream
CS23/RIP0059E0EE loops in native0059E0D0; WOW TLS VirtualQuery context is stale.
Manager+0C native256-node12-byte key list is corrupted: header active548,
lookup key42 reaches adjacent sentinel with key41 and next=self, never reaches
its own sentinel. Native0059E120 fresh insertion has no capacity guard. The
exact overflowing write/producer still needs focused static audit. Other mod
guards0 do not cover this list. R7 query active, vehicle free261/290, dispatch
556/802, hash54975/180000. Final280 request never READY; completed list18.
Next proposed: audit manager-list constructors/insert/search/remove/reset/
lifetimes, then configuration-sized extension with native regression and
Getaway1 return retry. Dump enough to start; no additional probe needed first.
Analysis only: no source/ASI/INI/build/deploy/game/debugger/commit/push changes.
Evidence: analysis/usercap_r7_getaway27864_2026-11-16/. Await implementation
direction; R7's earlier successful tests remain limited to those workloads.

## Previous R7 run11636:280-car Replay/Story/menu lifecycle verified in that run

Read `analysis/USERCAP_R7_RUN11636_RESULTS_2026-11-15.md`.
User completed menu->full Replay->menu->Story/driving->Replay->menu->normal
quit; both replays smooth, no indicator/audio/render issues. All42 generic
checks pass. Nine18/280 phases all native READY; capacity override27 times.
Reader2MiB actually exercised:2 owners,10construct/10destroy/8reuse; no read/
allocation/guard failures. Audio704/704 setup succeeds,0 failures (21688's
transition failures do not recur). Handler peak551/802, hash54749/180000.
R7 is supported for this280-car lifecycle plus21688 Story/activity/pursuits.
Keep same ASI/INI/AI; no capacity increase or new ASI indicated. Larger rosters,
all activities/chapters and indefinite lifetime remain unverified. Analysis/
handoff only; no production/game/build/deploy/INI/asset/save changes.

## Previous R7 run21688: Story/activity/pursuits successful

Read `analysis/USERCAP_R7_RUN21688_RESULTS_2026-11-15.md`.
~12m27s log records18->280->18->280, all native READY; both280 phases use
same roster. Widened capacity query overridden28 times, ModelSlots290 vs
metadata255. No fatal/guard/pool-full evidence. Handler peak551/802,
hash extra55032/180000, audio registry284/384; no capacity raise indicated.
Generic analyzer fails only strict audio_setup:48 failures across six models
at431-432s after280->18 transition; all six subsequently succeed, no further
failure increments through747s. This is not a wholly clean analyzer pass.
User confirms Story loaded, speed trap/free-roam pursuits worked, no missing
audio/rendering/FPS issues; recalls normal quit. No menu/Replay transitions.
Replay reader constructs0. Next: same R7/INI/280 AI, Story->menu->Replay->
menu->Story then quit normally. No further ASI/capacity increase indicated.
Analysis/handoff only; no source/build/deploy/game/INI/asset changes this review.

## R7 candidate: mission capacity getter corrected

Read `analysis/USERCAP_R7_MISSION_CAPACITY_FIX_2026-11-15.md`.
User authorized implementation. New `dist/USERCAP_R7/` contains Release/Win32
ASI, same active HASH180K INI values and R7_TEST.md. Keep280 AI and same save.
005A0AD0 now returns full-width ModelSlots for recognized vehicle package/
subtype0; other packages/subtypes and native admission/loaded checks remain.
The280 native fixture reproduces255 rejection then queues the mission request
against290, without fabricating READY. All46 offline outcomes and compiled
checks pass. This is not a live gameplay pass; retry Story loading first.
Log prefix DSFV29s_usercap_r7_; CAPACITY_QUERY_START/STATUS report hook/query
usage only. R6 evidence/artifacts unchanged. No deployment/game/debugger run,
installed INI/AI/save changes, commit or push. Project now targets USER CAP R7.
Exact scoped pre-task bytes/diff: analysis/mission_capacity_r7_2026-11-15/.

## Previous R6 softlock36080: remaining255 mission capacity query

Read `analysis/USERCAP_R6_SOFTLOCK36080_MISSION_CAPACITY_2026-11-15.md`.
HASH180K applied. All280 native READY by34984ms and remain READY ~94s;
dump's completed list exactly matches280 roster. Hash extra52793/180000,
handler286/802, no integrity/fatal failures. This is not a gameplay pass.

Full1.47GB Task Manager dump PID36080 is decoded read-only via memory mapping
and WOW64 CPU contexts. Captured Lua callback waits on mission vehicle123
(Free Drive); spoolingBusy=false, traffic stagewaitingForTrafficSet, stepFunction
active, native AllTrafficVehiclesLoaded=true.123 is present in catalog but
absent from completed traffic/mission lists. Native mission admission needs
281 entries but0059C5C0 ->005A0AD0 still reads metadata byteFF, returning255,
although290 slots were initialized and10 remain free. Thus request is rejected.
This is another downstream byte-reader omission, not more hash/audio demand.

Next proposed correction: scoped full-width configured ModelSlots getter for
vehicle package/subtype0, preserving other packages/subtypes and real budget/
catalog/readiness checks; add native mission-admission regression and focused
telemetry. Dump supplies enough evidence; no new live probe needed first.
Keep280 AI/HASH180K INI. This analysis made no production source/ASI/INI change,
no build/deploy/game/debugger run, commit or push. Await implementation direction.
Evidence and reusable analysis-only dump/Lua readers:
`analysis/usercap_r6_softlock36080_2026-11-15/`. Do not reparse its generic
Driver.DMP filename later without checking the saved SHA256 manifest.

## Current R6 result19636: configured hash arena exhaustion; INI-only retry

Read `analysis/USERCAP_R6_EXIT19636_HASH_LIMIT_2026-11-15.md`. Supplied R6 log
ends in LEGACY_FATAL hashExternalExhausted/ExitProcess0x29 at99953ms. This is
an explicit capacity refusal. Native cursor equals hash end; configured extra
45000 nodes are exhausted. R6 progressed to286 active handlers and last READY
checkpoint226/280; fourth queue peak37, no stream integrity failures. Full280
and broader lifecycle remain unverified; do not call the session a pass.

No new ASI/build needed: compiled R6 allocation and comparison both read the
runtime HashExtraEntries field. Staged `dist/USERCAP_R6/HASH180K/DriverSF_Unlimiter.ini`
sets only HashExtraEntries180000 (native45000+extra180000), adding about3.09MiB.
Keep the same R6 ASI and280 AI. Next user retry must report180000 at startup;
collect the new R6 log in `dist/USERCAP_R6`. Headroom is not a proven minimum
or guaranteed budget. Existing packaged/installed INI, ASIs and source remain
unchanged; only separate alternative config, report/evidence and handoffs added.
No game/debugger run, build, deployment, commit or push. Earlier R6 retry notes
remain relevant after this one-setting correction; R5 stays superseded.

## Current candidate: USER CAP R6 fixes the R5 fourth-queue regression

Read `analysis/STREAM_HANDLERS_R6_FOURTH_QUEUE_FIX_2026-11-15.md` and
`dist/USERCAP_R6/USER_CAPACITY_R6_TEST.md`. New dump211728 is PID19620/R5,
crashing005A55D7 via005A4D78 before any traffic roster request (GENERIC calls0).
The exact Temp R5 log was read and preserved in
`analysis/usercap_r5_crash211728_2026-11-15/`. R5 missed a fourth512-pointer
scheduler vector at15D5C/count1655C. Its relocated count and legacy base/helper
were inconsistent. This is an R5 implementation error, not a280 roster budget
finding. Do not retryR5; earlier R5 readiness notes are superseded.

R6 coordinates all four vectors, adds7 operands/eighth guard hook, and uses
correct scheduler bytes184H+544 (H802 ->148112). H stays802 for the same INI.
Revision2 telemetry includes queue3. The real failing native caller/search/
insertion path is now fixture-tested: restoring legacy routing reproduces
C0000005; corrected routing fills/drains through capacity and guards full.
11 focused +25 inherited process outcomes and compiled/cold-start checks pass.
Release/Win32 ASI304128 bytes, SHA256
`851f22dddc3ff314b148ac7bde61a1046f670da085b5de94689b45a465f4aec5`.

Next: user runs onlyR6 with the same280 AI and effective INI; first menu/Story,
then activity/menu/Story/Replay/normal exit if successful. Collect
`%TEMP%\DSFV29s_usercap_r6_<PID>.log` in `dist/USERCAP_R6` plus a new dump if
needed. Live280 verification remains pending. Existing R4/R5 artifacts and
pre-edit bytes retained. No game/debugger run, game deployment/file/save
change, commit or push. R5 offline passes did not cover this fourth queue.

## Current candidate: USER CAP R5 handler/scheduler extension; await280 retry

Read `analysis/STREAM_HANDLERS_R5_FIX_2026-11-15.md` and the guide in
`dist/USERCAP_R5/`. User authorized static implementation and a280-entry retry.
R5 extends the native256 dispatch pool and coupled scheduler (three pointer
vectors, fourteen membership lists, runtime tail), preserving stable handler
slabs and native lifecycle. Capacity=max(512,ModelSlots+retained), hence802
for the user's unchanged280/290/512 configuration. Seven hooks, eleven base
selectors, two initialization calls and355 signed instruction operands.
Release/Win32 build,9 focused plus25 inherited offline processes, compiled
and startup/analyzer checks pass. Candidate303616 bytes, SHA256
`9e617bddcf3fb5732f54c66b04212f05c27a0f8fb5a7f01f5aeededd44c9d332`.
Next: user runs onlyR5 with the same280 AI/effective INI; first Story load,
then activity/menu/Story/Replay/normal exit. Collect
`%TEMP%\DSFV29s_usercap_r5_<PID>.log` in `dist/USERCAP_R5`, plus a new dump
if needed. Occupancy sampled, queue/membership insertion peaks projected;
no280/700/1000 gameplay certification yet. R4 artifacts/pre-task edits retained.
No game/debugger run, deployment, game file/save change, commit or push.
Earlier analysis-only/await-authorization notes below are superseded.

## Current crash28316: separate native256 streaming-handler pool

Read `analysis/USERCAP_R4_CRASH28316_STREAM_HANDLERS_2026-11-15.md`.
Matching280-ID log/config/AI audits pass, but initial Chapter7 loading crashes
at0059DA4A reading a null handler. Native scan has no free candidate; setup
still constructs256 shared80-byte records. Existing INI/R4 queues do not
extend this array. Exact pool occupancy is absent from the small dump.
Next authorized implementation would need coordinated stable backing,
all base/count/lifetime consumers, exhaustion handling and telemetry; merely
raising the initializer corrupts adjacent fields. No source/ASI/INI/asset
change, build/game/debugger launch or commit/push. Analysis complete; await
user direction before a new patch. Prior152-model R4 success remains valid;
280-model support is not verified. Evidence: `analysis/usercap_r4_crash28316_2026-11-15/`.

# DriverSF V29s ? current baseline

## TrafficEditor 0.3: selected-table bulk weights and Lua ID import delivered

See `analysis/TRAFFIC_EDITOR_BULK_WEIGHTS_2026-11-15.md`. v0.2 Add missing IDs
preserved existing weights; v0.3 now offers Set weights across selected/all
tables, optional missing-row insertion and non-executing vehicleStats ID import.
89 core checks and WPF Release publish pass. Sibling source/build are updated
with hash-verified backup; ready app is `publish/bulk-weights-v0.3/` and workspace
packages are `dist/TrafficEditor_v0.3/`. The extracted Lua sample has146 IDs;
applying them to the19 expanded tables on a scratch copy yields280 requested
IDs, beyond the current160 configuration. Actual game AI/Lua/INI/ASI and R4
source are unchanged. No game/GUI launch, commit or push occurred.

## R4 run36472: Story/menu/Replay/Story/menu lifecycle passes at152 models

Read `analysis/USERCAP_R4_RUN36472_RESULTS_2026-11-15.md`. User reports completing
the requested sequence including in-game quit; timeline supports Story -> menu
-> Replay -> menu -> Story -> final frontend.13 complete18/152-ID phases all
reach READY.Three distinct traffic owners, retirement peak147/512 with three
redirects,0 queue failures/guards; pending/cleanup end0. Four replay restores
include6/7/12 retained pairs; reader5 constructions/4 reuses,0 faults and no open
waits.84 audio setup failures across12 models all show later successful setups,
without measured exhaustion; strict generic audio_setup remains false. Keep R4
and current160-request/170-model configuration. Basic requested R4 lifecycle
coverage is complete; no new patch/increase indicated. No production changes,
build/deploy or push. Evidence in `analysis/usercap_r4_run36472_2026-11-15/`.

## R4 run20408: Activity2/large-small traffic replacement succeeds live

Read `analysis/USERCAP_R4_RUN20408_RESULTS_2026-11-15.md`. User confirms Race
Takedown Activity2 completed normally and overall behaviour looked fine; ended
withAlt+F4.160 request/170 model slots, five152-ID phases plus five18-ID phases,
all READY; four152->18->152 cycles. Retirement peak147/512, external redirect1,
recycled insertions499, failures/guard damage0 across all three new queues.
Keep R4/current INI; no new patch/increase indicated.165 audio setup failures
across12 models all show later successful setups, without measured exhaustion;
strict generic audio_setup check remains false. Replay recording249 saves is
exercised, playback/menu re-entry/normal teardown are not. Evidence in
`analysis/usercap_r4_run20408_2026-11-15/`. No production changes/build/deploy.

## USER CAP R4 candidate: traffic replacement queue overflow fixed offline

Read `analysis/TRAFFIC_QUEUES_R4_FIX_2026-11-15.md`. SS Test dump31908/Driver.log
show retirement-list overflow while Activity2 switches Chapter7 -> Chapter3.
The supplied AI predicts152 ->18 IDs and146 removals;62 overflowed12-byte nodes
are captured beyond the native50-node list, corrupting the adjacent work header.
R4 extends all three traffic lists through the existing hooks with guarded,
generation-owned storage derived from max(512,ModelSlots+ReplayRetainedReferences).
The active one-project solution builds R4 Release/Win32.19 inherited processes,
six focused queue processes and compiled checks pass. Candidate and instructions:
`dist/USERCAP_R4/`. R3 distribution/INI and existing editor work are preserved.
Reuse the user's current INI; the152-ID AI needsRequestCapacity>=152 (160 previously
suggested). R4 logs use `DSFV29s_usercap_r4_<pid>.log`. No game deployment/run,
game-asset/config/save change or commit/push occurred; live retry is pending.

## TrafficEditor 0.2 implemented and copied to its source folder

User requested expanding existing traffic probability tables. Read
`analysis/TRAFFIC_EDITOR_EXTENSION_IMPLEMENTED_2026-11-15.md` for the implementation,
57 passing core checks, independent validation, backup and deployment manifest.
Sibling `DriverSF_TrafficEditor_Source` now has variable-row parsing/serialization,
bulk add/remove IDs and an R3 routed-capacity report. Build is at its
`publish/expandable-tables-v0.2/`; workspace packages are in `dist/TrafficEditor_v0.2/`.
All21 listed source/package file hashes were verified after copy. No editor Git
commit/push, game/debugger/GUI launch, ASI/INI or game-asset/save change occurred.
Expanded files still need a user game test with matching assets/capacities.
Native spool groups remain replacement-only; R3 AssetWeights requests added
positive probability IDs. The Unlimiter baseline remains R3/default150.

## R3 run34672: Replay -> Story -> menu/quit passes at150 defaults

Read `analysis/USERCAP_R3_RUN34672_RESULTS_2026-11-15.md`. User reports menu -> Replay watch -> menu -> Story/chase -> menu -> normal game quit; sampled worlds/teardown agree. Generic/baseline/reader checks pass;550 audio setups/0fail.11 complete150-ID phases;10 all READY, one superseded143/150 then replaced by all-ready phase. Pending/cleanup drain cleanly; reader4 reuses.10 short playback waits all clear (max42.295ms), at the same clock/client positions as36508. Historical callback checker has only the known startup-label mismatch, validated separately. Together36508/34672 cover basic default150 lifecycle both directions. Retain R3/INI; larger/nondefault gameplay and same-address/nonzero-retained scopes remain unverified. No source/ASI/INI/build/deploy changes.

## R3 run36508: Story/Replay/menu succeeds; audio recovery caveat

Read `analysis/USERCAP_R3_RUN36508_RESULTS_2026-11-15.md`. User reports Story/chase/activities/dares -> Replay -> main menu; sampled timeline agrees and R2 load crash does not recur.13 complete150-ID phases;2..13 all READY. Structural/guard/queue/reader checks clean;12 native audio setup failures across6 models all show later success and no capacity pressure. Replay10 short playback waits all clear (max39.437ms); reader4 reuses. Historical callback parser has only the expected expandedFrame150/expandedFrame label mismatch, explicitly validated against R3 schema. Retain R3/defaults; visual/audio/FPS/smoothness/final-exit confirmation pending. No production changes/build/deploy.

## USER CAP R3 supersedes crashing R2

Both supplied R2 dumps (34196/31312) crash at005A209F after a completed-map insertion returns0. R2 moved external backing to VirtualAlloc but retained an address-order exhaustion guard. Old code deterministically rejects the first native insertion when backing is below the package. R3 uses explicit native/external cursor provenance and guards before writes. See `analysis/USER_CAPACITY_R3_STORY_FIX_2026-11-15.md`. Candidate/compatible unchanged-value INI are in `dist/USERCAP_R3/`;19 offline processes pass. No game install or live retry occurred. Original R1 fallback and R2 artifacts are preserved. Earlier R2 readiness notes below are superseded.

## USER CAP R2 implemented; live regression next

Current active project builds USER CAP R2. Previous GENERIC_R1 ASI/default INI are preserved byte-for-byte. Read `analysis/USER_CAPACITY_R2_BUILD_2026-11-15.md` and `dist/USERCAP_R2/USER_CAPACITY_R2.md`. Known coupled capacities now follow INI geometry; old testing-only maxima are removed with checked allocation/layout/stack bounds. No game installation, assets or saves changed. New candidate needs150 live regression before larger matching rosters. Earlier feasibility-only/no-build/150-parser-ceiling notes below are historical and superseded.

Updated 2026-11-15. This file and `NEXT_ACTION.md` describe the current state.
Historical reports' proposed next steps are superseded by the current request.

## User-capacity feasibility: further static work, no new live probe first

Read `analysis/USER_CAPACITY_FEASIBILITY_2026-11-15.md`; scoped native output
`analysis/user_capacity_feasibility_2026-11-15/`. User asks whether four-step
user-controlled-capacity plan can be implemented using available material.
Feasibility reviewed; no wide-capacity source/INI/ASI implementation/build.
Existing source/Driver.exe support continuing static migration; no fresh live
probe is currently identified as necessary before that work. Indirect native
provider/index methods remain untraced, so complete wider-capacity feasibility
is not yet proved. Fixed150/256 scratch/arena/diagnostic bounds still exist.

Keep verified150 default. The next native dependency is resolving provider
consumers beyond0059F9C0 (indirect calls0059FA85/0059FAA0), alongside coordinated
dynamic storage and stack probing. Offline build/fixtures and later user live
regression are needed before claiming wider runtime behavior. Do not merely
delete the parser ceiling or claim no remaining game limits.

## Capacity-cost audit: exact metadata, empirical shared demand

Read `analysis/CAPACITY_COST_AUDIT_2026-11-15.md`; arithmetic/observations in
`analysis/capacity_cost_audit_2026-11-15/`. Request ID4 bytes/array position;
audio registry16 bytes/node/manager; global list pointer4 bytes/slot/list
(three lists), excludes actual audio objects. Replay retained pair8 bytes.
Default audio budget and replay ring have no universal per-car multiplier.
Original18 run12180 audio registry peak26, demonstrating non-traffic/headroom
demand. Exact allocation geometry is established; exact per-car asset RAM
and worst-case shared demand are not.500 arithmetic is not supported500 play.

Policy: verify structural bounds statically, use tested headroom/defaults and
measured occupancy/pressure/waits for shared budgets; no need to find an exact
minimum for every car. Greater reader capacity only targets relevant I/O
starvation, not all replay choppiness. Keep current build/config. No production
change/build/game/debugger; await next concrete task.

## Latest10556: same-process Story/menu/Replay/menu sequence passes

Report `analysis/GENERIC_LIFECYCLE10556_RESULTS_2026-11-15.md`; evidence
`analysis/generic_lifecycle10556_2026-11-15/`. User reports menu -> Story ->
menu -> Replay -> menu -> quit. All four analyzer groups pass.29 captures/
saves, one150-ID import/restore, two different world/registry owners, reader
four reuses. Loading-completion fix exercised: seven native holds then one
completion, with paused/rate/clock unchanged. No playback busy interval;
348 audio setups/0fail. Pending/cleanup drain cleanly; no guard/allocation fault.

Keep GENERIC_R1/default limits. This covers Story-to-Replay in one process,
not Story re-entry after Replay, same-address registry reconstruction, nonzero
retained-reference restoration or non-default INI geometry. Next planned
milestone is a non-default supported config with a matching asset roster;
do not lower RequestCapacity below the current asset's150 required models.
No production/ASI/INI/asset/save changes or build. Await next concrete task.

## Latest24080: saved replay from22828 passes in a fresh process

Report `analysis/GENERIC_REPLAY24080_RESULTS_2026-11-15.md`; evidence
`analysis/generic_replay24080_2026-11-15/`. User loaded22828's saved replay
from main menu, left controls untouched, recalls no choppy/bursting symptom,
and quit normally. Eight150-ID import/restore pairs succeed; reader five
constructions/destructions, four reuses,0 faults/rejected reads. These are
internal operations, not eight user Replay entries. All four analyzer groups
pass, including665 audio setups/0fail. Nine request phases all150 READY.

Eight short playback busy intervals total232.61ms, max78.48ms, all clear;
one1.59s context-ended interval is initial loading. No unresolved playback
wait; not wait-free or an FPS measurement. Nonzero retained-reference restore
and held-completion branch are unexercised. Pending/cleanup drain cleanly.
Keep GENERIC_R1/default limits. Same-process Story/Replay/menu cycling and
non-default INI validation remain open; no patch/build/capacity increase.

## Latest22828: valid garage car resolves activity retry

Read `analysis/GENERIC_RUN22828_RESULTS_2026-11-14.md`. Driver.log selects212,
completes StreetraceTakedownActivity5 plus DownhillDrift/ChaseActivity4/
StreetraceTakedownActivity4 and their end-loading flows. User confirms normal
quit, correct audio/rendering and no unusual FPS drops. Supports24048's
unavailable garage122 diagnosis; Forza provenance remains inferred.

Eight150 requests, phases2..8 all READY,11,800 successful parked picks.
Pending peak154 and cleanup51,821 drain to0 without integrity faults. Generic
checks pass except audio_setup:123 native failures across19 models, all with
later successful setups; user heard no issue. No sampled audio pressure or
slot ceiling hits. Driver.log also catches startup nil setRegionArtStatus;
no link to this resolved wait established. Keep GENERIC_R1/default limits.
Replay import/playback/restore and registry reconstruction remain untested
here. No production/ASI/INI/asset/save changes or build. Await next request.

## Latest24048 activity softlock: unavailable garage model122

Read `analysis/GENERIC_ACTIVITY24048_SOFTLOCK_2026-11-14.md`. Dump confirms
StreetraceTakedownActivity5 waits in `active - loadActivityVehicle` for player
model122. Native requested/current fields both122, but resource catalog lacks
122 and READY query cannot succeed. All150 traffic models are READY; no
recorded capacity/guard/fatal failure. User drove191 but suspects122 is a
saved garage choice left over from Forza; selection code reads garage model
when no activity vehicle exists. Proven unavailable ID; provenance inferred.

User will pick another available garage car and retry. Keep GENERIC_R1/INI
unchanged; no new ASI/capacity increase indicated. Await result before claiming
resolution.18 audio setup failures (118/181/239,6each) are separate/unresolved;
do not call24048 an unconditional telemetry pass. No build/game/debugger or
production/source/asset/save change in this analysis.

## Latest GENERIC_R1 telemetry:34904 modified /12180 original pass

Report `analysis/GENERIC_PAIR_34904_12180_RESULTS_2026-11-14.md`. Same default
AssetWeights INI in both.34904 derives two150 lists and active150 phase reaches
all READY,62 audio setups ok/0fail.12180 preserves two18 lists, both all READY,
104 setups ok/0fail. All four analyzer groups pass. No queue/map/guard/fatal
errors; pending/cleanup drain. User reports Story plus a police chase in each;
separate visual/audio/FPS/exit confirmation pending at report time.

150 parked cache builds, but picker calls0 in34904;12180 has133 successful
parked picks. No replay playback/import/reconstruction or callback extra-node
exercise. Do not overstate broader lifecycle or drawable certification. Legacy
chapter-summary parser fixed for absent CHAPTER_STATUS; no ASI/source/INI patch.
Keep GENERIC_R1; no further limit increase indicated by supplied logs.

## Current staged candidate: GENERIC_R1 configured-storage milestone

Read `analysis/GENERIC_LIMITS_R1_BUILD_2026-11-14.md`. Built Release/Win32,
12 isolated fixtures/160 actual asset profiles and compiled checks pass.
Staged `dist/MAIN_150/DriverSF_Unlimiter_GENERIC_R1.asi`,274944 bytes, SHA256
`a8c2915aad6c3791410891be78d9066f68d35b5ab8e5a32185b2bf055c05722b`.
New `DriverSF_Unlimiter.ini` beside ASI; no installation/game run yet. Prior
TRAFFIC_POLICY_R1 remains the paired-live-pass fallback with its original bytes.

Generic binary has no compiled car lists/profile whitelist. Default AssetWeights
reads actual settings, preserves required native IDs and adds reachable positive
weights. Original80 profiles unchanged; modified55/80 profiles expand from data.
The same INI works for original and known modified150 assets. Native is optional
for already-consistent explicit lists; old DriverSF_Traffic.ini is ignored.

Supported numeric keys: request capacity1..150; audio models45..512, global audio
255..8192, shared audio16..256MiB, reader1/2/4/8/16MiB, retained50..4096. Models/
retained must cover requests. Requests/parked/replay backing and associated
notification/category geometry consume configuration. Graphics/definitions160,
request scratch physical150 and other queue/map capacities remain fixed/tested.
700 is explicitly unsupported, not silently accepted. Invalid config/undersized
required rosters stop with a clear log reason before partial publication.

First user comparison: keep numeric defaults/AssetWeights, original then known
modified150 AI in fresh processes, Story/Challenges/menu/Replay/normal quit.
Guide `dist/MAIN_150/GENERIC_LIMITS_R1_TEST_2026-11-14.md`. New log prefix
`DSFV29s_generic_limits_`; use `scripts/analyze_generic_limits_log.py` for actual
configured dimensions. Await supplied logs before another patch/capacity change.

## Current direction: generic INI limits, asset-owned rosters, eventual700

User wants capacities from INI and model IDs/probabilities from AI assets,
without compiled car lists. Read `analysis/CONFIGURABLE_LIMITS_PATH_2026-11-14.md`.
Focused native trace finds byte loop-bound read005A14EA in initializer005A1420;
loop counters/ordinals are DWORD, so a scoped widened bound is plausible but
indirect factory/provider consumers still need checking. Request frame700
would be8424 bytes; native005A7490 uses unprobed SUB ESP, requiring safe scratch
or a probing prologue. Runtime sizing must cover all coupled structures.

This is initial structural research/config contract, not an implemented INI
capacity feature. No production/project/INI/ASI change or build. Keep paired-
live-pass TRAFFIC_POLICY_R1 while developing the generic replacement. Its INI
still selects policy only; unknown proposed limit keys are not implemented.
The target workflow is original or correctly edited preload lists/weights,
not automatic forced700 selection. Do not claim700 safe from one byte hook.

## Latest live result: intended Native/Preset150 pair passes

Read `analysis/TRAFFIC_POLICY_PAIR_28428_2760_RESULTS_2026-11-14.md`.
Native28428: user confirms original AI, two18-ID lists preserved, final active
roster all18 READY,275 audio setups ok/0fail. Preset1502760: two checked18->150
expansions, all150 READY observed,626 setups ok/0fail, audio registry peak150,
parked150 cache and2060 extended picks. All four analyzers pass for both.
Pending154 and cleanup51668 exceed stock bounds in2760 without errors and
drain to0. User reports both looked fine and exited normally. No exact FPS.

Keep TRAFFIC_POLICY_R1 as the first paired-live-pass candidate. Initial
requests were superseded at17/18 and144/150; the later active lists complete.
No replay playback/import, registry reconstruction or callback extension
exercise in either run; do not claim broader transition/full-campaign or
simultaneous drawable certification. No source/ASI/asset changes or build.
Await concrete next request/log; no further capacity increase indicated.

## Run15256: Native preserves18; installed modified AI weights150

Read `analysis/TRAFFIC_POLICY_RUN15256_RESULTS_2026-11-14.md`. User reports
widespread fallback Challengers in Native. Exact Save Bug Test/media AI checked
with permission: modified434e1a... asset, Chapter7/f0 native list18 versus150
positive models across16 tables,132 outside that list. Run15256 records two
18-ID lists preserved and all18 READY in each sequential sweep; policy,
baseline/callback/READ1M integrity passes, audio setup88ok/0fail. No reader
playback exercised. This strongly supports asset preload/weight mismatch,
not a newly demonstrated pool overflow; fallback branch itself not captured.
Use Preset150 for this modified file, Native for original/consistent assets.
No source/ASI/asset changes or new build. Await concrete next request/log.

## Current staged candidate: TRAFFIC_POLICY_R1; live test pending

User authorized native-roster/optional150-preset separation. Read
`analysis/TRAFFIC_POLICY_R1_BUILD_2026-11-14.md`. `DriverSF_Traffic.ini`
beside the ASI defaults to Native; Preset150 explicitly retains the checked
adapter. Original AI +Native and adjusted uniform150 AI +Preset150 are distinct
acceptance cases. Modified weights/native request lists must still agree.
All capacities remain150/160; ED/EF and inherited lifetime/audio/replay fixes
retained. Actual-roster request diagnostics replace canonical comparisons.

User authorized compilation/build/staging. Isolated Win32 fixture, static/
parser and new compiled PE32/import/RET8/12-geometry/10-hook-table checks pass.
Built Release/Win32 through active single-project solution, output
`build/v29s_traffic_policy_r1/`. Staged279040-byte ASI, SHA256
`bb7411bad4e1532c11789ba6966ec01e27e197ddd6a68b870895526bdc292a2d`.
Guide `dist/MAIN_150/TRAFFIC_POLICY_R1_TEST_2026-11-14.md`. No installation or
game run. Last live-tested binary remains READ1M_R1/run10556, preserved.
Await policy logs/user results; do not rebuild or enlarge automatically.
Old compiled verifier results were not relabeled as new verification.

## Latest static audit: limit roles and native-roster separation

Read `analysis/UNLIMITER_STATIC_AUDIT_2026-11-14.md`. Most active limit roles
are identified; entries, bytes, model registries and owner directories are
different units. The named-setting adapter substitutes a canonical150 list;
capacity-only mode must separate that policy explicitly. Removing CHAPTERS
alone activates the old Chapter7 fallback. Modified AI weights/native request
lists must agree; current adjusted AI may still need the150 preload preset.
Graphics admission is a byte field;256+ cannot fit the current path. This is
not certification that255 is safe. Coupled150/160 structures remain unchanged.
No production source/project/ASI/build/game change. Next implementation, if
requested: native-roster pass-through within current capacities plus an
explicit optional150 preset, before arbitrary INI sizes. READ1M_R1 remains
the live-tested candidate. This audit does not certify all limits for700 cars.

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

## Current: user-authorized R10 repository publication

User requested the relevant changes pushed to GitHub. Publication scope is R9
audio diagnostics, R10 summary logging/parser/analyzer/tests, configuration
guide/profiles, dated reports and compact verification summaries. The existing
tested R10 ASI is copied byte-identically into dist/USERCAP_R10 with a baseline
INI, optional profiles, installation README and SHA256 manifest; no rebuild.
ASI327168bytes/SHA25611df34dd...b96825f9. All125 recorded fixture dependencies
and3 fixture executable hashes match; five Python files pass syntax checks;
run23108 analyzer remains49/50 with only intentional logging_full_trace=false.
R9 candidate inputs used by verification are retained separately. Full user
logs/dumps, backups, generated fixtures and unrelated TrafficEditor files stay
local. No game/install/assets/save changes or new gameplay claims. Actual500+
rosters remain unverified. Commit/push uses an explicit reviewed file list;
no force push, broad staging, cleanup or GitHub Release creation.

## Current: configuration explanations and experimental 500-model example

User requested practical limit descriptions and guidance for a 500-model roster.
CONFIGURATION.md now explains all 15 budgets, roster versus population, coupled
sizing, evidence for tuning and a worked setup. New config/profiles/Experimental500.ini
uses requests512/audio640/retained1024; omitted coupled entries derive to models522,
object/pending/callback526, definitions522 and work1546. Shared budgets retain
4096 global audio/128MiB audio/4MiB reader/HASH180K from the exercised setup.
This profile is explicitly untested for actual 500-model gameplay; values are
trial headroom, not per-car estimates or proven sufficient sizes. Existing
150/280/320 profiles and installed files unchanged. Static documentation checks
pass: supported keys, matching embedded INI, dependencies/derived arithmetic,
relative links, file1769bytes below4096. No native build/game/source/ASI change,
installation, package, commit or push. R10 run23108 below remains latest live evidence.

## Current R10 run23108: Replay smoke supported, summary log129KB

Read `analysis/USERCAP_R10_RUN23108_REPLAY_2026-11-18.md`.
User calls run a Replay watch and says everything seemed okay.5m17.3s log,
128645bytes,532 records,correct320/330/448audio/768refs/4MiB/HASH180K/level1.
18 and280 phases all READY;280 import/restore matches699365A7. One reader owner,
4MiB backing,5construct/4destroy/4reuse by last periodic sample, no reader fault.
Client3begins/2clears/1contextEnd by61.5s, unchanged through303s; open/held0,
read/dropped/clock errors0. No per-busy/owner detail retained at level1: do not
claim exact interval timing or zero busy starts from absent records. Final exit
snapshot/precise normal quit not independently established.430 model-audio
setups succeed, reason counters0. Separate legacy native per-object16-event
gate has1 refusal, no identified audible symptom/model/event at summary level;
retain observation, do not claim every audio request accepted or blindly raise.
49/50 generic checks true; only intentional logging_full_trace=false. No widened
storage/reader/guard fatal. R10 Story20268 + Replay23108 smoke now supported;
no patch/increase or repeat long/full-trace session indicated. Packaging/push
is a separate stage requiring user authorization. Analysis/docs only; no code/
binary/INI/build/game/install/assets/saves/package/commit/push changes.

## Previous R10 run20268: Story loads correctly; real summary log135KB/5m17s

Read `analysis/USERCAP_R10_RUN20268_RESULTS_2026-11-18.md`.
User says it worked. Supplied config/DSFV29s_usercap_r10_20268.log confirms
correct320/330/448audio/768refs/4MiB/HASH180K INI, level1. Both18 and280 request
phases reach native READY.441 audio setups succeed, all failure reasons0.
No fatal/capacity/guard errors. File134687bytes,317406ms (~5m17s),606 records;
sampled33237 attempted/572accepted/32665suppressed, fileStatus0. Similar rate
would be~0.76MB/30min, not a hard cap.49/50 analyzer checks true: only intentional
logging_full_trace=false, so not a full diagnostic certification. No need to
turn on verbose merely to remove this scope flag. R10 Story/summary smoke now
has live evidence. Reader constructs/owners/imports/restores all0; playback/
menu lifecycle and normal Quit not established by this log. Remaining R10
smoke portion is brief Replay/menu/normal Quit, not repeat capacity tests.
No patch/increase indicated. Analysis/docs only, no source/binary/INI/build/
install/game/assets/saves/package/commit/push changes. R9 retains broad Replay/
activity validation; input backup/manifest from installation remains intact.

## Previous: complete Headroom320 INI installed into SS Test/plugins

User explicitly requested a full INI installed in SS Test/plugins. The shorter
Headroom320 profile was already complete using automatic/default advanced
values. Added config/profiles/Headroom320_Full.ini with all15 Limits keys,
AssetWeights and summary LogLevel1, equivalent at current320/330 dimensions.
Verified one R10 traffic ASI in plugins and hash11df34dd...b96825f9. Backed up
the previous installed150 INI byte-for-byte, then copied the full file into
E:/Games/Driver San Francisco SS Test/plugins/DriverSF_Unlimiter.ini with
user-authorized sandbox escalation. Readback/hash match source:848 bytes,
SHA256c0e202ae...22f9b54d. Limits320/330/448audio/768retained/128MiBshared/
4MiBreader/HASH180000, internal work1098. ASIs/assets/saves unchanged; no game
launch/build/commit/push. Backup and verified manifest in
analysis/r10_ini_install_2026-11-18/ (install_20261118_201608_037847.json).
Restart required; R10 live Story/activity/Replay smoke still pending. Explain
full configuration does not mean full trace: logging remains summary level1.

## Previous R10 run22856: guarded exit from150 INI against280-model AI

Read `analysis/USERCAP_R10_RUN22856_REFUSAL_2026-11-18.md`.
Supplied config/dsfv29s_usercap_r10_22856.log explicitly ends GENERIC_FATAL
rosterExceedsConfiguredCapacity required280/configuredRequests150, partialList0.
Startup found a valid baseline150 INI (150/160/192audio/256retained/64MiB/1MiB),
LogLevel1. Initial18 phase READY; subsequent280 request intentionally refused.
Not an unexplained logging crash. Correct whole installed INI to the existing
config/profiles/Headroom320.ini, named DriverSF_Unlimiter.ini beside the actual
R10 ASI, then restart. Profile source verified320/330/448audio/768retained,
128MiB shared/4MiB reader/HASH180000. Do not change only RequestCapacity or
remove/bypass the guard. No new ASI/dump needed for this cause. Full installed
INI path not logged; review did not open/replace game files. R10 live Story/
activity/Replay smoke remains pending. Essential fatal survived summary mode.
Analysis/handoff only; no source/binary/INI/build/install/game/assets/saves/
commit/push changes. R9 remains last successful gameplay-tested candidate.

## Previous R10: optional verbose output and INI documentation implemented

Read `analysis/USERCAP_R10_LOGGING_CONFIGURATION_2026-11-18.md` and
`CONFIGURATION.md`. User authorized steps1/2 only, not packaging/publication.
Active Release/Win32 project targets build/v29s_usercap_r10; ASI327168 bytes,
SHA25611df34dd...b96825f9. R9 artifacts preserved; R9 remains last live-tested.
Diagnostics LogLevel0=minimal,1=summary(default),2=full trace. Filtering is at
CapacityWrite only: all native hooks/guards/counters/limits/decisions retained.
Fatal/refusal/nonzero error records and unexpected audio details bypass output
filtering. Summary keeps traffic/rosters/first READY and periodic counters;
full tracing preserves existing record selection. LOGGING_START/STATUS declare
scope and suppression. Analyzer cannot certify reduced logs as full trace.
20 offline Win32 processes pass (243 logging/parser/backend assertions,1084
audio assertions/20 ABI calls,storage18-1000,documented150/280/320 startup and
output levels). Compiled PE32/filter/native-hook/parser checks pass. Supplied
7044 output-selection test:16145690 bytes full,218045 summary,15247 minimal;
full hashes identically. This is not an R10 gameplay/performance test.
New config/ main baseline150, Expanded280 and Headroom320 keep tested limit
values, add Diagnostics. New files require R10; existing old INIs work on R10
with default summary. Keep user's current280/320 values for a smoke test; do
not replace their280 AI's INI with baseline150. All keys/automatic formulas,
units/dependencies and live-evidence limits documented. No repeat of the earlier
completed original/150/280 configuration comparisons is requested.
No game/debugger/install/asset/save/commit/push or packaging work. Next runtime
step before declaring R10 tested: short default-summary Story/activity/Replay
smoke test. Stage3 packaging/publication stays separately authorized work.

## Previous R9 run7044: Headroom320 succeeds with actual280-model traffic

Read `analysis/USERCAP_R9_RUN7044_RESULTS_2026-11-18.md`.
User reports everything fine playing.8m53.8s log confirms320 requests/330 model
slots/1098 work-key-handler entries/448 audio profiles/768 retained refs/4MiB
reader.9 accepted18/280 phases all native READY; actual maximum remains280,
not320. Four snapshot imports/restores match; reader really uses4MiB, one owner,
5construct/5destroy/4reuse,1363 reads, no reader fault. Key peak280/1098,2buffers/
redirects,4generations/3destructions, failures0/guards0. Query330 overridden24
times. Audio791 attempts,743 successes,48 previewSuppressed, zero other failure
reasons, dropped0; all6 affected IDs later succeed. Strict analyzer46/47 stays
unchanged. No capacity/guard/fatal evidence or new patch/increase indicated.
Exact activity/exit sequence asked separately; do not infer normal Quit from
zero final active entries. No matching new Driver.log supplied/reused.
User clarified original AI with150/280 INIs and150 AI with150 INI were covered
previously; do not request those again solely for R9. This was the one new
configuration comparison from the proposed group, now exercised at280 demand.
Other chapter missions/frequency1-4/actual larger rosters retain open scope.
Analysis/handoff only; no source/binary/build/game/install/INI/assets/saves/
commit/push changes. Evidence in usercap_r9_run7044_2026-11-18.

## Previous R9 run26476: combined test succeeds; audio132 are preview suppression

Read `analysis/USERCAP_R9_RUN26476_RESULTS_2026-11-18.md`.
User completed every R9_TEST step1-7/substep, no visual/audio or other issues,
including normal Quit.49m35s, unchanged280 configuration.26 accepted phases
(13x18/13x280) all reach native READY. New audio trace classifies all132 false
returns as previewSuppressed, modes1/3, preview flag1, caller00556541; dropped0.
All20 affected model IDs later succeed.3769 attempts,3637 successes, no bank/
profile/default/state/invalid-mode failures. Strict analyzer remains46/47 due
to setupFail!=0; no waiver or source change. R8's older450 still lack exact
branch evidence; do not retroactively classify them all from this run.
Replay7imports/7restores,3 reader owners,15construct/15destroy/12reuse; no faults.
Four loading0 busy intervals coincide with import transitions, clear23-144ms;
no evidence of the former repeated uninterrupted-playback hitch pattern.
Key peak280/802,9generations/8destructions,5buffers/redirects; guards/failures0.
Activity settings Chapter1/3/5/7 have request evidence. Manual1->7->4->7 Lua
commands are logged without four extra distinct request/READY phases; do not
invent Chapter4 allocator coverage. Other chapters' missions remain untested.
Keep R9/settings; no audio fix or limit increase indicated. Next useful tests:
Baseline150 with original/150 AI, then same280 assets with Headroom320. This
run's18 phases still use280-capacity backing. Frequency1-4 and real500-1000
rosters remain untested. Analysis/handoff only; no binary/build/install/game/
INI/asset/save/commit/push changes. Evidence in usercap_r9_run26476_2026-11-18.

## Previous R9 diagnostic candidate: combine audio reasons with coverage tests

Read `analysis/USERCAP_R9_AUDIO_AND_COVERAGE_2026-11-17.md` and
`dist/USERCAP_R9/R9_TEST.md`. User authorized logging extension and review of
activity/chapter/configuration coverage; available save is Chapter7 plus Lua.
Active Release/Win32 project now targets USERCAP_R9; R8 artifacts preserved.
R9 adds10 signature-checked diagnostic hooks, reuses the bank hook, counts7
outer failures/3 helper subreasons, and drains a1024-record failure detail ring
throughout the session. Native returns/capacities/selection remain unchanged.
ASI320512 bytes, SHA25626f738c6...c28409a, packaged in dist/USERCAP_R9 with
unchanged280/HASH180K main INI and optional Baseline150/Headroom320 profiles.
15 offline processes pass:1084 trace assertions/20 bridge ABI invocations,
storage18/150/255/256/280/500/700/1000, full cold startup18/150/280/700/1000 and
custom700. Compiled hook/parser checks pass; no parent overlap. Existing strict
audio_setup check unchanged. No live R9 gameplay test yet; high numeric fixture
counts do not establish high-roster gameplay. Game/install/INI/assets/saves
untouched, no game/debugger launch or commit/push. Main first test: same280
setup, Getaway1/Race Takedown/other activities, Lua setting switches, reverse
Story/menu/Replay lifecycle and normal Quit. Return usercap_r9 log plus notes.
Lua wrapper uses frequency0; direct setter bypasses its cached previous state,
so frequency tests are separate. Traffic-setting swaps do not test other
chapters' actual missions/progression. Separate INI comparisons are documented.

## Previous audio static audit: remaining failures are not a proven capacity problem

Read `analysis/AUDIO_SETUP_STATIC_AUDIT_2026-11-17.md`.
User authorized static analysis only. Seven branches feed00553AD5; run18736
excludes disabled vehicle, bank-not-ready and missing model profile: all6128
attempts have lookup hits/bankReady, disabled0/misses0/bankNotReady0. Remaining
branches are missing shared default, explicit preview suppression, invalid
mode and mode-specific vehicle state/snapshot checks. EnterPreview/ExitPreview
bindings set/clear root+25C byte4; it is a flag, not a capacity. Callers clear
failed assignments and later selection can retry. All43 affected models recover.
Early128-result detail sampling ends112532ms; first failure406141ms, so exact
reason cannot be recovered from this log. Strict audio_setup check unchanged.
Keep R8/settings. No new limit increase or fix indicated. Optional next step,
only if authorized: reason-specific failure counters/details across a long run,
preserving native behavior; no speculative capacity increase. Audit/report/
handoffs only; no production/build/ASI/game/INI/asset/save/commit/push changes.
Run3600 and remaining R8 lifecycle validation retain their scope below.

## Current R8 run3600: Story->Replay clean44/44; exit method uncertain

Read `analysis/USERCAP_R8_RUN3600_RESULTS_2026-11-17.md`.
User did menu->Story->Replay->exit, reports everything fine/no crash; unsure
whether menu exit or Alt+F4. All44 generic checks pass. Four18/280 phases all
native READY;280-ID replay import/restore matches699365A7. Reader2MiB actually
exercised:5construct/5destroy/4reuse,1144 read issues, no read/allocation errors.
Key peak280/802,2buffers/2redirects,4generations/3destructions; no guard/failure.
Audio351/351 setup succeeds,0 failures. No non-loading busy episode logged.
Adds R8 Story->Replay coverage to18736's successful Getaway/Story workload.
Full reverse Replay->Story/menu matrix and identified normal quit remain
unverified;18736's recovered transient audio failures remain unclassified.
Keep R8/current settings; no new fix/increase indicated. Analysis/handoff only,
no source/build/deploy/game/INI/asset/save/commit/push changes.

## Previous R8 run18736: Getaway1 fixed in52-minute Story test

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

# Next action ? work only on the next concrete request

## TrafficEditor bulk task complete; await next concrete request

v0.3 supports importing literal vehicleStats keys and setting their weights in
selected tables, updating existing rows and optionally adding missing IDs.
See `analysis/TRAFFIC_EDITOR_BULK_WEIGHTS_2026-11-15.md`. Source and a separate
`publish/bulk-weights-v0.3/` build were copied to the editor repo after89 passing
core checks and independent verification. No game files/configuration changed.
The actual146-ID Lua/19-table scratch example yields280 requested IDs while
keeping Forza weights; current160 request capacity does not cover that output.
User should select their intended inventory, apply weights, review the capacity
report and configure dependent limits before testing. No Unlimiter change,
deployment, commit/push or larger-roster gameplay is implied by this editor task.

## R4 requested lifecycle complete in36472; await next concrete task

See `analysis/USERCAP_R4_RUN36472_RESULTS_2026-11-15.md`. Story/menu/Replay/menu/
Story/activity/menu sequence and reported in-game quit succeed with the current
152-model AI/160-request configuration. Three distinct owners,0 traffic queue
faults/guards, all13 phases READY, four nonzero-retained replay restorations,
reader reuse and final frontend/drains are recorded. Recovering audio warnings
do not indicate measured capacity exhaustion. Keep R4/current INI; no additional
limit increase or ASI is indicated. Documentation/release consolidation can be
the next user-selected task; do not commit/push or expand scope automatically.
Same-address reconstruction, very large rosters and exhaustive gameplay remain
outside this live result, with existing offline coverage retained.

## R4 retirement fix verified in20408; retain candidate/configuration

User confirms Activity2 success andAlt+F4 ending. See
`analysis/USERCAP_R4_RUN20408_RESULTS_2026-11-15.md`: four152->18->152 cycles,
retirement peak147 with external backing/recycling and0 guard/failure events;
all10 accepted phases reach full READY. Keep R4/current160-request configuration.
Audio setup warnings recover per model with no measured pool exhaustion, so do
not raise budgets solely from cumulative failures. This run has one owner/no
constructor resets, records replay but does not play it, and ends in Story via
Alt+F4. Optional next scope is R4 menu/Story re-entry, saved/fresh Replay and
normal in-game exit with the same assets/configuration. Await the user's next
concrete task/log; no further build, limit/asset/config change or push is implied.

## Next: user retry of Chapter7 -> Race Takedown Activity2 with R4

User authorized the traffic retirement-list fix; candidate is built and staged
in `dist/USERCAP_R4/`. See `analysis/TRAFFIC_QUEUES_R4_FIX_2026-11-15.md` and its
package guide. Replace R3 with R4 as the only Unlimiter; keep the Forza mod and
DriverNGHook, matching assets and the existing INI. The supplied AI needs at least
152 request slots (160 previously suggested); no replacement INI is packaged.
The retirement and adjacent work lists now derive capacity from configured
models+retained references, minimum512; native sentinel/recycling stays intact.
Offline25 process scenarios plus compiled checks pass, including reproduction
of the old51st-entry overwrite and146-entry native merge/reuse without corruption.
Await `DSFV29s_usercap_r4_<pid>.log` and observations for Activity2, Story return,
menu/re-entry and normal exit. No live-game fix is claimed yet; do not broaden
limits, deploy, build another ASI or push without a new concrete instruction.

## TrafficEditor extension complete offline; await user test or concrete request

See `analysis/TRAFFIC_EDITOR_EXTENSION_IMPLEMENTED_2026-11-15.md`. Editor source
and v0.2 framework-dependent runtime package were copied into the authorized
sibling editor folder with original-source backup/hash checks.57 core checks,
independent expanded-resource validation and WPF Release publish pass, including
208x1000 row serialization and repeated-save byte identity. The editor reports
the exact static R3 native+routed-positive union; it does not configure pools or
install models. Expanded-file gameplay and actual WPF interaction remain
unverified. Do not build another ASI, change assets/configuration, increase counts
or push editor changes without a new concrete instruction. Unlimiter R3 stays
unchanged. Original source ZIP and deployment manifest are retained in
`analysis/traffic_editor_extension_impl_2026-11-15/`.

## Default150 lifecycle coverage complete for these two runs; retain R3

Latest `analysis/USERCAP_R3_RUN34672_RESULTS_2026-11-15.md`: saved Replay -> menu -> Story/chase -> menu -> user-reported normal quit passes tracked integrity.550 audio setups/0fail; no capacity/guard/allocation faults. Combined36508/34672 cover Story-to-Replay and Replay-to-Story. Keep R3/default settings; no patch or increase indicated. User can choose next scope: nondefault capacities with matching assets, original-AI behavior, or targeted remaining lifetime cases. No need to repeat the same basic sequence automatically. Broader/exhaustive missions and larger configurations remain unverified. Await the next concrete task/log.

## R3 run36508 analyzed: retain candidate; await observations or next test

See `analysis/USERCAP_R3_RUN36508_RESULTS_2026-11-15.md` and evidence directory. Story load plus chase/activities/dares/Replay/menu completes without recorded fatal/guard/allocation faults.150 READY phases verified, cleanup/pending drain, playback waits clear.12 native setup failures recover for all6 affected IDs; no audio exhaustion supports an increase. Optional user visual/audio/FPS/replay/exit question remains pending. No new ASI or capacity change indicated. Remaining scopes include Replay->Story re-entry, same-address reconstruction, nonzero retained restore and non-default matching rosters. Analyze the next supplied observation/log before changing code.

## Next: retry Story with USER CAP R3 at unchanged150 defaults

Use `dist/USERCAP_R3/DriverSF_Unlimiter_USERCAP_R3.asi` as the only traffic ASI and keep the same INI values/AI. Both R2 dumps match a completed-map guard regression reproduced offline and corrected in R3. Read `analysis/USER_CAPACITY_R3_STORY_FIX_2026-11-15.md`.19 offline processes pass, including both allocation orders, full/recycle/reconstruction and invalid-cursor checks. Await the specific `DSFV29s_usercap_r3_<pid>.log` and user Story-load result before claiming live resolution or increasing capacities. No game/asset/save changes or deployment; no new live session needed for diagnosis. Earlier R2 next steps are superseded.

## Next: user live regression of USER CAP R2

Implementation and offline verification are complete for the user-capacity migration; see `analysis/USER_CAPACITY_R2_BUILD_2026-11-15.md`. Candidate/default INI/test guide are in `dist/USERCAP_R2/`. Preserve exact GENERIC_R1 fallback and enable exactly one traffic ASI. No deployment/game run occurred. First test the supplied150 configuration against the known AI through Story/Challenge/activity/menu/Replay/Story re-entry/normal exit. Analyze the resulting `DSFV29s_usercap_r2_<pid>.log` before proposing changes. Larger configurations require matching installed model assets/AI and remain live-untested. Do not claim all native game limits are absent. Earlier next steps below are historical.

## User-controlled capacity plan: feasibility review only this turn

Report `analysis/USER_CAPACITY_FEASIBILITY_2026-11-15.md`. Available source and
Driver.exe allow static migration work without a new live probe first, but
native indirect provider/index consumers are unresolved; wider-capacity path
not yet fully established. Actual150 bounds remain in arenas/scratch/diagnostic
arrays; model/graphics and other sidecars remain fixed. No parser-ceiling
removal, source/INI/ASI change, build, game/debugger or deployment occurred.

Next native dependency: resolve/trace target methods reached by0059FA85 and
0059FAA0; then coordinate configured storage, all bounds and stack probing.
Preserve tested150 fallback/default. New code needs offline build/fixtures,
then user live regression; literal absence of all machine/format bounds must
not be claimed. Follow the user's next concrete instruction.

## Capacity sizing clarified by static audit; no configuration applied

Report `analysis/CAPACITY_COST_AUDIT_2026-11-15.md`. Model ID arrays4 bytes/
entry, model registry16/node/manager, global audio pointer slots4/list/entry,
retained pair8 bytes; actual pointed-to resource data is extra. Coupled parked/
notification/replay formulas documented. Shared audio and replay read-ahead
need workload-based sizing, not a fixed MiB/car multiplier. Original18 roster
run had audio registry peak26; keep headroom for other models/transitions.

User need not calculate exact per-car minimums: statically safe coherent
bounds plus tested defaults/overrides and measured demand are the intended
approach. Higher reader capacity only helps relevant reader starvation.
500 remains unsupported; no source/ASI/INI/asset changes or build. Keep current
candidate and await the next concrete task; do not automatically tune pools.

##10556 checked: same-process lifecycle pass, preserve candidate

Read `analysis/GENERIC_LIFECYCLE10556_RESULTS_2026-11-15.md`. User's menu ->
Story -> menu -> Replay -> menu -> quit matches telemetry. All analyzer groups
pass,348 audio setups/0fail, no playback busy interval. Replay ready-completion
branch holds seven times then completes once without changing pause/rate/clock.
Two world/registry owners, one150-ID import/restore, reader four reuses;
pending/cleanup drain without faults. Keep GENERIC_R1/default limits.

This supplies same-process Story-to-Replay coverage missing from24080. It
does not test Story re-entry after Replay, same-address registry reconstruction,
nonzero retained refs or non-default INI dimensions. Next planned milestone:
a non-default supported config with an asset roster that fits. Current150-
positive asset requires RequestCapacity>=150; do not propose simply lowering
that value against it. Await concrete authorization/task before config/asset
changes, widening, another ASI or build. No failure indicating a new patch.

##24080 checked: saved-replay playback pass; await next task

Read `analysis/GENERIC_REPLAY24080_RESULTS_2026-11-15.md`. Fresh-process menu
Replay uses22828's recording, controls untouched; user recalls no choppiness/
bursting indicator and confirms normal quit. Eight150-ID imports/restores,
reader four reuses, all analyzer groups pass;665 audio setups/0fail.
Eight short playback waits clear (max78.48ms), no unresolved interval.

Keep GENERIC_R1/default INI. This covers saved-replay loading/playback, not
same-process Story -> Replay -> Story/menu cycling (user explicitly did none),
nonzero retained-reference restoration, or non-default INI geometry. Those
are remaining validation scopes before larger-capacity widening. No automatic
ASI, probe, build or capacity increase; wait for the user's next concrete task.

##22828 checked: retain current candidate, await next task

Report `analysis/GENERIC_RUN22828_RESULTS_2026-11-14.md`, evidence
`analysis/generic_run22828_2026-11-14/`. User confirms valid garage car resolves
Activity5, correct sound/rendering/FPS and normal quit. Driver.log confirms212
selection and four completed mission/activity/end-loading flows.150 READY
phases2..8,11,800 parked picks, extended pending/cleanup paths drain cleanly.

Keep GENERIC_R1/default limits.123 audio setup failures have later successes
for all19 affected models and no audible problem reported; not a zero-failure
telemetry pass, and no audio-limit increase indicated. Separate startup Lua
nil setRegionArtStatus has no established causal link. Replay playback/import/
restore and registry reconstruction remain uncovered. No automatic patch,
probe, build or capacity change. This supersedes awaiting the garage retry.

## Latest24048: await retry with a valid garage car

Report `analysis/GENERIC_ACTIVITY24048_SOFTLOCK_2026-11-14.md`; evidence
`analysis/generic_activity24048_2026-11-14/`. Activity5 is waiting for player
model122, absent from the captured available-resource catalog. All150 traffic
models are READY. Loader selects saved garage model when no activity vehicle
exists; user suspects old Forza selection and will pick another garage car.
Keep GENERIC_R1 unchanged. No ASI/build/capacity increase needed for that test.
Await result; do not claim fixed yet.18 audio setup failures remain a separate
observation. No production/INI/asset/save changes or debugger/game launch.

This supersedes treating24048 as an unexplained generic capacity regression.

## Generic34904/12180 analyzed: initial default telemetry pass

Report `analysis/GENERIC_PAIR_34904_12180_RESULTS_2026-11-14.md`; evidence
`analysis/generic_pair_34904_12180_2026-11-14/`. Same default INI derives150
for modified asset and preserves18 for original. Active lists all READY;
audio setups62/104 allok, structural/queue/guard checks clean. No new ASI.
User reports Story/chase; optional rendering/audio/FPS/exit question pending.

Replay reading/import/restore/reconstruction remains unexercised.150 parked
cache builds but selection is unexercised in34904; original picks133 times.
Do not interpret absent activity as a demonstrated bug. Legacy callback tool
needed a missing-CHAPTER_STATUS guard; offline parser only fixed. Await user
results/next concrete request; no further probe, capacity change or rebuild
is warranted automatically. This supersedes the initial-await-log instruction.

## GENERIC_R1 built/staged: analyze supplied live comparison next

Report `analysis/GENERIC_LIMITS_R1_BUILD_2026-11-14.md`; evidence
`analysis/generic_limits_r1_2026-11-14/`. New ASI274944 bytes, SHA256
`a8c2915aad6c3791410891be78d9066f68d35b5ab8e5a32185b2bf055c05722b`.
Default new INI AssetWeights reads real file settings/weights, no compiled IDs
or chapter allowlist.12 native fixtures including actual160 asset profiles pass;
PE/import/RET8/79 retained-site checks pass. No game installation/launch yet.

Keep default150/192/2048/64MiB/1MiB/256 for first original-vs-modified150 runs,
same INI in both. Guide `dist/MAIN_150/GENERIC_LIMITS_R1_TEST_2026-11-14.md`.
Log `%TEMP%/DSFV29s_generic_limits_<PID>.log`; analyze with
`scripts/analyze_generic_limits_log.py`. Old Native/Preset150 policy analyzer
is not this binary's acceptance rule. Record rendering/audio/parked/FPS plus
Story/Challenge/menu/Replay/normal-exit coverage. Previous tested ASI retained.

RequestCapacity ceiling150 is deliberate for this milestone. Graphics160 and
request scratch150 remain fixed, as do other resource queues/maps. Larger
requests are rejected explicitly. Native provider/index audit and safe scratch
handling plus remaining coupled pools are still needed before700. No automatic
widening or rebuild until current generic code has user data/new authorization.
Above supersedes the implementation-pending research instructions below.

## Current work: configurable-capacity architecture and native width audit

User direction is INI limits +asset-specified vehicle lists/weights, supporting
18/150/eventual700 without compiled presets. Report
`analysis/CONFIGURABLE_LIMITS_PATH_2026-11-14.md`; native evidence
`analysis/configurable_limits_static_2026-11-14/`.

Next structural dependency: inspect provider/index consumers beyond factory
0059F9C0 before implementing package0/kind0 widened count at005A14EA. Preserve
zero gate005A1469 and all other metadata consumers. Also plan runtime storage/
frames across requests, notification, parked, snapshots, handlers/definitions,
audio and independent queues.700 request nodes imply8424-byte frame; native
SUB ESP lacks a probe at005A7495, so merely changing constants is unsafe.

No new source/build yet. Current TRAFFIC_POLICY_R1 remains the verified test
candidate; its parser does not implement proposed numeric limit keys. Generic
150 configured backing should be the first regression milestone before larger
counts, with arbitrary-ID fixtures and normal transition/teardown coverage.
No need to rebuild the unchanged baseline or run the game during this research.

## Paired logs28428/2760 pass; retain candidate, await next request

Report `analysis/TRAFFIC_POLICY_PAIR_28428_2760_RESULTS_2026-11-14.md`;
evidence `analysis/traffic_policy_pair_28428_2760_2026-11-14/`.
Native28428 +original AI preserves18; Preset1502760 expands18->150 with
matching asset checks. Active rosters reach all READY. All policy/baseline/
callback/READ1M checks pass; audio setup failures0, queue/guard/fatal errors0,
pending/cleanup drain. User confirms both fine and normal exits.

No patch/rebuild/increase needed. TRAFFIC_POLICY_R1 now has an initial paired
live pass. Replay reading/import/reconstruction and callback extra nodes were
not exercised; broader Story/Challenge/menu/Replay cycling remains useful
future coverage, not a reason to build another ASI automatically. Preserve
150/160/ED/EF. User confirmation of original AI is the Native asset evidence;
the log itself does not fingerprint Native assets. No exact FPS supplied.

## Run15256 analyzed: use the appropriate mode/asset pairing

Report `analysis/TRAFFIC_POLICY_RUN15256_RESULTS_2026-11-14.md`. Current
Save Bug Test/media AI is the exact modified uniform150 asset; actual
Chapter7/f0 preload18 but positive weights150. Native run15256 preserves
two18-ID rosters, both all READY, with structural checks/audio setup clean.
User sees fallback Challengers; mismatch strongly explains it. No direct
native fallback-branch capture or replay-reader exercise in this run.

Use Preset150 with the current modified AI; test Native with original AI or
an asset with consistent preload/probability lists. Do not enlarge pools or
build another probe from these symptoms. Generic asset-derived preloading
or correcting asset preload lists would be separate authorized work, not
something Native currently does. No source/ASI/INI/asset changes this turn.

## TRAFFIC_POLICY_R1 built/staged; await supplied logs

Report `analysis/TRAFFIC_POLICY_R1_BUILD_2026-11-14.md`; evidence
`analysis/traffic_policy_r1_2026-11-14/`. User authorized build/staging; isolated
Win32 fixture, parser/static and new compiled checks pass. Staged
`DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1.asi`,279040 bytes, SHA256
`bb7411bad4e1532c11789ba6966ec01e27e197ddd6a68b870895526bdc292a2d`.
Previous READ1M_R1 bytes/live result retained. No installation/game run.

Test original AI +Native separately from adjusted uniform150 AI +Preset150.
Exactly one traffic ASI; INI beside it, restart after mode changes. New logs
`%TEMP%/DSFV29s_traffic_policy_<PID>.log`. Analyze policy/baseline/callback/
READ1M integrity before any patch. Check actual lists/order and preset reason,
then rendering/audio/parked variety/FPS and Story/Challenge/menu/Replay/quit.
Do not expect the150-weight adjusted asset to work in Native if its actual
preload list is18. No automatic derivation/arbitrary INI capacities yet.

The source-only handoff and build-await instructions below are superseded.
No further compilation needed absent a source change/failure. Keep150/160/
ED/EF; request/readiness counts do not prove drawable residency. The old ASI
ignores the INI. Current guide `dist/MAIN_150/TRAFFIC_POLICY_R1_TEST_2026-11-14.md`.

## Static unlimiter audit complete; no new ASI or source change

Report `analysis/UNLIMITER_STATIC_AUDIT_2026-11-14.md`; focused source/artifact
hash inventory `analysis/unlimiter_static_2026-11-14/source_inventory.json`.
Maps active capacities, units, policy, coupling and remaining owner bounds.
Byte-sized graphics admission prevents a simple700-target INI;255 is not a
verified safe maximum. Preserve150/160/ED/EF and READ1M_R1.

Next implementation, if authorized: preserve native request rosters within
current capacities and make the existing canonical150 preload policy optional.
Do not simply remove CHAPTERS: its else branch retains a Chapter7 replacement.
Keep native safety/lifetime checks and fix canonical-only request diagnostics.
Original AI and modified positive-weight/request-list consistency need distinct
tests. Arbitrary entry counts require coherent frames/backing/loops/consumers;
MiB applies to byte pools only. No build/run or larger-capacity test is implied
by this audit. Await concrete implementation request or supplied log.

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

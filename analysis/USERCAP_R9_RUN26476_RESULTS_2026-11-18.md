# R9 run26476: combined coverage succeeds; audio returns classified

The user reports completing every substep of R9_TEST steps1-7 without visual,
audio or other issues. That includes the activity returns, Lua traffic-setting
switches, Story/menu/Replay sequence and normal Quit requested by step7.
This supports R9 for this280-car workload. Exact FPS was not separately given
or measured by this review. This is not an all-chapter mission or larger-roster
certification.

The key diagnostic result is conclusive for this run: all132 audio setup false
returns took the explicit native preview-suppression branch. There was no
bank/profile/default/state/invalid-mode failure and no lost diagnostic detail.
No audio patch or capacity increase is indicated.

## Inputs and scope

| Input | Bytes | SHA256 |
| --- | ---: | --- |
| `dist/USERCAP_R9/DSFV29s_usercap_r9_26476.log` |94777906|`319637bebb16ca9dd0930057e5280d02e1cd468b48b9ab8b7370e8da77d7a989`|
| `dist/USERCAP_R9/Driver.log` |736889|`259854efc8d5ae8184924af441b0418b0fb8e79e241bd35a3204741c52523d02`|

Last recorded tick2975000ms:49m35s. Startup records confirm the intended
AssetWeights configuration:280 requests,290 model slots,384 audio models,
4096 global audio entries,128MiB shared audio,2MiB replay reader,512 retained
references,180000 extra hash entries and802 work/key/handler capacity.
All new diagnostic hooks report installed. The primary configuration and
actual patch records establish these values; legacy audio-reason metadata
strings are not the source of configured limits.

This was log analysis only. No source, ASI, INI, asset, save, installed plugin,
build, game/debugger execution, commit or push changed. Analysis artifacts and
handoff notes are the only writes. Existing uncommitted work is retained.

## Audio:132 expected preview rejections, not an exhausted pool

The existing counters reconcile to3769 attempts,3769 nonzero profile lookups,
3769 bankReady,3637 successful setups and132 false returns. Disabled,
bankNotReady, missing lookups and model1 fallback counts are all zero.

New cumulative counters and all132 individual failure records agree:

| Reason | Count |
| --- | ---: |
| previewSuppressed |132|
| vehicleDisabled / bankNotReady / modelProfileMissing / defaultProfileMissing |0 each|
| invalidMode / modeInitializer |0 each|
| vehicleState / snapshotGetter / snapshotFlag subreasons |0 each|

Every detail is from caller return00556541, with nonzero mode:76 mode3 and
56 mode1. All accompanying preview samples have previewKnown1/previewSample1.
This matches the audited native gate at00553960, which rejects nonzero modes
while the manager's preview flag is set. Mode0 does not fail this gate.
The failure-reason queue dropped0 records and cumulative counts never regress.
There is no unexplained difference between classified failures and setupFail.

Failure increments appear around694/695s,852/853s and1037/1038s. Twenty model
IDs are involved; every one later has a successful setup after its final
observed failure. This is per-model recovery, not proof of the same individual
vehicle/controller being retried. The user reports no audible loss.

The strict generic analyzer still passes46/47 checks: only its unconditional
setupFail==0 condition is false. Its recorded_integrity_pass remains false.
Do not rewrite that result as47/47 or silently waive the rule. Manual reason
review explains why this run's132 returns do not indicate an unresolved audio
capacity problem. The older R8 run18736's450 returns have no branch records;
the R9 finding supports the preview explanation but does not retroactively
prove that all older failures had the same cause.

## Traffic requests, activities and Lua coverage

All26 accepted request phases reach a native all-READY checkpoint: thirteen
18-model phases and thirteen280-model phases. Effective rosters are complete
and fit configured storage. Sequential READY observations do not certify that
every model was drawable simultaneously; the user's visual/audio observations
provide the corresponding gameplay evidence.

Driver.log records successful activity loading/return paths and these actual
traffic settings, in order among the free-drive returns:

| Logged setting/context | Matching named-setting hash in ASI request events |
| --- | --- |
| Chapter1 / ChaseActivities |AE88E83B|
| Chapter3 / StreetraceActivities |40868917|
| Chapter7 / StreetraceActivities3 |A9E52C22|
| Chapter5 / RaceAwayActivities |DEE21CB4|
| Chapter7 / Free Drive |47EB4D0E|

The smaller settings preserve18 requests; Chapter7 free drive augments its
native18 to280. This is useful evidence that real activity-specific traffic
settings and return transitions work with the enlarged backing. Mission
creation lists also contain definitions unrelated to the selected activity;
they were not used to claim which numbered activity was completed. Specific
Getaway1/Race Takedown2/5 completion/abort coverage comes from the user's
confirmation that all prescribed substeps were followed.

The supplied hook log explicitly records the manual console sequence:
Chapter1 ->Chapter7 ->Chapter4 ->Chapter7. Each wrapper prints the expected
previous setting. These calls do not have four additional separately tagged
request phases in the ASI log, so they establish executed Lua wrapper paths
and user-observed success, not independent fresh allocator/READY coverage for
each manual call. In particular, do not invent an extra Chapter4 READY phase.
Actual activity-driven requests already demonstrate other named-setting hashes.
All named request events use frequency0; frequency1-4 comparisons remain open.

Earlier chapters' missions, progression, unlocks and cutscenes have not been
tested by changing traffic names within the Chapter7 save.

## Capacities and lifetime

No ASI fatal or exception record appears; all capacity/startup/guard/queue and
ownership checks pass. Representative observed peaks/final lifetime counters:

| Component | Observed |
| --- | --- |
| Manager key lists | Per-list peak280/802;5 buffers/redirects;9 generations,8 destructions; failures0,guardBad0. |
| Streaming handlers | Peak289/802; four queue peaks262/3/288/37; failures0,guardBad0. |
| Audio model registry | Peak283/384; full0,misses0,guardBad0. |
| Global audio lists | A0/A1/B peaks100/53/61 against4096; ceilingHits0,invalid0. |
| Mission capacity query |104 widened overrides,5 native fallbacks; correct290 slots. |
| Registration map |5 constructions,1 reconstruction/reattachment,2130 recycled nodes; faults/allocationFailed0. |
| Parked selection |24 builds,280 candidates,largest bucket50;42420 picks,88 peak unique selections; no invalid/guard/concurrency errors. |
| Hash extra arena | Lifetime issued peak55247/180000. |

Sampled private memory peaks1411272704 bytes (~1.31GiB); working set peaks
1061396480 (~0.99GiB). These are process totals, not per-car sizes or headroom
guarantees for future rosters. Final manager/handler samples still have active
entries before the log ends; do not claim that the log independently captured
their final shutdown drain. Normal Quit is user-confirmed through completion
of step7, not proven by absence of an ASI fatal record.

## Replay playback and transitions

Seven snapshot imports and seven restores succeed with no invalid records.
Five imports contain280 models with roster hash699365A7; two contain18 models
with hash22565046. Matching restore records retain their counts/hashes.
Peak snapshot traffic280; peak retained references14. No deferred restores
or active temporary snapshots remain in the final sample.

The2MiB replay reader is exercised across three owners:15 constructions,
15 destructions and12 reuses.4462 owner samples and26 managed boundary records
pass configuration-aware backing/state checks. No reader allocation failure,
fallback, fault, unmanaged call or rejected read is recorded. Retained backing
6291456 bytes is three reusable2MiB owner buffers.

All replay-client telemetry schema/pair/integrity checks pass with0 dropped,
unknown, read-failure or clock-failure records; final open0 and held0. Thirteen
busy intervals are paired. Nine have loading1. Four have loading0 but
transition3/phase0 and coincide with snapshot imports at2045328,2192125,
2518172 and2664828ms; they clear after about88,23,144 and29ms respectively.
These are observed transition boundaries, not evidence of recurring stalls
during uninterrupted playback. Three loading contextEnd intervals around1.3s
are not independently interpreted as readiness recovery. The user confirms
the prescribed normal-speed replay watches and reports no problems.

The inherited replay decoder has a fixed150/160 startup predicate. Its raw
legacy result is retained in evidence, while the applicable280/290 startup
values are validated explicitly. No obsolete150 assumption is waived silently
or used to reject the configured reader geometry.

Driver.log includes repeated startup missing-script messages for
RouteDataE3Files/dailyupdatedisplay and setRegionArtStatus stack traces through
DriverNGHook's autoexec. They are not ASI capacity failures or evidence that
the user-observed run crashed. No baseline comparison establishing their
origin was performed, and no referenced external script was opened.

## Conclusion and next work

The first combined R9 test is successful for the user's reported280-car
activity/traffic-setting/Replay workload, including normal Quit. The new
diagnostics classify every audio setup return that previously would have been
an ambiguous failure counter. Keep the current ASI and capacities; no new audio
fix, probe or limit increase is justified by this session.

Useful next validation is separate configuration comparison, not repetition
of the same sequence solely to eliminate expected preview returns: the original
AI and150 AI with Baseline150, followed by the same280 assets with Headroom320.
Those profiles have only offline validation so far. The18 phases inside this
280-capacity session do not test the smaller150-capacity backing. Actual
500/700/1000 rosters, other chapters' missions and nonzero frequency settings
retain their separate unverified scope.

Reproduce with `python analysis/usercap_r9_run26476_2026-11-18/summarize.py`.
Evidence in that directory: generic.json, summary.json and driver_evidence.json.
The script streams the large supplied log and uses the existing validators;
production analyzers and binaries are unchanged.

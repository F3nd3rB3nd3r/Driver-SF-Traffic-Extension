# R9 run7044: Headroom320 works with the280-model roster

The supplied log confirms the alternate INI was active and exercised in actual
traffic/replay use. All nine request phases reach native READY; there is no
capacity, startup, guard, ownership or reader fault in the recorded checks.
The user reports everything seemed fine while playing. This supports the
Headroom320 configuration for the observed280-model workload.

All48 audio setup false returns are classified as the existing native preview
restriction. No new audio fix or capacity increase is indicated. The strict
generic analyzer remains46/47 because its setupFail==0 criterion is unchanged.

## Input and analysis scope

`dist/USERCAP_R9/DSFV29s_usercap_r9_7044.log`:16145690 bytes, SHA256
`075d25efe9ab5cb0211bcdeb88bdd0a994d32ef5ec8eecf31d7f7a068bcd4f1b`.
Final tick533765ms (~8m53.8s). No DriverNGHook log was supplied specifically
for this run; the older run26476 Driver.log was not reused or attributed here.
Exact activity names, sequence completion and exit method were asked separately;
they are not inferred solely from these counters.

Only analysis artifacts and handoff notes change in this review. No source,
ASI, INI, game assets, save, installation, build, game/debugger launch, commit
or push changes were made. Earlier uncommitted work is retained.

## The alternate configuration is real, not just an edited file

Configuration/startup records and runtime geometry agree:

| Setting | Prior main INI | Run7044 |
| --- | ---: | ---: |
| RequestCapacity |280|320|
| Derived ModelSlots |290|330|
| Work/handler/manager-key capacity |802|1098|
| AudioModelEntries |384|448|
| ReplayRetainedReferences |512|768|
| ReplayReaderMiB |2|4|
| Parked scratch stride / frame bytes |1124 /28732|1284 /32732|
| Notify frame bytes |1128|1288|
| Replay snapshot payload capacity bytes |5228|7436|

The payload calculation is4*RequestCapacity +8*retainedReferences +12.
AssetWeights,4096 global audio entries,128MiB shared audio and180000 extra hash
entries are unchanged. The configured330-slot mission query is exercised24
times, with four preserved native fallbacks. This verifies that these runtime
paths consume the alternate configuration rather than remaining pinned to
R8/R9's previous280/290 geometry.

## Traffic and lifecycle observations

Nine accepted phases all reach native all-READY checkpoints:

| Phase | Actual models | Requested tick | First all-READY tick |
| --- | ---: | ---: | ---: |
|1|18|28437|33390|
|2|280|33734|49468|
|3|18|156125|157078|
|4|280|259937|273687|
|5|18|302125|304843|
|6|280|309218|317921|
|7|280|406781|408515|
|8|18|409609|410531|
|9|280|504281|512046|

The native named-setting path preserves18-model lists and augments Chapter7's
native18 to280, followed by replay-origin requests. The intermediate named
hashEF19694A changes to18 and then returns to Chapter7/280. No activity name
is assigned to that hash without corroborating information. All named requests
use frequency0. READY observations are sequential, not independent proof of
simultaneous drawable residency for every model.

Manager key storage peaks280/1098, with two buffers/redirections, four
generations and three destructions. Streaming handler peak289/1098; scheduler
queue peaks262/2/319/37 are in bounds. Final sampled active key/handler/queue
counts are zero. Failures and guardBad remain zero. This does not by itself
establish the user's exact shutdown method.

Audio registry peak283/448, full0/misses0/guardBad0. Global audio A0/A1/B peaks
93/53/60 against4096, with ceilingHits0/invalid0. Shared default audio sampled
usage peaks18473580 bytes (~17.62MiB); pressureSamples0. Parked selection builds
eight times from up to280 candidates;1186 picks succeed, with no integrity error.
Hash extra lifetime issuance peaks53714/180000. Registration-map faults and
allocation failures remain zero;529 nodes are recycled.

Sampled process private memory peaks1412722688 bytes (~1.32GiB), working set
1045585920 (~0.97GiB). These are process totals for this run. Its shorter and
different workload is not a controlled memory/performance comparison with the
previous49-minute session.

## Audio classification

791 attempts reconcile to791 model-profile hits,791 bankReady,743 successful
setups and48 false returns. Every false return has a matching previewSuppressed
detail; mode3 appears41 times and mode1 seven times. Every preview sample is
known/set. No other outer reason or helper subreason occurs. detailDropped0;
counters are monotonic and the final classification total matches setupFail48.

The failure increments occur around158-159s during the18-model setting phase.
Six model IDs are affected and all later have successful setup records after
their last failure. As before, that is per-model recovery, not proof of the
same vehicle/controller retry. It is consistent with normal preview suppression
and the user's absence of noticed audio problems. No failure is evidence here
of an exhausted model bank, missing default profile or insufficient sound pool.

The generic analyzer's one failed strict check remains recorded as false;
the reason evidence explains it rather than changing the analyzer criterion.

## The4MiB reader is exercised

Runtime owner records confirm4194304-byte backing. One reader owner has five
constructions, five destructions and four reuses.1363 native read issues are
observed. All229 owner samples and14 managed boundary records pass configured
backing/state/schema checks, with no allocation failure, fallback, fault,
unmanaged call or rejected read.

There are four snapshot imports/restores: three280-model lists with hash
699365A7 and one18-model list with hash4135AD9F. Counts/hashes match their
restores; peak retained references7/768, no invalid/deferred/active-temp state.

All replay-client telemetry checks pass with no dropped/unknown/read/clock
errors, and final open0/held0. Seven busy intervals pair successfully. Four have
loading0 but transition3 and coincide with imports at406781,409609 and504281ms;
they clear in roughly10-109ms. The remaining three occur during loading,
including one contextEnd interval that is not independently treated as readiness
recovery. There is no evidence here of the old recurring uninterrupted-playback
hitch pattern. A4MiB buffer working does not prove it improves on the already
working2MiB buffer.

## Conclusion and remaining scope

The alternate configuration works in the observed session while the actual
roster remains280. It validates the changed backing geometry and lifecycle,
not320 actual resident models or filling every configured limit to capacity.
There is no new issue requiring a patch or increase.

The user has also confirmed that earlier tests already covered original AI
with150/280 INIs and150-car AI with150 INI. Those comparisons should not be
requested again solely because R9 added diagnostics. Headroom320 was the one
new configuration comparison in that proposed group; it now has runtime
evidence. Other chapters' missions, nonzero traffic frequencies, actual larger
rosters and indefinite session lifetimes retain their separate test scope.

Reproduce with `python analysis/usercap_r9_run7044_2026-11-18/summarize.py`.
The directory retains generic.json and summary.json, including input hash,
phase details, audio reconciliation and configured replay validation. The
legacy replay decoder's fixed150/160 startup predicate is retained separately;
the applicable320/330 configuration is explicitly checked.

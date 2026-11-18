# R10 run23108: Replay summary-mode smoke supported

The user supplied this run as the Replay watch and replied that everything
seemed okay when asked about playback/audio and return/quit. That supports a
successful observed Replay test; it is not a separately timed account of the
entire clip or independently proven normal shutdown. Together with Story
run20268, this adds the missing basic Replay exercise for R10's logging change.
No patch, capacity increase or repeated long/full-trace run is indicated.

Input: `config/DSFV29s_usercap_r10_23108.log`,128645 bytes, SHA256
`1663036be9e7b4befd00b58d854f4f216c5d0873c4ed6e4d9f3d478bb4ea9ec5`.
532 records, last tick317344ms (~5m17.3s). Only this supplied log was reviewed;
no older Driver.log was attributed to this process.

## Configuration and imported traffic

The complete Headroom320 configuration is active:320 requests,330 model slots,
448 audio profiles,4096 global entries,128MiB shared audio,4MiB Replay reader,
768 retained references,180000 hash extras and1098 work/key/handler entries.
Logging is level1/fullTrace0 with output-only filtering and no disabled checks.

Both recorded phases reach native READY:

| Phase | Models | Request tick | First READY tick |
| --- | ---: | ---: | ---: |
|1|18|26109|29328|
|2|280|34437|50469|

The280-model Replay snapshot imports at34437ms and restores at34469ms, with
retained1 and roster hash699365A7 on both records. There are no invalid or
deferred restores. Sequential READY still does not independently certify
simultaneous drawable residency for every model.

Manager key storage peaks280/1098 with one buffer/redirection and no failure
or guard error. Audio registry peaks283/448 with no missing lookup, full or
guard condition. No ASI fatal/exception record or widened-storage integrity
failure is present in the recorded checks.

## Reader exercise and playback observations

Unlike Story run20268, this run constructs a Replay reader. Runtime status
shows one retained owner,4194304 bytes of backing, five constructions, four
destructions and four reuses by the last periodic sample. Allocation failures,
fallbacks, faults and unmanaged calls remain zero. One active construction in
the last periodic sample is not evidence of a leak or proof that the final
destruction did not happen: summary status is sampled about every30seconds and
there is no final exit snapshot.

The client probe reports three busy starts, two clears and one contextEnd by
61547ms. These counts do not increase through the last303250ms status sample;
open0 and held0 remain recorded. Read failures, dropped/thread-dropped records,
unknown clients and clock failures are zero; readChecks11 confirms the probe
was exercised. This supports absence of the earlier recurring busy-burst
pattern in the recorded playback interval and is consistent with the user's
positive observation.

Level1 omits individual REPLAY_CLIENT_EVENT and REPLAY_READ_OWNER records.
There are therefore no per-interval timestamps/durations or owner-field samples
to validate retrospectively. Do not run the full interval decoder and treat an
empty detail list as proof of zero busy intervals. The cumulative three starts
are retained. Exact loading flags/durations at those starts are not established
from these summary records.

Late CAP_QUEUE records show zero/null traffic headers with invalidCounts0,
consistent with unloading that context. They do not independently establish
the precise menu/quit sequence. R9's broader lifecycle results retain their
original version/workload scope.

## Audio: model setup succeeds; one local event-cap observation

All430 vehicle-audio setup attempts succeed with430 profile hits and bankReady,
setupFail0 and every R9 setup-reason counter0. There is no evidence of a missing
vehicle model/default profile or exhausted enlarged profile registry.

The older event-wrapper probe separately records full16=1 for a single owner,
with refused1 and peakObserved16, first visible in the151172ms summary. Static
inspection of the existing probe confirms it counts requests entering the
native per-object16-event gate at owner+16C. That is distinct from the enlarged
resident model-profile and global audio capacities. The lookup/create probe
has createFailureCalls0; its other zero/none-event counters are not the vehicle
setup failure counter.

The full16 detail record is not present at level1, so the event/model/category
and audible consequence of this one local refusal are not identified here.
The user reports playback/audio seemed okay. Preserve this observation; do not
claim that every native audio request was accepted, assume it is harmless in
all workloads, or propose a blind16-slot increase from it. It is not evidence
that the original global vehicle-audio problem has returned. A focused follow-up
would only be useful if an audible symptom makes this local gate relevant.

## Summary output and analyzer scope

The file is only128645 bytes (~126KiB). The final logging sample at300422ms
records32838 attempted outputs,487 accepted and32351 suppressed, fileStatus0.
Counts are before that line and are not a terminal snapshot.

The generic analyzer has49/50 checks true. Only logging_full_trace is false,
as intended at level1. recorded_integrity_pass is therefore false; this is a
full-evidence restriction, not a newly detected gameplay fault. The separate
per-object full16 observation is outside that generic check set and is retained
explicitly above. No change to the analyzer or safety rules was made.

The basic R10 Story and Replay summary-mode smoke checks now have positive
observed results. Broader chapter missions, nonzero traffic frequencies,
actual larger rosters and indefinite lifetimes remain separate validation.
Packaging/publication can be the next stage if authorized; this review does
not package, commit, push or change the installed game.

## Evidence

`analysis/usercap_r10_run23108_2026-11-18/generic.json` is reproduced with:

`python scripts/analyze_generic_limits_log.py config/DSFV29s_usercap_r10_23108.log --output analysis/usercap_r10_run23108_2026-11-18/generic.json`

`summary.json` retains configuration, phase times, import/restore events,
periodic client counts, audio observation and input hash. Analysis/docs only:
no production source, ASI, INI, build, installed plugin, assets, save,
game/debugger launch, package, commit or push changes were made.

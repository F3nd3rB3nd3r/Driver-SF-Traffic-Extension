# R7 run21688: widened query exercised; transition audio failures recover

Supplied log: `dist/USERCAP_R7/DSFV29s_usercap_r7_21688.log`,24539113 bytes,
SHA256 `1050dc7d2a7aeef6bf4de5bd00742a3051d073925be6752003154763b3afb02e`.
Last recorded tick747515ms (~12m27.5s). This review writes analysis/handoff
only; no source/ASI/INI, game asset or save changes, build or game/debugger run.

## Capacity correction and traffic phases

Startup applies280 requests,290 model slots,384 audio models,4096 global
audio entries,128MiB shared audio,2MiB reader,512 retained references,
180000 extra hash nodes and802 coupled stream-handler/scheduler entries.
CAPACITY_QUERY_START is valid. The final sampled query status shows28 vehicle
overrides, one native fallback, configured290 and last metadata byte255.
The correction was exercised in the live run; counters do not identify an
individual mission request or prove its subsequent loading completion.

Four accepted traffic phases are recorded, all with complete rosters and a
native all-READY checkpoint:

| Phase | Requested | Request tick | First observed all READY |
| --- | ---: | ---: | ---: |
| 1 | 18 | 29609 | 32297 |
| 2 | 280 | 32656 | 46375 |
| 3 | 18 | 429390 | 430468 |
| 4 | 280 | 554031 | 562140 |

The two280 phases use the same roster hash699365A7. Phase2's last record is
429468ms; phase4's last record747109ms. These are sequential native readiness
observations, not direct proof of simultaneous drawable residency or audible
playback. Substantial audio/vehicle activity and traffic transitions distinguish
this session from the R6 loading-screen wait; the user's observed Story entry
and exact activities are recorded in the confirmation below.

## Capacity and lifecycle observations

No fatal or exception record appears. All startup, configured geometry and
instrumented integrity checks pass except the deliberately strict zero-audio-
setup-failure condition discussed below. Stream-handler/scheduler and traffic
queue guards/failures stay zero. Sampled handler occupancy peaks551/802;
four queue peaks262/3/550/37 remain within802. Two manager and two scheduler
generations are seen, with one recorded destruction each; this is lifecycle
coverage, not proof of a complete final shutdown or unbounded lifetime reuse.

Hash lifetime extra issuance peaks55032/180000. Audio registry peaks284/384
with full0, guardBad0 and lookup misses0. Shared audio usage peaks18594184
bytes (~17.73MiB)/128MiB; no sampled90% pressure. Global audio ceiling hits
and per-record8/255 full counters stay zero. These observations give no reason
to increase these capacities for this run. Sampled process private memory
peaks1396260864 bytes (~1.30GiB), working set1062944768 (~0.99GiB); these
are process metrics, not vehicle asset sizes or an address-space guarantee.

The replay reader records constructs0/owners0 and writerBypass1, so playback
and the reader fix were not exercised in this log. No absence of fatal records
can independently certify how the game exited.

## Audio exception to a clean analyzer result

The generic analyzer returns `recorded_integrity_pass=false` solely because
`audio_setup=false`:1138 successful setups and48 failed setups were observed.
All48 failures appear in the samples at431484ms and432484ms, immediately
after the280-to18 request at429390ms. Six model IDs each accumulate eight
failures:171,184,190,197,213,272. No further failure increment appears through
the last sample747109ms, including the later return to280 traffic.

Every affected model subsequently records additional successful setups:
five by433484ms and model184 by441531ms. Final successes per model are
3/12/19/12/13/20 respectively. Their keys show bankNotReady0, misses0, full0,
disabled0. Thus the failures were confined to this transition and were followed
by successful setups. This does not establish why each native setup returned
false, or whether a brief audible interruption occurred; detail logging is
bounded and the counters cannot establish subjective audio quality.

The pattern does not support persistent silence from registry capacity
exhaustion, nor a capacity increase on its own. The user subsequently reports
no missing vehicle sounds; the native failure cause remains unclassified.
Do not relax the analyzer or label the entire run a clean pass to hide48
failures. No probe or further ASI was built for this review.

Evidence: `analysis/usercap_r7_run21688_2026-11-15/generic.json` and
`summary.json`, plus the scoped reproducible `summarize.py`.
## User confirmation and conclusion

The user confirms Story loaded properly, a speed-trap activity and free-roam
pursuits worked, and no missing vehicle sounds, rendering problems or FPS issues
were noticed. No menu/Replay transitions were attempted. The user recalls a
normal game exit; the log has no independent definitive exit marker.

This supports the R7 mission-capacity correction for the supplied280-car
Story/activity/pursuit workload. The strict analyzer remains false for the
48 transient setup failures, despite later successful setups and no reported
audible issue. No further capacity increase or ASI is indicated by this log.
Next useful validation is Story -> main menu -> Replay -> main menu -> Story,
then normal exit, retaining the same ASI/INI/AI. Replay playback and that
larger-roster lifecycle are not established by run21688.

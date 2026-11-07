# Default64MiB and audio2048 run comparison — 2026-11-05

Both supplied tests installed and ran with the intended settings. The2048 global audio expansion did not approach the original255 ceilings: actual insertion peaks were28,51 and36, with no global ceiling hits or selected reuse. Neither separate per-record8/255-user gate refused. Together with the user's unchanged-audio observation, this is strong evidence against these specific slot limits causing the remaining audio loss during run30860.

The previously missing default64MiB log is useful: its roughly63-minute run confirms substantial shared-default and platform memory headroom. It does not support another increase to those budgets. This comparison does not rule out other uninstrumented playback, stream or voice constraints.

## Inputs and validity

| Log | Size / duration | Validation |
|---|---|---|
| `dist/MAIN_150/DSFV29s_audio_default64m_16680.log` | 6198926B; last audio status3787875ms (~63m08s) | All existing analyzer startup/live/file/head/counter checks pass; live shared default request67108864B, threshold3F666666. |
| `dist/MAIN_150/DSFV29s_audio_slots2048_30860.log` | 449971B; last audio status163782ms (~2m44s), new slot snapshots through163985ms | Same checks pass; AUDIO_SLOTS_START status1/patches1/hooks9. All three initialized lists repeatedly report configuredMax2048 and allocatedSlots2048; valid headers and monotonic cumulative counters. |

Both retain150 requests,160 graphics slots, ED/EF and indexed262144. Log files end with a newline. There is no evidence here of a startup rejection or a capacity patch that failed to apply.

Reproducer: `python scripts/compare_audio_slots_logs.py dist/MAIN_150/DSFV29s_audio_default64m_16680.log dist/MAIN_150/DSFV29s_audio_slots2048_30860.log --output analysis/audio_default64_vs_slots2048_runs_16680_30860_2026-11-05`

Machine-readable per-run summaries, input hashes and comparison: `analysis/audio_default64_vs_slots2048_runs_16680_30860_2026-11-05/`.

## Exact slot observations in30860

| Global list | Allocation attempts / inserts | Actual insertion peak | Update-entry peak | Ceiling hits | Selected reuse |
|---|---:|---:|---:|---:|---:|
| A0 | 767 /767 | 28 | 28 | 0 | 0 |
| A1 | 352 /352 | 51 | 51 | 0 | 0 |
| B | 196 /196 | 36 | 10 | 0 | 0 |

All lists have7631 native update samples and invalid0. Manager B's update-entry peak10 is lower than its actual insertion peak36 because objects can be inserted and removed between update entries; use the actual insertion peak for that comparison. Final update counts are0,0,2; removal after earlier playback does not erase the retained peaks. Native observations begin at1250ms, preceding the first150-request sample at28188ms.

The eight-user attachment path was exercised227 times, largest pre-attachment count2, full0. The byte255-user path was exercised40 times, largest pre-attachment count2, full0. These are pre-attachment counts, not proof that no record ever held a third user after insertion. They do show no measured full-gate refusal. Both paths have invalid0.

## Other audio measurements

| Measurement | Default64 run16680 | Slots2048 run30860 |
|---|---:|---:|
| Traced bank callbacks /actual AKRESULT1 | 178 /178 | 160 /160 |
| Distinct retained bank IDs | 157 | 155 |
| Emitter attempts /peak active before /refusals | 9808 /45 /0 | 697 /28 /0 |
| Nonzero-size Malloc /Malign failures | 0 /0 | 0 /0 |
| Shared-default sampled byte peak /usable | 5485704 /67105684 (8.175%) | 5428928 /67105684 (8.090%) |
| Shared-default pressure samples /samples | 0 /18841 | 0 /811 |
| Platform sampled byte peak /usable | 1093396 /3142548 (34.793%) | 933668 /3142548 (29.711%) |
| StaticBanksPool sampled byte peak /usable | 4239752 /4369300 (97.035%) | 4239752 /4369300 (97.035%) |
| Numeric normal event returns /zero results | 22384 /3545 | 1512 /123 |
| Early external-source allocation zero results | 0 | 0 |
| Audio bank-operation queue peak /boundary attempts | 2 /0 | 2 /0 |

The shared-default periodic pressure rows have slightly different maxima from the pool snapshots because they sample at different times. Both are far below90%. StaticBanksPool still has the original allocation, no traced allocation failure, and does not participate in the demonstrated shared-default/platform90% playback gate. Its high occupancy alone is insufficient to identify it as the cause. Successful bank callbacks establish success only for traced operations, not that every needed bank/event/object exists or produces voices. Zero allocation failures also do not measure every allocator/stream path.

The runs have different durations/routes; the8.13% versus15.84% zero-result percentages cannot establish a2048 improvement or regression. User reports remaining audio unchanged.

## Persistent failed-event lead

All123 normal zero-result details were retained in30860. Three event hashes account for118 failures, all through the shared object-event wrapper return0084CCC9:

| Event hash | Failures in30860 | First /last tick |
|---|---:|---|
| 2CA33BDB | 95 | 86235 /162141 |
| 38C61846 | 12 | 34313 /151407 |
| DFBD7040 | 11 | 42907 /154579 |

The same three hashes account for124 of the first128 retained failures in16680. That log's3545 total zero results exceed its128-detail bound, so later individual event IDs/reasons cannot be reconstructed. The other30860 details are the four startup/event failures A6073B55,2E385822,12E6FA03,72DCBB24 and one zero event ID through0072F37D. In total120/12330860 failures pass through0084CCC9.

These hashes are not yet mapped to Zonda/370Z/Bentley or to engine/tire/horn categories. They could include unrelated events. A zero playing ID identifies failed admission on the instrumented API path, not its exact internal reason. A nonzero playing ID also does not prove eventual audible output.

## Next bounded investigation

Keep existing artifacts/settings fixed. Further growth of these measured slot limits is unsupported. Trace the failing shared event-post route and distinguish event-definition lookup/registration, object state and playing-record/admission failure branches. Existing static entry points are numeric PostEventC54DA0, shared lookupC53FB0 and playing-record allocationC5F0CA. Establish which event/object/bank is used by a working vehicle and by a silent vehicle before attributing the repeated hashes to the missing car sounds. If static evidence cannot resolve runtime branch outcomes, define a narrow reason probe; successful vehicle posts would instead require following subsequent playback/voice/stream/mixer behavior.

This turn only read the two supplied logs and focused existing notes, added a comparison analyzer and wrote results/handoffs. No ASI/source behavior change, build, install, game run, new executable disassembly, debugger, web search or other game-folder inspection.

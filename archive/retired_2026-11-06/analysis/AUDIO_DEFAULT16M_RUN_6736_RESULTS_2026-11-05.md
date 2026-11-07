# Run6736: default-pool pressure removed; vehicle-specific audio remains missing

Input: `dist/MAIN_150/DSFV29s_audio_default16m_6736.log`, 685285 bytes,
2411 lines, SHA256
`191AA5C5CD4DEC04B4908B5196D0988B50ADF0CDF6DBD5AB641678B1D025A289`.
The final audio-status tick is398953ms (about6m39s).

The user reports tire/engine/horn audio working on the Ford Crown Victoria Cop
Car, Volkswagen Scirocco and Chevrolet C10 tow truck, while the Pagani Zonda
and Nissan370Z remain silent in those categories. No new FPS observation was
supplied. The log has no car-selection/name markers, so those observations
cannot yet be bound to individual bank IDs or failed event hashes.

**Conclusion:** the16MiB test installed and removed sampled default-pool pressure.
Neither that pool nor the platform pool approaches its90% gate in this run.
Further expansion of either pool is not supported by these measurements.
There are109 actual numeric PostEvent failures to investigate for the remaining
vehicle-specific symptom. Their precise cause and sound categories remain unknown.

## Startup and live configuration

All expected markers pass:

- Indexed startup revision2, stock/owned-handler verification,13 indexed patches,
 31 pending redirects, external262144 table.
- `AUDIO_LIMIT_START status=1 reason=armedBeforeAudioInit patches=1`, request
  changed5872025 to16777216B, threshold bits3F666666 unchanged.
- Six audio diagnostic hooks and three capacity hooks installed.
- Live `AUDIO_PRESSURE` confirms configuredBytes16777216 and usableBytes16774036.
- Later requests150/graphics160/startupStatus2/indexedStatus2; all sampled heads
  valid and all fileStatus0. Audio counters are monotonic.

The expansion did not merely report an intended setting: the live default-pool
descriptor has262144 blocks of64B and roughly16MiB usable storage.

## Capacity results

| Measurement | Run6736 |
|---|---:|
| Default pool sampled high-water | 5466932 /16774036B =32.592% |
| Default pool spare bytes at sampled peak | 11307104B |
| Default pressure samples | **0 /1976** |
| Platform pool sampled high-water | 978544 /3142548B =31.139% |
| Platform spare bytes at sampled peak | 2164004B |
| Malloc /Malign nonzero-size null returns | 0 /0 |
| Emitter attempts /active-before peak /refusals | 1113 /30 /0 |
| Middleware pool descriptors | Peak7/20 |
| Bank queue projected peak /boundary attempts | 2/50 /0 |
| Deferred cleanup sampled active peak | 4/90 |

The new pressure monitor emits396 one-second rows, retaining cumulative counters
for1976 approximately200ms samples through398141ms. Every row agrees with the
native strict predicate `used *16777216 > usable *15099494` and all classify as
below threshold. The dedicated pressure rows cover the default pool only;
platform headroom comes from the independent200ms high-water sampler reported
in `AUDIO_POOL` rows. Neither is a hook on the exact native gate, so very short
transients between samples are not excluded. With both observed peaks near32%,
there is no measured basis for another increase of these two pools.

Newly restored playback increased platform peak versus the earlier314840B to
978544B, but this remains far below its gate. Old and new routes/durations are
not matched, so the change in usage is descriptive rather than a controlled
per-sound allocation measurement.

Other sampled pool peaks:

| Pool | Peak /usable | Interpretation |
|---|---:|---|
| FilePackagesPool (ID2) | 481448 /553876B (86.923%) | No observed allocation failure |
| PrepareEventPool (ID3) | 0 /304020B on byte counter | No observed byte allocation |
| StaticBanksPool (ID4) | 4239752 /4369300B (97.035%) | Tight; no recorded allocation/bank failure |
| Fixed-block pool1 | Minimum1 free out of4 blocks | No sampled exhaustion; peak byte counter786432B |

StaticBanksPool remains close to full. The native90% playback gate checks the
default and platform pool IDs, not this pool. Its97% occupancy alone therefore
does not reproduce the demonstrated pressure mechanism. Failure-only counters
also remain zero, so this run does not justify expanding it as a claimed fix.
Other unmeasured paths, fragmentation and transient failures are not ruled out.

## Bank completion and failed events

All154 instrumented bank callbacks report actual result1, and all154 detail
records were retained. There are154 distinct bank IDs, with no repeated ID among
these completions. The last callback is at95219ms for bankC620DB70. The scope is
still the instrumented resource wrapper: completion does not prove subsequent
residency, correct vehicle association or that every expected event is defined.

There are2847 normal numeric PostEvent returns:2738 nonzero admissions and109
zeros (3.8286%). The early external-source failure return count is0. All109
zero details fit within the128-record bound, so this run's zero-result ID/caller
distribution is complete for the hooked normal exit:

| Event hash | Native return address | Count |
|---|---|---:|
| 2CA33BDB | 0084CCC9 | 40 |
| 38C61846 | 0084CCC9 | 32 |
| DFBD7040 | 0084CCC9 | 30 |
| 00000000 | 0072F37D | 2 |
| A6073B55 | 00848422 | 1 |
| 2E385822 | 0084CCC9 | 1 |
| 12E6FA03 | 00848439 | 1 |
| 72DCBB24 | 0084CCC9 | 1 |
| FA8A6AF7 | 0084CCC9 | 1 |

The three leading hashes account for102/109 failures.105/109 use return address
0084CCC9, the shared object-event call at0084CCC4. These are return addresses,
not unique car identities. A zero event ID in the other two calls is recorded
separately and is not evidence of a particular car's missing sound.

Failed IDs80710A6C/80710A6D/FB05593C/FB05593D appeared in the earlier retained
run4188 failures but do not appear among the new run's complete109 zero details.
This cannot prove those same events succeeded: R1 does not retain successful
event IDs, and the user's activity and duration differ.

Existing static evidence identifies missing event-definition lookup at00C54012
and playing-record failure cleanup at00C54052 in common helper00C53FB0. R1 does
not distinguish these branch reasons. With no observed allocator failure and
ample default/platform headroom, event-definition availability and association
are the next focused leads for these exact hashes. They are not yet proven to
be the Zonda/370Z engine, horn or tire events, or the cause of every missing sound.
A nonzero admission ID likewise does not establish audible playback.

## Preserved rendering and next action

Indexed empty samples remain0 across1984 initialized samples; pending peak512,
final0. No observed native bank-queue boundary issue occurs. This validates the
preserved configuration under this run; it does not count simultaneously drawable
vehicles or substitute for the user's visual observations.

Keep the16MiB default-pool build as the improved baseline. Do not increase it or
the platform pool again on this evidence. Next bounded investigation:

1. Resolve what the three repeated failed hashes represent, using the already
   scoped executable and authorized Install/media audio definitions if needed.
2. Follow lookup/registration and resource association for a working vehicle
   against Zonda/370Z. Check unloaded/missing definitions and wrong or absent
   bank association before treating the issue as a new capacity failure.
3. If static evidence cannot distinguish failure reasons, design a focused
   missing-definition versus playing-record/registration probe for those IDs.
   Do not build another broad capacity variant without resolving that gap.

No new ASI/game-source change, build, installation, executable disassembly,
process attachment, game run or web search occurred in this log-analysis turn.
The reusable log analyzer was extended to validate the16MiB variant, its live
configuration and exact sampled pressure predicate.

Reproducer:

```powershell
python scripts/analyze_audio_probe_log.py dist/MAIN_150/DSFV29s_audio_default16m_6736.log --output analysis/audio_default16m_run_6736_2026-11-05/summary.json
```

Machine-readable evidence: `audio_default16m_run_6736_2026-11-05/summary.json`.

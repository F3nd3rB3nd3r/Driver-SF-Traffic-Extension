# CHAPTERS_R1: two challenge-switch runs

The chapter-independent request selector passes its recorded guards, but the build fails challenge-transition stability. The user reports trying random Challenges including Gone!, President's Run and Tanner's Day Off; their related traffic loaded, then the game crashed without a displayed exception/source. Exact challenge-to-event correspondence and visual/audio outcomes were not recorded. Both logs terminate immediately after admitting another Challenge traffic request.

## Inputs and reproducibility

Only the two supplied logs were analyzed, with current telemetry definitions and the existing callback-arena dump report for interpretation. No old-log discovery, game/debugger execution, ASI changes, build or installation.

| Log | Bytes | SHA256 | Last recorded time |
|---|---:|---|---:|
| DSFV29s_chapters_33044.log | 3161022 | c5accac3618113c14b3646c9514d3d3e05ac34c443a402fba9a3c8aaeddce3ac | 465.969s |
| DSFV29s_chapters_18228.log | 1190617 | eff95034a5aa9f78a1f0e66fb223c0bbce13dda1c7f663745729b469bcda4edb | 184.078s |

Reproduce:

```powershell
python scripts/compare_chapter_logs.py dist/MAIN_150/DSFV29s_chapters_33044.log dist/MAIN_150/DSFV29s_chapters_18228.log --output analysis/chapters_pair_2026-11-06
```

Evidence: per-run JSON and comparison.json in `analysis/chapters_pair_2026-11-06/`. The analyzer separates structural checks from stability and playback; it does not label these crashed runs a pass.

## Selector coverage and terminal events

Both CHAPTER_START records have revision1/status1, 150 requests/160 slots, 80 profiles/55 expandable, checked live assets and no added native hooks. Each log contains nine expanded events through caller0076FE9A, effectiveCount150, with one stable request owner. No recorded unknown profile, source/asset mismatch, refusal or other caller. Observed settings include Chapter1/f0, Chapter7/f0, Exposition/f0 and Challenge/f0/f2. These are traffic-setting names, not proof of actual chapter/save-manager coverage.

| Run | Ordered settings (C1/C7 = chapter traffic, E = Exposition, Q = Challenge) |
|---|---|
| 33044 | C1 -> C7 -> Q/f2 -> C7 -> E -> C7 -> C1 -> C7 -> Q/f0 |
| 18228 | C1 -> C7 -> Q/f0 -> C7 -> E -> C7 -> E -> C7 -> Q/f2 |

Last lines:

```text
33044: CHAPTER_EVENT reason=expanded tick=465969 caller=0076FE9A owner=1BDBF9D0 hash=D433D631 frequency=0 nativeCount=18 effectiveCount=150 profile=34
18228: CHAPTER_EVENT reason=expanded tick=184078 caller=0076FE9A owner=1B5DA298 hash=D433D631 frequency=2 nativeCount=16 effectiveCount=150 profile=36
```

There is no subsequent CAP_STATUS in either log. The previous CAP_STATUS is719ms/78ms before the terminal event. CHAPTER_EVENT is emitted **before** the request copy/publication and native processing, so these lines prove admission, not completed native processing or the faulting instruction. Both final periodic CHAPTER_STATUS records show8 expansions because the ninth event follows that snapshot; this is expected sampling lag. No ASI FATAL record appears. A silent user-visible crash does not establish absence of an OS exception; no new exception context is supplied here.

## Inherited limits and integrity

Last sampled state is healthy for the patched structures; it cannot exclude corruption in an uninstrumented arena or after that sample.

| Measurement | 33044 | 18228 |
|---|---:|---:|
| Indexed slots / last request / graphics slots | 262144 /150 /160 | 262144 /150 /160 |
| Indexed status / head valid / sampled empty count | 2 /1 /0 | 2 /1 /0 |
| Cleanup peak | 1176 | 1176 |
| Cleanup appends = processed | 192835 | 147748 |
| Cleanup drain starts = ends | 43240 | 16622 |
| Cleanup current / depth | 0 /0 | 0 /0 |
| Audio registry current / capacity | 150 /192 | 150 /192 |
| Audio setup success / failure | 676 /169 | 107 /248 |
| Audio profile hits / misses | 845 /0 | 355 /0 |
| Audio bank ready / not ready | 845 /0 | 355 /0 |

All inherited startup/structural checks pass. Cleanup has zero full/invalid/guardBad/duringDrain/drainMismatch and no32768 boundary crossing. Audio registry has zero full, fallback, attachment refusal, invalid, instance/key overflow or guard errors; no profile removal occurred. Measured global audio-slot peaks A0/A1/B are250/53/88 and32/53/41, with zero ceiling hits. Per-record8/255 checks show peak-before2 and no full event. Shared64MiB audio-pool maximum logged usage is5,476,824 /5,442,472 bytes; no90% threshold pressure sample.

PENDING512 initialized successfully but recorded zero generations, attempts, stores and walks in both logs. This path was **not exercised**; these runs cannot revalidate main-menu teardown or normal exit. One request owner and one audio attachment per run likewise do not validate reload lifetimes. Counts do not establish150 simultaneously drawable vehicles.

## Audio setup failures after traffic changes

Failures are downstream of successful profile lookup and bank-ready checks, with disabled count0. They do not indicate exhaustion of the192-profile registry, 2048 global slots or64MiB pool. The failure reason remains undecoded; native setup return0 can include a transitional or unsupported outcome and is not by itself proof of audible loss.

Counter deltas between each event and the next event:

| Run | Setting event time | Setting | Added setup failures |
|---|---:|---|---:|
| 33044 | 135.047s | Challenge/f2 | 54 |
| 33044 | 269.765s | Exposition/f0 | 84 |
| 33044 | 442.781s | Chapter1/f0 | 31 |
| 18228 | 49.875s | Challenge/f0 | 80 |
| 18228 | 94.813s | Exposition/f0 | 84 |
| 18228 | 132.750s | Exposition/f0 | 84 |

Other completed event intervals add zero; terminal events have no subsequent snapshot. This timing points to setup behavior during traffic/mission changes rather than a registry that never loaded particular profiles. It does not distinguish expected mission policy from a regression. These are event-window correlations, not exact per-call causal attribution. Bounded detailed setup logging may finish before later failures, so absence of retained failure details cannot override aggregate failure counts.

## Next investigation

Prior supplied dump `Driver_20261106_001049.dmp` on PENDING512_R1 showed a malformed native callback iterator and null indirect call at00859039. Static geometry identified21 global callback groups with64 inline12-byte nodes each and unchecked fresh insertion005567D0. A conditional adjacent-pool overflow replay reproduces the exact malformed iterator. See `CRASH_20261106_001049_CALLBACK_ARENA.md`; the actual overflowing write/count was not captured.

CHAPTERS_R1 still leaves that arena at64. Crashing during another challenge switch is consistent with the candidate, but these two text logs cannot prove the same fault. The next focused work should audit every relevant callback writer, removal/recycling/reset path and header user before designing an extension with occupancy/integrity logging. Raising an unrelated already-expanded limit or requesting another general driving test would not resolve that missing evidence. A new dump is useful if available, but is not required to begin that static audit. Separately trace native audio setup failure branches under setting changes; avoid assuming these failures share the crash cause.

Keep CHAPTERS_R1 marked experimental; preserve PENDING512_R1 as the earlier normal-driving/menu-tested baseline, with its challenge-switch crash limitation still documented. Parked-car fallback remains unresolved and unlinked to these crashes.

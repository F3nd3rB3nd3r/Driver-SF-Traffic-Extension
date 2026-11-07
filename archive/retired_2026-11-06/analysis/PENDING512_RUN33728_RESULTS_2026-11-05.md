# Pending512 run33728: passing main-menu/reload result

The user reports no freezes, crashes or exceptions. The supplied log corroborates a strong passing result for the targeted pending-map and cleanup fixes over40m49.547s, with three150-request preload passes and three distinct pending-map lifetimes. Keep PENDING512_R1 as the current working baseline; no additional capacity increase or new ASI is justified by this run.

## Input and checks

`dist/MAIN_150/DSFV29s_pending512_33728.log`,14073147 bytes. Input SHA256 and full evidence are recorded in `analysis/pending512_run33728_2026-11-05/summary.json`.

Pending startup passes status1/capacity512/two traversal hooks/shared insertion hook007D7270. Inherited cleanup, audio192, global audio-slot, audio pool, capacity and indexed checks pass. No fatal diagnostic appears. All checked cumulative pending counters are monotonic; invalid/full/allocationFailed/guardBad remain0 throughout.

## Pending map

| Lifetime | Owner/map | Maximum recorded active/issued | Redirects beyond64 | Recorded empty completion |
|---|---|---:|---:|---:|
|1 |1AF986C4 |154 /154 |1 |243829ms |
|2 |2D868ECC |154 /154 |1 |854407ms |
|3 |1B80EECC |154 /154 |1 |2443766ms |

All462 insertion attempts become462 stores:154 per lifetime. All56 nonempty traversal starts have56 returns. Final current count0/depth0;154 recycled nodes remain in the final native free list, which is expected reuse bookkeeping rather than an active-entry leak. Active/issued high-water154 is safely below512. Each lifetime explicitly redirected at64; this exercised the path that previously overwrote the native sentinel and neighboring map headers.

These are three distinct owner addresses. Same-address constructor reuse and recycled insertion were fixture-tested, but this live run reports resets0/recycledInserts0, so do not claim those particular branches were exercised in-game. Old arena guards remain healthy across the observed lifetimes.

## Cleanup and audio

Cleanup peak52176 exceeds the original32768 limit, supporting retention of its separate expansion. Total509502 tracked appends equal509502 processed entries, with208620 drain starts and ends. Final queue count0/depth0; full, guard damage, invalid observations, appends during drain and drain mismatch remain0. There are three original-boundary crossings and56876 cumulative append attempts above that boundary.

Audio registers450 profiles and removes450 across the three150-profile passes. All2027 measured model lookups, bank-readiness checks and vehicle-audio setups succeed, with no missing profile, fallback or setup failure. Model registry peak150 remains within192. The inherited measured audio pools/slots do not report the previously tested allocation/capacity failures. Setup success is not a measurement of every audible category, but there is no measured regression in this run.

Final capacity status reports150 requests,160 graphics slots, ED/EF baseline, indexed262144/status2/valid free head and pending0. Indexed empty samples0 and fileStatus0. These do not establish150 simultaneously drawable vehicles.

## Assessment and next state

The observed menu freeze is resolved in the user's test, with completed native traversals and intact storage rather than skipped work or an early loop escape. Both new boundaries were exercised:154 pending entries and52176 cleanup entries. Preserve pending512, cleanup262144, audio192 and the existing150/160/ED+EF/indexed262144 settings.

This is one sustained run with multiple map lifetimes, not proof against every future game/mod failure. The log does not record the final OS exit status; the user's report supplies the successful visible outcome. No further forced test or dump is needed on this evidence.

## Reproduction and scope

`python scripts/analyze_pending_log.py dist/MAIN_150/DSFV29s_pending512_33728.log --output analysis/pending512_run33728_2026-11-05`

The analyzer checks pending startup, guards/refusals, insertion/walk balance, generation census and expanded-path use, reusing inherited cleanup/model/audio/indexed analyzers. Evidence is saved in the summary JSON with per-generation events and final counters.

Only this explicitly supplied pending512 log and relevant current handoff/analyzer/telemetry definitions were read. No prior-log scan, game-source behavior change, new ASI build, installation, game execution, debugger, dependency installation or web search.

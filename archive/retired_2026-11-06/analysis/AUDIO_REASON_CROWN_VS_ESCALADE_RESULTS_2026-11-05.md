# Police Crown Victoria versus Escalade — reason-probe comparison

Both runs validate the probe and show no measured capacity refusal or completion-record cleanup failure. They loaded the identical set of153 traced bank IDs, each successfully. Immediate lookup-failure patterns are nearly identical despite the reported vehicle audio difference. In the Escalade's final30-second window,114 wrapper events were admitted with no lookup failures, creation failures or16-entry refusals; this includes background audio and does not prove the Escalade requested or played its own sounds.

The user clarified that the working Ford Crown Victoria is a **cop car**: pressing horn turns on the **siren**. Treat the observations as working engine/tire/siren, regular horn untested. Escalade lacks its own engine/tire/horn sounds. Crown-only event IDs may represent valid police/siren behavior, so their absence from Escalade is not by itself evidence of broken vehicle initialization. Their meanings remain unverified.

## Inputs and user observations

- `dist/MAIN_150/DSFV29s_audio_reason_24164_crown_victoria.log`:341334B, last reason status88437ms (~88.4s).
- `dist/MAIN_150/DSFV29s_audio_reason_20332_cadillac_escalade.log`:366637B, last reason status96453ms (~96.5s).
- User estimates about40s from launch to in-game. Exact later action times were not recorded. Both exits were Alt+F4 with no intentional menu sequence before quitting.
- User confirms Escalade missing its own engine/tire/horn while Crown Victoria works, then clarifies the Crown horn input activates its police siren.
- Other cars' driving sounds/horns and city ambience remain audible, including horns from traffic blocked by the user. Thus background events in the Escalade capture cannot automatically be assigned to the driven car.

Reproducer: `python scripts/compare_audio_reason_pair.py dist/MAIN_150/DSFV29s_audio_reason_24164_crown_victoria.log dist/MAIN_150/DSFV29s_audio_reason_20332_cadillac_escalade.log --output analysis/audio_reason_crown_vs_escalade_24164_20332_2026-11-05`

Per-run summaries/hashes and end-relative profile comparisons: `analysis/audio_reason_crown_vs_escalade_24164_20332_2026-11-05/`.

## Setup and native outcomes

Both pass inherited startup/live checks and reason status1/hooks10/changedLimits0. Global2048/default64MiB allocation and native16-entry object gate are retained. Invalid0, ownerOverflow0, eventOverflow0. Both complete with a final newline, although Alt+F4 does not guarantee a final counter flush.

| Measurement | Police Crown Victoria | Cadillac Escalade |
|---|---:|---:|
| Lookups |353 |279 |
| Definition hit /miss |345 /8 |270 /9 |
| Playing-record creation failures |0 |0 |
| Wrapper requests /accepted /zero |337 /331 /6 |260 /253 /7 |
| Full16 refusals /largest observed count |0 /10 |0 /10 |
| Delivered /validated callbacks |315 /315 |259 /259 |
| Delivered type1 /handled type1 /removals |305 /305 /305 |245 /245 /245 |
| Unmatched completion entries |0 |0 |
| Distinct owner addresses /registrations |24 /113 |20 /124 |
| Distinct observed event IDs |82 |74 |
| Actual bank callbacks /AKRESULT1 |153 /153 |153 /153 |
| Nonzero-size Malloc /Malign failures |0 /0 |0 /0 |
| Game emitter peak-before /refusals |23 /0 |19 /0 |

Final lookup/wrapper/cleanup balances are exact in both runs. Bank callback histograms are identical, not merely equal totals: each of the same153 IDs appears once with success. This does not prove the correct Escalade content was selected, that expected child/media objects exist, or that accepted events generated audible voices.

Global A0/A1/B peaks are23/51/35 and23/11/6, no ceiling/reuse. Eight-user/byte255-user refusals0 in both runs. Shared default sampled peaks8.114% and8.057%, platform28.180% and25.617%, with no sampled pressure. The allocator/gate/refusal results remain clear. Indexed empty samples0 across439/479 initialized samples, pending final0, bank-operation queue peak2/no boundary attempts.

## Approximate in-game and exit-relative windows

The comparator uses cumulative aggregate snapshots: latest at/before40s as an approximate load boundary, then the final30s and15s relative to the last reason status. Aggregate updates occur about every5s, so these are approximate intervals, not precise keypress reconstructions. The last owner/event aggregate occurs at85422ms and95453ms; the Crown capture can omit roughly3s of final aggregate activity and Escalade roughly1s. The user supplies no exact vehicle-entry/keypress marker.

| Window | Crown accepted /lookup misses | Escalade accepted /lookup misses |
|---|---:|---:|
| After estimated40s load |291 /2 |224 /2 |
| Last30s |137 /1 |114 /0 |
| Last15s |105 /1 |60 /0 |

These are object-wrapper event deltas across the whole scene. Runs differ in duration, traffic, driving and police siren behavior, so totals/rates do not establish an audio improvement or failure. They establish that background admission continues without a new general failure in the silent-car run.

## Missing definitions do not separate the two cars

Both runs have the same four early singleton misses (A6073B55,2E385822,12E6FA03,Menu_Move72DCBB24), two misses of38C61846 near32..34s and oneDFBD7040 near41s, followed by a single none-hash2CA33BDB miss later in the scene. Escalade has one additional early missFA8A6AF7 at10391ms.

The recurring38/DF pair still uses the same generic caller0072DFE5 and the corresponding persistent object in each process. None occurs at77937ms in Crown and52078ms in Escalade, through0084CDD0, on different scene objects. All failures are directly lookup misses; creation/full16 remain0. This near-identical miss pattern in working and silent captures is poor evidence that those IDs explain the Escalade's missing engine/tire/horn.

The Escalade final30s show no lookup miss at all. That narrows the next investigation to whether the driven car issues the right requests/gets its audio state initialized, versus failure after admission. It cannot distinguish those cases without identifying the driven vehicle's requests separately from traffic/ambient sounds.

## Candidate request differences, with the police-siren confound

Three IDs are accepted in Crown and absent from the entire Escalade event table:

| Candidate event | Crown total accepted | Owner /registrations | Final30s accepted |
|---|---:|---|---:|
|4E382ED9 |79 |1A89459C /1 |31 |
|A00FFADF |4 |1A894424 /1 |1 |
|BA068AF5 |3 |1A894424 /1 |1 |

All use generic numeric default wrapper caller0084CDD0 and have no lookup/refusal failure. The first owner has81 total accepted requests/78 completions across all its events; the second18 accepted/9 completions, largest observed count10. These are object-level lifetime aggregates, not proof of outstanding/leaked voices. Additional accepted IDs differ between the runs as expected from different models, siren behavior and background traffic.

These addresses were registered once in the working capture, making their requests useful candidates to identify. They are **not yet tagged as the player vehicle**. Other persistent objects also exist, and an audio object can belong to a shared handler. Across processes, addresses and even pool-slot reuse patterns cannot identify the same car. Most importantly, the cop-car siren can legitimately introduce event IDs absent from a civilian Escalade. Do not label any candidate engine/tire/horn/siren until its assignment or caller is traced.

## Next bounded investigation

The user supplied enough context for this comparison; more precise old timestamps cannot be reconstructed or required retroactively. No immediate additional random run or capacity increase is justified.

The next static task should identify these candidate event roles and trace **per-vehicle sound-object initialization and model-to-audio/event/bank assignments**, including police/siren-specific branches. The current wrapper's original caller often remains0084CDD0, one helper level above the real vehicle controller; tracing that caller chain/model context is needed to bind requests to the driven car. If the correct Escalade requests are present/admitted, continue to event actions, switch/parameter/media selection and voice/stream construction. If absent, follow the earlier initialization/selection gate instead. The pair does not yet choose between those causes.

No ASI or game behavior changed. This turn read only handoffs and the explicitly supplied reason logs, used analysis scripts, and saved reports. No build/install/game run/new executable disassembly/debugger/web search or other game-folder scan.

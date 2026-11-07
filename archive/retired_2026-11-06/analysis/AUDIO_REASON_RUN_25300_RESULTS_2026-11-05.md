# Audio reason run25300 — 2026-11-05

The reason probe installed correctly. During this roughly3m56s run the measured16-entry sound-object gate never refused a request, its largest directly observed and projected accepted counts were10, and every validated type1 completion reached record removal. This does not justify increasing16 or patching this cleanup path.

All55 normal numeric PostEvent zeros are now directly accounted for as absent event definitions at lookup time. Playing-record creation failures were0. Most of those misses concern the none hash or numeric0; two persistent unnamed IDs account for17 misses on one sound object and share a caller with successful Menu_Move posts. Their connection to the silent cars remains unproved. The next useful investigation is actual vehicle event/bank selection and downstream playback construction, with working/silent-car identity correlation.

## Input and user observations

Input: `dist/MAIN_150/DSFV29s_audio_reason_25300.log`,994397B, complete final newline, reason/audio status through236047ms. SHA256 and full summaries: `analysis/audio_reason_run_25300_2026-11-05/summary.json`.

The user confirmed that previously working cars still had full engine/tire/horn audio, specifically Ford Crown Victoria; visibility remained fine and FPS around100. Car entry times/order were not tracked. The reply did not supply an identified/timed silent-car sequence, so individual object addresses cannot be assigned to a silent vehicle from these observations. FPS/visibility are user reports, not measured by the audio hooks.

Reproducer: `python scripts/analyze_audio_reason_log.py dist/MAIN_150/DSFV29s_audio_reason_25300.log --output analysis/audio_reason_run_25300_2026-11-05`

## Validation and complete measurements

All startup/live baseline checks pass. Reason startup status1/hooks10/changedLimits0/objectEventLimit16, inherited global2048 and shared default67108864B allocations validated. Cumulative counters are monotonic, invalid0, ownerOverflow0 and eventOverflow0.29 sound-object addresses and170 event IDs appear in aggregate records; no original string-wrapper calls occurred, so the probe obtained no event-name strings.664 registrations demonstrate substantial address reuse, not664 simultaneous emitters or unique cars.

| New observation | Count |
|---|---:|
| Event lookups |1802 |
| Lookup hit /miss |1747 /55 |
| Selected playing-record creation failures |0 |
| Sound-object wrapper requests /returns |1735 /1735 |
| Wrapper accepted /zero |1684 /51 |
| Pre-PostEvent full16 refusals |0 |
| Maximum directly observed count /accepted projected count |10 /10 |
| Delivered callbacks /validated callbacks |1711 /1711 |
| Delivered type1 /handled type1 completions |1674 /1674 |
| Selected removal/decrement calls |1674 |
| Unmatched completion playing IDs |0 |

The final balances match exactly: lookups=hit+miss; wrapper returns=accepted+zero; wrapper requests=returns+full16. Owner-table totals also match the corresponding wrapper/callback/removal/registration counters. All51 wrapper zeros have lookup misses; another4 lookup misses occur on routes outside this object wrapper, accounting for all55 middleware zeros. There is no unexplained immediate failure counter or observed pre-PostEvent16 refusal in this capture.

All1674 type1 completions reaching the validated handler matched an entry and reached the original removal/decrement. This is evidence that this particular event-record cleanup works for observed completions, not that every streaming resource or downstream voice is managed correctly. Playback can complete without having produced audible output.

Accepted count1684 minus1674 removals is not, by itself, a proof of ten currently active/leaked records: observations cover distinct stages and reused object lifetimes. In particular, owner `lastCount` may be read immediately before the original decrement, and aggregates are independently sampled. Do not sum lastCount or convert registrations/accepted-event totals into simultaneously active voices/vehicles.

## The55 missing-definition failures

| Event ID | Misses | Positive lookups later/elsewhere | Context |
|---|---:|---:|---|
|2CA33BDB (`none`) |32 |0 | Eight object addresses, numeric default-argument wrapper caller0084CDD0. |
|00000000 |2 |0 | GameObjectFFFFFFFC; not an identified real sound definition. |
|38C61846 |9 |0 | Object1A8A2CA4, caller0072DFE5. |
|DFBD7040 |8 |0 | Same object and caller. |
|A6073B55 |1 |1 | Startup miss7797ms, default gameObjectFFFFFFFE. |
|2E385822 |1 |1 | Startup miss7813ms, object1A8A2F94. |
|12E6FA03 |1 |1 | Startup miss10422ms, default gameObjectFFFFFFFE. |
|72DCBB24 (`Menu_Move`) |1 |96 | Startup miss10500ms; later admitted menu moves. |

All55 lookup-miss details and51 wrapper-zero details were retained; none of these buffers truncated.32 none misses and23 other misses use separate budgets. Missing-lookup helper returns are00C54E03, consistent with numeric API posting. Creation failure, object-full and unmatched-completion detail counts are0.

The four singleton startup misses subsequently receive positive definition lookups: Menu_Move is already positively observed in the15125ms aggregate; A6073B55/2E385822 by25156ms and12E6FA03 by50266ms. Thus those four are not persistently missing throughout the run. These are first positive aggregate observations, not exact first-hit timestamps. Menu_Move's96 admitted events also show that its definition was available after its initial miss; admission alone does not prove the user heard every such effect.

The two recurring unnamed IDs remain absent for all observed requests, between46719..214922ms and90578..220672ms respectively. Both use0072DFE5 on1A8A2CA4. The same event-call site/object also posts Menu_Move successfully (its last recorded caller/owner context). This ties the failures to a shared caller that handles at least some menu/UI events; it does not establish that all sounds through it are UI, nor identify the two failures as engine/tire/horn events. Do not count those17 failures as a proven silent-car cause.

The32 none-hash requests can be optional/unset references or missing mandatory assignments; original strings were not supplied through the two instrumented string wrappers. A known hash association does not prove every runtime request literally came from that string. Vehicle-specific associations remain unresolved.

## Other limits and graphics baseline

Global A0/A1/B observed peaks26/51/53, all configured/allocated2048, no ceiling hits/reuse. Per-record eight-user attempts407, peak-before2, refusals0; byte255-user attempts18, peak-before2, refusals0. Native game emitter attempts664, peak-before28/90, refusals0. Traced bank completion results157/157 are actual AKRESULT1; successful loading of traced banks does not establish expected vehicle content/definitions.

Malloc/Malign failures0. Shared default pool sampled peak5460172/67105684B (8.137%), pressure0/1166; platform1168200/3142548B (37.174%). StaticBanksPool remains4239752/4369300B (97.035%) with no measured allocation failure; it is outside the demonstrated90% default/platform gate. High static occupancy alone does not identify a cause.

Indexed baseline has1174 initialized samples, empty samples0, pending peak1218 and final pending0. The audio bank-operation queue has peak2 and boundary attempts0. These observations support keeping the current visual settings; they do not prove150 simultaneous drawable bodies.

## Next target

Further global-slot,16-entry or shared-default growth is unsupported by this run. The measured16-entry gate and completion-record removal are not failing in the captured activity. Immediate lookup failures are fully classified; their strongest recurring non-placeholder IDs are on a caller that also handles Menu_Move, rather than being mapped to the missing vehicle sounds.

The next static trace should follow actual vehicle sound requests after successful event lookup/admission: selected per-vehicle event/bank content, switch/parameter state, event-action dispatch, voice/media/stream creation and any exact refusal or absent-child outcome. A focused runtime comparison should bind one known working car and one known silent car to those events/objects (with order/times), rather than adding another unverified capacity. The logs do not yet distinguish a wrong/missing per-vehicle assignment from a downstream playback/media failure. x32dbg remains optional; no debugger is required just to define those next measurement points.

This turn analyzed only the supplied log and updated the analyzer/report/handoffs. No ASI or game behavior changed; no build/install/game run/new executable disassembly/debugger/web search or other game-folder scan.

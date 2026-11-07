# Audio probe run 4188: measured admission failures, no measured capacity rejection

**Subsequent finding:** the user's requested follow-up identified a native90%
default-pool pressure gate that can stop/refuse playback before allocation
failure. This run satisfies that exact predicate in15 snapshots from40344ms
onward. Therefore this report's zero-failure counters do not exclude capacity-
driven playback loss. See `AUDIO_LIMITS_AND_DEFAULT16M_2026-11-05.md` for the
verified mechanism and staged16MiB comparison. The earlier analysis below is
retained as the evidence available before that discovery.

Input: `dist/MAIN_150/DSFV29s_audio_probe_4188.log`, 199960 bytes, 776 lines,
SHA256 `762B9CBC468A49514C42C35FBBF51CEED996A9683E096C1EB449E98E56B38D1E`.
The last monitor tick is 111860 ms. The user confirmed the same audio loss:
initial menu sounds work, disappear after save loading, vehicle sounds remain
missing; car bodies loaded normally. FPS was not supplied.

The probe is valid. All six audio hooks, three capacity hooks, stock preflight,
owned-handler verification, 13 indexed patches and 31 pending redirects report
success. Later rows show requests150/graphics160/indexed262144, startup status2,
valid heads and fileStatus0. Counters are monotonic. This is a measurement run,
not a startup refusal. No new source/ASI change, build, installation, executable
disassembly, game run, process attachment or web search followed this log.

## What the run establishes

| Measurement | Result | Meaning |
|---|---|---|
| Instrumented bank callbacks | 153/153 actual result1; all 153 details retained | No failed asynchronous bank result on this wrapper path |
| Bank-operation queue | Peak2/50; 153 inserts; zero boundary attempts | No observed queue boundary pressure |
| Native emitter gate | 443 attempts; largest active-before31; zero >=90 refusals | This measured 90-emitter limit did not reject an attempt |
| Malloc/Malign failure hooks | 0 / 0 | No observed nonzero-size null return on either hooked API path |
| Middleware pool descriptors | 7/20 throughout initialized samples | Descriptor maximum was not approached |
| Numeric PostEvent normal returns | 3244, including 167 zeros | 3077 returned nonzero playing IDs; 5.148% failed admission |
| Early external-source PostEvent return | 0 | No observed failure on that separate early exit |

The actual callback result is captured separately from the pool ID. All bank
details contain `result=00000001 pool=FFFFFFFF`; the latter is the signed -1
callback pool value, not a bank failure result. These results correct the old
trace's ambiguity, but cover only this instrumented bank-loading wrapper.

Nonzero PostEvent IDs prove admission only. They do not prove a voice started,
remained active, decoded, reached a bus, or produced audible samples. Thus 3077
admitted calls cannot be equated with 3077 audible sounds. The probe did not
retain successful event IDs, so it cannot say which admitted calls corresponded
to the inaudible menu/engine/horn categories.

## The concrete failure lead

The first two PostEvent zeros occur at 7610/7625 ms; two more at 10500/10563 ms,
before the first instrumented bank callback at 22688 ms. Zeros are therefore
not exclusively a new phenomenon appearing after the 150-bank loading period.
There is no explicit audible-loss/loading-complete timestamp in the log.

All 153 bank callbacks occur between 22688 and 48797 ms. By the 49406-ms status,
111 normal event posts include 7 zeros. At the end, 3244 include 167. After that
49406-ms sample, a further 3133 posts include 160 zeros and **2973 nonzero IDs**.
This supports investigating both failed event availability and what happens
after otherwise successful event admission.

The first 128 zero-result details are retained, through tick96750; 39 later
zeros are counted without individual details. Of those retained:

| Event ID (hex) | Native return address | Retained failures |
|---|---|---:|
| 2CA33BDB | 0084CCC9 | 74 |
| 80710A6C | 0084CCC9 | 12 |
| 80710A6D | 0084CCC9 | 11 |
| FB05593C | 0084CCC9 | 11 |
| FB05593D | 0084CCC9 | 11 |
| 38C61846 | 0084CCC9 | 3 |
| DFBD7040 | 0084CCC9 | 2 |
| 2E385822 | 0084CCC9 | 1 |
| 72DCBB24 | 0084CCC9 | 1 |
| A6073B55 | 00848422 | 1 |
| 12E6FA03 | 00848439 | 1 |

126/128 retained failures come through the shared object-event call at
0084CCC4 (return0084CCC9). Neither that wrapper nor the event hashes identify
the sound category by themselves. Repeated failures of 2CA33BDB are a focused
target, not proof that it is the engine or menu event. Counts in this table are
bounded samples, not the distribution of all 167 failures.

Existing exact-executable research (`AUDIO_STATIC_AUDIT_2026-11-05.md`, section4)
identifies missing event-definition return at 00C54012 and failed playing-record
allocation cleanup at 00C54052 in common helper00C53FB0. No hooked allocation
failure was observed here, so definition/registration availability is a stronger
lead than increasing the default pool. R1 does not distinguish these individual
return reasons; it cannot prove the missing-definition branch was taken or that
the zeros explain the broad silence.

## Memory and state during the transition

| Pool | ID | Sampled peak bytes / usable bytes | Utilization |
|---|---:|---:|---:|
| FilePackagesPool | 2 | 481448 / 553876 | 86.923% |
| PrepareEventPool | 3 | 0 / 304020 | 0% on this byte counter |
| StaticBanksPool | 4 | 4239752 / 4369300 | 97.035% |
| Shared default | 5 | 5361532 / 5868820 | 91.356% |
| Platform | 6 | 314840 / 3142548 | 10.019% |

StaticBanksPool has only 129548 sampled spare bytes; the shared default has
507288. These are narrow margins, particularly the bank pool, but no associated
Malloc/Malign failure or non-1 bank callback was recorded. Usage alone does not
justify a capacity patch. Fragmentation and transient/non-hooked allocation
failures remain outside what these sampled totals establish.

The fixed-block pool1 has four 262144-byte blocks and sampled minimum free2.
Its separate sampled peak Malloc byte counter is 524288. Neither counter showed
exhaustion; freeBlocks=0 on the non-fixed pools is not evidence of exhaustion.
Seven stable descriptor identities/generation1 were logged; there was one
readable sound-manager owner. Pool reuse entirely between samples is not ruled
out. CreatePool and fixed-block GetBlock failures are not hooked by R1.

Manager byte30, soundInit and musicInit remain1 after initialization. Four raw
values at A+50..5C remain float0.5. Values at A+60/+64/+68 start at1, are sampled
at0 at30266/35313 ms, rise to raw3F0AAAA8 (about0.542) at40344, and return to1 by
45375 ms, remaining1 in all later snapshots. Thus these sampled fade-like fields
do not remain stuck at0. Their recovery does not prove the actual mixer/bus gain
or output path recovered; their exact downstream role remains unproven.

## Rendering baseline and next action

The indexed baseline remains healthy under this test: zero empty samples across
554 initialized samples, pending peak930 and final0. This agrees with the user's
normal body-loading observation, without proving 150 simultaneous drawable cars.

Preserve this visual configuration and all current audio limits. The next bounded
static investigation should use the concrete failing event IDs/callers to follow
event-definition lookup/registration, then trace admitted events through command
consumption and voice/bus/output processing. Broad silence despite thousands of
admissions makes later processing a live possibility. Neither the event zeros
nor a downstream mixer fault is yet established as the root cause.

Do not repeat an unchanged R1 run or raise a limit on these results alone. A new
probe would be useful only after narrowing the static gap, for example direct
missing-definition versus playing-record-failure accounting, or downstream
command-consumer/voice evidence for a known inaudible event. No such build is
authorized or performed in this log-analysis turn.

Reproduce this analysis with the one focused analyzer command:

```powershell
python scripts/analyze_audio_probe_log.py dist/MAIN_150/DSFV29s_audio_probe_4188.log --output analysis/audio_probe_run_4188_2026-11-05/summary.json
```

Machine-readable evidence: `audio_probe_run_4188_2026-11-05/summary.json`.

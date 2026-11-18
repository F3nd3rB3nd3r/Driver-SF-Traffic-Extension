# R8 audio setup: static audit of the recovered failures

The remaining 450 false returns in run18736 are not evidence of another
exhausted audio pool. The existing counters exclude the vehicle-enable,
selected-bank readiness and model-profile lookup failures for this run.
The remaining native return paths test a shared default profile, preview
state, mode validity and vehicle sound-data state. Existing telemetry cannot
select the exact branch retrospectively. Keep R8 and the current capacities.

This audit reads the supported Save Bug Test `Driver.exe`, SHA256
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`, the current
audio probe source and the supplied R8 evidence. Microsoft dumpbin disassembles
the executable on disk; the game and a debugger were not launched. No source,
ASI, INI, asset, save, build, deployment, commit or push change was made.
Analysis artifacts and handoff notes are the only writes.

## Every branch into the common failure return

`00553860` takes a vehicle and a mode and returns a boolean. All seven direct
failure branches converge on `00553AD5`. The table at `00553ADC` is jump-table
data, not additional instructions. Addresses below refer to the unpatched EXE.

| Branch | Native reason | What run18736 establishes |
| --- | --- | --- |
| `0055386E` | `008CD230` reads vehicle byte `+5E7`; zero rejects setup. | Excluded: disabled0; all6128 entries reach the later bank check. |
| `0055391A` | `0048F780(selectedModel)` is false. It searches type3 bank records and requires state4 and a nonzero backing pointer. | Excluded: bankReady6128, bankNotReady0. |
| `00553927` | Selected per-model profile at controller `+BC` is null after optional model1 fallback. | Excluded by6128 nonzero lookup hits, misses0, fallback0. |
| `00553934` | Shared default profile at controller `+C0` is null. | Not classified: only early, bounded pointer details exist. |
| `00553960` | Manager at root `+25C` has byte `+4` set and controller mode is nonzero. | Explicit preview gate; no per-attempt preview/mode telemetry exists. |
| `00553994` | Unsigned mode exceeds3. | Valid modes are0..3; no per-attempt mode telemetry exists. |
| `005539CD` | Mode-specific initializer returns false. | Its false paths test vehicle state / sound-data availability, described below. |

The lookup-hit counter tests the actual returned profile pointer, not merely
the presence of a registry node. Thus zero misses meaningfully excludes the
null per-model profile branch here. These conclusions assume the installed
probe counters describe the native calls consistently; their totals reconcile.

## The preview flag has a concrete owner and purpose

The executable binding table contains:

| Binding row | Exported name | Callback | Native target |
| --- | --- | --- | --- |
| `00F0D228` | `EnterPreview` | `0063CAE0` | `00544AD0`: sets manager byte `+4` to1 |
| `00F0D230` | `ExitPreview` | `0063CB10` | `00544B00`: clears that byte |

Both callbacks obtain the same root `+25C` manager as the audio setup gate.
`00544540` also initializes its byte `+4` to zero. This is a state flag, not a
count or a byte-sized capacity. Increasing it would not add audio resources.
This establishes the flag's preview relationship; it does not establish that
preview was active at any particular failed attempt in the supplied session.

## Mode-specific initializers

The verified jump table maps mode0/2 to `005535C0`, mode1 to `00551EF0`, and
mode3 to `00551FA0`. Modes0,1,2 reject setup if:

- Vehicle dword `+544` is not3.
- `008CEE80(vehicle, snapshot)` returns false. That getter repeats the `+544`
  state check, then copies sound-related simulation data through `00873150`.
- Snapshot byte `+97` is nonzero. The snapshot is initialized by `00554650`;
  one producer path explicitly sets this byte when simulation field `+C20`
  is set (`00873185` through `008731AB`). The exact high-level enum/flag names
  are not established by this audit; do not label them pool exhaustion.

After those checks, these initializers return true. Mode3 returns true without
these snapshot checks. `00553390` does acquire/release a downstream sound
instance, but its boolean result is ignored at `00553986`; that result does
not cause the shared setup-failure return. Likewise, event setup calls within
the mode-specific initializers do not branch to false based on their results.

Consequently, neither a failure nor a success of this outer setup function is
a complete measurement of downstream event playback. Successful setup alone
does not prove audible engine, horn and tire sounds. This audit is scoped to
the observed false returns, not an exhaustive proof that all audio capacities
are covered for arbitrary future rosters.

## Shared default profile lifetime

Registry vtable `00DE3824` has setter `00556E00` at `+8` and getter `00556E10`
at `+14`. Both access one pointer at registry `+2EC`, outside the model list.
The native constructor clears it. `0048F970` recognizes `VSCH` data and
installs its pointer through the setter at `0048FA45`; its type2 unload path
sets that pointer to zero at `0048F942`/`0048F944`. The same loader recognizes
per-model `VSVH` records and registers them separately.

This distinguishes shared-profile lifetime from model-registry capacity.
A missing shared default remains a possible transient failure reason even
when every per-model lookup and bank-readiness check succeeds. No assertion
is made that it happened in run18736.

## Caller handling and retry opportunities

There are three validated direct setup calls and no raw function-pointer
references to its entry in this EXE:

| Call | Mode source | False-return handling |
| --- | --- | --- |
| `005552E3` | Constant3 | Clears vehicle assignment and resets mode to-1 at `005552EC`/`005552F3`. |
| `0055653C` | Selected policy mode; ordinary paths choose0..3, an explicit override comes from the manager list. | Same clear/reset at `00556545`/`0055654C`. |
| `005566DC` | Constant0 | Same clear/reset at `005566E5`/`005566E8`. |

The update path calls `005551E0` at `005563DE`: it walks current candidates,
keeps existing assignments and fills empty assignments. Failed assignments
therefore become eligible for selection/setup again if their vehicle remains
a candidate. This is reselection on later updates, not an unconditional retry
of the same vehicle on the next frame. There is no permanent failed-model
blacklist on these traced return paths. All43 failed model IDs subsequently
have successful setups in the supplied log, consistent with this behavior.

The owner has seven controller records of stride `2CC`, and its ordinary
policy selects mode1 for at most six qualifying records (`005564F2`), with
other modes for other roles. These are active-controller/quality-selection
budgets, not the45-entry resident audio model registry. A count of seven does
not mean only seven resident models may have sound. Exhausting the assignment
budget exits before calling setup; it cannot directly account for the450
counted setup false returns. Do not increase these budgets from this evidence.

## What the supplied log can and cannot answer

Run18736 totals reconcile exactly:6128 attempts,6128 model-profile hits,
6128 bankReady,5678 successes,450 failures; disabled/bankNotReady/misses and
fallback counters are all zero. Registry peak283/384, full0 and integrity
guards0 in the existing review. Every one of43 failed models later succeeds.
The user reported no missing audio, rendering or FPS issue. Most failure
increments follow traffic switches, but the previously identified clusters
around1067/1068s and1265/1266s occur within an ongoing280 phase; do not call
all failures traffic-switch-only.

The128 `bankCheck` and128 `setupResult` detailed records cover ticks47985
through112532ms. All128 results succeed and all sampled default pointers are
nonzero. The first failure increment is at406141ms, long after both detail
buffers stop collecting. Thus the absence of a detailed failure record is a
sampling limitation, not proof that a particular gate did or did not fire.
Run3600's351 successful setups provide a separate clean workload; they do not
classify the long run's failures or establish vanilla behavior.

The strict generic analyzer's `audio_setup=false` result remains unchanged.
Do not suppress or relabel those450 failures without their branch reasons.

## Recommended next step

No audio fix or capacity increase is supported by this audit. If exact
classification is desired, a later, separately authorized diagnostic build
should count each of the seven failure branches, with separate mode-initializer
state/snapshot subreasons. Capture failures throughout the session using
bounded per-reason records, rather than spending the entire detail budget on
early successes. Include caller, mode, model, controller/vehicle, `+544`,
snapshot `+97`, preview flag, and shared-default pointer at the point tested.
Do not re-read them only at the final return and assume they are unchanged.
Preserve native returns and capacity values. This would be a classification
probe, with no accompanying speculative limit increase.

Evidence and reproduction: `audio_setup_static_2026-11-17/audit.py` generates
the scoped disassembly, verified binding/jump-table evidence and streaming
log reconciliation. Run it from the workspace with `python
analysis/audio_setup_static_2026-11-17/audit.py`. It asserts the EXE and log
hashes and counter totals; it does not execute the game or change binaries.

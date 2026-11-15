# GENERIC_R1 run24080: previous-session replay loads and plays successfully

Input: `dist/MAIN_150/DSFV29s_generic_limits_24080.log`,9,695,802 bytes,
31,087 records. Identity and analyzer results are retained in
`analysis/generic_replay24080_2026-11-15/`.

User accessed the previous session's replay directly from the main menu in
a new process; its recording was made during run22828. Playback controls were
untouched. User does not recall choppy motion or a bursting loading indicator,
and confirms normal game exit. No Story/menu/Replay cycling was performed.

This is a successful **saved-replay loading/playback retest** for GENERIC_R1,
with initial live coverage of configured snapshot restore and reader reuse.
It is not eight separate user Replay entries and does not certify every
Story/Replay transition, seek operation or non-default INI configuration.
Keep the current ASI/default limits.

## Configuration and integrity

Same supported default AssetWeights configuration:150 requests,160 physical
model/graphics slots,192 audio-model entries,2048 global audio slots,64MiB
shared audio,1MiB reader,256 retained references. No compiled car list.

Generic, baseline, callback/inherited and replay reader/client analyzer
checks all pass. No fatal, invalid, guard, allocation, rejected-read, schema,
event-pair or event-drop failure is recorded. Log continues to about350.6s.
User confirmation establishes normal exit; absence of a fatal log does not.

Nine accepted150-ID request phases all reach150 READY. Initial asset request
expands18->150; eight later requests preserve the150-ID lists supplied by the
restore path (`reason=otherCaller`). They are not eight named-setting policy
expansions or evidence of manual seeking. READY checks remain sequential
native predicate sweeps, not proof of150 simultaneously visible vehicles.

## Replay restoration and reader use

- Eight import/restore pairs, each count150 and roster hash5CBDFFEA, with
  matching ID50=181 and extra000000D4.0 invalid/deferred events; final active
  temporary slots0. No new recording captures/saves in this process, consistent
  with watching the previous process's recording.
- Retained-reference count/peak is0. Traffic snapshot copying is exercised,
  but nonzero retained-reference array restoration is not covered here.
- One reader owner, five constructions, five destructions and four backing
  reuses. These are internal reader lifecycle operations, not proof of five
  manual Replay entries. Every managed boundary uses the configured1MiB ring.
-1,661 reads issued;1,658 commits,54,307,392 committed bytes across reader
  generations.0 rejected reads, fallbacks, faults or unmanaged calls.
  Final wrapper state is inactive. The1MiB backing is intentionally retained
  for process-lifetime reuse; this does not indicate outstanding active I/O.
- Native ready-completion service sees eight imports but0 candidates/attempts;
  its held-completion branch is not exercised. Registry reconstruction also
  remains unexercised (one construction/owner).

## Observed waits: small playback boundaries, not zero waiting

Client instrumentation records10 busy intervals: nine clear normally and one
ends with a context change; final open/held counts0. All observed begin
conditions are `missing_current`, without unreadable-node/pointer errors.

One1,589.756ms interval is during initial loading, at clock0, and terminates
as `contextEnd`. That marker is a context change, not a demonstrated readiness
recovery. A following20.035ms loading interval clears in the new context.

During playback (`loading=0`, requested speed1.0 at event boundaries), eight
intervals total232.610ms:

| Client | Begin tick ms | Duration ms | Recorded clock |
|---|---:|---:|---:|
|7|46453|19.118|286.1583|
|7|53953|30.188|287.6583|
|7|56859|21.416|290.4000|
|7|86828|78.482|302.7833|
|7|120094|30.125|325.8416|
|7|122969|25.424|328.5666|
|1|122984|6.889|328.5666|
|7|200484|20.969|373.0333|

The seven client7 playback boundaries coincide with snapshot import/restore
events. These polling intervals include scheduler gaps, not solely disk time
or frame-rendering time. Their clocks are raw recorded simulation timestamps,
not guaranteed Replay UI timecodes. No unresolved or sustained playback wait
is recorded; the user did not notice the previous bursting/choppy symptom.
Do not call the test wait-free or infer exact displayed FPS from these rows.

32,788 reader space stops and680 wrap stops are prefetch/backpressure counters,
not counts of visible hitches. Minimum sampled free space40 bytes in the ring
does not itself demonstrate insufficient capacity. No additional reader-size
increase is indicated by this run. Periodic clock samples are retained in
`playback_bands.json` as context, not a rendering-performance benchmark.

## Other exercised systems

- Audio:665 setups,665 successes,0 failures;150 model registrations/removals,
  no registry overflow/guard failure. Previous run22828's transient failures
  do not recur in this particular workload; their underlying reason remains
  unproven rather than retroactively resolved by this clean run.
- Parked: seven cache builds,150 candidates,7,224 successful picks,6,248 in
  the extended range.0 empty/invalid/guard failures.
- Pending: peak154, one redirect, final active0/recycled154; no integrity fault.
- Cleanup: peak51,754,18,986 insertions beyond stock capacity, one stock-boundary
  crossing; final current0, drain balance0, no full/guard/invalid/mismatch fault.
- Callback groups stay within stock64; beyond64 callback expansion and
  notification sidecar copies are not exercised.

## Scope and next action

Analysis only. No production source, INI, ASI, asset or save was modified;
no build, debugger, game launch or live process attachment. Existing analyzers
were sufficient for this supplied default configuration. Handoff notes and
analysis artifacts only were written.

Retain GENERIC_R1. Still-open validation includes same-process Story -> Replay
-> Story/menu lifecycle cycling and non-default INI storage with a matching
asset roster. Widening beyond the deliberate150 request/160 slot milestone
remains separate static/implementation work. Await a concrete next task; this
log does not justify a new probe, patch or automatic limit increase.

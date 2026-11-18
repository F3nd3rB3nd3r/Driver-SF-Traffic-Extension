# R7 run11636: successful280-car replay/Story lifecycle

Input: `dist/USERCAP_R7/DSFV29s_usercap_r7_11636.log`,19775190 bytes.
Identity, parsed evidence and reproducible review are saved in
`analysis/usercap_r7_run11636_2026-11-15/summary.json`, `generic.json` and
`summarize.py`. Last recorded tick587781ms (~9m47.8s).

The user reports main menu -> Replay (full watch, no timeline controls) ->
main menu -> Story (about one minute driving) -> Replay (watch without timeline
controls) -> main menu -> normal quit. They separately confirm both playbacks
were smooth, with no bursting loading indicator or missing audio/rendering.

## Result

All42 checks from the current generic-capacity analyzer pass, including
audio_setup and the R7 full-width capacity-query checks. No fatal or exception
record appears. This supports the R7 correction and the observed280-car
Story/menu/Replay lifecycle. It is a tested-workload result, not certification
of every activity/chapter, unlimited session duration or larger rosters.

Same configuration as run21688:280 requests,290 model slots,384 audio models,
4096 global audio entries,128MiB shared audio,2MiB reader,512 retained refs,
180000 extra hash entries and802 coupled handlers/queue entries. The capacity
query reports27 widened vehicle queries, three native fallbacks and metadata
byte255 while configured slots remain290.

Nine accepted phases all reach native READY:18,280,280,18,280,18,280,18,280.
The replay imports/restores retain280 entries and roster hash699365A7.
The full-width query and large-roster restore are exercised after lifecycle
changes; no compiled vehicle list or further capacity increase was needed.

## Replay reader and transition evidence

Two reader owners use the configured2097152-byte private backing. There are
10 constructions,10 destructions and eight buffer reuses. Both owners end in
state0;4194304 retained backing bytes are expected process-lifetime reuse
storage, not evidence of active I/O. Allocation failures, fallbacks, faults,
unmanaged calls and rejected reads are zero. The owner/boundary schema and
buffer validation pass across652 owner samples and22 managed boundaries.

Native reads actually occur: the two retained owners' final issue counts
are1751 and446. Reader polling/prefetch space, wrap, end and backend-busy
stops also occur; these counters alone do not demonstrate visible stutters.

Narrow client telemetry has11 busy intervals: nine clear, two contextEnd,
with zero open/held intervals at the final sample. The two roughly1.43s/1.25s
contextEnd intervals start with loading1 and replay clock0; contextEnd itself
is not a readiness recovery. The remaining intervals clear, with the largest
about119.9ms. Several align with logged replay imports/traffic restoration.
No dropped events, thread drops, unknown clients, read or clock failures,
schema errors or unmatched event pairs occur. There is no logged rapid series
establishing recurrence of the previous playback hitch, and the user confirms
smooth playback. Timings include scheduler/polling gaps and are not render FPS.

The older dedicated replay scripts contain fixed150/160/1MiB expectations.
This review reuses the client's interval/schema decoder and explicitly checks
its startup against recorded280/290 values; reader backing/boundary validation
uses the recorded2MiB. The original fixed150 startup result is preserved in
the summary for transparency. No production analyzer or ASI was changed.

## Audio and remaining capacities

Audio setup704/704 succeeds, with setupFail0, no missing registry lookups,
registry-full or guard failures. Registry peak282/384. The48 transient setup
failures in run21688 do not recur. Audible results are user confirmed.

Handler occupancy peaks551/802; queue peaks262/2/549/37 stay below802.
Five handler and scheduler generations, four destructions and one remaining
live generation are recorded at the last sample; no integrity failures.
The absence of a final atomic shutdown snapshot prevents stronger claims
about engine destruction after the last log row. Normal exit is user reported.

Hash extra lifetime issuance peaks54749/180000. Sampled private memory peaks
1390129152 bytes (~1.29GiB), working set1058893824 (~0.99GiB). No further
limit increase is indicated by this run. Parked, callbacks, pending, cleanup,
registration maps and replay snapshot checks all pass.

Together with21688's Story/activity/pursuit result, this supplies successful
live280-car coverage for the immediate R7 loading/lifecycle regression. Keep
R7 with the current configuration as the tested280-car candidate. Larger
rosters and longer/repeated workloads remain separately unverified.

Analysis/handoff updates only. No source/ASI/INI/asset/save changes, build,
game/debugger launch, deployment, commit or push were performed.

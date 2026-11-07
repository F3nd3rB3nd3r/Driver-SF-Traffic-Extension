# R2 runs 5644, 22624 and 23064: rendering improved, audio still fails

## User observations

The user reports that cars appeared to load properly in all three tests.
Audio remains largely absent: menu effects, engines, tire screech, horn and
potentially other sources. On clarification, **menu effects worked at the
initial menu and disappeared after loading the save**. Only a couple of Shift
view effects remained audible; music/radio, sirens and dialogue were not
reported working. FPS was not supplied for these three runs.

These are observations across the tested drives, not proof that 150 vehicle
bodies were simultaneously READY/drawable. The logs do not measure pixels or
audio playback directly.

## All three expansion installs succeeded

Each log reports revision 2, successful stock preflight and owned-handler hook
validation, `status=1`, all 13 operands installed, 31 pending-array redirects,
external base `04B10000`, and `capacityChanges=1`. Each then observes
`indexedStatus=2`, 262144 slots, 150 chapter requests and 160 graphics slots.
All three measurement hooks remain installed. No malformed head, file error,
boundary or fatal line was found.

| PID | Last printed tick from ASI start | Initialized 200 ms samples | Empty samples | Pending peak sampled | Pending final | Audio inserts | Audio queue peak |
|---|---:|---:|---:|---:|---:|---:|---:|
| 5644 | 240016 ms | 1194 | 0 | 1728 | 0 | 153 | 2/50 |
| 22624 | 110468 ms | 549 | 0 | 698 | 0 | 153 | 2/50 |
| 23064 | 64344 ms | 319 | 0 | 602 | 0 | 152 | 2/50 |

Totals: **2062 initialized samples, zero empty-head samples or transitions**.
The ticks measure time from ASI startup, not driving duration or exact world
entry. Two-hundred-millisecond polling can miss brief empty intervals.

All printed status rows after the 150 request in runs 5644 and 23064 have zero
pending references. In run 22624, the first such row has 289 pending; the other
83 rows have zero. Transient backlog between printed rows is represented only
by the sampled peak.

Each run also observes a free-head slot index above the original table range:
maximum printed positions 144646, 145908 and 138450 respectively. This confirms
actual use of addresses beyond the old inline range. **A free-head index is a
list position, not the occupied count**, and cannot be used to estimate required
capacity or total live registrations.

## Comparison with the native-capacity runs

Native run 16316 had 371 consecutive empty-head samples after its first null
head, pending peak 15827 and final pending 14546. The later failed-expansion
run 24376 remained native: 699/984 empty samples, pending peak 16009 and final
1771. R2 eliminated observed exhaustion in all three supplied runs and left no
final pending backlog, while the user reports that vehicle visibility improved.

This is strong practical evidence that the native indexed-reference capacity
contributed to the reported vehicle-body problem under these test conditions.
It supports preserving R2 as the working visual baseline. It does not establish
every late dependency or guarantee other routes/saves will never encounter
pressure. No further indexed-table increase is supported by these runs.

## What the audio measurements establish

All three audio-bank queues peaked at only 2/50 (both projected insertion peak
and sampled actual peak), with zero boundary attempts and final active count
zero. The other three measured traffic pools had zero observed inserts and
sampled occupancy, with the previously documented coverage limits.

The 152 versus 153 insertion total in the shortest run is not evidence of a
particular missing bank. No per-bank playback correlation exists here; both
153-insert runs still had the reported audio failure. A queue operation count
does not measure resident bank bytes, successful decoding, event acceptance,
voices, mixing or audible output.

`src/V29sAudioBankTrace.inc` was reviewed for existing coverage. That older
trace follows vehicle-bank submission/callback/file-open/lookup, and records
the callback result raw. It does not trace the common menu/music/vehicle event
playback and mixer path. Those older probes are not enabled in R2. Their older
accepted submissions/callbacks and state-4 lookups cannot establish successful
playback in these new runs.

The persistence of audio failure while the larger table stays available and
pending work drains shows that this indexed expansion **is not sufficient to
restore audio**. These logs provide no reason to expand the 50-node audio queue.

## Next audio target

Use the save-loading transition as the comparison boundary: initial menu audio
works, then nearly all categories disappear while a few Shift effects survive.
Investigate shared audio behavior covering menu/music and vehicle events.
Vehicle-bank queue length alone cannot explain the observed categories from
these data.

A focused static audit should identify the common middleware error/result and
event-posting paths, plus bank-resident/audio-memory allocation and voice/mixer
admission state. Follow the existing vehicle-bank wrapper `0084BEB0` into the
shared backend and locate menu/event routes; decode exact native result values
from the identified implementation before interpreting them. Memory/voice
limits, bank eviction/lifetime, event rejection and changed bus/mixer state are
**hypotheses**, not findings of this log analysis.

If a new live measurement is needed after resolving those paths, preserve R2's
150/160/ED/EF and indexed capacity. Capture errors and selected menu/vehicle
event results before/during/after loading, with bounded reporting and low-rate
state snapshots. Avoid reinstating the old high-frequency handle/hash traces
or changing audio limits without a failed stage. Engine/tire sound was already
absent in the quiet 45 control, so preserve the distinction between that earlier
defect and the broader suppression after loading at 150.

## Evidence and reproduction

Only the three explicitly supplied new logs were read. Individual parsed
summaries and `comparison.json` are in
`analysis/indexed_capacity_r2_runs_2026-11-05/`. The raw SHA256 values are recorded
there. Prior native-run figures come from the existing handoff/results, without
scanning old log folders.

```powershell
python scripts/compare_indexed_capacity_logs.py dist/MAIN_150/DSFV29s_indexed262144_5644.log dist/MAIN_150/DSFV29s_indexed262144_22624.log dist/MAIN_150/DSFV29s_indexed262144_23064.log --output analysis/indexed_capacity_r2_runs_2026-11-05
```

No ASI/game behavior changes, build, installation, live game/debugging session,
new executable disassembly, web search or dependency installation occurred.

# REPLAY_READ1M run10556: first live pass

## Result and user observations

The combined test passes its first supplied live run. The private 1 MiB
file-reader buffers were used, all telemetry/inherited checks pass, and
**client3 recorded zero busy results**. Run30212 with the 256 KiB ring had
seven ordinary client3 missing-next waits, including five at replay135–137s.
Run10556 has no ordinary-playback wait intervals for any observed client.

The user does not recall seeing the bursting loading indicator/choppy motion.
Their reported sequence was main menu -> Replay -> full replay viewing ->
main menu -> Story -> full replay viewing -> main menu -> normal quit.
Rendering/audio/FPS were not explicitly assessed in this reply. Do not
convert clean counters into a new audible/drawable/FPS confirmation.

This is strong evidence that larger read-ahead resolved the measured symptom
in this run. It is not a universal playback/whole-game certification or a
controlled identical-recording frame-time A/B measurement. Keep 1 MiB for
continued use/testing; this log gives no reason to increase it further or
change native readiness/refill order. No new ASI/source/build/game changes
were made for this analysis.

## Input and integrity

- `dist/MAIN_150/DSFV29s_replay_read1m_10556.log`.
- 24,917,107 bytes;79,708 lines;complete final newline.
- SHA256 `897c10c4fa3139bc4199cbd0dede48ca3b9d098fd3d9c3d105edf126174266a3`.
- Last tick959875ms, approximately16 minutes.
- READ, CLIENT, baseline150, callback and inherited structural checks all pass.
- READ_START status1,1048576-byte ring,32768-byte I/O blocks,8 targeted hooks.
- No allocation failures, fallbacks, faults, unmanaged calls or rejected reads.
- CLIENT dropped/threadDrops/unknown/readFailures/clockFailures all0;open0.

Evidence: `replay_read1m_run10556_2026-11-14/read.json`, `baseline.json`,
`callback.json` and `comparison.json`. The reproducible `summarize_run.py`
reads the supplied log and the already-analyzed run30212 comparison only.
Console outputs are retained separately.

## Playback coverage and comparison

| Measurement | CLIENT run30212,256 KiB | READ1M run10556 |
| --- | ---: | ---: |
| Ordinary client3 missing-next intervals | 7 | 0 |
| Late135–137s client3 intervals | 5,15–23ms | 0 |
| Observed ordinary-playback waits, clients1/2/3/7 | client3 waits present | 0 |

Two sampled mode2 playback windows are present:

- Owner3BEFECE0, tick41297–287625, replay clock0.9000–201.3833s;246 samples.
- Owner2DC74FB0, tick615828–954657, replay clock0.9167–233.5083s;338 samples.

Both windows contain10 one-second status samples in replay130–140s, so the
absence of the old2:15–2:17 cluster is not merely an unvisited replay range.
The first viewing reaches the same sampled endpoint201.3833s as run30212;
that alone does not prove identical recording-file bytes. Mode2 windows can
include paused/start/end states; this sampling is not per-frame timing or
an independent normal1x speed measurement. Periodic OWNER rows can lag the
status by a sample, so their generation hints are not extra playback windows.

All12 CLIENT boundary records belong to six loading-transition waits:
four client7 starts and two client1 starts. Their begin states have loading1,
transition2/3 and requested rate0. Two are contextEnd events, not recoveries;
four clear. Client2 and client3 have zero busy results. The final CLIENT status
reports490340 fast-clear observations and no unfinished interval. These
figures describe the instrumented readiness implementations, not every
possible cause of visible frame stutter.

## Backing reuse and reader counters

READ final: two distinct owners,10 managed constructions,10 destructions,
8 reuses and3 unchanged writer constructions. Both owners reach generation5
and finish state0 (closed). Their private bases remain distinct and aligned:
3D220000 and3B3F0000; original native capacity was262144 in both cases.
Only2 MiB of payload backing is retained, matching two reusable buffers;
it did not allocate another MiB on each of the10 constructions. This is
expected process-lifetime retention, not evidence of active I/O or a leak.

Aggregate cumulative owner counters:

| Counter | Total |
| --- | ---: |
| Reader polls / backend-busy results | 73029 / 122 |
| Commits / committed bytes | 4656 / 152531384 |
| Issues / issue-byte bookkeeping | 4662 / 152727992 |
| Wrap checks / wrap-space stops | 345 / 199 |
| Ordinary free-space checks / stops | 59008 / 54346 |
| End checks / stops | 72708 / 13700 |
| Rejected native reads | 0 |

The fifth generations' minimum checked free space reaches8 and36 bytes;
maximum checked wrap-assembly need is65536 bytes. **These stops are not
playback failures.** The native prefetcher intentionally fills available
space, then waits for consumption, and end stops are expected after data
ends. Despite this normal backpressure, no client3 successor starvation is
observed. The minimum free count is not a request to enlarge the ring again.

Issue/commit counts are bookkeeping at different native sites, across
reposition/teardown generations. Their difference is not a proof of lost
reads; close can drain an issued operation without committing it into the
ordinary playback parser afterward. No direct OS completion-latency or
total probe-overhead measurement was made.

## Inherited behavior

- Six accepted150-ID roster phases;five reach an all-ready sequential sweep.
  One short phase ends at145 observed READY before replacement; no new
  failure or missing-model diagnosis follows from that incomplete phase.
- Snapshot52 captures/saves and2 imports/restores;150 traffic maximum,
  invalid0,activeTemps0;final simulation/replay pointers null.
- Both paused loading completions succeed after native-held retries:
  13 attempts,11 held,2 completed,no skip/identity/reentrancy errors.
- Parked5333 successful picks,5010 beyond the old inline range;
  empty/invalid/guard errors0.
- Audio450 stores/removes,937 setups,all937 successful;no full/miss/fallback
  or setup failure in the reported totals. This is structural evidence.
- Definition maps:3 owners/constructions,450 checks,external peak105;
  faults/allocation failures0. This does not re-exercise the older fifth-owner
  or same-address definition-map cases, which have prior R2 evidence.
- Cleanup384632 appended/drained,91809 balanced walks,current/depth/errors0.
  Pending462 stores,peak154,73 balanced walks,current/depth/errors0.

## Next action

Retain READ1M_R1 as the successful replay-test candidate. No further capacity
increase, readiness bypass, scheduling patch or new probe is justified by
this result. Longer ordinary play and additional saved recordings can extend
confidence. The user did not explicitly report a rewind/seek exercise in this
run; do not mark all playback-control paths or a full campaign verified.
Await a concrete next request or additional log before changing implementation.

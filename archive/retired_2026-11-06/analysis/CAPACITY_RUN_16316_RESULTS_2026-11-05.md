# Capacity run 16316 — 2026-11-05

## Result

The native indexed-handle free list becomes empty and remains empty in all subsequent recorded 200 ms samples, while thousands of resource references remain pending. This is direct native-metadata evidence, stronger than the allocation/free ledger used in earlier runs. The measured 50-node audio queue peaks at only two entries; expanding it is not justified by this run.

This establishes an available-slot bottleneck for native reference reconciliation. It does not identify one missing body dependency or prove that the table causes the audio failure. Slot retention errors and insufficient capacity remain separate possibilities.

## Input and observations

- Log: `dist/MAIN_150/DSFV29s_capacity_16316.log`, 52164 bytes.
- SHA256: `F7D0BF4CC96F0FE4E99195A3027B92C8287EB11D55A9EA434177C17C0740890D`.
- User reports no music, engine/tire/horn sounds or menu effects.
- On clarification, some vehicle bodies eventually appeared while others remained invisible. Driving FPS stayed around 100.
- No exact world-entry, body-reveal or audio-failure tick was supplied.
- Only this capacity log was analyzed; no old logs, game folders or processes were scanned.

## Validation

One session header validates `hooks=3 expectedHooks=3`, the quiet ordered-150 baseline, 160 slots and ED/EF, with no capacity changes. There are 106 printed status summaries through tick 105484 (105.484 seconds after diagnostic initialization).

At tick 1063, startup status is 2 and graphics slots are 160; a valid nonzero free head has been observed and initialization qualification is 1. At tick 26172, `requestLast=150` and `chapterHits=1` validate the expanded chapter request. These fields do not establish 150 READY/drawable vehicles.

Every initialized status has `headValid=1`; all file statuses are 0. No boundary line appears. The log ending does not by itself establish a crash.

## Queue measurements

| Queue | Observed insertions | Projected peak after insertion | Sampled active peak | Boundary attempts |
|---|---:|---:|---:|---:|
| Audio banks | 153 | 2 | 2 | 0 |
| Retiring IDs | 0 | 0 | 0 | 0 |
| Retained vehicles | 0 | 0 | 0 | 0 |
| Vehicle operations | 0 | 0 | 0 | 0 |

All four native capacities are 50. All invalid-count and owner-change counters are zero. Audio has 21 valid summaries; each traffic pool has 18. The final audio state is active 0, with two recycled nodes.

The audio peak is captured at insertion, so this is stronger than sampling an empty queue after a brief burst. The 153 insertions are calls, not necessarily unique banks. Queue retirement does not establish successful asynchronous bank results or audible playback.

Traffic zeros apply to the covered insertion paths and sampled headers. They do not test every hypothetical alternate writer or a large retirement/mission-operation burst. These pools are not compelling capacity-change targets for the reported session.

## Indexed-handle timeline

| Tick (ms) | Native free head | Pending references | Observation |
|---:|---|---:|---|
| 1063 | `018B0C8C` | 0 | Initialized table with available slots |
| 26172 | `0193F2C4` | 0 | Expanded request recorded |
| 30188 | `0199C8A4` | 0 | Last printed nonempty head before exhaustion |
| 31188 | `00000000` | 2256 | First null-head sample; one empty transition |
| 34203 | `00000000` | 13732 | Backlog has accumulated |
| 99453–104484 | `00000000` | 15353 | Same pending count in six consecutive printed rows |
| 105484 | `00000000` | 14546 | Final status; substantial backlog remains |

Highest sampled pending count: **15827**. Final initialized sample count: **524**; empty-head samples: **371**; empty transitions: **1**; sampled empty span: **74296 ms**.

The cumulative counters imply all 371 samples from tick 31188 through 105484 saw a null head, with no sampled recovery. Short-lived slot returns/reallocation between polls remain possible. Pending counts vary; the engine is not proved completely deadlocked. Cancellation/removal and brief reuse can change the count despite sampled empty heads.

The static audit established that `00BAE790` reads native free head `R+0x27E094`; the null-head branch at `00BAE841` leaves references pending. This run observes that condition alongside the backlog. It does not capture each branch execution or the dependency keys.

Keep three structures distinct: the native **131072 indexed slots**, the expanded pending pointer/pair arrays, and the resource hash arena. A peak of 15827 pending references is not indexed-slot occupancy or a reason to enlarge the pending collection again.

## Next priority

Audit indexed-slot retention and all relocation/initialization/addressing/bounds sites before an isolated indexed-table expansion test. Determine whether the working set needs more slots or incorrect retention consumes them; extra capacity could merely delay a retention fault.

The graphics indexed table has not been shown to be used directly by Wwise. Audio event playback, routing, global-bank initialization and decoded completion results are outside this trace. A table expansion must not be promised as an audio fix.

## Reproduction and actions

Parsed evidence: `capacity_trace_2026-11-05/run_16316_summary.json`.

```powershell
python scripts/analyze_capacity_log.py dist/MAIN_150/DSFV29s_capacity_16316.log --output analysis/capacity_trace_2026-11-05/run_16316_summary.json
```

This task added the analyzer/results and updated the handoff. No ASI source, game behavior or capacity was changed. No build, installation, game run or debugger attachment was performed.

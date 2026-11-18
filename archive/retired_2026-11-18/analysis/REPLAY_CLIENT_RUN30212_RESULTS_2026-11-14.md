# REPLAY_CLIENT run30212: client3 following-sample waits

## Finding and user observation

The narrow probe identifies client3, the two-position track with a native
0.25-second recording timer, as the client returning busy during the reported
normal-playback burst. Its current sample exists but its following sample is
absent. Five waits span replay time135.2557 through136.9470 seconds; each
clears when a following sample becomes available. This matches the user's
approximately2:15 symptom. The user confirms the indicator cleared around2:17,
did not recur, playback continued normally to the end, and the game quit normally.

This establishes the missing-following-sample readiness condition. It does
not yet establish why the reader/delivery path supplied that sample late.

## Input and verification

- Input: `dist/MAIN_150/DSFV29s_replay_client_30212.log`.
- 14,466,840 bytes,47,283 lines, complete final newline; last tick565453ms.
- SHA256 `3ce160af071cfbc49a769d093728d6dc60017de82cc1b08f07bb880b87e40954`.
- Baseline150, callback and narrow-client analyzers all pass.
- CLIENT_START: status1, hooks3, clients4, broadHooks0, requests150/slots160.
- Final: dropped/threadDrops/unknown/readFailures/clockFailures all0;
  open0, held0,10 begins,9 clears,1 startup contextEnd.

Machine-readable results and a reproducible comparison script are in
`replay_client_run30212_2026-11-14/`: `client.json`, `baseline.json`,
`callback.json`, `comparison.json`, and `summarize_run.py`.

## Normal-playback boundaries

All seven ordinary-playback waits are client3 with current valid/next absent.
Two short earlier waits occur near45.69 and45.96 seconds. Other observed
client1/7 waits belong to initial loading, not the reported late episode.
The late pairs are:

| Replay clock at busy (s) | Wait (ms) | Ring free at busy (bytes) | Unparsed (bytes) | Following timestamp after clear (s) |
| --- | ---: | ---: | ---: | ---: |
| 135.2557 | 15.0911 | 107384 | 496 | 135.5000 |
| 135.5140 | 17.3144 | 168800 | 8 | 135.7583 |
| 135.7722 | 22.7861 | 118424 | 8 | 136.0167 |
| 136.0305 | 19.4948 | 172516 | 29376 | 136.1583 |
| 136.9386 | 16.1070 | 145364 | 208 | 137.1750 |

For all five pairs:

- Backend1 is the native file wrapper; mode0 is its reader path.
- Ring capacity is262144 bytes (256KiB). Free space is approximately105–168KiB,
  more than one32KiB I/O block. This is not a full-ring snapshot.
- Current payload is24 bytes/type3. The current timestamp has been passed;
  the required following event is null until clear.
- Raw in-flight byte bookkeeping is0 at busy,32768 after clear. End flag is0.
- File-position bookkeeping advances65536 or98304 bytes between boundaries.
  These fields do not measure disk-read latency or exact completed-byte counts.
- Phase/transition/loading are0 and requested rate is1. The native loading
  field is separate from the user-visible loading indicator.

No checkpoint import/restore occurs during this ordinary-playback episode.
The only logged import/restore is at initial loading. Client identity is now
measured directly; cadence alone is no longer the basis for selecting client3.

## Reader mechanism and next static target

Existing disassembly in `replay_subscriber_static_2026-11-14/` shows reader
`00746940` committing completed input, assembling complete16-byte-header plus
variable-payload records, inserting them through `00746400`, and then issuing
at most one32KiB block per invocation. Wrapped incomplete records can require
additional contiguous assembly space. Scheduling checks free space at
`00746B62`; its ordinary read call is at `00746BD5`.

The updater disassembly in `replay_hitch_static_2026-11-14/` shows playback
client-chain processing before its tail dispatch to recording service
`00746CA0` at `0073A714`. These are concrete places to audit lookahead,
refill cadence, whole-record assembly and retained-buffer policy. The current
capture does not identify the blocking packet type or prove that reordering
or issuing additional reads would be safe or sufficient.

The busy snapshots argue against a simple exhausted ring at those instants.
They do not exclude earlier retention/capacity pressure indirectly reducing
lookahead. Nor do they prove slow storage. A larger read-ahead buffer remains
a possible controlled experiment only after backing ownership is addressed:
native providers return neighboring fixed partitions, so increasing just the
advertised capacity can overlap memory.

Do not raise audio/model capacities, change the0.25-second sampler, or bypass
client3 readiness on this evidence. A focused static fix-design audit can now
start at the known file-reader path; another broad identity probe is unnecessary.
Any final runtime fix still needs a normal-speed live validation.

## Inherited checks and observer limitations

Four accepted roster phases and three all-ready sequential sweeps are recorded.
Audio stores/removes balance300/300;726 setups succeed with no reported
miss/full/failure. Parked selection records3802 successes, including3211 beyond
the old50-entry boundary, without empty/invalid/guard errors. Pending and cleanup
queues drain to0 with balanced walks and no errors. Map checks report no faults
or allocation failures; this run does not exercise the earlier fifth-owner or
same-address reconstruction cases anew.

The narrow probe captures20 boundary records and performs48 readability checks.
Their measured snapshot-body elapsed time totals0.3887ms. That is only a partial
overhead measure, not total probe CPU/frame overhead. Reproduction in the narrow
probe means the earlier broad observer's overhead is not sufficient to explain
the existence of the symptom, but probe influence is not excluded entirely.

No runtime source, ASI, capacities, request counts or game files were changed;
no build, game run or debugger launch was performed for this analysis.

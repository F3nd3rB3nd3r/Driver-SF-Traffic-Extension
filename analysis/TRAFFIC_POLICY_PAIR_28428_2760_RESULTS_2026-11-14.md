# Native28428 / Preset1502760: first intended-pair live pass

2026-11-14. Both supplied logs analyzed with policy, baseline150, callback/
arena and READ1M analyzers. All checks pass for both runs. User confirms Native
28428 used original san_franciscoAI.fchunk and both sessions exited normally.
They report everything seemed fine; exact FPS was not separately supplied.
No source, ASI, INI or game asset changes; no build/game/debugger execution.

## Evidence identity

| Run | Mode | Log bytes / lines | Last sampled time |
|---|---|---:|---:|
|28428|Native|2458243 /10498|182938 ms, about3m03s|
|2760|Preset150|8950482 /29802|353891 ms, about5m54s|

Logs: `dist/MAIN_150/DSFV29s_traffic_policy_<PID>.log`.

- 28428 SHA256: `fa1e91dd0ef4c15e0e62377efca1141af70725053d23bd8aef198965f03db3e9`.
- 2760 SHA256: `24b16200573d3252af100331acbf0706d4559c4f2a57bdb9626cf1cf70005567`.

Evidence directory: `analysis/traffic_policy_pair_28428_2760_2026-11-14/`.
Four analyzer JSON outputs per run, console captures and a comparative
`summary.json`. File identity for Native is user-confirmed; Native mode does
not capture the loaded AI fingerprint. Successful Preset150 expansion requires
the compiled matching asset/profile checks, which passed in2760.

## Native28428: original rosters preserved

- ModeNative, explicitly configured, assetCheckedOnRequest0. Two named-setting
 requests preserve18 IDs in their original order, with no preset substitution
 or rejected request.
- The first short phase is superseded after about2 seconds with17/18 READY.
 The next18-model phase reaches all READY at tick26969 and retains an all-ready
 recorded count. The early phase did not remain active long enough to certify
 completion; its17/18 is not evidence of a persistent missing-model problem.
- All275 audio setup calls succeed, bank-not-ready0, registry misses/full0,
 registry peak27. Parked cache contains18 candidates;120 picks, empty/invalid/
 guard/concurrency errors0.
- Definition registry remains in its native capacity; pending peak31, cleanup
 peak17826, both final queues empty with balanced walk/drain counters.

This validates original-asset request pass-through in this session. Unlike
the earlier Native15256 run, the user restored original probability data,
so that prior modified-weight/preload mismatch is not this acceptance case.

## Preset1502760: checked expansion and extended structures exercised

- ModePreset150, explicitly configured, assetCheckedOnRequest1. Both eligible
 requests expand18 to the exact canonical150 IDs. Setting hashes AE88E83B/f0
 then47EB4D0E/f0 pass their asset/source checks. No refusal or rejected request.
- The first phase is superseded after about8 seconds with144/150 READY. The
 second150-model phase records all150 READY at tick32047, after acceptance at
31985. Much of this roster was already requested by the first phase. This is
 not a measurement of cold-loading all150 models in62 ms.
- All626 audio setup calls succeed, bank-not-ready0, registry misses/full0.
 Per-model audio registry peak150;105 insertions exceed the old45-entry gate
 without exhausting the192-entry replacement. Definition-map private nodes
 reach105, faults/allocation failures0.
- Parked cache150 candidates, largest category21;2072 picks, of which2060 use
 indices beyond the old50-entry prefix. Invalid/empty/guard/concurrency errors0.
13 unique parked candidates were selected; this does not mean every category
 or all150 parked models were exercised.
- Pending map peak154, using private extension beyond native64. Cleanup peak
51668 exceeds native32768, with18900 beyond-stock append observations. Both
 enlarged structures stay within their configured capacities and end sampled
 empty; walks/drains balance with no integrity/full/guard errors.

The extension usage is direct evidence that the retained audio, definition,
parked, pending and cleanup changes remain useful in this workload. It does
not establish a universal per-car scaling formula or justify raising limits.

## Shared integrity and scope

No fatal/exception/warning/rejected-request records found. Baseline event drops,
lock skips, obsolete checks, memory-snapshot failures and unmapped body calls0.
Callback/arena checks pass; callback private nodes beyond64 are not exercised
by either run. User reports normal exits; periodic telemetry is not a final
OS-exit snapshot, but final recorded pending/cleanup gauges are clean.

Both runs record/save replay snapshots, but replay loads/imports/restores and
file-reader owners/constructions remain0. Definition-map reconstruction/
reattachment is also unexercised. Thus these runs do not newly verify replay
playback smoothness, replay/menu cycling, repeated challenges or owner reuse.
Old parent evidence remains relevant but is not replaced by a false claim
that those transitions occurred here.

READY observations are sequential checks, not atomic150-model drawable/GPU
residency snapshots. User-visible/audible success applies to what they tried,
not a documented exhaustive check of every150 models or a full campaign.

## Conclusion

Keep TRAFFIC_POLICY_R1 as the successful first paired live candidate: original
AI +Native and matching modified AI +Preset150 behave as intended, with normal
exits reported. No new patch, capacity increase or probe is warranted from
these logs. Continued Story/Challenge/menu/Replay transition use can broaden
coverage; analyze new supplied logs before proposing a change.

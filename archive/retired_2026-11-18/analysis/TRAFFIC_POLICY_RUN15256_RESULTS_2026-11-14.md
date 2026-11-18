# Native run15256: modified AI probability/preload mismatch

2026-11-14. User reports most civilian traffic visually resembles fallback
Tanner Challengers, with a few other working vehicles. They believe the
modified AI remains installed and explicitly authorized checking the exact
Save Bug Test/media/san_franciscoAI.fchunk. No source, ASI, INI or game asset
changes, build, game launch or debugger use for this analysis.

## File inspection

The file is4403200 bytes, SHA256
`434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13`:
the exact modified uniform150 asset used to generate the compiled preset.
Inspected its actual Chapter7 setting at frequency0:

- Native preload list:18 IDs.
- Applicable probability tables:16, each with the same150 positive-weight
  models and total weight150.
- 132 of those weighted models are absent from the18-ID preload list.

These are separate structures in the same file. Native mode preserves its
native request list; it does not restore original probabilities or derive
requests from positive weights. The optional Preset150 adapter supplies the
larger request list for the matching modified asset.

## Run evidence

`dist/MAIN_150/DSFV29s_traffic_policy_15256.log`:2720435 bytes; SHA256
`001e924b5a878e16615e9ddda480a195c2f56f3cb1436a2eda7858d3cc4e53d5`.
Last sampled tick264547, about4 minutes25 seconds. Policy, baseline,
callback/arena and READ1M structural checks pass.

- ModeNative, reasonconfigured, no preset substitution. Two named-setting
  requests at ticks26156 and28859, both18 original/effective IDs in the same
  order. No rejected requests. The second roster exactly matches the known
  Chapter7 native list; Native events intentionally leave hash/frequency
  uninspected, so the event hash0 is not a broken setting hash.
- Both18-model phases reached all READY in a recorded sequential sweep.
  This says nothing about the other132 weighted models or simultaneous
  drawable residency. Visible traffic is still wrong according to the user.
- Audio setup88/88 succeeded, failures0; registry peak25, no reported model
  registry overflow/guard error. Other cars can have resources loaded by
  player/mission/global paths, so a few working models do not establish that
  the full weighted roster was requested.
- Callback/arena integrity passes. Final pending/cleanup queues are empty,
  with no recorded full/guard/invalid errors. Definition-map faults and
  allocation failures0.
- Replay recording occurred, but reader owners/constructions0 and no replay
  loads/imports/restores. This run does not validate replay playback or the
  original smoothness/lifetime issue. Normal process exit was not established
  merely from the log ending.

The old chapter comparison reports all_recorded_requests_expanded=false.
That is expected in Native mode; the policy-aware analyzer passes preservation.

## Interpretation and action

The installed file's150 weighted models versus18 requested models strongly
explains the reported widespread fallback. The log demonstrates the intended
Native pass-through behavior and no sampled capacity error. It does not
record the exact native chooser/fallback branch for each visually observed
car, so do not claim that branch has now been directly traced.

Use RosterMode=Preset150 with this unchanged modified file and restart. Use
Native with the original AI to validate original traffic, or with a modified
asset whose own named preload lists cover its weighted candidates. Native
plus this particular probability-only widening is not a valid150-car test.
No additional pool increase or new ASI is supported by the supplied evidence.

For a future generic solution, either correct the asset's preload lists or
explicitly add an asset-derived preload adapter that preserves required
native IDs and adds eligible weighted models. That remains separate work;
the current Native option intentionally does not implement it.

Evidence: `analysis/traffic_policy_run15256_2026-11-14/` contains policy,
baseline, callback and READ1M JSON and summary. User file inspection is a
current on-disk check, not an in-process asset fingerprint for run15256.

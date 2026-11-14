# Analysis index

## Current conclusions

- [READ1M run10556](REPLAY_READ1M_RUN10556_RESULTS_2026-11-14.md): first
  live pass; private1 MiB buffers active and client3 busy0 across two full
  viewings covering the earlier problem range. User does not recall bursts
  and reports normal exit. Ten constructions/destructions balance with
  eight reuses; all integrity checks pass. Keep1 MiB; no new patch/build.

- [REPLAY_READ1M_R1 build](REPLAY_READ1M_R1_BUILD_2026-11-14.md): combined
  1 MiB file-reader backing/capacity and targeted stop/progress counters,
  with narrow CLIENT boundary counter views. Native/ABI/guard/ownership/
  parser/compiled/inherited checks pass; staged only, live log pending.
  Native 32 KiB transfers, format, gates/order and traffic fixes retained.

- [Replay refill static audit](REPLAY_REFILL_STATIC_2026-11-14.md): shared
  256 KiB ring retention can limit lookahead before space is released.
  Adjacent captured sample-layout spans exceed capacity; not direct proof
  of a past space stop. Next requested experiment: 1 MiB reader-only owned
  backing/capacity plus bounded stop/progress counters, retaining native
  32 KiB transfers and gates/order. No new ASI/source/build; checks pass.

- [REPLAY_CLIENT run30212](REPLAY_CLIENT_RUN30212_RESULTS_2026-11-14.md):
  client3 current-valid/next-absent causes five15–23ms waits at135–137s,
  matching the user's brief indicator episode; no recurrence and normal quit.
  File-reader ring still has105–168KiB free. Audit lookahead/refill/record
  assembly next; no simple full-pool or slow-disk finding. All checks pass.

- [REPLAY_CLIENT_R1 build](REPLAY_CLIENT_R1_BUILD_2026-11-14.md): narrow
  client/sample/ring probe staged and native/compiled/inherited verified;
  broad HITCH hooks removed from this build. Normal clear path bypasses C++/
  QPC/readability/FXSAVE. New live identity capture pending; all limits retained.

- [Replay subscriber static trace](REPLAY_SUBSCRIBER_STATIC_2026-11-14.md):
  four of11 native clients can produce +20 busy, with recorded-data/horizon
  conditions. One has0.25s sampling; identity still needs narrow live data.
  Fixed replay-buffer partitions make a size-only increase unsafe. No build.

- [REPLAY_HITCH run28136](REPLAY_HITCH_RUN28136_RESULTS_2026-11-14.md): late
  +20 subscriber-readiness intervals recur at~250ms while clock gate+24
  stays clear. No restore/request during first viewing. Native caller can
  withhold service updates yet advance replay time. Subscriber identity is
  the next target; broad probe overhead is significant. No new ASI.

- [REPLAY_HITCH_R1 build](REPLAY_HITCH_R1_BUILD_2026-11-14.md): user-authorized
  passive timing probe staged; native bridge/installer/observer/parser and
  compiled/inherited R2 checks pass. Normal playback timing capture pending;
  no capacities or readiness decisions changed.
- [Replay hitch static trace](REPLAY_HITCH_STATIC_2026-11-14.md): distinct
  clock-advance gates, snapshot phases, recording reposition and simulation
  rate paths identified; the actual live blocker remains unknown.

- [R2 run24888](REGMAP_REUSE_RUN24888_RESULTS_2026-11-14.md): both old map
  failure paths exercised live,150 roster and repeated transitions/normal exit
  pass. User reports loading-related replay-motion hitches at steady render
  FPS; short-wait timing remains open. No new ASI for this analysis.

- [Dump21280 Forza roster mismatch](REGMAP_REUSE_DUMP21280_FORZA_MISMATCH_2026-11-14.md):
  active SS Test AI differs from vanilla fingerprints; none of its weighted
  IDs is registered, so native lookup falls back to key15. No map exhaustion;
  next comparison is R2 in the vanilla Save Bug Test installation.

- [REGMAP_REUSE_R2 startup fix](REGMAP_REUSE_R2_STARTUP_FIX_2026-11-14.md):
  R1 refused startup; reproduced with actual Readable/VirtualQuery and RX
  pages. R2 handles readable protection-region boundaries without weakening
  byte checks. Native/compiled/inherited checks pass; live startup pending.

- [REGMAP_REUSE_R1 build](REGMAP_REUSE_R1_BUILD_2026-11-12.md): dynamic
  stable definition-map owner records and explicit constructor generations;
  eight owners/100 reconstructions and inherited checks pass. Staged;
  repeated Story/Replay/menu live transitions pending.
- [Definition-map static audit](DEFINITION_MAP_STATIC_2026-11-12.md): native
  create/destroy and linked-node consumers; two independent ASI lifetime defects.
- [REPLAY_PAUSE run19868](REPLAY_PAUSE_RUN19868_RESULTS_2026-11-12.md): two
  live paused completions, then explicit older vehicleDefinitionMap exit0x31.

- [REPLAY_PAUSE_R1 build](REPLAY_PAUSE_R1_BUILD_2026-11-11.md): guarded native
  loading completion while paused, one game-thread updater hook and import
  observer. Fixture/compiled/inherited checks pass; staged for live testing.

- [REPLAY150 run8944 softlock](REPLAY150_RUN8944_SOFTLOCK_2026-11-07.md): intact150-ID
  snapshot and ready native gates, but zero-rate simulation clock skips the
  callback that clears replay loading. Initial pause caller/order unproven;
  target pause-safe completion servicing, with no new ASI built in this analysis.

- [REPLAY150 run10520](REPLAY150_RUN10520_RESULTS_2026-11-07.md): first live replay
  pass;84 saves/52 restores preserved all150 IDs and181, no replay integrity
  errors, user-confirmed normal exit. Repeated Story/Replay/menu cycles pending.

- [REPLAY150_R1 build](REPLAY150_R1_BUILD_2026-11-07.md): complete traffic snapshot
  field-overlap correction built/verified/staged; fresh Story/Replay lifecycle
  test pending. PARKED150_R1 run19620 validated parked variety after reloads,
  correct audio/rendering and about100FPS, but ended around a replay failure.

- [PARKED150_R1 build](PARKED150_R1_BUILD_2026-11-06.md): parked flat cache and25
  category arrays expanded50-to-150; complete native fixture and inherited checks
  passed. Live parked variety subsequently passed in user run19620; replay
  failures motivated REPLAY150_R1.
- [Latest parent live run29344](NOTIFY150_RUN29344_RESULTS_2026-11-06.md): extended
  Challenge/menu/Story/chase workload, roughly100 FPS and normal exit confirmed
  by user;14 expanded requests and three pending unloads. Native audio setup
  return failures occurred despite correct audible playback.

- [NOTIFY150 live run7480](NOTIFY150_RUN7480_RESULTS_2026-11-06.md): exact150-ID
  sidecar notification and native readiness passed; no reported integrity errors.
  Only four requested models had draw-call coverage, so full150-model visual/
  audio and lifecycle validation remains incomplete.

- [NOTIFY150_R1 correction](NOTIFY150_R1_BUILD_2026-11-06.md): native notification
  frame/source fix built and staged; complete callback/writer/ring/replay fixture
  and inherited checks pass. Targeted live retest7480 passed. Separate full snapshots remain
  unexpanded in that parent; see REPLAY150_R1 above. Evidence: `notify150_r1_2026-11-06/`.

- [Two BASELINE150 crashes](BASELINE150_CRASH_PAIR_2026-11-06.md): native50-ID
  notification stack overflow, exact0x96 return overwrite verified against both
  supplied dumps. Direct150 remains withdrawn on the older builds; use the new
  NOTIFY150_R1 guide for the controlled retest.

- [BASELINE150_R1 logging test](BASELINE150_R1_BUILD_2026-11-06.md): staged,
  compiled/fixture verified; direct Lua live runs failed as documented above.
  All capacity settings preserved.
  `baseline150_r1_2026-11-06/` contains native spans, fixture log and inherited checks.

- [Latest live verification](ARENA_REUSE_RUN20236_RESULTS_2026-11-06.md):33 setting requests, two unloads, no guard/fatal errors.
- [Current implementation and native validation](ARENA_REUSE_R1_BUILD_2026-11-06.md).
- `arena_reuse_run20236_2026-11-06/`: supplied-run summaries and lifetime verification.
- `arena_reuse_r1_2026-11-06/`: current native/compiled fixture evidence and inherited verification output.
- `WORKSPACE_ARCHIVE_2026-11-06.json`: organization verification, not a new gameplay test.
- [Workspace archive record](WORKSPACE_ARCHIVE_2026-11-06.md): what moved and how preservation was checked.

The build report records what was known when it was built. Its outstanding-live-test paragraph is superseded by the successful run report above.

Open questions are summarized in `../NEXT_ACTION.md`. Historical reports moved to `../archive/retired_2026-11-06/analysis/`; their proposed next steps are not current instructions.

## Retained prerequisites

The current verifier imports shared checks and validates source/native-byte digests recorded in earlier fixtures. These directories remain at their original paths to preserve that chain:

- `audio_default16m_r1_2026-11-05/`: shared-default-pool regression fixture; its historical name does not change the current64MiB configuration.
- `audio_model192_r1_2026-11-05/`, `audio_probe_r1_2026-11-05/`, `audio_reason_r1_2026-11-05/`, `audio_slots2048_r1_2026-11-05/`.
- `callback512_r1_2026-11-06/`, `chapters_r1_2026-11-06/`.
- `cleanup262144_r1_2026-11-05/`, `pending512_r1_2026-11-05/`.
- `indexed_capacity_r2_2026-11-05/`, `indexed_handles_static_2026-11-05/`.
- `traffic_docs_chapter_audit_2026-11-06/`: exact asset/profile inputs for the chapter manifest and fixture.

These are provenance/regression inputs, not independent current test plans. Read only the required file. Baseline ASIs used for hash checks now live in the dated archive and are resolved by `workspace_paths.historical_path`.

## Historical context for specific questions

- Parked Lua/native fallback: archived `PARKED_VEHICLES_FALLBACK_REPORT_2026-11-06.md`.
- Native audio registry geometry: archived `AUDIO_SHARED_MODEL_REGISTRY_45_STATIC_2026-11-05.md`;45 is the native layout, the current extension is192.
- Prior Challenge exits: archived `CALLBACK512_RUN23636_RESULTS_2026-11-06.md` and `CHAPTERS_PAIR_RESULTS_2026-11-06.md`; current run 20236 supersedes their stability result.
- Callback dump: archived `CRASH_20261106_001049_CALLBACK_ARENA.md`; its replay was conditional and current live callback peak is9.

All of those filenames are under the dated archive's `analysis/` directory. Use its manifest for exact restored paths and hashes.

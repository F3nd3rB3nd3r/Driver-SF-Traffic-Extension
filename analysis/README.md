# Analysis index

## Current conclusions

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

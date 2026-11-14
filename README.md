This is an AI generated repository with the intention of extending Driver: San Francisco's civilian traffic related limits.

# DriverSF V29s workspace

Read [START_HERE.md](START_HERE.md), then [NEXT_ACTION.md](NEXT_ACTION.md).

The current combined test is **REPLAY_READ1M_R1**, retaining REGMAP_REUSE_R2 fixes and traffic capacities. It gives file replay readers separate 1 MiB backing and adds targeted refill counters alongside the narrow client probe. Offline checks and [the first live run10556](analysis/REPLAY_READ1M_RUN10556_RESULTS_2026-11-14.md) pass: the earlier client3 waits are absent across two full viewings, and the user reports normal exit. Broader validation remains open. See [the test guide](dist/MAIN_150/REPLAY_READ1M_R1_TEST_2026-11-14.md). The active `DriverSF_V29s_NATIVE_TRACE.slnx` contains only `DriverSF_V29s_ARENA_REUSE.vcxproj`, Release / Win32.

- `src/`: preserved production source and shared conditional implementation.
- `dist/MAIN_150/`: current ASI, its test guide and latest validated user log.
- `build/v29s_replay_read1m_r1/`: current generated build output; prior output/artifacts are retained.
- `analysis/`: current reports plus explicitly retained verification prerequisites; see its README.
- `scripts/` and `tests/`: current entry points and inherited checks; see their READMEs.
- `archive/retired_2026-11-06/`: retired projects, tests, ASIs, logs, reports and deferred Forza files, with a SHA256/path manifest.

The dated archive preserves history; it is not a second active build tree. Restore specific historical files to their recorded original paths before trying to rerun an old experiment. Existing earlier archive, reference, tools, IDE and git data were left in place.

This reorganization preserves the source and working binary. It reduces active clutter; archived files still occupy disk space.

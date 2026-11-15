This is an AI generated repository with the intention of extending Driver: San Francisco's civilian traffic related limits.

# DriverSF V29s workspace

Read [START_HERE.md](START_HERE.md), then [NEXT_ACTION.md](NEXT_ACTION.md).

The current candidate is **USER CAP R3** in [dist/USERCAP_R3](dist/USERCAP_R3/USER_CAPACITY_R3.md).
It corrects R2's reproduced Story-loading crash: an address-order guard rejected
valid native completed-map storage after its external buffer became dynamic.
The15 configured capacities and150-request/160-model defaults remain unchanged.
Nineteen offline process cases pass. R3 now has live150-default coverage in both
Story/Replay directions and a reported normal exit; see [run34672](analysis/USERCAP_R3_RUN34672_RESULTS_2026-11-15.md). Larger configurations remain live-untested.
See the [crash analysis and fix](analysis/USER_CAPACITY_R3_STORY_FIX_2026-11-15.md).
R2 artifacts and the exact prior GENERIC_R1 fallback remain preserved.

Traffic selection follows the active AI data (`AssetWeights`) or native request
lists (`Native`), with no compiled car/chapter roster. A larger capacity does not
supply missing assets, change probabilities or prove sufficient shared budgets.
R3 has live default150 Story/activity/Replay/menu coverage. Larger asset
rosters and non-default capacities still need separate live validation.

The previous paired-live-pass candidate is **TRAFFIC_POLICY_R1**: an ASI-local INI chooses
`Native` (default, preserve requested IDs/order) or `Preset150` (the existing
asset-checked preload adapter). Capacities remain150/160. Static/parser checks
pass, as do the isolated Win32 fixture and compiled PE32/ABI/hook checks.
Built Release/Win32 and staged in `dist/MAIN_150/`; output
`build/v29s_traffic_policy_r1/`. See the [build report](analysis/TRAFFIC_POLICY_R1_BUILD_2026-11-14.md)
and [test guide](dist/MAIN_150/TRAFFIC_POLICY_R1_TEST_2026-11-14.md).
The supplied [INI](dist/MAIN_150/DriverSF_Traffic.ini) is ignored by older ASIs.

The latest live-tested baseline is **REPLAY_READ1M_R1**, retaining REGMAP_REUSE_R2 fixes and traffic capacities. It gives file replay readers separate 1 MiB backing and adds targeted refill counters alongside the narrow client probe. Offline checks and [the first live run10556](analysis/REPLAY_READ1M_RUN10556_RESULTS_2026-11-14.md) pass: the earlier client3 waits are absent across two full viewings, and the user reports normal exit. Broader validation remains open. See [the test guide](dist/MAIN_150/REPLAY_READ1M_R1_TEST_2026-11-14.md). The active `DriverSF_V29s_NATIVE_TRACE.slnx` contains only `DriverSF_V29s_ARENA_REUSE.vcxproj`, Release / Win32.

- `src/`: preserved production source and shared conditional implementation.
- `dist/MAIN_150/`: current ASI, its test guide and latest validated user log.
- `build/v29s_usercap_r3/`: active candidate output; previous outputs remain preserved.
- `analysis/`: current reports plus explicitly retained verification prerequisites; see its README.
- `scripts/` and `tests/`: current entry points and inherited checks; see their READMEs.
- `archive/retired_2026-11-06/`: retired projects, tests, ASIs, logs, reports and deferred Forza files, with a SHA256/path manifest.

The dated archive preserves history; it is not a second active build tree. Restore specific historical files to their recorded original paths before trying to rerun an old experiment. Existing earlier archive, reference, tools, IDE and git data were left in place.

This reorganization preserves the source and working binary. It reduces active clutter; archived files still occupy disk space.

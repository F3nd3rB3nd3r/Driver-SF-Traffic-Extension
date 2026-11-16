This is an AI generated repository with the intention of extending Driver: San Francisco's civilian traffic related limits.

# DriverSF V29s workspace

Read [START_HERE.md](START_HERE.md), then [NEXT_ACTION.md](NEXT_ACTION.md).

The current tested candidate is **USER CAP R8** in
[dist/USERCAP_R8](dist/USERCAP_R8/R8_TEST.md). It adds configuration-sized backing
and integrity checks for fourteen native manager key/refcount lists, fixing the
Getaway Activity 1 exit freeze found in R7. It retains the earlier traffic queue,
streaming-handler/scheduler and full-width mission-capacity corrections.
The active project builds Release/Win32 to `build/v29s_usercap_r8/`.

The compiled defaults remain 150 requests / 160 model slots. The included R8 INI
is the tested **280-request / 290-model** example, with 384 audio models,
4096 global audio entries, 128 MiB shared audio, a 2 MiB replay reader,
512 retained references and 180000 extra hash nodes. Enable exactly one traffic
ASI and use installed assets and AI weights appropriate for your chosen roster.

All 56 expected offline outcomes pass. In the 52-minute
[R8 run 18736](analysis/USERCAP_R8_RUN18736_RESULTS_2026-11-16.md), the previously
freezing Getaway exit succeeded, all 22 traffic phases reached native READY,
and the user reported no FPS, audio or rendering issues. The new key-list
backing was exercised beyond the old 256 entries, peaking at 280 of 802 per list
without integrity failures. The strict analyzer passes 43 of 44 checks: it
records 450 audio setup failures, with later successful setup for every affected
model and no reported audible issue. Their underlying cause remains unclassified.

R8 Story/menu/Replay validation remains open. Earlier
[R7 run 11636](analysis/USERCAP_R7_RUN11636_RESULTS_2026-11-15.md) covered smooth
replay and lifecycle changes in R7, before the Getaway-specific failure was found.
These results retain their exact workload/version scope; they do not validate
every chapter/activity, larger rosters or indefinite reuse. See the
[R8 implementation report](analysis/USERCAP_R8_MANAGER_KEY_FIX_2026-11-16.md).

Traffic selection follows the active AI data (`AssetWeights`) or native request
lists (`Native`), with no compiled car/chapter roster. A larger capacity does not
supply missing assets, change probabilities or prove sufficient shared budgets.
R8 has live Story/activity coverage with the supplied 280-car configuration. Larger asset
rosters and other capacities still need separate live validation. Entry capacities
and MiB budgets represent different resources; there is no universal MiB-per-car
conversion. Missing vehicle assets cannot be fixed by raising limits.

Local crash dumps, full session logs, temporary builds, generated asset fixtures
and pre-change snapshots are retained locally rather than bundled in the current
publication. Native verification requires the supported retail executable and,
for inherited fallback-preservation checks, the separately retained historical
workspace artifacts. See [scripts](scripts/README.md) and [tests](tests/README.md).

The following candidates describe earlier milestones and retain their original
validation scope.

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
- `dist/USERCAP_R8/`: current ASI, INI, validation notes and SHA256 manifest.
- `dist/MAIN_150/`: historical candidates and local test inputs.
- `build/v29s_usercap_r8/`: active candidate output; previous outputs remain preserved locally.
- `analysis/`: current reports plus explicitly retained verification prerequisites; see its README.
- `scripts/` and `tests/`: current entry points and inherited checks; see their READMEs.
- `archive/retired_2026-11-06/`: retired projects, tests, ASIs, logs, reports and deferred Forza files, with a SHA256/path manifest.

The dated archive preserves history; it is not a second active build tree. Restore specific historical files to their recorded original paths before trying to rerun an old experiment. Existing earlier archive, reference, tools, IDE and git data were left in place.

This reorganization preserves the source and working binary. It reduces active clutter; archived files still occupy disk space.

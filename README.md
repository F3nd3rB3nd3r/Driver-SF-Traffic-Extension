This is an AI generated repository with the intention of extending Driver: San Francisco's civilian traffic related limits.

# DriverSF V29s workspace

Read [START_HERE.md](START_HERE.md), then [NEXT_ACTION.md](NEXT_ACTION.md).

The current tested candidate is **USER CAP R7** in
[dist/USERCAP_R7](dist/USERCAP_R7/R7_TEST.md). It includes the traffic replacement
queue, streaming-handler/scheduler and full-width mission-capacity corrections
needed by the tested 280-car configuration. The active project builds Release/Win32
to `build/v29s_usercap_r7/`.

The compiled defaults remain 150 requests / 160 model slots. The included R7 INI
is the tested **280-request / 290-model** example, with 384 audio models,
4096 global audio entries, 128 MiB shared audio, a 2 MiB replay reader,
512 retained references and 180000 extra hash nodes. Enable exactly one traffic
ASI and use installed assets and AI weights appropriate for your chosen roster.

All 46 expected offline fixture outcomes pass. Live tests cover Story loading,
a speed-trap activity, pursuits, main-menu/Story/Replay changes, two uninterrupted
replay viewings and normal exit. [Run 11636](analysis/USERCAP_R7_RUN11636_RESULTS_2026-11-15.md)
passes all 42 logged checks, including 704 successful audio setups with no
failures. [Run 21688](analysis/USERCAP_R7_RUN21688_RESULTS_2026-11-15.md) also
worked in gameplay but recorded transient audio setup failures during a traffic
change; they recovered and did not recur in 11636. These results validate the
tested workload, not every chapter/activity, larger rosters or indefinite reuse.
See the [R7 implementation report](analysis/USERCAP_R7_MISSION_CAPACITY_FIX_2026-11-15.md).

Traffic selection follows the active AI data (`AssetWeights`) or native request
lists (`Native`), with no compiled car/chapter roster. A larger capacity does not
supply missing assets, change probabilities or prove sufficient shared budgets.
R7 has live coverage with the supplied 280-car configuration. Larger asset
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
- `dist/USERCAP_R7/`: current ASI, INI, validation notes and SHA256 manifest.
- `dist/MAIN_150/`: historical candidates and local test inputs.
- `build/v29s_usercap_r7/`: active candidate output; previous outputs remain preserved locally.
- `analysis/`: current reports plus explicitly retained verification prerequisites; see its README.
- `scripts/` and `tests/`: current entry points and inherited checks; see their READMEs.
- `archive/retired_2026-11-06/`: retired projects, tests, ASIs, logs, reports and deferred Forza files, with a SHA256/path manifest.

The dated archive preserves history; it is not a second active build tree. Restore specific historical files to their recorded original paths before trying to rerun an old experiment. Existing earlier archive, reference, tools, IDE and git data were left in place.

This reorganization preserves the source and working binary. It reduces active clutter; archived files still occupy disk space.

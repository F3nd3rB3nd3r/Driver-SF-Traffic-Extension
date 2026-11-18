# Current USER CAP R10 logging/configuration checks

`test_logging_policy.py` compiles/runs20 isolated Win32 processes: production
INI/parser/output-policy tests, retained R9 audio bridge ABI checks, configured
storage at18/150/255/256/280/320/500/700/1000 and complete copied-image cold
startup with the three documented profiles and all output levels. It replays
only the explicitly named supplied7044 log through the output selector; this
is not a game or speed benchmark. Output is `analysis/logging_r10_2026-11-18/`.

Build the active solution in Release/Win32, then run `verify_logging_policy.py`
to check the current PE32, fixture hashes, retained native audio signatures,
default summary policy and reduced/full parser semantics. These scripts do
not package, publish, install or launch the game. R9 artifacts stay preserved.
Other revision-specific verifiers below remain pinned to their original
outputs/manifests; do not retarget or run their old packaging modes blindly.

`analyze_generic_limits_log.py` recognizes R10's `LOGGING_START` declaration.
LogLevel0/1 intentionally lack full diagnostic evidence, so
`logging_full_trace=false` prevents a full integrity certification even when
available summaries are healthy. This is a scope restriction, not evidence of
a game fault. R9 and older full-trace logs retain their original checks.
See [CONFIGURATION.md](../CONFIGURATION.md) for settings/profiles.

# Previous USER CAP R8 checks

`verify_mission_capacity.py`, `verify_wide_limits.py`,
`verify_traffic_queues.py`, `verify_stream_handlers.py` and `verify_manager_keys.py`
exercise 56 expected
offline outcomes using fixture-owned state and copied retail native routines.
They never invoke the game entry point. Their matching `*_build.py` scripts
verify the compiled ASI, fixture/source hashes, signatures, startup records and
retained fallback artifacts. Use the current Release/Win32 project only.

These checks require the supported local Driver.exe, MSVC/Windows SDK, and the
historical local artifacts named by inherited fallback-preservation checks.
The repository publication omits full user logs/dumps, backup archives and
generated asset/build fixtures; restore those prerequisites separately when
reproducing the complete local verification chain. `analyze_generic_limits_log.py`
validates a supplied R8 log against recorded INI capacities, including the
full-width capacity query and manager-key storage. R7 and older log schemas
remain supported. An integrity pass is not a rendering/audio guarantee.

The entries below describe earlier checks and their original scope.

Current USER CAP verifier output is `analysis/wide_limits_r3_2026-11-15/` and its isolated build directory is `build/wide_limits_r3_fixtures/`. R2 evidence is preserved. The verifier now includes completed-map insertion in both allocation orders and three invalid fresh-cursor cases.

`verify_wide_limits.py` runs the USER CAP R2 isolated Win32/native fixtures; `verify_wide_limits_build.py` checks the compiled PE, preserved fallback and recorded fixture hashes. `analyze_generic_limits_log.py` accepts both R1 and R2 schemas and recorded capacities. Older150-only verifiers remain historical; do not use them to reject the R2 geometry.

# Current tooling

Use these entry points for the current build, only when the corresponding task is requested:

- `verify_generic_limits.py --fixtures / --verify-build / --package`: current
  GENERIC_R1 offline suite, compiled geometry/ABI/hook checks and workspace-only
  staging.12 isolated fixtures; cached runs require exact dependency/command/
  binary hashes. No game or installed ASI execution.
- `audit_generic_traffic_asset.py <asset> --output <json>`: read-only actual
  preload/positive-weight union audit; default file AI offset0x1080 is optional
  via `--ai-offset`. No source fingerprint/model allowlist required.
- `analyze_generic_limits_log.py <log> --output <json>`: validates recorded
  INI dimensions, requests/readiness and resource error counters. Test its
  nondefault/malformed cases with `test_generic_limits_log.py`.

- `verify_traffic_policy.py --static-only`: current source/project/inherited
  hash checks; no compiler or game access. `--fixture-only` compiles/runs the
  isolated Win32 policy/config/request tests. `--verify-build` inspects the
  current PE32/ABI/retained hook tables and geometry; `--package` stages the
  verified ASI without installing it. Build/fixture options were authorized
  for TRAFFIC_POLICY_R1 and pass. Old compiled verifiers
  remain locked to their earlier candidates; do not use their configuration/
  packaging modes blindly on the new TRAFFIC_POLICY_R1 project.
- `test_traffic_policy_logs.py`: synthetic Python log-parser regression cases.
- `analyze_traffic_policy_log.py <log> --output <json>`: Native preservation
  or explicit Preset150 validation. Run inherited baseline/callback/READ1M
  integrity checks separately; it does not certify rendering/audio.

- `verify_replay_read1m.py`: retained READ1M baseline capacity/counter verifier;
  `--fixture-only`, `--verify-build`, `--package`. Native reader/bridge/
  installer, guarded private backing, reuse/failure/destructor handling,
  parser and inherited client/R2 checks. Output
  `analysis/replay_read1m_r1_2026-11-14/`, `build/v29s_replay_read1m_r1/`.
- `analyze_replay_read1m_log.py <log> [--output <json>]`: validates private
  capacity/backing, native wait/progress counters and embedded CLIENT data.
  Stops alone do not prove a hitch; retained process-lifetime backing is
  expected. Explicit fallbacks/faults/rejected reads invalidate the test.
- `verify_replay_client.py`: current narrow replay probe verifier;
  `--fixture-only`, `--verify-build`, `--package`. Real native predicates,
  fast path, ECX scratch/input-self identity, ABI, boundary data/ring math,
  installer/parser checks and inherited R2 verification. Output
  `analysis/replay_client_r1_2026-11-14/`, `build/v29s_replay_client_r1/`.
- `analyze_replay_client_log.py <log> [--output <json>]`: client busy
  intervals, post-return sample conditions and raw recording ring/I/O state;
  rejects missing counters/pairs/drops and does not certify root cause.
- `verify_replay_hitch.py`: preceding broad timing probe verifier, `--fixture-only`,
  `--verify-build`, `--package`. Executes production observer/bridges/installer
  with bounded pressure/ABI/RX/rollback checks, parser checks and inherited R2
  verification. Output `analysis/replay_hitch_r1_2026-11-14/` and
  `build/v29s_replay_hitch_r1/`; no game installation/run.
- `analyze_replay_hitch_log.py <log> [--output <json>]`: passive replay timing,
  QPC conversion, wait/call/clock/rate and telemetry-integrity summary. Does
  not certify visible smoothness or independently identify a busy subscriber.
- `verify_regmap_reuse.py`: inherited definition-map lifecycle verifier.
  R2 adds production-identical Readable/real VirtualQuery, RX/RWX boundary
  regression, no-access/guard/corruption rejection and RX restoration checks;
  current output is `regmap_reuse_r2_2026-11-14`/`v29s_regmap_reuse_r2`.
  The fixture also covers actual native
  constructor/destructor/map operations, the production two-site installer,
  eight owners/100 generations, bounds/recycling/ABI/failure/rollback,
  compiled targets, parser and the inherited verifier chain. Focused options:
  `--fixture-only`, `--verify-build`, `--parser-only`, `--resume`, `--package`.
- `verify_replay_pause.py`: inherited paused-completion verifier; original native
  readiness/completion code and production bridge, guard/retry/ABI/rollback,
  compiled observer/targets, parser and inherited snapshot/capacity checks.
  `--fixture-only`, `--verify-build` and `--parser-only` support focused phases.
- `verify_replay150.py`: inherited snapshot verifier; native save/load slices,
  original overwrite reproduction, reference-list parity, cached/deferred restores,
  bounds/ABI/rollback, compiled targets, parser and inherited checks.
  `--fixture-only` checks the isolated fixture; `--verify-build` checks an already
  built ASI after that fixture; `--resume` reuses unchanged, hash-checked parent
  results after an interrupted downstream verification.
- `verify_parked150.py`: inherited parked candidate-cache/category build; complete
  native builder/copy/picker fixture, compiled ASI/bridge, parser and inherited
  notification/baseline/arena/capacity checks. Invoked with the active artifact parameters by the replay verifier.
- `verify_notify150.py`: inherited notification fix; complete native callback,
  writer, ring and replay fixture, compiled ASI checks, parser checks and inherited
  baseline/arena/capacity verification, parameterized by the parked verifier.
- `verify_baseline150.py`: inherited logging/native ABI verifier, parameterized
  by the current top-level verifier. Its default targets the older build.
- `analyze_baseline150_log.py <explicit-log> --output <summary.json>`: request
  roster, replay/parked/notification startup/cache/copy/failure checks, per-model coverage and
  per-phase sampled metrics. Use alongside the
  inherited callback analyzer. It never certifies visible/audible success.

- `analyze_callback_log.py <explicit-log> --output <summary.json>`: current arena/callback/chapter/cleanup/pending/audio log analysis. Structural checks do not establish audible playback or an OS exit code.
- `verify_arena_reuse.py`: inherited arena verifier; invoked with the new build
  parameters by `verify_baseline150.py`.
- `test_arena_reuse.py`: isolated Win32 native-byte fixture for the current repair.

The other retained `verify_*`, `test_*`, parsers and generators are shared prerequisites or tools for the open audio/traffic questions. Child verifiers' default names describe historical variants; invoke them through the current top-level verifier for the active build. Do not build an old variant because its script is present.

`workspace_paths.py` resolves an existing original evidence file or its unchanged copy under `archive/retired_2026-11-06/`. It is used for preserved baseline ASI hashes and the quiet-project flag comparison.

`archive_workspace_20261106.py` records the completed one-time archive. Its completed manifest is authoritative; it refuses to repeat a completed archive. Historical standalone investigations/variant checks are in the archive's `scripts/` directory and require restoration before reuse.

`verify_workspace_archive.py` checks organization, hash preservation, imports and file references without compiling, running game/native fixtures or reading external game files.

# Current tooling

Use these entry points for the current build, only when the corresponding task is requested:

- `verify_replay_read1m.py`: current combined capacity/counter verifier;
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

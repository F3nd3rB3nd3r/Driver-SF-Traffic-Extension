# Retained regression fixtures

These fixtures support the current ARENA_REUSE verification chain. They cover shared audio, indexed startup, pending/cleanup, chapter selection, callback storage and arena reuse. Older names identify the feature's origin, not an alternate current build.

Native-byte fixtures execute selected instructions in fake fixture-owned memory; they do not run Driver.exe or validate gameplay. Generated `.inc` files remain because compiled verifiers compare them against source/native provenance.

`ReplayReadTest.cpp` supports the active combined 1 MiB replay-reader test:
production backing/owner/constructor/destructor/installer and six branch
bridges, RX rollback, guards, allocation fallback, 100 same-address generations,
more than4 owners and writer passthrough. It also executes the native reader
body with isolated backend/link services for a three-block wrapped record,
async-busy/space/wrap/end stops and unchanged 32 KiB issue cadence. Run through
`scripts/verify_replay_read1m.py`; this invokes inherited client/R2 fixtures too.

`ReplayClientTest.cpp` supports the active narrow replay probe: actual busy
predicates and production bridges/observer/data/installer, native output and
saved input-self identity, fast-clear path, ring calculations, bounded
telemetry and rollback. Run through `scripts/verify_replay_client.py` when
requested; broad timing fixture remains historical evidence for28136.

The retired static-audio-pool variant fixture is preserved under `archive/retired_2026-11-06/tests/`. Current audio uses the shared default64MiB pool. Run only a fixture relevant to an explicitly requested check.

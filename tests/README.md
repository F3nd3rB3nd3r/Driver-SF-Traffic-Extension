# Retained regression fixtures

These fixtures support the current ARENA_REUSE verification chain. They cover shared audio, indexed startup, pending/cleanup, chapter selection, callback storage and arena reuse. Older names identify the feature's origin, not an alternate current build.

Native-byte fixtures execute selected instructions in fake fixture-owned memory; they do not run Driver.exe or validate gameplay. Generated `.inc` files remain because compiled verifiers compare them against source/native provenance.

The retired static-audio-pool variant fixture is preserved under `archive/retired_2026-11-06/tests/`. Current audio uses the shared default64MiB pool. Run only a fixture relevant to an explicitly requested check.

R3 adds completed-map tests to `WideLimitsTest.cpp`: real native insertion with backing below/above the package, exact capacity, recycling and in-place reconstruction. The original R2 guard fails insertion0 in the retained reproduction; the fixed guard passes the expanded matrix.

`WideLimitsTest.cpp` exercises USER CAP R2 through selected native routines and production helpers. It never invokes the game entry point or game loop. It maps a private copy of Driver.exe at a fixture address; the fixture-only identity branch and specifically relocated operands/vtable entries are excluded from the ASI. See the R2 report for scope.

# Retained regression fixtures

These fixtures support the current ARENA_REUSE verification chain. They cover shared audio, indexed startup, pending/cleanup, chapter selection, callback storage and arena reuse. Older names identify the feature's origin, not an alternate current build.

Native-byte fixtures execute selected instructions in fake fixture-owned memory; they do not run Driver.exe or validate gameplay. Generated `.inc` files remain because compiled verifiers compare them against source/native provenance.

`GenericLimitsTest.cpp` tests generic config parsing, arbitrary request IDs/
settings and runtime capacities18/50/100/150. `verify_generic_limits.py` also
generates parameterized parked18/100/150, snapshot100/retained400, reader1/4MiB,
audio256/global4096/default128MiB fixtures from retained source. It rebases
read-only copies of both actual AI files and checks all160 original/modified
profiles through production helpers. Expected fixture IDs are not ASI data.
No game executes; larger request capacities remain unsupported.

`TrafficPolicyTest.cpp` is the passing isolated Win32 fixture for Native versus
Preset150, the bounded startup config reader/parser, production request storage
and actual-ID diagnostics. It uses synthetic AI/native services and a substituted
return-address identity. `scripts/verify_traffic_policy.py --fixture-only`
compiles/runs it. The same script's `--verify-build` adds compiled candidate
ABI/retained hook/geometry checks. These do not certify live gameplay.

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

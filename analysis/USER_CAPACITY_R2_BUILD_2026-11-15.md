# USER CAP R2 implementation and offline verification

**Superseded after live failure:** R2 crashes on the first Story load in both supplied runs34196/31312. A missing completed-map allocation-order test allowed a guard regression through. See `USER_CAPACITY_R3_STORY_FIX_2026-11-15.md` and use the R3 candidate. The offline R2 results below retain their original limited scope.

The authorized four-step migration is implemented and staged as a candidate.
The known coupled traffic capacities now consume checked INI values; the old
150-request parser ceiling and its incompatible fixed backing are removed.
The supplied default remains the proven150/160 configuration values. The new
binary has **no live game verification yet**. No deployment or game/debugger
launch occurred. No game executable, AI file, assets or save were modified.

## Deliverables and preservation

- Candidate: `dist/USERCAP_R2/DriverSF_Unlimiter_USERCAP_R2.asi`, Release/Win32,
  276480 bytes, SHA256 `d5eb59980a2f31e77a0753d98d812388bff0f7f0c25820e71b7f9d0e0eb68f01`.
- Default INI and usage/limit guide: `dist/USERCAP_R2/DriverSF_Unlimiter.ini`
  and `USER_CAPACITY_R2.md`. All15 numeric capacities are documented. Use exactly
  one traffic ASI. The new package is separate from MAIN_150.
- Exact prior source/tests/project/INI/ASI snapshot:140 hash-verified files in
  `analysis/wide_limits_r2_2026-11-15/baseline/manifest.json`.
- Original staged GENERIC_R1 ASI remains SHA256
  `a8c2915aad6c3791410891be78d9066f68d35b5ab8e5a32185b2bf055c05722b`;
  original INI remains SHA256
  `2a0851edff6aa31dee19cfbc230319b345b27eabefb4e7cf0358cc51fd5315f4`.
  Existing uncommitted work was retained. Nothing was committed or pushed.

## Native provider/ordinal audit

The supported Driver.exe remains SHA256
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
Captured vehicle-handler vtable00E37118 resolves factory indirect calls0059FA85
and0059FAA0 to00858CD0 and00858860. Initialization accepts DWORD parameters;
the latter stores the ordinal as a DWORD at+564. Consumers00859162 and
00BB5690 preserve the DWORD, with the graphics selector clamping to the
runtime descriptor count minus one. No byte truncation was found on this trace.

The native initializer005A1420 zero gate still uses its original byte metadata.
R2 stores a nonzero clamped byte for vehicle kind0 and hooks only the package0/
kind0 loop bound at005A14EA to consume the configured DWORD ModelSlots.
Other kinds keep their original metadata and behavior. The recognized metadata
remains35/2/2, with four non-vehicle handlers; this is executable-specific.

Offline initializer tests substitute only factory object creation, then run the
actual initializer, actual ordinal setter and actual downstream graphics-selector
instructions. They exercise model counts255/256 and greater. Factory creation
with real resources, rendering and actual residency remain live-test scope.

## Coordinated storage changes

`V29sWideConfigCore.inc` owns parsing, derivation and checked native byte geometry.
The original eight-field GenericLimits prefix is retained; nine extended capacity
fields follow. The original R1 branch remains available for historical fixtures.
The active project enables WIDE_LIMITS and builds only USER CAP R2.

| Family | Migrated behavior |
|---|---|
|Requests/notification/parked|Configured private arrays,25 category buckets, ID snapshots, copy/range checks, request frame offsets and merge-clone offsets|
|Native stack|Request and notification page-probing bridges; parked builder retains native probing and gets a per-call reservation check; impossible frames log/refuse|
|Models/graphics|Configured counts and proportional native budgets; descriptor initialization bound, global/kind arrays, type16 capacities and ED/EF/selector/third/array backing grow coherently|
|Work/spool/clone/include|Configured node counts and cycle masks; stable growable owner directories; persistent owner/generation/recycling rules retained|
|Audio|Configured model registry, global pointer lists and default budget; stable per-manager backing replaces the eight-lifetime ceiling; model keys use a growable hash directory|
|Definition/pending/callback|Configured external/total counts, stable node addresses, guard regions and dynamic validation masks; native headers/sentinels retained|
|Cleanup/indexed/resource/hash|Separate configured counts and allocation arithmetic; indexed operand relocation; resource collection checks before its first native append write|
|Replay/baseline|Configured IDs/retained pairs and deep-copy diagnostic snapshots; dynamic text/scratch arrays; replay observer honors configured reader size|

The object-pool monitor now uses configured offsets/counts rather than512-only
geometry. Handler sidecar overflow explicitly refuses instead of returning the
last slot. Parked code's residual150-ID temporary and600-byte append check were
also migrated. Legacy startup lines are normalized to actual configured sizes.

Defaults preserve the prior requested capacities; derived private storage can
have more headroom (for example clone nodes derive from requests+retained).
This is not a byte-identical replacement for the old binary. Ownership and
ED/EF/reconstruction/replay lifetime fixes are preserved.

## Verification

Reproduce with the installed Microsoft toolchain, without the game running:

```
python scripts/verify_wide_limits.py
MSBuild DriverSF_V29s_NATIVE_TRACE.slnx /p:Configuration=Release /p:Platform=Win32
python scripts/verify_wide_limits_build.py
```

Evidence: `analysis/wide_limits_r2_2026-11-15/fixtures.json`,
`fixture_manifest.json`, `build_verification.json`, `build.log`, native and
compiled disassemblies, and startup fixture logs. The fixture manifest pins
its source dependencies, project flags, supplied INI and verification tools.

All16 isolated process tests pass:

- Nine sizes:18,150,245,246,255,256,500,700,1000 requests.245/246 derive255/256
  models. Arbitrary high valid IDs, actual wide native initialization/selection,
  request reads and notification copy, synthetic positive-weight asset roster
  derivation versus Native preservation, replay deep copies including nonzero
  retained refcounts, large baseline snapshots,12 audio manager lifetimes and
  more than512 diagnostic model keys all pass.
- Pending/callback fill/recycle, same-address pending/definition reconstruction,
  definition end refusal, spool/scratch validation, concentrated/mixed parked
  native builder/picker and a96 KiB page-crossing stack probe pass.
- Three complete cold installer sequences at150/700/1000 pass every startup
  dependency through replay reader installation. A fourth overrides every
  extended capacity (700 requests/720 models, audio800/global16384/default512 MiB,
  reader32 MiB/retained900, objects1024, definitions1500, pending2048,
  callbacks4096, cleanup1048576, collection300000, indexed524288, hash90000).
  Startup patch geometry is tested; a512 MiB real audio allocation is not.
- Three negative process tests explicitly terminate with29F3 for collection
  exhaustion before any out-of-bounds store (canary intact), real failed
  allocation, and invalid allocation-byte geometry.

The fixture maps its own image copy away from Windows loader allocations and
relocates the specific absolute operands/vtable data needed for these tests.
Only the fixture defines NATIVE_FIXTURE. The compiled ASI check proves the
production identity gate still requires00400000 and contains no fixture entry
points/car list. It also checks PE32 DLL/entry/bridge instructions, default
configuration data and imports (KERNEL32/PSAPI only). Build has0 errors;
remaining warnings are inherited unused optional probes and exported DllMain.

The analyzer accepts the existing10556 live R1 log unchanged. Its R2 branch
checks configured geometry and all corresponding startup records against
fixture logs, and correctly refuses to call startup-only logs a gameplay pass.
READY remains separate from visible/drawable residency and audible results.

## What is still bounded or unverified

This removes testing-only maxima from the migrated INI capacities; it does not
prove the absence of every native engine limit. Signed arithmetic, valid16-bit
vehicle IDs, byte setting/table selectors,25 parked categories, native unrelated
subsystems and actual32-bit address space/stack reservations remain. Some
passive detail samplers/observation rings remain bounded and are documented as
such; they do not allocate the engine's roster. Native per-object audio event/
list formats and fixed audio pool types are not arbitrarily widened here.

Auto capacities and graphics budget scaling provide consistent geometry, not a
worst-case resource-demand estimate. No exact per-car geometry/texture/VRAM/RAM
size or adequate shared budget at500/700/1000 is established. Constructor reuse
coverage in fixtures does not transfer to every possible game transition.

Next is user live regression with the supplied150 defaults and the known AI,
including Story/Challenge/activity/menu/Replay/Story re-entry/normal exit. Then
try a non-default capacity against a matching available asset catalog and AI.
Analyze the specific new log before tuning further. Larger rosters remain
live-untested and must not be advertised as verified gameplay.

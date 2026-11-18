# TRAFFIC_POLICY_R1 built, offline verified and staged

2026-11-14. User explicitly authorized fixture compilation and Release/Win32
build, verification and staging. No installation, game launch, ASI execution
inside another process or debugger attachment. The isolated test executable
runs only fake native services and fixture-owned memory.

## Artifact

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1.asi`

- 279040 bytes; SHA256
`bb7411bad4e1532c11789ba6966ec01e27e197ddd6a68b870895526bdc292a2d`.
- PE32 DLL, Release/Win32, built through the sole active solution
`DriverSF_V29s_NATIVE_TRACE.slnx` / ARENA_REUSE project.
- Output `build/v29s_traffic_policy_r1/`.
- Config `dist/MAIN_150/DriverSF_Traffic.ini`, beside the ASI at installation.
- Guide `dist/MAIN_150/TRAFFIC_POLICY_R1_TEST_2026-11-14.md`.
- Log `%TEMP%\DSFV29s_traffic_policy_<PID>.log`.

Default mode is Native, preserving actual requested IDs/order within the150
request capacity. Explicit Preset150 retains the previous checked55-profile
expansion;25 excluded profiles remain native. Capacities stay150/160 with
the inherited audio, ED/EF, pending/callback/cleanup, parked, REGMAP lifetime,
snapshot/pause and1 MiB replay-reader fixes. No weights or population edits.
See the source report for policy/parser behavior and error fallbacks.

For the currently adjusted uniform150 AI, select Preset150. Its positive
weights may cover150 while native named-setting request lists still cover18.
Native intentionally does not repair that asset mismatch. Test Native using
the original AI, or assets with consistent weights and preload lists.

## Actual checks

`scripts/verify_traffic_policy.py --fixture-only` passes. The first fixture
compile found a test-stub return-type error: the substituted return-address
function returned an integer while production expects a pointer. Corrected
the fixture to return void*. No production change was needed. The successful
fixture extracts the real request bridge/probe and includes production config
and chapter code. It verifies:

- INI/BOM/comments/whitespace and invalid/duplicate/missing config behavior;
 module-path truncation, missing file and read failure handling.
- All80 named source profiles preserved in Native without touching AI/root.
- An arbitrary noncanonical150 roster, inline prefix/sidecar integrity and
 actual-ID diagnostics, including detection of a deliberately wrong read.
- Oversized count, null source and IDs beyond16 bits rejected while retaining
 the previous list; zero/negative clear semantics.
- All55 eligible preset profiles expanded;25 excluded profiles retained.
 Synthetic asset mismatch, admission and other-caller guards; executable/
 setter-byte preflight; idle native processing called once.

The first sandboxed MSBuild attempt failed in FileTracker with Access Denied
after compilation. The authorized elevated retry completed with0 errors and
28 warnings: existing signedness/unreferenced-function warnings and the
existing exported-entry-point warning. No policy-module compiler warning.
The production sources were not changed during this build turn.

`scripts/verify_traffic_policy.py --verify-build` passes:

- Native executable identity checked against the supported SHA256.
- PE32/x86 DLL, only KERNEL32/PSAPI imports, with the four new local-file APIs
 present and no CRT dependency. Compiled default mode is Native.
- Request bridge fastcall/RET8 retained; DllMain RET0Ch reviewed. Disassembled
 chapter decision confirms the Native branch returns before preset lookups.
- 12 fixed geometry/data inventories exactly match the tested READ1M parent:
 pool/frame/indexed operands, notification/parked layouts, cleanup operands,
 profile data, setter signature and row IDs/weights.
- 10 retained hook tables compared against that parent: audio model8, global
 audio9, pending2, callback3, cleanup6, file reader8, replay clients3, snapshot7,
 REGMAP2 and the31 main request/handler/selector/ED/EF sites. Native addresses,
 stolen/dependency bytes and target identities match; linker jump thunks are
 resolved before comparing target symbols.
- Reader/client bridge GP/flags/FP-state and saved-stack patterns preserved;
 broad replay timing hooks absent. Fixed150/160 exports and policy markers
 confirmed. All24 unchanged inputs in the prior focused source/artifact
 inventory retain their hashes, including the old ASI and lifetime cores.
- Synthetic policy-log parser cases pass, including malformed/truncated logs,
 native order/noncanonical150 and explicit preset caller/roster checks.

`--package` verifies recorded source/artifact identities, refuses an unrelated
destination overwrite and rechecks staged bytes. Previous READ1M_R1 remains
276480 bytes with SHA256
`fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.

Evidence: `analysis/traffic_policy_r1_2026-11-14/` contains fixture build/run,
static/fixture/compiled/package JSON, MSBuild log, imports and selected compiled
disassembly. New checks inspect this candidate; old source-locked compiled
verification results were not relabeled or modified to imply a fresh pass.

## Scope and next step

This is an offline-verified candidate, not a new gameplay result. Native
services and return-address identity are stubbed in the isolated fixture.
Compiled table/bridge checks plus unchanged core hashes do not rerun every
historical failure fixture or certify installation/startup/gameplay lifetime.

Use one traffic ASI. Test original AI +Native separately from adjusted AI
+Preset150, including Story/Challenge/menu/replay transitions, traffic/parked
variety, rendering/audio/FPS and normal quit. Analyze supplied logs with the
new policy analyzer and inherited baseline/callback/READ1M analyzers before
proposing further changes. Counts do not prove simultaneous drawable residency.

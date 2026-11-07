# BASELINE150_R1: bounded diagnostics for a150-model vanilla baseline

Live update: runs23504/8732 crash on the direct150 Lua request. The missed native
notification buffer is diagnosed in `BASELINE150_CRASH_PAIR_2026-11-06.md`;
the direct test is withdrawn. Compiled/fixture results below remain valid within
their stated scope and did not exercise that complete native callback.

Built/staged at the user's request. No game installation/execution or capacity
increase. Active `.slnx` still contains the same single ARENA_REUSE project;
its target/output now select BASELINE150_R1, Release/Win32.

ASI: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_BASELINE150_R1.asi`.
205312 bytes; SHA256
`bfbb726524bff658166fbb0dbeb968f94fa3f567f50f120b7e514ab99202247f`.
Last live baseline ARENA_REUSE_R1 remains191488 bytes/SHA256
`879236089985e71ae7e601d74bfc8a9f5570e3679128ec8957b5ccfbb015b5f7`.

## Implementation

- Five conditional additions to the existing request writer capture original
  caller/input, three refusal reasons and the accepted effective roster after
  sidecar/count publication. All original request selection/storage text remains
  hash verified after stripping only those five diagnostic blocks.
- Native update entry005A7B80 has ten non-relative stolen bytes, ending at8A.
  Its bridge preserves GP registers,flags,DF,x87/SSE and the original stack;
  the native prologue executes through its trampoline.
- Once per second, on that native update path, verify the owner/manager/package
  context and query0059CFC0(manager,type0,id) for the current roster. Its exact
  body rejects type4, checks package membership/exclusion and the global
  registry; the calling convention is thiscall/RET8. This is a sequential sweep,
  not an atomic worker-thread view or drawable-resource test.
- At008D590C, replace only the direct CALL target with a preserving bridge
  which observes live EBX gameVehicle and then tail-jumps to008A8DA0. The native
  return address008D5911 and argument layout are preserved. Track original
  key+5AC and resolved model+5B4 -> definition+4850 -> ID+14 when the model
  has the verifiedE39EAC vtable. Unknown mapping is counted. CALL means observed
  render path, not visible pixels.
- Eight queued state snapshots record accepted/rejected/superseded phases;
  exact original/effective IDs are printed on acceptance. Overflow/contention
  are counted without rejecting requests or adding process exits. Checkpoints
  have timestamp,age and duration. Diagnostic startup failure is reported and
  leaves the parent behavior available; the new probes are not then usable.
- Audio scalar samples are frozen before supersession, with baseline values
  alongside; later phases cannot overwrite earlier audio snapshots. Native
  setup failures remain undecoded, audibility unknown.
- Sample published queue/callback/audio-slot gauges and PSAPI private/working
  bytes about every200ms. Summary records every second, full model/metric rows
  every five seconds and at phase boundaries. Existing native/insertion/lifetime
  peak records remain available. Shared audio bytes remain in AUDIO_PRESSURE.

## Verification

MSBuild Release/Win32:28 existing warnings,0 errors. The sandbox initially
blocked MSBuild FileTracker with access denied; the same build was rerun with
approved escalation. No toolchain/dependency installation.

`python scripts/verify_baseline150.py` passes. Evidence in
`analysis/baseline150_r1_2026-11-06/verification.json`, fixture.log,
fixture_analysis.json, native spans and inherited verification outputs.

The isolated Win32 fixture includes production diagnostic code and exercises:
150 unique IDs, correct readiness ABI/type, all-ready sweep, loss/recovery,
direct three-ID preservation, rejection retention, duplicate reporting,
explicit overflow/contention counts, original/resolved draw fallbacks,
phase peaks, immutable superseded audio snapshots and both bridges' complete
GP/flags/DF/x87/SSE/stack preservation while the mocked readiness service
deliberately clobbers floating-point/SIMD state. The log analyzer detects
intentional drops, identifies the exact vanilla roster and does not certify
the synthetic run. Parent arena/callback/chapter/pending/cleanup/audio/indexed
compiled checks and exact executable SHA/span verification also pass.

## Limits and next observation

The readiness service is mocked in the isolated fixture; its real bytes/ABI
are statically checked. This is not a whole-game test. Native resources are
queried on their native thread; the monitor does not walk vehicle objects.
No final-exit hook means abrupt termination can lose recent queued records.
Memory sampling includes the world and probe overhead. Geometry/texture/GPU
allocations, every pool's demand and downstream audio reasons remain unknown.
Missing draw coverage can mean a model was never exercised; the roster contains
trailers/special vehicles without conventional engine/horn behavior.

Use the staged guide/Lua helper, then inspect both baseline coverage and inherited
integrity analyzers alongside the user's rendering/audio/FPS/exit observations.
Retain ARENA_REUSE_R1 as rollback; do not call the150-model preset verified yet.

# Audio probe R1 build, 2026-11-05

The user explicitly requested the probe build. Release / Win32 succeeded for
the single active `DriverSF_V29s_NATIVE_TRACE.slnx` project,
`DriverSF_V29s_AUDIO_PROBE.vcxproj`. This implements the narrow measurement in
`AUDIO_PROBE_PLAN_2026-11-05.md` on the working indexed R2 configuration.

Artifact: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_PROBE_R1.asi`  
Size: 104448 bytes  
SHA256: `A9F5AB4720068F3424D159D8675DE2EE7D16A1018A6B566626AF645F69522975`

Staged only. No installation, plugins inspection, game run, process attachment,
external SDK download, unrelated folder inspection or web search occurred.
The original working R2 ASI is preserved with SHA256
`55EDA80BF5972CF0333DE3BF631AC6D145F529BC92026BDAA2789328D4398A46`.

## Configuration and measurements

The new project retains every quiet ordered-150 baseline definition and the
R2 capacity/indexed definitions, adding only `DSF_V29S_AUDIO_PROBE=1`. Its
artifact/output directory are distinct. The old R2 project remains available.
Audio byte budgets, max20 pool descriptors, 90 emitters, deferred cleanup90 and
bank queue50 are unchanged. Graphics160, requests150, ED/EF, pending redirects
and external indexed262144 remain enabled. Legacy heavy traces remain disabled.

`src/V29sAudioProbe.inc` provides fixed storage, helpers, transaction-style
startup installation, monitor summaries and pool/state snapshots.
`src/V29sAudioProbeBridges.inc` contains the six production naked bridges, shared
verbatim by the isolated test harness. Main-source integration adds the include,
post-indexed installation/startup line and monitor poll. Capacity logging selects
the distinct `%TEMP%\DSFV29s_audio_probe_<PID>.log` prefix only for this variant.

| Native site | Measurement |
|---|---|
| 0084BE74 | Bank ID, actual AKRESULT, signed pool ID, cookie, input bytes/buffer before native state4 |
| 0084AA29 | Native emitter attempts, pre-allocation active peak and >=90 refusals |
| 00C99AA5 | Nonzero-size Malloc null returns; pool/bytes/caller |
| 00C99B04 | Nonzero-size Malign null returns; pool/bytes/caller/alignment |
| 00C54E17 | Normal PostEvent return count and zero-result details |
| 00C54DD3 | Early external-source PostEvent failure details |

Successful allocations save/test/restore flags and jump to native epilogues
without C++ or FXSAVE. Normal successful PostEvent adds only its atomic return
counter and flags-preserving filter. Helpers use fixed unique record slots and
atomic publication, no middleware calls/heap/list walks/VQ/file writes. Detail
records are drained on the monitor. The inherited capacity monitor retains its
existing at-most-four emergency boundary writes; the new audio hooks add none.

Pool sampling validates table bounds (1..128 defensive maximum), record identity
and active marker. It reads fixed-block free counts independently of Malloc byte
accounting. Observed table/backing/attribute/size/inactivity changes start a new
sampled generation. Twenty descriptors is not twenty banks. Raw IDs in allocation
records are not dereferenced as pools or passed into middleware APIs.

Details are bounded independently by category (512/64/64/64/128/64); counts
continue afterward. No ordinary hot success path does file I/O. Summaries are
one-second, pool/state detail five-second, pool high-water sampling 200ms.

## Verification

Commands:

```powershell
& 'E:\Visual Studio\MSBuild\Current\Bin\MSBuild.exe' DriverSF_V29s_NATIVE_TRACE.slnx /p:Configuration=Release /p:Platform=Win32 /verbosity:quiet
python scripts/test_audio_probe_bridges.py
python scripts/verify_audio_probe.py
```

Evidence lives in `analysis/audio_probe_r1_2026-11-05/`:

- `bridge_tests.json`: 10 synthetic invocations through all six actual production
  bridges; distinct bank result/pool values, allocator failure/success, emitter
  89/90, both event exits, return values/stack cleanup, DF/CF, ECX/EDX and complete
  saved x87/SSE image. The helper intentionally changes FP/SSE state.
- `startup_checks.json`: all 29 production startup checks passed, reproducing
  the original self-rejection and verifying the owned-handler correction.
- `indexed_verification.json`: all 13 compiled operand rows, original plus three
  external-base constructor/exhaustion/recycle cases and 165 address checks passed.
- `verification.json`: quiet flags, single active project, three existing
  capacity bridges/native signatures passed.
- `audio_verification.json`: six compiled signature rows, linker-thunk-resolved
  bridge/helper targets, argument offsets and restore paths passed; whole-text
  near/short branch scan found no candidate incoming transfer into stolen
  instruction interiors. Containing native functions and compiled bridges were
  disassembled into adjacent evidence files.

Reusable baseline verifiers now accept project/output parameters. Their defaults
remain R2. The capacity verifier handles incidental E8 bytes in immediates by
discarding unbacked candidate call destinations, then still requires exactly one
resolved call to the expected helper. No game behavior depends on these scripts.

Build warnings concern inherited signedness, unused quiet diagnostic functions
and the exported DllMain entry point. There were no compile/link errors.

These checks do not prove runtime installation, audible output or performance.
They do not exhaustively prove indirect entry aliases, simulate the entire
DllMain, fault-inject OS patch/cache/protection failure, or detect pool reuse
entirely between samples. GetBlock/CreatePool/voice/mixer paths remain outside
the first probe. A nonzero playing ID is admission, never proof of playback.

## Next user test

Follow `dist/MAIN_150/AUDIO_PROBE_R1_TEST_2026-11-05.md`: initial menu for 10s,
load the same save, exercise pause-menu/horn/engine/tires and drive about two
minutes. Return the complete new log plus audio/visual/FPS observations.
Analyze that transition before proposing any audio capacity change.

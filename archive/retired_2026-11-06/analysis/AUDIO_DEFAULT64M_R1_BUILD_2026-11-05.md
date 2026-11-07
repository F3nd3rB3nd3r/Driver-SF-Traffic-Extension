# Corrected comparison: shared default audio pool64MiB

The user clarified that their64MiB request meant the shared default pool, which
the first audio fix increased to16MiB. The assistant had instead interpreted
the request as targeting the97%-used StaticBanksPool. The earlier `STATIC64M`
variant is preserved but is not the requested shared64 test. This build corrects
the target and returns the static-bank allocation to its original size by
omitting that optional patch in a fresh process.

The user's latest observations: Cadillac DTS has audio, Bentley Arnage T lacks
proper audio, Pagani Zonda remains silent when found again, and cars working in
the previous run continue working. Earlier default16 observations also identified
Nissan370Z as missing engine/tire/horn, while Crown Victoria Cop Car, Scirocco
and C10 tow truck worked. No new matching static64 log was supplied/analyzed in
this turn; these are user observations, not verified event-to-vehicle mappings.

Artifact: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT64M_R1.asi`  
Release / Win32;109056 bytes  
SHA256: `6EC8CA1FFE69452DFC3E69E9C1E158D1F70109F0CFBFEFCAF5C601C9770B1167`

Staged only, not installed or game-tested. All earlier artifacts are preserved.
No plugin inspection, process attachment, game run, web search, dependency
installation or unrelated-folder access occurred.

## Configuration

The single active solution now selects `DriverSF_V29s_AUDIO_DEFAULT64M.vcxproj`.
It derives from the default16 project, replacing the16MiB feature definition
with `DSF_V29S_AUDIO_DEFAULT_64M=1`. It does not enable
`DSF_V29S_AUDIO_STATIC_64M`. Exactly one ASI project was built, Release / Win32.

The production shared-default installer remains in
`src/V29sAudioDefaultPoolPatch.inc`. Its selected request is67108864B in this
variant,16777216B otherwise. The same native operand at0084A650 changes within
the seven-byte `MOV [EBP+40],imm32` at0084A64D. Static-bank `PUSH 0042B800` at
0084A683 stays stock; the64MiB default installer additionally fingerprints it
and refuses a conflicting static expansion.

Relative to run6736, this changes only the default request16 to64MiB (+48MiB).
The platform request remains3MiB; StaticBanksPool remains4372480B. Thresholds
remain0.9 and all fixed object/queue/descriptor counts remain unchanged. The
working rendering settings150/160/ED+EF/indexed262144 and pending/hash expansions
are preserved. No new playback hot hooks are added. The six existing audio
bridges and three capacity bridges remain.

Native CreatePool uses32-bit division/stores for block counts and can represent
1048576 blocks of64B. The heap's checked size range includes64MiB. These checks
establish representation and static acceptance; real allocation is not exercised
until the game runs.

The main include/install/poll guards accept either the16 or64 default-pool
feature. The default-pool logging code reports the selected request. Distinct log
prefix: `%TEMP%\DSFV29s_audio_default64m_<PID>.log`. `AUDIO_LIMIT_START` must show
testRequestedBytes67108864, and `AUDIO_PRESSURE` confirms the live configuration,
used/usable bytes and sampled pressure. No static-expansion startup/live markers
are compiled into this ASI.

The analyzer recognizes default16, default64 and the separate static64 profile,
and checks the live requested value corresponding to the startup marker.

## Validation

```powershell
& 'E:\Visual Studio\MSBuild\Current\Bin\MSBuild.exe' DriverSF_V29s_NATIVE_TRACE.slnx /p:Configuration=Release /p:Platform=Win32 /verbosity:quiet
python scripts/verify_audio_default64_pool.py
```

Evidence: `analysis/audio_default64m_r1_2026-11-05/`.

- Shared production installer passes22 scenarios in64MiB mode, including the
  original20 cold/signature/identity/rollback cases plus refusal of a modified
  static-bank opcode/budget. This executes the actual production C++ in fake
  game memory with fault-injected OS calls.
- The same source also passes20 scenarios in16MiB mode, confirming that the
  size selection preserves the prior branch. Only standalone harnesses are
  compiled in both modes; the old ASI projects were not rebuilt.
- Fifteen isolated native-pressure predicate cases per mode pass, including
  unsigned/boundary arithmetic and recorded historical usage. Native absolute
  references are rebound to test-owned constants and mocked pool stats; the
  test never loads an ASI or starts Driver.exe.
-29 R2 production startup checks passed again.
- Compiled quiet flags, exact signatures, six audio bridges, three capacity
  bridges, near/short incoming-branch checks,13 indexed patch rows, constructor/
  free-chain models and165 indexed address checks passed. The unchanged shared
  bridge harness's10 invocations were reused with matching source hash.
- The sole-default-operand model preserves the static-bank instruction; the
  compiled default installer contains64MiB, the binary has no static-expansion
  markers, and native block/heap checks pass. Original default16/static64 ASI
  hashes remain unchanged.
- Historical pressure evidence was reused from the existing analyzed JSON;
  no old raw-log scan was required.

Results: `default64_verification.json`, `default_pool_verification.json`,
`pool_tests.json`, `default16_regression/pool_tests.json`, `startup_checks.json`,
`audio_verification.json`, `indexed_verification.json`, `verification.json`.
Build log: `build/audio_default64m_r1_build.log`; no build errors, inherited
signedness/unused diagnostic/exported-entry warnings remain.

This fulfills the clarified capacity comparison. The run6736 default peak32.6%
does not predict that64MiB will restore the remaining cars, and a repeated
car-specific pattern still needs event/bank investigation if extra capacity has
no effect. No audible fix or per-car root cause is claimed from offline tests.

Next: follow `dist/MAIN_150/AUDIO_DEFAULT64M_R1_TEST_2026-11-05.md`, test Zonda/
Bentley/Nissan against known working cars, and return the new complete log.

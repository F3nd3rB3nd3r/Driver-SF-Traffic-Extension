# Requested StaticBanksPool64MiB comparison

The user requested a64MiB test after run6736 showed partial audio recovery,
StaticBanksPool97.035% usage and remaining silent Zonda/370Z audio. This new
variant isolates **StaticBanksPool**, the97%-used pool, while preserving the
working16MiB shared default pool. It does not equate StaticBanksPool occupancy
with the native90% gate, which tests the default/platform pool IDs only.

The existing evidence still does not establish that static-bank capacity causes
the remaining failures. The user's requested experiment is now concrete and
reviewable, without assuming that particular cars are deliberately excluded.

Artifact: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_STATIC64M_R1.asi`  
Release / Win32;112640 bytes  
SHA256: `1C36DBAD10B092555D8B3253D6BF7B93879C2F8AD8F3C24AC4B21284707CABD8`

Staged only. No installation, plugins inspection, game run, process attachment,
web search, external SDK or unrelated-folder access occurred. Supported exact
Driver.exe SHA256 remains
`C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.

## Single additional setting

Native `0084A683` is `PUSH 0042B800`; its four-byte operand at0084A684 becomes
`04000000` (67108864B). This increases requested static-bank storage by62736384B,
about59.8MiB. The subsequent `0084A696` call to CreatePool00C99D90 retains
blockBytes1024, attributes1 and the native pool creation/lifetime machinery.

CreatePool uses32-bit division and stores the block count with `MOV [ESI],ECX`
at00C99F40, so65536 blocks do not truncate to a16-bit zero. Native general-heap
initialization00C9A380 accepts this size within its checked approximately1GiB
range, subtracting metadata before setting up its heap. This establishes the
size representation and acceptance check, not that real allocation will succeed.
No inline array is enlarged by changing only a compare.

All previous behavior settings remain: default16MiB, platform3MiB, native90%
thresholds, emitter90, cleanup90, bank queue50, pool descriptors20, requests150,
graphics160, ED/EF, pending/hash expansions and indexed262144. The old R2,
audio-probe and default16MiB ASIs remain preserved. The active solution selects
only `DriverSF_V29s_AUDIO_STATIC64M.vcxproj`.

## Startup and logging

`src/V29sAudioStatic64Patch.inc` requires successful indexed/audio/default16
baseline status, cached preflight/owned-handler success, stable image identity
and the installed default16 operand. It refuses an existing engine root or
initialized audio memory manager (max/count/table nonzero), as this static bank
pool is created before SoundEngine::Init. Native initial root/max/count/table
are zero, and default ID isFFFFFFFF, verified in the exact PE's initialized data
or loader-zeroed virtual section tail.

The exact stock size instruction,1024-byte block context and CreatePool call
must match. Cold/signature state is rechecked immediately before the write.
Normal commit/cache/protection failure restores the original static operand;
unsuccessful rollback is separately reported as-2 and never claimed as success.
The retained default16 operand is not reverted by a static-patch failure.

`src/V29sAudioStatic64.inc` adds startup and low-rate live pool lines. Live
snapshots wait until sound initialization, validate manager/pool ID/table/record
and recheck descriptor identity. They report actual static-bank used/usable
bytes and block count, not just the requested configuration. No new hot audio
hook, middleware call or allocator/list walk is added.

Log: `%TEMP%\DSFV29s_audio_static64m_<PID>.log`. Distinct `AUDIO_STATIC_START`
and `AUDIO_STATIC_POOL` rows supplement the preserved default-pool pressure and
bank/event/capacity diagnostics. Boolean `audioCapacityChanges=1` remains true;
the two allocation patches have separate startup rows. The log analyzer now
recognizes the static64 variant and requires its startup/live-size success.

## Verification

Commands:

```powershell
python scripts/test_audio_static64_pool.py
& 'E:\Visual Studio\MSBuild\Current\Bin\MSBuild.exe' DriverSF_V29s_NATIVE_TRACE.slnx /p:Configuration=Release /p:Platform=Win32 /verbosity:quiet
python scripts/verify_audio_static64_pool.py
```

Evidence: `analysis/audio_static64m_r1_2026-11-05/`.

- `static_pool_tests.json`:29 synthetic scenarios execute both production
  installers in order, including missing/altered baseline, root/memory-manager/
  engine initialization refusal, changed signatures/context, late mutation,
  cache/protection rollback and incomplete rollback reporting. Only the four
  new operand bytes change; default16 survives static rollback.
- `startup_checks.json`:29 inherited R2 production startup checks passed.
- `audio_verification.json`, `indexed_verification.json`, `verification.json`:
  compiled six audio bridges, three capacity bridges, native signatures, near/
  short incoming branch scans, quiet flags,13 indexed operands, constructor/
  free-chain models and165 indexed address checks passed. The unchanged shared
  audio-bridge10-invocation harness result and default-patch20-scenario result
  were reused with matching production source hashes.
- `static64_verification.json`: native instruction/context fingerprints, dynamic
  block-count/heap-size checks, compiled16/64MiB patch functions, startup order,
  sole additional operand model and preserved baseline ASI hash passed.
- Focused native CreatePool/heap disassembly is retained alongside those checks.

No build errors. Inherited signedness, unused diagnostic-function and exported
entry-point warnings remain. Build log: `build/audio_static64m_r1_build.log`.

Production tests use fake image/OS state. Full DllMain, real allocation/OS-failure
behavior, audible results and FPS remain untested. No runtime fix is claimed.

Next: follow `dist/MAIN_150/AUDIO_STATIC64M_R1_TEST_2026-11-05.md`, test both
previously silent and working vehicles, and return the new log before another
capacity change.

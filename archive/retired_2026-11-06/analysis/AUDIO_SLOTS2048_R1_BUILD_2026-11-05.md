# Combined audio slots2048 and count test — 2026-11-05

Built at the user's request to combine a capacity increase with measurement. Staged `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_SLOTS2048_R1.asi`,116224 bytes, SHA256 `C0210BF47A3C61B7B9A547D0AB30A359D9D991D6BBD153F96F35952A3ABFEFCC`. Release/Win32 only; active `DriverSF_V29s_NATIVE_TRACE.slnx` selects the single new `DriverSF_V29s_AUDIO_SLOTS2048.vcxproj`. No installation or game execution.

## Behavior

One DWORD operand in `C50F0C MOV ECX,255` becomes2048. The native stores atC50F17/C50F1A feed both manager settings before initialization, allowing native initializers to allocate all three arrays at2048 pointers each. Both arrays in manager A share the configured maximum; manager B has its separate configured maximum. No allocation-gate-only patch or global byte-index widening is used. Separate per-record8/255 limits remain unchanged.

The previous shared-default64MiB baseline is retained, including original StaticBanksPool4372480B, platform3MiB and unchanged0.9 thresholds. The existing audio probe,150 requests/160 graphics slots, ED/EF, indexed262144 and pending/resource expansions remain enabled. Previous ASIs remain unchanged.

## New observations

Nine cold-installed measurement detours, with exact native signatures and complete branch-free stolen instruction spans, record:

| Site | Observation |
|---|---|
| C6B42F | A selected-list allocation attempt/count/ceiling |
| C6B92B | B allocation attempt/count/hard ceiling |
| C6B4F4 | A actual insertion count and peak |
| C6B975 | B actual insertion count and peak |
| C6B4A0 | A selected reuse candidate, including fallback after allocation failure |
| C6B9B5 | Separate eight-user pre-attachment count/full gate |
| C6B589 | Separate byte255-user pre-attachment count/full gate after duplicate check |
| C6B2B6 | A native update entry count/peak, separately for A0/A1 |
| C6B794 | B native update entry count/peak |

Hot helpers only read the object/vector headers already owned by the executing native path and update atomic scalar counters. They do not allocate, perform file I/O, walk records or call middleware. The monitor prints scalars once per second, avoiding object-destruction races caused by background vector traversal. Separate fields need not describe one simultaneous snapshot. Peaks persist across later removal; last native update counts may fall. Actual initialization/capacity is reported from observed live headers. A selected reuse is not proof of successful replacement or audible playback.

## Startup and verification

Installation requires verified R2/indexed startup, the existing six-hook audio probe, the shared-default64MiB patch, correct executable identity, stock new signatures and uninitialized audio/managers. All nine trampolines are allocated before mutation. Both code pages are pre-protected and cold/signature checks repeated. Settings and hooks commit together; ordinary cache/protection failure restores stock settings/hooks. Failed rollback is explicitly status-2; trampolines remain process-lifetime if a detour may still reference them.

`python scripts/test_audio_slots.py` passed using production core/bridges/installer against fake memory and injected OS failures. Coverage includes missing baseline/identity/cold phase/signatures; all nine allocation failures; both pre-protection failures; late cold/signature changes; cache/protection commit rollback; explicit rollback failure; distinct A0/A1/B counts above255 and at2048; insertion/update/ceiling/reuse counters; separate8/255-user boundaries; malformed headers; nine bridge argument/register/flags/DF/x87/SSE preservation invocations.

`python scripts/verify_audio_slots.py` passed for the compiled artifact: all nine native and compiled signature rows, instruction boundaries, helper arguments/calls/trampoline targets, no decoded incoming branches into stolen spans, native settings stores and unchanged StaticBanksPool instruction. It also passed inherited29 startup guards, six audio bridges, three capacity bridges,13 indexed rows,165 addressing checks and constructor-chain/exhaustion checks. The previous DEFAULT64M and working indexed R2 hashes were verified unchanged.

MSBuild initially compiled but its FileTracker failed with sandbox E_ACCESSDENIED. The same Release/Win32 build was rerun with approved escalation and succeeded. Build log: `build/audio-slots2048-build.log`. Existing unreferenced-function/signedness/exported-entry warnings remain; no new slot-source warnings. Evidence: `analysis/audio_slots2048_r1_2026-11-05/fixture_results.json`, `verification.json`, `slot_branch_candidates.json`, focused native disassembly and `baseline/` verification outputs. No debugger, web search or other game directories inspected.

## User test

Procedure: `dist/MAIN_150/AUDIO_SLOTS2048_R1_TEST_2026-11-05.md`. Return `%TEMP%\DSFV29s_audio_slots2048_<PID>.log` into `dist/MAIN_150`, with working/silent-car comparisons, menu effects and rough FPS. Require new startup status1/patches1/hooks9 and observed live configured/allocated2048, plus inherited startup/live64MiB validation. Only runtime results can establish whether exceeding255 helps or whether another gate remains.

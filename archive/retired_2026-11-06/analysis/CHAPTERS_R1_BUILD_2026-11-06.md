# Chapter-independent preload R1: built and staged

User prioritized chapter independence over the parked-car investigation. This extension retains the validated150request/160slot PENDING512_R1 baseline and changes the preload selector only. All live game results remain pending user testing.

## Artifact

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_CHAPTERS_R1.asi`,176640 bytes.

SHA256 `602AA57D7F5464FAA3456499AC3009A07E854E76FBC2B8D6264C2CCD1B229442`.

Active solution `DriverSF_V29s_NATIVE_TRACE.slnx` selects only `DriverSF_V29s_CHAPTERS.vcxproj`, Release/Win32. No other ASI project was built. The previous PENDING512_R1 staged artifact is unchanged at SHA256 `F96FC542561528B8F1CFA3D8F958D88F92A508EE372D5DB6F0392F077FC0409B`. No plugin-folder installation, game run or game asset write was performed.

## Verified boundary and policy

The supported Driver.exe SHA256 was rechecked as `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`. Bounded entry-aligned disassembly confirms the named setter stages its small original list, writes actual hash/frequency, and calls005A79F0 at0076FE95, returning to0076FE9A. A binary direct-call search finds the same three documented writer call sites:0066E5F3,0074BA58,0076FE95. The first two retain native request behavior in the new selector. This is not proof that all indirect callers are known; unrecognized callers are also passed through.

The new feature uses the existing request bridge; it installs no additional native detour. It captures the caller's return address through the existing compiler intrinsic, checks root+214 owner identity and the actual context hash/frequency, then matches the full-DWORD native list against the generated profile. Existing graphics initialization/admission checks remain required.

Generated data covers all80 active profiles.55 expand:51 named SP profiles plus4 profiles under the two still-unnamed hashes whose tables already select the common150 roster. This is a data-content policy, not a new assignment of those hashes' game modes.20 named Online and5 remaining unnamed profiles retain their native lists. No current supported profile needs more than the existing150-model roster.

Before expansion the request thread checks live AI counts/pointers, the profile's setting record and canonical list,319-group table routing, and every row/weight/total of its reachable tables. The weights must describe exactly the existing uniform150 roster. No live-array traversal occurs in the monitor. Unexpected data leaves the native request unchanged and logs a reason. An incompatible native setter signature fails startup before baseline patching with exit29F1 and a CHAPTER_START refusal.

The stock40-DWORD setter frame remains unchanged: it stages the original small native list, and the150-ID source substitution happens afterward in the already expanded writer. Native empty/reset requests, separate mission-pot calls, setting/frequency state, road/placement configuration and normal releases remain native.

## Code and logging

- `src/V29sChapterProfiles.h`: generated80-profile catalog,191 row IDs/expected weights,198 exact setter signature bytes.
- `src/V29sChapterCore.inc`: caller/owner/profile/live-content checks and request selection.
- `src/V29sChapterRuntime.inc`: startup/event/summary/per-profile diagnostics.
- Main source integrates these behind `DSF_V29S_CHAPTERS`; older builds keep the prior Chapter7 branch.
- `src/V29sCapacityTrace.inc` selects new log prefix `%TEMP%\DSFV29s_chapters_<PID>.log` for this build.

New authoritative markers are CHAPTER_START, CHAPTER_EVENT and CHAPTER_STATUS. Legacy c7pre counters can remain0 and must not be treated as a failed chapter-independent substitution.

## Offline verification

Evidence: `analysis/chapters_r1_2026-11-06/` and bounded native disassemblies in `analysis/chapter_entry_static_2026-11-06/`.

`scripts/generate_chapter_profiles.py` reads only the specifically authorized installed/original AI files plus the supported executable and the current workspace's audited name reference. It validates exact file identities,80 profiles/55 uniform targets, source IDs within the common150, selector/slot identity and every table total/row order. One profile has17 distinct route tables; the catalog covers that actual maximum rather than assuming Chapter7's16 is universal.

`scripts/test_chapter_requests.py` passes a standalone Win32 fixture using the exact production request-bridge body and chapter policy. It executes the actual198-byte native setter with four absolute global operands relocated into fixture-owned storage; native relative-call distances are retained. Native setting lookup, road-rebuild helpers and request processing are explicitly stubbed. This harness never attaches to or executes the game or ASI.

Checked outcomes:

- All55 target profiles produce the exact same150-ID request; all25 excluded profiles retain native counts.
- All55 expansion profiles refuse the original nonuniform AI content.
- Both other known call-site return addresses preserve the original18-ID request even with Chapter7 active.
- Empty native frequency requests clear normally; processing dispatch still occurs when the original pending flag permits it.
- Full-DWORD source mismatches, owner/context/graphics/admission failures, and altered IDs/weights/totals/routes/counts/pointers refuse expansion.
- Three repeated55-profile cycles retain the same request-sidecar owner.
- Native setter call stack/RET8 and production intrinsic return-address handling work through the executed boundary.

`scripts/verify_chapter_requests.py` passes compiled catalog/signature/marker/startup-order checks and all inherited pending512, cleanup, model-audio, slots, audio pool/probe, indexed/capacity and29 startup checks. The production source/data digests match the fixture. Compiled request code reads its true caller return from EBP+4 and retains RET8; the policy compares the correctly rebased0076FE9A boundary.

The pending verifier was parameterized for another child build while retaining original defaults and original fixture evidence. A raw-E8 scanner in the audio-reason verifier was corrected to ignore unbacked immediate/padding candidates, matching the other existing verifiers; this is verification code only.

One Release/Win32 MSBuild build succeeded. The initial sandbox attempt compiled but FileTracker failed with E_ACCESSDENIED; the authorized retry outside the sandbox passed. Compiler/linker output contains existing C4018/C4505/LNK4216 warnings and no chapter-module warnings.

## Limits and next run

Offline fixtures do not execute the real chapter managers, mission logic, renderer, audio or teardown. They do not establish150 simultaneous READY/drawable vehicles, every owner-table lifetime across chapters, or a fix for parked cars. The live AI validation intentionally refuses unsupported asset edits and does not implement arbitrary rosters/INI configuration.

User guide: `dist/MAIN_150/CHAPTERS_R1_TEST_2026-11-06.md`. First regress Chapter7, then load a different real chapter and exercise a normal mission/activity transition plus menu/reload/exit. Return the new chapters log and visible/audio observations before another probe or capacity change.

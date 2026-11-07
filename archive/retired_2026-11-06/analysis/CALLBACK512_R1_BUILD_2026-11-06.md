# CALLBACK512_R1 built and staged

User authorized the extension following the callback-arena static audit. Staged artifact:

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_CALLBACK512_R1.asi`

- Release/Win32,185856 bytes.
- SHA256 `00A7F6E81FED401031302A10E4B169FDEB0D7DAD15E2A9A74A25D98F5683BF18`.
- Built through active `DriverSF_V29s_NATIVE_TRACE.slnx`, which selects only `DriverSF_V29s_CALLBACK512.vcxproj`.
- Built/staged only; no game installation, launch, debugger attachment or game asset modification.
- Previous CHAPTERS_R1 and PENDING512_R1 artifacts retain their recorded hashes.

## Behavior and scope

Each of21 native global callback lists retains its header,64 inline12-byte nodes, sentinel and314h spacing. Private process-lived storage adds448 nodes per group, for512 total. Only a recognized header's fresh cursor is redirected at its original64-node boundary. Recycled nodes continue through the original native allocation path and retain the fresh cursor at their chain's tail. Storage is never reclaimed or replaced on challenge/menu changes.

Three added hooks:

| Native site | Action |
|---|---|
| 005567D0,6 bytes | Shared insertion wrapper. Only exact global header membership/alignment uses the extension; other headers call the native trampoline. |
| 00858FB5,6 bytes | Mid-function writer check validates handler selector before native group-address arithmetic. Preserves GP registers, flags, DF, x87/SSE through a saved machine-state bridge. |
| 00859000,5 bytes | Calls native polling through its trampoline, validating chains/guards/counts before and afterward and accounting for native removals, including same-thread insertions during callbacks. |

Native insertion, callback invocation, traversal ordering, removal and recycling are retained. The extension does not suppress release or silently discard callbacks. Header resets with issued nodes, wrong ownership/alignment, cycles, capacity exhaustion and invalid selectors are diagnosed before proceeding. A same-group concurrent mutation or recursive same-group poll is diagnosed without waiting; same-thread registration during a callback is supported. This test does not introduce a blocking lock across native callbacks. The restriction may reveal a previously unmeasured native concurrency path; if triggered, preserve the log for analysis rather than disabling the guard.

Startup requires the chapter/pending/cleanup parent, executable identity, cold state and exact hook signatures. Trampolines and private storage are allocated before a three-page protection/commit sequence. Commit failures attempt rollback, with distinct restored/rollback-failed diagnostics. Startup and runtime failures emit `CALLBACK_FATAL` and exit29C1. This is a controlled diagnostic exit, not proof of the original crash's exception type.

The build retains150 requests,160 graphics slots,ED+EF,chapter-independent supported-profile selection,indexed262144,resource collection131072,audio-model192/global2048/shared64MiB,cleanup262144 andpending512. Traffic weights, base IDs and audio pool sizes are unchanged. Audio native setup failures and parked-car fallback remain separate investigations.

## Diagnostics

Log: `%TEMP%\DSFV29s_callback512_<PID>.log`.

- `CALLBACK_START`: revision1/status1,21 groups,64->512,3 hooks, process lifetime and preserved sentinel.
- `CALLBACK_EVENT kind=redirect`: one per group at its first inline-storage exhaustion.
- `CALLBACK_STATUS`: invalid/full/guardBad/concurrent error counters.
- `CALLBACK_GROUP`: attempts/stores,redirects,recycled inserts,active peak,issued peak,current/recycled,polls/removals,depth and last handler/payload.

The monitor reports published per-group gauges once per second and never walks native chains or waits on callback execution. Fields may be sampled independently in flight. Peak or redirect above the native boundary establishes extension use; a quiet zero-error log alone does not prove that path was exercised. Neither occupancy nor150 request counts prove150 simultaneously drawable vehicles.

## Validation completed

`python scripts/test_callback_arena.py` passed using the exact production core, bridge and installer. It executes selected supported-executable native blocks in fixture-owned executable memory:

- Full005567D0 insertion implementation.
- Registration fragment00858FB5..00858FD4.
- Polling/callback/remove/recycle prefix00859000..00859073.
- Complete21-group initializer00DB40A0..00DB40DC.

Fixture-only changes rebase six absolute global operands and registration's CALL. The remaining native resource-handler state machine is replaced by a return stub. No actual game resource manager, mission, renderer or audio system runs.

Passed stock65th-overwrite reproduction; all21 lists simultaneously at512;513th refusal before native mutation; native callback retention/removal and256 recycled reinserts; full drain and refill; registration from inside a callback; exact writer fragment/selector check; unrelated-header passthrough; bad guard/cycle/free chain/reset/selector/concurrent/recursive-poll refusal; startup parent/cold/signature/allocation/protection/cache/rollback cases; and mid-function GP/flags/DF/x87/SSE preservation.

The first fixture attempt exposed a missing fixture-only relocation of registration's global-base operand. Correcting that remapping allowed the native fragment test to pass; no game run was used to discover or debug it.

`python scripts/verify_callback_arena.py` passed compiled hook table/bytes/boundaries, bounded incoming-branch checks, saved-state bridge, startup order/failure code,log markers,fixture/native byte identity,source digest correspondence and all inherited chapter/pending/cleanup/audio/indexed/29 startup checks. Chapter verification was parameterized with unchanged defaults to allow checking this derived artifact. The active solution contains one Release/Win32 project.

MSBuild's initial sandbox attempt failed in FileTracker with E_ACCESSDENIED. The authorized outside-sandbox rerun succeeded:28 inherited warnings,0 errors; no warning originates from the new callback files. Build log `build/v29s_callback512_r1/build.log`.

Evidence: `analysis/callback512_r1_2026-11-06/fixture_results.json`, `verification.json`, compiled/native disassemblies and inherited verification subtree. Reproduction scripts: `scripts/test_callback_arena.py`, `scripts/verify_callback_arena.py`. Supplied-run analyzer: `scripts/analyze_callback_log.py <explicit-log> --output <summary.json>`; it separates integrity, extension coverage and audio failures from user-visible stability.

## User test outstanding

Guide `dist/MAIN_150/CALLBACK512_R1_TEST_2026-11-06.md`: use only this traffic ASI, repeat Gone!/President's Run/Tanner's Day Off transitions and return the matching log with sequence/body/audio/FPS outcome. If successful, test normal menu return, reload and exit. No crash fix, workload headroom, real concurrency behavior or performance result is claimed before that run. A new crash dump is useful if a failure persists. Prior dump strongly supports callback overflow; the two newer CHAPTERS crashes were never fault-matched.

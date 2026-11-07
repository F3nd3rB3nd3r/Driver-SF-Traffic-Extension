# ARENA_REUSE_R1: scratch-frame correction, spooler owner reuse and exit logging

User authorized the repair after CALLBACK512 run23636 ended silently at its ninth setting request. New staged artifact:

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_ARENA_REUSE_R1.asi`

191488 bytes, SHA256 `879236089985E71AE7E601D74BFC8A9F5570E3679128EC8957B5CCFBB015B5F7`.

Built Release/Win32 through the single active ARENA_REUSE project in `DriverSF_V29s_NATIVE_TRACE.slnx`. Staged only; no installation, game run, debugger attachment or game asset writes. Previous CALLBACK512_R1,CHAPTERS_R1 andPENDING512_R1 artifacts are preserved.

## Finding that changes the repair

The existing baseline already changes13 instructions in005A7490, enlarging its frame from270h to720h and moving its local list sentinel fromESP+278h toESP+728h. The existing merge clone changes only its source offsets from264h/268h to714h/718h to consume that expanded list. Those changes precede the older external scratch helper.

The external helper nevertheless still considersESP+278h the allocation boundary. It therefore allocates a256-node external arena after50 nodes even though the actual native frame has room for150. Repeated fresh frames consume all eight registry entries. In an isolated native fixture, the exact old scratch adapter reaches exit2D on the ninth such invocation. This reproduces a concrete bug that fits the three supplied logs' terminal ninth request; the actual exit code of run23636 remains unrecorded, so attribution is not claimed as captured fact.

Simply reusing more scratch buffers is unnecessary for this150-request build. The repair uses the existing full150-node frame and validates its real header, cursor, count and sentinel before fresh writes. It allocates no external scratch arenas and refuses a151st fresh write. Startup verifies the installed13 frame replacements and the150-source merge clone before gameplay.

## Lifetime analysis

Bounded disassemblies are saved in `analysis/arena_reuse_r1_2026-11-06/`.

-005A74DC..005A7507 initializes the local list. Parent frame patches relocate its sentinel fields to728h/72Ch.
-The two append paths005A75DD..005A761A and005A7665..005A76A2 store one WORD payload and link one node per applicable request iteration.
-005A7762..005A776E passes the local source header to the second merge. The existing clone uses source+714h/718h while leaving the destination's264h/268h fields intact.
-005A7D50 copies WORD payload values into destination-owned nodes, retains/recycles existing destination nodes, and does not splice or publish the source-node addresses into the persistent list.
-005A7854..005A785E restores the frame and returns. Known clone005A6920 likewise copies payload values into another list rather than retaining source-node ownership.

Consequently the local scratch nodes expire with the reconcile frame, while the spooler destination's own nodes persist. A permanent arena can be reused for the same owner only when validation shows the current header's active and recycled chains contain native inline nodes and their fresh tail is inline. This prevents overwriting backing that the current header still references.

The new per-owner spool backing is retained for process lifetime. Distinct owner addresses never share it. When an owner header is genuinely reset at the same address, validated current native-only chains allow reattachment of that owner's previous arena. Native recycled chains are preserved: the replacement updates their **fresh tail**, rather than overwriting the free-list head. No native release or callback is skipped. This audit/fixture does not establish arbitrary game-wide concurrency or repair unrelated lifetime corruption.

## Implementation

New conditional flag `DSF_V29S_ARENA_REUSE` derives from CALLBACK512/CHAPTERS. Other variants retain the original allocation policy when rebuilt without this flag.

-`src/V29sArenaReuseCore.inc`: native/private ownership and chain validation, process-lived per-owner256-node backing, reset reattachment, inline150 scratch bounds checks. No fixed eight-owner registry. Node capacity and request count are not increased.
-`src/V29sArenaReuseBridges.inc`: replaces the three existing reconcile/scratch bridge implementations at their existing native hook sites; saves GP registers,flags,DF,x87 andSSE around diagnostic helpers.
-`src/V29sArenaReuse.inc`: startup verification,1-second published telemetry and file-backed legacy fatal logger.
-Conditional adapters in `V29sNativeLoadingTrace.inc` select the repair. Legacy native-loading guard exits now route through the file logger. Three legacy hash exits in the main source also route through it. The declaration/definition share the existing anonymous namespace.

No new hook sites. Existing150 requests/160 graphics slots,ED+EF,chapter profiles,callback512,audio-model192/global2048/shared64MiB,indexed262144,resource collection131072,cleanup262144 andpending512 remain in place.

The generated log is `%TEMP%\DSFV29s_arena_reuse_<PID>.log`.

`DSFV29s_ARENA_START` reports `spoolPolicy=ownerReuse fixedOwnerLimit=0 privateNodes=256 scratchPolicy=expandedFrame150 scratchExternal=0 framePatches=13 newHooks=0`. `ARENA_STATUS` publishes check counts, distinct spool owners, redirects/reset reuses, observed issued peak and scratch active count before insertion. `scratchPeak=149` can be normal for150 inserts. The monitor does not walk live native lists.

`DSFV29s_LEGACY_FATAL` records reason,exitCode,tick,owner,cursor,active/recycled counts,arena usage and guard/allocation errors before the original protective process exit. Callback/pending/cleanup/indexed guards retain their existing fatal logging. Chapter preflight already emits its startup failure status before its separate early exit. Native game exits and external modules' termination calls are not intercepted; absence of all fatal records still cannot prove a native exception.

## Completed validation

`python scripts/test_arena_reuse.py` passed. It compiles the exact production core,adapters,bridges and logging runtime into a Win32 fixture. Selected native append fragments have the same eight sentinel displacements as the installed frame patch; the native merge has the same two source-offset changes as the existing production clone. Other native services and the whole game do not run.

Verified:

-Exact old scratch adapter exhausts its eight arenas on the ninth50-boundary reset.
-64 sequential150-entry frames using both real native append paths, without external scratch allocations.
-Real native merge copies150 values into persistent spool storage; overwriting the local source frame does not affect destination values.
-16 native owner-header resets reuse one arena;16 distinct owners receive distinct process-lived backing beyond the old eight-owner cap.
-Native shrink/recycled refill and attachment with an existing native-only recycled chain preserve all nodes and links.
-151st fresh scratch write,bad cursor/count,guard failure and allocation failure are refused. Legacy fatal records contain their reason and exit code.
-Mutated frame startup state is refused.
-All three changed bridges preserve GP/flags/DF/x87/SSE despite a helper deliberately clobbering FX state.

`python scripts/verify_arena_reuse.py` passed compiled saved-state bridges,exact fixture/source/native-byte correspondence,removal of old fixed spool/scratch arrays from the repaired image,startup order/log markers and preserved parent hash. It also ran the complete inherited callback/chapter/pending/cleanup/audio/indexed and29 startup checks on this artifact. The callback verifier was parameterized while retaining its previous defaults.

Build succeeded with28 inherited warnings and0 errors; no warning originates from the new files. The first build caught a logger declaration placed outside the main source's anonymous namespace; moving it into the correct scope resolved it. Approved outside-sandbox MSBuild was used because this environment's FileTracker previously required it. Log: `build/v29s_arena_reuse_r1/build.log`.

Evidence: `analysis/arena_reuse_r1_2026-11-06/fixture_results.json`, `verification.json`,selected native/compiled disassemblies and inherited verification subtree. Fixture generation includes exact production adapter hashes so copied fixture logic cannot silently diverge. `scripts/analyze_callback_log.py` now recognizes arena startup/status and legacy fatal records alongside callback diagnostics.

## Outstanding live test

Guide `dist/MAIN_150/ARENA_REUSE_R1_TEST_2026-11-06.md`: at least six Challenge/free-drive cycles to cross the former ninth-setting boundary, then menu/reload/exit if stable. Return the matching arena_reuse log with order/body/audio/FPS/outcome. Callback capacity use is separately measured and may remain below64. Audio native setup failures and parked-car fallback remain unresolved; this repair does not claim to fix them or prove150 simultaneous drawable vehicles.

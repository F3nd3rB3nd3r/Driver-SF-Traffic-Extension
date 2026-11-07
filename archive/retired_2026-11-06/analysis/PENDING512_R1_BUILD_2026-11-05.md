# Pending-handler512 R1 fix and verification

Built at the user's request after Driver.dmp captured the streaming-list cycle. Staged `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_PENDING512_R1.asi`,160256 bytes, SHA256 `F96FC542561528B8F1CFA3D8F958D88F92A508EE372D5DB6F0392F077FC0409B`.

## Change

The missed package0 pending16-byte keyed map at section+5F5C has64 native slots. Its65th fresh node aliases sentinel+6368; the dump showed nodes extending into subsequent map headers and an infinite include-list cycle at005A237C. The completed map's existing expansion did not cover this pending map.

This build gives the pending map512 total slots (64 inline+448 external), using its existing generic007D7270 insertion detour. A guarded pending-specific path runs before ED/EF/completed-map handling; it covers calls through005A2740 and direct generic insertions, including005A231A. Native header and sentinel addresses stay fixed, as do find/remove operations and key/handler values. Existing completed/include storage is distinct and unchanged.

Each lifetime receives a separate7232-byte VirtualAlloc arena, first allocated only when the map is pristine (zero active/recycled, initial cursor, self-linked sentinel). Earlier arenas remain allocated through process exit. A constructor reset at the same owner address creates a new arena rather than clearing or reusing external nodes from a prior lifetime. Previously recognized owners remain recognizable when the current package pointer changes. No live migration is attempted, and nonempty unowned maps refuse execution.

Insertion checks the active list and free chain for owned/aligned issued nodes, unique membership, correct backward links, WORD-key ordering, counts and the native fresh tail. It validates position/output/data pointers, redirects only the first fresh allocation beyond64, preserves native recycled-node allocation, and checks native post-insertion results and guards. Capacity exhaustion refuses before a native write. It never drops an entry or repairs a cycle by skipping links.

Two new six-byte detours at005A2230 and005A2466 verify pending traversal entry/return and include-list integrity on the streaming thread. GP registers, flags/DF, x87/SSE and original entry/return instructions are preserved. Nonempty walks are paired; empty checks have separate counters. These bounded checks prevent an already corrupted list from becoming an unexplained endless traversal. The normal empty path avoids the full include walk. Monitoring accesses only owned arena guards and scalar counters.

The installer checks parent cleanup/identity/cold state and both exact native signatures before and after allocation/protection. It verifies installed bytes/cache/protection and restores its own sites if commit fails. Startup refusal or incomplete rollback is logged and terminates before continuing the test. Runtime integrity/capacity/allocation failures likewise produce `PENDING_FATAL`, exitcode29E1. This controlled diagnostic stop is not a claim that a menu transition succeeded.

## Preserved baseline

Audio192/global2048/shared default64MiB; cleanup262144;150 requests/160 slots/ED+EF/indexed262144; other native pools and existing sidecars remain. Previous ASIs are preserved. The previous CLEANUP262144_R1 staged hash was verified unchanged.

Active `DriverSF_V29s_NATIVE_TRACE.slnx` contains only `DriverSF_V29s_PENDING512.vcxproj`, Release/Win32. No deployment, game execution or debugger launch. Main-menu/reload/exit benefit is not yet observed in-game.

## Verification

`python scripts/test_pending_map.py` passes. Selected native insertion, constructor-reset and node-removal instructions execute in isolated fake packages with the actual production pending core, installer and bridges. The stock65th insertion reproduces sentinel/neighbor overwrite. The fixed path inserts512 nodes, refuses513 before writing, recycles256 native nodes and reinserts them, and preserves adjacent headers/guards. Real native entry/return instructions exercise production traversal bridges; the intervening fixture work uses the exact native pending-node removal fragment. The fixture does not pretend to run the entire engine traversal/handler callbacks.

Tests cover empty/nonempty/incomplete/completed walks, constructor/address reuse with old external bytes retained, separate owners and completed/pending maps concurrently, allocator failure, identity/parent/cold/signature refusal, protection/cache/rollback failure, and cycle/free-chain/header/guard rejection. Both bridges preserve GP registers, stack arguments, flags/DF and x87/SSE despite helper clobbering. A fixture adapter was corrected to treat the native boolean return as AL; this did not require a game-code change.

`python scripts/verify_pending_map.py` passes compiled table/bridge/trampoline/helper identities, native byte spans, instruction boundaries and no decoded branch into overwritten instruction interiors. It verifies the pending path is wired into MapInsertBridge before other map handlers, per-generation allocation size, fatal path and install-before-monitor order. Inherited cleanup/model192/reason/slots/audio/capacity/indexed verification also passes, including29 startup regression checks.

Release/Win32 MSBuild succeeds with the existing baseline warnings/exported-entry warning and no pending-source warnings. Final build log `build/pending512-build.log`. Execution outside sandbox was approved for the known FileTracker restriction.

Evidence: `analysis/pending512_r1_2026-11-05/fixture_results.json`, `verification.json`, inherited reports and native assembly. Sources: `src/V29sPendingMapCore.inc`, `V29sPendingMap.inc`, `V29sPendingMapBridges.inc`, `V29sPendingMapPatch.inc`; fixture/reproducers in `tests/PendingMapTest.cpp`, `scripts/test_pending_map.py`, `scripts/verify_pending_map.py`. Existing compiled verifiers were parameterized with unchanged defaults for this inherited variant.

## User test

Follow `dist/MAIN_150/PENDING512_R1_TEST_2026-11-05.md`: one traffic ASI, fresh process, same save, normal main-menu transition and multiple same-process reloads. Return `%TEMP%\DSFV29s_pending512_<PID>.log`. Require startup status1, no fatal/full/guard/invalid/allocation failure, a redirect or issued peak above64, and completed nonempty walk counters/depth after the menu transition. Review the run before increasing another limit or changing more lifecycle behavior.

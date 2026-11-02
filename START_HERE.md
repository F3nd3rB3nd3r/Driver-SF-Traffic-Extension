# Driver: San Francisco — V29s VS 2026 lean handoff
**Snapshot:** 2026-11-02 · **Active build:** V29s native request/dispatch/transfer trace · **state:** compiled and offline verified, NOT tested in-game.

## Open
- `DriverSF_V29s_NATIVE_TRACE.slnx` (VS 2026, v145, **Release|Win32**). This solution contains ONE project: 150 requests / 160 graphics+admission slots.
- Working source: `src/DriverSF_V29b_NativeFastPath.cpp` and `src/V29sNativeLoadingTrace.inc`; dependent .inc/.h files are in `src/`.
- Compiled **150** test ASI: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_TRACE.asi`.
- Optional matched **45** test ASI: `dist/CONTROL_45/DriverSF_V29s_160Capacity_45Request_TRACE.asi`. Use on a separate game launch, NEVER concurrently.
- Analyzer: `tools/analyze_v29s_run.py`; offline verifier: `verification/verify_v29s_native.py`. `reference/V29S_NATIVE_LOADING_TRACE.md` contains the detailed native trace **only if needed**.

## Known outcome (do not repeat old work)
- V29r **45 requests / 160 slots** reached playable city with visible cars, confirmed by the user. Completed map 52; collection at `0x0188A6B8` was 0; hashLeft ~2216; general release calls 8; spatial residency queries 26.
- Matched V29r **150 requests / 160 slots** stalled at loading with completed map 150, collection value 14836, hashLeft 0, extended hash allocations ~24428, EF extras 105, EF guards 0, zero observed general release/spatial residency calls. The 160-slot setting is therefore not alone responsible.
- V29q proved `00BAE790` lookup returns; V29r attributed ~100 calls/s in BOTH configurations to `0069926C`. Do not revisit that maintenance path without new evidence.
- Map entries, resource transfers, READY, and drawable residents are DIFFERENT measures. The byte gate at `[ECX+74DB9]` is not yet semantically classified.
- Hash spillover is not automatically hash allocation failure. `0x0188A6B8` is a collection length used by `00BAE790`, not an established request queue.
- Previous x32dbg long-pause exceptions are not the primary lead, per user experience.

## What V29s observes (no gameplay state changes)
Five byte-validated detours: `005A7490` reconciliation; `0059DB40` manager gate; `0059D740` dispatch; `0059D778` membership success; `0059BAD0` handler transfer/cleanup. No V29q high-frequency traversal hook or V29r caller hooks. All five passed static signature checks on retail `Driver.exe` SHA256 `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616` in the prior authoring environment. In-game runtime validation remains pending.

## Scope
Focus solely on native resource admission / request reconciliation / completion gating for the **150-vehicle** experiment. Traffic-table editing, DSF-Forza, splitscreen, V29i obsolete binaries, broad map research and rewriting the entire ASI are **out of scope** unless asked. Old evidence/history remains in the separate original full V29s package, not in this lean workspace.

## Current action
Read `NEXT_ACTION.md`. If the user has NOT supplied both V29s logs, do not start V29t or more hooks; provide the minimal test guidance and wait for the logs. If both are supplied, compare the first divergence and propose one causal next step.

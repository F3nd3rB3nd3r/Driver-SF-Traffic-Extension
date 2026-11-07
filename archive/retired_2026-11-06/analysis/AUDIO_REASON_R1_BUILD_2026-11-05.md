# Audio event reason/object probe R1 — 2026-11-05

Built at the user's request. Staged `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_REASON_R1.asi`, Release/Win32,130048 bytes, SHA256 `FCD56D9AD3FAA845ECFF75A86E4FD75288A8342B8B13453F44C11480B4661715`. Active single-project solution selects `DriverSF_V29s_AUDIO_REASON.vcxproj`. No game installation/run/debugger.

## Why measure16

The prior tests showed the global255/2048 managers and per-record8/255-user gates did not saturate. The game sound-object wrapper has a distinct16-entry outstanding-playing-event array. Its full check occurs before middleware PostEvent; the old probe therefore cannot count those refusals. This makes it a specific observation gap, not a proven cause or the sole focus of this test.

The count is at a field inside each sound object, not a single global counter. A shared or global game subsystem can own one of these objects, and a vehicle can use more than one; current code/logs do not establish a one-object-to-one-car mapping. Object IDs in this path are addresses, and pool addresses can be reused. The new observations identify original callers, requested event IDs, any names supplied through object string wrappers and registration counts to help establish those roles.

## Baseline preserved

No new capacity changes relative to SLOTS2048: object event limit16, global settings2048, shared default64MiB, original StaticBanksPool4372480B/platform3MiB, threshold0.9. Requests150/graphics160/ED+EF/indexed262144 and all previous diagnostics remain. Previous artifacts, including SLOTS2048 and working R2, were hash-verified unchanged.

## Ten new measurement sites

| Site | Observation |
|---|---|
| C5400B | Exact event-definition lookup result, ID and gameObject |
| C54057 | Selected playing-record creation failure, ID/object/result |
|84CC98 | Every sound-object wrapper request, count,16-entry refusal and original game caller |
|84CCC9 | Wrapper PostEvent result and pre-insertion count |
|84C9B0 | Delivered callbacks/type1 count, without dereferencing potentially stale cookies |
|84C920 | Validated-object callback handling, completion count and playing-ID match in the bounded local array |
|84CE5F | Selected native erase/decrement, owner and count before original instruction |
|84CD90 | General object narrow-string event wrapper input |
|84CDE0 | Default-argument object narrow-string event wrapper input |
|84C2F0 | Sound-object registration, exposing address reuse |

Lookup, creation and gate observations address separate hypotheses; the test does not assume16 is hit. Delivered callback entry only reads the callback type from its native stack. Object reads/walks are limited to native paths already using valid objects; callback matching scans at most16 records. Name capture is bounded to the first128 calls and63 bytes per record, checking readability and emitting hex. Hot callbacks perform no file I/O, middleware calls or new allocations.

The monitor reads only128 owner-stat entries,512 nonzero event-stat entries plus one zero-ID entry, and fixed detail buffers. Each table uses atomic key claims/counters; overflow is explicit and no native data is modified. Aggregates are updated on native paths and are not simultaneous cross-field snapshots. Owner rows aggregate reused address lifetimes, with registrations counted rather than claiming vehicle identity. Event rows retain first/last contexts, not every event/object combination. None-hash misses have a separate64-detail budget, preserving256 other missing-lookup records. Other bounds are64 creation failures,64 full refusals,128 wrapper zeros,64 unmatched completions and128 names.

`acceptedPeakProjected` is a nonzero wrapper result's pre-insertion count+1; the native append follows the hook. Direct native field reads populate `peakObserved`. Removal accounting occurs immediately before the original decrement, which the trampoline replays; the helper never performs it. Callback totals can differ because global entry, validated handler and selected removal are distinct stages and fields are sampled independently.

## Installation and validation

The installer requires the owned2048 baseline (status1, patched setting, nine hooks and exact owned signatures), executable identity and cold audio. It preallocates all ten trampolines, checks all stock signatures, protects the two unique code pages and repeats phase/signature checks before mutation. Only ten detour spans change; capacities remain untouched. Cache/protection failures roll back all new detours; failed rollback is explicitly status-2 and no potentially referenced trampoline is freed.

`python scripts/test_audio_reason.py` passed: optimized Win32 fixture includes production core/bridges/installer with fake memory and fault-injected OS calls. Checks cover baseline/cold/identity/signature refusal, all ten allocation and both pre-protection failures, late state changes, commit rollback/failed rollback, unchanged bytes outside hook spans, retained native16 compare, frame offsets, lookup/creation/gate/post classification, callback match/removal accounting, no native-state writes, none/menu name hashing, unreadable/null names, detail/table bounds and ten bridge invocations preserving native frame/arguments/GP/flags/DF/x87/SSE state.

`python scripts/verify_audio_reason.py` passed for the built ASI: ten new and nine inherited slot inventory rows; complete native instruction spans; no decoded incoming branch into stolen-span interiors; compiled helper/frame/kind/trampoline targets; unchanged native16 instruction and memory budgets; startup order. Inherited29 startup guards, six audio bridges, three capacity bridges,13 indexed rows,165 addressing checks and constructor/free-chain/exhaustion checks also passed. Existing source fixture hashes and prior staged artifact hashes match. Evidence: `analysis/audio_reason_r1_2026-11-05/fixture_results.json`, `verification.json`, `incoming_branch_candidates.json`, focused native disassembly and `baseline/`.

MSBuild Release/Win32 completed with approved escalation because its FileTracker previously failed under the sandbox. Log: `build/audio-reason-build.log`. Existing signedness/unreferenced-function/exported-entry warnings remain; no new reason-source warnings. No other configurations/solutions were built, no dependencies installed and no other game directories inspected.

## Runtime comparison

Use `dist/MAIN_150/AUDIO_REASON_R1_TEST_2026-11-05.md`. Require reason startup status1/hooks10/changedLimits0/objectEventLimit16 plus inherited setup/live sizes. Return `%TEMP%\DSFV29s_audio_reason_<PID>.log` into `dist/MAIN_150`, with working/silent-car order and approximate entry times. Compare direct lookup misses versus creation failures, actual full16 refusals, completion/removal behavior and real event/caller identities. A silent car with admitted real events and no16 refusal requires downstream action/voice/stream/switch/mixer tracing. No audio restoration is expected solely from this diagnostic addition, and no runtime benefit/overhead claim is made before that test.

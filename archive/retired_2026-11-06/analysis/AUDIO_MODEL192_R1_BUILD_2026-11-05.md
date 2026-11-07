# Combined model-audio192 expansion and census — 2026-11-05

Built at the user's explicit request to increase the verified45-entry registry to cover150 cars while measuring it. Chosen capacity192 leaves42 entries of headroom. Staged `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_MODEL192_R1.asi`,142848B, SHA256 `5BBB884F4C20AD24B099F012B2A4A9F036FE593446B9B8500B3B7EC6D6CB2FD1`. Release/Win32; active single-project solution selects `DriverSF_V29s_AUDIO_MODEL192.vcxproj`. No installation/game/debugger execution.

## Audited layout and implementation choice

The native provider object remains2F0B with its vtable, map header, sentinel2E0 and common-data pointer2EC unchanged. Constructor556EE0/factory557000 initialize the cursor to object+10; lookup556E90/finder556F10, removal556E50 and generic insertion7D7270 follow node pointers rather than requiring nodes to be inline. The selected destructor791990 resets the vtable and optionally invokes unsized object deletion; it does not independently free the map's inline node area. Common getter/setter retain2EC. No factory/constructor/getter/default/sentinel offsets or generic helper code are patched.

This permits a narrower expansion: preallocate eight192x16B external node areas with16B trailing guards (24704B total), then redirect only the native free-node cursor when registration first observes a pristine empty provider. Requirements: expected vtable, active/recycled0, cursor==object+10 and both sentinel links self-referential. Active entries are never migrated. Failed/late attachment keeps the stock45 gate and is explicitly logged. Existing providers with no registration yet may be observed as state0. Address reuse after a constructor reset can reattach the same process-lifetime storage with a new generation count.

Provider/node storage is retained until process exit, avoiding frees while native references might remain. Up to8 provider addresses are supported; overflow is explicit. Existing native mutation serialization is assumed, as before; this does not make the native container thread-safe. Stats use atomic counters/keys, and the monitor does not traverse live objects.

Registration556E30 originally includes a relative JAE. Its new branch bridge computes the exact unsigned capacity decision (192 only for owned external storage, otherwise45), reproduces CMP flag semantics and selects native556E36 or556E49. Thus native find/insert and pointer-store code remain in use. The ignored pushad saved-ESP slot carries a per-call continuation; no global branch target or caller stack mutation is used. A second explicit branch bridge reproduces the original TEST-AL/JZ bank-ready decision at553918. Raw trampolines are used only for spans without relative control flow; setup-return spans replay original pop/RET8 behavior.

## Eight new sites

| Site | Purpose |
|---|---|
|556E30 |capacity decision, empty-only external attachment, attempts/old45 gate/new full |
|556E43 |native store path, model/data pointer and post-insert count |
|556E80 |selected unregister/recycle decrement and model key |
|556EBB |model lookup pointer/null and primary/fallback call context |
|553860 |shared vehicle setup/original model/raw+5E7 |
|553918 |selected bank readiness/model/common-profile state, preserving branch |
|553ACF |shared setup true return |
|553AD5 |shared setup false return |

The census has512 nonzero model keys plus a separate zero key. It tracks original-model fallback results using the known5538EF return context and saved controller pointer; fallback1 is not confused with the original vehicle ID. All counts concern observed registrations/setup, not guaranteed audible output or unique simultaneous cars. Detailed budgets256/256/64/128/128/128/128 are independent; counters and key aggregates continue after detail saturation. Latest native counts can be pre-decrement snapshots.

## Preserved baseline

Retains previous reason probe10 hooks, global-slot probe9 hooks/settings2048, audio probe6 hooks/default64MiB, native16-event objects, original static/platform budgets/0.9 thresholds,5/7 controller pools and150 requests/160 graphics/ED+EF/indexed262144. Those are separate constraints. The previously reported45 request/control variants and160 graphics slots did not enlarge this newly identified45-profile audio registry. Prior staged reason/slot/R2 artifacts remain unchanged.

## Validation

`python scripts/test_audio_model_registry.py` passed with the production core/bridges/installer and an isolated copy of selected native constructor, registration, find/insert, lookup and unregister routines. The selected original native bytes are generated into `tests/AudioModelNativeBytes.inc`; only a test vtable immediate and the call crossing rebased code pages are retargeted. Driver.exe is never loaded/run, and the ASI is not loaded.

The native fixture actually registers192 nonconsecutive model keys, retrieves all192 supplied values, rejects a193rd key without altering map capacity, preserves full-map duplicate-update semantics, updates an existing key below full, removes/reuses external nodes and empties the map back to its self-linked sentinel. It verifies common-data preservation, provider boundary guards, external canaries, constructor/address reuse and refusal to migrate a preexisting nonempty45-entry object. These native tests exercise the production gate/store/remove/lookup bridges and helpers.

Other fixture checks cover baseline/cold/layout/signature refusal; all seven node/trampoline allocation failures; both pre-protection failures; late state/signature changes; cache/protection commit rollback and explicit failed rollback; unchanged bytes outside detour spans; setup/bank/fallback frame decoding; eight synthetic bridge ABI invocations with GP/x87/SSE/DF state, CMP/TEST flags and per-call continuations preserved.

`python scripts/verify_audio_model_registry.py` passed for the compiled142848B artifact: all eight native/compiled rows, bridge arguments/helper/trampoline or explicit continuation paths, selected native fixture-byte matches, no actual incoming branch into stolen-span interiors and unchanged native constructor/sentinel/default/generic insertion/destructor regions. One raw scanner candidate at55383F is a ModRM byte inside `CMP BYTE [EAX+20],0`, confirmed by decoding from5535C0; it is not an incoming jump. A raw E8 displacement/padding candidate is ignored unless it resolves to a valid compiled call target.

Inherited checks also passed: ten reason+nine slot bridges, six audio bridges, three capacity bridges,29 startup guards,13 indexed rows,165 addressing checks and constructor/free-chain/exhaustion cases. Passed inherited results are reused only for the identical built-artifact SHA256 when retrying a corrected scanner. Full evidence is under `analysis/audio_model192_r1_2026-11-05/`, including fixture/verification JSON, focused disassembly, branch candidates and inherited checks.

MSBuild Release/Win32 succeeded with approved escalation for the known FileTracker sandbox restriction. Log `build/audio-model192-build.log`; existing signedness/unreferenced-function/exported-entry warnings remain. No other production configuration/solution, dependency installation or game-file deployment occurred.

## User test

Procedure `dist/MAIN_150/AUDIO_MODEL192_R1_TEST_2026-11-05.md`; use exactly one traffic ASI and fresh process. Return `%TEMP%\DSFV29s_audio_model192_<PID>.log` into `dist/MAIN_150` after testing formerly silent models and known working models. Require new startup status1/hooks8 plus runtime registry state1/capacity192, valid guard/census counters and inherited startup/live sizes. A peak beyond45 and wouldStockFull>0 show use of the expanded path; audio restoration and model-specific lookup/setup improvements remain runtime questions. If attachment is refused or a registry stays45, do not call the expansion tested successfully for it.

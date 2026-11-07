# Combined192-entry vehicle-audio registry test and census

Use `DriverSF_V29s_160Capacity_150Request_AUDIO_MODEL192_R1.asi` as the **only enabled traffic ASI**, replacing the previous test while the game is closed. Start a fresh process. The artifact is staged here, not installed or run by the assistant.

This increases the newly identified **45-profile model-audio registry to192 entries**,42 above the150 requested-car roster. The larger node storage is attached only to a pristine empty registry. It preserves the native provider object/sentinel/common-data layout; changing only the old compare would be unsafe. It also records registration, lookup, fallback and vehicle setup outcomes across model IDs.

The prior global2048/shared default64MiB settings remain, as do original static/platform budgets,16 events/object,5/7 controller pools and150 requests/160 graphics/ED+EF/indexed262144. These are separate limits. The150 requested cars and160 graphics slots did not previously expand this45-entry audio registry.

## Run

1. Load the same save and let the150-car preload settle.
2. Try formerly silent vehicles such as Escalade/Zonda/370Z and one known working car. Check engine, tire sounds and horn. For the police Crown Victoria, label the horn input as **siren**, not a regular horn test.
3. Drive for a few minutes and report which cars/categories changed, car visibility and rough FPS. Approximate order is useful; no exact timing is required for the global registration census.
4. Wait a few seconds after final actions, then quit. Return `%TEMP%\DSFV29s_audio_model192_<PID>.log` into this `dist/MAIN_150` folder.

## Required checks

- `DSFV29s_AUDIO_MODEL_START revision=1 status=1 reason=armedColdEmptyOnly hooks=8 expectedHooks=8 stockCapacity=45 testCapacity=192 nativeLayoutPreserved=1 emptyOnly=1 activeMigration=0`.
- Existing reason status1/hooks10, global-slot status1/hooks9/testMax2048, audio probe status1/hooks6, default-pool status1/testRequestedBytes67108864 and indexed status1/13 patches/31 redirects must also pass. Later baseline/indexed statuses should become2.
- A registry with registration attempts should show `DSFV29s_AUDIO_MODEL_REGISTRY state=1 capacity=192 external=<nonzero>`; `generations` at least1 and `guardBad=0`.
- MODEL_STATUS should show `attachRefused=0 invalid=0 instanceOverflow=0 keyOverflow=0`. A state-1 registry remains at45 because it was not safe to attach; that is not a successful192 test for that instance. State0 can mean an observed lookup-only instance has not registered anything yet.
- A peak above45 demonstrates actual use beyond the old size. `wouldStockFull` counts attempts for which the original count>=45 gate would skip registration; it includes updates of existing keys, not necessarily distinct rejected cars. `full` is the new192 ceiling and should remain0 for the intended roster.

## New output

`AUDIO_MODEL_REGISTRY` prints per-provider attempts, stores, full/wouldStockFull, removals, lookup hits/misses, peak/latest native count, external-storage state/generation and guard status. Up to8 provider addresses can be observed; storage is preallocated, not allocated per event. Native counts can fall when profiles unregister. Latest count can be a pre-decrement snapshot, so use peaks and explicit remove counters appropriately.

`AUDIO_MODEL_KEY` aggregates by32-bit model ID: registration/store/refusal, lookup and fallback outcomes, setup attempts, bank readiness and setup success/failure. `disabled` means raw vehicle+5E7 was0 at entry; the flag's exact semantic name is not established. `lastProfile=0` can identify null supplied profile data, even if registration occurred. `lastSelected` shows a selected/fallback key when bank readiness was checked. Setup success is not proof of audible output.

`AUDIO_MODEL_DETAIL` bounded records use:

| Kind /bound | owner | model | a (hex) /b /c /d |
|---|---|---|---|
|registerAttempt /256 |provider |requested key |profile pointer /pre-count /effective capacity /full flag |
|registeredStore /256 |provider |requested key |profile pointer /post-insert count /capacity /0 |
|storage /64 |provider |0 |192 or45 /external or native cursor /common pointer or refusal count /0 |
|lookupMissing /128 |provider |requested key |caller /active count /0 /0 |
|fallback /128 |provider |original vehicle key |fallback key /profile pointer /caller /0 |
|bankCheck /128 |vehicle controller |original vehicle key |selected key /ready flag /model-profile pointer /common pointer |
|setupResult /128 |vehicle controller |original vehicle key |success1 orfailure0 /raw+5E7 /0 /0 |

An accepted store is observed just before the native pointer write, which immediately follows; no helper writes the profile pointer itself. Removal is observed before the original native count decrement. Detailed buffers stop at their bounds while counters/model-key aggregates continue. The monitor only reads scalar census tables and its owned storage guards; it never walks live game profile lists. Independent fields may advance while a line is written.

192 live registrations/lookups,193rd refusal, removal/reuse, duplicate updates and sentinel/default/guard preservation passed in an isolated native-code fixture. Startup/rollback/ABI and inherited compiled checks also passed. Runtime audio, real model counts and FPS remain untested.

Release/Win32,142848 bytes. SHA256 `5BBB884F4C20AD24B099F012B2A4A9F036FE593446B9B8500B3B7EC6D6CB2FD1`.

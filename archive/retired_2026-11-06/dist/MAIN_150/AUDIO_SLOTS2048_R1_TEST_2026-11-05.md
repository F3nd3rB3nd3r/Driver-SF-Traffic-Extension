# Combined audio2048 capacity and count test

Use `DriverSF_V29s_160Capacity_150Request_AUDIO_SLOTS2048_R1.asi` as the only enabled traffic ASI in the game plugins folder. Start a fresh game process after replacing the previous traffic ASI. This artifact is staged here; it was not installed or run by the assistant.

This test raises both global audio manager settings from255 to2048 before native initialization. Native initialization sizes their three pointer arrays accordingly. It retains shared default64MiB, original StaticBanksPool4372480B, platform3MiB and the0.9 pressure thresholds. Requests150, graphics160, ED/EF and indexed262144 stay as before. The per-record8/255 limits are measured, not increased. These managers are not established to represent banks or physical audio voices.

1. At the initial menu, check menu sound effects.
2. Load the same save and drive the same route. Compare a previously silent Zonda/370Z/Bentley with a working Crown Victoria/Scirocco/C10/Cadillac when available. Check engine, tires and horn, plus menu sounds after loading.
3. Note any changes to car visibility or rough driving FPS. Quit normally after several minutes so the last counters are written.
4. Copy `%TEMP%\DSFV29s_audio_slots2048_<PID>.log` into this `dist/MAIN_150` folder and report which cars/categories worked.

## Required validation

The new startup line must read `DSFV29s_AUDIO_SLOTS_START revision=1 status=1 reason=armedBeforeAudioInit patches=1 hooks=9 expectedHooks=9 stockMax=255 testMax=2048`. Existing indexed startup must have status1/13 patches/31 pending redirects; existing AUDIO_START must have status1/hooks6; AUDIO_LIMIT_START must have status1/patches1/testRequestedBytes67108864. Later baseline startup/indexed status should become2.

Each observed initialized `DSFV29s_AUDIO_SLOTS` list (A0, A1, B) should report `configuredMax=2048 allocatedSlots=2048`, with invalid0. An owner0 row means that list has not yet been observed, not that its capacity was validated. `AUDIO_PRESSURE` must show configuredBytes67108864 and thresholdBits3F666666. A failed new startup or different live maximum makes the capacity comparison invalid; supply the log before another change.

## New measurements

- `nativeUpdateCount` is the last count captured at native update entry; it can fall as records are removed. `updateSamples` proves coverage, `updatePeak` retains its maximum.
- `peakBefore` is the largest count at a native allocation gate. `peakInserted` is the largest count at an actual insertion. These are not projected allocations or vehicle counts.
- `ceilingHits` counts native count>=configured maximum checks: manager B refuses there; manager A enters fallback selection. `selectedReuses` counts actual A candidate selections and can also occur after allocation failure below the ceiling; it does not mean every selected record was successfully reinserted.
- `DSFV29s_AUDIO_SLOT_USERS` records separate eight-user and byte255-user pre-attachment peaks/full gates. A byte attempt is counted after its native duplicate check; the eight-user gate precedes its duplicate check. A full gate does not by itself prove audible muting.

Counters are captured on native audio paths and logged about once per second. The monitor only reads scalar counters; it never walks live middleware arrays. Fields may advance independently while a line is written. There are no hot-path file writes or unbounded detail buffers. No runtime audio or FPS result is claimed yet.

Release/Win32,116224 bytes. SHA256 `C0210BF47A3C61B7B9A547D0AB30A359D9D991D6BBD153F96F35952A3ABFEFCC`.

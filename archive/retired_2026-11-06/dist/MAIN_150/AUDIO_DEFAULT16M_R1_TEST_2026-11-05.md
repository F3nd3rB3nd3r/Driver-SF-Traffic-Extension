# Audio default-pool16MiB test

ASI: `DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT16M_R1.asi`  
Release / Win32;108544 bytes; staged only  
SHA256: `4106331601430E38A3BCB888DE0F5FC310B47CE9597EFFFE128742EC786D3535`

This changes **one audio allocation request** from5872025B (about 5.6 MiB) to
16777216B (16MiB). It keeps the native 90% pressure safeguard. Static analysis
found playback stop/refusal paths at that threshold; your prior log remains
above it after loading. That can happen before any allocation fails.

The working visual configuration remains150 requests /160 slots /ED+EF /
indexed262144 with existing pending/hash expansions. Other audio pools, emitter
and queue counts remain native. Both prior ASIs are preserved for comparison.
Driver.exe on disk is untouched. Audio restoration is not yet confirmed.

## Test once from a fresh process

1. Close the game. Disable your currently enabled traffic ASI, including the
   audio-probe R1, by giving it a non-`.asi` extension. Copy this new ASI into
   the plugins folder. Keep **exactly one traffic ASI enabled** and retain the
   loader/unrelated plugins used for run4188.
2. Start normally without a debugger. Exercise initial menu sounds for10 seconds.
3. Load the same save. Test pause-menu sounds, engine, tires, horn, music and
   the previously surviving Shift effects. Drive the same route for2–3 minutes,
   including the state where those effects also disappeared if reproducible.
4. Note which audio categories work, when any stop, body loading and rough
   driving FPS. Exit and copy the complete
   `%TEMP%\DSFV29s_audio_default16m_<PID>.log` into `dist/MAIN_150`.

## Successful installation and live-size checks

Require the following fields (other fields can intervene):

```text
DSFV29s_INDEXED_START status=1 ... startupCheckRevision=2 stockPreflight=1 ownedHookVerified=1 patches=13 ... pendingRedirectPatches=31
DSFV29s_AUDIO_LIMIT_START revision=1 status=1 reason=armedBeforeAudioInit patches=1 stockRequestedBytes=5872025 testRequestedBytes=16777216 thresholdBits=3F666666 thresholdChanged=0
DSFV29s_AUDIO_START revision=1 status=1 reason=armed hooks=6 ... audioCapacityChanges=1
DSFV29s_CAP_START hooks=3 ... request=150 slots=160 ... capacityChanges=1
```

After initialization, require `AUDIO_PRESSURE` rows with `patchStatus=1`,
`configuredBytes=16777216`, `thresholdBits=3F666666`, and usableBytes roughly
16MiB (native metadata reduces it slightly). Check `aboveThresholdSampled` and
`pressureSamples`: zero would show that the sampled pool stayed below this
gate. Actual playback decisions are not directly hooked.

Inherited `CAP_STATUS` should reach `startupStatus=2 graphicsSlots=160`,
`requestLast=150`, `indexedStatus=2 indexedSlots=262144 headValid=1`, and
`fileStatus=0`. These are setup checks, not drawable-body counts.

Negative `AUDIO_LIMIT_START status` means this is not a valid expanded audio
comparison. Status-1 is ordinary refusal/restored commit failure; status-2 is
an unsuccessful rollback. Exit and return the complete log before changing
another limit. The installer refuses already initialized audio and checks all
original instruction/threshold bytes; it does not modify a running pool.

## How to interpret the result

- Audio restored with healthy pressure rows supports default-pool pressure as a
  cause. Compare all sound categories, because earlier45-vehicle tests had a
  narrower engine/tire problem that may be separate.
- Audio still absent while pressure remains below threshold means this change
  is insufficient. The log retains bank results, event zeros, emitter counters
  and pool snapshots for the next targeted investigation.
- New pressure despite16MiB calls for examining actual growth/lifetime before
  another increase. Do not change additional limits for this comparison.

Offline checks passed:20 production patch/rollback scenarios,15 isolated native
pressure checks,29 R2 startup checks and the compiled indexed/audio/capacity
verification. Full report: `analysis/AUDIO_LIMITS_AND_DEFAULT16M_2026-11-05.md`.

# StaticBanksPool 64 MiB comparison

**Clarification:** this earlier ASI changes StaticBanksPool, not the shared
default pool. For the user's clarified shared-default64MiB request, use
`AUDIO_DEFAULT64M_R1_TEST_2026-11-05.md` and the `DEFAULT64M` ASI instead.

ASI: `DriverSF_V29s_160Capacity_150Request_AUDIO_STATIC64M_R1.asi`  
Release / Win32; 112640 bytes; staged only  
SHA256: `1C36DBAD10B092555D8B3253D6BF7B93879C2F8AD8F3C24AC4B21284707CABD8`

This is the requested 64 MiB headroom test for **StaticBanksPool**, the pool
that reached 97% in run6736. It grows from 4372480 bytes (about4.17MiB) to
67108864 bytes (64MiB). It adds about59.8MiB of requested backing storage.

The improved shared default pool stays at16MiB. The platform pool stays at3MiB.
The native90% playback policy stays enabled. That policy checks the default and
platform pools, not StaticBanksPool: its97% usage is not proof that the policy
was activated. This comparison tests whether extra static-bank headroom helps.
It does not assume particular cars are intentionally excluded or promise a fix.

Requests150 /graphics160 /ED+EF /indexed262144 and the existing six audio hooks
remain. Previous ASIs are preserved; Driver.exe on disk is untouched.

## Run from a fresh process

1. Close the game. Disable your current traffic ASI by giving it a non-`.asi`
   extension, then copy this new ASI into the plugins folder. Keep **exactly one
   traffic ASI enabled** and keep the same loader/unrelated plugins as run6736.
2. Start normally without a debugger. Test initial menu sounds, then load the
   same save and use the same route.
3. Test the **Zonda and Nissan370Z** for engine, horn and tire sounds. Also test
   at least one previously working car: Crown Victoria Cop Car, Scirocco or C10
   tow truck. Note the order, any delay before audio starts, and whether a sound
   works initially then disappears. Include music/menu/Shift observations and
   rough driving FPS. Drive for about3–5 minutes if possible.
4. Exit and copy the entire `%TEMP%\DSFV29s_audio_static64m_<PID>.log` into
   `dist/MAIN_150`, together with the observations.

## Required markers

Require both allocation patches, plus inherited setup success:

```text
DSFV29s_AUDIO_LIMIT_START revision=1 status=1 ... patches=1 ... testRequestedBytes=16777216
DSFV29s_AUDIO_STATIC_START revision=1 status=1 reason=armedBeforeAudioMemoryInit patches=1 stockRequestedBytes=4372480 testRequestedBytes=67108864 ... defaultRequestedBytes=16777216
DSFV29s_AUDIO_START revision=1 status=1 ... hooks=6 ... audioCapacityChanges=1
```

`audioCapacityChanges=1` is a boolean indicator. There is one retained default-
pool patch and one additional static-bank patch, each reported separately.

After audio initialization, `AUDIO_STATIC_POOL` must show `patchStatus=1`,
`requestedBytes=67108864`, `blocks=65536`, `blockBytes=1024`, and usableBytes
roughly64MiB (allocator metadata reduces it slightly). `playbackGateAppliesToThisPool=0`
keeps the distinction from default-pool pressure explicit.

The existing `AUDIO_PRESSURE` rows must still show default configuredBytes16777216
and thresholdBits3F666666. Require successful indexed revision2/13 patches,
pending redirects31, audio hooks6 and capacity hooks3. Later `CAP_STATUS` must
show requests150, graphics160, indexed262144, valid heads and fileStatus0.

Negative `AUDIO_STATIC_START status` means the additional patch did not install
successfully. Status-1 is ordinary refusal/restored failure; status-2 indicates
failed restoration. Return the full log. The stricter guard requires an untouched
engine root and audio memory manager before their initialization; the test never
resizes an initialized pool. A failed static patch can leave the working default16MiB
baseline active, but that is not a valid64MiB comparison.

If this restores the missing cars, the comparison supports static-bank headroom
contributing. If it does not, compare actual usage, bank results and failed event
IDs before changing another setting. None of these markers count drawable cars.

Offline checks passed:29 production static-patch startup/rollback scenarios,
29 R2 startup checks, compiled audio/capacity/indexed checks, and native dynamic
allocation/block-count/heap-size verification. Real audio and allocation remain
untested until your run. Report: `analysis/AUDIO_STATIC64M_R1_BUILD_2026-11-05.md`.

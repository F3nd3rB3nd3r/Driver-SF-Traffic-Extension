# Shared default audio pool64MiB test

**This is the corrected SHARED DEFAULT pool test.** The earlier `STATIC64M`
ASI increased a different pool. Use this `DEFAULT64M` ASI for your clarified
request.

ASI: `DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT64M_R1.asi`  
Release / Win32;109056 bytes; staged only  
SHA256: `6EC8CA1FFE69452DFC3E69E9C1E158D1F70109F0CFBFEFCAF5C601C9770B1167`

| Pool | This test |
|---|---:|
| **Shared default** | **64MiB** (67108864B) |
| StaticBanksPool | Original4372480B (about4.17MiB) |
| Platform | Original3MiB |

The90% pressure thresholds remain. Compared with run6736's working default16MiB
configuration, only the shared allocation increases,16 to64MiB. Compared with
the previous `STATIC64M` experiment, the static expansion is omitted. A fresh
process starts with the original static-bank instruction; no disk Driver.exe
patch or manual restoration is required.

Requests150 /graphics160 /ED+EF /indexed262144, existing pending/hash expansions,
bank/event diagnostics and default-pressure monitoring remain. Previous ASIs
are preserved. The remaining audio cause and audible benefit of this64MiB
comparison are not established yet.

## Run from a fresh process

1. Close the game completely. Disable the current traffic ASI, including
   `STATIC64M`, by giving it a non-`.asi` extension. Copy this `DEFAULT64M` ASI
   into the plugins folder. Enable **exactly one traffic ASI**; retain the same
   loader/unrelated plugins.
2. Start normally without a debugger. Test menu audio before loading the same
   save. Drive the same route for3–5 minutes if possible.
3. Check Zonda, Bentley Arnage T and Nissan370Z against working Cadillac DTS,
   Crown Victoria Cop Car, Scirocco or C10 tow truck. Record engine, horn and
   tire sound separately, whether delayed sounds recover, car-test order and
   rough FPS. Include music/menu/Shift observations.
4. Exit and return the complete
   `%TEMP%\DSFV29s_audio_default64m_<PID>.log` into `dist/MAIN_150`.

## Required markers

```text
DSFV29s_AUDIO_LIMIT_START revision=1 status=1 ... patches=1 stockRequestedBytes=5872025 testRequestedBytes=67108864 thresholdBits=3F666666 thresholdChanged=0 changedPool=sharedDefault
DSFV29s_AUDIO_START revision=1 status=1 ... hooks=6 ... audioCapacityChanges=1
```

After initialization, `AUDIO_PRESSURE` must report `patchStatus=1`,
`configuredBytes=67108864`, thresholdBits3F666666 and usableBytes roughly64MiB.
The common pool should have1048576 blocks of64B. Match StaticBanksPool's ID
from `AUDIO_STATE` to `AUDIO_POOL`: it should retain4270 blocks of1024B and
usableBytes4369300. There should be **no `AUDIO_STATIC_START` expansion marker**
in this variant.

The startup's stockRequestedBytes5872025 refers to the original executable;
the comparison baseline is the16MiB test, not that original5.6MiB setting.

Also require inherited indexed revision2/stock+owned-hook success/13 patches,
pending redirects31, capacity hooks3 and later requests150/graphics160/
indexed262144/headValid1/fileStatus0.

A negative audio-limit status is a refused/restored-failure test; return the log.
The64MiB installer refuses a modified static-bank budget to avoid combining
experiments. Existing cold/identity/signature checks and rollback remain. The
test cannot resize an already initialized pool, so do not inject/reload it into
a running process.

Offline checks passed:22 shared64 patch scenarios,20 shared16 regression
scenarios,15 native-pressure cases per mode,29 R2 startup checks and compiled
audio/capacity/indexed checks. Real startup, audio and FPS still need your test.
Report: `analysis/AUDIO_DEFAULT64M_R1_BUILD_2026-11-05.md`.

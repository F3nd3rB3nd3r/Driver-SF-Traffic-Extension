# Audio probe R1

Built Release / Win32 on 2026-11-05. Staged only; not installed or game-tested.

ASI: `DriverSF_V29s_160Capacity_150Request_AUDIO_PROBE_R1.asi`  
Size: 104448 bytes  
SHA256: `A9F5AB4720068F3424D159D8675DE2EE7D16A1018A6B566626AF645F69522975`  
Supported Driver.exe SHA256: `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`

This measures the missing audio; it does not increase audio limits or promise
an audio fix. It preserves the working R2 indexed expansion (262144), 150
requests / 160 graphics slots, ED/EF and existing pending/hash expansions.
The previously tested R2 ASI remains unchanged.

## Run from a fresh process

1. Close the game. Disable your current traffic ASI by changing its extension
   away from `.asi`, then copy this probe into the game plugins folder. Keep
   exactly **one traffic ASI enabled**. Leave the existing loader and unrelated
   plugins as in your successful R2 tests.
2. Start normally without a debugger. At the initial menu, exercise menu sounds
   for about 10 seconds before loading the same save.
3. After loading, try the pause menu, horn, engine and tire sounds. Drive for
   about two minutes. Note when audio disappears, what remains audible, whether
   vehicle bodies load properly, and rough driving FPS.
4. Exit and copy the complete `%TEMP%\DSFV29s_audio_probe_<PID>.log` into
   `dist/MAIN_150`. Return that log with the observations. Include startup lines;
   a startup refusal or crash log is useful too.

## Required markers

Require these fields (other fields can appear between them):

```text
DSFV29s_INDEXED_START status=1 ... startupCheckRevision=2 stockPreflight=1 ownedHookVerified=1 patches=13 ... pendingRedirectPatches=31
DSFV29s_AUDIO_START revision=1 status=1 reason=armed hooks=6 expectedHooks=6 ... audioCapacityChanges=0
DSFV29s_CAP_START hooks=3 ... request=150 slots=160 ... capacityChanges=1
```

Later `CAP_STATUS` rows should show `startupStatus=2 graphicsSlots=160`,
`requestLast=150`, `indexedStatus=2 indexedSlots=262144 headValid=1`, and
`fileStatus=0`. There should be repeated `AUDIO_STATUS` rows, then `AUDIO_STATE`
and `AUDIO_POOL` rows after audio initialization. Readability can be zero before
initialization; that alone is not an error. Negative startup status or absent
monitor rows means this is not a valid full probe run; return the log before
making another change. Do not treat map counts or these markers as drawable
vehicle counts.

`capacityChanges=1` refers to the preserved indexed expansion.
`audioCapacityChanges=0` explicitly confirms the audio configuration is unchanged.

## What the log measures

- `bankCallbacks` and `bankResult1`: actual callback result at original ESP+08,
  with bank ID, signed pool ID, job cookie, input size and pointer kept separate.
  Detail values are eight-digit **hexadecimal** without a `0x` prefix.
  The old trace's `cbRaw` was a pool ID and must not be compared as a result.
- `emitterAttempts`, `emitterPeakBefore`, `emitterRefused`: attempts at the native
  90-emitter gate. The sampled emitter and deferred-cleanup lists are separate
  from bank queues and middleware voice counts.
- `mallocFailed` / `malignFailed`: null returns for nonzero-size requests,
  including raw pool ID, requested bytes, caller and alignment. Successful
  allocations bypass the C++ diagnostic helper.
- `postNormalReturns`, `postZero`, `postEarlyZero`: numeric PostEvent results.
  Normal nonzero returns are normal returns minus normal zeros. Nonzero means
  event admission, not audible playback. Failure details include event ID,
  game-object ID and caller; early external-source failures have their own count.
- `AUDIO_POOL`: current descriptor bytes, fixed-block counts and sampled peaks.
  For `fixedBlocks=1`, use free/total blocks rather than interpreting
  `mallocUsedBytes=0` as unused. Match named pool IDs from `AUDIO_STATE`.
  Pool generation changes on observed inactivity or changed table/backing/
  attributes/size. Reuse entirely between samples can be missed.
- `AUDIO_STATE`: raw initialization and volume/fade values; these are evidence
  to compare across loading, not an automatic declaration of mixer muting.

Counters continue after detailed storage fills. First-record bounds are 512
bank callbacks, 64 emitter refusals, 64 Malloc failures, 64 Malign failures,
128 normal PostEvent zeros, and 64 early PostEvent zeros. Details are not a
complete event history. Status is emitted about once per second; pool/state
details about every five seconds, with pool peaks sampled about every 200 ms.
Snapshots are concurrent and best-effort, with identity/header rechecks.

This first probe does not hook CreatePool, fixed-block GetBlock failures, voice
rendering, decoding or mixer output. A full descriptor table alone does not
prove a failed CreatePool. Healthy bank/event results cannot prove audible sound.

## Installation refusal and offline checks

The six signatures and R2 startup must pass before installation. All trampolines
and four writable code pages are prepared before diagnostic jumps are written.
Ordinary preflight refusal leaves no new audio jumps. A commit/cache/protection
failure attempts restoration. Audio status -1 is refusal/restored failure;
status -2 (`osRollbackFailed`) means OS restoration was unsuccessful: exit and
return the log. Trampolines are retained if a jump might remain.

Offline verification passed: six compiled signature/argument/helper targets,
near and short branch interior scans, 10 isolated Win32 invocations using the
actual production bridges, 29 R2 startup checks, all 13 indexed patch rows,
constructor/free-chain simulation and 165 indexed address checks. The harness
checks return values, stack cleanup, DF/CF, ECX/EDX and x87/SSE state. It never
loads the ASI or executes Driver.exe. OS failure paths were not fault-injected;
runtime installation, audio and performance still require this user test.

Evidence: `analysis/audio_probe_r1_2026-11-05/`; report:
`analysis/AUDIO_PROBE_R1_BUILD_2026-11-05.md`.

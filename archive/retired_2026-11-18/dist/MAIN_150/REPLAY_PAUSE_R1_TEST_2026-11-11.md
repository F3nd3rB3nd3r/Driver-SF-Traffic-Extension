# REPLAY_PAUSE_R1 — paused replay loading test

Staged ASI:`DriverSF_V29s_160Capacity_150Request_REPLAY_PAUSE_R1.asi`.
Release/Win32,244224 bytes, SHA256
`2847467ed26424b6c17773b2c0bd179b6218fbca1f019dc97a50feb86e2c083f`.

Includes the full REPLAY150_R1 snapshot correction and its parked/audio/
streaming/cleanup parents. This test adds one game-thread hook for the loading
completion that was stranded behind a paused simulation clock in run8944.
It does not change vehicle requests, graphics slots, weights, assets or pools.

## Setup and sequence

Use exactly one traffic ASI in the game's loader folder: this file instead of
REPLAY150_R1/PARKED150_R1 or another traffic test. Keep the AI asset/settings
used in the failing run. The agent staged the file; it has not installed it
or launched the game.

1. Repeat the failing sequence: enter Story, drive, then enter Replay once.
   If the previously failing recording is still available, test it too.
   Recordings can persist across restarts; deleting them is not required.
2. Once Replay loads, check play, pause and resume, and seek if normally used.
   A recording opening paused is acceptable; it should finish loading and
   remain controllable. The mod does not force playback to resume.
3. Leave Replay, return to Story, then enter Replay again. Also try a fresh
   recording, and a main-menu/Story reload before a further replay entry.
4. Check car rendering/audio and parked variety, then quit normally.

Note whether loading completed, playback controls worked, where any stall
occurred, whether it used a retained/fresh recording, and any regression.
One clean run without a recovery event may mean the timing issue did not occur;
it does not independently prove the new service was exercised.

## Log

`%TEMP%\DSFV29s_replay_pause_<PID>.log`; copy it into`dist/MAIN_150`.
If it stalls again, keep that log and provide the matching Task Manager dump.

Startup must include:

```text
DSFV29s_REPLAY_PAUSE_START revision=1 status=1 hooks=1
```

The full line also reports`site=0073A090 completion=007390BE`, import/thread/
native-readiness guards,`forceUnpause=0`, and requests150/graphics160. All parent
startup markers, including`REPLAY150_START status=1 hooks=7`, remain required.

`REPLAY_PAUSE_EVENT kind=completed` means the original completion block cleared
loading through the guarded service. It records import epoch, game thread,
owner, simulation, loading/lifecycle, pause/rate and replay clock before/after.
`kind=nativeHeld` means the native readiness check retained loading; the service
will retry on a later update while its other guards remain valid.
`kind=ownerChanged` records a lifetime/owner transition during native callbacks.
Native callbacks may change their own game state; the mod does not directly
write simulation pause/rate or advance the replay clock.

`REPLAY_PAUSE_STATUS` counters show update coverage, tagged imports, candidates,
attempts/completions, native-held results, and guard/reentrancy skips. If loading
stalls with missingImport/snapshotSkipped rising, those counters explain why
the service declined to run. Individual in-flight counter differences are not
a leak or integrity failure.

Snapshot events must still preserve the full150 roster/hash and181 for the
canonical150 preset. Valid18-car/custom subsets need not include181. Existing
recordings already containing the earlier corrupt model0 cannot automatically
recover their missing ID.

Startup signature/commit failures explicitly log`REPLAY_PAUSE_FATAL` and exit
code`0x29F5`. Runtime guard misses skip recovery; they do not force an exit.
The separate null simulation-manager Lua pause crash remains outside this fix.

After the run:

```powershell
python -B scripts/analyze_baseline150_log.py dist/MAIN_150/DSFV29s_replay_pause_<PID>.log --output analysis/replay_pause_<PID>_baseline.json
python -B scripts/analyze_callback_log.py dist/MAIN_150/DSFV29s_replay_pause_<PID>.log --output analysis/replay_pause_<PID>_callback.json
```

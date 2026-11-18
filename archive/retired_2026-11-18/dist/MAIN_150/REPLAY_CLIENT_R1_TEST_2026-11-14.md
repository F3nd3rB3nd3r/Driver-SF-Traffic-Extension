# REPLAY_CLIENT_R1: identify the busy replay subsystem

Use `DriverSF_V29s_160Capacity_150Request_REPLAY_CLIENT_R1.asi` as the
**only traffic ASI**, replacing the enabled REPLAY_HITCH_R1 traffic ASI.
Keep the same installation, AI file and settings as run28136.

This diagnostic retains the R2 fixes,150 requests/160 slots, parked/audio/
snapshot/lifetime fixes and all capacities. The broad timing hooks are
disabled. Three narrow native hooks identify the four possible busy replay
clients and capture recorded sample/buffer state at busy boundaries.

## Test

1. Play Story for about3–5 minutes to make a fresh replay.
2. Watch that replay at normal1x speed through its end, especially the final
   20–30 seconds. Avoid seeking, rewinding or changing speed before the
   choppy/loading-indicator section is captured.
3. Note roughly where the bursts begin and whether the loading indicator
   still blinks. Then return to the menu, wait about3 seconds and quit
   normally. If testing rewind afterward, mention that separate segment.

Copy `%TEMP%\DSFV29s_replay_client_<PID>.log` into `dist/MAIN_150`.
Report whether the choppiness still occurs, approximate replay time and FPS,
and how the session ended. If startup fails, retain that attempt's log.
No debugger or memory dump is needed for this initial capture.

## What changes in this probe

The ordinary all-clear path performs no new C++ observer call, QPC query,
readability scan or floating-point save. Busy-to-clear boundaries record:

- Client identity and its existing native result.
- Current, next and following sample pointers/timestamps.
- Recording backend, ring capacity/free space/cursors, pending bytes and
  end flag. No additional native readiness/I/O methods are invoked.

New records enter a bounded queue and are written by the existing monitor.
Long busy states produce at most a one-second heartbeat per client/thread.
Existing parent logging remains. Actual probe overhead and playback outcome
still need live verification.

## Verified artifact

- Release / Win32,265216 bytes.
- SHA256 `d7c234b55a115acc825d14741b32649a8a92879688b531132f264a946f1491b3`.
- Native predicate/bridge/data/installer/parser and inherited R2 checks pass.
- Staged only; the agent did not install it or run the game.
- Report: `analysis/REPLAY_CLIENT_R1_BUILD_2026-11-14.md`.

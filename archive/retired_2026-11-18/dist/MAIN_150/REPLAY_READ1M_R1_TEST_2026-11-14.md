# REPLAY_READ1M_R1 combined capacity and refill test

Use `DriverSF_V29s_160Capacity_150Request_REPLAY_READ1M_R1.asi` as the **only
traffic ASI**, replacing the previous CLIENT/HITCH/REGMAP traffic ASI for this
run. Keep your working 150-car AI file and other test settings unchanged.
The agent has staged this file only; it has not installed or run it.

This test changes the file-replay read ring from 256 KiB to **1 MiB**, using
separate backing memory, and adds counters for actual reader wait branches.
Native 32 KiB transfers, recording format, readiness checks, update order and
all existing traffic/audio/parked/lifecycle fixes are retained. It is an
experiment, not a confirmed replay-stutter fix.

1. If the previously choppy replay is still available, watch that first at
   normal 1x speed. Otherwise make a fresh recording by driving in Story for
   about 3–5 minutes, then enter Replay.
2. Watch uninterrupted to its end, especially around 2:15 and the later part.
   Avoid seeking or rewinding until that first viewing finishes. Note any
   loading-indicator bursts and their approximate replay times.
3. Afterward try a short rewind, leave Replay, then do another Story -> Replay
   -> main-menu transition to check reuse and teardown.
4. Wait about 3–5 seconds at the main menu, then quit normally. Copy the new
   log into `dist/MAIN_150` and report whether the
   bursts/choppy motion changed, whether traffic/audio stayed correct, and
   whether transitions and exit completed normally. A dump is useful if the
   game actually crashes or freezes; no dump is needed for a clean run.

Log: `%TEMP%\DSFV29s_replay_read1m_<PID>.log`.

The log contains the original narrow `REPLAY_CLIENT` busy/clear records and
new `REPLAY_READ_START/STATUS/OWNER` rows. Managed client boundaries also carry
`rf*` refill counters. Startup should report status1 and readerBytes1048576;
allocationFailed/fallbacks/faults and rfIssueRejected should remain0. An explicit
fallback uses native backing and invalidates the capacity comparison; it does
not silently claim the 1 MiB experiment succeeded.

Private backing is retained for the process lifetime and reused after wrapper
destruction. One owner keeps 1 MiB of committed payload plus small metadata;
the reported retainedBackingBytes is expected even after leaving Replay.

Artifact: 276480 bytes.
SHA256 `fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.
Offline build/native/ABI/ownership/parser/inherited checks pass. Live effects
and total instrumentation overhead still require this game test.

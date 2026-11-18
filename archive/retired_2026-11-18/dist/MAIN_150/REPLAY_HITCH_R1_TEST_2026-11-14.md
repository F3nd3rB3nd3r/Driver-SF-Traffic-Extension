# REPLAY_HITCH_R1 test

This is a passive replay timing test based on the successful REGMAP_REUSE_R2.
It preserves150 requests/160 graphics slots and all existing capacity,
snapshot, parked-car, audio and lifetime fixes. It measures the remaining
replay-motion/loading hitches; it does not attempt a new playback fix.

Use `DriverSF_V29s_160Capacity_150Request_REPLAY_HITCH_R1.asi` as your **only
traffic ASI**, replacing the enabled REGMAP_REUSE_R2 traffic ASI. Keep your
current working vanilla150 AI file, settings and other inputs unchanged.
The agent staged the file here; it was not installed or run in the game.

## One useful session

1. Play Story for about3–5 minutes, then enter Replay.
2. Watch about2–3 minutes at normal1x speed without seeking, rewinding or
   changing speed. Note roughly when loading indicators or choppy recorded
   motion appear and whether actual rendering FPS changes.
3. Return to the main menu, wait about3 seconds, then quit normally.

If the problem is easier to reproduce with an existing saved replay, use
that instead and mention it. Do not delete recordings. Controls can be
tested afterward, but keep that segment separate from normal playback.
If the game fails to start or freezes, retain the log from that attempt.

The log is `%TEMP%\DSFV29s_replay_hitch_<PID>.log`.
Copy it into `dist/MAIN_150` and report what happened, including whether the
recording was fresh or retained. Approximate hitch times are enough.
No memory dump is needed for the first normal-playback capture.

## What the new telemetry measures

- Native replay steps and actual clock advance/holds versus wall time.
- The two distinct readiness checks; observed busy-to-clear intervals of
  at least about1ms, plus all busy-call counts in one-second summaries.
- Checkpoint caller, target, force flag and selected time.
- Recording-stream reposition, import, restore and traffic-request duration.
- Requested/applied playback rate, pause, snapshot/transition phase and
  traffic/retained pending counts on the calling game thread.

New high-frequency observations enter a bounded memory queue. Only the
monitor writes the new event records; no additional engine helper calls or
forced readiness/unpause decisions are introduced. Existing parent logging
is retained. Gate pending-state counters are candidates, not a conclusive
identity for a blocking subscriber. Probe overhead remains to be assessed
in the live capture.

## Artifact / offline checks

-270848 bytes, Release / Win32.
- SHA256 `758805067a6b08ef0e01bbc191d92b35809f37a0264fb49265c6a70b3e3da6c8`.
- New observer/bridge/installer/parser fixtures and compiled/inherited R2
  verification pass. Actual playback remains untested with this probe.
- Detailed build report: `analysis/REPLAY_HITCH_R1_BUILD_2026-11-14.md`.

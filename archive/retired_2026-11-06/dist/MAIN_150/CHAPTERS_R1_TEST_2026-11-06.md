# Chapter-independent vanilla preload test R1

The new test ASI is `DriverSF_V29s_160Capacity_150Request_CHAPTERS_R1.asi` in this folder,176640 bytes, SHA256 `602AA57D7F5464FAA3456499AC3009A07E854E76FBC2B8D6264C2CCD1B229442`.

Built/staged only; no game installation or live chapter test was performed by the agent. Keep PENDING512_R1 as the previously validated fallback.

Later dump review: a Challenge traffic-switch crash on PENDING512_R1 points to a separate unexpanded64-node callback arena. CHAPTERS_R1 retains that arena. Keeping the common roster may reduce the triggering release work, but this is not a callback-storage fix. If this test crashes, preserve its log and new dump. Details: `analysis/CRASH_20261106_001049_CALLBACK_ARENA.md` in the workspace.

## What changes

The existing150-model preload now applies at the verified named-setting request boundary to55 supported setting/frequency profiles:51 named Single Player profiles and4 unnamed profiles whose tables already use the same uniform150 roster. Names/modes of those unnamed hashes remain unresolved. The25 other active profiles retain their native request lists.

It keeps150requests/160graphics slots, ED+EF, indexed262144, audio model192, global audio2048, shared default64MiB, cleanup262144 and pending512. The game's requested hash/frequency, road placement data, mission pots and normal cleanup remain native. No new native hook sites or capacity changes were added. The parked-car issue is queued separately and this test does not target it.

The test requires the currently installed uniform150 AI content documented in the audit. It checks the live native list, route/table data and weights before substitution; unsupported/mismatched data retains the native request and logs a refusal. Startup rejects a changed named-setter instruction signature before applying the baseline patches.

## User procedure

1. Close the game. Use this ASI **instead of** the current traffic ASI; exactly one traffic-capacity ASI should be enabled in the game's plugin directory. Keep the working PENDING512_R1 file available for rollback. No AI chunk or other game asset replacement is needed.
2. Start a fresh process and load the same Chapter7 save as a regression control. Check that loading finishes, city/vehicles render, engine/tire/horn/menu audio works and driving performance is similar.
3. Load an available different chapter through normal game loading. Chapter1,2,5 or6 is a useful first comparison. Check traffic variety, timely vehicle bodies, audio and ordinary mission/player vehicle identity. Different models appearing over a drive does not by itself count150 simultaneously drawable models.
4. Enter and leave a normal mission/activity if convenient. Settings can change without the progression chapter changing; observe loading and correct mission/preview vehicles. A nonzero-frequency profile is useful coverage if naturally encountered, but no forced console call is required.
5. Return from gameplay to the main menu, reload, then exit normally. Report any delay, freeze, crash or model substitution and which chapter/mission was active.

Actual chapter loading matters: a console traffic-setting change alone does not reload all chapter-specific managers. If only a Chapter7 save is available, its regression test is still useful, but it does not establish cross-chapter support.

## Log and what to look for

Log: `%TEMP%\DSFV29s_chapters_<PID>.log`. Copy the completed log into this `dist/MAIN_150` folder, along with the chapter sequence and visible/audio outcome.

- Startup: `DSFV29s_CHAPTER_START revision=1 status=1`, with `profiles=80 expandedProfiles=55 requests=150 graphics=160`.
- Successful profile selection: `DSFV29s_CHAPTER_EVENT reason=expanded caller=0076FE9A ... effectiveCount=150`.
- The important new evidence is an expansion for a hash other than Chapter7's `47EB4D0E`, while gameplay remains healthy. Examples: Chapter1=`AE88E83B`, Chapter2=`3781B981`, Chapter5=`A9E52C22`, Chapter6=`30EC7D98`. Frequency is logged separately.
- `reason=otherCaller`, `empty` and `nativeProfile` can be expected passthrough operations. `sourceMismatch`, `assetMismatch`, `ownerMismatch`, `graphicsNotReady`, `admissionMismatch`, or an unexpected `unknownProfile` need analysis; do not respond by increasing limits.
- `DSFV29s_CHAPTER_STATUS refused=0` is the desired result for supported normal loading. Per-profile rows identify which actual hash/frequency was exercised.
- Inherited pending/cleanup/audio startup checks should succeed, with no fatal/capacity/integrity diagnostics. Native cleanup and pending traversals should finish after returning to the menu.

The legacy `c7pre` fields are not the chapter-independent success marker and may remain0 in this build. Use `CHAPTER_EVENT` and `CHAPTER_STATUS`, plus native request/capacity and the visible result. Counters are independent snapshots, not proof of per-vehicle READY/drawable state.

## Rollback

Close the game and replace this test ASI with the previously working PENDING512_R1 traffic ASI. Keep only that one traffic-capacity ASI enabled. The staged baseline artifact's SHA256 remains `F96FC542561528B8F1CFA3D8F958D88F92A508EE372D5DB6F0392F077FC0409B`.

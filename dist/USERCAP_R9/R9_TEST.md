# R9: audio reasons plus activity and traffic-setting coverage

Use `DriverSF_Unlimiter_USERCAP_R9.asi` and `DriverSF_Unlimiter.ini` beside it
in plugins, with exactly one traffic-extension ASI enabled. R8 remains in its
own dist directory for comparison. The supplied INI is the same280-car setup
as R8; no game assets, installed plugins or saves have been changed for you.
The `profiles` directory contains optional INIs for later, separate runs.

R9 adds diagnostics only. It counts all seven outer audio setup failure reasons
and three mode-initializer subreasons. Failure details use a1024-record queue
drained throughout the session; cumulative reason counts continue even if
details are dropped. `AUDIO_SETUP_START status=1 hooks=10` confirms installation.
These ten hooks supplement the existing bank check; no capacity increase or
audio suppression/retry policy change accompanies them.

## First combined run: keep the current280-car assets and INI

Your available Chapter7 save plus Lua console is sufficient for this run.
Do the following in order; a short note of each completed step is enough.
There is no requirement to supply exact timings.

1. Start at the main menu, watch the saved replay at normal speed without
   seeking, then return to the menu and enter Story. If no saved replay exists,
   skip that first playback and note it. Drive for roughly two minutes.
2. Enter and leave **Getaway Activity1** twice through the previously frozen
   return path. Check that driving resumes both times.
3. Enter and leave **Race Takedown Activity2**, then Activity5 if available.
   Use a car whose model exists in this installation. A save/garage reference
   to a removed Forza vehicle is a separate missing-asset test, not a useful
   capacity comparison. One completion and one abort/retry cover different
   exits if those options are available; note which you used.
4. Do one Speed Trap activity, one dare and one free-roam police chase. Try a
   few vehicles and listen for engines, tires, ordinary horns and police sirens.
5. End active events and return to ordinary free driving before Lua switching.
   Use valid names present in this AI file, one command at a time:

   ```lua
   spooling.spoolTraffic("Chapter 1")
   ```

   Let the fade/loading finish, then drive for30-60seconds. Return with:

   ```lua
   spooling.spoolTraffic("Chapter 7")
   ```

   Repeat with an intermediate setting, for example `"Chapter 4"` if present,
   then `"Chapter 7"` again. Wait for each operation to finish before the next.
   The wrapper performs vehicle cleanup and temporarily disables events, so
   do this after the activity tests. Different settings may legitimately have
   different roster sizes in the current AI file. Do not expect280 everywhere.
6. Drive a few more minutes, checking civilian/parked variety and vehicle audio
   after the switches. Aim for a20-30minute session overall; a longer session
   is useful for capturing the previously intermittent audio returns.
7. Story -> Replay, watch normally -> main menu -> Replay -> main menu ->
   Story, drive briefly -> main menu -> **normal Quit**. Note any skipped step.

This combines audio classification, the two historical activity failures,
several other activity paths, repeated roster changes, reverse replay loading
and an identified shutdown. It is traffic-setting coverage for other chapters,
not a playthrough of their missions, unlocks, cutscenes or progression.

Return `%TEMP%\DSFV29s_usercap_r9_<PID>.log` to this directory. Also keep the
matching DriverNGHook `Driver.log` before another launch overwrites it. Note
the activity/setting order, audible/visible problems, rough FPS and exit method.
If it freezes or crashes, keep a fresh dump and name the transition. The log
alone cannot prove audible output, smooth rendering or a normal exit.

## Subsequent configuration comparisons

Restart the game after every INI/AI change. Copy the selected profile beside
the ASI **as `DriverSF_Unlimiter.ini`**, keeping a copy of the previous file.
Use the same save and a short repeatable route/activity/replay sequence.

| Run | Active AI / assets | INI | What it establishes |
| --- | --- | --- | --- |
| A, first | Current working280-car AI and matching vehicle assets | Supplied main INI | New diagnostics with the known working capacities; broader activity/roster/lifecycle coverage. |
| B | Original AI with its original probability data | `profiles/Baseline150.ini` | Small native rosters with the standard150-capacity backing. It does not force150 cars. |
| C | Your150-car AI and matching assets, if still available | `profiles/Baseline150.ini` |150-car backing with an actual150-car request; compare the same route and transitions. |
| D | Same280-car AI/assets as A | `profiles/Headroom320.ini` | Different backing geometry and4MiB replay reader while keeping the actual assets/weights constant. This is not a320-car residency test. |

Do A first. B/C/D are separate follow-ups, not prerequisites for interpreting
A's audio reasons. Report the AI version and the copied INI with each log.
For an additional Native-policy comparison, change only `RosterSource` from
AssetWeights to Native on the original AI; keep other inputs identical.

Baseline150 uses150 requests,160 derived model slots,192 audio profiles,
2048 global entries,64MiB shared audio,1MiB replay reader,256 retained refs and
45000 extra hash nodes. Original traffic may contain around18 requested models,
but that does not make18 a verified safe capacity for every mission/setting.

Headroom320 uses320 requests,330 derived model slots,448 audio profiles,
4096 global entries,128MiB shared audio,4MiB reader,768 retained refs and180000
extra hash nodes. Work/key/handler capacities derive to1098. It exercises
different layouts without requiring additional vehicle assets.

500/700/1000 live tests should follow only when the installation really has
that many valid vehicle models and an AI file requesting them. Raising only
the INI ceiling while still requesting280 does not validate those larger
rosters. Cold startup and isolated storage tests for higher counts have
passed; audio asset demand, address-space use, mission/replay behavior and
performance at those actual roster sizes remain unmeasured.

## Optional frequency checks, after the first combined run

The supplied `spooling.spoolTraffic` wrapper uses frequency0. The inspected
Lua file also caches `previousTrafficSet` and `previousTrafficFrequency`.
Direct `civilianTraffic.setTrafficSettingsName` calls bypass that cache; mixing
the two methods can make a later wrapper skip a request it thinks is current.

For a separate frequency test, start a fresh Story session and use only the
direct setter for that test: valid setting name with frequency0, then2, then4,
waiting for native loading/road updates after each. Those are valid selector
indices in the inspected native path, not guarantees of different rosters.
Use frequencies defined by your AI file; zero/empty requests can be intentional.
Do not call another setter while loading and do not treat this as the wrapper's
full cleanup/re-enable workflow. Restart before returning to the wrapper-based
combined sequence. Frequency coverage is optional for the first run.

The Lua file's separate `numberOfVehicleAllowed=18` bounds its mission-vehicle
helper list. It is not the named traffic roster size and is not changed by R9.

## How the result will be judged

Existing generic logs already identify setting hash/frequency, original and
effective request counts, roster IDs, READY checkpoints, pools/queues and
replay lifecycle. The new records classify the previously ambiguous audio
returns. Outer-reason counts can be compared with setupFail at a quiet point;
subreason counts are additional detail, not extra outer failures. Concurrent
status samples are not an atomic snapshot and may briefly differ.

The strict `audio_setup` analyzer condition is unchanged: a false return still
makes that check fail, even if its reason is an expected temporary state.
`detailDropped>0` means detail coverage is incomplete while cumulative reason
counters remain available. Context fields ending in `Sample` are accompanying
state; the reason, branch flags and subreason's `observed` value identify the
actual tested condition. No broad claim of all-chapter or1000-car support is
made from these tests.

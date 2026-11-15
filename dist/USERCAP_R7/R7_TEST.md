# R7 mission-capacity loading fix

Validation update2026-11-16: run21688 completed Story, speed trap and pursuits;
run11636 completed menu/Replay/Story/Replay/menu and normal exit, with smooth
playback and no noticed audio/rendering issues. Run11636 passes all42 logged
checks, including zero audio setup failures. See the
[run report](../../analysis/USERCAP_R7_RUN11636_RESULTS_2026-11-15.md).
Keep this as the tested280-car configuration; larger rosters remain unverified.

This fixes the remaining byte-sized vehicle capacity query found in the
PID36080 dump. The mission loader now receives configured `ModelSlots`
(290 for this test), rather than the metadata byte's maximum of255.
Other package/subtype capacities and native admission/readiness checks remain.

1. Close the game. Replace the R6 traffic ASI with
   `DriverSF_Unlimiter_USERCAP_R7.asi`. Enable exactly one traffic ASI.
2. Keep the same280-car `san_franciscoAI.fchunk` and HASH180K settings.
   The included `DriverSF_Unlimiter.ini` has the same active settings as the
   R6 HASH180K retry:280 requests,290 derived model slots,384 audio models,
   4096 global audio entries,128MiB shared audio,2MiB replay reader,
   512 retained references and180000 extra hash nodes.
3. Enter Story using the same save. Confirm that loading completes and you
   can drive. Note any invisible cars, missing engine/tire/horn sounds or FPS
   changes. If this succeeds, try the activity and Story/menu/Replay paths.
4. Return the new `%TEMP%\DSFV29s_usercap_r7_<PID>.log` to this folder and
   describe the result. If loading stalls again, provide a fresh dump too.

`CAPACITY_QUERY_START status=1 modelSlots=290` confirms hook installation.
Sampled `CAPACITY_QUERY_STATUS overrides>0 lastNativeByte=255` confirms that
the widened vehicle query was exercised. These counters do not prove that a
mission vehicle finished loading; reaching gameplay is still required.

Release/Win32 build and46 offline fixture outcomes passed. No live game run,
installation, save/AI change or installed-INI change was performed here.

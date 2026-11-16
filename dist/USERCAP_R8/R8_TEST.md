# R8: manager key-list capacity fix

Live validation: run18736 completed a52-minute Story session including the
previously freezing Getaway Activity1 exit, without noticed FPS/audio/rendering
issues. All22 traffic phases reached native READY; key storage exceeded the old
256-entry boundary with no integrity failure. The strict analyzer passes43/44
because450 audio setup failures occurred, with later success for all affected
models. R8 Replay/menu lifecycle coverage remains pending. See the
[run report](../../analysis/USERCAP_R8_RUN18736_RESULTS_2026-11-16.md).

R8 addresses the Getaway Activity1 exit freeze captured in PID27864. It gives
all14 manager key/refcount lists configuration-sized external overflow nodes,
preserving their native sentinel addresses, sorting, refcounts and recycling.
The current configuration supplies802 entries per list instead of256:
`max(512, ModelSlots + ReplayRetainedReferences)` =290+512.
No new INI setting is required. The included INI keeps R7's active values.

1. Close the game and replace the R7 traffic ASI with
   `DriverSF_Unlimiter_USERCAP_R8.asi`. Enable exactly one traffic-extension ASI.
   Keep `DriverSF_Unlimiter.ini` beside it in the plugins folder.
2. Keep the same280-car AI file, vehicle assets and save for the first retry.
3. Enter Story, run **Getaway Activity1**, then leave it by the same path that
   froze. Confirm that loading finishes and normal driving resumes. If it works,
   repeat the activity exit a few times, then check Story/menu/Replay transitions.
4. Note any audio/rendering issue or FPS change. Return the new
   `%TEMP%\DSFV29s_usercap_r8_<PID>.log` to this folder. For another freeze,
   also provide a fresh dump and describe the exact transition.

`MANAGER_KEY_START status=1 capacity=802` confirms installation. Sampled
`MANAGER_KEY_STATUS` records expose demand, overflow redirection, generations
and failures. A `MANAGER_KEY_FATAL` identifies a refused invalid/full list
before the corresponding native search/write. Counters alone do not certify
successful loading, visible cars, audible sounds or normal game exit.

Release/Win32 build and56 expected offline outcomes pass, including the original
native overflow and looping-lookup reproductions,548-key transitions, all14
list types, recycling, guarded failures and13 manager generations. Getaway and
user-observed performance now have the run18736 coverage above; additional
activity/lifecycle cases and larger configurations retain their own test scope.
R7 artifacts are preserved. No game files or installed plugin/INI were changed.

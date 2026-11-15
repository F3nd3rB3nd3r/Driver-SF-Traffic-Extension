# R6 PID36080: traffic READY, mission admission still capped at255

The supplied full dump and log identify a remaining byte-sized capacity query
on the mission-vehicle admission path. All280 requested traffic models reach
native READY. Chapter7/Free Drive then waits for a separate mission vehicle123,
whose request is rejected against a reported255 capacity despite290 allocated
model slots and ten free vehicle handlers. A further INI increase cannot widen
this byte reader. The next correction is a scoped native capacity-query patch.

This turn performs read-only dump/log/native analysis and writes analysis and
handoff evidence. No production source, ASI, INI, game file or save change,
build, live process/debugger launch, commit or push was performed.

## Input identity and scope

`dist/USERCAP_R6/Driver.DMP` is PID36080,1475074918 bytes, SHA256
`9c7da9165e48b28663d835ce9efad701ad090a5718a4aab2086c0c23ff0af040`.
It is a full64-bit Task Manager dump of the WOW64 game, not an exception dump.
The diagnostic reader memory-maps it and recovers x86 contexts from each
captured TEB64 TLS1 CPU-reserved context. It does not invoke game code.

Evidence directory: `analysis/usercap_r6_softlock36080_2026-11-15/`.
`input_manifest.json` pins the dump, supplied log and current matching SS Test
Driver.log. The helper files `dumpview.py` and `luaview.py` read only captured
memory. Lua tables/closures/upvalues and the active wait's bytecode are saved
in JSON, alongside native disassembly and `admission_evaluation.json`.

## What the hash retry achieved

Startup applies HashExtraEntries180000 with the same280 requests,290 model
slots and384 audio registry entries. No fatal/guard/allocation failure appears.
The phase starts25766ms and reaches280/280 READY at34984ms. It remains280 READY
through the final checkpoint at128359ms/final phase sample129172ms. The native
completed-traffic list in the dump has exactly280 distinct IDs, terminates at
its proper sentinel and matches the entire logged roster without extras or
omissions. This is stronger evidence than request/map counts, but still does
not certify drawable residency while gameplay has not started.

Hash extra lifetime issuance stabilizes at52793/180000. Handler occupancy is
286/802, with four-queue guards healthy; fourth-queue insertion peak37. Audio
registration stores280/384 without full/guard failures, but engine/tire/horn
setup is not exercised by sitting on the loading screen. The hash exhaustion
and R5 fourth-queue failures do not recur in this captured workload.

The generic analyzer's integrity checks pass. They do not measure completion
of this Lua mission wait, so the session is still a failed loading test.

## Captured Lua wait

Driver.log ends at Chapter7/Free Drive `waitingForTrafficSettings`.
The actual registered user update is recovered through the Lua global table,
not inferred solely from that last text line:

- `userUpdateFunctions.loadingSystem_requestVehicles.func` is the DriverNGHook
  error wrapper, wrapping loadingflow.lua's request-vehicles callback.
- Its `loadingSpoolCheck` closure has `trafficSet="Chapter 7"`, frequency0,
  `trafficSpooledState="waitingForTrafficSet"`, `spoolingBusy=false` and
  `missionVehiclesSet="Free Drive"`.
- Its non-nil `stepFunction` is the mission-vehicle completion checker.
  The shared `vehiclesRequestedForSpool` table contains exactly one ID:123.
- The captured step-function bytecode iterates that table, calls
  `TrafficSpooler.IsMissionVehicleLoaded` and returns false unless every entry
  succeeds. This validates the active wait's behavior even though the supplied
  decompiled Lua source has different line numbering from the loaded script.

The game's float32/8-byte-TValue Lua layout is confirmed by its push-boolean
code005E9660 and consistent object tags, source strings, closure/upvalue names
and bytecode constants. No Lua or engine function was executed for inspection.

## Native gate evaluation from captured data

Owner1CF1FCE8; stream manager1CCA0458; vehicle package1CCAB3C0; metadata317CF838.
The relevant captured instruction spans match the supported on-disk executable.

1. `TrafficSpooler.AllTrafficVehiclesLoaded` binding0066E610 calls005A63B0.
   That returns true because owner+DAC=0 and owner+C8=0. Traffic completion is
   not the condition holding this wait.
2. `TrafficSpooler.IsMissionVehicleLoaded` binding0066E6D0 calls005A6EA0.
   ID123 is absent from the completed traffic list and the mission list at998.
   Its fallback returns false because123 exists in the catalog. The loaded
   catalog has975 records and an actual ID123 record `(123,426,0)`. Thus this
   differs from an unavailable/absent garage ID diagnosis.
3. `RequestMissionVehicle` calls005A7860 ->005A6FD0. The native reference and
   pending mission lists are empty; spooler state0 gives no transient credit.
   The admission requirement at005A7093..005A70BC is therefore
   `0 reference adjustment + 0 pending + 280 traffic + 1 new =281`.
4. Capacity query0059C5C0 ->005A0AD0 returns the byte at metadata+18 for
   subtype0. That captured byte isFF, so the result is255. Manager state is2,
   making that query active. The comparison at005A70BC rejects281>=255 before
   queuing the mission request.
5. Configured ModelSlots is290, the native initializer produced294 total
   handlers (290 vehicle plus the two small groups), and vehicle free count
   at package+5834 is10 after the280 traffic allocations.281 would fit290.
   This is a mismatch between widened backing/initialization and an unchanged
   downstream byte query, not exhaustion of the allocated model slots.

The dump captures a render/Present wait in the main thread, not a stack proving
a loader mutex deadlock. The stable log checkpoints plus the decoded Lua wait
and deterministic native branch inputs identify the logical loading stall.
This is static evaluation of captured state, not execution tracing of an
individual historical mission request instruction.

## Recommended next correction

Widen the remaining capacity query for the active vehicle package/subtype0 to
return configured ModelSlots as a full-width value. Preserve native values for
other packages and subtypes;005A0AD0 has one direct caller0059C5DF. Do not patch
the byte to290 (it cannot represent it), force the loaded predicate, bypass
catalog/readiness checks, hardcode ID123 or add it to a compiled traffic list.

Add a native admission regression:280 completed traffic IDs, available but
unlisted mission model, byte capacity255 and actual290 slots. The original
path must reject; the corrected path must queue the request, still rejecting
a genuinely insufficient configured capacity and unavailable IDs. Verify
other package/subtype behavior and constructor generations. Add narrow query/
mission-admission telemetry so the next live log can show the correction.

The existing HASH180K INI and280 AI should be retained for that next build.
There is sufficient captured evidence for a targeted static implementation
before another user live run. The current analysis does not certify all other
downstream limits or the actual model123 asset's rendering/audio after admission.

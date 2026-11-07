# ParkedVehicles toggle: current-build model fallback report

## User observation

Chapter7, PENDING512_R1. Calling the live Lua API `ParkedVehicles.SetEnabled` to disable then enable parked vehicles briefly produces different visible cars. Within about0.5 seconds they appear as the Dodge Challenger/Tanner model associated with ID62. Identification is visual; no before/after `model_id`, persistent object identity or native assignment event has been captured.

The user clarified that ParkedVehicles is a runtime Lua table. The read-only query of `debug.getinfo(ParkedVehicles.SetEnabled, "S")` returned:

`function C =[C]`

This establishes that SetEnabled is a C/native function exposed to Lua, rather than a Lua function body with a source file. It does not identify its owning EXE/DLL, prove that every parked update is native, or identify which routine later changes the visible vehicle.

This is a new current-build behavior report. Earlier successful loading/audio/menu/exit tests did not establish parked-vehicle correctness. It occurs inside the currently expanded Chapter7 case, so the unimplemented extension to other chapters does not explain it on its own.

## Existing evidence relevant to ID62

The authorized traffic docs contain no direct ParkedVehicles or parked-manager trace. `SPAWN_MODEL_TRACE.md` and `TRAFFIC_ELIGIBILITY_VS_RESIDENCY.md` do document a native model resolver at preferred VA0086B9E0 for the examined executable. It searches for the requested model and, if absent or in the branch's disallowed state, can select the first list entry with signed state<=0 at0086BA31..0086BA58. In the documented historical runs that entry was key/model62.

Consequently ID62 is a plausible fallback result, but those findings do not establish that parked vehicles use that resolver. It is also inaccurate to describe the decoded branch as always hardcoding model62. The exact semantic name of its state field remains unverified.

The documented ordinary-civilian assignment route writes the resolved ID to object+5AC at008D144F; the Lua model_id getter reads that field. These are landmarks for a future parked call-graph comparison, not confirmed parked-path hooks.

## Scoped script checks

Searched the already authorized research `docs/*.md` for ParkedVehicles, parked-car terms and the documented fallback addresses. Checked all19 primary installed `media/Scripts*.fchunk` packages, excluding the explicit copy, for literal ParkedVehicles/parkedVehicles. Neither table-name spelling occurred. Two common-script `parked` constants were associated with riding/scoring vehicle properties; other matches concerned an explosion sound/event and mission text.

This negative literal search does not contradict the user-visible table. Native registration, hook-provided globals or dynamic Lua construction need not place the name in these compiled script packages. No external unpacked Lua, CSV, archive, research source/tool, process memory or module implementation was opened. No new native disassembly was performed in this check.

## Focused next trace

1. Locate the registration/owner of the native function exposed as ParkedVehicles.SetEnabled. Inspect what it changes or schedules and how parked vehicles are refreshed afterward.
2. Trace the parked path's requested model, model lookup/resolution and resource retention. Compare it with the documented resolver without assuming they share a selector, cache, variant or owner.
3. Distinguish three possible outcomes on the same tracked object/generation: its model_id becomes62; its model_id stays the intended value but geometry becomes Challenger; or the original object is removed and a new one replaces it. The brief visual difference alone does not distinguish them.
4. If static tracing leaves the timing unresolved, use a narrowly scoped runtime timeline around the toggle: requested/resolved ID, entity identity/generation, caller, selector/resource state and removal/recreation. A general capacity increase or forced ID override would not establish the cause.

Keep150requests/160slots and all validated PENDING512_R1 fixes. Chapter-independent selection remains a separate planned task. No ASI/source behavior change, build, install, game execution by the agent, debugger launch, limit increase or game-file write was performed.

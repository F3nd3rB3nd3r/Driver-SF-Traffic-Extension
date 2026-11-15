# DriverSF Unlimiter USER CAP R3

R3 corrects the R2 crash on the first Story load. An inherited completed-map
check assumed an external buffer was above native storage in memory. Dynamic
allocation invalidated that assumption, so an empty native map could be
mistaken for exhausted external storage. R3 checks both actual storage ranges
and alignment. No capacity or roster setting was increased for this fix.

Use the R3 ASI as the only traffic ASI. Your existing R2 INI remains compatible;
the supplied R3 INI changes comments only. Retry loading Story first, then the
normal lifecycle test below. R3 logs use `DSFV29s_usercap_r3_<pid>.log` in TEMP.
The old guard fails the new regression test; the corrected build passes19
isolated process tests. Actual Story loading still needs your live retry.

This candidate removes the old testing-only maxima from the15 configurable
traffic/resource capacities. The supplied150-car settings retain the previous
live-tested baseline values. **The new R2 binary still needs live regression.**
Offline fixtures at500/700/1000 do not establish successful gameplay at those sizes.

## Using the candidate

Use exactly one traffic ASI. Keep the previous GENERIC_R1 ASI as your fallback.
The new ASI and `DriverSF_Unlimiter.ini` belong together in the ASI loader's
plugin directory. This package has not been installed into your game.
Restart after changing the INI; there is no live resize. Logs are written to
`%TEMP%/DSFV29s_usercap_r3_<processid>.log`; copy the relevant log into MAIN_150.

Start with the supplied INI and your known150-car AI. Check Story, parked
variety, engine/tires/horn, a Challenge/activity, menu reload, saved/fresh Replay,
Story re-entry, and normal exit. Report the transitions and any visible/audible
problem. Then try a non-default configuration with a matching available-model
catalog and AI roster. Missing vehicle assets or stale garage IDs are not repaired
by allocating more slots.

## What each setting controls

`N` is RequestCapacity, `M` is ModelSlots and `R` is ReplayRetainedReferences.
Entries are counts. Only the two `MiB` keys specify memory budgets.

| INI key | Supplied/default value | Meaning and coupling |
|---|---:|---|
|RequestCapacity|150|Maximum IDs in one request; grows request sidecars, notification/parked buffers and native temporary frames. Does not set traffic density.|
|AudioModelEntries|192|Audio-profile registry nodes per manager; includes models outside the traffic roster. Must be at least N and45.|
|GlobalAudioEntries|2048|Capacity of each of three global audio pointer lists; does not promise that many simultaneously audible voices. Native minimum255.|
|DefaultAudioMiB|64|Shared default audio allocation budget. Other native fixed audio pool types are unchanged. Minimum16 MiB.|
|ReplayReaderMiB|1|Read-ahead ring per retained replay-reader owner. Positive power of two, whole MiB. Targets refill starvation.|
|ReplayRetainedReferences|256|Retained ID/refcount pairs in expanded replay snapshots; must be at least N and50.|
|ModelSlots|auto: max(160,N+10)|Vehicle graphics/descriptors/handlers and model-dependent sidecars; must be at least N. Related native graphics budgets scale from the original35-slot allocation ratio.|
|ObjectEntries|auto: max(512,M+4)|Native object records plus free-pointer storage and all corresponding constructor/pop/return/destructor offsets. Minimum512.|
|DefinitionEntries|auto: max(512,M)|External24-byte definition-map nodes, in addition to45 inline nodes.|
|PendingEntries|auto: max(512,M+4)|Total16-byte pending-map nodes per constructor generation, including64 inline nodes.|
|CallbackEntriesPerGroup|auto: max(512,M+4)|Total12-byte callback nodes in each of21 native groups, including64 inline nodes.|
|CleanupEntries|262144|IDs in the cleanup queue; independent of the number of active traffic models.|
|ResourceCollectionEntries|131072|Resource pointer/pair collection; guarded before native append. Shared demand can exceed the traffic roster.|
|IndexedReferenceEntries|262144|Native indexed-reference nodes. Structural minimum65536 for the handle/index domain.|
|HashExtraEntries|45000|Extra24-byte resource-hash nodes; native45000-node inline portion remains.|

Omitting an optional coupled key selects the formula above; a literal0 is invalid.
AudioModelEntries and ReplayRetainedReferences remain explicit budgets: when
raising N past their values, raise those too. Allow headroom for non-traffic cars,
transitions and replay. The auto formulas are consistent storage geometry,
not measured worst-case demand or a per-car resource-size prediction.

For example, an **untested500-car starting configuration** can use N=500,
AudioModelEntries=600 and ReplayRetainedReferences=600, while leaving the
optional coupled keys commented. That derives M=510, Object/Pending/Callback=514
and DefinitionEntries=512. Shared audio and replay MiB values remain independent;
no static calculation proves their sufficiency. Adjust the AI weights and the
available asset catalog yourself; increasing N alone does not add models.

`RosterSource=AssetWeights` augments each named setting's native requests with
its positive-weight model IDs, preserving required native IDs. It does not change
the weights. `Native` preserves the native request list. Neither mode uses a
compiled chapter/car list. With the original AI, leaving N=150 does not force150
models into every setting. Original named settings also vary;18 is not universal.

## Bounds that remain

There is no150/512/8192-style testing ceiling on the migrated INI capacities.
Actual signed-byte arithmetic, native layouts, valid IDs, allocations and thread
stack reservations still constrain them. The reader ring must remain a power of
two; currently this permits1..1024 MiB, and the shared audio byte layout permits
16..2047 MiB. These are representation checks, not useful RAM recommendations.
A32-bit process can run out of usable address space much earlier. Large native
parked/request/notification frames must fit the calling thread's reserved stack;
R2 probes pages and explicitly refuses an impossible frame.

Vehicle IDs remain1..65535; named-setting/probability-table selectors remain
bytes, and parked categories remain25. This work does not widen every format or
every unrelated game subsystem: native per-object event/list limits, fixed audio
pool types and city/region budgets still exist. Some inherited passive detail
samplers and observation rings are bounded; full per-model baseline snapshots
and audio model-key/manager directories now grow. A sampling limit does not cap
the engine's live roster. This is a configurable traffic-capacity mod, not proof
that every possible Driver.exe limit has been removed.

Bad INI values or exhausted/invalid storage are logged and refused explicitly.
Capacity guards are retained; they can intentionally terminate the process to
avoid a corrupting write. Keep the complete log if that happens. INIs use decimal
integers, known keys, no duplicates, and at most4096 bytes.

See the implementation/verification report in
`analysis/USER_CAPACITY_R3_STORY_FIX_2026-11-15.md` and run
`python scripts/analyze_generic_limits_log.py <log> --output <report.json>`
from the repository to check the configuration and recorded integrity. A pass
still requires your visual/audio/lifecycle observations; READY is not drawable.

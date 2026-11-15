# Static capacity costs: model records versus shared runtime demand

User asks which configured limits can be sized from model counts and which
can be tuned empirically. Focused read-only source audit plus the supported
Driver.exe's audio configuration/vector code. Previously supplied/analyzed
logs are reused as evidence. No production/INI/ASI/assets/saves changed, no
build, debugger/game launch, dependency installation or web search.

Evidence: `analysis/capacity_cost_audit_2026-11-15/cost_formulas.json` and
`observed_demand.json`. Formulas describe logical storage requests/payloads,
not complete vehicle memory, allocator overhead, page-rounded commitment or
working-set residency.500 examples are arithmetic, **not supported settings**.

## Interpretation

| Setting | Counted unit | Static storage relationship | Demand relationship |
|---|---|---|---|
|RequestCapacity N|Model-ID positions in one accepted traffic roster|4N bytes per full ID array; additional coupled arrays below|Covers the complete required roster, not spawned vehicle instances|
|AudioModelEntries A|Registered model/profile node in one registry manager|16 bytes/node, plus16-byte tail guard per manager|Normally one node per registered model; may include models outside the traffic roster|
|GlobalAudioEntries S|Runtime pointer slots in each of three instrumented global lists|4 bytes/pointer/list;3*4S pointer payload across those lists|No fixed entries-per-model ratio; not hardware voices or a single aggregateS-slot bucket|
|DefaultAudioMiB D|Shared default middleware allocation budget|D*1048576 bytes budget|No universal MiB/model ratio; not all audio pools/bank memory|
|ReplayReaderMiB B|Replay input read-ahead ring|B*1048576 committed payload bytes per managed wrapper owner|I/O/event workload, not a model-count allocation|
|ReplayRetainedReferences R|Snapshot ID/reference-count record|8 bytes per pair|Actual record count depends on retention sources; a record can contain refs>1, and collection has multiple sources|

Multiple vehicles of the same model do not each require a separate traffic
model-ID slot or model-registration node. Individual vehicle audio instances
can still create additional runtime work and audio records. Sharing sound
assets does not eliminate the separately keyed model registration.

`AudioModelEntries>=RequestCapacity` and `ReplayRetainedReferences>=RequestCapacity`
are conservative configuration rules in this build. They do not prove either
resource's exact peak equals traffic count. The observed original-AI18 run12180
had audio-model peak26; the modified150 runs peaked150. Thus18 is not a safe
assumption for every associated entry pool even in an18-model traffic setting.

## Exact coupled metadata formulas

Current Win32 generic branch, S0=max(N,50):

- Request sidecar per owner:16+4N bytes requested. (`RequestSidecar`,
  `RequestSlot` in DriverSF_V29b_NativeFastPath.cpp.)
- Asset roster scratch per owner/thread:8216+4N bytes requested, including
  the8192-byte seen-ID bitset. (V29sGenericRosterCore.inc.)
- Parked cache per owner:60+8N bytes requested, including IDs, selection
  counters, header and guards. (V29sParked150Core.inc.)
- Parked builder frame:25*(4*S0+4)+632 =100*S0+732 bytes.
  Each additional capacity position above50 adds100 frame bytes because all25
  categories reserve that capacity. This does not mean each real car belongs
  to all25 categories. The native builder retains its stack probing.
- Notification frame:4*S0+8 bytes. (V29sGenericConfigCore.inc,
  V29sNotify150Layout.inc.)
- Replay serialized maximum payload:12+4N+8R bytes; actual written sizes
  follow the recorded counts. External cached slot allocation:60+4N+8R;
  external temporary slot:64+4N+8R. Headers/guards are included in the latter
  two, while the native object's0x264-byte layout is preserved.
- Audio external registry allocation is upfront for eight manager records:
  8*16*(A+1) bytes requested. AtA192 this is24,704 bytes before page rounding.
  These are nodes, not the audio profile/bank data they point to; original
  native objects and diagnostics remain separately allocated.
- Reader per distinct wrapper address:payloadB MiB committed,128KiB additional
  reserved guard address space and a4KiB metadata allocation. Original native
  backing and other native objects are not included. Backing is process-lived
  and reused after native teardown.

Examples atN150/R256: ID array600 bytes; request sidecar616; parked cache1260;
parked frame15732; notification frame608; replay serialized maximum2660,
cached slot2708 and temporary slot2712 bytes. These tiny metadata values must
not be presented as a vehicle's model/textures/audio RAM footprint.

For a hypotheticalN500, the ID array would be2000 bytes, parked cache4060 and
builder frame50732. A500-node audio registry would use8000 bytes of nodes per
manager (plus guard). These calculations do not remove the current150 request
ceiling,160 model/graphics slots, or establish all needed500-car headroom.

Native request scratch remains physically150 in this milestone; its known
frame formula12*N+24 would require6024 bytes at500 versus1824 now. Its native
SUB ESP prologue lacks stack probing. Widening requires coherent native code
and storage changes, not merely allocating the cheap ID arrays.

Windows page/allocation rounding means reducing a logical array may save no
committed pages at all. Per-owner/thread storage and process-lived retention
also prevent summing the formulas into a single universal per-car total.

## Shared pools and replay buffer: measured sizing

Reusable original18/modded150 comparison illustrates the lack of a universal
multiplier.12180 has audio registry peak26, maximum logged default-pool use
4,189,096 bytes and global A0/A1/B insertion peaks25/51/38.150-car22828 has
registry peak150, maximum logged shared use5,492,448 bytes and list peaks
236/53/88. Neither has a global ceiling hit or sampled default-pool pressure
event. The18->150 traffic expansion did not multiply all shared demands by8.33.
Sampled/logged peaks are observations for those workloads, not worst-case
minimum budget requirements or precise allocations attributable to cars.

The64MiB default-pool budget only changes the shared default allocation site
0084A64D. Static bank pools and other audio limits are separate; the90% native
pressure policy is preserved. Exact allocation routing between ambient,
vehicle and other content is not fully catalogued by this scoped audit.

Global list counts are pointer-array counts (native difference/shift-by2);
the underlying active records, sounds and middleware allocations are additional.
Source C50F0C stores the same configured count in two manager settings. Three
live list capacities consume it. Separate per-record8/255 limits remain; raising
the global count does not widen those.

Increasing ReplayReaderMiB can help a demonstrated read-ahead starvation case.
It cannot fix invalid models, broken event handling, decode/CPU work or all
sources of choppy recorded motion. Recent1MiB runs24080/10556 have successful
playback and no sustained/unresolved playback wait;10556 has no observed
playback busy interval. More reader memory is not presently indicated.

## Engineering policy

1. Statically establish entry meaning, widths, allocation formulas and every
   coupled bound. Capacity inputs must allocate matching storage and stay
   representable. Trial-and-error cannot replace this structural verification.
2. Size model lists from required IDs and model registries from maximum
   concurrent registrations, including relevant non-traffic models/transitions.
   Additional headroom is a policy choice, not a proven per-model multiplier.
3. Keep tested shared-budget defaults and allow validated manual overrides.
   Use occupancy/peak/full/pressure/read-wait evidence to adjust the relevant
   budget, one dimension at a time, across meaningful lifecycle workloads.
4. Exact minimum memory per car is unnecessary for a usable unlimiter. Larger
   supported capacities with headroom are acceptable. Array metadata is often
   cheap; large MiB budgets consume address space/resources, and increasing one
   pool neither repairs unrelated limits nor guarantees performance.

Current supported inputs remainN1..150, A45..512 withA>=N, S255..8192,
D16..256, B1/2/4/8/16, R50..4096 withR>=N. Audio manager registry8 and diagnostic
model-key table512 remain additional fixed structures to revisit for larger
scope. This is a cost/meaning audit, not completion of the provider/index,
asset-footprint or above150 migration. No new test profile was applied.

# Active unlimiter limits and traffic policy: static audit

2026-11-14. Requested static audit of the current REPLAY_READ1M_R1 source.
No production source, project, asset, ASI or build output changed. No build,
game, debugger, benchmark or new Driver.exe disassembly was run. This maps
the active implementation and its previously established native boundaries;
it is not an exhaustive discovery of every remaining limit in Driver.exe.

## Conclusions

Most expanded limits already have identified roles. They are several separate
systems, not interchangeable amounts of RAM or one global vehicle counter.
The audit clarifies their coupling, separates traffic policy from capacity,
and identifies constraints that prevent a simple arbitrary-car-count INI.

The current implementation supports bounded requests of up to150 IDs with
160 graphics/definition slots. Its named-setting adapter also replaces
eligible native requests with a specific150-ID roster. That replacement is
a selection/preloading policy, not a capacity expansion. It must be separated
from the storage patches for a native-roster-preserving unlimiter.

An important scaling constraint is the package graphics-admission metadata:
the bridge writes and reads a single byte at configuration+0x18. Values256+
cannot be represented there. The bridge explicitly casts the target to U8.
This is a confirmed constraint of the current path, not proof that255 is a
safe engine maximum. Global handler storage is separately256 entries and
initialization expects admission+4 handlers; both must also be considered.
700-car support therefore requires further native-consumer analysis and
coordinated structural changes, not merely a larger configured number.

## What selects traffic, and what permits it to load

The named setting/hash/frequency, its model request list, and AI probability
tables serve different purposes. Requesting a model makes it eligible for
streaming; it does not assign its traffic probability or prove it is READY,
drawn, or simultaneously resident with every other requested model.

`src/V29sChapterCore.inc:22` validates AI settings/list/group/route/weight
structure and content. `ChapterDecide` at line56 matches the named setter
caller0076FE9A, owner, setting hash, frequency, source roster, graphics state
and the compiled asset fingerprint. The80 compiled profiles include55 marked
for expansion and25 preserved profiles. The matching path at line85 replaces
the request with `kChapter7Active150Ids` and count150. It reads the probability
data; it does not rewrite their weights. The Chapter7 variable names are
historical: this adapter covers multiple named settings, not only Chapter7.

`RequestWriteBridge`, main source line968, otherwise retains the supplied
roster within its150-ID capacity and copies all actual IDs into its sidecar
before publishing the count. Direct Lua requests do not get substituted merely
because they contain150 entries. Notifications, parked caches and replay
snapshots also preserve actual IDs rather than substituting this canonical
roster.

Two implementation traps matter:

- Removing `DSF_V29S_CHAPTERS` alone activates the older Chapter7 exact18-to150
  replacement in the `#else` branch, main source line991. It does not produce
  capacity-only behavior. The build also rejects the old DISABLE_PRELOAD flag.
- An adjusted AI can give150 cars positive probabilities while its native
  named-setting request list still contains18. Removing replacement without
  repairing that relationship can leave selected models unrequested. The
  original AI and a properly matched modified AI need separate acceptance
  cases; changing capacities does not repair asset selection data.

## Active capacity map

Values below distinguish total capacities from additional private nodes.
Counts are entries unless a byte unit is explicitly shown. There is no
established one-vehicle-to-one-resource-entry conversion.

| Structure | Current capacity | What it controls | Main source anchor |
|---|---:|---|---|
| Traffic request and reconcile frame |150 IDs/nodes | Models submitted to the spooler in one request; actual IDs retained | main:862,968,2033; ArenaReuseCore:25 |
| Graphics descriptors / package admission |160 | Graphics-resource admission and descriptor storage; not150 visible road actors | main:207,229,2221 |
| Vehicle definition pool |160 objects, native45 | Capacity for0xF8 definition objects; native allocation retained | main:515 |
| Shared native object pool |512 records, native157 | Shared resource/object records; multiplicity per vehicle not established | main:693 |
| Resource collection |131072, native16384 | Coupled pointer and pair arrays;12 bytes/entry,1.5 MiB backing | main:562 |
| Indexed references |262144, native131072 | Indexed reference/handle table;8-byte slots | IndexedCapacity:11 |
| Resource hash nodes |45000 native +45000 extra | Resource hash nodes and free-list reuse; extra nodes consume1080000 bytes | main:247 |
| Global/kind0 handler arrays |256 each | Package handler pointers; total handlers include other kinds | main:778,1143 |
| Completed-resource map |256 extra nodes | Additional package resource-map records | main:779,795 |
| ED/EF resource maps |45 inline +256 extra per owner | Resource-map nodes, with ownership/lifetime handling | main:799 |
| Selector / third lists |160 extra each | Supporting resource selection/list nodes | main:780 |
| Selector pointer arrays |45 inline +160 extra | Supporting selector pointer storage | main:1793 |
| Definition registry map |45 inline +512 extra | Stable24-byte map nodes; dynamic owner directory | RegMapCore:2 |
| Persistent spool list |50 native +256 extra physically; logical checks150 | Reconciled spool entries, process-lifetime backing and safe reuse | ArenaReuseCore:4,25 |
| Clone helper list |50 native +256 extra | Temporary native clone-list nodes | NativeLoadingTrace:396 |
| Include/membership list |256 total, native64 | Resource-section membership entries | NativeLoadingTrace:880 |
| Notification copy |150 IDs | Complete actual-roster notification rather than the native50-ID prefix | Notify150Core; Notify150Layout |
| Parked caches / category lists |150 IDs per cache/category | Parked candidates available to native picking;25 categories remain taxonomy | Parked150Core:1 |
| Audio model registry |192 total, native45 | Per-model audio registration; separate from sound instances and byte pools | AudioModelRegistryCore:2 |
| Global audio record lists |2048 | Three global audio list capacities, separate from per-model registry | AudioSlotsCore:10 |
| Shared default audio pool |64 MiB, native0x599999 bytes | Shared middleware allocation space; native90% pressure policy retained | AudioDefaultPoolPatch:3 |
| Pending-resource map |512 total:64 native +448 extra | In-flight resource-map records per constructor lifetime | PendingMapCore:2 |
| Cleanup queue |262144 U32 entries, native32768 | Resource-release queue bursts;1 MiB payload | CleanupQueueCore:2 |
| Callback groups |512 nodes/group,21 groups | Native callback queues;64 inline +448 private nodes/group | CallbackCore:2 |
| Replay traffic snapshot |150 IDs +256 retained-reference pairs | Recorded/restored traffic payload;2660-byte separate backing | Replay150Core:3 |
| Replay file-reader ring |1 MiB per owner, native256 KiB | Playback read-ahead and retained sample bytes;32 KiB transfers retained | ReplayReadCore:3 |

`main` means `src/DriverSF_V29b_NativeFastPath.cpp`; other short anchors
refer to matching `src/V29s*.inc` files. Line numbers describe this snapshot.
The resource-collection and indexed-reference tables are distinct, as are
the two256/512 definition/resource maps and the various150-ID payloads.

## Remaining bounds and diagnostic assumptions

There are still fixed owner-directory bounds in active code. An owner is a
native object/header address, not a vehicle model or a sound voice:

- Request sidecars:16 owner records (`main:868,924`). No owner-release path
  was found in the focused source references. This is a lifetime-audit target,
  not evidence that16 vehicles is a limit or that a supplied run exhausted it.
- Clone helper:32 distinct header records; include list:16 section records
  (`NativeLoadingTrace:396,883`).
- Selector/third lists:64 owner records each; selector arrays and ED/EF maps:
  96 each (`main:799-832,1796`). Selector-array clear releases its owner record.
- Audio model extension:8 manager instances (`AudioModelRegistryCore:2`).
  These are separate from the untouched per-record eight-slot audio limit.

The current spool, definition-map, parked, snapshot and replay-reader owner
directories use dynamic/process-lived records. Historical8-owner spool and
4-arena definition-map paths are not the active ARENA_REUSE/REGMAP_REUSE paths.

The audio reason probe observes a native16-entry per-object event structure;
it does not enlarge it. Eight-slot and byte-sized255 per-record audio bounds
also remain distinct from the2048 global lists. Their existence does not prove
they restrict the number of unique vehicle models. No new failure at these
bounds was established by this audit.

`RecordRequestProbe`, main:1079, compares sampled positions of a150-ID list
against the canonical list. A different valid150-ID roster can therefore
increment a boundary diagnostic without an actual storage overflow. That
diagnostic must be made roster-aware before arbitrary rosters are supported.
The baseline parser already distinguishes exact recorded rosters from
`matches_vanilla150`; structural roster completeness is not itself restricted
to the canonical150 set. Its specific150-baseline success labels remain
appropriate only for that test.

Existing draw-queue/submission diagnostics observe other native bounds; they
are not additional expanded vehicle-model pools. No new consumer-by-consumer
audit of those native structures, entity population budgets, ID widths, or
all allocation arithmetic has been completed here. This report is not a
claim that every possible limit for700/1000 models has been discovered.

## Configuration implications and recommended next implementation

Keep the tested150 request /160 slot baseline and ED/EF lifetime fixes.
Separate two explicit behaviors in a future change:

1. Capacity-only mode preserves the native named-setting roster and weights,
   including original assets. It retains actual-list bounds, native ABI and
   executable/layout/startup/ownership validation. Asset fingerprints specific
   to the optional150 preset should not gate native pass-through.
2. An optional tested150 preset retains the current replacement policy and
   its content checks. It is useful for the adjusted AI whose wider positive
   weights are not matched by the native preload list.

An asset-derived preload adapter could later form a deduplicated union of
positive-weight models reachable through the current setting/frequency and
the native required IDs. That requires its own native data-flow analysis and
mission/challenge tests; positive weights alone are not proof that all other
required models may be dropped. It is not implemented by this audit.

Use MiB only for byte pools such as the default audio pool and replay ring.
Use entry counts for model registries, request lists, maps and queues. Frame
offsets, loop bounds, backing sizes, guard geometry, relocation operands,
diagnostics and replay/parked/notification copies must agree. Many current
values are compile-time literals; exposing just one of them through an INI
would leave mismatched structures. Even a count below256 is not automatically
safe: this build is audited for150/160, not for every representable byte value.

The next implementation should first establish native-roster pass-through
within the existing tested capacities, with the150 preset separately enabled.
It should not start by increasing every number or promising a computed
memory-per-car formula. Subsequent live validation must cover original assets,
the matched150 preset, named-setting/Lua transitions, Story/Challenges/Replay,
and normal teardown. Those tests would verify behavior; static analysis alone
cannot certify READY/drawable residency or playback smoothness.

## Evidence

`analysis/unlimiter_static_2026-11-14/source_inventory.json` records hashes of
the explicit project/source inputs and current staged ASI. The current ASI
remains276480 bytes, SHA256
`fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db`.
Latest live evidence remains READ1M run10556; this audit adds no runtime result.

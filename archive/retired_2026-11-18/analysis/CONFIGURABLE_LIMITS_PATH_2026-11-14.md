# Generic asset-selected traffic and runtime-configured capacities

2026-11-14. User direction: the INI should set limits, san_franciscoAI.fchunk
should choose the vehicle roster/probabilities, and the same mod should work
with18,150 or eventually700 models. Compiled car-ID lists and chapter/profile
allowlists are not the intended final architecture. This report begins that
transition with focused static research; it does not implement runtime limits
or claim700-model support in the existing ASI.

No production/project/INI/ASI/game-asset change, compilation, game launch or
debugger use. Latest tested/staged TRAFFIC_POLICY_R1 remains unchanged.

## Intended user contract

1. User installs the mod and configures entry capacities and byte pools.
2. The user's AI file provides model IDs, requested traffic lists and weights.
   Model definitions and resources also have to exist in the game's assets.
3. The mod sizes all coupled structures before native initialization and
   preserves model IDs/order, probability weights and unrelated mission IDs.
4. Setting capacity700 reserves room; it does not force700 requests into an
   original18-model setting or assign equal weights.
5. Configuration is read once at process startup. Invalid/overflowing sizes,
   unsupported structural limits and rosters beyond configured capacity must
   be diagnosed explicitly. Never wrap a count, silently select the first N
   models, or claim a larger configured capacity while allocating smaller data.

For a properly edited asset, its native requested model lists remain the
authoritative preload source. This avoids any need for a compiled model list.
Editing only weights still leaves the known18-versus150 mismatch. If the
user instead wants weights to generate the preload roster, that is a separate
explicit asset-derived adapter: union reachable positive-weight models with
required native IDs, deduplicate, preserve native requirements and validate
capacity before publishing. Do not replace required IDs merely because a
model has zero probability in an ordinary civilian table.

Executable/native-code signatures, ABI layouts and genuine format strides
remain version-specific implementation constants. These are distinct from
compiling particular car IDs or Chapter7 profile content into traffic policy.

## Proposed configuration groups (design, not active INI keys)

| Group | Example fields / tested default values | Unit / relationship |
|---|---|---|
| Traffic |RequestCapacity150|Maximum requested model IDs, not spawned actor population|
| Models |ModelSlots160, AudioModelEntries192|Graphics/definition slots and per-model audio registration; preserve headroom|
| Streaming |ResourceCollectionEntries131072, IndexedReferenceEntries262144|Separate native resource/reference structures|
| Work queues |PendingEntries512, CleanupEntries262144, CallbackEntriesPerGroup512|Independent resource work counts, not models|
| Audio |GlobalAudioEntries2048, DefaultAudioMiB64|Global record count and shared middleware bytes|
| Replay |ReaderMiB1, RetainedReferences256|Read-ahead bytes and separate retained-reference entries|

Names are a proposed contract, not settings understood by TRAFFIC_POLICY_R1.
Do not ship an example with an apparent700 setting until that path actually
consumes the setting. The current parser ignores unknown keys.

Request/notification/parked/replay-traffic storage should derive from the same
validated request capacity. Handler arrays/definition storage derive from
model-slot capacity, including other handler kinds. Node counts and byte pools
remain separately configurable because there is no verified universal resource
or MiB-per-car multiplier. Use the successful150 workload as default evidence,
not a promise that every resource count scales linearly with vehicle count.

Checked arithmetic must cover count*stride, alignment, guard/header space,
allocation size, relative operands and cumulative reservations. The native
32-bit address space and allocation failures still exist; a larger INI value
does not create virtual address space or missing assets.

## New native finding: exact byte-admission loop boundary

Supported Driver.exe SHA256
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
Initializer005A1420 has these relevant boundaries:

- 005A1469: `cmp byte ptr [eax+ecx+18h],bl`, the per-kind zero-count gate.
- 005A14EA: `movzx edx,byte ptr [eax+edx+18h]`, the loop-bound load.
- 005A14E4/14EF/14F3: DWORD loop counter and comparison.
- 005A14A8: DWORD handler ordinal increment.
- 005A14B2 /14D4: DWORD total/per-kind handler counters.
- 005A1476: full-register ordinal pushed into factory0059F9C0.
- Outer traversal visits four kinds; the current package has35,2,2 plus an
 inactive fourth kind, consistent with the tested admission+4 total.

The representational bottleneck at this initializer is the byte load, not a
byte-wide loop counter. A package0/kind0-only widened-bound hook is therefore
a plausible implementation site: source ESI identifies the package and EDX
identifies the kind before the displaced MOVZX. Other packages/kinds should
retain native behavior and original file metadata layout.

This is a candidate approach, not a proof that one hook makes700 safe. The
zero gate must remain coherent, all other consumers of configuration+0x18
must be identified, and provider/factory/virtual-method argument consumers
must be checked for index truncation. Direct factory code uses DWORD stack
arguments and forwards them into virtual calls; those indirect consumers are
not certified by this limited trace. Rewriting one metadata byte to700 is
invalid and current handler sidecars256 would also be too small.

Evidence: `analysis/configurable_limits_static_2026-11-14/` contains the
initializer, factory and helper disassembly. Bytes0059FBB8 onward include the
factory jump table; linear disassembly of those table bytes is not executable
code and must not be interpreted as additional instructions.

## New scaling finding: request scratch needs stack handling

Native005A7490 allocates its frame using plain `sub esp,270h` at005A7495,
without a stack probe at that prologue. Current patches use720h for150 nodes.
The existing list-frame geometry is12*N+24 bytes:

| Logical nodes N | Frame bytes |
|---:|---:|
|50 native|624|
|150 current|1824|
|700 proposed|8424|

A700-node frame exceeds a4 KiB page. Changing only the immediate would risk
skipping Windows stack guard/probe behavior. A native-compatible probing
prologue or external scratch design is required; preserve the four saved
registers, stack offsets, sentinel geometry, merge-helper source displacements
and call/return ABI. This is a static stack-safety concern, not a reproduced
700-model crash or a reason to change the currently working150 build.

Notification local storage has its own4*N+8 byte frame. Parked construction
uses25 category arrays and a much larger frame; it already calls the native
stack-probe helper, and that relocated call must be retained while resizing.
Replay snapshots use separate backing, so their native object/frame views
must remain legacy-sized while external payload becomes runtime-sized.

## Coordinated implementation work

- Central startup config and checked layout plan: immutable after hooks become
  active. Preserve tested150/160/192 defaults, distinguish logical counts
  from physical native layout floors.
- Replace compiled request arrays, notification scratch, parked caches,
  category strides, replay traffic/reference payloads and diagnostic arrays
  with runtime-sized backing or coherent runtime relocation operands.
- Allocate graphics/definition/handler/selector/ED/EF backing from configured
  capacities; preserve constructor generation, reset and process-lifetime
  ownership fixes. Current static owner directories are a separate bound.
- Replace the byte-limited admission path after validating its downstream
  provider/index consumers. Retain other native kinds and configurations.
- Make audio model/global/default pools and independent resource maps/queues
  consume their corresponding entry/MiB settings. Update every matching bound,
  allocation, sentinel, recycle path and diagnostic together.
- Remove active canonical car lists/profile allowlists; log actual requested
  IDs and configured capacities. Preserve version/ABI/startup safety checks.
- Validate with arbitrary synthetic IDs/counts and geometry boundaries,
  especially255/256 and large-stack transitions, before real larger assets.

## Acceptance milestones

First port the verified150 workload onto generic, configured storage with no
canonical selection. Re-run original AI18 and a correctly described150 asset,
using arbitrary rosters in fixtures rather than only the existing150 list.
Then validate larger configured counts across native byte/index and scratch
boundaries. Only afterward claim a700-capable test candidate, followed by user
tests with correctly structured assets and all model definitions present.

Normal Story/Challenge/menu/Replay/exit testing remains necessary for each
structural change. No single car-count-to-MiB estimate substitutes for that.

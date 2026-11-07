# Chapter-independent traffic and configurable capacities

Later evidence update: `TRAFFIC_DOCS_CHAPTER_AUDIT_2026-11-06.md` supersedes the preliminary unknowns below. The supplied docs already identify three request-writer callers and the named-setter call at0076FE95; focused signature/bridge verification is next, rather than rediscovering the whole chain. Direct asset checks establish that all125 namedSP tables are already uniform150, while all51 namedSP profiles retain native12-22-ID lists whose IDs fit that150 roster. Keep post-staging substitution; widening the named setter's40-ID frame is not needed if its native file lists remain unchanged. Configuration and pack sizing remain deferred.

Current scope clarification: the user has deferred the Forza pack and wants the extended vanilla experience first. Chapter independence is the immediate goal. Pack-specific work and arbitrary-count configuration below are future planning only. Include chapter traffic-selection eligibility in the audit: successfully preloading a model does not itself make the native traffic selector choose it.

User direction: make the working vehicle expansion available outside Chapter 7, and establish useful capacity guidance/configuration for vehicle packs such as the mentioned Forza cars mod. This is a source-level assessment and implementation plan, not a new ASI or a claim of mod compatibility. Preserve PENDING512_R1 as the validated baseline.

## Confirmed Chapter 7 coupling

`src/DriverSF_V29b_NativeFastPath.cpp` implements the native request writer at `005A79F0` through `RequestWriteBridge`.

- `IsExactChapter7StockRequest` requires exactly 18 IDs, in the order in `src/Chapter7Preload.h`.
- `CurrentTrafficHashForPreload` must return `47EB4D0E` before substitution.
- Graphics startup and admission checks must also pass.
- The substituted list is a compiled 45- or 150-ID roster. The active project selects the sorted 150-ID variant.
- The header identifies the 150-ID list as generated from the positive roster in a particular installed AI chunk. This is not evidence that it enumerates every installed vehicle or every chapter's required vehicles.

Removing only the hash condition would leave the exact 18-ID fingerprint. Removing both conditions indiscriminately would substitute other requests, potentially including changes in native traffic requirements and teardown requests. A chapter-independent selector needs a verified traffic context and lifecycle boundary, not just fewer conditions.

The capacity patches are separate from this substitution, but successful installation does not establish correct roster selection in other chapters.

## What one vehicle means

Capacity planning must distinguish a distinct resident model ID, a spawned vehicle instance, an audio profile, a resource/reference, and a lifetime/owner. One extra traffic instance does not necessarily require a second model profile. Likewise a new model can introduce many resources and references.

| Structure | Current capacity | Evidence and scaling interpretation |
|---|---:|---|
| Request IDs | 150 per request; 16 tracked contexts | One DWORD per requested ID in the external list. Native request scratch/merge layout is also explicitly patched for 150. |
| Graphics descriptors/admission | 160 | Slots support model loading; they are not a count of simultaneously rendered instances. Preserve current headroom initially. |
| Model-definition pool | 160 | Its patch writes a separate hardcoded 160; changing the request count or graphics target alone does not resize it. |
| Audio-model registry | 192 per tracked registry; 8 tracked registry owners | Latest run has 150 simultaneous entries and three 150-profile registration/removal passes. One 16-byte node per registered model/profile entry is established; 16 bytes is metadata, not all audio memory. Shared/custom profiles must be measured. |
| Pending streaming map | 512 per map lifetime | Latest run issued/activated 154 nodes for each 150-request pass. This supports near-model-count sizing for that workload, not a universal formula of N+4. Each node is 16 bytes. |
| Global and kind-0 handler arrays | 256 each | Need capacity for the actual handlers, including non-roster overhead. |
| Completed map | Native inline area plus 256 extra 16-byte nodes | Extra capacity must not be confused with total capacity. Shares the generic insertion hook with separately identified maps. |
| ED/EF maps | 45 inline + 256 extra nodes per owner; 96 owner records | Per-owner nodes and number of owners are separate limits. Preserve both variants and their lifecycle handling. |
| Selector and third-node lists | 45 inline + 160 extra nodes per owner | Counts cannot be interpreted as merely 160 total entries. Selector pointer-array extension similarly has 45 inline + 160 extra pointers. |
| Spool-0, scratch, clone, include | 256 nodes each | Other list/layout limits can become the first bottleneck even if audio/pending capacities are larger. |
| Additional List A / List B | 512 nodes each | Separate structures with 12-byte and 20-byte nodes, respectively. |
| Indexed references | 262,144 entries, 8 bytes each | Global resource-reference capacity. No reliable per-vehicle coefficient established; shared resources and reference fan-out matter. |
| Resource collection arrays | 131,072 entries per array | Separate pointer/pair arrays; not the indexed table or the pending handler map. No established per-vehicle coefficient. |
| Resource hash nodes | 45,000 native + 45,000 extra, 24 bytes each | Global resource metadata, not vehicle IDs. |
| Deferred cleanup queue | 262,144 pointers | Latest peak 52,176. This is outstanding destruction work, including shared/non-vehicle work, and depends on transition timing. Dividing peak by 150 does not establish a per-vehicle requirement. |
| Global middleware lists | 2,048 slots per list | Playback/record activity rather than one slot per vehicle model. Earlier observed occupancy did not approach the previous 255 boundary. |
| Shared default audio pool | 64 MiB | Byte budget; actual assets and playback activity matter. Not a fixed number of MiB per vehicle. |

Source locations: active `DriverSF_V29s_PENDING512.vcxproj`; main source constants/sidecar structures; `src/V29sNativeLoadingTrace.inc`; `src/V29sPendingMapCore.inc`; `src/V29sAudioModelRegistryCore.inc`; `src/V29sIndexedCapacity.inc`; `src/V29sCleanupQueueCore.inc`; audio slots/default-pool implementations. Runtime evidence is the existing `analysis/PENDING512_RUN33728_RESULTS_2026-11-05.md`; no logs were newly scanned.

The Forza pack itself was not inspected. If it replaces existing IDs, model-count requirements might stay the same while asset/reference/byte demand changes. Added IDs and a larger simultaneously requested roster can increase model-related requirements too. Installed pack size alone is not the sizing input.

## Stage 1: chapter-independent selection, same validated capacities

1. Trace native traffic-request callers and context creation/reset/destruction in the already authorized executable/data scope. Establish how ordinary traffic loads differ from mission-specific requests, empty/unload requests, and other contexts.
2. Identify the intended roster source: native chapter roster plus configured additions, or an explicitly configured common roster with required native models retained. Deduplicate IDs and validate availability; never silently discard required IDs to fit 150.
3. Keep 150 requests maximum, 160 slots and every existing capacity fix during the initial chapter change. If a native/configured union exceeds that bound, diagnose/refuse the unsupported expansion rather than claiming the present build supports it. Define safe fallback behavior before implementation.
4. Audit context/owner lifetimes, reuse and owner-table exhaustion. Examples: 16 request sidecars, 8 spool/scratch arenas, 32 clone arenas, 96 ED/EF map owner records, 8 audio registry owners. Owner counts are not per-vehicle sizing factors. Pending storage is intentionally retained across lifetimes; include its memory growth in repeated-transition coverage.
5. Verify selection and preservation behavior offline, then validate normal loading, mission-required vehicles, chapter changes, reload, menu return and exit in chapters other than 7. All-chapter support remains unproven until tested.

Next concrete research target is the selector/caller/lifecycle audit. Do not replace every request or immediately raise vehicle counts.

## Stage 2: startup INI and pack sizing

Read and validate configuration before engine constructors/patch installation. A restart is required to change capacities: the present storage redirects do not implement live resizing.

Proposed user settings, not implemented options:

- Chapter-independent traffic selection toggle/mode.
- Explicit vehicle roster source/list and distinct model capacity. Keep density/instance counts separate.
- Validated baseline profile, initially the current 150/160 configuration.
- Usage summary showing capacity, current occupancy, peak occupancy, failed insertions, and owner/context counts.
- Advanced capacities, only after each structure's allocation, native operands, bounds, traversal, teardown and startup verification are parameterized together.

Do not advertise arbitrary `VehicleCapacity=N` yet. The current source deliberately accepts only 45/150 requests and 150/160 graphics targets. Besides arrays, request stack-frame immediates, the cloned merge helper source offsets (`714`/`718`), budgets, probes and static geometry assertions assume current sizes. Pending validation uses a fixed 512-bit visited set; the audio key ledger has 512 diagnostic entries. These must be included in a larger-capacity audit, while distinguishing diagnostic coverage limits from native functional limits.

A useful future estimator separates:

- Model-related structures: configured distinct model count plus measured non-vehicle/transient overhead and headroom.
- Global resource/reference/byte structures: a measured base workload plus observed incremental pack demand and transition peaks. Retain current validated floors until lower settings or a scaling policy are tested.

To estimate incremental demand, compare known rosters at controlled sizes on the same chapter/route and transitions, including representative custom assets. Track unique-model occupancy separately from vehicle instances and shared assets. At least two distinct roster sizes are needed even to separate a baseline from an increment; more than one workload is needed before calling that increment a practical sizing guide. Total cumulative insertions are not simultaneous capacity demand.

## State after this assessment

Achieved: located exact Chapter 7 gates, documented current coupled limits and relevant units, and defined staged acceptance criteria. Pending: caller/context audit, chapter-independent implementation/test, runtime configuration, and measured pack-sizing guidance. No source behavior changes, build, installation, game run, dependency installation, web search, debugger or new executable disassembly was performed.

# DriverSF Unlimiter configuration

Put `DriverSF_Unlimiter.ini` beside the enabled traffic-extension ASI in the
game's plugins directory. Enable one traffic-extension ASI. Restart the game
after changing the INI; it is read once at startup.

The mod provides capacity. The active `media/san_franciscoAI.fchunk` supplies
traffic settings, model IDs and probability weights. Installed vehicle assets
must contain those models. Increasing a capacity does not add models, assign
probabilities or change the number of vehicles on the road.

The files in [config](config/DriverSF_Unlimiter.ini) use the new `[Diagnostics]`
section supported by R10. R9 and older builds reject that section; use an
updated build with these files. The memory and traffic settings retain their
existing meanings and defaults.

## Setting up a larger roster: a worked 500-model example

Here, **500 vehicles means 500 different vehicle model IDs available to a
traffic setting**. It does not mean 500 vehicle instances on the road at once.
Several instances of the same model do not each need a separate roster entry.
The game's traffic population and probability rules still decide what appears.

There are three parts to the setup:

1. Install working assets for the vehicle IDs you intend to use. The INI
   cannot create a vehicle or repair an ID pointing to missing assets.
2. Edit the relevant settings/probability tables in
   `media/san_franciscoAI.fchunk`. Give the intended models positive weights
   in the tables used by the desired setting/frequency and locations. Equal
   weights are suitable for a variety test; the unlimiter does not change them.
3. Install R10 and choose sufficient capacities in `DriverSF_Unlimiter.ini`.
   Use `RosterSource=AssetWeights` to include the additional weighted models
   in supported named-setting requests. Restart after changes.

Do not count every row in the whole AI file as one simultaneous roster. For
each supported named-setting request, the mod starts with the native requested
list and adds positive-weight IDs from the probability tables used by that
setting/frequency. Already-present IDs are not added again. Different settings
can therefore have different effective counts. RequestCapacity must cover the
largest effective request you actually use, including native models outside
your added list. Activities and Replay also need to fit their requests.

For an initial experiment, copy
[Experimental500.ini](config/profiles/Experimental500.ini) beside the R10 ASI
and rename it `DriverSF_Unlimiter.ini`. Its active settings are:

```ini
[Limits]
RequestCapacity=512
AudioModelEntries=640
GlobalAudioEntries=4096
DefaultAudioMiB=128
ReplayReaderMiB=4
ReplayRetainedReferences=1024
HashExtraEntries=180000

[Traffic]
RosterSource=AssetWeights

[Diagnostics]
LogLevel=1
```

**This is an untested starting configuration for a 500-model experiment, not
a verified 500-car preset or a calculation of its memory requirements.** Its
choices are deliberately explained:

| Choice | Reason |
| --- | --- |
| RequestCapacity=512 | Room for a nominal 500-model roster and 12 additional request entries. That margin is arbitrary; increase it if the actual combined request is larger. |
| AudioModelEntries=640 | Allows more resident model-audio profiles than request entries. Other gameplay models can contribute; 640 is a trial budget, not a measured requirement. |
| ReplayRetainedReferences=1024 | Trial headroom for resource references captured with Replay snapshots. References are not in a fixed ratio to vehicle models. |
| GlobalAudioEntries=4096, DefaultAudioMiB=128 | Retains the shared audio budgets exercised with 280 models. There is no evidence that these are sufficient for 500 or that doubling them is necessary. |
| ReplayReaderMiB=4 | Retains the exercised Replay read-buffer size. This is not a model-count multiplier. |
| HashExtraEntries=180000 | Retains the exercised extra arena size. Longer sessions and more resources may need more; measure the demand rather than treating 180000 as a 500-model guarantee. |

ModelSlots and the coupled advanced entry counts are intentionally omitted so
they grow automatically. This example resolves as follows:

| Automatically selected backing | Entries |
| --- | ---: |
| ModelSlots |522|
| ObjectEntries |526|
| DefinitionEntries |522|
| PendingEntries |526|
| CallbackEntriesPerGroup |526 in each of 21 groups|
| Work queues / streaming handlers / manager key lists |1546 each|

CleanupEntries, ResourceCollectionEntries and IndexedReferenceEntries keep
their existing defaults. They are shared resource budgets, not values derived
from 500 cars. An omitted key still has a value; it does not mean unlimited.

If starting from `Headroom320_Full.ini` instead, its explicit ModelSlots=330
would be too small for RequestCapacity=512 and startup would reject it. Remove
the ModelSlots/ObjectEntries/DefinitionEntries/PendingEntries/
CallbackEntriesPerGroup overrides to use automatic sizing, or update each
deliberately. Raising RequestCapacity alone is insufficient: the explicit
audio-model and Replay-reference capacities must cover it too.

After startup, check that `GENERIC_START` reports the intended values. After
loading the modified setting, check `GENERIC_EVENT effectiveCount` to see the
actual request; a 512-capacity startup with only 18 requested models does not
exercise 500-model traffic. If the full request is too large, the log records
`rosterExceedsConfiguredCapacity`, `required` and `configuredRequests`, and
the process intentionally exits without publishing a partial list. Use the
reported requirement to revise the configuration; do not bypass the refusal.

Then drive long enough to check variety, visible bodies and vehicle audio;
exercise activities/traffic changes, return to the menu, and watch a Replay.
Keep that process's log and note any symptoms. Native READY checkpoints are
useful loading evidence, but do not by themselves prove every model was drawn
or every sound played. Start with LogLevel=1; use level 2 for a focused
investigation if summary evidence is insufficient.

## Starting profiles and evidence

Copy one complete file beside the ASI as `DriverSF_Unlimiter.ini`:

| File | Intended use | Values exercised in gameplay |
| --- | --- | --- |
| [Default baseline](config/DriverSF_Unlimiter.ini) | Original traffic data or a150-model traffic roster | Earlier tests covered original AI with150-capacity backing and150-model AI with150-capacity backing. |
| [Expanded280](config/profiles/Expanded280.ini) | The existing280-model AI and matching assets | R9 run26476:49m35s of activities, traffic changes, Story/menu/Replay and user-confirmed normal Quit. |
| [Headroom320](config/profiles/Headroom320.ini) | Alternate backing while retaining the same280-model AI/assets | R9 run7044:320 capacity with280 actual models;9 phases all READY and4MiB Replay backing exercised. |

[Headroom320_Full.ini](config/profiles/Headroom320_Full.ini) lists all supported
settings explicitly and is equivalent to Headroom320 at these values. The
shorter profile is also complete: omitted advanced keys use calculated/default
values. The full file pins those values, so update them deliberately if changing
the request/model capacities later. Both keep summary logging at LogLevel1.

| Setting / derived value | Baseline | Expanded280 | Headroom320 |
| --- | ---: | ---: | ---: |
| RequestCapacity |150|280|320|
| ModelSlots, automatic |160|290|330|
| AudioModelEntries |192|384|448|
| GlobalAudioEntries |2048|4096|4096|
| DefaultAudioMiB |64|128|128|
| ReplayReaderMiB |1|2|4|
| ReplayRetainedReferences |256|512|768|
| HashExtraEntries |45000|180000|180000|
| Work/key/handler entries, automatic |512|802|1098|

These are tested workloads and example values, not minimum requirements or
universal recommended sizes for every installation. R10 adds output control;
the broad activity/Replay evidence above is from R9. R10 also has successful
[Story](analysis/USERCAP_R10_RUN20268_RESULTS_2026-11-18.md) and
[Replay](analysis/USERCAP_R10_RUN23108_REPLAY_2026-11-18.md) summary-mode smoke
tests with the 320-capacity configuration and 280 actual models.
A320-capacity test with280 requested models
does not establish320-model residency. The earlier18-model request phases
also do not mean an18-capacity INI covers every mission.

## Logging

```ini
[Diagnostics]
LogLevel=1
```

| LogLevel | Output |
| ---: | --- |
|0|Startup/configuration and essential error/failure records. Healthy periodic diagnostics and expected preview details are omitted. |
|1, default|Startup, traffic changes, roster IDs, first READY checkpoints, Replay snapshot events and periodic summaries, plus essential failures. Ordinary summaries are sampled about every30seconds. |
|2|Full diagnostic records at the existing frequencies, including per-model data, detailed counters, reader samples and preview-suppression records. Use this when investigating a problem. |

The output is `%TEMP%/DSFV29s_usercap_r10_<PID>.log`. Keep the log from the
same process as any dump and note the transition/activity that failed.

Every level retains fatal/refusal records, guards and relevant nonzero failure
counters. Unexpected audio setup reasons retain their available detail;
the native `previewSuppressed` reason remains in cumulative summaries rather
than printing every expected preview rejection at levels0/1. Full detail is
still subject to each probe's existing bounded buffers; its drop counters are
not hidden. A logging failure is reported through the existing file-status
mechanism/debug-output fallback, not a guarantee that a full disk can accept
another record.

LogLevel controls output only. Hooks, native decisions, resource limits,
counter collection, guard checks and normal resource cleanup stay active.
It does not disable safety checks or promise a higher frame rate. The bounded
summary-stream directory can evict old streams and emit extra summaries under
heavy owner churn; it is not a limit on game resources.

`LOGGING_START` declares the level/fullTrace state. `LOGGING_STATUS` reports
attempted/accepted/suppressed output and file status at levels1/2. Accepted
means selected for output, not proof that a file write succeeded. No final
shutdown snapshot is guaranteed. Summary/minimal logs intentionally omit some
data needed by the full diagnostic analyzer; a missing full-trace assessment
must not be presented as a clean all-checks gameplay certification. Set2 and
restart for a full investigation.

## Traffic selection

```ini
[Traffic]
RosterSource=AssetWeights
```

- `AssetWeights`, default: for supported named-setting requests, retain the
  native requested models and include positive-weight models referenced by that
  setting/frequency's probability tables in the loaded AI data. Probabilities
  are not rewritten. The complete effective request must fit RequestCapacity.
- `Native`: preserve native requested IDs/order. Other enlarged capacities
  remain active. This does not create an expanded roster merely because the
  probability file contains additional IDs.

There is no compiled car/chapter allowlist in this path. Lua traffic-name
switching tests traffic settings, not the other chapter's missions/progression.
The inspected named-setting path accepts frequency indices0-4; the supplied
`spooling.spoolTraffic` wrapper defaults to0. Direct setter calls bypass that
wrapper's cached previous setting/frequency, so do not mix methods and assume
every same-name call reloads resources.

## Main capacities

MiB means1048576 bytes. Entry settings are counts, not memory sizes. Values
are independent budgets; there is no general MiB-per-car or one-entry-per-car
formula covering them all.

| `[Limits]` key | Default when omitted | Meaning and constraints |
| --- | ---: | --- |
| RequestCapacity |150|Maximum effective model-ID request list. Must cover the full native/asset-derived request. It is not a target population or a promise that every requested model is drawable. |
| AudioModelEntries |192|Resident per-model audio profile registry entries. Must be at least45 and at least RequestCapacity. Allow headroom for models used outside the civilian roster; observed280-model traffic used up to283 profiles. |
| GlobalAudioEntries |2048|Capacity of each widened global audio list, shared by vehicle and other sound activity. Minimum255. It is not the number of vehicle sound categories. |
| DefaultAudioMiB |64|Requested shared default audio backing. Minimum16MiB; checked for signed byte-size overflow. Other fixed audio pools and the native playback threshold remain unchanged. |
| ReplayReaderMiB |1|File Replay reader backing per retained reader owner. A positive power of two is required:1,2,4,8,..., subject to checked byte geometry. Several owners can retain separate buffers. |
| ReplayRetainedReferences |256|Retained-reference storage for Replay resource snapshots. Minimum50 and at least RequestCapacity. Reference demand is not equivalent to unique vehicle count. |

AudioModelEntries and ReplayRetainedReferences do **not** automatically grow
with RequestCapacity. For example, changing only RequestCapacity to280 leaves
192 audio entries and256 retained references, so startup rejects that
combination. Use the complete Expanded280 profile or set the dependent values.

The global sound budget and Replay buffer cannot repair absent model assets
or incorrect AI/save references. A larger Replay reader can address refill
pressure in some workloads; it is not a general cure for all choppy recordings.

## Optional capacities and automatic calculations

Omit a key to use its automatic/default value. Zero is not an automatic-sizing
request: explicit zero entries are rejected. An explicit value pins that key
and replaces its automatic calculation, even if RequestCapacity later changes.

Let `R=RequestCapacity`, `M=ModelSlots`, and `T=ReplayRetainedReferences`.

| `[Limits]` key | If omitted | What its entries cover |
| --- | --- | --- |
| ModelSlots |`max(160, R+10)`|Resident vehicle model descriptors/admission and associated graphics backing. Must be at least R. Not the on-road vehicle population. |
| ObjectEntries |`max(512, M+4)`|Native streaming object-pool backing. Explicit minimum512. |
| DefinitionEntries |`max(512, M)`|Resource-definition registration-map backing. Explicit minimum45; actual resource demand still has to fit. |
| PendingEntries |`max(512, M+4)`|Pending resource-map entries. Explicit minimum64. |
| CallbackEntriesPerGroup |`max(512, M+4)`|Capacity per callback group, across21 native groups. Explicit minimum64; it is not one shared total. |
| CleanupEntries |262144|Queued resource entries that can be cleaned up in native order. Shared resource demand, not a car count. |
| ResourceCollectionEntries |131072|Shared resource-collection backing. Many resource types can contribute. |
| IndexedReferenceEntries |262144|Indexed resource/reference handles. Explicit minimum65536. |
| HashExtraEntries |45000|Additional hash-node arena after native storage. Demand includes lifetime resource issuance; it is not simply current active cars. |

The following dimensions derive automatically and have no separate INI key:

| Backing | Calculation |
| --- | --- |
| Work queues, streaming handlers/scheduler, manager key lists |`max(512, M+T)`|
| Physical request scratch storage |`max(50, R)`|
| Parked category scratch stride |`4*max(50,R)+4` bytes|
| Parked scratch frame |`25*(4*max(50,R)+4)+632` bytes|
| Notify scratch frame |`4*max(50,R)+8` bytes|
| Replay snapshot payload capacity |`4*R+8*T+12` bytes|

Related clone/sidecar and graphics budgets derive from configured dimensions
as well; they are not independently user-sized switches. These formulas size
backing and do not establish actual peak occupancy. Multiple owner generations
or retained buffers can allocate more than one backing of a given size.

The existing optional example lines such as `ModelSlots=160` should remain
commented/omitted when you expect automatic growth. Explicit smaller values
for independent queues can pass geometry validation yet exhaust at runtime;
startup validation is not a workload-specific capacity estimate.

## What each budget does in practice

These explanations describe the widened storage, not a claim that every
loading/audio problem is a capacity problem. Use the relevant occupancy and
failure counters alongside the symptom. A guard failure or invalid reference
needs investigation; increasing a number does not make it safe.

### Requested models and resident model backing

**RequestCapacity** is the number of model IDs the traffic request can carry.
For example, a supported setting with an effective request of 500 needs at
least 500 request entries. Setting it to 700 permits a larger request but does
not select 700 models, make a rare car more likely, or increase traffic density.
It also sizes request scratch storage, parked-category scratch capacity,
notification scratch storage and the model-ID portion of Replay snapshots.
The 25 parked categories are temporary classification buckets; they are not
25 extra cars to add to your roster count.

**ModelSlots** provides resident vehicle-model descriptor/admission backing and
scales related graphics backing. It must cover RequestCapacity; the automatic
value adds ten slots above the request ceiling, with a minimum of 160. This
is why most users should omit it when resizing. It does not allocate a fixed
amount of texture/mesh memory for each car or guarantee drawable residency:
the real assets, the other resource systems and available address space still
matter. Ten spare slots are an automatic sizing policy, not a proven margin
for every mission or mod.

### Audio: profiles, shared lists and sound-data memory

**AudioModelEntries** stores the resident per-model audio profile registrations.
Think of it as room to register models' audio information, rather than room
for individual horn/engine/tire playback events. A car with several sound
categories does not imply one entry for every category. Allow headroom above
the requested civilian roster for models used by other gameplay contexts;
the observed 280-model workload reached 283 registry entries. This observation
does not establish a universal '+3' rule. A full registry is evidence to raise
this setting. A missing/broken audio profile in the assets is a different issue.

**GlobalAudioEntries** sizes each widened global audio list. These lists are
shared by vehicle and other sound activity, so ambient/UI/gameplay sounds can
compete for bookkeeping space. This is neither a vehicle-model count nor a
promise of that many simultaneously audible voices. Raise it if the affected
global lists reach their configured ceiling. It does not enlarge the separate
native per-object event list: a local `full16` rejection is not evidence that
GlobalAudioEntries=4096 was exhausted.

**DefaultAudioMiB** is the shared default audio backing measured in bytes,
unlike the two entry counts above. It addresses a memory budget rather than
the number of registered models or event records. Actual demand depends on
which sound resources are loaded and their sizes, including non-vehicle audio.
Raising it can help when this particular pool is under pressure; it cannot
repair missing sound-bank data, a full model registry, a local event-list gate
or deliberate native preview suppression. It does not resize every other
audio pool or remove the native playback threshold.

Consequently there is no reliable rule such as '500 cars need 500 sound slots'
or 'every car needs 1 MiB'. For everyday use, start from a known shared budget
and look for evidence of pressure in the specific system before changing it.

### Replay: saved references and disk-read buffering

**ReplayRetainedReferences** provides space for resource references in Replay
snapshots. A recorded vehicle can depend on multiple resources, and resources
can be shared, so the count is not interchangeable with model count. Increasing
it also increases the derived work/handler/manager-key backing through M+T.
It does not change the Replay recording duration or the game's actual retained
resource pool. Start with headroom above RequestCapacity and check captured
reference demand across Story/Replay transitions; the validator's minimum is
only a structural requirement, not a sufficient workload budget.

**ReplayReaderMiB** sizes the file-read backing for each retained Replay reader
owner. A larger power-of-two buffer can reduce refill pressure during playback.
It does not add models or references and cannot repair an incorrectly captured
recording. Loading indicators/choppy recorded motion justify examining reader
and client activity first, rather than assuming any stutter means this buffer
is too small. Several owners can retain separate buffers: 4 MiB is per owner,
not a 4 MiB total Replay-memory ceiling. The demonstrated benefit of a larger
reader in the earlier workload does not guarantee the same result elsewhere.

### Streaming registration, pending work and cleanup

**ObjectEntries** sizes the widened native streaming object-pool backing. These
are internal streaming records, not a count of traffic instances. The automatic
M+4 rule grows the pool with model capacity while preserving a minimum of 512.
It is a starting policy, not proof of one streaming object per car. Consider an
override when this pool's own usage/refusal evidence requires more room.

**DefinitionEntries** sizes the resource-definition registration map. It holds
resource registration records; it is not the number of models in the AI file.
More resource definitions can require more records than the model count.
Automatic sizing follows ModelSlots with a 512 minimum. If resource-definition
registration reaches capacity, increase this budget rather than the unrelated
Replay read buffer or audio byte pool.

**PendingEntries** sizes the map used while resource operations are pending.
Demand depends on overlapping loading/transition work as well as roster size.
Exhaustion can affect the completion of resource work; it does not identify
missing assets or a slow read as the cause. Omission uses max(512,M+4). An
explicit value remains pinned when ModelSlots grows.

**CallbackEntriesPerGroup** is room for callback records in each of 21 native
groups. Callbacks let resource work notify its consumers. A burst can fill one
group even if others are lightly used; 526 means 526 in each group, not 526
divided across all groups. Omission grows it using max(512,M+4). An increase
provides room for more callback records, not faster callback processing or a
direct increase in visible cars.

**CleanupEntries** sizes the queue of resources awaiting native cleanup. It
provides bookkeeping room while leaving the native cleanup order in place.
Large transitions can create a burst of cleanup work that differs from the
steady driving load. Increase it when queue occupancy/capacity failures support
that change. A larger queue does not make cleanup faster or cure a missing
release/lifetime bug, and its default does not grow with RequestCapacity.

**ResourceCollectionEntries** sizes the widened shared resource-collection
backing. Vehicle dependencies and other resource types can contribute; it is
not another list of one entry per car. Its default is independent of the roster.
Use evidence from the collection itself to choose an override. It does not
create more memory for the actual mesh, texture or sound bytes behind a record.

**IndexedReferenceEntries** provides indexed resource/reference handle storage.
A model can use several references; an entry identifies/tracks a resource
rather than containing the complete asset. The default is independent of the
roster, and the explicit minimum is 65536. Handle-storage pressure justifies
more entries; invalid/stale references need a correctness investigation.

**HashExtraEntries** adds hash-node arena space after the native storage.
It tracks resource issuance and lifetime activity, so demand can grow across
loading, activities and repeated transitions even when the current roster
stays unchanged. A short successful load does not establish sufficient arena
headroom for a long session. The observed 280-model setup uses 180000 extra
nodes; neither that value nor a linear multiplication by 500/280 is a proven
requirement for 500 models. Increase it for evidenced arena pressure, and
investigate unexpected growth across repeated lifetimes as well.

### Automatically sized storage and memory tradeoffs

Work queues, streaming handlers/scheduler storage and manager key lists grow
from both ModelSlots and ReplayRetainedReferences. They have no separate INI
keys: increasing either input can increase all these dimensions. This keeps
the related storage consistent, but it is not complete demand-driven sizing
of all game resources. The independent audio, hash, cleanup, collection and
indexed-reference budgets remain yours to choose.

Entry counts primarily add bookkeeping storage; MiB settings allocate byte
budgets. Larger values can reserve more address space before it is all useful,
and multiple live owners/generations can require multiple allocations. This
32-bit game still needs address space for its code, assets and other pools.
Setting every budget to an enormous value can make allocation failures more
likely. Increase the budget that the evidence identifies, then exercise the
same path again.

## Choosing what to change after a test

| Observation | What to inspect or change |
| --- | --- |
| Startup reports an incompatible dependent value | Check ModelSlots >= RequestCapacity, AudioModelEntries >= RequestCapacity and ReplayRetainedReferences >= RequestCapacity. Remove stale explicit advanced overrides or resize them. |
| Exit with `rosterExceedsConfiguredCapacity` | Read `required` and set a sufficient RequestCapacity plus dependent capacities. This is a deliberate refusal, not silent truncation. |
| `effectiveCount` stays at 18 after editing the AI | Confirm the active file, setting/frequency, positive weights and AssetWeights policy. More unused INI capacity does not add IDs. |
| Native READY completes but bodies/audio are wrong | Check actual assets/IDs and the relevant renderer/audio diagnostics. Request completion is not proof of drawable/audible output. |
| Audio registry full or global list ceiling reached | Raise AudioModelEntries or GlobalAudioEntries respectively, using the affected system's counters. These are separate budgets. |
| Audio setup reports `previewSuppressed` | This can be deliberate native behavior while previewing. Do not treat it as evidence to enlarge a pool. |
| Local audio `full16` occurs | It is a separate per-object gate, not either configurable audio-entry limit. Investigate audible impact/event lifetime if needed. |
| Replay has repeated loading bursts | Check reader/client activity and whether reference/restore operations succeeded. Consider a larger power-of-two reader only if refill pressure fits the evidence. |
| A named resource pool/arena reaches capacity | Increase that pool's corresponding entry budget and repeat the transition. Visible symptoms alone do not identify which pool is full. |
| Allocation, guard or invalid-reference failure | Keep the matching log/dump. More capacity is not a general fix; excessive allocations or a lifetime bug may be involved. |

Summary logging is the normal starting point. A log can show successful requests
and no reported failures without measuring every resource's complete demand.
If it does not identify the bottleneck, collect a focused full trace before
assigning a larger value or claiming that all limits have sufficient headroom.

## Limits that configuration cannot remove

The validator enforces native identifier/layout constraints and checked signed
allocation arithmetic. Vehicle IDs in the widened request path must be valid
installed IDs in1-65535. Setting/probability-table selectors remain byte-sized
in the audited path. A32-bit process also needs address space for the rest of
the game and its assets. An accepted number is not a guarantee that its memory
allocation or actual workload will fit.

There is no tested universal maximum roster. Numeric fixtures exercise higher
dimensions, but live500/700/1000-model support requires corresponding installed
models, populated AI requests and its own driving/activity/Replay validation.
Increasing only the INI ceiling while retaining280 requests tests backing
geometry rather than a larger actual roster.

The supplied Lua file's separate18-entry mission-vehicle helper is not the named
traffic roster capacity and is not changed by these INI settings. Other chapter
missions, nonzero frequencies and indefinite lifetimes retain their separate
validation scope.

## File format and rejected settings

The supported sections are `[Limits]`, `[Traffic]` and `[Diagnostics]`. Names
and RosterSource values are case-insensitive. Numeric values are unsigned
decimal integers; LogLevel permits0,1,2. Use `;` or `#` comments. UTF-8 with or
without a BOM is supported; keep the file at most4096 bytes.

Unknown sections/keys, duplicate keys, malformed numbers, incompatible dependent
values and arithmetic overflow are rejected rather than silently ignored.
A missing file uses the compiled baseline and summary logging. An invalid
file records the startup refusal and stops startup; consult that process's log.

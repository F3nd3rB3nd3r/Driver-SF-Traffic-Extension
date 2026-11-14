# Replay subscriber readiness: focused static follow-up to run28136

2026-11-14. User authorized further investigation and confirms the rapid
mini-bursts coincide with the loading indicator appearing/disappearing.
Scope: supported Driver.exe and existing run28136 evidence. No build, game
run, debugger attachment, runtime-source patch or capacity change.

## Result

The static search narrows the native +20 readiness result to **four built-in
replay clients**, using three different nontrivial functions. Their busy
conditions concern missing recorded data, missing future samples or, for the
vehicle client, its separate pending-state flag. These are not direct tests
of the expanded audio/model pools.

One client has an explicit **0.25-second recording timer**, matching the
observed quarter-second spacing. That is a useful candidate, not sufficient
evidence to identify the actual live blocker. The existing probe recorded
the aggregate gate result, not which virtual call returned true.

A further **narrow live identity/data-availability capture** is needed to
choose a justified fix. Static analysis has already identified the small set
of methods and state fields to observe; a repeat of the broad timing probe
is unnecessary. No new ASI is built in this task.

## Supported executable and evidence

`E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.

Evidence directory `replay_subscriber_static_2026-11-14/` contains scoped
disassembly, reference candidates, `subscriber_map.json`, its original
vtable extraction `vtables_initial.json`, and `inspect_native.py`.
The helper checks the executable hash and never executes/attaches to it.
Candidate references and complete instructions were checked before use;
mid-instruction range starts, jump-table data and cut endpoints are not
interpreted as executable evidence.

## Registration and client map

Manager initialization `007383D0` constructs the client members. Setup
`007385C0` registers11 objects through `0073B560`, using each object's
virtual+4 constant ID. The IDs are0 through10. The registry begins at
manager+4E480; head at registry+100 = manager+4E580. Nodes carry key+8,
client pointer+0C and traversal link+4, matching the busy predicate's walk.

The hash arithmetic in `0073B560` is modulo13. This is a **bucket count**,
not a vehicle limit or proof of a13-client maximum. The inline node storage
from4E48C to4E54C describes12 sixteen-byte nodes;11 built-in registrations
fit. None of this registry needs150 entries merely because150 vehicle models
are requested. Additional external/custom registrations are not established
by this static trace.

| Client ID | Manager-relative object | Vtable | Virtual+20 | Virtual+24 |
| --- | --- | --- | --- | --- |
|0|4E360|E2A818|constant false|constant false|
|1, vehicle client|4E3A8|E2B0CC|755DF0|754FF0|
|2, player client|4E3E0|E2AB20|7523B0|constant false|
|3, two-position sampled track|4E388|E2ADAC|7549D0|constant false|
|4|4E408|E2AC64|constant false|constant false|
|5|4E414|E2AE2C|constant false|constant false|
|6|4E420|E2A93C|constant false|constant false|
|7,28-byte sampled track|4E428|E2A8D0|7549D0|constant false|
|8|4E468|E2AA78|constant false|constant false|
|9|4E470|E2AD18|constant false|constant false|
|10|4E478|E2A9DC|constant false|constant false|

The constant-false method is `008203B0` (`xor al,al; ret`). Vehicle/player
names are supported by native diagnostics in their implementation regions.
Client3/7 labels above describe observed payloads, not recovered official
class names. In particular, client3 should not yet be called a vehicle-model
streaming pool or assumed to be the failing client.

## Exact nontrivial busy conditions

Recorded event headers have time at+4, payload size at+8 and next event at
+0C. The generic virtual+1C setter `009275C0` stores the event pointer into
client+4; some clients override this delivery method. Record dispatch
`00746400` routes events into per-client chains, `007460A0` returns a chain
head, and `00746D70` advances that head. The manager's existing outer updater
uses these operations to deliver/advance client data.

### Clients3 and7: `007549D0`

Returns busy when either:

- client+4 is null, or
- that event's next pointer at+0C is null.

Otherwise returns clear. This check does not consult vehicle count, audio
memory, a model-admission cap, or a resource READY predicate. It requires a
current/following recorded sample. Corresponding playback routines use the
sample timestamps and payloads; client3 interpolates between samples.

Client3's constructor `00754600` initializes its timer using0.25f at
`00DD6A18`. Its recording step `00754990` checks timer `00835E70` and calls
producer `007548E0`; producer records event type3 with a24-byte payload,
collecting up to two position pairs through root+210 services. Playback
`00754680` interpolates those position pairs. `0059F3A0` remains a spatial
query; it is not being relabelled as a resource-readiness function.

The0.25 value is a sampling timer, **not an identified capacity to raise**.
Changing it would change recording cadence rather than establish more
lookahead. Clients2/7 use0.05-second timer values; the vehicle client uses
0.1 seconds normally and has a0.3-second branch. Other buffering or delivery
patterns can also produce250ms spacing, so cadence alone cannot select3.

### Vehicle client1: `00755DF0`

Returns busy if the current or next recorded event is absent. If the next
event has no successor, it also compares current replay time with that next
event's timestamp and returns busy once playback has passed it. Otherwise
it returns byte client+34.

Its virtual+24 method `00754FF0` returns just client+34. Run28136's +24 gate
was never busy. For the normal built-in clients, that leaves the vehicle
client's **recorded-data availability/time-horizon branches** as candidates;
the separate pending flag is not demonstrated active by that log.

The diagnostic in `00755000` explicitly refers to
`m_PendingInitialStates` and `VehicleClient::StepPlayback()`. The flag's full
producer/lifetime semantics were not audited here, and no change to it is
proposed.

### Player client2: `007523B0`

Returns busy only when client+4 is null. This is a current recorded-event
pointer test, not a vehicle roster-size test. No live evidence yet shows
that this client is the one repeatedly missing data.

## Recording-stream buffering and why a single size change is unsafe

Reader `00746940` parses complete16-byte event headers and variable payloads
from a circular buffer. It inserts records into the dispatch chains through
`00746400`, handles wrapped records and schedules another read when there is
more than0x8000 bytes of free space. Native I/O blocks are32KiB. The reader
tracks the in-flight byte count at+8584 and completion through virtual+0C.
It does not use a simple fixed event-count-per-frame budget in this path.

Buffer fields include capacity+855C, base+8560, retained boundary+8564,
filled boundary+8568 and parse cursor+856C. `00746BF0` finds the oldest
still-retained record across chains and updates the retained boundary.
Consequently, a missing next sample can arise from several different live
conditions: a read still in flight, insufficient free buffer space while
older records remain retained, end-of-data, or event delivery/advancement.
The static code identifies these alternatives; run28136 does not log their
state to distinguish them.

File-wrapper construction paths pass a0x40000-byte buffer size (**256KiB**),
for example before `0073FAB0` at00739705/00739770/007397AC. Other paths pass
0x80000 to memory-wrapper construction. Which backend/path and occupancy
were active during the late bursts are not recorded in the current probe.
Fast `00746000` reposition durations do not measure these ordinary async
read/refill waits.

Importantly, buffer providers `00736C00`, `00736CB0` and `00736D60` return
regions in a fixed service allocation rather than allocating the caller's
requested number of bytes. C00/CB0 can return the service base; D60 returns
**base+40000**, with separate exclusive/shared-use flags at+C6004/5/6.
Simply changing the advertised40000 capacity would not move the neighboring
partition and could let one buffer overwrite it. A real extension would
need matching owned backing, partition/ownership and cursor handling after
identifying a capacity-related failure. No such extension was made here.

## Focused live evidence needed

The next probe should observe **which existing +20 virtual call returns
true**, not call the methods again. The three native implementations above
cover the four built-in candidates; their caller/object/vtable distinguish
client3 from7 and exclude unrelated uses of shared methods.

On busy-state boundaries, capture:

- Client ID/object/vtable and actual caller/result.
- Current, next and (where relevant) next-next event presence, timestamps,
  and replay time; vehicle-client+34 separately.
- Current recording backend/mode, ring capacity/cursor/free-space values,
  in-flight bytes and end-of-data state.

Use bounded records and monitor-thread output. Avoid full-manager
VirtualQuery/readability scans at every step/update, additional native helper
calls, per-frame disk writes and unrelated timing hooks. Check narrow extra
pointer dereferences only when capturing a boundary. The previous broad
probe's significant overhead makes this reduction necessary before treating
timings as representative.

A normal-speed run through the late section should then distinguish missing
lookahead from a capacity/full-buffer condition or another client state.
Only after that evidence should a capacity or scheduling fix be selected.
No gate bypass, forced clock advance, sampler-rate change or new request
count is supported by the current evidence.

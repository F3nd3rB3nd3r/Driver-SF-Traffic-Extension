# Replay reader refill and retention audit

2026-11-14. Follow-up to narrow probe run30212. Static analysis only: supported
Driver.exe, the supplied log and directly relevant existing evidence/source.
No runtime source, ASI, project settings or game files changed; no build, game
run, installation, debugger attachment, dependency installation or web search.

## Result

The native file reader has a concrete limitation worth a controlled test:
**all replay tracks share a 256 KiB byte ring, and live samples pin older bytes
while subsequent samples must be read in file order**. It has no per-client
time-horizon target or priority refill for client3. It requests one 32 KiB
block per ordinary reader invocation and only dispatches complete records.

This can produce missing lookahead even when a later busy snapshot shows
substantial free space: advancing clients releases old records, but the
reader still has to refill the newly available space. Free space at the wait
boundary is not proof that space was available early enough to prefetch.

Run30212 supplies supporting layout evidence, not a continuous capacity trace.
The reader's exact earlier return reason and low-level completion timing remain
unknown. A **1 MiB file-read ring with matching separate backing** is a justified
first experiment. It is not yet a proven fix or a calculated universal size.
Keep the native 32 KiB transfer size, sampler, readiness decisions and update
order for that experiment so its effect can be assessed separately.

## Executable and reproducibility

Supported executable: `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`.
SHA256 `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.

New evidence is in `replay_refill_static_2026-11-14/`. Its `inspect_native.py`
checks that hash before reading/disassembling the PE. `correlate_boundaries.py`
checks the supplied log hash and derives `boundary_spans.json` without changing
the log. `verify_static.py` passes 22 focused native byte checks, validates the
five direct file-wrapper constructor sites, and checks the three staged ASIs
against their prior hashes. Results: `audit_checks.json`.

Directly reused disassembly is in `replay_subscriber_static_2026-11-14/`:
reader `00746940`, retention `00746BF0`, dispatch `00746CA0`, dequeue `00746D70`,
providers `00736C00/CB0/D60`, constructor `007464D0`, and producer/consumer
`00754600–007549E5`. Updater and replay-step evidence is in
`replay_hitch_static_2026-11-14/0073A070-0073A71E.asm.txt` and
`00738D20-00739136.asm.txt`.

Byte-reference results are candidate lists; only the decoded instructions and
data/vtable entries traced here support conclusions. Partial instructions or
data at the end of disassembly ranges are not interpreted as code. References
to the same numeric field displacement in other classes are not assumed to
refer to the replay ring.

## File backend and actual I/O

The measured backend is vtable `00E2A088`, mode0. Its relevant entries are:

| Virtual offset | Target | Meaning established by trace |
| --- | --- | --- |
| +0C | 0073FB30 | Poll file handle through 008574E0 |
| +10 | 0073F910 | Reposition file handle through 00857490 |
| +14 | 0073F930 | Request read through 008573B0 |
| +20 | 0073F9F0 | Close/drain file path through 0073F980 |
| +24 | 0073F970 | Return cached file length |

The read path is `73F930 -> 8573B0 -> 85AD10 -> 85AB30 -> 85B7E0`.
`85AD10` rounds requests to 2048-byte units, checks open/mode/busy/remaining
file bounds, records buffer/offset/length and schedules native I/O. This is
not a plain synchronous memcpy into the recording ring.

The poll path reaches `85AB10`, which services completion bookkeeping through
`85B550` before reporting whether the file object is busy. A zero recording
field at +8584 is not a measured OS read latency or a direct read-completion
event. Native read-wrapper return values are also not acceptance/error counters
in the current probe. No failed read is demonstrated by this audit.

## Reader control flow

`00746CA0` selects the reader only when active byte+8595 is set and mode+4 is0.
Mode1 goes to writer `00746310`; it must not receive a reader-only scheduling
change. Ordinary `00746940` performs the following sequence:

1. Poll backend virtual+0C; return immediately while it is busy.
2. If +8584 is nonzero, commit that many received bytes to filled+8568,
   wrapping with capacity+855C, then clear +8584. A reposition's initial skip
   at +8588 is applied before parsing.
3. Parse while at least a 16-byte header and its entire variable payload are
   available. Link each complete record through `00746400`, then advance
   parse+856C. A partial record prevents reaching every record behind it in
   the file, including a tiny client3 sample.
4. If an incomplete header/record would cross the ring end, round the required
   assembly size up to 32 KiB. If free space is insufficient, return at
   `00746ADB` before the ordinary refill call. Otherwise relocate only the
   unparsed tail near the buffer beginning and update parse/filled.
5. If end flag+858C is set, return. Otherwise calculate free space from retained
   and filled; require **more than** 32 KiB at `00746B62`.
6. Issue one 32 KiB read at `00746BD5`, set pending-byte bookkeeping and advance
   file-position bookkeeping. The final block's valid byte count is shortened
   at the end of recorded data. There is no ordinary loop to issue several
   blocks in this invocation.

The 32 KiB unit participates in transfer, reposition alignment, file header and
wrap assembly. Increasing just one read-size immediate would make these paths
inconsistent. Increasing ring capacity is a distinct change and can preserve
these native units. Neither one invocation nor one captured busy interval is
equivalent to one rendering frame.

## Retained samples and delivery order

`00746400` routes records with a nonzero high header word to the event chain;
other records go to per-client chains keyed by the low word. Client3's measured
typeFlags00000003 takes the latter path. Its native payload is only 24 bytes;
the large byte distance between its samples includes other replay records.

The per-client head returned by `007460A0` remains retained until explicitly
advanced through `00746D70`. The latter removes one head and calls `00746BF0`.
`00746BF0` starts with the parse cursor, compares all per-client heads and the
event head by circular distance from the old retained boundary, and selects
the oldest. It cannot discard an old track's data just because another track
needs a future sample.

Client3 playback `00754680` advances its current pointer while replay time is
strictly greater than the following timestamp. After advancing, it requires
another successor to interpolate. `007549D0` reports busy if current or next
is null. Bypassing that result does not supply interpolation data; playback
itself also returns early when the pair is absent.

Updater `0073A090` reconciles each client's consumed chain heads at
`0073A3EB–0073A461`, before its tail call to recording service at `0073A714`.
The clock/client step `0073A070 -> 00738F20` is separate. There are multiple
callers of aggregate readiness `007379D0`: `0053AB23`, `00738D35`, `00738F9A`
and `00799794`. At the last, the outer caller runs `0073A090` first; at the
first, a busy result skips a service block while still reaching the replay
step. The narrow probe captures the inner gate return007379F1, not which of
these outer callers is active.

Consequently, the audit establishes local ordering but does not prove a single
global frame ordering for every captured boundary. Moving the refill before
delivery, calling it from the predicate, or looping it until ready would add
native work/side effects and can wait for I/O. There is no statically proven
safe, sufficient one-line scheduling fix here.

## Correlation with run30212

All seven ordinary waits have current3 valid/next absent; five match the late
indicator episode. The next sample after one wait is the identical current
pointer/time at the next adjacent wait. Derived circular layout spans include:

| Sample times (seconds) | Current-to-next ring distance (bytes) |
| --- | ---: |
| 135.2417 -> 135.5000 | 179480 |
| 135.5000 -> 135.7583 | 165732 |
| 135.7583 -> 136.0167 | 121660 |
| 136.0167 -> 136.1583 | 117728 |

The first two distances total 345212 bytes (about 337 KiB), exceeding the
262144-byte ring; the next overlapping pair totals 287392 bytes. Layout and
relocation gaps are included. These are **not exact serialized byte counts,
simultaneous occupancy measurements or a proof of an earlier full-ring exit**.
They do support a mechanism where retaining the current interval prevents
enough of the subsequent interval from being prefetched before advancement.

At busy boundaries the ring has 107384–172516 bytes free and +8584=0. This
does not negate earlier retention pressure. At clears, another 32 KiB request
is bookkept as pending; the reported free distance does not subtract bytes
reserved by that in-flight request.

Two late starts have only eight unparsed bytes. If invoked at those exact
states with a ready backend, the reader could not parse the next header, but
it has enough space for its header-assembly/refill checks. Thus those snapshots
do not themselves demonstrate a large-packet wrap-space failure. The probe
does not record the last reader return reason, next header/payload size, all
retained heads, backend-poll result or outer gate caller. These distinctions
matter before declaring any particular native branch the root cause.

## Capacity, backing and lifetime requirements

The audited cursor/distance helpers use the 32-bit capacity at +855C and base
at +8560; no 256 KiB mask or byte-sized capacity was found in this reader path.
They perform single-wrap arithmetic, so backing/size consistency, bounded
record lengths and a capacity divisible by native block alignment matter.
This is a scoped reader audit, not a certification of every limit in Driver.exe.

All five direct `0073FAB0` construction sites were checked:

| Call | Mode | Capacity | Buffer provider |
| --- | ---: | ---: | --- |
| 00739705 | reader0 | 256 KiB | 00736CB0 |
| 00739770 | reader0 | 256 KiB | 00736C00 |
| 007397AC | reader0 | 256 KiB | 00736C00 |
| 00739AB9 | writer1 | 512 KiB | 00736C00 |
| 0073B048 | writer1 | 256 KiB | 00736D60 |

Provider C00 leases the service base exclusively against C00/CB0/D60 flags.
CB0 leases the first partition while D60 can concurrently lease base+40000.
The next independent region begins at base+80000. The exclusive path already
supports native 512 KiB uses, but that does **not** justify changing the shared
CB0 path's size to 512 KiB. A 1 MiB ring exceeds these native partitions.

For a consistent reader-only experiment, supply separate backing together with
the matching constructor capacity. Keep native provider leases/releases and
writer/memory-wrapper paths intact. Use backing aligned for native I/O, associate
it with the wrapper's construction generation, and handle initialization failure,
same-address reconstruction, transitions and final shutdown. Do not copy an
active ring to a new address: linked sample pointers point into that backing.

Native teardown examples drain through `00746E00`, call the virtual destructor
with flag0, then release the matching provider. The file destructor closes the
handle and resets the base vtable; base `00745DF0` does not free separately owned
ring memory. Therefore a new owner must explicitly manage its own allocation
without handing it to the native object-deletion path, releasing native leases
incorrectly, freeing pending-I/O memory or reusing it before teardown. Relevant
paths include initialization failure73991D, background completion73935A and
mode exit73AD4C. These are implementation obligations, not changes made here.

## Recommended next experiment and success criteria

Prepare, only when requested, a **1 MiB file-reader backing/capacity test** on
the working R2/client-probe lineage. Preserve 150 requests/160 slots, ED/EF,
all inherited lifecycle fixes, the 32 KiB read unit, recorded format and native
readiness/update decisions. Retain the narrow client boundary captures.

Small bounded reader counters should distinguish backend-busy, free-space stop,
wrap-assembly stop and successful issue/progress, with minimum free space and
partial-record requirement before a wait. Aggregate or capture boundaries;
avoid a new per-frame full-manager scan or game-thread file output. This makes
the capacity experiment informative even if the symptom remains.

No additional general live identity session is needed before preparing this
test. A later normal-speed replay run is needed to evaluate it: compare missing
next-sample waits and visible bursts, exercise playback to end/rewind and repeated
Story/Replay/menu transitions, then verify normal exit and drained ownership.
If missing-next waits persist with ample earlier headroom and no space/assembly
stops, investigate refill scheduling/completion latency next. Do not keep raising
the ring on that result. Static analysis alone cannot certify smooth playback.

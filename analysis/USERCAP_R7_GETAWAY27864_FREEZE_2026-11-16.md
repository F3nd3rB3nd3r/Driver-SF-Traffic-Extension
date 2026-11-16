# R7 PID27864: native manager resource-key list cycles after activity exit

The dump identifies a deterministic infinite lookup loop in a remaining fixed
256-entry native manager list. This is distinct from R7's corrected byte-sized
mission capacity query and from the already widened dispatch/scheduler arrays.
The next change should audit and extend the manager's resource-key lists,
including allocation, lookup, removal and reset/lifetime paths. The supplied
dump is sufficient to begin that correction without another diagnostic run.

This task performs read-only dump/log/native analysis and writes analysis and
handoff evidence. No production source, ASI/INI, installed asset or save change,
build, deployment, game/debugger launch, Git commit or push was performed.

## Inputs and actual execution context

`dist/USERCAP_R7/Driver.DMP`:1315442246 bytes, PID27864, SHA256
`bde5509d729cffd735ed963f4da8244e0a1b72e3bad21509ddda844728418923`.
It is a full Task Manager dump with no exception stream. The matching
`DSFV29s_usercap_r7_27864.log` is26261563 bytes, SHA256
`e0cd16bfb05e1c4c286888083ea202684056b67836bc8a8510f21445c5fe6e45`.
The dump identifies the SS Test installation, R7, DriverNGHook and dsfforza ASIs.
The matching SS Test Driver.log confirms Getaway mission-end cleanup and a
Chapter1 -> Chapter7 / Free Drive traffic restoration wait. Its hash and tail
are retained with the scoped evidence.

Important context distinction: the thread stream is AMD64 format but the live
main-thread context has CS23 and RIP0059E0EE, describing current x86 execution.
Its live RSP is0019EAA0. The WOW TLS saved context instead points into a prior
NtQueryVirtualMemory/VirtualQuery call made by the scratch diagnostic hook.
That saved syscall context is stale here and must not be diagnosed as a live
VirtualQuery wait, scratch-hook deadlock or graphics Present wait. The relevant
mod locks are zero. The early saved-TLS stack interpretation was superseded by
the actual stream context; `mapped_threads.json` is explicitly labeled as such.

## Exact looping condition

Main thread19028 is at0059E0EE in the native lookup0059E0D0, reached from
0059D8C1 (return0059D8C6). Its native search bytes in the dump exactly match
the supported retail executable. Manager is1CCAE488; the list starts at
manager+0C=1CCAE494. Native geometry is a12-byte header,25612-byte nodes,
then the sentinel at header+C0C=1CCAF0A0. Adjacent lists have strideC18.

The lookup's captured state is:

| Item | Captured value |
| --- | --- |
| Expected list sentinel (EAX) | 1CCAF0A0 |
| Current node (EDX) | 1CCAFcb8 |
| Current node's next pointer | 1CCAFcb8 (itself) |
| Current node's low16 key | 41 |
| Requested low16 key | 42 |
| Header's active/recycled counts | 548 / 0 |

The current node is the neighboring list's sentinel at manager+1830, not the
expected sentinel. At0059E0E8 the comparison41<42 keeps the lookup searching.
0059E0EE reads the self pointer,0059E0F1 sees a different address from EAX,
and0059E0F3 branches back. Every iteration repeats the same state forever.
The captured EFLAGS agree with the unsigned comparison result. No memory fault
or timeout is necessary, explaining a freeze rather than an exception crash.

The first header reports548 active entries against its native256-node backing;
neighboring headers and the original sentinel contain overwritten key/pointer
data. This is clear list corruption. The548 value is a damaged header, not a
verified count of548 valid live records or a sizing formula per vehicle.

The native insertion helper0059E120 uses a recycled node if available;
otherwise it writes the payload and advances the fresh cursor by12 without
checking the inline sentinel boundary. Generic callers also exist outside this
manager, so a global unconditional patch to this common helper would be unsafe.
Widening allocation/metadata alone cannot repair lookup/reset/sentinel behavior.
The exact historical overflowing write is not traced by this single dump;
the overflow-prone native geometry and current cyclic-list mechanism are
directly established. The precise producer/lifetime path needs the focused
static audit before implementation.

## Transition and capacity observations

Recorded accepted traffic phases are18 ->280 ->12 ->280 ->18 ->280. The first
five reach native all-READY. The last280 request starts722047ms and has no
all-READY checkpoint through876438ms (~154.4s). At capture the owner still has
requested280/completed18; Driver.log is waiting for the Chapter7 return traffic.
The main thread is stuck inside native reconciliation, rather than waiting for
a successful mission request to load as in the earlier R6 dump.

Existing expanded capacities are not full:

- R7 capacity query is active:30 overrides, native metadata255, configured290.
- Native package initializes294 handlers (290 vehicles plus2+2);261 vehicle
  handlers are still free in the dump.
- Dispatch occupancy is556/802, scheduler/traffic queue integrity counters0.
- Hash extra issuance peaks54975/180000; no hash-exhaustion fatal.
- Audio registry peak283/384, no full/guard/missing-key failures. Its48 setup
  failures mean the generic analyzer is not a clean pass, but the main thread's
  cyclic native lookup independently identifies the freeze mechanism.
- Existing mod guards cannot detect this omitted manager list. Their zero
  failures are not evidence that all native lists or the activity transition
  are safe. Replay playback is not exercised in this run.

## Next correction and validation scope

Audit the manager list family rooted at manager+0C and its native256/512-node
variants. Identify exact constructors, insertion callers, lookup/removal helpers,
clears/resets and destructor/generation behavior. Use configuration-sized stable
backing while preserving each native sentinel and sorted/recycled-list behavior,
or coordinate every relocated consumer if changing layout. Guard before native
insertion, validate links and count bounds, and report this list's own demand.
Account for transitional resource keys rather than assuming one slot per car.
Do not merely raise existing INI values, force the traffic-loaded predicate,
discard pending keys or break out of the lookup and pretend success.

Add isolated native regression coverage for beyond256 records, repeated
280-to-small-to280 transitions, sorted lookup/removal/recycle, neighboring-header
preservation and constructor reuse, including a reproduction of this self-loop.
Then a user live Getaway Activity1 exit retry can validate the corrected path.
No new live probe is required to identify this loop or begin its static audit.
All-activity/indefinite-lifetime claims remain unsupported; successful R7 runs
21688/11636 retain their narrower tested scopes.

Evidence directory: `analysis/usercap_r7_getaway27864_2026-11-16/`.
Primary records are `input_dump.json`, `native_list_diagnosis.json`,
`manager_lists.json`, `generic.json`, scoped native disassembly, and the
reproducible memory-mapped inspection scripts. Do not reparse a later generic
Driver.DMP without checking this saved identity.

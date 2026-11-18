# Vehicle-definition map lifetime: static analysis

Static analysis identifies two independent lifecycle defects in the current
ASI's registry extension. A correction can be implemented without first
running the game or attaching a debugger. Run19868 proves that this guard
terminated the process; its exact failing branch remains unrecorded.

## Inputs and scope

- Supplied result: `REPLAY_PAUSE_RUN19868_RESULTS_2026-11-12.md`, with both
  analyzer outputs in `replay_pause_run19868_2026-11-12/`.
- Supported `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256
  `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
- Current `src/V29sNativeLoadingTrace.inc`, source SHA256
  `665415a2c065c341a791970377e5ecef974e78e59699967786a351e9e9c58642`;
  integration/capacity setup in `src/DriverSF_V29b_NativeFastPath.cpp` and
  active `DriverSF_V29s_ARENA_REUSE.vcxproj`.
- Staged REPLAY_PAUSE_R1 remains244224 bytes, SHA256
  `2847467ed26424b6c17773b2c0bd179b6218fbca1f019dc97a50feb86e2c083f`.

Only scoped native code and current source/handoffs were read. No new dump,
external Lua/plugin source, archive/history, unrelated game files or web
sources were examined. No production source/project/ASI changed, no build,
game execution or debugger launch. Microsoft dumpbin only disassembled bytes.
Evidence is under `definition_map_static_2026-11-12/`; `inspect_native.py`
checks the EXE hash before extracting each requested range/reference list.
Reference scans return candidates; relevant call/instruction boundaries were
then checked in the saved disassembly. Mid-instruction range-start decoding
is not used as evidence.

## Native lifetime is shorter than the ASI's owner bookkeeping

| Native address | Established behavior |
|---|---|
|00DB51D0|Registers factory0086C070 for type0x1DE via0086A4F0.|
|0086A2A0|Factory dispatcher invokes the indexed function pointer.|
|0086C070|Allocates0x6A4 bytes and invokes constructor0086BFA0.|
|0086BFA0|Constructs registry with vtable00E395D0, map at owner+0x254 and sentinel at owner+0x698.|
|005A0166..005A0194|Requests type0x1DE via0086A4C0 and stores returned owner at root+0x778.|
|005A0679..005A067E|World teardown passes type0x1DE to0085D6A0.|
|0085D6A0|Fetches root[type], calls virtual destructor, returns/frees owner through native allocator paths, then zeros root[type]. 0x1DE*4 =0x778.|
|0086C040|Registry virtual destructor; calls00888D50 and conditionally operator delete00C2875A.|

This proves create/destroy/recreate is a supported native lifecycle. Actual
heap addresses on subsequent allocations depend on runtime; static analysis
does not predict whether the next instance uses an old or new address.

00888A10 (called at the constructor's end) initializes the shared definition
allocator singleton. 00888D50 tears that singleton down; **these are not
definition-map clear/rebuild routines**. The actual fresh-map initialization
is inline in0086BFA0. No separate in-place clear method was identified in the
scoped registry methods; the correction need not assume an unproven one.

## Map layout and allocator semantics

Let M = owner+0x254.

| Offset from M | Meaning |
|---|---|
|+0|Cursor: next unused node, or head of recycled nodes when recycled count>0.|
|+4|32-bit recycled-node count.|
|+8|32-bit active-node count.|
|+0xC..+0x443|45 inline nodes,24 bytes each.|
|+0x444,+0x448|Sentinel links; sentinel address M+0x444.|

Constructor0086BFEE sets cursor=M+0xC;0086BFF4/FA clear recycled/active
counts;0086C000/06 set sentinel next/previous to itself. These writes are
performed for every newly constructed registry, including an allocation that
reuses a former owner's address.

0086BE10 first performs equal-key search0086BDC0. Existing keys return their
payload without allocation. For a new key the ASI hook at0086BE5F runs before
native node allocator008CCAA0. The allocator:

- With recycled count>0: pops cursor->link, decrements recycled count,
  increments active count, and initializes payload.
- Otherwise: uses cursor, advances cursor by24, increments active count.
- It has no native45-entry or512-entry bound check.

0086BE6C..79 links the new node into the sorted doubly linked list.
0086BE90 removes a matching node, unlinks it, pushes it on the recycled chain,
increments recycled count and decrements active count. The end of a recycled
chain leads back to the saved unused-node frontier. A repeated insertion and
removal does not by itself consume another unused slot indefinitely.

0086BDC0 lookup and registry enumeration methods0086B870/0086B8D0/0086B930
follow links until the fixed sentinel. Reference methods0086BC20/0086BC80
change the node's32-bit payload reference field. They do not index a45-entry
array, truncate node addresses, or require adjacent node allocation. Thus
keeping the native map/sentinel and external nodes is compatible with these
inspected consumers. This is a scoped consumer audit, not an assertion that
every instruction in the executable was formally analyzed.

The shared0xF8 definition-object pool remains160, patched from45 before its
initialization at0088A4F3. That is distinct from map bookkeeping. The existing
map storage totals45 inline +512 external nodes =557 nodes per owner.512
external nodes occupy12288 bytes (12KiB), excluding metadata/guards. There is
no evidence here requiring more node storage for the150-car baseline.

## Two defects in the current extension

### A. Fixed four-address lifetime registry

`g_sRegMapArenas[4]` remembers an address the first time its map reaches the
inline boundary. No inspected production path clears these address records
at world destruction. A fifth distinct map reaching that boundary has no free
record, increments `g_sRegMapFault` and exits with0x31. This is an ASI lifetime
bookkeeping cap, not a four-live-map native limit or a vehicle/audio capacity.

### B. Same-address reconstruction is rejected before reaching the boundary

If a previous owner has an arena record, the current guard accepts only:
cursor==M+0x444 (redirect), or an aligned pointer in that arena's external
storage. After native reconstruction at the same address, the first fresh-key
insertion arrives with cursor=M+0xC and recycled count0. The record still
exists, so the guard takes `else if(arena)`, rejects this legal inline cursor,
increments fault and exits0x31.

The existing boundary reattachment can never help that first insertion: the
guard rejects it44 nodes before reaching the boundary. Increasing4 to a
larger fixed value would leave this defect. Disabling the fatal would leave
genuine node-overrun/corruption branches unprotected.

The adjacent clone helper already accepts valid inline cursors, but its
stack-local lifetime differs; do not transplant it wholesale.

`check_mechanism.py` transcribes the relevant existing guard conditions and
checks both counterexamples, an ordinary external insert, recycled bypass and
external-end protection. `mechanism.json` records passing assertions and the
source hash. This is an arithmetic/branch model, **not native execution,
production-fixture verification or proof of which branch run19868 took**.

## Recommended implementation scope

1. Replace the fixed four-entry owner directory with stable process-lived
   per-address records allocated as needed. Retain512 external nodes per
   address, native45 inline nodes, original node/sentinel layout and native
   recycled-node behavior. Allocation remains fallible; report allocation
   failure explicitly. This removes the artificial owner-count cap, not all
   finite memory limits. Do not free storage while native nodes may reference
   it; process-lived records match existing retention policy.
2. Explicitly recognize a new native map generation. A narrowly scoped
   post-constructor observer at0086BFA0 is a suitable verified lifecycle site:
   run the complete original constructor first, verify its returned owner,
   vtable, fresh cursor/counts/sentinel, then reset only associated bookkeeping.
   Its initial whole instructions `56 8B F1 33 C9` make a5-byte relocatable
   entry span; full constructor/consumer signatures must still be preflighted.
   No blanket reset of live external nodes or native state. A prior arena can
   be reattached when the new generation reaches its inline boundary.
3. Accept only aligned inline/external cursor states appropriate to the
   recorded generation; validate native recycled-node pointers/counts and
   keep end/overrun checks. An unexpected inline cursor in an already-external
   generation should not silently authorize overwriting existing nodes.
4. Record owner/generation count, constructor resets, redirects/reattachments,
   inline/external use and precise faults. Fatal output needs actual cursor,
   recycled/active counts, backing start/end, used count and a branch reason
   such as allocationFailed/externalEnd/unexpectedCursor/badRecycledNode.
   The current default zeros and unrelated arena counters are insufficient.
5. Make installation/rollback atomic across the existing insertion hook and
   added lifecycle hook. Preserve all replay fixes and150/160/ED+EF/audio/
   parked/pending/cleanup/callback capacities. Do not hook the generic008CCAA0
   allocator globally: reference scans show it serves many unrelated maps.

The constructor observer is an implementation recommendation, not a patch
already made or a requirement for another live diagnostic run. At build time,
check full native signatures and installation order before finalizing hooks.
Simple unconditional inline acceptance is less precise than recognizing the
known constructor generation and could mask a cursor moving backwards.

## Verification required after implementation

A native fixture should execute the actual constructor and map operations
with only external allocator/registration services stubbed:150 new IDs,
duplicates, lookups/enumeration, mixed inline/external removal and reuse,
more than four distinct addresses, same-address destroy/reconstruct, repeated
generations, exact external end, invalid alignment, allocation failure and
install/rollback. Check calling convention, registers and stack balance for
the new adapter. The branch model above is not a substitute for that fixture.
Run inherited verifier checks once the requested build exists.

Then live-test repeated Story -> Replay -> Story and menu reloads (both new
and reused owner addresses if observed), normal exit and playback controls.
Telemetry must prove owner/generation paths were exercised. No preliminary
live test/debugger is needed to start this correction, but static analysis
cannot certify final game stability or retrospectively select run19868's
missing branch. Any recurring exit must be interpreted from the new precise
fatal record.

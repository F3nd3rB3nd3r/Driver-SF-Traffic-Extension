# Deferred resource-cleanup queue: static audit, 2026-11-05

The audit supports a controlled storage expansion while preserving the native cleanup timing. The queue has32768 pointer slots, three unchecked producer bodies, and a count immediately after the array. Its consumers use DWORD counts and pointer traversal; the inspected paths contain no additional32768-entry traversal ceiling or16-bit queue index. Five storage references need coordinated relocation, including an indirect count reader. There are concrete reasons to retain deferred destruction rather than flush from inside an append hook.

This is static analysis only. No game behavior/source changes, ASI build, installation, game execution, debugger, new live probe, dependency installation or web search. New files are an offline audit script, evidence and documentation. Existing audio192 and150/160/ED+EF/indexed262144 settings remain unchanged.

## Scope and verification

Exact authorized `Driver.exe` SHA256 `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`. Read queue routines, their direct callers, initialization, wrapper release and immediate processing/shutdown boundaries. Reused the prior run260 crash interpretation; no other logs/dumps were searched.

`python scripts/audit_cleanup_queue_static.py` passes21 exact native instruction-span assertions, confirms21 incoming calls at disassembled instruction boundaries, resolves eight relevant imports, verifies the known array/count reference inventories, and saves32 bounded disassembly ranges. Evidence: `analysis/cleanup_queue_static_2026-11-05/inventory.json` and neighboring assembly files. Reference searches cover exact immediates, relative CALL/JMP targets and literal function pointers in this executable. They do not prove the absence of arbitrary computed aliases or unresolved indirect callers.

## Layout and failure boundary

| Field | Native location | Meaning |
|---|---|---|
| Owner |01732BF0 |Statically addressed resource manager |
| Queue |owner+15E090 |32768 entries,4 bytes each,131072 bytes |
| Count |owner+17E090 |DWORD, immediately after queue |
| Indexed slots |owner+17E094 originally |Separate table already relocated by indexed262144 test |

All three appends calculate `queueBase + 4*count`, store a pointer, then increment the count. Their `TEST destination,destination`/zero branch is a null-address check, not a capacity check. At count32768, the store aliases the count. Increment then operates on the pointer just written there; the next append can address far outside the list. The prior dump's exact bad destination is consistent with this mechanism, but the first corrupting write remains absent from that dump.

Entries concern retired engine resources, not one entry per vehicle. Vehicle count alone cannot establish the required maximum cleanup burst.

## Producers and ownership transitions

| Routine | Role established by code | Append | Incoming calls found |
|---|---|---|---|
|00BAEA60 |Direct pointer enqueue |00BAEA75 |No direct CALL/JMP or literal pointer reference found; retain in relocation inventory |
|00BAEBE0 |Replace an existing keyed resource |00BAEC2A |5 calls, including texture/material paths |
|00BAECA0 |Release a keyed resource after reference decrement |00BAED45 |13 calls, including bulk cleanup00BBA760 |

`00BAECA0` resolves the keyed record, gets its wrapper and pointee, then calls imported **InterlockedDecrement** on `pointee+8`. It returns without enqueue if the result is nonzero. On the zero path it rewrites linked handles and recycles their indexed slots, queues the wrapper, performs conditional short-key recycling, then unlinks/recycles the keyed hash node through00BAEEA0. The queue retains the independent wrapper pointer, not the recycled hash node. Hash-node recycling therefore does not itself free that wrapper.

`00BAEBE0` has different lifetime semantics: it queues the old wrapper, sets the new pointee's reference count to the old count+1 through imported **InterlockedExchange**, publishes the replacement wrapper, and updates linked users. Those updates happen **after enqueue**. Consequently an early drain from inside enqueue could destroy the old resource before native pointer redirection finishes. A nonzero old reference count on this replacement branch is not automatically a lifetime error.

The pointer-wrapper interpretation follows the actual dereferences: map value ->wrapper ->pointee; queued wrapper is later returned to the native allocation manager. No change to these pointers, reference counts, hash-node recycling or handle rewrites is needed merely to relocate queue storage.

## Drain, order and reset

Drain00BAE6B0:

1. Reads the native count and records diagnostic fields.
2. Forms the array base, reads count again through `[arrayBase+20000]`, and computes an end pointer once.
3. Walks queued wrappers in insertion order. For each, calls the pointee's virtual slot0 with argument0, then returns the wrapper via allocation-manager routine00BB4EE0.
4. Advances4 bytes per entry; resets native count to0 after the captured range is processed. The empty path also writes0.

The virtual call has the expected destruction shape; the exact concrete resource destructor varies by queued pointee and is not resolved for every type. The native wrapper release call and order are explicit.

The end pointer is a **snapshot**. The loop does not extend it for newly appended entries. An enqueue during the drain could therefore be lost when the native count is reset. This is a constraint on preserving native serialization and avoiding new reentrancy; this audit does not demonstrate that such an enqueue occurs in the user's run.

Two direct drain callers are verified:

-00BAD80E, within00BAD790: a higher-level processing boundary, before frame-counter advancement and a semaphore release. It conditionally waits through WaitForSingleObject before reaching the drain.
-00BABC3D, within00BABC10: a shutdown-style sequence after other resource cleanup/processing-manager teardown and before allocator/worker cleanup.

Bulk handle cleanup00BBA760 loops over its complete handle range, calling00BAECA0, with **no drain inside that loop**. A large batch can accumulate before the next native drain. This fits an exit-only overflow without establishing the actual peak of the supplied run. The queue itself uses plain stores/increments; atomic reference-count operations do not make queue mutation thread-safe. The inspected higher-level coordination must be preserved. A complete proof of serialization across every indirect callback/worker is outside this bounded static audit.

## Initialization and backing-storage lifetime

Initializer00DC3F50 supplies the static owner01732BF0 to constructor00BAE930, then passes callback00DC7F30 to a runtime callback-registration helper. Constructor00BAE930 zeros count17E090. Callback00DC7F30 is simply `RET`; it contains no queue-specific free or drain. The queue's original backing is part of static manager storage, not a per-session heap allocation.

The current ASI DllMain only handles process attach. Existing indexed/hash/audio external storage is retained for process lifetime rather than freed during native resource teardown. A future external cleanup array should have that same lifetime, be installed before native use, and retain the native count/reset location. Installation after entries already exist would require explicit migration and is not justified by this design. Do not free or restore the array on a guessed shutdown/menu boundary while game code can still append or drain.

## Complete identified storage-reference set

| Instruction | Bytes/operand | Required change if native count stays in place |
|---|---|---|
|00BAE6D3 |6-byte LEA, displacement+2 |Drain array base ->external base |
|00BAEA66 |7-byte LEA, displacement+3 |Direct append base ->external base |
|00BAEC1F |7-byte LEA, displacement+3 |Replacement append base ->external base |
|00BAED3A |7-byte LEA, displacement+3 |Zero-reference append base ->external base |
|00BAE6E4 |6-byte MOV, displacement+2 |Count read ->original native count relative to external base |

For the four bases, the new displacement is `externalBase - loadedOwnerAddress` modulo2^32. For00BAE6E4 it is `loadedNativeCountAddress - externalBase`. Simply increasing its old20000 displacement to the new array size would read the wrong location if count remains native.

The ten explicit owner-relative count references remain native: diagnostic read, constructor reset, two drain resets, and read/increment pairs for three producers. The11th count access is the indirect drain reader above. Constructor initialization, diagnostics and the existing indexed/free-head layout can therefore remain intact. Expanded storage alone still needs pre-store bounds detection; there is no native refusal branch to enlarge.

## Assessment and next step

There is no statically established need for a broad lifetime probe before preparing a storage-only test. The audit supplies the required relocation set and ownership/order constraints. A future combined expansion and bounded census should measure pre-append count/peak, producer identity, drain start/end counts, guard integrity and unexpected enqueue-during-drain. Preserve ordinary native enqueue/destruction timing; do not clamp/reset count, skip releases, or force a nested drain to mask an overflow.

Static evidence cannot establish that the user's first corruption was the32769th append, choose a universally sufficient capacity, or exclude every unrelated stale-pointer/destructor fault. The supplied dump corroborates overflow but lacks queue memory. If a guarded expansion removes overflow and exit still fails, the next trace should target wrapper/pointee lifetimes and the concrete destructor at the remaining fault. No new ASI is built by this audit.

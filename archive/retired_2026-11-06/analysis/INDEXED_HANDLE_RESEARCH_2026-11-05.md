# Indexed-handle lifetime and relocation research — 2026-11-05

## Findings

1. The 131072 entries are **registrations of individual mutable reference locations**, not 131072 unique resources or vehicles. Several handle locations can refer to one resource and consume separate slots.
2. The native reference has two representations: a UID pair, or a direct resource pointer plus a flagged tracking index. Conversion into the second form consumes a slot. Some graphics readers resolve the UID through the hash when conversion has not happened.
3. Consequently, null free head and a pending list establish stalled reference processing, **not that every pending entry points to unavailable geometry**. The free-head check even precedes removal of some already-converted/stale pending records.
4. Normal individual unlink and resource-release paths return slots. The resource itself has a separate reference count. No definite reference leak was established from this static inspection.
5. An external-table expansion appears mechanically feasible. **13 candidate operand changes** are verified against the exact executable. The two next-reference reads at `00BAEC7F` and `00BAED08` must be included; they are used during resource replacement/removal.
6. The native slot area ends immediately before metadata and other globals. Increasing the constructor count in place would overwrite unrelated engine state. External storage and startup validation are required.

This is a research result, not an implemented capacity patch. It supports a carefully isolated expansion test, without promising a rendering or audio fix.

## Scope and reproducible evidence

- Exact executable: `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`.
- SHA256: `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.
- Runtime premise: the previously analyzed `DSFV29s_capacity_16316.log`; no additional live run was performed.
- No game folders outside the previously authorized executable/Install/media scope were inspected. This follow-up needed no additional external data files.
- No ASI/source-behavior changes, builds, installations, process attachments or game runs.

`scripts/research_indexed_handles.py` validates the operands and streams the executable's `.text` disassembly, retaining only address/offset candidates. It does not save a huge full executable listing. Output is under `analysis/indexed_handles_static_2026-11-05/`:

- `relocation_inventory.json`: original operand bytes/addresses, literal references and arithmetic checks.
- Focused function disassemblies.
- `absolute_slot_address_candidates.txt` and `relative_slot_offset_candidates.txt`: candidates for manual review, not automatically established aliases.

The code sweep parsed 3361787 instruction-address lines. Four numerical constants fell within the absolute native slot-address range, but none is a bracketed absolute slot-memory operand: they occur as a command-record constant, a bit mask and C++ exception magic values. Relative-range matches include the already identified indexed-table fields, an unrelated part-UID registry around `00494300` and ordinary numeric constants. `00BAF303` computes a numeric allocation argument for another object; it is not a slot-table base.

The sweep and validated literals reduce the chance of missed direct accesses. They are not a complete proof against every computed pointer alias or indirect call. Disassembly boundary fragments in selected ranges are excluded from conclusions.

## 1. Structure and handle representations

Let `R = 01732BF0`, the native global resource-reference manager used by the relevant callers.

| Field | Absolute address | Role |
|---|---|---|
| `R+127AC8` | `0185A6B8` | Native pending-reference pointer array; currently redirected by ASI |
| `R+137AC8` | `0186A6B8` | Native pending snapshot pairs; currently redirected by ASI |
| `R+157AC8` | `0188A6B8` | Pending count, kept in its native location |
| `R+15E090` / `R+17E090` | — | Separate deferred resource cleanup list/count |
| `R+17E094` | `018B0C84` | First indexed slot |
| `R+27E08C` | `019B0C7C` | Final indexed slot |
| `R+27E094` | `019B0C84` | Indexed free-head pointer |

There are 131072 slots of eight bytes each (1 MiB).

Before conversion, a reference location `H` holds its two-word resource key. During successful conversion, `00BAE790` writes:

```text
H.word0 = resolved resource-object pointer
H.word1 = 0x80000000 | slot_index
```

The slot contains predecessor/successor **reference-location pointers**. The resource hash payload has the head of that reference chain. Several locations referencing the same resource can therefore consume multiple slots. A slot does not contain the vehicle model itself, a GPU buffer or a simple loaded-resource ID.

The inspected consumers test bit 31 to select direct-pointer versus UID lookup. Slot-index extraction uses `0x7FFFFFFF`, not a 17-bit mask. This leaves representational space for an illustrative 262144-entry table; the native neighbor bound of `0x20000` still needs changing. This is a conclusion about the inspected format, not a claim that any arbitrarily large table is safe.

`006BDCA0` also resolves a UID into a pointer and sets its second word to `FFFFFFFF`. This demonstrates a separate direct-pointer form without a normal new tracking index. Callers `00BA9FD0` and `00BC3790` select immediate resolution versus pending registration using the parameter-list object's `+20` mode. Do not treat every bit-31-set pair as an ordinary allocated indexed slot in future diagnostics. Ordinary invalid-pair checks and lifecycle discipline still matter; this pass does not prove that every immediate-mode owner avoids indexed cleanup.

## 2. Allocation and what a full table blocks

Verified path in `00BAE790`:

1. Read pending handle address `H` and its saved pair.
2. Inspect the current pair. A UID-form pair is looked up through the resource hash; an already direct form is handled separately.
3. At `00BAE839`, read native free head. `00BAE841` leaves the entry pending if it is null.
4. With a free slot and a matching resource/snapshot, pop the slot at `00BAE85A/00BAE85C`.
5. Compute the slot index, publish the direct pointer/index pair at `00BAE888/00BAE88A`, and link the handle location into that resource's chain.
6. Remove the processed pending entry by replacing it with the final array entry.

The null-head test occurs **before** branches at `00BAE854` and `00BAE858` that can remove an entry without allocating another slot. Thus exhaustion can also keep already-converted or changed-snapshot records in the pending collection. The capacity log's pending count is not a count of unique unresolved resources, and must not be described as that many missing textures/meshes.

An unavailable resource can independently leave a reference pending. The trace does not classify the pending entries by these reasons.

The allocation path still has the previously documented malformed-neighbor hazard: it publishes the handle and pops a slot before validating an existing resource-chain head's derived index at `00BAE8A9`. Rejection can skip final slot linking. This is distinct from a null free head. An expansion should not silently change that behavior as part of a capacity comparison, and malformed chain values remain a separate correctness concern.

## 3. Return, ownership and replacement paths

### Individual reference removal: 00BAEA80

- For UID-form references, it searches/removes matching pending records. This explains one way the pending count can fall while the free head samples remain empty.
- For indexed/direct references on the indexed branch, it obtains the resource record, derives the slot and neighbors, unlinks the reference and pushes the slot onto the free list at `00BAEBCC/00BAEBCE`.
- The routine does **not** universally clear the caller's pair. Correct setter/destructor ordering is therefore important. A retained stale pair or an unregistered copy cannot be assumed safe from the existence of the unlink routine alone.

Examples of lifetime discipline:

- Setters `00BAA290` and `00BAA300` unlink the previous handle before replacing it and appending the new reference to the pending collection.
- `00BA97A0` walks a resource parameter table and unlinks type-5 handle fields (16-byte stride).
- The previously implicated cleanup caller `00BC5380` unlinks an outer handle at `00BC53BE`, then calls type-5 cleanup at `00BC53C7` before freeing the allocation. This links the known crash area to reference lifecycle, without identifying a leak or the first bad writer.

### Resource reference count and release

Import-table inspection identifies:

| IAT slot | Imported function |
|---|---|
| `00DC915C` | `KERNEL32!InterlockedDecrement` |
| `00DC9160` | `KERNEL32!InterlockedIncrement` |
| `00DC9168` | `KERNEL32!InterlockedExchange` |

`00BAE9B0` increments the resource's separate counter at object `+8`. `00BAECA0` decrements that counter. If it remains nonzero, resource-wide removal does not proceed.

On the zero-count removal path, `00BAECA0` traverses the reference chain. It writes the original UID pair back into each handle location and returns its indexed slot through `00BAED21/00BAED23`. It advances through slot second words at **`00BAED08`**, then schedules resource cleanup and removes the hash record.

This shows why indexed slot occupancy and resource reference count are different quantities. Keeping the larger vehicle working set resident can also keep many reference registrations alive; static code alone cannot decide whether the observed population is expected or erroneous retention.

### Resource replacement: 00BAEBE0

This updates the resource payload and rewrites registered handle locations to the replacement resource pointer while preserving their slot indices. It advances the chain through **`00BAEC7F`**. Relocating only the insert/unlink paths would leave this path reading the old inline array and could strand or corrupt references.

No definite missing release was established in these examined normal paths. That does not rule out a caller-lifetime bug, an unregistered copy, a malformed chain or a runtime retention imbalance elsewhere.

## 4. Graphics readers have a UID fallback

This is an important qualification to the runtime result.

`00BB42E0`, a graphics command enqueue path, accepts handle pairs. At `00BB42EF..00BB4328`, a direct form uses the pointer; a UID form calls `00BAE770` at `00BB4313`, obtains the object from the hash payload and continues. Its material argument has the equivalent fallback at `00BB4388`. The path ultimately enqueues via `00BB5A50` at `00BB4402`.

`00BAB930`, which accesses the resolved object's buffer-related fields and invokes subsequent graphics methods, likewise resolves a UID through `00BAE770` at `00BAB96B`. `00BABAA0` is another helper that performs the same distinction before updating the resource's graphics-use stamps.

Neither hash lookup nor the inspected fallback tests indexed free-head availability. Therefore a reference can remain unconverted/pending while its resource can still be found and used by these paths. Conversely, if the actual resource or dependency is absent, a fallback lookup need not succeed. These observations do not identify the user's exact world-body path or prove GPU readiness.

**Consequence:** expansion may restore reference conversion, reduce repeated lookup/pending work and possibly improve the reported symptoms. It cannot be promised to make every invisible body appear. Audio playback remains outside the demonstrated connection.

## 5. External relocation inventory

Candidate changes are validated original DWORD operands, not patches made by this task. Let `E` be an external slot-array base, `D = E - R` modulo 32 bits, and `N` the new slot count.

| Instruction VA | Original operand | Replacement meaning |
|---|---|---|
| `00BAE86E` | `17E094` | `D`: subtract the new base displacement to encode index |
| `00BAE8B3` | `17E094` | `D`: insertion neighbor slot |
| `00BAE968` | `17E094` | `D`: constructor first slot |
| `00BAEB5D` | `17E094` | `D`: unlink first link |
| `00BAEB64` | `17E094` | `D`: unlink current slot |
| `00BAEB80` | `17E094` | `D`: predecessor slot |
| `00BAEBA3` | `17E094` | `D`: successor slot |
| `00BAED0F` | `17E094` | `D`: resource-removal slot |
| `00BAEC7F` | `17E098` | `D+4`: replacement traversal next reference |
| `00BAED08` | `17E098` | `D+4`: removal traversal next reference |
| `00BAE9A3` | `27E08C` | `D + 8*(N-1)`: final free-link terminator |
| `00BAE984` | `1FFFF` | `N-1`: constructor link iterations |
| `00BAE8A9` | `20000` | `N`: valid neighbor-index bound |

The seven native free-head references at `R+27E094` can remain at their existing metadata address. The resource-manager owner and pending-array/count layout also remain where they are. The existing 31-site pending-array redirect must be preserved independently.

The native slot region ends at `019B0C84`; graphics-manager global `019B0CD0` begins only **76 bytes after that boundary**. Other state occupies the intervening area. A larger count without relocation is unsafe.

An illustrative `N=262144` requires a **2 MiB external allocation**. The original 1 MiB image area remains reserved; this is not merely a 1 MiB net allocation. This count is a controlled-test proposal, not a measurement of exact required headroom. The current saturated log cannot reveal the true unsaturated peak.

Fifteen offline arithmetic checks exercise three possible external bases, including wraparound owner-relative displacements and indices `0`, `1`, `131071`, `131072`, `262143`. They preserve index/address correspondence. No ASI was built and no actual external memory was allocated in the game.

## 6. Startup and integration conditions

Native constructor `00BAE930` is called with `ECX=01732BF0` from `00DC3F50`, whose pointer occurs in the initializer sequence at `00DD6298`. It registers `00DC7F30`, which is a return-only destructor stub. No second direct constructor caller was found.

The current ASI's load timing relative to that native constructor has not been proved from the authorized files. A future implementation must explicitly handle or reject the startup state:

1. **Native manager not initialized:** validate cold state, install all operand changes, then allow its constructor to build the external free list. Keep the allocation alive for the process.
2. **Manager initialized but all slots still free:** while still at the validated early startup point, verify the complete native free chain and no pending/active references, initialize the external free chain and switch the head consistently with the operand changes.
3. **References already allocated or startup state ambiguous:** refuse an early-only patch. Do not reinitialize or blindly copy an active table. Moving the slot array also requires rebasing its free links while preserving occupied reference links and all encoded indices; this task does not validate a live migration.

A no-patch fallback is preferable to installing only some of the operand changes. A future implementation needs whole-instruction signature checks, coordinated installation and rollback, and compatibility checks with the current baseline detours. No current 150/160/ED/EF behavior should be changed to hide a failed install.

The current capacity sampler's valid-head range is still the original inline table. It would flag an external head as invalid and fail to qualify initialization. **Any expansion test must update its reported base/range/capacity together with the relocation**. Old handle-lifetime diagnostics also contain fixed 131072 assumptions and must not be reused unchanged.

## 7. Recommended next experiment

A narrowly scoped external-table capacity test is now justified by the direct free-head exhaustion and the bounded relocation map. Preserve 150 requests, 160 base vehicle slots, ED/EF, existing pending-array/hash expansions and the quiet monitor. Change only the indexed-table backing/capacity, startup validation and the sampler's corresponding bounds.

Compare the same save/route with run 16316:

- Does the new table retain available slots?
- Does the pending collection drain or substantially shrink?
- Do invisible bodies recover sooner or stop occurring?
- Do the reported audio categories recover, independently of the graphics result?
- Does normal driving remain near the quiet build's observed FPS?

If slots remain available and the backlog drains but symptoms persist, table exhaustion is not a sufficient explanation; follow the actual missing body dependency or audio event path. If the larger table eventually runs empty, additional data about handle ownership/retention would be needed before another increase.

No broad new lifetime trace is required before implementing this guarded test. If startup cannot be established safely or a native lifecycle invariant fails, stop the expansion work and use a narrower diagnostic for that specific ambiguity.

## Reproduce

```powershell
python scripts/research_indexed_handles.py
```

This verifies the executable identity, inventories literals, creates focused disassemblies, performs the filtered code sweep and checks relocation arithmetic. It never writes to, loads or executes Driver.exe or an ASI.

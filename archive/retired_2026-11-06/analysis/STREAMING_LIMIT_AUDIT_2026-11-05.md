# DriverSF streaming capacity audit — 2026-11-05

## Result

The current expansion does not establish that every streaming capacity supports 150 resident vehicles. Direct inspection of the exact executable confirms several native 50-node structures outside the expanded base vehicle list. A newly identified audio bank queue is particularly relevant to the audio symptom. None of their runtime overflows is established by this static audit.

The separate indexed resource-handle table also retains its native 131072 slots. Its reconciliation routine explicitly leaves work pending when its free head is null. This supplies a concrete deferred-work mechanism, but a connection to missing world-body geometry remains unresolved.

No game code, ASI, project settings, request counts, capacities or installed files were changed. Only the read-only analysis helper, this report, evidence files and the handoff were added/updated. No build, game run, debugger attachment, dependency installation or web search was performed.

## Scope and provenance

- Executable: `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, 12773688 bytes.
- SHA256: `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.
- PE image base: `00400000`; Large Address Aware flag is **clear**.
- Vehicle archive: `Install\dngvehicles.sp`, 483110912 bytes.
- Archive SHA256: `0DDC885E23028F708A92A2426B643938A6934C99204C780E01755C5B7778AE35`.
- Project comparison: current `DriverSF_V29s_ORDERED_150_PERF.vcxproj`, quiet 45 project, and the relevant loading/patch source.
- Game-file inspection stayed within the explicitly authorized executable, `Install` and `media`. `media\product_config.yml` contains launcher/product settings; it did not supply a streaming budget. Other game folders were not inspected.
- Evidence directory: `streaming_static_2026-11-04/` next to this report. The directory retains the date on which the work started; completion is November 5.

The helper uses Python's standard library for PE/container metadata and the already installed Microsoft `dumpbin` for disassembly. Relative-call and immediate-reference searches produce **candidate byte matches**; relevant callers below were validated against disassembled instructions. Some evidence ranges begin/end inside neighboring instructions; incomplete boundary instructions are not used as evidence.

## 1. New finding: audio bank submission queue has 50 inline nodes

Let `A` be the audio manager reached through `[Engine+0x50]` in `0084BF30`.

| Field | Meaning established by code |
|---|---|
| `A+92DC` | allocation/free cursor |
| `A+92E0` | recycled-node count |
| `A+92E4` | active-node count |
| `A+92E8 .. A+9540` | 600 bytes of inline nodes, **50 × 12 bytes** |
| `A+9540`, `A+9544` | doubly linked list sentinel |

Constructor `008498E0` initializes the node cursor at `008499DD/008499E3`, zeros both counts, and initializes the sentinel at `008499C9..008499FB`.

Verified path:

1. Bank loading at `0084BF30` records a bank pointer and byte size, sets bank state 1 at `0084BFBE`, then calls **`00849870` at `0084BFDD`** to append the bank to this queue.
2. `00849870` reuses a free node if available. Otherwise it writes the bank pointer at `node+8`, advances the cursor by 12 at `008498A9`, increments the active count, and links the node.
3. The fresh-node path has no comparison with `A+9540`, no 50-entry check and no allocation of a larger block. If 50 nodes have been consumed without a reusable node, the next node address aliases the list sentinel. That is a structural overwrite, not a graceful fallback.
4. The audio update loop at `0084A16B..0084A1C2` visits the queued banks. State 1 calls Wwise submission wrapper `0084BEB0` at `0084A1B8`. Submission changes state to 2.
5. Callback `0084BE70` sets state 4. The update loop removes state-4 nodes and returns them to the free list at `0084A18B..0084A1A9`. Immediate submission failure also sets state 4, with bank ID zero.

Thus this is a limit on **outstanding queued bank operations**, not a limit of 50 loaded banks. Entries remain active across asynchronous submission until state-4 retirement, or another observed cancellation/removal path such as `008497D0`.

No adjustment of `00849870`, the constructor cursor, or this queue's fields was found in the current patch source. The existing five audio trace hooks observe downstream submission/callback/lookup and CHNK opening, not this queue's insertion peak or boundary crossing.

**Interpretation:** This is a concrete unexpanded capacity that could damage audio-manager state during a sufficiently large loading burst. The matched totals of 68 or 153 submissions/callbacks cannot establish concurrent occupancy. In particular, neither 150 requested cars nor 68 submissions proves a peak above 50. It could affect the quiet-45 run if other bank work overlaps, but that overlap has not been observed. No causal link to rendering is established.

Evidence: `008498E0-00849A70.asm.txt`, `00849000-00849A00.asm.txt`, `0084BE30-0084C270.asm.txt`, `0084A140-0084A240.asm.txt`.

## 2. List A: 50-node retirement/wait list remains unexpanded in quiet projects

Let `S` be the traffic spooler. Constructor `005A6D20` initializes:

| Pool | Header | Inline nodes | Sentinel | Capacity |
|---|---|---|---|---:|
| Base request set | `S+CC` | `S+D8 .. S+330` | `S+330` | 50 originally; current source expands its allocation path |
| List A | `S+338` | `S+344 .. S+59C` | `S+59C` | **50 × 12 bytes** |
| List B | `S+5A4` | `S+5B0 .. S+998` | `S+998` | **50 × 20 bytes** |
| Queued operations | `S+9A0` | `S+9AC .. S+D94` | `S+D94` | **50 × 20 bytes** |

Reconciliation `005A7490` has two distinct copies through `005A7D50`:

- `005A775D`: destination `S+338` (List A), source `S+CC`.
- `005A7769`: destination `S+CC`, source the temporary list of newly requested IDs.

The current merge fix redirects only the **second** call to a helper with widened source-layout offsets. It does not enlarge List A's destination pool.

The first call must be interpreted after the preceding loop: IDs retained in the new request set have already been removed from the old base list and copied into the temporary list. The old base list at the first copy therefore contains departing IDs, subject to the special retained/current-ID handling. List A receives this retirement set.

`005A65A0` confirms the interpretation: the base list is checked for include-map membership, whereas List A entries are retired only once `0059CFC0` returns false (`005A661D..005A6644`). This is manager membership bookkeeping, not drawable readiness.

At `005A7DD1..005A7E25`, appending to List A reuses freed nodes or advances the native cursor by 12. There is no end-capacity check or dynamic growth. The 51st unrecycled fresh node aliases `S+59C`; its payload write at `005A7E07` reaches `S+5A4`, the following pool's header.

**Trigger:** a request-set change that requires more than 50 simultaneous retirement entries, including any still-pending retirements carried forward. An identical 150-ID request does not imply 150 retirements: shared IDs are removed from the old list before this copy. Initial loading from an empty set also does not, by itself, fill this retirement list.

`DSF_V29S_LISTA_EXPANDED` implements an optional 512-node path, but neither quiet project defines it. Do not merely enable it without checking lifecycle and all relevant writers; this audit changes no flags.

Evidence: `005A6800-005A7F00.asm.txt`, `005A65A0-005A6780.asm.txt`; source `V29sNativeLoadingTrace.inc` near line 695; `InstallMerge150Fix` in the main source.

## 3. List B and queued operations have separate 50-node capacities

### List B: retained vehicle records

The pool at `S+5A4` holds 20-byte linked nodes with three DWORD payload fields. Its sentinel is `S+998`. Searches/refcount changes occur in `005A6810`, `005A6850` and neighboring functions.

The state machine at `005A7B80`, state 2, checks current kind-0 ID membership at `005A7BD4`, then appends the current record to List B through `005A7E30` at `005A7BE8`. State 3 can remove a corresponding record through `006D2710` at `005A7C62`. Reconciliation also frees matching List B entries at `005A77B0..005A77CE`.

The insertion wrappers `005A7E30` and `005A7E60` call `0083B170`, which has a free-list path and an unchecked fresh cursor advance of **20 bytes** at `0083B1BE`. There is no 50-node capacity check. The optional `DSF_V29S_LISTB_EXPANDED` implementation checks the header specifically against `S+5A4`; it is not enabled in either quiet project.

These records are a distinct retention/operation path. Bulk-preloading 150 IDs does not establish that 150 records enter List B. This is a real capacity with an unmeasured workload, rather than a demonstrated 150-car blocker.

### Operations queue: another pool at S+9A0

This has the same 20-byte node allocator, but separate storage and free-list state. Examples:

- `005A6FD0` queues an operation through `005A7E30` at `005A7062` or `005A70DE`, with header `S+9A0`.
- `005A7100` queues another operation at `005A716D` through `005A7E60`.
- `005A6850` can append at `005A68A2`.
- `005A7A50` copies the next operation into the current-operation record at `S+D9C`, returns its node to the queue's free list, and dispatches according to its operation field. State-machine progress controls when subsequent operations are handled.

The admission calculation at `005A7093..005A70BD` uses net queued additions/removals (`005A6480`), retained/base counts, and package capacity (`0059C5C0 -> 005A0AD0`). It does **not** compare the operation queue's occupied-node count against 50. Increasing the package capacity therefore does not itself protect this physical queue.

The current optional List B hook would not cover `S+9A0`: its owner/header filter accepts only `S+5A4`. No expansion for this fourth pool was found in the examined current loading/patch source.

**Interpretation:** bursts of queued operations or a stalled current operation can increase queue residence and occupancy. Static inspection establishes the fixed capacity and lack of a node-bound check; it does not establish a runtime peak above 50 or a connection to one delayed car.

Evidence: `005A6800-005A7F00.asm.txt`, `005A6300-005A6500.asm.txt`, `0083AF00-0083B450.asm.txt`, `0059C5C0-0059C680.asm.txt`.

## 4. Indexed resource handles: native capacity and actual full-table behavior

For resource-manager owner `R`:

- Inline slot array: `R+17E094`, **131072 × 8 bytes**.
- Last slot begins at `R+27E08C`; free-head metadata is at `R+27E094`.
- Constructor `00BAE930` links slots at `00BAE984..00BAE99C`, using `0x1FFFF` steps, and terminates the last link with zero.
- Indexed handles use bit 31 plus the slot index. Neighbor validation at `00BAE8A9` compares against `0x20000`.

`00BAE790` processes pending resource-reference entries. At `00BAE839` it reads the free head; `00BAE83F/00BAE841` tests it and branches to `00BAE902` if null. That path advances the scan without decrementing the pending count. A successful processed entry takes `00BAE8CD` and removes the pending entry by replacing it with the final entry in the array.

Normalized behavior:

```text
for each pending reference:
    resolve its referenced resource
    if free_head == null or resource is unavailable:
        leave this reference pending
        advance to the next reference
    else:
        perform the applicable reconciliation/link work
        remove the processed pending entry
```

The call at `00699267` invokes reconciliation. This audit does not establish its cadence during the reported invisible interval. Returning a slot through ordinary unlink or alternate removal can permit later progress; the routine does not dynamically grow the table or invoke an eviction policy on the observed null-head branch.

The current **resource pointer/pair collection expansion** changes pending-array storage. It does not relocate this inline indexed table or its free-head metadata. The 45000-node resource hash arena, with current external overflow storage, is another separate structure.

**Interpretation:** table pressure can defer reference reconciliation. It is not established that every deferred reference prevents drawing: this audit has not completed the path from one pending body dependency through lookup to final world-body submission. Old `slotBalance` values are diagnostic ledger values, not a substitute for observing the actual native free head. The `LIFE_REJECT` branch checks malformed neighbors and must not be relabeled as empty-free-list rejection.

Evidence: `00BAE700-00BAEE80.asm.txt`, `006991C0-006992A0.asm.txt`, `00BAA260-00BAA3D0.asm.txt`.

## 5. Installed archive and finite memory

Metadata was read directly from the installed `dngvehicles.sp`; no resource was changed or unpacked into the game folders.

| Check | Measured file value | Interpretation |
|---|---:|---|
| Root CHNK entry count | 453 | Below documented native 1100-entry reader capacity |
| VEHC entries | 450 | 150 vehicle types × three entries |
| SSLP index records | 450 | Matches the root vehicle-entry set |
| SSLP slot bytes | 35 / 2 / 2 / 0 | Disk metadata remains stock; current ASI changes kind-0 admission in memory |
| NONR buffer sizes | 446464 / 4096 / 4096 bytes | Per-kind settings read by native handler initialization |
| Largest NONR, kinds 0 / 1 / 2 | 446000 / 1152 / 1568 bytes | All fit their respective configured buffers |
| Largest examined DRSP child count | 382 | Below 1100; does not survey every nested format inside GPCH |
| Largest VEHC | 1650688 bytes | Below documented 2.5 MiB shared-buffer threshold, including ordinary small alignment overhead |
| Vehicle SBAK entries | 150 | Sum 40093005 bytes, approximately 38.24 MiB |
| Shared SSIC SBAK | 1289652 bytes | Additional shared bank payload |

The bank-byte sum is archive payload size, **not** measured Wwise memory or a predicted additional allocation. Some bank data are referenced in place, and the exact native path can allocate another buffer. Do not double-count it against NONR memory without tracing ownership.

160 kind-0 handlers configured with `0x6D000` NONR buffers imply **68.125 MiB of those buffers alone**, assuming all allocations succeed. Both quiet 45 and quiet 150 create the expanded handler set, so this fixed initialization cost alone does not explain their differing symptoms. Actual loaded geometry, resources and audio work can still differ with request count.

The exact executable is not Large Address Aware. Finite 32-bit process address space and allocation failures remain audit targets; neither actual peak address-space use, fragmentation nor a specific memory-allocation failure was measured. This finding does not authorize changing executable flags or budgets.

`00857270` independently confirms a **32-entry file-wrapper limit**, returning `FFFFFFFF` when its occupied-slot scan reaches 32. This is a concurrent file limit, not 32 vehicle types. Existing matched-run findings report no failures on the observed CHNK-open path. The serialized CHNK worker and its scheduling were documented previously; this audit does not measure their wait time.

Evidence: `package_metadata.json`, `executable_identity.json`, `00857270-00857360.asm.txt`, and the prior focused spool-format audit.

## 6. Priorities and practical boundaries

1. **Audio queue:** audit/measure its actual active count and cursor against the 50-node boundary across loading bursts. It is directly upstream of the already observed Wwise submissions, and current audio traces miss it.
2. **Roster retirement:** establish whether the user's route/save causes more than 50 simultaneous List A retirement entries. Existing 150 steady-state occupancy is insufficient evidence.
3. **Operation/retention pools:** establish peaks and lifetimes separately for `S+5A4` and `S+9A0`; a patch for one does not expand the other.
4. **Indexed handles:** correlate a real null free head and a named pending body dependency with the body-reveal interval before considering relocation. Native full-table behavior is now clear; downstream drawable impact is not.
5. **Memory and final resource publication:** follow specific allocation failures or resource substitutions if the preceding boundaries stay safe. No single global GTA-style resident-byte budget was established in this bounded pass.

Any future expansion needs to cover initialization, every fresh-node writer, recycling, reset and destruction, with owner identity preserved. The existing optional code should not be treated as a validated remedy simply because its capacity is larger.

Static conclusions are strongest for **capacity, layout and branch behavior**. Whether a particular capacity is reached in a faulty session remains a runtime question. This report does not establish one shared root cause for audio and rendering, does not prove 150 vehicles READY/drawable, and does not claim a complete enumeration of every engine limit.

## Reproduction

From the workspace, with the same executable and installed Microsoft tools:

```powershell
python scripts/static_streaming_audit.py identity
python scripts/static_streaming_audit.py packages
python scripts/static_streaming_audit.py disasm 005A6800:005A7F00 005A65A0:005A6780 0083AF00:0083B450 00BAE700:00BAEE80
python scripts/static_streaming_audit.py disasm 008498E0:00849A70 00849000:00849A00 0084BE30:0084C270 0084A140:0084A240
```

The helper writes analysis evidence only under the workspace. It never runs or writes the target executable.

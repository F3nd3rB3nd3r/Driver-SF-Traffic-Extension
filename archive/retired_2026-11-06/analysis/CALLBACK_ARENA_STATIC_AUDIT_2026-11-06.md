# Native callback arena: static writer, consumer and lifetime audit

## Result

The separate64-node callback arena is a justified capacity-extension target. Its native insertion routine writes beyond inline storage without a capacity check; its consumers follow pointers and use DWORD counts, so no additional64-entry consumer cutoff was found. A scoped extension can retain native headers/sentinels and supply additional nodes. Changing a numeric limit or the global314h stride alone cannot enlarge the existing allocation.

There is enough static evidence to design a combined extension/census test. Suggested initial test capacity is512 nodes per group, retaining64 native nodes plus448 private nodes. This is a test allowance, not measured demand or a universal per-vehicle ratio. A probe-only run that knowingly permits the65th native overwrite is unnecessary.

The prior dump's exact malformed iterator remains strongly consistent with adjacent-list overflow. The two CHAPTERS logs lack exception contexts and callback counts, so this audit does not establish that all three failures share a cause. Expanded storage will not independently repair unrelated callback object lifetime faults or the audio setup failures recorded after setting changes.

## Scope, identity and reproducibility

Read-only analysis of the already authorized `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256 `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`, current relevant source, and the existing supplied-dump analysis. No other user folders, external documentation references, old logs or git history were scanned. No web access, debugger, target execution, build, ASI installation or production-source change.

Reproduce the executable signatures, bounded native disassemblies, literal-reference census and Python decoded-operation replay:

```powershell
python scripts/audit_callback_arena.py --output analysis/callback_static_2026-11-06 --census
```

Result: passed. Evidence: `analysis/callback_static_2026-11-06/audit_summary.json` and `*.verified.asm.txt`. The replay executes Python memory operations corresponding to decoded instructions. It does not execute native instructions or a production hook. The reference census decodes the executable only; it is not a workspace scan. Linear disassembly/literal references cannot exhaustively disprove computed/indirect references.

Current CHAPTERS_R1 ASI hash remains `602AA57D7F5464FAA3456499AC3009A07E854E76FBC2B8D6264C2CCD1B229442`.

## Geometry and meaning of the limit

Native constructor00DB40A0 initializes21 groups (indices0..20):

`header(i) = 014427EC + i * 0314h`, exclusive arena end01446890.

| Offset from header | Meaning |
|---:|---|
| +00 | Recycled-head or next fresh-node pointer |
| +04 | Recycled/free node count, DWORD |
| +08 | Active node count, DWORD |
| +0C..+30B | 64 inline12-byte nodes |
| +30C /+310 | Native sentinel previous/next links |

Each active node contains previous/next pointers and a DWORD callback-object pointer at+8. The64 comes from `(30C-0C)/0C`, not an8-bit count or a single configurable compare instruction. All21 groups together occupy16,548 bytes.

This is storage for **outstanding callbacks**, grouped by the DWORD selector at resource handler+560. Multiple resource handlers with the same selector share a list. One node is one queued callback pointer, not necessarily one car model, vehicle instance or sound. Names for all21 selectors and a callbacks-per-vehicle formula are not established by this audit.

## Registration, polling and recycling

| Function/site | Established behavior |
|---|---|
| 008589C0 /008589F0 | Handler setup methods store a caller-supplied DWORD selector at+560; no local0..20 check. |
| 00858F80 | Resets handler state, conditionally invokes current callback virtual+8, then appends its pointer to the selected global list through00858FC8. Sets handler+56D after insertion. |
| 005567D0 | Copies `*argument` into node+8. Uses recycled head when free count is nonzero; otherwise advances fresh pointer by12. Increments active count and appends to native sentinel. No capacity/end comparison or new allocation. Returns node in EAX, RET4. |
| 00859000 | Selects the same group using handler+560. Walks sentinel.next and invokes each callback virtual+0C. False keeps node active; true unlinks and recycles it. Traversal is sentinel-driven, with no64-iteration limit. |
| 0085903F..68 | Recycling updates both active neighbors, chains the node onto header+0, increments free count and decrements active count. Original node memory is reused; no private allocation/free dependency. |
| 00858A30 | Handler completion wrapper additionally requires this group's active count to be zero. |
| 00859073..9F /008590E3..F6 | Further state/flag transitions wait for the selected group's count to become zero. |

The fresh pointer must be retained at the tail of the recycled chain. When the last recycled node is consumed, native insertion restores that saved fresh pointer. Treating header+0 as an unconditional fresh cursor would break this behavior. The replay specifically checks it.

Native polling reads the next link after callback execution. It may encounter work appended while a callback runs; an extension must preserve native order and reentrant behavior. No serialization guarantee was established. Do not hold a new non-reentrant lock across callback virtual calls or infer thread safety from this static audit.

Polling does not explicitly delete or release a callback pointer when recycling its list node. Storage ownership and callback-object ownership must remain separate. The earlier dump already shows why skipping a malformed callback or calling through an unchecked payload is unsafe; an extension should diagnose corruption, not silently discard entries.

## Shared helper callers and scope restriction

Decoded direct callers of005567D0 are exactly four in the supported executable:

| Call | Context | Treatment for proposed patch |
|---|---|---|
| 0055690F | 005568F0 paired sorted query/result lists: floating-point ordering companion and pointer-result list, supplied by caller. Other branches manipulate those lists inline and include their own64 checks. Callers00556C83/00556D79 use position/distance calculations. | Preserve original behavior for headers outside the global callback arena. Do not globally rewrite its64 checks or inline layout. |
| 005A399A | 005A3870 constructs and fills a temporary stack list while comparing resource controllers, then walks it. ECX is a stack-derived header. | Preserve. This is not the global callback storage. |
| 00858FC8 | Explicit ECX=`014427EC + handler.selector*314h`; payload source is handler+55C. | Confirmed target writer. |
| 008CCF4C | 008CCE80 radius/predicate query appends pointers to a caller-supplied result list. | Preserve for non-arena headers. |

No absolute byte reference to the helper's entry address was found. This supports the decoded direct-call census but is not a proof that indirect calls are impossible. The target header must be identified by exact membership/alignment in the21 global headers, rather than assuming every caller of the helper is a streaming callback writer.

The literal arena census contains only nine decoded instructions:00858A4A,00858FC2,00859013,0085901D,00859023,00859053,00859088,008590EF and00DB40A5. Their functions cover the initializer, confirmed registration, active-count consumers, traversal and recycling. Other314h arithmetic at0078A5BB/0078A6D2/0078A705/0078ADE7 belongs to object-relative lists and must not be changed. Raw byte searches produced many instruction-overlap false positives; none were treated as independent writers. One non-executable raw candidate at unaligned00F10D71 overlaps the aligned00F10D70 DWORD442F0000 and adjacent bytes; it is retained as a byte candidate, not asserted to be a pointer to a global callback list.

The current relevant V29s source contains no existing005567D0/global-arena extension. The diagnostic reference to00859000 is not an installed capacity fix for this list.

## Reset and storage lifetime

The global initializer has no decoded direct E8 caller; its absolute pointer is at00DD50F4, consistent with static initialization. Handler constructor00859260 initializes its own fields, including+560, and does not reconstruct the21 global headers. Handler reset/cancel00858C90 changes local state and callback flags; handler destruction00858B60 processes its member lists but does not directly reset the global callback arena. No additional literal global-reset path was found.

Therefore the proposed extra-node storage should have **process lifetime**, retaining ownership even when a group becomes empty. A recycled chain can still point into it after all active callbacks finish. Freeing or replacing it on challenge change, traffic-owner change or menu return could create a use-after-free. Do not copy the pending512 per-map generation policy onto these shared global lists without a separate reset requirement.

Installation must account for loader order: native CRT initialization can occur after ASI DLL initialization. Native initialization must not overwrite a preinstalled external cursor while extension metadata assumes it remains attached. Lazy attachment at the validated first inline exhaustion avoids needing to patch the global constructor. Startup signatures and pristine/header checks remain necessary; do not migrate an already corrupt list.

## Feasible extension design

Retain all native headers, selector arithmetic, sentinel locations and64 inline nodes. At a recognized header's insertion boundary, validate active/free chains and pointer ownership. If recycled count is zero and the fresh cursor has reached that header's sentinel after64 issued nodes, redirect it to448 private12-byte nodes for that group. Native insertion/linking and native polling/recycling can then operate on those nodes using their existing pointers.

For21 groups the added node payload is `21 * 448 * 12 = 112896` bytes (110.25KiB), plus metadata/guards. There is no reason to enlarge the number of groups, change314h strides or move sentinels for this design. Keep all unrelated helper calls native. Check handler selector range at the confirmed writer before header arithmetic is trusted; membership filtering of the generic helper alone would otherwise pass an out-of-range computed header back to unsafe native code.

The guard must stop before an exhausted private cursor is dereferenced. Native callers expect successful insertion/node return; silently dropping callbacks or returning a fabricated success is not a compatible overflow policy. A controlled diagnostic/failure path must be designed with the implementation. Preserve register/stack/EAX/RET4 behavior and displaced instructions. Maintain process-lived allocations and handle failed allocation/guard/signature checks before mutation.

Suggested per-group diagnostics: active/recycled/issued/peak, first64-boundary crossing, extra-node usage, fresh/recycled inserts, guard/ownership/cycle/capacity failures, and bounded recent handler/payload registration information. Counts should distinguish live demand from lifetime registration totals. Avoid high-frequency per-callback text output. Preserve150 requests/160 graphics slots/ED+EF and all current fixes.

## Replay verification and remaining work

Python replay follows decoded005567D0 and0085903F..68 memory operations, plus the proposed fresh-cursor guard. It passed:

- All21 groups filled to512 simultaneously with original sentinels, neighboring headers and private guards intact.
- Proposed513th-insert guard stops before memory mutation.
- Alternating removal and256 recycled reinserts retain correct links/counts.
- Complete drain and512-node refill preserve saved fresh cursor and use recycled nodes.
- Partial recycling before crossing64 reaches150 active entries while the neighboring group remains pristine.

These checks establish internal consistency of the design model. They do not validate a compiled hook, callback side effects, native ABI, concurrency, demand peaks or live crash resolution. Before staging a test ASI, use an isolated Win32 native fixture for the exact native insertion/polling bridge, exercise non-target callers and failure paths, and run the inherited verifier/startup checks. Then use the same challenge-switch sequence with per-group counts, followed by menu/reload/exit validation. A new live dump is helpful if a crash recurs but is not required to begin implementation from this audit.

The supplied old dump's pool10 malformed iterator is still explained by the documented conditional pool9 overflow replay. Actual occupancy/first write remains absent. The new extension should test that hypothesis without claiming a guaranteed crash fix. Audio setup-return tracing and parked-car fallback remain separate unresolved investigations.

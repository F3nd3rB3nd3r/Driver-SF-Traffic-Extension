# Driver.dmp: streaming-list cycle and an unexpanded pending map

The full dump is usable. It directly captures the main thread in a streaming-list loop whose next links form a12-node cycle and never reach the sentinel. The strongest upstream cause is another missing storage expansion: package0's pending-handler map still has64 inline16-byte nodes. Its live/recycled nodes extend into neighboring structures, and its node114 aliases the corrupted include-map header exactly. The completed-handler map is expanded; this matching pending map is not covered by the current insertion bridge.

The cleanup queue is empty, outside a drain, with intact guards. No further cleanup-capacity increase or second dump is needed to identify this next target. The first historical corrupting store is not captured, so distinguish the proven cycle from writer attribution supported by layout and native insertion code.

## Artifact and analysis method

Input: `dist/MAIN_150/Driver.dmp`,890009660 bytes, SHA256 `0AB99CDE773596BA7358D912B08A1A20BC3A5D1BA53DFE3BF1942013CE699FE5`. PID19792; this is a separate run from logs15948/22352. The exact matching19792 log was not present at its expected filename when checked. Dump memory is sufficient for the findings below.

The dump contains49 threads,2073 memory ranges and889622528 captured memory bytes. Read using Python memory mapping, not a whole-file byte allocation. All work is offline: no target execution, process attachment, debugger, dependency installation, web search or other user/game-folder discovery.

This is a64-bit collector's WOW64 dump. Its thread records contain1232-byte AMD64 contexts, so treating them as716-byte x86 contexts would be incorrect. Main-thread context has CS23 and live RIP005A237C. Its low32-bit registers give the active32-bit state. Other threads' saved WOW64 contexts are read only after validating the CPU block's I386 machine tag and context flags. The32-bit TEB is validated by self pointer and threadID before using its stack bounds. The stale saved WOW64 context for the main thread is deliberately not substituted for its live CS23 state.

## Directly proven freeze

Main thread22636 starts at00C2FBD0, matching Driver.exe's PE entry point. Captured live registers:

| Register/field | Value |
|---|---|
| EIP |005A237C |
| ESI, package0 section |1B788658 |
| Section state+10 |6 |
| EBP, searched WORD key |2284 (08EC) |
| EAX, first matching node |1B78EF50 |
| ECX, current node |1B78EEB0 |
| EDX, expected sentinel |1B78EFF0 =section+6998 |
| ESP |0019FC8C |

Native bytes in the dump and exact executable agree for the repeating block:

```text
005A2373  compare searched key with WORD[current+8]
005A2377  exit if current key is greater
005A2379  current = [current+4]
005A237C  compare current with sentinel
005A237E  repeat if unequal
```

Following+4 from captured ECX returns to the same address after12 nodes. Keys are:

`293 ->300 ->303 ->2271 ->2284 ->195 ->274 ->277 ->280 ->284 ->287 ->290 ->293 ...`

Every key is at most2284, and no node is sentinel1B78EFF0. Neither exit condition can succeed on this captured topology. The loop contains no call, sleep or write that could repair the links. This is a concrete endless traversal, not simply a wait or a long cleanup batch. An unrelated future concurrent mutation is not ruled out by a single snapshot, but the list is already structurally invalid.

Native prologue/stack layout verifies return005A24CD from005A2230 and return0059D5FA from005A2470. These are actual caller slots validated against CALL instructions, not arbitrary module-address words found on the stack. Other stack candidates are retained separately without claiming a complete optimized-code unwind.

## Missing64-entry map expansion

| Structure | Header offset | Node area | Node size | Native capacity | Captured backing |
|---|---|---|---:|---:|---|
| Completed-handler map |5B44 |5B50..5F4F |16 |64 |External arena in ASI |
| Pending-handler map |5F5C |5F68..6367 |16 |64 |Native addresses, including nodes beyond its end |
| Following intermediate map |6374 |6380..667F |12 |64 |Header/sentinel corrupted |
| Include map |668C |6698..6997 |12 |64, separately expanded |Header corrupted; traversal reaches the foreign cycle |

Pending map header1B78E5B4 has cursor/free-head1B78EF40, recycled count88, active count66. Since recycled count is nonzero, this pointer is currently a free-list head, not a fresh-allocation cursor; its displacement must not be treated as a measured fresh high-water mark. Nevertheless it lies at pending slot152, and the node at1B78EF50 lies at slot153, well beyond the64-slot inline area. Active+recycled counts total154. Active count66 also exceeds the original storage count by itself.

The65th inline allocation, index64, begins exactly at the pending sentinel, section+6368. Index114 begins at section+6688 and aliases the next include-map header:

| Pending node114 field | Aliased field | Captured value |
|---|---|---|
|+4 link |Include cursor+668C |1B78EF40 |
|+8 key DWORD |Include recycled count+6690 |1B78011F (low WORD287) |
|+C value |Include active count+6694 |30318BF3 |

These are not sensible include-map counts. The intermediate map's header and sentinel are also overwritten. The pointer/key/value pattern matches16-byte handler-map nodes extending beyond the pending arena. It explains why an already expanded12-byte include list can become corrupt: its own allocator does not need to exceed its capacity for a neighboring writer to damage it.

The source `MapInsertBridge` expands the package0 map only when `map==g_pkg0+5B44`; there is no corresponding5F5C condition. Dump g_pkg0 equals the captured ESI1B788658. Completed-map external flag is1/high-water153. Include redirect count1/high-water153/fault0 shows that its earlier expansion ran; fault0 does not validate its links after subsequent neighboring writes.

## Verified native writer path

Three inspected native paths obtain the pending map at+5F5C and call005A2740:005A1A98/005A1A9E,005A1EF0/005A1EF6 and005A21BE/005A21C4. The frozen routine005A2230 also inserts directly through007D7270 at005A231A with the pending header. Helper005A2740 calls the same generic insertion routine at005A277D.

Generic007D7270 uses16-byte keyed nodes, consumes recycled nodes when available, otherwise advances its cursor by16 at007D72AC, and increments active count at007D72AF. It has no inline-arena capacity check. Constructor instructions initialize the pending cursor tosection+5F68 and its sentinel tosection+6368. These native spans are asserted by the reproducer.

The observed cycle is proven independently of this writer inference. The dump does not capture the first65th/115th append or exclude every earlier unrelated write, but the escaped pending-map nodes, exact aliases, insufficient active storage and unguarded writer provide strong evidence of this missed expansion.

## Cleanup remains healthy in this run

Reading the current ASI's scalar symbols and guards from captured memory gives cleanup peak33545, total drained88423, native queue count0, depth0, mismatch0 and all eight guard DWORDs intact. The earlier cleanup expansion is still needed; removing it would reintroduce its separate32768 boundary hazard.

ASI timestamp/image size match the built CLEANUP262144_R1 layout used for symbol addresses. This is a layout identity check, not a claim of a full byte-for-byte loaded-module hash. Native fault-loop bytes are checked directly against the exact Driver.exe SHA256 `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.

## Next implementation target

Give package0's pending16-byte map its own guarded arena, covering all generic insertion paths and retaining its original sentinel/header addresses. Keep its storage distinct from the completed map and12-byte include map. Audit native reset/reuse/removal paths and test simultaneous completed/pending maps, overflow beyond64, recycling and teardown traversal before staging another ASI. Measure pending-map node ownership, counts and cursor provenance; detect a damaged list rather than skipping nodes or breaking the loop to conceal it.

Keep150 requests/160 slots/ED+EF/indexed262144, successful audio192 and the cleanup262144 fix. This is a backing-storage/lifetime coverage correction, not another blind queue-size increase. No game-source behavior change or new ASI build was made during this analysis.

## Reproduction

`python scripts/analyze_driver_hang_dump.py dist/MAIN_150/Driver.dmp --output analysis/driver_hang_dump_2026-11-05`

`python scripts/trace_hang_streaming_cycle.py`

Evidence: `analysis/driver_hang_dump_2026-11-05/dump_summary.json`, `streaming_cycle.json`, targeted native assembly. Assertions verify the active main context, executable entry, live loop bytes,12-node nonterminating branch conditions, immediate caller slots, map geometry/header aliases, eight pending insertion/construction spans, source coverage and cleanup guards/count. Large raw heap data is not copied into the reports.

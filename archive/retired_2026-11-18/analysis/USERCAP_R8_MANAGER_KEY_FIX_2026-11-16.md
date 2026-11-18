# R8: configuration-sized native manager key lists

User authorized the static audit, fix and test build for the Getaway Activity1
exit freeze. R8 extends the remaining manager key/refcount lists implicated by
PID27864, keeping native lookup/refcount/removal semantics. Release/Win32 and
all56 expected offline outcomes pass. The live Getaway return and performance
still require the user's retry; no game/debugger was launched or installed
ASI/INI/AI/save changed. No Git commit or push was performed.

## Audited ownership and producer/consumer paths

The manager constructor0059D1C0 initializes fourteen256-node lists beginning
at manager+0C. Its loop0059D1F7..0059D223 uses count0D inclusive, strideC18,
12-byte nodes and sentinel header+C0C. These fields precede the separate
AF68 package array and the6FDA0 dispatch pool. They are not the scheduler's
fourteen512-node membership lists already handled by R6/R7.

The producer path0059D8C1 first searches0059E0D0;0059D8D7 then calls the
native counter accessor0059E290, which inserts an absent key via0059E120 at
0059E2CC and returns its16-bit refcount slot. The outer routine increments the
counter.0059CD20 queries the same per-type list.0059CD80's release path scans/
decrements references and can call0059E030 to unlink/recycle a node. Native
addresses, sentinel calculations, payload key/refcount widths and request/
loaded predicates are preserved. The captured infinite41/self ->42 search is
therefore prevented by protecting the list storage, not by returning fake data.

0059E120 has23 direct caller candidates throughout the image, including package
lists and unrelated systems.0059E080 is a separate512-node template used by
the scheduler and other paths. R8 does not globally change their geometry.
The common allocator/search detours recognize only exact list headers belonging
to a registered manager and one of its14 types. Other callers execute their
original trampoline with original state. Whole native constructor/query/search/
insert/counter/removal contracts are checked before hook installation.

## Implementation and lifetimes

`src/V29sManagerKeys.inc` maintains a stable owner generation record plus lazy
overflow storage for each list. Capacity is the existing configured work-node
count `max(512, ModelSlots + ReplayRetainedReferences)`; the supplied280-car INI
therefore supplies802 total entries per list. The original256 nodes stay in
place. Before an insertion would consume the native sentinel, the fresh cursor
switches to the external546-node suffix. Existing nodes/sentinels never move;
all native sorted search, insertion, unlink, reference values and recycle links
continue to work. Native nodes and external nodes may be recycled together.

The manager registration is attached to the existing cold StreamConstruct path,
after the native14-list initialization and before request processing. Retirement
has its own hook at0059D566, after the package destructor loop, so the key lists
remain protected throughout package cleanup. It deliberately does not retire
them at the earlier dispatch-pool destruction hook0059D547. A reconstructed
manager gets distinct generation storage; prior allocations/guards remain valid
until process exit. A native list that empties through unlink/recycle can reuse
its nodes within that generation.

Pre-search and pre-insertion validation checks bounds, guards, sorted active
links/backlinks, duplicates/cycles, recycled aliases, fresh cursor and issuance.
Known-node ranges make pointer validation bounded without querying the OS for
every node. A configuration-full or invalid list is refused before entering
the corresponding native search/write. This avoids an unbounded native loop;
it does not repair already-corrupted lists or drop keys. Three bridges preserve
registers, flags, DF and FX state. Cold startup refuses missing hooks.

Sampled `MANAGER_KEY_START/STATUS` reports capacity, owner generations, active
entries, per-list projected peak, buffers, redirections, native fallbacks and
failure counters. The query path writes no log files unless a fatal is detected.
The generic analyzer requires the new startup/status records for R8 and
validates the configured geometry. Previous R7/R6 log schemas remain supported.

## Verification and practical limits

Ten focused fixture outcomes pass:

- Native548 insertions without R8 overwrite the256-node sentinel and adjacent
  header. An isolated original native lookup with the captured41/self state
  demonstrably hangs and is terminated by the test harness's expected timeout.
-150/280/700/1000 configurations pass real native14-list initialization,
  counter/lookup/insert/remove/recycle operations and the production hooks.
  The280 case fills all14 lists to802 entries while preserving their neighbors.
- Five overlapping280-to280 transitions with12 shared keys exercise548 distinct
  keys before retirement, then280 after retirement, repeatedly. Key42 absence
  queries terminate. Sorting and16-bit reference increments remain native.
-13 constructor/destructor generations verify reused manager addresses get new
  external nodes, while old guards remain intact. An unrelated list retains
  native allocation behavior.
- Signature refusal, capacity exhaustion, guard damage and captured cyclic-link
  input are refused before the corresponding native operation. Negative tests
  check that neighboring canary memory is preserved.

All46 inherited fixture outcomes pass, including mission admission, configured
storage, traffic replacement queues, dispatch records and all four scheduler
vectors. Combined cold startup passes150/700/1000/customized configurations.
Compiled checks pin six native contracts, three FX-preserving bridges, end-of-
destructor placement, constructor registration, rollback and fail-closed startup
analysis to the actual PE32. Imports remain KERNEL32/PSAPI. MSBuild produced
only the inherited unused-helper/exported-entry warnings.

These are offline native slices with fake owner state and unrelated factories,
not execution of the game entry point or proof of700/1000-car gameplay. Full
list validation adds work on intercepted manager queries; its actual FPS impact
and asynchronous engine behavior require the user's live test. Other future
native limits are not certified absent by this correction.

## Delivery and next test

`dist/USERCAP_R8/DriverSF_Unlimiter_USERCAP_R8.asi`:312832 bytes, SHA256
`5a7b62b2170881be116bca1de8bdf6ea2a6f8f9ae71db299256d5bcf0d3838bc`.
The included INI has the same active values as R7's280/HASH180K example;
only comments change. Existing R7 ASI/INI and evidence are retained.

Replace R7 with R8 as the only traffic ASI, keep the same AI/assets/save and
repeat Getaway Activity1's exit. Report loading completion, audio/rendering,
rough FPS and the R8 log; provide a new dump for another freeze. If successful,
repeat the exit and Story/menu/Replay transitions. See `dist/USERCAP_R8/R8_TEST.md`.

Evidence, exact pre-change source snapshot, native contracts/audit and new
focused fixture/build results are in `analysis/manager_keys_r8_2026-11-16/`.
Inherited evidence has separateR8 output folders; R7 results were not overwritten.

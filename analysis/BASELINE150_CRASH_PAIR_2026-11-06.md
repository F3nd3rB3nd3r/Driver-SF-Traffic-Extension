# Direct Lua150 crashes: native50-ID notification payload overflow

Both new supplied dumps match a missed native50-ID stack buffer in the traffic
request notification callback00754C70. It copies150 DWORDs from the spooler's
inline request area into a50-DWORD local payload. The first excess DWORD is
the spooler's request count150; that DWORD overwrites the callback return
address. Both dumps fault atEIP0x96 (150), exactly as this layout predicts.

This is a concrete capacity/storage gap inherited from the parent implementation,
not evidence to increase an audio or resource-memory pool. The new observational
hooks are not needed for this overflow mechanism. No game-side fix/newASI was
built during this analysis. The current direct150 Lua test must not be repeated
until this callback and its data source are fixed.

## Supplied evidence

| Run | Log | Dump | Direct request tick | Last log tick |
|---|---|---|---:|---:|
|23504|DSFV29s_baseline150_23504.log|Driver_20261106_180604.dmp|328641ms|369719ms|
|8732|DSFV29s_baseline150_8732.log|Driver_20261106_180728.dmp|68875ms|72188ms|

Both logs record two initial named18-ID requests, then the exact ascending150
roster from Lua caller return0066E5F8. Original/effective rosters match, with
zero duplicates and hash5CBDFFEA. The entire150-item scratch pass is observed
(scratch peak149). Each dump's persistent spool list has150 active nodes and
zero recycled nodes. Neither direct-request phase executes a readiness sweep
or gains further body-call observations: the main request path does not finish.
`ready=0,checkpoints=0` means unmeasured, not150 failed readiness queries.

The monitor continues briefly/for about41 seconds after the respective direct
requests. Its continuing output does not establish main-thread progress.
Existing arena/callback/pending/cleanup guards report no failure. Their
structural checks pass, but both processes demonstrably crashed.

The installed AI file now hashes to
`da5b1ca3e4e51b6c8d937ed0eb2cbeb2a54048c9aca5a3e5a26f8704b1f0b936`,
matching `san_franciscoAI_original.fchunk`; the user confirms switching to the
original. `assetMismatch` therefore retains the normal18-ID named rosters.
This tests an18-to150 transition via direct Lua, unlike the earlier edited-file
native150 workload. Original traffic data is a valid unlimiter test; reverting
the asset would not repair the missing callback storage/source handling.

## Static chain and exact overwrite

Exact Driver.exe SHA256 remains
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.

1. Lua binding0066E560 dynamically allocates the table input and calls shared
   request writer005A79F0 at0066E5F3. The ASI accepts all150 into its sidecar,
   retaining only the first50 in the native object before count+0xC8.
2. With owner+DAC zero, the writer invokes005A7490 synchronously. This traversal
   builds the expanded scratch list and merges into the persistent owner list.
3. Before completion/clearing the request count,005A77E1 reads owner+E68 and
   invokes virtual+4 at005A77F6 with `(owner,count)`.
4. Native constructor0074B6E0 installs vtableE2A708. Its+4 entry0074B660 forwards
   to its registered listener's virtual+4. Registration00754A90 installs the
   listener subobject with vtableE2AE18; that+4 entry is00754C70. Small dumps
   do not include the live owner+E68 pointer, but the exact copy footprint below
   independently matches both captured stacks.
5.00754C70 has a0xD0-byte local frame: an8-byte header and200-byte/50-ID payload.
   Under its parent+4E588==1 and parent+4E9E0==0 gates, it copies count*4 bytes
   using native memcpy00C28820 at00754CD0. Source is the raw spooler pointer,
   not the extended request sidecar. There is no count clamp on this copy.
6. For150 IDs the copy is600 bytes,400 beyond the local payload. Source DWORD50
   is owner+0xC8=150, and destination DWORD50 is exactly the saved return slot.

Both dumps have ESP0019EDE4 after the failing RET8. Therefore callback entry
return slot was0019EDD8 and local payload starts0019ED10 (200 bytes earlier).
`[0019EDD8]=0x96`, matching owner+0xC8 and exception EIP/EBP0x96. Captured owner
data and stack data match continuously for source offsets0xB0..0x257, including
the corrupted return and copied node links. This is the expected600-byte copy
of the native object prefix, not a correctly assembled150-ID payload.

The parent native frame's intact ASI return candidate is at0019F514; the fault
occurs on the nested callback's return, before the outer request can finish.
The two dumps have different process/heap/module addresses but the same stack
geometry and overwrite value. No symbol-based unwind is claimed from EBP0x96.

## Required correction

- Expand this callback's local payload to150 IDs and adjust every affected
  stack argument/prologue/epilogue operand together. For the present layout,
  frame0xD0 becomes0x260; arg loads0xE0/0xE4 become0x270/0x274.
- Change this specific copy path to obtain the full IDs from the matching
  request sidecar. Simply enlarging the stack still copies internal spooler
  fields as IDs51..150; the native inline storage remains50.
- Audit the subsequent event writer007467D0 and replay/reader paths, including
  packet length and any fixed roster/snapshot arrays. Keep notification behavior;
  do not globally hook memcpy or suppress the callback.
- Exercise the complete native callback, both its gates and return ABI, in a
  bounded fixture before another live150 test. Prior fixtures covered request
  storage/scratch/merge and new observational bridges, not this full callback.

The earlier statement that reaching the expanded common writer was sufficient
for the direct150 test missed this downstream callback. That validation scope
was insufficient. No claim that every pool/limit is covered follows from fixing
this one; the evidence does identify this specific new boundary with high confidence.

## Reproducible offline verification

`scripts/verify_traffic_notification_crash.py` validates the native instruction
spans/vtable entries and the stack/source arithmetic against the two supplied
logs/dumps. Output: `baseline150_crash_pair_2026-11-06/notification_overflow_proof.json`.
That JSON contains input SHA256 values and both process-specific footprints.
The same folder contains both dump summaries and both coverage/inherited analyses.
No debugger, game execution, build or live-memory access was used.

The common log reader initially rejected `BASELINE_START ... type=0` because
the payload key collided with its record-type metadata. It now preserves that
payload as `field_type`. Both actual logs parse successfully; the offline proof
checks their startup records as a regression test. This parser fix does not
change an ASI or explain the game crash.

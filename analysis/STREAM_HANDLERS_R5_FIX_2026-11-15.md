# R5: shared dispatch handlers and coupled scheduler extension

User authorized a static implementation followed by their existing280-entry
live test. R5 is built and verified offline; live280 loading and lifecycle
validation remain pending. No debugger/game launch or game deployment occurred.

## Cause and audit scope

PID28316 crashes at0059DA4A with ESI=0 after native dispatch cannot select a
free80-byte handler. Both initialization paths construct256 entries. The
matching AI/config/log are valid for280 requests; see
`USERCAP_R4_CRASH28316_STREAM_HANDLERS_2026-11-15.md`.

The follow-on audit found coupled storage in the scheduler pointed to by
manager+0x6FD94/root+0x200: three256-pointer vectors, fourteen512-node membership
lists, then runtime fields through original object size0x16620. Changing the
handler count alone would leave vector overwrites. The scheduler's per-kind
membership list is a single512-node12-byte list, not two256-node lists.

Evidence is in `analysis/stream_handlers_r5_2026-11-15/`:

- Focused disassembly, whole-image decoded scheduler-tail accesses, candidate
  operand references and355 audited instruction/operand signatures in
  `scheduler_operands.json`. Coincidental four-byte matches across instruction
  boundaries were excluded; signatures cover the complete instructions.
- Eleven pool-base consumers, twelve count references, two initialization
  callsites, native record constructors/vtable predicates/destructor, native
  scheduler allocation/constructor/destructor, insertion/erasure and membership
  consumers were traced. Native pointer-based downstream handling is preserved.
- Scheduler-tail references outside its own methods are included, including
  gameplay reads of root+0x200 state and three materialized165C0 base offsets.
- `before_r5.zip`/`before_manifest.json` retain the pre-task edited source and
  project files; existing uncommitted R4 work and its distribution are retained.

## Implementation

`src/V29sStreamHandlers.inc` and `V29sStreamHandlerLayout.inc` implement the
new component, enabled by `DSF_V29S_STREAM_HANDLERS=1` in the active one-project
solution. Active output is `build/v29s_usercap_r5/`; package `dist/USERCAP_R5/`.

Capacity H=`max(512,ModelSlots+ReplayRetainedReferences)`, with checked signed
allocation geometry. No new INI key is necessary. The user's current280/512
request/retained settings give ModelSlots290 and H802. Default150/160/256 gives
H512. Existing request rosters, weights and capacities retain their configured
values; this change adds no vehicle list or forced selection.

- Manager construction allocates a guarded, zeroed external H*80-byte slab
  and records a generation. The old inline pool's first word holds its base;
  the native count and all later manager fields retain their addresses.
  Eleven LEA/ADD base calculations become pointer loads. At the three former
  ADD sites, flags are overwritten by native end-address calculations before
  being consumed.
- Both native256 initialization calls go through an ABI/FX-preserving wrapper,
  using the actual native record copy constructor0059E1D0. Repeated setup
  preserves existing records. Destruction still traverses the records through
  their native virtual destructor. Old slabs remain allocated until process
  exit; reconstructed owners receive a distinct slab. No active relocation.
- Scheduler geometry grows together: vector stride4H+4, membership stride
  12H+24, fourteen memberships, and the same runtime tail. Logical scheduler
  object bytes=180H+544, plus16 guard bytes. Its native allocator requests the
  larger size and the native constructor/destructor retain ownership.355
  exact-version field/layout operands coordinate all audited consumers.
- Seven detours observe manager construction/destruction, guard the captured
  null dereference, observe scheduler construction/destruction, and guard
  vector/membership insertion. Two initializer CALLs and eleven base selectors
  are also patched. All original signatures are preflighted before mutation.
  A partial installation failure refuses startup; it never runs the game with
  an incomplete installation. Register/flags/x87/SSE state is preserved.
- Bounds/guard failure produces a specific diagnostic and the inherited error
  exit0x29F3. Exhaustion is not suppressed or presented as loading success.
  The native general allocator's existing null return contract is preserved;
  the captured dispatch dereference is explicitly protected.
- Status tracks generations, initialization/destruction, active owners,
  sampled handler occupancy/peak, queue counts/projected insertion peaks,
  projected membership peak, integrity and failures. Handler samples run at
  the existing five-second queue cadence; no per-frame file logging or extra
  native readiness/resource calls are added.

## Verification

Release/Win32 build of `DriverSF_V29s_NATIVE_TRACE.slnx` succeeds. The first
sandboxed MSBuild attempt failed in FileTracker with access denied after
compilation; the authorized elevated rerun succeeded. Compiler/link warnings
are the existing unused historical helpers and exported DllMain warning.

- `verify_stream_handlers.py`:9 isolated processes. Actual retail native slices
  allocate past256 through configured capacity, scan the captured dispatch
  branch, find a tail record through native lookups, reuse a released slot,
  fill/erase/insert all three vectors, fill/query/recycle all fourteen
  memberships, destroy/reconstruct scheduler and manager at the same address,
  and retain distinct slabs across twelve manager generations. Positive
  layouts cover150,280,700 and1000 requests; negative cases cover signature
  mismatch, the actual patched null site, guard damage, vector and membership
  exhaustion. The280 case uses H802 and passes5775 checks.
- `verify_wide_limits.py`:19 inherited processes, including full cold installer
  checks at default/nondefault geometries and expected failure paths.
- `verify_traffic_queues.py`:6 inherited R4 queue processes. Their isolated
  setup does not install the separate Stream component; full cold installer
  coverage above exercises combined installation.
- Three compiled verifiers pin the ASI to fixture dependency hashes, its
 355 instruction signatures, exact bridge register/FX preservation, imports,
  defaults, startup telemetry, analyzer negative coverage and preserved R4
  artifact identity. `verify_stream_handlers_build.py` is the final check.

34 process outcomes pass. The fixtures execute selected native routines in a
separate harness, with documented relocation/middleware boundaries; they do
not start Driver.exe's entry point, rendering, audio or game loop. Higher
offline layouts are not higher-roster gameplay certification.

Candidate:303616 bytes, SHA256
`9e617bddcf3fb5732f54c66b04212f05c27a0f8fb5a7f01f5aeededd44c9d332`.
Game Driver.exe, AI, INI, saves, installed plugins and the R4 candidate are
unchanged. No commit or GitHub push occurred.

## Next live check and remaining scope

Follow `dist/USERCAP_R5/USER_CAPACITY_R5_TEST.md`: same280-car AI and effective
INI, only R5 as the traffic ASI. First repeat the crashing initial Story load,
then driving/audio/visibility, traffic-changing activity, menu/Story and Replay
transitions, and normal exit. Supply the R5 log and observed outcome; retain a
matching new dump if it fails. Analyzer supports the added component and rejects
a missing R5 startup record.

Static evidence is sufficient to implement this failure correction without a
new live probe. It does not prove every downstream limit in the game has been
found, that every modded asset works, or that280/700/1000 resident models are
drawable and audibly correct. The new private scheduler geometry is specific
to the verified retail executable; live testing with the user's other plugins
is still required. The former152-model R4 tests remain the previous live
baseline until the R5 retry is supplied.

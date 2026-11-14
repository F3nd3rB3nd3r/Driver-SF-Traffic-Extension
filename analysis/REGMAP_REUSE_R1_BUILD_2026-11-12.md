# REGMAP_REUSE_R1: definition-map lifetime correction

Built, native-fixture/compiled/inherited verified and staged following the
user's explicit request to fix the two statically established defects.
The game was not launched and nothing was installed into its directory.

Artifact: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R1.asi`.
Release/Win32,252928 bytes, SHA256:
`7bec339a6bc22d3ba599fd8533863604d20da23a4ed82add357e59e724b93f9b`.
Guide: `dist/MAIN_150/REGMAP_REUSE_R1_TEST_2026-11-12.md`.
Log: `%TEMP%\DSFV29s_regmap_reuse_<PID>.log`.

## Problem and resulting behavior

Run19868 explicitly terminated with vehicleDefinitionMap/code0x31. Static
analysis (`DEFINITION_MAP_STATIC_2026-11-12.md`) identified a fixed four-address
ASI directory and rejection of legal inline cursors after native reconstruction
at a previously remembered address. That run's exact branch is unknown.

The new conditional implementation allocates stable per-address records as
needed. A new native constructor generation resets that record's issued-node
bookkeeping. The next45 nodes use the original inline storage; the subsequent
node reattaches that address's stable512-node backing. A fifth distinct map
gets its own record. A reconstructed map starts from its native initialized
state and no longer trips the stale external-cursor check on its first insert.

The original map fields/sentinel/native algorithms remain in place. No
external storage is freed or moved while native nodes may refer to it.
Each distinct address retains12288 bytes of node payload plus32 bytes of
metadata/guards; VirtualAlloc rounds the12320-byte request to page granularity
(16KiB with4KiB pages) and aligns reservations to allocation granularity. Actual
live virtual-address footprint has not been measured. Reused addresses
reuse the same record. There is no fixed owner-count ceiling; address space
and allocation remain finite. This is lifecycle bookkeeping, not an increase
to audio slots, traffic requests or nearby vehicle population.

## Native boundaries and protection

- Existing new-key insertion boundary0086BE5F:8 displaced bytes, unchanged
  native continuation0086BE67. Adapter preserves GP registers, flags/DF,
  x87 and SSE state around the guard, then executes the original instructions.
- New constructor entry0086BFA0:5 whole displaced bytes. Wrapper calls the
  complete original constructor first through an RX trampoline, validates
  return value, vtable, fresh cursor/counts and self-linked sentinel, then
  updates the matching record's generation. C++ thiscall-compatible adapter
  returns the original owner and preserves callee-saved registers/stack.
- Full constructor, insertion context, lookup, removal and allocator byte
  signatures are checked, alongside image identity and cold engine state.
  The shared generic008CCAA0 allocator is not globally hooked.
- Both hook sites share one original code page. Installation prepares RX
  trampolines before modifying that page, rechecks before commit, verifies
  both patches/dependencies and flush/protection outcomes, and restores both
  sites on failure. Later parent rollback also restores both. This is a cold
  startup transaction, not a claim of concurrent hot-patching safety.

The core tracks expected fresh-node frontiers and total issued nodes. Native
active/recycled counts must agree. Recycled nodes must belong to this map's
issued inline/external storage; their next link must be a valid issued node
or the saved fresh frontier. Guards and external end checks remain. A known
external generation is not silently rewound to inline storage without an
observed constructor. An unknown owner can attach only in a completely fresh
native state. Invalid state still exits rather than permitting an overwrite.

Only the constructor and first external redirect produce events; the monitor
prints counters without dereferencing engine owners or invoking engine calls.
REGMAP_FATAL includes branch reason, map/cursor, backing bounds, actual
active/recycled counts, generation, issued counts and owner count, then the
existing LEGACY_FATAL/ExitProcess(0x31) path. Startup/rollback failures are
also explicit. Redirect events capture the pre-native allocation: active45,
externalIssued1 is expected because the guard has reserved the next slot.

## Verification performed

The fixture includes the production core, adapters, installer, layout and
runtime logging. It executes exact supported Driver.exe constructor,
destructor, insertion, lookup, removal and node-allocation bytes in a relocated
fixture image. Relative native internal calls remain intact. The constructor's
shared object-pool service, destructor's shared teardown service and owner
operator delete are stubbed. This tests map mechanics without executing the
game or claiming557 live vehicle objects from the160-object pool.

Passing checks include:

- Eight distinct150-ID owners, exceeding the old four-address cap.
-100 same-address native destructor/constructor generations and backing reuse.
- Duplicate key payload and missing/present lookup; sorted forward/backward
  linkage and sentinel consistency.
- Mixed inline/external removal/reinsertion without consuming fresh capacity.
- Actual557-node map bound (45+512), rejection at external end, and successful
  recycled-node insertion while fresh storage is full.
- Bad cursor alignment, recycled link, counts, guard and allocation failure.
- Cold fresh attachment and exact fatal fields/code.
- Full signatures and the real two-site installer; allocation/protection/
  trampoline-flush rejection, post-commit page-flush failure with rollback,
  explicit restore/reinstall and exact original bytes afterward.
- Callee-saved GP/ESP/x87 checks through actual constructor/destructor and
  insertion calls; insertion bridge SSE preservation.

The native fixture and inherited replay pause/snapshot fixtures passed before
the active solution was built once. The compiled artifact passed the complete
inherited verification chain (snapshot, parked, notify, baseline, arena,
callbacks, chapters, pending/cleanup/audio/indexed checks), plus its new hook
targets/byte tables/adapter disassembly, old fixed-directory absence and
startup/log markers. Parser tests cover owner/generation exercise and reject
bad startup/state/bounds/fatal records without certifying gameplay stability.
New fixture compiled without warnings; the ASI build has the existing C4018,
C4505 and LNK4216 warnings, with no new RegMap warning and0 errors.

All source and tool hashes, compiled checks and packaging evidence are under
`analysis/regmap_reuse_r1_2026-11-12/`. Entry point:
`python -B scripts/verify_regmap_reuse.py`; focused options `--fixture-only`,
`--verify-build`, `--parser-only`, `--resume` and `--package`.

## Preserved behavior and remaining validation

150 requests/160 graphics slots/ED+EF; snapshot150 IDs/256 reference records;
paused completion guard; parked150/25x150; definition-object pool160; indexed
262144/resource collection131072; pending512/cleanup262144; audio models192/
global slots2048/default64MiB; callbacks21x512 and per-owner spool256 all remain.
REPLAY_PAUSE_R1 and other parent artifacts were retained and hash checked.
No assets, recordings, request counts, dependencies or game files were edited.

Live repeated Story/Replay/menu transitions, playback controls and normal exit
remain to be tested using only REGMAP_REUSE_R1. Telemetry distinguishes actual
same-address reconstruction/reattachment from distinct owner creation. The
separate null simulation-manager pause crash and original zero-rate caller
are not patched or attributed here. No claim of complete150 simultaneous
drawable vehicle coverage or universal lifecycle stability is made.

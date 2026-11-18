# Generic limits R1: first configured-storage milestone

2026-11-14. Implemented, built Release/Win32 through the sole active solution,
offline verified and staged for a user test. No game installation, game launch
or debugger attachment. Previous TRAFFIC_POLICY_R1 remains unchanged as the
paired-live-pass fallback. This candidate has no live-game verification yet.

## Artifact and configuration

`dist/MAIN_150/DriverSF_Unlimiter_GENERIC_R1.asi`,274944 bytes, SHA256
`a8c2915aad6c3791410891be78d9066f68d35b5ab8e5a32185b2bf055c05722b`.
Output `build/v29s_generic_limits_r1/`. Use exactly one traffic ASI.
New config: `DriverSF_Unlimiter.ini`, beside the ASI, including in plugins/.
The old DriverSF_Traffic.ini is not read by this candidate. Config is immutable
after process attach; restart after edits. No config file is written by the mod.

| INI key in Limits | Default | Implemented range |
|---|---:|---|
|RequestCapacity|150|1..150|
|AudioModelEntries|192|45..512, at least RequestCapacity|
|GlobalAudioEntries|2048|255..8192|
|DefaultAudioMiB|64|16..256|
|ReplayReaderMiB|1|1,2,4,8,16|
|ReplayRetainedReferences|256|50..4096, at least RequestCapacity|

Traffic/RosterSource is AssetWeights by default; Native is an explicit option.
Unknown/duplicate keys, invalid integers, unsupported ranges and nonfinite
weights are rejected. A roster beyond configured capacity is diagnosed before
publication, including its full required count for an asset-derived union.
No partial private list is handed to the game and no wrapping/truncation occurs.
Bad configuration or an undersized required roster stops the process with an
explicit GENERIC_START/GENERIC_FATAL reason; inspect the log before requesting
a crash dump. Missing config uses the documented defaults and logs missingFile.

## Generic roster selection

No compiled model-ID arrays, preset profile/hash allowlists or uniform150 asset
fingerprint are in the generic ASI. The exact supported native setter signature
and executable/ABI validation remain. Fixture-only expected rosters are not
linked into the ASI.

AssetWeights uses the current native named-setting caller, owner, hash and
frequency, then finds the setting in the loaded AI arrays. Counts, bounds,
IDs, routes and finite weights are validated from those arrays. It preserves
all required native IDs, including zero-weight native requirements, and adds
positive-weight candidates across reachable regional tables, deduplicating
additions. Newly augmented lists are sorted numerically for stable request
order; unaugmented/native lists keep original order and IDs. No weights,
population, actor count or native frequency is changed.

Direct Lua/other requests retain their own lists; empty/negative requests retain
clear semantics. Unmapped named settings retain their native requests with a
diagnostic. Malformed mapped data or insufficient capacity is not silently
treated as a successful native fallback.

Read-only audit of the actual original file da5b1ca3... finds80 active profiles,
no profile needing augmentation, maximum native/weighted union27. The known
modified copy434e1a51... has55 profiles needing augmentation, maximum union150.
All160 original/modified profiles were also run through the production roster
and request helpers in an isolated native fixture: exact expected lists match.
For example original Chapter7 remains18; modified Chapter7 requests150 from
its data, without a preset list compiled into the mod.

## Runtime storage and preserved geometry

- Request sidecars allocate their configured ID capacity and use a process-lived
  owner directory rather than16 fixed owner slots. Asset-selection scratch is
  leased per owner/thread; its ID buffer follows capacity and its bitset covers
  the16-bit model-ID domain. The lease is released after request/telemetry copy.
- Notification local-frame operands derive from capacity with the native50-ID
  physical floor. Parked ID/pick arrays and tail guards are dynamically sized;
 25-category stride, count offsets and probed native builder frame are derived
  together. Actual picking/category behavior is retained.
- Replay traffic/retained arrays have configured process-lived backing and
  guards. Copies are deep, not pointer aliases. Restore holds a private saved
  snapshot while separately collecting/releasing current references. Native
  views/object sizes and serialized field order remain unchanged.
- Audio model allocation/stride/limits/guards, global audio list capacity and
  shared default allocation consume their INI fields. Reader reservation/
  commit/capacity/guards consume replay MiB;32 KiB transfers/order remain.

This is deliberately not the complete700-model implementation. Graphics and
definition slots remain160; native request scratch retains the tested150-node
frame. Notification/parked physical storage has a50-ID minimum even when logical
capacity is lower. Diagnostic model arrays still have the150-stage ceiling.
Resource hash/collection/indexed, selector/ED/EF, pending/callback/cleanup
capacities and audio manager/key directories retain their tested sizes. These
are not new configurable keys yet. Existing lifetime fixes remain.

RequestCapacity above150 is explicitly rejected. Future work must migrate the
byte-admission/provider/index path, safe larger request scratch, the remaining
coupled pools and diagnostics before claiming700 support. Changing this parser
bound alone would be unsafe.

## Verification actually performed

Evidence: `analysis/generic_limits_r1_2026-11-14/`.

- 12 compiled isolated fixtures pass. Generic requests use arbitrary IDs and
 an unknown-to-old-presets hash at capacities18/50/100/150. Bounds, original
 order, zero/negative clears, positive/zero/negative weights, duplicate table
 elimination, invalid/unknown/duplicate config, IO failures and exact required
 capacity are covered. More than16 request owners work.
- Actual relocated parked builder/picker passes all counts0..150, plus fresh
 configured100/18 owners and corresponding frame/stride/storage geometry.
- Native replay slices/ABI/guards, cached/deferred/immediate restore and100
 repeated transitions pass with dynamic/deep storage. A second configuration
 validates traffic100 and retained400 in512-entry backing. The allocation-
 failure fixture now holds all available temps, accounting for the extra
 owned restore snapshot rather than assuming a single free temp.
- Native reader/installer/rollback/bridge/guard/destructor/generation fixtures
 pass with1 MiB and4 MiB backing. Native audio registry256, global slots4096
 and shared default128 MiB also pass production-core/native fixtures.
- All160 actual asset profiles match independent audit expectations. Original
 and modified files were read only; relocations occur in fixture-owned copies.
- Configuration-aware Python log parser passes a nondefault100/256/4096/
128MiB/4MiB/512 fixture and13 negative schema/capacity/guard/error cases.
- Compiled PE32 imports only KERNEL32/PSAPI; no CRT dependency. Request RET8 and
 entry RET0Ch retained. Native setter bytes match Driver.exe. All10 retained
 hook tables/79 sites match the tested parent in addresses, signatures and
 target identities. The original150 frame/pool/indexed operand tables match.
 Map/data checks confirm no canonical car/profile tables or Preset150 policy.
- Build0 errors,28 existing signedness/unreferenced-function/exported-entry
 warnings. TrackFileAccess=false avoids the previously observed sandbox
 FileTracker issue; no elevated build retry or dependency installation needed.

Fixtures are not gameplay. Unchanged fixtures may be reused only when their
full quoted-include dependency/command/binary hashes match. Recorded source
and fixture identities are checked again before packaging. A preliminary
own staged build is retained with its original path/SHA manifest under
preliminary_stage before final replacement; unrelated files are never replaced.
The explicitly scoped prechange manifest preserves21 input files.

## User test

Guide: `dist/MAIN_150/GENERIC_LIMITS_R1_TEST_2026-11-14.md`.
Keep numeric defaults for the first run. Use the same AssetWeights INI for
original AI and the known modified150 AI, in separate fresh processes. Include
Story/Challenges/menu/replay and normal exit. Log
`%TEMP%/DSFV29s_generic_limits_<PID>.log`. Use
`scripts/analyze_generic_limits_log.py`, which checks the actual recorded
configuration; old policy/preset-only analyzers are not its acceptance rule.
READY checks remain sequential and do not prove simultaneous drawable residency.

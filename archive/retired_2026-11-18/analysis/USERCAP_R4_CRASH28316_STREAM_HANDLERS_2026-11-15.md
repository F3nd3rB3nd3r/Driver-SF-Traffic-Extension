# R4 PID28316: 280-model initial-load crash

The matching dump, Driver.log and copied Unlimiter log identify a separate native streaming-handler allocation failure. R4 accepts the 280-ID asset roster, but native dispatch reaches an unchecked null handler at0059DA4A. Static initialization still provisions256 shared handlers. Existing INI limits do not extend this array. No production source, ASI, INI, game asset or save was changed; no build, game or debugger launch was performed.

## Inputs and configuration

Evidence directory: `analysis/usercap_r4_crash28316_2026-11-15/`. `input_manifest.json` records paths, sizes and SHA256 values; `dump_analysis.json`, `asset_audit.json`, `generic.json` and address-named disassembly files retain the focused results.

- SS Test dump `Driver_20261115_185525.dmp`: PID28316,521036 bytes, SHA256 `9bead187911038013f08706210db08637bba9003a029bc6d5384c7eca73664b2`.
- Copied `dist/USERCAP_R4/DSFV29s_usercap_r4_28316.log`:509587 bytes, SHA256 `6585acd3b2b29dd2fb9ba13967d1ed579e84fff330e8f7811149c864ef7a29ff`.
- Active AI SHA256 `ef0d582f8523942462312f55650d129e155689e5e7911c40bfb00b96a07ab514`, matching the independently verified TrafficEditor0.3 bulk-weight output. Structural and roster-capacity audits pass:59 settings,80 nonempty frequency groups,208 probability tables,42274 rows; maximum routed union280. No malformed table, duplicate ID or invalid weight was found. This is structural validation, not proof that every vehicle asset is usable in gameplay.
- Accepted configuration: RequestCapacity280, ModelSlots290(auto), AudioModelEntries384, GlobalAudioEntries4096, DefaultAudioMiB128, ReplayReaderMiB2, retained512, three R4 queue capacities802(auto). AssetWeights mode; no compiled vehicle list.

## Matching runtime evidence

Log line724 requests the preserved18-ID roster at tick24593. Line966 selects the asset-derived280-ID roster at tick27562, hash47EB4D0E/frequency0, same owner1CB520F0. The18-ID phase reaches all READY; the280-ID phase never records all READY. Last sample tick33234 still has zero readiness checkpoints for that phase. A complete roster record means all IDs were logged, not that loading completed.

The generic analyzer reports recorded integrity checks passing. This only covers its existing telemetry: it has no occupancy counter for the newly identified handler pool and cannot turn this crashing run into a successful capacity validation. No ASI fatal marker is present because this is a native access violation.

Existing measured queues/guards show no failure. R4 retirement/work queues are not yet exercised; shared audio uses1676780/134217728 bytes in the last sample, registry18/384, and global audio lists have no ceiling hits. These observations do not support raising those limits as a fix for this crash.

Driver.log places the exception inside `setTrafficSettingsName`, Chapter7/Free Drive initial loading. This differs from the previous activity-transition retirement-list overflow fixed in R4.

## Native failure chain

1. Exception C0000005, read address0, instruction0059DA4A `mov eax,[esi]`; captured ESI=0. EDI=0x124 (vehicle292), EBX=1C8EB188 (package0), ESP=0019EAB0. Captured caller return005A75BB belongs to request reconciliation005A7490, dispatching through0059DB40 into0059D740.
2. The stack holds manager1C8E0220, package1C8EB188, request count0x118=280 and ordinal0xF9=249. Vehicle292 is position249 in the sorted280-ID union. This supports failure while dispatching the250th roster entry; it does not establish a defect in model292.
3. At0059D9D2..0059DA08, dispatch scans records from manager+0x6FDA0 through count at manager+0x74DA0, stride0x50 (80 bytes). Virtual method+0x40 tests each record's in-use flag. If every candidate is busy, or the count is zero, the function explicitly sets ESI=0. The following code nevertheless dereferences ESI at0059DA4A. No allocation-failure branch protects that dereference.
4. The vtable is00DE4A80. Method+0x40 is0059C300, reading byte+0x4D; method+0x3C is0059C2F0, writing that byte. The available-record branch0059DB09 sets it to1 before use. Thus the predicate is an allocation/in-use flag, not a vehicle probability or drawable check.
5. Native setup0059DCE6 and0059DE4C both push0x100=256 and call0059E5B0 on manager+0x6FDA0. That helper constructs80-byte records via0059E580/0059E1D0 and stores the count at pool+0x5000 (manager+0x74DA0). Physical inline storage is exactly256*80=20480 bytes, immediately followed by the count and other manager fields.

The dump does not capture the pool/count pages, so the exact live occupancy and identities of other consumers cannot be enumerated. Pool exhaustion is the strongly supported explanation from the null-selection path, native256 setup, and failure at roster ordinal249. Do not claim a measured256/256 occupancy or an exact seven non-traffic users from this small dump.

## Recommended next work

This exposes an additional hardcoded limit not covered by current user-capacity configuration. It needs a separate coordinated pool extension: audit every base/count consumer, initialization/reset/destruction and outstanding handler pointers; provide stable backing and preserve native handler lifetime/flag behavior. Add occupancy/high-water/failure telemetry and a safe exhaustion path. Derive capacity with headroom for shared non-traffic consumers rather than assuming one slot per requested vehicle is universally sufficient.

Changing only the256 initializer would overrun inline storage. Increasing ModelSlots, audio MiB or the already expanded R4 work queues does not move this pool. Do not bypass loading readiness or simply swallow the null pointer and declare the request successful.

No new live session is necessary to identify this failure mechanism. A later coordinated fix requires offline verification and a user280-model/lifecycle retry. Current live evidence remains the prior successful152-model R4 runs, not support for280/700/1000 models. Await authorization before implementing/building a new ASI.

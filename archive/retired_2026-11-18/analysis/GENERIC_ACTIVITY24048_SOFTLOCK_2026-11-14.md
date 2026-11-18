# Activity 24048: unavailable saved garage model, not a demonstrated capacity exhaustion

Analyzed supplied `dist/MAIN_150/DSFV29s_generic_limits_24048.log` and
`Driver.DMP` read-only. No production source/INI/ASI/asset/save changes, build,
game launch, debugger launch or process attachment. Evidence is in
`analysis/generic_activity24048_2026-11-14/`.

## Finding and next test

The activity is **StreetraceTakedownActivity5**, confirmed by the live Lua
loader's captured parameters. Its state is `active - loadActivityVehicle`.
The registered `spawnVehicle` update is polling
`TrafficSpooler.IsPlayerVehicleLoaded()` and waiting to spawn **model 122**.
The native player-spool fields also contain requested/current model 122.
That ID is absent from the package's available-resource catalog, so native
code cannot load it. The traffic roster itself has all150 READY.

User recalls driving model191, an old Ford Mustang. This is compatible with
the snapshot:191 and its1191/2191 resources are present, but the pending
activity-spawn model is122. `localPlayer.currentVehicle=false`, `inZap=true`.
The loader's no-activity-vehicle branch selects
`vehicleManager.getBestOwnedModelID()`, which reads
`ProfileSettings.GetGarageVehicle()` and returns any value except-1 without
checking resource availability. Only-1 receives fallback179.

User notes the earlier Forza mod fills otherwise absent IDs. A saved garage
selection left over from that installation is therefore a strong explanation
for122, **not independently proven save provenance**. The dump does not
establish which file/mod originally stored it.

**Next test:** choose an available car in the garage, then retry the activity
with the same GENERIC_R1/default INI and150-car AI. No new ASI or capacity
increase is indicated for this test. An optional read-only Lua check is
`print(ProfileSettings.GetGarageVehicle())`. Await the user's result; do not
claim the issue fixed until the retry succeeds. Do not modify the profile
automatically or silently substitute a different car.

## Native chain, checked against captured bytes

- Lua binding0066E760 calls005A63F0 on traffic spooler1B294470.
- `owner+DB0=122`, `owner+DB4=122`, `owner+DB8=FFFFFFFF`, `owner+DAC=4`.
  Manager is1B0189E0; package0 is1B023948.
-005A63F0 requires matching player IDs, then READY0059CFC0 and no pending
  state0059C560.122 is absent from the captured READY set, so the first query
  cannot succeed.
- Availability0059C620 calls005A0A80. It scans the six-byte catalog records at
  `package+72FC`, using the WORD entry count at `[package+72F8]+2`.
  Metadata2A91AF20, records2A91AF3C, count450. No record for122.
-005A6EA0 checks traffic membership, mission references and finally that
  availability query.122 is absent from all three; the final query returns0
  and the function returns1 (`sete`). In005A7930 this takes the branch that
  recordsDB4 and state4 without dispatching a resource load.
- The Lua polling callback subsequently has no ready resource to observe.
  This is the native control-flow explanation of this captured loading state,
  not an observed historical trace of every call.
- Dump bytes match the supported on-disk executable at0066E760,005A63F0,
  005A0A80,005A7930 and005A10C0 (`native_gate_byte_checks.json`). No target
  function was executed by the analysis tools.

## Supporting resource and log results

Catalog has150 base IDs, exactly equal to the accepted traffic roster, plus
their1000/2000 variants:450 unique records.122 has no record. The completed
map and READY set each have154 resources:150 traffic plus1191/1239/2191/2239.
The pending-flags list is empty; native dispatch suppression byte is0.
Package has164 handlers and10 free kind0 handlers at the snapshot; this does
not show the160 kind0 capacity exhausted. No captured ASI lock is held.

Log spans four asset-derived requests:18->150,18->150,12->150,18->150.
Phases2/3/4 each reach all150 READY; phase4 first does so at303140ms and still
reports150 at401187ms. There are no generic fatals, callback full/invalid/
guard faults, arena allocation/guard failures or registry overflow failures.
Cleanup drains to0. The pending extension is unexercised, not broken on that
evidence. Replay reader/import/completion paths are unexercised.

Generic integrity analyzer fails only its **audio_setup** check:18 setup
failures, six each for118/181/239, alongside429 successes. Shared audio is
about5.17MiB used of64MiB and has0 sampled pressure events. These failures
remain a separate observation; no evidence links them to the captured wait
for model122. Do not label this log an unconditional pass.

The dump is PID24048,1,215,523,883 bytes, with no exception stream. Both
DriverNGHook and GENERIC_R1 are loaded. The main-thread snapshot passes
through Direct3D resource creation and an OS wait; that one sample does not
prove a persistent Direct3D deadlock. The active Lua state provides the
specific incomplete activity condition. DWORD stack scans are candidates,
not unwound call stacks. Task Manager supplied AMD64 contexts; the analysis
also recovers validated WOW64 x86 contexts through TLS slot1.

## Scope still open

The present evidence supports retrying with a valid garage model. It neither
certifies all activity transitions nor demonstrates that larger limits are
needed. A future optional validity diagnostic could report an unavailable
player/mission request rather than allowing a silent indefinite wait; that
would be separate implementation work, with no hardcoded car list required.

Keep GENERIC_R1 unchanged. Retain prior paired original/modified successes
and the known remaining replay/transition coverage limits.

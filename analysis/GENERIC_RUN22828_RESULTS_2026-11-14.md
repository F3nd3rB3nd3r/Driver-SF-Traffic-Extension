# Run22828: activity retry and extended150-car session succeed

Inputs: `dist/MAIN_150/DSFV29s_generic_limits_22828.log` (19,290,675 bytes)
and user-supplied DriverNGHook `Driver.log` (222,284 bytes). Their last-write
times are about1.55seconds apart. Driver.log has no explicit PID/timestamps
for the Lua flow; its association with22828 follows the user's supplied pair,
not independent per-line clock synchronization. Evidence:
`analysis/generic_run22828_2026-11-14/`.

Read-only analysis of supplied logs. Production source/ASI/INI, game assets
and profile are unchanged; no build, game launch or debugger was performed.
Only analysis artifacts and handoff notes were updated.

## Result

User confirms Race Takedown Activity5 loaded after choosing another garage
car, the game quit normally, and no missing audio, rendering problems or
unusual FPS drops were noticed. Driver.log confirms the previously blocked
loader now selects **model212** rather than122, advances through Load Activity
Vehicle -> Load Data -> Fade Up, launches StreetraceTakedownActivity5, creates
its instance and records its completion save. Mission-end loading finishes.

This supports the unavailable saved garage model diagnosis for24048. The
earlier Forza installation remains a plausible source of that saved ID, not
proven provenance. Keep GENERIC_R1 with the current default limits. This run
does not indicate a need to increase a capacity or produce another ASI.

## Loading/flow evidence in Driver.log

| Mission/activity | Launch | Completion save | End-loading DONE |
|---|---:|---:|---:|
| StreetraceTakedownActivity5 |1463|1565|1675|
| DownhillDrift |2063|2160|2268|
| ChaseActivity4 |2807|2902|3012|
| StreetraceTakedownActivity4 |3304|3417|3527|

Numbers are file line numbers. Loaders for the three activity-vehicle spawn
paths log model212 at1448,2783,3289. All four challenge-loading flows reach
Fade Up. Later log records return through script/config initialization;
normal process exit is established by user confirmation, not by an absent
fatal line or a definitive OS-exit event in the logs.

## Capacity, readiness and lifetime telemetry

Same generic default AssetWeights config:150 requests,192 audio-model
entries,2048 global audio slots,64MiB default audio,1MiB replay reader,
256 retained references;160 model/graphics slots.

- About747seconds (12min27s) of ASI telemetry. Eight accepted asset-derived
  requests, each18->150. Seven later phases each reach all150 READY. Initial
  phase1 is superseded at143/150; phase2 completes. No rejected/generic-fatal
  requests or compiled-car-list fallback.
- Parked:150 candidates, largest category21,11,800 picks, all selected;
  10,245 selections use the extended range.0 invalid/empty/guard errors.
- Pending-map extension is exercised: peak154, one redirect,154 recycled,
  final current0;0 invalid/full/allocation/guard failures.
- Cleanup queue peaks51,821, records19,053 insertions beyond stock capacity
  (one threshold crossing), drains
  to0 with261,507 drained and matching drain-start/end counts.0 full/invalid/
  guard/drain-mismatch faults. This is meaningful extended-path coverage.
- Callback and arena integrity checks pass. Callback occupancy stays within
  stock64; the beyond64 extension is not exercised by this workload.
- Registry one owner/one construction,105 external nodes,0 faults; no registry
  reconstruction/reuse coverage. No notification sidecar-copy exercise.
- Replay records142 captures/saves,0 imports/loads/restores; reader owners0.
  Reader integrity checks pass as unexercised. Do not claim playback/replay
  transition coverage from recording snapshots.

Baseline, callback and reader structural checks pass, including inherited
queue/map checks. Generic checks all pass **except audio_setup**, discussed
below. READY remains a sequential native-predicate sweep, not proof of150
simultaneously visible/drawable vehicles.

## Audio observations: retries recover, not an unconditional all-clear

Final native audio setup counters:1,079 attempts,956 successes,123 failures.
Failures affect19 model IDs:
62,135,138,144,156,183,184,195,197,202,229,232,233,249,251,272,278,279,303.
Every one has an increased success counter **after** its final failure count
was observed (`audio_model_followup.json`). Thus these are not19 permanently
silent models on the supplied evidence. User reports correct audible results.

Failure counters rise in short groups around78-79s,447-448s and512s. These
are periodic observation times, not exact event timestamps. No additional
failures are recorded after512.609s through the session's end. Do not equate
the generic analyzer's zero-failure rule with a proven audible regression,
or erase the failures because playback sounded correct.

Registry has0 full/invalid/instance/key overflow errors. Global slots have0
ceiling hits; observed update peaksA0/A1/B=236/53/39, below2048. Default pool
has0 sampled pressure events. These observations do not support increasing
audio capacity. The reason for native transient setup failures remains open
if a subsequent audio investigation is requested.

## Separate script/startup findings

Driver.log contains a caught Lua error at577:
`regionArtStatus.lua:23: attempt to call field 'setRegionArtStatus' (a nil value)`.
Trace points through DriverNGHook's user-update wrapper during loadFiles,
before Game Running at862. Later mission flows complete. This is real
startup-script evidence, but no causal link to the150-car streaming limits
or previous model122 wait is established. Its underlying implementation/
optional-debug provenance has not been inspected. Do not silently patch it
or label it a rendering root cause from this log alone.

Other log entries include missing RouteDataE3Files/dailyupdatedisplay scripts,
optional user bootstrap/settings files, and an online-authentication failure.
They do not stop the recorded activity flows. No additional repeated user-
update error, out-of-spool-slot warning or activity-loader traceback appears.

## Next

Retain GENERIC_R1 and current configuration. The specific activity softlock
has a successful retest after changing the garage car. Remaining general
coverage includes replay playback/import/restore, registry reconstruction,
broader campaign transitions and later configurable-capacity widening work;
this run alone does not certify those. Preserve the transient audio and
startup Lua findings for any explicitly requested follow-up.

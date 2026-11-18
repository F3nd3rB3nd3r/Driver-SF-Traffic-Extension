# USER CAP R4 run20408: retirement overflow fix exercised successfully

Input: `dist/USERCAP_R4/DSFV29s_usercap_r4_20408.log`,29,814,620 bytes,
100,590 parsed records, SHA256
`e9cd29f8c78c7587b8ee00dd9f68211561394fd340c6672bd7722ccc1022cbb8`.
Recorded elapsed time reaches1,297,141ms (~21m37s). Analysis outputs are in
`analysis/usercap_r4_run20408_2026-11-15/`.

The user reports everything seemed fine, confirms Race Takedown Activity2 ran
without abnormal behaviour, and says the session ended with Alt+F4. No normal
main-menu exit is claimed. The log remains in Story recording at its final sample.

## Conclusion

This is successful live coverage of the previously crashing activity transition
and repeated large/small traffic replacement with R4. RetireIds exceeds its old
50-node limit, uses the new external backing, repeatedly recycles it and records
no failure/guard damage. Keep R4 and the current configuration; no further limit
increase or new ASI is indicated by this run.

This is a targeted queue-fix result, not a claim that all R4 lifecycle paths or
every Forza asset/audio effect are certified. Audio has recovering setup warnings
described below; the strict generic analyzer therefore does not report a blanket
all-checks pass.

## Configuration, rosters and transitions

Recorded configuration is valid AssetWeights withRequestCapacity160,ModelSlots170,
AudioModelEntries192,GlobalAudioEntries2048,DefaultAudio64MiB,ReplayReader1MiB and
ReplayRetainedReferences256. All three new queues have successful startup status,
capacity512 and the expected12/20/20 strides. Notification, parked, replay,
callback, pending, registry and resource startup metadata match the configured
geometry. No fatal record or truncated final record occurs.

There are10 complete accepted request phases, five18-ID and five152-ID phases.
All10 reach their full native READY predicate count in a recorded sweep. This is
sequential READY coverage, not proof of every model's drawable pixels at once.

| Event tick(ms) | Setting hash | Effective IDs |
|---:|---|---:|
|25922|AE88E83B|18|
|29328|47EB4D0E (Chapter7)|152|
|254812|40868917|18|
|357797|47EB4D0E|152|
|479047|EF19694A|18|
|611906|47EB4D0E|152|
|768656|1B53D236|18|
|809484|47EB4D0E|152|
|847781|AE88E83B|18|
|912750|47EB4D0E|152|

Thus four complete152->18->152 cycles follow initial loading. All five152-ID
rosters exactly match the native-plus-positive union independently derived from
the supplied expanded AI, SHA256
`a518f1b044ebd03c52db8a6fd349a96ec8b78b7484e4ab9eb0ca963e7fe31492`.
Generic events show five augmentations and five preserved native lists,0 failures.

## New queue storage

| Queue | Insert checks | Projected active peak | External buffers/redirects | Recycled insertions | Failures/guard damage |
|---|---:|---:|---:|---:|---|
|retireIds|646|147|1 /1|499|0 /0|
|retainedVehicles|63|14|0 /0|49|0 /0|
|vehicleOperations|117|7|0 /0|110|0 /0|

The retirement peak147 exceeds the old50-node storage and exercises the fix
directly.147 fresh issued nodes plus499 recycled insertions account for all646
insertions. The final retirement count is0, with147 recycled nodes retained for
reuse. No fresh external backing is repeatedly accumulated in this owner lifetime.

Each queue records one generation and0 resets. This verifies reuse within the
same live owner; same-address reconstruction remains offline-only coverage.
The two20-byte queues stay within their inline50 nodes in this run, so their
external expansion likewise remains offline-tested. Their live guards are clean.

## Other monitored storage

No callback/pending/cleanup/arena/parked/registry invalid/full/guard/allocation
fault is recorded. Pending reaches147/512, uses503 recycled insertions and ends
at0 active;916 walk starts/ends balance. Cleanup peaks at32641/262144, ends0,
and127495 drain begin/end pairs balance. These are final periodic observations,
not an OS-exit snapshot.

Parked storage rebuilds9 times for one owner, reaches152 candidates, performs2744
selections including736 beyond the original50-entry prefix, and has no invalid,
guard or concurrent state. Body diagnostics record21492 calls with0 unmapped
calls; their coverage still cannot certify every vehicle's visible rendering.

## Audio caveat

2352 native audio setup attempts include2187 successes and165 failures across
12 model IDs:127,148,154,158,176,179,251,272,274,278,281,286. Each model has a
later sampled record with an increased success counter after its last observed
failure increase. Therefore recovery is observed per model; this does not prove
that every failed individual emitter or sound instance was retried successfully.

These warnings do not coincide with recorded capacity exhaustion: registry
peak164/192 with0 full/misses/guard failures, global-list peaks255/54/85 against
2048 with0 ceiling hits, emitter refusal0, allocator failures0, object8/255-slot
refusals0 and16-event refusals0. Shared default audio peaks at17,177,308 bytes
(16.38MiB) against64MiB with0 sampled pressure crossings. All878 bank callbacks
report result1. There is no evidence here that another budget increase is needed.

The reason probe reports19880 eventOverflow occurrences. Source confirms that
this is its bounded512-entry diagnostic event directory, returning no detail
record when full; it is not the game's live global sound list. Detailed reason
coverage is consequently incomplete. This counter must not be presented as
19880 failed game sound allocations or unique missing events.

## Analyzer scope and remaining lifecycle checks

The current generic analyzer passes configuration, geometry, all monitored
integrity/storage checks and both new queue checks. Its sole false check is
`audio_setup`, which requires zero cumulative setup failures; raw output and
`recorded_integrity_pass=false` are retained unchanged.

Historical BASELINE150/reader/client tools additionally reject their hardcoded
150/160 startup values and150 roster ceilings. This run uses160/170 and152 IDs.
Those mismatches are recorded, not treated as runtime corruption. Separate
current-schema checks validate parked-build uniqueness/counts, replay capture
bounds, reader/client configured startup and all READY phases. No production
analyzer or source was changed to force a pass.

Replay captures/saves249 snapshots with peak152 traffic/14 retained references;
there are0 loads/imports/restores and0 reader constructions. This exercises
recording, not replay playback or nonzero-retained restoration. The sampled
world remains the same Story simulation/recording, without a main-menu/re-entry
or teardown interval. R4 menu/Story reconstruction, replay playback/restoration
and normal in-game shutdown still need separate user coverage if that scope is
chosen next. Alt+F4 ends the supplied test; it is not normal menu-exit validation.

No production source, ASI, INI, asset/save or game installation change, build,
game/debugger launch, commit or push occurred during this analysis.

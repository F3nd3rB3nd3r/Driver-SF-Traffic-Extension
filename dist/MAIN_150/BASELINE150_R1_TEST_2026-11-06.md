# BASELINE150_R1: controlled roster/readiness logging

**Direct150 Lua request test withdrawn after two live crashes.** A native
50-ID notification stack buffer overflows on the150 request and reads the
wrong source beyond50. Do not run `DSF150Baseline.request()` again until patched.
See `analysis/BASELINE150_CRASH_PAIR_2026-11-06.md` for the dump-backed finding.
The procedure below records the original test, not a currently safe instruction.

This is an experimental logging build based on the validated ARENA_REUSE_R1.
It preserves150 requests,160 graphics/model-definition slots,ED/EF sidecars,
indexed262144,resource collection131072,cleanup262144,pending512,
audio-model192,global audio slots2048,shared-default audio64MiB andcallback512.
It does not implement an INI, expand any capacity, change the installed AI
asset, or certify150 drawable/audible models.

## Install and first run

1. Close the game. Keep the working ARENA_REUSE_R1 ASI outside the enabled
   plugin directory as your rollback copy.
2. Replace it with `DriverSF_V29s_160Capacity_150Request_BASELINE150_R1.asi`.
   Enable exactly one traffic ASI. This package was staged, not installed.
3. Start a fresh process and load your usual save normally. Keep the current
   tested AI asset for this comparison. Automatic supported-profile150
   requests are recorded before you issue any Lua command.
4. Confirm `%TEMP%\DSFV29s_baseline150_<PID>.log` contains
   `DSFV29s_BASELINE_START revision=1 status=1`. A negative status means the
   new probes are unavailable; do not count that run as a baseline test.

The build has two extra hook sites: the traffic-spooler update entry for
readiness sweeps, and the native vehicle-body draw CALL for observations.
Timing/performance can therefore differ from ARENA_REUSE_R1; report rough FPS.

## Controlled Lua request

Run the contents of `BASELINE150_R1_LUA_TEST.lua` in your existing Lua execution
tool. Loading that helper only defines its table/functions; it does not issue
a request, spawn/delete vehicles, or switch traffic settings automatically.

After normal loading finishes, call:

```lua
DSF150Baseline.status()
DSF150Baseline.request()
```

The helper uses the exact ascending150-ID vanilla roster already compiled
into this build. Its request is `TrafficSpooler.RequestTrafficVehicles(#ids,ids)`.
Wait at least10 seconds, then call `DSF150Baseline.status()` again.
This tests the direct Lua adapter and a warm re-request. The normal initial
load provides the separate initial-loading observation.

Do not replace it with the three-ID example while the civilian picker still
selects the full150 roster: requests replace the roster and do not change
picker weights. A native named traffic-setting change can subsequently
replace the direct Lua request; the log records that boundary.

## Exercise the workload

- Drive through several areas, use Shift and the garage, and check a range of
  models, including previously silent and working examples. Note rendering,
  engine/tire/horn/siren behavior and rough FPS. Missing draw coverage can
  simply mean a particular model was never displayed.
- Exercise named settings through `spooling.spoolTraffic("Chapter 1")`, then
  a different supported setting such as `spooling.spoolTraffic("Chapter 7")`,
  one transition at a time, allowing the fade/loading sequence to finish.
  Repeating the same name/frequency can be skipped by the Lua wrapper cache.
- Repeat the normal Challenge/garage/free-drive transitions used in the
  successful run. Return to the main menu, load Story again, then exit normally.
- Traffic-setting names do not replace actual chapter saves, interesting-
  vehicle regions or all mission setup. Actual chapter/mission checks remain
  a separate part of validating the preset.
- The150-model roster contains trailers/special vehicles. Do not require
  engine or horn sounds from models which do not normally have them.

Copy the new `DSFV29s_baseline150_<PID>.log` into `dist/MAIN_150` afterward,
and report the sequence, visible/audible outcome, FPS and how the run ended.

## What the records mean

- `BASELINE_PHASE kind=accepted`: emitted after the full request was copied
  and its count published, before the native processing call. It is not a
  load-completion record. Direct Lua caller return is0066E5F8; the named setter
  return is0076FE9A. Rejections retain the previous active diagnostic phase.
- `BASELINE_ROSTER`: exact original/effective IDs, one sequence per accepted
  request, with no24-request cutoff. Hash is a word-wise FNV-style check, not
  a cryptographic digest. Require150 unique expected IDs for the full test.
- `BASELINE_PHASE ready=150`:150 successful native0059CFC0/type0 predicates
  in one bounded game-thread sweep. Check `checkpoints`, timestamp, duration
  and age. Sweeps are sequential, not atomic against worker threads. An old
  READY observation after menu/unload is not current readiness.
- `BASELINE_MODEL`: readiness/loss observations, calls with the original
  model key, resolved model-definition key, and observed body fallbacks.
  This records the CALL to008A8DA0 at008D590C; it is not a GPU/pixel guarantee.
- Audio fields are cumulative counter samples frozen before supersession,
  with their eight `audioAtStart` values in the same field order. Registration,
  lookup, bank readiness and setup outcomes remain separate. Setup failure
  reasons and actual audibility are not inferred.
- `BASELINE_METRIC`: per-request start/last/sampled peaks for published queue,
  callback and audio-slot gauges plus process private/working bytes. Emitter,
  hash-extra and spool-arena fields explicitly describe lifetime counters.
  Inherited exact/projected/lifetime peak records remain in the same log.
  Whole-process memory includes the world and diagnostic overhead.
- Shared-default audio bytes remain in `AUDIO_PRESSURE`. Divide bytes by
  1048576 forMiB. Geometry/texture allocation and GPU usage are unmeasured.
- `BASELINE_STATUS`: event drops, contention skips, obsolete checks, unmapped
  body calls and process-memory snapshot failures. Drops make request coverage
  incomplete. Skips can omit observations; they do not change game behavior.
  Event storage is bounded to eight queued snapshots; the monitor drains it.
- `superseded` closes a diagnostic request phase; it is not proof of unloading.
  There is no final-exit hook/snapshot. Retain the inherited pending/cleanup
  completion evidence and the user's exit outcome.

## Analyze one supplied log

```powershell
python scripts/analyze_baseline150_log.py dist/MAIN_150/DSFV29s_baseline150_PID.log --output analysis/baseline150_PID/coverage.json
python scripts/analyze_callback_log.py dist/MAIN_150/DSFV29s_baseline150_PID.log --output analysis/baseline150_PID/inherited.json
```

Neither analyzer certifies visible/audible success or universal pool coverage.
The second analyzer supplies the inherited queue,arena,audio and unload checks.
The build/native fixture evidence is under `analysis/baseline150_r1_2026-11-06`.

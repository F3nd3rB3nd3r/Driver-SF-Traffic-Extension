# R9 diagnostic candidate and remaining coverage

R9 extends R8 with audio setup failure classification and keeps its capacities,
traffic selection and native return policy. A Release/Win32 candidate and
three production-parser-validated INIs are in `dist/USERCAP_R9`. The installed
game, assets, INI and saves have not been changed; no game/debugger was launched,
and no commit/push was made. R8's packaged ASI is preserved byte-for-byte.

The recommended first runtime test combines the new audio diagnostics with
activity returns, Lua named-setting changes and Story/menu/Replay lifecycle,
using the existing280-car assets and unchanged280/HASH180K settings. The user
has Chapter7 plus Lua switching. Earlier chapters' actual mission/progression
coverage therefore remains a separate future task.

## Diagnostic implementation

`src/V29sAudioSetupTrace.inc` installs ten signature-checked hooks: six outer
failure branches and four blocks containing mode-helper state/snapshot checks.
The seventh outer reason reuses the existing bank-readiness hook. It does not
patch that site a second time. All new spans match the supported EXE and are
disjoint from the eight inherited audio-model spans. Original CMP/TEST results,
branches and continuations are reproduced explicitly; copied relative-branch
trampolines supplied by the generic installer are rollback storage only and
are never executed. All sites are preflighted; failed installation restores
the new spans and refuses diagnostic startup rather than partially arming it.

Each bridge preserves GP registers, flags, direction flag, x87 and SSE state.
Outer branch hooks consume the original saved flags. Mode-helper hooks read
the tested value once and use it for both diagnostics and the native decision.
Subreasons are recorded only for helpers called from the audited setup
function, excluding unrelated controller reconfiguration calls.

The seven cumulative outer reasons are vehicleDisabled, bankNotReady,
modelProfileMissing, defaultProfileMissing, previewSuppressed, invalidMode and
modeInitializer. The three additional subreasons are vehicleState,
snapshotGetter and snapshotFlag. Sum only the seven outer counters when
comparing with the existing setupFail counter; the subreasons describe some of
those same failures. A concurrent poll is not an atomic counter snapshot.

Failure details include time, reason, model/selected model, mode, caller,
controller/vehicle and accompanying profile/default/state/preview samples.
Saved branch flags identify outer decisions; `observed` identifies a tested
state/snapshot subreason value. Fields ending in Sample are accompanying state
and must not be substituted for the tested condition if they differ. The mode
comes from controller+88 after eligibility because the native function reuses
the incoming mode argument slot as output scratch at00553966. The bank hook
reports the TEST flags corresponding to the original getter return.

A fixed1024-record nonblocking queue is drained by the existing monitor for
the whole session; only failures use it. Ring saturation or producer contention
increments detailDropped without losing cumulative reason counts. There is
no allocation or file I/O in the failure producer. This avoids the previous
first128-result budget being consumed by early successes. It does not promise
lossless details under arbitrarily high failure rates.

`AUDIO_SETUP_START/STATUS/DETAIL` records accompany the existing schema. R9 has
its own log prefix. The generic parser requires R9's trace startup/status and
reports lost details, while retaining all existing R8 safety/startup checks.
The strict setupFail==0 condition is unchanged; reasons are evidence for review,
not automatic permission to declare any observed failure harmless.

## Offline verification completed

`scripts/test_audio_setup_trace.py` compiles/runs15 isolated Win32 fixture
processes, never the game entry point:

- One focused test covers all outer branch outcomes, all three subreasons,
  native CMP/TEST flag comparisons, unrelated helper callers, argument-slot
  reuse, continued late-session recording, queue saturation/contention/reuse,
  ten site signatures and20 actual bridge ABI invocations.1084 assertions pass.
- Existing production/native storage tests pass at18,150,255,256,280,500,700
  and1000 capacities, including asset-derived synthetic rosters and parked,
  queue, stack-frame, audio-directory and replay snapshot storage paths.
- Complete cold installer sequences against a copied native image pass at
  18,150,280,700 and1000 capacities, plus the existing customized700 setup.
  This includes successful installation of the new hooks after the parents.
- The actual packaged280,150 and320 INIs parse with the production validator;
  their model/work capacities derive to290/802,160/512 and330/1098 respectively.

`scripts/verify_audio_setup_trace.py` validates the compiled PE32 hook table,
native signatures, branch bridges and parent-hook non-overlap. It checks the
new log schema with mutation cases and keeps the clean R8 run3600 accepted.
The independent existing generic-parser tests retain all13 negative cases.
Startup-only fixtures remain explicitly insufficient for a gameplay pass.

Evidence: `analysis/audio_setup_r9_2026-11-17/fixtures.json`,
`fixture_manifest.json`, build logs and `build_verification.json`. The latter
contains the exact ASI size/hash. Standard Release/Win32 build succeeded with
the existing unused-function/exported-entry warnings. MSBuild's sandboxed
file tracker was denied access; the authorized elevated retry succeeded.
No game launch was involved.

These are new diagnostic/configuration checks, not a claim to have re-run all
historical experiment fixtures or to have live-tested500/700/1000 models.

## What broader activity/chapter coverage still needs

The R8 long Story run verifies the formerly frozen Getaway1 return with280-car
traffic and user-observed normal rendering/audio/FPS. The shorter R8 run3600
adds Story-to-Replay. The complete reverse lifecycle and a clearly identified
normal quit remain useful R8/R9 coverage; earlier successful versions retain
their original evidence scope. The first R9 sequence repeats Getaway1, Race
Takedown2/5, another activity/dare/chase and both Replay/Story directions.

The current generic selector reads the active setting hash and frequency,
looks up the setting in the loaded AI arrays and unions its native list with
positive-weight models from the referenced tables. It contains no active
Chapter7-only profile choice. Logs already expose those hash/frequency/count
changes and READY checkpoints. Existing logging is sufficient for roster,
pool and lifecycle comparisons; the new audio reasons fill the identified gap.
Activity/mission names and observed sound/render quality still need the user's
short ordered notes, optionally matched with DriverNGHook's Driver.log.

The explicitly supplied `spoolingsystem.lua` was read at its exact path only:
`E:\_DSF Mods\LuaSF_new\Resources\ScriptsLuaScripts\gamelogic\spoolingsystem\system\spoolingsystem.lua`.
Its wrapper waits for traffic and road updates and maintains cached previous
setting/frequency. Use `spooling.spoolTraffic` for the first combined sequence;
it defaults to frequency0. Direct civilianTraffic setter calls bypass those
Lua caches and should be a separate frequency test. Do not mix the methods
and assume that a same-name wrapper call necessarily reloads traffic.

Lua switching can exercise earlier/later chapter traffic hashes and different
asset rosters, but does not load the actual chapter's missions, unlocks,
cutscenes or progression. It is not a substitute for those future chapter
saves/playthrough tests. The separate Lua mission helper's18-item local list
is not the traffic roster capacity; this work does not change it.

## What other configuration validation needs

Use separate processes and keep one axis identifiable:

| Comparison | Recommended setup | Remaining runtime observation |
| --- | --- | --- |
| Known baseline plus broader flows | Current280 AI/assets and unchanged main INI | First combined R9 run, audio reasons and lifecycle/activity outcomes. |
| Original small roster | Original AI with Baseline150 INI | Native-sized requested rosters, normal driving/audio, activity return and replay; capacity150 is headroom, not a forced roster. |
|150 roster |150 AI/assets with the same Baseline150 INI | Actual150 requests and READY, visible/audible cars, parked variety and transitions. |
| Other valid geometry | Same280 AI/assets with Headroom320 INI | Derived330 slots/1098 work entries,448 audio profiles,768 retained refs and4MiB reader under real driving/replay. Actual roster remains280. |
|500/700/1000 actual models | Correspondingly populated AI, installed catalog/assets and sufficient configured capacities | Real high-count residency, audio asset demand, memory use, activity/replay changes and performance; not proved by numeric startup alone. |

No per-car MiB formula or automatic safe upper roster size has been inferred.
Fixed representation constraints still apply: model IDs must fit the native
identifier format; setting/probability-table selectors remain bytes in the
audited path. Invalid/removed garage or AI model references must be separated
from capacity failures. Keep asset/probability data constant when comparing
two INIs. Keep the INI constant when comparing original and150 AI data, while
recording the actual effective requests rather than its configured ceiling.

Detailed user sequence and profiles: `dist/USERCAP_R9/R9_TEST.md`.

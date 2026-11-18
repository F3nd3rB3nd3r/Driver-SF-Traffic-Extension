# Native roster / optional150 preset: source candidate

Build follow-up: now compiled, offline verified and staged. Read
`TRAFFIC_POLICY_R1_BUILD_2026-11-14.md` for the current artifact/evidence.
The compilation-pending statements below record the earlier source handoff
and are superseded by that report; no live-game result yet.

2026-11-14. Implements the requested separation after the static audit.
Source and the active project are prepared. No C++ compilation, ASI build,
installation, native execution or game run has been performed. The last
live-tested binary remains REPLAY_READ1M_R1. This is not a game-verified or
release-ready replacement yet.

## Behavior

`DriverSF_Traffic.ini` beside the ASI selects the startup-only policy:

```ini
[Traffic]
RosterMode=Native
```

- `Native` defaults to preserving the game's requested IDs and order. It
  supports named settings unknown to the compiled preset profiles without
  consulting preset asset fingerprints or substituting a list.
- `Preset150` explicitly enables the previous55 eligible profiles with the
  same asset/source/caller/owner/graphics/admission checks. The25 excluded
  profiles and unknown/mismatched requests retain their native lists.
- Both modes retain executable/setter-byte startup validation,150-ID request
  bounds,16-bit ID validation, complete-list-before-count publication,
  sidecars, empty/negative clear semantics and native processing dispatch.
  No weight/population changes or new native hooks are introduced.
- Capacity remains150 requests/160 graphics and definition slots. Audio,
  ED/EF, pending/callback/cleanup, parked, replay snapshot, REGMAP lifetime
  and the1 MiB reader implementations remain intact.

The new `DSF_V29S_TRAFFIC_POLICY` flag requires CHAPTERS at compile time:
removing CHAPTERS cannot silently activate the legacy Chapter7 preload path
in this candidate. Builds without the new flag retain their old policy.
RequestWriteBridge itself is unchanged. Native mode returns before inspecting
the preset's context/profile/asset; the bridge still validates actual input.

## Configuration and diagnostics

Two includes provide a bounded parser and startup file loader without CRT or
Windows profile APIs. The path derives from the ASI module, never the current
directory or Driver.exe. Wide file APIs support the installation path. Static
buffers avoid a large stack frame. No config writes, worker initialization
race or live reload. File limit4096 bytes; a4097-byte read detects overflow.

ASCII settings/UTF-8 BOM, case-insensitive names/values, whitespace, CRLF and
`;`/`#` comments are supported. Missing file/section/key selects Native.
Invalid values, duplicate RosterMode, malformed sections, binary/control data,
oversized files or I/O/path errors also select Native with an explicit reason.
The new analyzer rejects configuration-error reasons as failed intended-mode
validation. Only an explicit valid setting enables the150 preset.

`DSFV29s_TRAFFIC_POLICY_START` records mode, reason, config location and fixed
capacities. CHAPTER_START records mode and whether preset asset checks apply.
Named native requests log `nativeRoster` with a separate counter. Native mode
does not inspect hash/frequency, so those event fields are0, not a diagnosis
of a bad traffic hash.

Request-reader diagnostics compare against actual accepted sidecar IDs, and
the monitor's `expected` field uses that captured value. A valid different150
list no longer produces a canonical-roster mismatch. Bounded native lists are
also observed. Fields remain cumulative/sampled, not an atomic thread snapshot.

## Project and completed checks

The single active Release/Win32 project targets
`DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1.asi` in
`build/v29s_traffic_policy_r1/`. No such ASI is built/staged yet. Separate output
preserves the existing binary and its successful live result.

- `python -B scripts/verify_traffic_policy.py --static-only`: passes project
  XML/includes, exact inherited flags plus the policy flag,150/160, separate
  output, startup order, policy separation, actual-list diagnostics and byte
  identity of24 unchanged inventoried inputs. These include the previous ASI
  and audio/queue/parked/replay cores.
- `python -B scripts/test_traffic_policy_logs.py`: synthetic Python parser
  cases pass for native order/noncanonical150, zero/negative clears, config
  defaults/errors, preset expansion and caller/roster guards, wrong startup/
  capacity, duplicate startup and truncated records.
- Python syntax and `git diff --check` pass.

Evidence: `analysis/traffic_policy_r1_2026-11-14/static_checks.json` and
`parser_tests.json`. Static evidence records current source hashes. These
checks do not prove the production C++ compiles or behaves correctly live.

## Prepared checks awaiting explicit build authorization

`python -B scripts/verify_traffic_policy.py --fixture-only` is prepared to
compile/run an isolated Win32 fixture using production config/parser/chapter
code and extracted request/probe bodies. Coverage is intended for all80 native
profiles without AI access;55 preset expansions/25 native profiles; arbitrary
150 IDs and inline/sidecar integrity; invalid-count/source/ID rejection retaining
the previous list; clears; config/path/read failures; preset asset/admission/
caller guards; executable/setter preflight; and idle dispatch. Synthetic AI
and native services/return-address identity are stubbed. It has not been
compiled or run and is not a compiled-ASI ABI/installer test.

Next: compile/run that fixture, build the active solution Release/Win32, verify
PE32 imports/request ABI/retained hooks and geometry, then stage ASI+config+
guide. Earlier verifiers are hash locked to earlier source. Do not present old
results as compiled verification of this candidate or blindly invoke old
configure/package entry points targeting earlier names.

The repository instruction says "Do not build ... unless specifically asked";
this implementation turn therefore stops before compilation. Nothing was
installed or changed in either game installation.

## Live acceptance plan

Guide: `dist/MAIN_150/TRAFFIC_POLICY_R1_TEST_2026-11-14.md`.
Example: `dist/MAIN_150/DriverSF_Traffic.ini`.
Future log: `%TEMP%/DSFV29s_traffic_policy_<PID>.log`.

Test original AI +Native separately from the previously tested uniform150 AI
+Preset150. Current adjusted AI may weight150 models but still request18
natively: Native does not reconcile that mismatch. Use Preset150 for that
file unless its actual named request lists are corrected. Check actual lists,
then readiness/rendering/audio and Story/Challenge/menu/Replay teardown. No
arbitrary INI capacities or700-model certification are introduced here.

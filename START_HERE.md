# Start here: DriverSF Unlimiter R10

The current published candidate is **USERCAP_R10**, commit
`9900ef5595fefe6081356d277212a026f076f46d` on GitHub main.
Read [NEXT_ACTION.md](NEXT_ACTION.md) for remaining work and task boundaries.

## Active files

- Solution: `DriverSF_V29s_NATIVE_TRACE.slnx`.
- Project: `DriverSF_V29s_ARENA_REUSE.vcxproj` (the name is historical).
- Build configuration: Release / Win32 only.
- Build output: `build/v29s_usercap_r10/`.
- Candidate: [dist/USERCAP_R10](dist/USERCAP_R10/README.md).
- Configuration: [CONFIGURATION.md](CONFIGURATION.md) and
  [config/DriverSF_Unlimiter.ini](config/DriverSF_Unlimiter.ini).
- Machine-readable state: [CURRENT_BASELINE.json](CURRENT_BASELINE.json).

The R10 ASI is 327168 bytes, SHA256
`11df34ddf0700f9032f5f75d7e8e71547c366d89b6b7384ce853ac63b96825f9`.
Enable exactly one traffic-extension ASI. Existing asset weights and installed
vehicle assets supply the roster; there is no active hardcoded car/chapter list.
The default INI provides 150-capacity backing. Capacities are configurable:
do not impose that baseline on the user's existing 280/320 setup.

## What is established

R8 fixed the Getaway activity manager-key overflow. R9 retained those fixes
and added audio failure classification. R10 retained the capacity/native
decisions and added output levels: 0 minimal, 1 summaries (default), 2 full.
All safety hooks/counters remain active. No FPS improvement is claimed.

- [R9 combined test](analysis/USERCAP_R9_RUN26476_RESULTS_2026-11-18.md):
  49m35s, activities, Lua traffic changes, Story/menu/Replay and normal Quit;
  280-model workload. All 132 audio setup failures were native preview suppression.
- [R9 Headroom320](analysis/USERCAP_R9_RUN7044_RESULTS_2026-11-18.md):
  320 capacity with 280 actual models; request phases READY, 4 MiB reader exercised.
- [R10 Story](analysis/USERCAP_R10_RUN20268_RESULTS_2026-11-18.md) and
  [R10 Replay](analysis/USERCAP_R10_RUN23108_REPLAY_2026-11-18.md):
  positive user reports, 280 models with 320-capacity backing, roughly
  129-135 KB logs over five minutes each. Replay import/restore hashes match;
  reader/guard faults absent. These are summary-mode smoke tests.

READY alone does not certify simultaneous drawable residency. Summary logs
deliberately cannot certify every full-trace check. No log independently
proves the exact final quit/teardown sequence.

## Open scope

Actual 500/700/1000-model gameplay, other chapters' missions and nonzero
traffic-frequency coverage remain unverified. Lua setting switches do not
prove other chapters' progression works. The worked 500-model INI is explicitly
experimental, not a per-car memory formula or a guarantee of support.

Run23108 recorded one local native 16-event audio refusal without an observed
audible symptom. This is separate from configurable global/model audio budgets;
it does not justify a blind increase. No new runtime fix is indicated by the
latest supplied tests.

## Organization

The 2026-11-18 cleanup consolidates the accumulated handoffs and archives
superseded reports/closed evidence. See [the archive](archive/retired_2026-11-18/README.md)
and [analysis index](analysis/README.md). Original paths, bytes and SHA256 hashes
are preserved. Older files still needed by tooling remain in place as
prerequisites. Do not treat their dated next-step instructions as current.

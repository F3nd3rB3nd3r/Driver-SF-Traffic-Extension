# DriverSF Traffic Extension / Unlimiter

Extends Driver: San Francisco civilian-traffic resource capacities. Traffic
models and probabilities come from the installed AI data and vehicle assets;
the mod has no active hardcoded car/chapter roster.

Read [START_HERE.md](START_HERE.md), then [NEXT_ACTION.md](NEXT_ACTION.md) for
workspace state. Read [CONFIGURATION.md](CONFIGURATION.md) for every INI key,
automatic calculation, tested profile and logging level.
For a larger roster, start with the
[worked 500-model setup](CONFIGURATION.md#setting-up-a-larger-roster-a-worked-500-model-example).
It explains the individual budgets and includes an explicitly untested example
INI; the current gameplay evidence still covers actual rosters up to 280 models.

The current repository candidate is **USER CAP R10** with optional verbose
output. Its [downloadable files and installation instructions](dist/USERCAP_R10/README.md)
are in `dist/USERCAP_R10/`, including the tested ASI, baseline INI and optional
profiles. The active Release/Win32 project builds to `build/v29s_usercap_r10/`.
The last broadly activity/Replay-tested build is **R9**, preserved in
`dist/USERCAP_R9/`. The [first R10 Story/summary smoke run](analysis/USERCAP_R10_RUN20268_RESULTS_2026-11-18.md)
now verifies successful280-model loading and compact real-game output. The
[R10 Replay smoke run](analysis/USERCAP_R10_RUN23108_REPLAY_2026-11-18.md)
also exercises4MiB reader backing and280-model import/restore with a positive
user report. These are summary-mode smoke results, not full-trace certification.

R9 completed the [49-minute combined test](analysis/USERCAP_R9_RUN26476_RESULTS_2026-11-18.md)
and the [Headroom320 test](analysis/USERCAP_R9_RUN7044_RESULTS_2026-11-18.md).
All recorded traffic phases reached native READY, and the user reported normal
visual/audio behavior. Classified audio setup rejections were native preview
suppression. These results cover the observed280-model workloads, not every
chapter mission or arbitrary larger roster. READY alone does not certify
simultaneous drawable residency.

[Default INI](config/DriverSF_Unlimiter.ini) retains the150-capacity baseline.
[Expanded280](config/profiles/Expanded280.ini) and
[Headroom320](config/profiles/Headroom320.ini) retain the values exercised in R9.
R10 adds `[Diagnostics] LogLevel=0/1/2`;1 is summary output by default and2
restores full tracing. Safety checks and native limits/decisions are unchanged.
These new INIs require the updated build; R9 rejects the Diagnostics section.

Enable one traffic-extension ASI and put `DriverSF_Unlimiter.ini` beside it.
Restart after changes. Missing assets, incorrect IDs or probabilities are not
fixed by increasing memory budgets. There is no universal MiB-per-car formula.

For development, `scripts/test_logging_policy.py` exercises the output policy,
parser, retained audio ABI and copied-image storage/startup paths. It never
starts the game. `scripts/analyze_generic_limits_log.py` checks supplied logs;
reduced logging is explicitly insufficient for a full-trace certification.
Earlier verifiers remain pinned to their original revisions and prerequisites;
see [scripts/README.md](scripts/README.md) and [tests/README.md](tests/README.md).

Full session logs/dumps, generated fixtures and retired artifacts remain local.
Do not run old experiments as active build entry points. The dated archive
preserves earlier paths/bytes and is not a second active build tree.

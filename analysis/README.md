# Current evidence and retained prerequisites

The published candidate is **USERCAP_R10**. Start with these reports:

| Report | Relevance |
| --- | --- |
| [R10 implementation](USERCAP_R10_LOGGING_CONFIGURATION_2026-11-18.md) | Output controls, parser and offline verification; dated pre-smoke state. |
| [R10 Story](USERCAP_R10_RUN20268_RESULTS_2026-11-18.md) | 280 actual models, 320-capacity configuration, compact summary logging. |
| [R10 Replay](USERCAP_R10_RUN23108_REPLAY_2026-11-18.md) | 4 MiB reader exercised, matching 280-model import/restore, positive user report. |
| [R10 refused startup](USERCAP_R10_RUN22856_REFUSAL_2026-11-18.md) | Earlier 150-capacity INI versus 280-model request; configuration corrected, guard retained. |
| [R9 audio diagnostics](USERCAP_R9_AUDIO_AND_COVERAGE_2026-11-17.md) | Diagnostic implementation and original test instructions; subsequent results below supersede pending-test statements. |
| [R9 combined run](USERCAP_R9_RUN26476_RESULTS_2026-11-18.md) | 49-minute activity/Lua/menu/Replay test; native preview rejection reasons classified. |
| [R9 Headroom320](USERCAP_R9_RUN7044_RESULTS_2026-11-18.md) | Different backing geometry with the same 280 actual models. |
| [Static capacity costs](CAPACITY_COST_AUDIT_2026-11-15.md) | Per-record bookkeeping versus shared demand; arithmetic examples are not supported-roster claims. |
| [Workspace cleanup](WORKSPACE_CLEANUP_2026-11-18.md) | Archive scope, retained inputs and verification. |

The current limits and practical 500-model example are in
[CONFIGURATION.md](../CONFIGURATION.md), not in a historical experiment's
next-step paragraph. Report conclusions retain their original date and scope.

## Current data

Keep `logging_r10_2026-11-18/`, `audio_setup_r9_2026-11-17/`,
the two R9 run directories and the two R10 run directories. Their compact
JSON summaries/manifests document the supplied runs and offline checks.
Full logs/build outputs are local prerequisites, not new gameplay validation.

## Retained older material

Older fixture/native-input directories remain because tools may depend on
their bytes. The cleanup conservatively retains filename/directory references
found in tooling, tests and fixture/distribution manifests. See
`workspace_cleanup_2026-11-18/plan.json` for the retained list. A directory's
presence does not make its old candidate or proposed test current.

Superseded reports and unreferenced closed evidence are in
[archive/retired_2026-11-18](../archive/retired_2026-11-18/README.md).
Use its manifest or `scripts/workspace_paths.py` to recover an original path.
The older 2026-11-06 archive remains intact. Nothing was deleted to reclaim disk.

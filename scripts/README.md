# Current R10 tools

Use these entry points only for an applicable task:

| Tool | Purpose |
| --- | --- |
| `analyze_generic_limits_log.py <log> --output <json>` | Analyze a supplied log against its recorded configuration and evidence scope. |
| `test_generic_limits_log.py` | Synthetic Python log acceptance/negative cases; no game/native execution. |
| `test_logging_policy.py` | Compile/run isolated R10 output-policy/parser, audio ABI and native-storage fixtures. Requires build authorization. |
| `verify_logging_policy.py` | Inspect the existing R10 build, hashes, native signatures and parser scope. Requires supported local executable/toolchain and retained inputs. |
| `audit_generic_traffic_asset.py <asset> --output <json>` | Read-only inspection of an explicitly supplied AI asset's preload/positive-weight requests. |
| `verify_workspace_cleanup.py` | Read-only verification of the 2026-11-18 archive, protected bytes and current entry points; writes a result JSON only. |
| `workspace_paths.py` | Resolve an original historical file or its unchanged archived counterpart. |

Current R10 build output is `build/v29s_usercap_r10/`; the candidate is
[dist/USERCAP_R10](../dist/USERCAP_R10/README.md).
See [CONFIGURATION.md](../CONFIGURATION.md) for supported keys and profiles.

LogLevel 0/1 intentionally omits full diagnostic detail.
`logging_full_trace=false` is an evidence restriction, not a game fault.
READY counters do not alone prove simultaneous drawable residency, and
absence of a fatal record does not prove normal Quit.

## Local prerequisites and historical tools

The native fixture/build checks require the supported local Driver.exe,
MSVC/Windows SDK and named historical inputs. R10's output-selection check
uses R9 run7044; retained older verifiers also use R8 run3600 and earlier
candidate bytes. Full user logs/dumps and generated build outputs are not
published. Restore prerequisites before reproducing the complete local chain.

The remaining revision-specific scripts are retained regression or historical
tools. Their output names, manifests and packaging modes are pinned to their
own revision; do not run them as a replacement R10 build/deployment workflow.
In particular, `verify_workspace_archive.py` audits the old 2026-11-06 state,
not today's CURRENT_BASELINE.json. Use the current cleanup checker instead.

`organize_workspace_20261118.py` is the one-time archive planner/mover.
Its archive manifest already exists; do not rerun --apply over it.
The original longer tool index is preserved in
[the control snapshots](../archive/retired_2026-11-18/README.md).

# R10: optional verbose output and configuration documentation

The user authorized next steps1 and2 only: make verbose logging optional while
preserving safety/failure reporting, and document the INI/default calculations
and tested profiles. Both are implemented. No packaging, publication, commit,
push, installation, game launch, asset or save change was performed.

R9 remains the gameplay-tested candidate. The active Release/Win32 project
builds a separate local R10 ASI at
`build/v29s_usercap_r10/DriverSF_Unlimiter_USERCAP_R10.asi` so R9's artifacts
are not overwritten. R10 is327168 bytes, SHA256
`11df34ddf0700f9032f5f75d7e8e71547c366d89b6b7384ce853ac63b96825f9`.
It has offline verification but no live R10 smoke run yet.

## Output policy

`[Diagnostics] LogLevel` is restart-only:

| Level | Output |
| --- | --- |
|0|Startup/configuration plus essential faults/refusals. |
|1, default|Traffic and Replay snapshot events, roster IDs once per phase/kind, first READY checkpoints and roughly30-second summaries, plus essential faults. |
|2|Full existing trace records at their existing frequencies, plus logging metadata. |

No native detour, resource capacity, admission, suppression, retry or cleanup
policy changes with this setting. All hooks, counters, checks and monitor
sampling remain active. Filtering happens at CapacityWrite before startup-line
rewriting/file output. This reduces file output; no CPU/FPS improvement is
claimed. Full tracing takes the unfiltered selection path.

Fatal/exception/refusal/boundary records, negative startup statuses and relevant
nonzero error counters bypass filtering. Unexpected audio reason details remain
available at every level. Native preview suppression retains cumulative
summaries at level1 while its repeated individual details require level2.
Reader fallbacks remain essential while ordinary native mission-query fallbacks
do not become false fault reports. Healthy summary sampling uses bounded,
nonblocking stream bookkeeping; replacement/contention can emit extra summaries
and never disable safety or fault reporting.

Logging config is separate from GenericLimits, preserving its layout and
aggregate initializers. The existing capacity defaults/formulas are unchanged.
The parser accepts only0/1/2, checks duplicates/section placement and applies
the output level only after the complete configuration validates. Missing
Diagnostics defaults to1. Existing INIs remain compatible with R10. The new
documented files include Diagnostics and therefore require R10; R9 rejects that
unknown section. The installed user's280/320 INI was not overwritten by the
new documented150 baseline.

Logs use `%TEMP%/DSFV29s_usercap_r10_<PID>.log`. LOGGING_START declares the level
and absence of any disabled safety checks. LOGGING_STATUS at levels1/2 counts
attempted/accepted/suppressed records and file status. Accepted is output
selection, not a successful-write guarantee. There is no guaranteed terminal
exit snapshot; existing debug/file failure behavior remains in place.

The generic log analyzer now recognizes R10 metadata and explicitly checks
logging_full_trace. Levels0/1 cannot produce a full-trace integrity certification
merely because retained records have no errors. Their reduced evidence is an
intentional scope limit rather than a game failure. Older full R9 logs retain
the previous47 checks, including strict setupFail handling.

## Documentation and profiles

`CONFIGURATION.md` describes every supported key, units, defaults, derivation,
dependencies, format validation, representation constraints and evidence scope.
It distinguishes configured capacity, actual roster, visible traffic, audio
budgets and Replay reference storage. In particular, audio profile and retained
reference settings do not automatically grow with RequestCapacity; explicitly
setting a coupled key also disables its omitted-key automatic calculation.

`config/DriverSF_Unlimiter.ini` is the unchanged150/160 baseline with summary
logging. `config/profiles/Expanded280.ini` and `Headroom320.ini` retain the values
exercised in R9, with only the Diagnostics section added. A320 ceiling with280
actual models is labelled accordingly. README and scripts/tests READMEs now
identify current work separately from historical pinned verifier chains.

The user's earlier original-AI/150, original-AI/280 and150-AI/150 comparisons
are accepted as completed; this task does not request those repetitions.
Other chapter missions, frequencies1-4 and actual larger rosters retain their
existing unverified scope. No per-car MiB formula or universal upper roster
limit has been invented.

## Verification

`python scripts/test_logging_policy.py` passes20 isolated Win32 processes:

-243 parser/policy/backend assertions, including unchanged capacity fingerprints
  across all levels, transactional invalid config, essential error retention,
  summary timing/tick wrap/contention, first READY/rosters and actual file output.
-R9's1084 audio assertions, including20 actual bridge ABI invocations.
-Storage/native helpers at18,150,255,256,280,320,500,700 and1000 dimensions.
-Complete copied-image cold startup with the documented150/280/320 profiles,
  minimal/full output levels and the existing customized700 configuration.

The named supplied7044 log was passed through the production output selector:

| Selection | Lines | Bytes |
| --- | ---: | ---: |
| Full input/full selection |51333|16145690|
| Summary selection |924|218045|
| Minimal selection |56|15247|

Full selection hashes identically to the input. These are output-selection
results from existing records, not an R10 game session or a speed benchmark.
Actual R10 adds logging metadata and its configured logLevel field; its startup
output is not claimed byte-identical to the older R9 schema.

`python scripts/verify_logging_policy.py` passes fixture/dependency hashes,
PE32/x86/no-CRT imports, default level1, compiled filter/monitor call paths and
18 retained audio-hook signatures/ABI patterns. R9's packaged ASI retains its
original SHA256. Reduced/full/malformed logging metadata cases and the13
existing generic-parser negative cases pass. Supplied R9 run7044 still has
only its expected strict audio_setup check false. Synthetic parser-positive
copies are explicitly fixtures, not gameplay evidence.

The Release/Win32 solution build succeeds with the existing unused-function
and exported-entry warnings. MSBuild used the sandbox access required by its
file tracker in earlier builds. No target executable entry point, ASI loader
or debugger was invoked.

Evidence is `analysis/logging_r10_2026-11-18/`: fixtures.json,
fixture_manifest.json, build_verification.json, build/import logs and the
filtered-record outputs. A live smoke test of R10 at default summary level is
the remaining runtime check before treating it as the tested build. Stage3
packaging/publication was not authorized in this task and remains separate.

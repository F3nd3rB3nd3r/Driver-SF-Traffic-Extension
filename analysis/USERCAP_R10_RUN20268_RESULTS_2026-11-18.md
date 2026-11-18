# R10 run20268: corrected INI loads Story; summary output remains small

The user reports that the game worked after installing the complete Headroom320
INI. The supplied log confirms that the previous150-capacity refusal is gone:
the expected320-capacity configuration is read and the280-model Story request
reaches native READY. Summary logging works in a real game session.

Input: `config/DSFV29s_usercap_r10_20268.log`,134687 bytes, SHA256
`f85c67c859b6e206c7a3d3ed9d02b142d2ce9bccb2a9531d9a6192967b8d20bd`.
There are606 records and the last recorded tick is317406ms (~5m17.4s).

## Configuration and loading

Startup confirms valid/configured AssetWeights, LogLevel1,320 requests,
330 model slots,448 audio profiles,4096 global audio entries,128MiB shared
audio,4MiB configured Replay reader,768 retained references and180000 extra
hash entries. Derived work/key/handler capacity is1098. Explicit advanced
capacities match the installed full Headroom320 profile.

Two accepted phases have complete effective rosters and all-READY checkpoints:

| Phase | Models | Request tick | First all-READY tick |
| --- | ---: | ---: | ---: |
|1|18|22500|25219|
|2|280|25515|40328|

The native18 request is preserved; the Chapter7 setting's native18 is expanded
to280 from the active AI weights. There is no GENERIC_FATAL, exception record,
capacity fault or guard failure. Manager key peak280/1098, streaming handler
peak289/1098 and audio profile peak283/448 remain in bounds. All441 sampled
cumulative audio setup attempts succeed; setupFail0 and every new audio reason
counter0. No missing lookup or audio registry-full condition is recorded.

Sequential READY checks do not independently prove simultaneous drawable
residency. The user's observation supports successful play; detailed FPS/audio
category observations were not separately supplied for this short run.

## Live summary output

LOGGING_START declares level1, fullTrace0, summaryMs30000, checksDisabled0 and
outputOnly1. The final logging sample at300281ms reports33237 attempted
records,572 accepted and32665 suppressed, with fileStatus0. Those counts are
before that logging-status line and are not a final process-exit snapshot.
They reconcile exactly at the sampled point.

The actual file is only134687 bytes (~132KiB). At the same overall output rate,
30minutes would be roughly764000 bytes (~0.76MB/0.73MiB), not a guaranteed cap.
Startup overhead, workload and essential error output can change the rate.
This is now a live summary-size observation, unlike the earlier offline
selection test. No speed or CPU benchmark was performed.

The generic analyzer has49/50 checks true. The only false check is
logging_full_trace, intentionally false at LogLevel1. There are no recorded
fault-check failures. Its recorded_integrity_pass stays false because reduced
output cannot certify full diagnostic evidence; do not relabel it a50/50
full-trace pass. No switch to full tracing is needed merely to remove that
intentional scope restriction.

## Coverage boundary

The log shows Story use and Replay snapshot recording (48 captures/saves).
It records zero Replay reader owners/constructions, zero imports/restores,
zero client read checks and no snapshot loads. Therefore Replay playback/menu
lifecycle is not exercised in this R10 log. The configured4MiB value is read
correctly but no runtime file-reader backing is allocated in this session.
There is no independent normal-quit confirmation or final cleanup snapshot.
R9's earlier full activity/Replay results retain their R9 scope.

This verifies the configuration correction, R10 Story loading and live compact
summary logging. No patch, new ASI or capacity adjustment is indicated. The
remaining R10 smoke-test portion is a short Replay/menu lifecycle and identified
normal quit, not another150/280 capacity comparison or long full-trace run.

## Evidence and changes

`analysis/usercap_r10_run20268_2026-11-18/generic.json` is generated with:

`python scripts/analyze_generic_limits_log.py config/DSFV29s_usercap_r10_20268.log --output analysis/usercap_r10_run20268_2026-11-18/generic.json`

`summary.json` retains the input hash, size/duration, configuration, phase times,
final counters and limitations. Analysis/documentation only: no production
source, ASI, INI, game installation/assets/saves, build, game/debugger launch,
packaging, commit or push changes were made. The prior installed-file backup
and manifest remain preserved.

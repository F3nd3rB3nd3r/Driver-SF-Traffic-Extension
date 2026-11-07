# Run23636: silent termination; legacy eight-arena guards remain a candidate

Yes, the ASI still contains deliberate `ExitProcess` paths. The new callback guard writes `DSFV29s_CALLBACK_FATAL` before exit29C1, but this supplied log contains no such record. Older active spooler/scratch guards can exit after writing **only OutputDebugStringA**, leaving no reason in the capacity log. Therefore absence of CALLBACK_FATAL does not rule out a controlled ASI exit.

The earlier test implementation/report missed this logging gap. The supplied log cannot identify an OS exception, native game exit, or one of those legacy guard exits. Do not describe this run as a confirmed native crash or assume it reached callback capacity.

## Input and reproducibility

Only user-supplied `dist/MAIN_150/DSFV29s_callback512_23636.log` and current relevant guard source were reviewed. Existing two-log report used for its already established nine-event pattern; no old raw-log discovery, game/debugger execution, build, patch or installation.

-1348034 bytes,6149 lines,complete final newline.
-SHA256 `C4D34BA74925971C12ECA0CBB474944F82295876DAE5549B0D677A7DB3710D0C`.
-Last recorded time155781ms; previous monitor155328ms,453ms earlier.
-All new/inherited startup records indicate success.

```powershell
python scripts/analyze_callback_log.py dist/MAIN_150/DSFV29s_callback512_23636.log --output analysis/callback512_run23636_2026-11-06/summary.json
```

Analyzer structural checks pass; live stability fails per user report. Those are separate outcomes.

## Callback extension was not exercised before termination

All recorded callback error counters remain zero: invalid/full/guardBad/concurrent. No redirect event appears. Maximum recorded active and issued peaks are **one node per used group**; none approached64. Last recorded active/current counts and poll depths are zero.

| Group | Stores | Recycled inserts | Peak/issued peak | Native removals |
|---:|---:|---:|---:|---:|
|7|124|123|1/1|124|
|10|5|4|1/1|5|
|11|5|4|1/1|5|
|14|6|5|1/1|6|
|15|6|5|1/1|6|

Groups0/9/13/16 were polled but have zero stores/peaks. In particular the earlier dump's adjacent pool9 overflow scenario was **not observed here**. The old conditional replay remains an overflow mechanism, not a recovered runtime history. Nothing in this new run supports raising callback512 further. Telemetry does not cover the final453ms with another periodic snapshot, so an unrecorded later change cannot be excluded.

Cleanup last state:103052 appends=processed,14384 drain starts=ends,peak1176,current/depth0,no full/guard/invalid/mismatch. Indexed/head checks remain healthy and fileStatus0. Pending512 path is still unexercised. Audio registry150/192,353 profile hits and bankReady,zero miss/full/disabled;139 setup successes and214 native setup failures. Audio failures remain a separate undecoded issue.

## Ninth traffic-setting request is the terminal event again

Nine expansions, one stable owner1B10C370, all admitted with150 effective IDs:

`Chapter1 -> Chapter7 -> Challenge/f2 -> Chapter7 -> Challenge/f2 -> Chapter7 -> Exposition -> Chapter7 -> Challenge/f0`.

The last line is the ninth request:

```text
DSFV29s_CHAPTER_EVENT reason=expanded tick=155781 caller=0076FE9A owner=1B10C370 hash=D433D631 frequency=0 nativeCount=18 effectiveCount=150 profile=34
```

This is emitted before request publication/native processing. Prior CHAPTERS33044 and18228 also ended on their ninth admitted setting request. Different durations and settings yielded the same ordinal boundary. This makes a finite per-invocation sidecar registry a stronger next lead than elapsed-time pressure; the ordinal correlation alone does not prove its cause.

## Specific legacy guard candidates in current enabled source

`DSF_V29S_SPOOL0_EXPANDED=1` is enabled in CALLBACK512. In `src/V29sNativeLoadingTrace.inc`:

| Guard | Capacity | Fresh-reset behavior | Failure |
|---|---:|---|---|
|DiagSPrepareSpool0|8 external arenas,256 nodes each|If no recycled nodes and cursor is inline, the allocation branch consumes another arena. Existing-owner matching only accepts a cursor already inside its private arena; fresh inline reset does not select/reuse a prior arena by owner.|OutputDebugStringA then ExitProcess(0x2C).|
|DiagSPrepareScratch|8 external arenas,256 nodes each|At the native sentinel, consumes another arena even for a previously seen sentinel. Existing matching accepts only a cursor already inside its corresponding external arena.|OutputDebugStringA then ExitProcess(0x2D).|

Thus eight fresh/reset uses can consume all slots; a ninth qualifying use reaches a deliberate process exit. `legacy_guard_reset_replay.json` records this conditional source-policy replay. It assumes the native reset conditions on each invocation and does not capture actual runtime values. The current quiet log contains neither arena-use counters nor these failure reasons, so it cannot distinguish2C/2D from another termination path.

The enabled clone sidecar has32 arenas and explicitly reuses a matching header at its native boundary; it is not the same eight-slot policy. Other legacy ExitProcess paths also exist. None should be removed merely to make the game continue after an unsafe allocation.

## Next step

Prioritize the spool0/scratch sidecar lifetime/reset audit. Determine when old nodes are no longer referenced and implement correct reuse or process-lived generation handling based on that lifetime, rather than only increasing the eight-slot registry. Ensure all relevant legacy exits write their reason/exit code and header/cursor/count/arena-use state to this same log. Then the next controlled build can distinguish expected protection exits from native exceptions and test the repair.

A matching dump or captured process exit code would be useful if already available, but is not required to investigate the newly identified source-policy gap. No new ASI was built this turn. Keep current artifact for reference; do not repeat the identical uninstrumented workload or raise callback/audio limits based on this result.

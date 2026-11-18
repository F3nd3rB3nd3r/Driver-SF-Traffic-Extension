# R8 run3600: Story-to-Replay path passes44 checks

The user reports main menu -> Story -> Replay -> exit. They confirm everything
seemed fine, with no crash, and normal replay/audio/rendering behavior. They are
uncertain whether they returned to the menu or used Alt+F4, so this is not an
independently confirmed normal-quit test. Full replay duration and absence of
timeline manipulation were not separately specified.

Input: `dist/USERCAP_R8/DSFV29s_usercap_r8_3600.log`,12273048 bytes. SHA256
identity is saved in `analysis/usercap_r8_run3600_2026-11-17/summary.json`.
Last recorded tick333860ms (~5m33.9s). All44 generic-capacity checks pass;
`recorded_integrity_pass=true`. No fatal or exception record appears.

## Traffic and manager list lifecycle

All four accepted traffic phases reach native READY:

| Phase | Requested | Request tick | First all-READY tick |
| --- | ---: | ---: | ---: |
| 1 | 18 | 23625 | 26235 |
| 2 | 280 | 26625 | 40297 |
| 3 | 18 | 187235 | 190047 |
| 4 | 280 | 194203 | 203110 |

Replay import194203ms and restore194250ms retain280 IDs, one retained reference
and roster hash699365A7. This exercises the replay roster path after Story.
Sequential READY checks still do not independently certify drawable residency.

R8 key storage peaks280/802 per list and288 sampled aggregate entries; two
external buffers and two redirections are recorded. Four manager generations,
three destructions and one remaining generation appear at the last sample;
active keys drain to0. Key failures0, guardBad0. Dispatch peak289/802, ending0;
all four scheduler queues end0, with no integrity errors. The widened mission
capacity query is exercised nine times, with two native fallbacks.

## Replay reader and busy boundaries

The configured2MiB reader is actually exercised: one retained owner, five
constructions, five destructions, four reuses,1144 native read issues and no
rejected reads. Allocation failures, fallbacks, faults and unmanaged calls are0.
Retained backing2097152 bytes is expected process-lifetime reusable storage.
All145 owner samples and six managed boundary records pass configuration-aware
buffer/state/schema checks. No original fixed1MiB/150/160 assumptions are used
to reject this2MiB/280/290 configuration.

Three narrow client busy intervals occur during replay loading (loading1):
one approximately1.31s contextEnd and two clear intervals of73.9ms/0.17ms.
The contextEnd is not itself proof of readiness recovery. Final open/held counts
are0; no dropped records, schema/pair errors, read failures or clock failures.
No busy episode outside loading appears in these records. Normal prefetch
space/backend/end stops are not evidence of visible hitching by themselves.
The user reports no observed issue; telemetry does not independently measure
render FPS or certify every playback frame.

## Audio and conclusion

All351 setup attempts succeed, setupFail0. Audio registry peak283/384, full0,
missing lookups0 and guardBad0. The recovered audio failures from the longer
run18736 do not recur in3600; their underlying cause remains a separate open
observation and is not proved resolved by this shorter session.

The run adds successful R8 Story -> Replay coverage to18736's Getaway/Story
result. No new patch or limit increase is indicated. The broader reverse
Replay -> Story/menu reload matrix and a clearly identified normal quit retain
their separate validation scope. Do not describe this one-way path as the full
menu/Replay/Story loop or as a confirmed normal shutdown.

Evidence: `generic.json`, `summary.json`, reproducible `summarize.py` and its
analysis-only preparation helper in `analysis/usercap_r8_run3600_2026-11-17/`.
The retained client interval decoder's historical fixed150 startup result is
preserved separately; the review validates startup against recorded280/290 and
reader backing against2MiB. Analysis/handoff updates only; no source/ASI/INI,
build, game/debugger/deployment/asset/save change, Git commit or push occurred.

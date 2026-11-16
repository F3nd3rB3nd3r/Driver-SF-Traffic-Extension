# R8 run18736: Getaway1 exit succeeds during52-minute Story session

The user confirms the previously freezing Getaway Activity1 exit was included
and nothing went wrong. They report no FPS, audio or rendering issues during a
lengthy Story-only session. No Story/menu/Replay validation sequence was done.
The reply does not separately describe the exit method; normal shutdown is
therefore not independently established here.

Input: `dist/USERCAP_R8/DSFV29s_usercap_r8_18736.log`,89738056 bytes, SHA256
`a7b43667e835e03a839e730a790d0a7814364c91e3c412fa077251a53f576c1e`.
Last recorded tick3134422ms (~52m14.4s). Analysis/handoff changes only; no
source/ASI/INI/build/deployment/game/debugger/asset/save/Git publication changes.

## Targeted fix exercised successfully

Startup applies the expected R8 configuration:280 requests,290 model slots,
384 audio models,4096 global audio entries,128MiB shared audio,2MiB reader,
512 retained references,180000 extra hash entries and802 manager-key entries
per list. All22 accepted traffic phases reach a native all-READY checkpoint.
They include repeated280 ->12/18 ->280 changes, with eleven280-car phases.
The final280 request at3113985ms reaches all READY at3122360ms.

MANAGER_KEY_START/STATUS validate correctly. Native inline256 capacity was
exceeded: per-list projected peak280, one external buffer and one fresh-cursor
redirection. The same extension served the session without further allocation
growth. Peak aggregate sampled active keys across lists is288. Final counters:
2910 inserts,11209 lookups,194853 native fallbacks, three owner generations,
two destructions and one remaining live generation. Failures0, guardBad0.
The final sampled active count is0. These observations directly exercise the
new backing while retaining unrelated native callers.

The original R7 dump's548 header was corrupt, not a trustworthy measurement of
548 valid keys. R8's280 projected per-list peak is the meaningful observation
for this workload. Do not turn either value into a universal per-car formula.

Combined with the user-confirmed Getaway exit and absence of the native loop,
this supports the R8 correction for the supplied280-car Story/activity workload.
It does not establish all chapters/activities, larger rosters or indefinite life.

## Other capacities and integrity

No fatal or exception record appears. All capacity, startup, pool/queue/guard,
callback, pending, cleanup, registration-map and replay-snapshot checks pass.
Dispatch projected/sample peak290/802; scheduler peaks263/3/288/37 are in
bounds. Audio registry peak283/384, full0, missing lookups0, guardBad0. Hash
extra lifetime issuance peaks55377/180000. Global audio ceiling/per-record full
counters stay zero. No capacity increase or further ASI is indicated by these
observations. FPS/audio/rendering quality is user confirmed, not inferred from
counts or sequential READY observations.

Sampled process private memory peaks1415155712 bytes (~1.32GiB); working set
1084813312 (~1.01GiB). These are aggregate process metrics, not asset sizes or
proof of sufficient address space under larger workloads.

## Audio caveat: analyzer is not a wholly clean pass

The strict generic analyzer returns `recorded_integrity_pass=false` solely for
`audio_setup=false`. It records6128 attempts:5678 successful setups and450
failed setups. All other43 of44 checks pass, including both new manager-key
checks. Do not relax this condition or report all44 checks passing.

The450 failures involve43 model IDs. Every affected model later has a successful
setup after its most recent observed failure. Most failure increments appear
roughly0.8-3s after traffic switches; additional clusters around1067/1068s and
1265/1266s occur during an ongoing280-car phase, approximately39s and237s after
its preceding traffic request. Therefore not every failure can be labeled a
traffic-transition failure. Detail sampling/counters do not establish why the
native setup returned false. There are no registry-full or missing-profile-key
indications, and the user reports no audible issue.

These recoveries and the user report argue against persistent vehicle silence
from registry exhaustion in this run, while leaving the underlying transient
setup behavior unclassified. No audio patch or additional limit increase was
made or proposed solely from these counters.

## Remaining validation

Replay reader constructs0/owners0 and writerBypass1: R8 playback was not
exercised. R7's successful replay run retains its original R7 scope; the new
R8 manager-list lifecycle should still receive a Story/menu/Replay sequence
when convenient. Final active key/dispatch/pending counts drain to0, consistent
with cleanup, but the log has no definitive final-exit marker.

Keep R8 and the same INI/AI as the successful Getaway candidate. The useful
remaining targeted check is R8 menu/Replay/Story/Replay/menu with normal exit;
there is no need for a new build based on run18736.

Evidence: `analysis/usercap_r8_run18736_2026-11-16/generic.json`, streaming
`summary.json`, and reproducible `summarize.py`. Full raw log stays local.

# R6: fix the R5 fourth scheduler queue regression

Dump `Driver_20261115_211728.dmp` is PID19620, with R5 loaded. It crashes at
005A55D7 reading[EAX-4], with EAX8F1D4308, ECX/ESI1C9EC704, EDI1C9D69A8 and
caller005A4D7D. This is an implementation error in R5's scheduler relocation.
It is not evidence that the user's280-car AI or audio budget needs changing.

The exact matching log was found at `%TEMP%/DSFV29s_usercap_r5_19620.log` and
preserved under `analysis/usercap_r5_crash211728_2026-11-15/`, alongside the
dump analysis, input manifest, log analysis and focused disassembly. The last
sample is107344ms, with GENERIC calls0 and no BASELINE request phases. Thus
the crash precedes any logged traffic roster request. Handler peak5/802,
two constructed manager/scheduler generations and one destruction each are
observed without existing guard/failure flags. Those flags missed the bad
fourth queue, which R5 neither relocated fully nor monitored.

## Specific missed dependency

The original layout is:

| Region | Original base | Geometry |
|---|---:|---|
| Three pointer vectors |0,404,808|256 pointers plus count each|
| Fourteen membership lists |C0C|1818 bytes each,512 twelve-byte nodes plus header/sentinel|
| Fourth pointer vector |15D5C|512 pointers, count at1655C|
| Runtime tail after that count |16560|Through original object size16620|

R5 incorrectly treated1655C as immediately following the fourteen lists.
The lists actually finish at15D5C; the2048-byte fourth vector lies between
that position and1655C. R5 moved the count/tail while leaving four uses of
array base15D5C and the helper's three800-byte count offsets unchanged.
The crash's ECX exactly equals EDI+15D5C, proving the legacy array base was
still used. Caller005A4D46 reads the relocated count, calculates a position
from that legacy base and calls005A55A0, which reads an unrelated word at
legacy base+800 as a count. Its insertion shift then dereferences the invalid
computed end pointer at005A55D7.

The old tests exercised only the first three vectors and fourteen membership
lists. Their passing result did not cover this fourth-vector dependency.
Earlier R5 readiness and scheduler-size conclusions are superseded.

## Correction

R6 preserves the H=`max(512,ModelSlots+ReplayRetainedReferences)` sizing rule;
H remains802 with the user's INI. It adds the fourth vector to the geometry,
guards and telemetry:

- Fourth base =3*(4H+4)+14*(12H+24).
- Fourth count =fourth base+4H; runtime fields after it retain their relative
  offsets. Scheduler logical bytes =184H+544, plus16 guard bytes.
- For H802, the object grows from R5's incorrect144904 bytes to148112 bytes.
  This corrects the missing region, rather than increasing H again.
- Four exact-version base operands and three helper count operands are added
  to the audited table, making362 total. The actual512-pointer insertion
  helper005A55A0 receives a matching eighth insertion guard hook.
- All four queues appear in revision2 stream telemetry, including queue3
  occupancy/projected insertion peak. Analyzer supports both historical R5
  revision1 and R6 revision2, and requires revision2 for R6-named logs.
- Complete-instruction signature preflight, stable handler slabs, existing
  manager/scheduler lifetime handling and inherited R4 changes are retained.

Evidence and pre-edit backup are in `analysis/stream_handlers_r6_2026-11-15/`.
The old R5 ASI/distribution remain byte-for-byte preserved; R5 is a failed
candidate and should not be retried.

## Regression evidence

The native fixture now executes the actual005A4D46 caller, its base/count
calculation, native binary insertion-position search and005A55A0 insertion.
Only the middleware resource comparator is replaced with deterministic raw
pointer ordering, and the unrelated continuation after the native insertion
is bounded by a fixture return. Descending values exercise the middle-insert
shift that faults in the dump, through configured capacity and beyond512.

The obsolete counter location is poisoned with0x1C9F9F01, the low30 count bits
implied by the dump's EAX/ECX arithmetic. The corrected path must leave it
unchanged. A separate disposable process restores the missed R5 routing and
produces the expected native C0000005 access violation. This is a controlled
reproduction of the routing defect, not a claim to reconstruct every prior
heap write in the user's small dump. The new fourth-vector-full test refuses
before an out-of-bounds insertion.

11 focused processes and25 inherited processes pass their expected outcomes.
The280 configuration performs8991 checks in the focused positive fixture.
Full cold installation, compiled bridge/layout checks, existing fallback
hashes and analyzer checks pass. Release/Win32 build succeeds. See
`build_verification.json` and the fixture manifests in each R6 evidence folder.

Candidate:304128 bytes, SHA256
`851f22dddc3ff314b148ac7bde61a1046f670da085b5de94689b45a465f4aec5`.
No game, debugger or live process was launched. Installed ASIs, game AI/INI
and saves were not changed; no Git commit/push occurred.

## User retry

Use `dist/USERCAP_R6/DriverSF_Unlimiter_USERCAP_R6.asi` as the only traffic
ASI, keeping the same280-car AI/effective INI. First reach the main menu and
load Story, then repeat driving/activity/menu/Replay transitions if successful.
Supply `DSFV29s_usercap_r6_<PID>.log` and the observed outcome. A live result
is still needed; the offline result does not establish280 resident/drawable
vehicles or complete coverage of every game/mod limit.

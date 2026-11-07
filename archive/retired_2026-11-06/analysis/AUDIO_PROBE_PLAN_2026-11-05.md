# Proposed narrow audio measurement on R2

This was the implementation specification before the user requested the build. R1 is now built and staged, not installed or game-tested: see `AUDIO_PROBE_R1_BUILD_2026-11-05.md` and `../dist/MAIN_150/AUDIO_PROBE_R1_TEST_2026-11-05.md`.
Static findings: `analysis/AUDIO_STATIC_AUDIT_2026-11-05.md`.

## Preserve the visual baseline

Use the verified R2 startup/handler checks and 13-operand external indexed table:
262144 indexed slots, ascending 150 requests, 160 vehicle graphics slots, ED/EF,
pending-array/hash expansions, and the existing quiet capacity monitor.
Keep the 50-node bank queue, 90 emitters, 20 pool descriptors and all audio byte
budgets unchanged. Do not turn on the legacy heavy graphics/handle/hash traces.
Exactly one traffic ASI would be installed for a user-driven test.

## Measurements that answer the current uncertainties

1. **Actual bank completion.** At `0084BE74`, before the native state-4 write,
   collect original stack `+04` bank ID, `+08` AKRESULT, `+0C` pool ID,
   `+10` job cookie, and the job's input size/pointer. With `pushfd; pushad`,
   AKRESULT is saved-frame `+44`, pool ID `+48`; the cookie is already in EAX
   at this site. Keep ID/result/pool/cookie separate. Count numeric result 1
   versus other values and retain a bounded first-failure detail list. Do not
   import unrelated SDK enum values. Preserve the native callback behavior.
2. **Emitter admission.** At `0084AA29`, owner ESI is the sound manager.
   Count calls and cases with `[ESI+EA0]>=90` before the original compare.
   Track the active peak; also sample active/recycled/cursor metadata at
   `A+E98/E9C/EA0`. Do not create an object or change the native rejection.
3. **Real allocation failures.** At `00C99AA5` (Malloc) and `00C99B04`
   (Malign), EDI is the returned pointer. For a nonzero requested size, count
   EDI==0 failures by validated pool ID, bytes requested and caller. At these
   epilogues current ESP is entry ESP minus 12: pool ID is current ESP+16,
   requested bytes +20, Malign alignment +24, original caller +12. After a
   further pushfd/pushad frame the corresponding offsets increase by 36.
   A successful allocation should take only a tiny flags-preserving filter;
   avoid full register/FX saves on every successful allocation.
4. **Shared numeric event results.** `00C54E17` is the normal PostEvent return
   with ESI=playing ID. Include the external-source allocation-failure return
   `00C54DD3` so that an early zero is not missed. Log returned IDs as admission
   results, not audible voices. At the normal epilogue, original caller/event ID/
   game object are current ESP+28/+32/+36. At the early epilogue there is no
   pushed ESI, so these are +24/+28/+32. Derive and verify both layouts in the
   compiled bridges. Aggregate normal/zero returns and retain bounded event ID,
   game-object ID and caller samples on failure. Numeric API entry `00C54DA0`
   is available if entry accounting is later necessary, but is not required
   merely to count completed return paths.
5. **Low-rate state snapshots.** Read pool-table pointer/max/active count at
   `01D72390/01D72388/01D7238C`. After initialization, validate IDs and record
   active marker, attributes, used bytes (`+44`) and usable total (`+48`),
   stride `4C`. Track stable named IDs from `A+A2C/A30/A34`, default ID
   `00FDD3B8`, platform ID `00FDD3C0`. Account for descriptor reuse; do not
   merge unrelated generations under one pool ID. For attributes with bit `08`
   set, also sample block count/size at `+00/+04` and free-block count at `+18`:
   fixed-block GetBlock does not update the Malloc/Malign `+44` counter.
   Capture manager byte `A+30`,
   sound/music init flags `A+9548/+9549`, and raw volume/fade state at
   `A+50..A+90`. Those fields are diagnostic snapshots, not a proven mixer mute
   cause. Existing R2 markers supply request/loading context.

The bank queue is already measured. The 90-node deferred-cleanup pool at
`A+A4C/A50/A54` can be sampled alongside emitter metadata with negligible extra
work; its semantics must remain distinct from emitter occupancy and voices.

## Candidate spans already verified offline

| Address | Whole-instruction signature | Purpose |
|---|---|---|
| `0084BE74` | `C7 40 14 04 00 00 00` | Bank callback before native completion write |
| `0084AA29` | `83 BE A0 0E 00 00 5A` | Emitter admission compare |
| `00C99AA5` | `5E 5B 8B C7 5F` | Malloc return |
| `00C99B04` | `5E 5B 8B C7 5F` | Malign return |
| `00C54E17` | `8B C6 5E 5F 5B 83 C4 10` | Normal PostEvent return |
| `00C54DD3` | `5F 5B 83 C4 10` | Early PostEvent failure return |

The verified inventory also contains the optional five-byte PostEvent entry.
No candidate near CALL/JMP/Jcc entered a stolen instruction interior in the
byte scan. This does not validate indirect/short branches, new bridges,
trampolines or integration automatically. They still require focused checks.

## Instrumentation constraints

- Preserve native integer registers, flags, x87/SSE state, return values,
  epilogue stack adjustments and calling conventions. Use the existing audited
  FXSAVE/FXRSTOR pattern only when a C++ callback is actually required.
- Use fixed-size counters/rings; no heap allocation, VirtualQuery, locks or
  file I/O on ordinary hot success paths. Do not recurse into middleware memory
  allocation or call middleware APIs from allocation hooks.
- Emit summaries on the monitor, detailed failures bounded per category/pool.
  Keep counters after detail storage fills so repeated failures remain visible.
- Prefer validated read-only snapshots over invoking pool APIs on invalid IDs
  or walking allocator lists. Record inconsistent/unavailable snapshots as such.
- Give the new test a distinct artifact/log revision, validate baseline install
  and diagnostic signatures, and fail without partial diagnostic detours.
- Do not use `SetLocalOutput`/`PostCode`/`PostString` as an error source: they
  share the return-4 stub in this executable. `GetPoolStats` detailed zero fields
  are likewise not useful usage measurements; `+44/+48` are the relevant counters.

## User test and decisions

Start from a fresh process. At the initial menu, exercise menu clicks for a few
seconds; load the same save; after the loss, open the pause menu, try the horn,
and drive briefly. Note which categories remain audible and when they vanish.
Return the complete startup-to-world log. Do not time this with debugger pauses.

| Measured outcome | Next conclusion/action |
|---|---|
| Non-1 actual bank callback results | Follow that exact asynchronous load failure; accepted queue totals are insufficient |
| Pool count at maximum plus confirmed CreatePool failure | Audit descriptor headroom/lifetime; do not equate pools with banks |
| Nonzero-size Malloc/Malign failure in a named pool | Audit its byte budget/fragmentation/request size before expansion |
| Emitter count reaches 90 and admission refuses | Audit emitter ownership and all relocation/lifetime paths before changing its capacity |
| PostEvent returns zero with otherwise healthy allocation | Follow event-definition availability and registration/object state for that caller/ID |
| Bank/admission measurements remain healthy but sound is absent | Continue into actual voice, command-consumer and mixer/stream state; nonzero playing IDs do not prove audio output |

The first probe need not hook every voice or decoder. These measurements
separate concrete failure classes without sacrificing the now-working rendering
configuration or increasing unmeasured capacities.

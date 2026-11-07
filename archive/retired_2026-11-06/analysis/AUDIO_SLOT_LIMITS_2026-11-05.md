# Audio slot ceilings independent of memory budgets

The user reports that the shared-default64MiB test did not restore the remaining
audio and asks whether fixed slots can run out despite free RAM. **Yes.** The
existing inventory mentioned two255-entry middleware settings; this focused
follow-up now verifies their full-handling code. Their live occupancy was not
measured by any current audio probe, so zero Malloc/Malign failures do not rule
them out.

No `DSFV29s_audio_default64m_*.log` was present in `dist/MAIN_150` when checked
this turn. The latest64MiB result is therefore a user observation; its live
startup/size/counters have not yet been verified from a supplied log. No new
ASI was changed/built/installed or game/debugger/web search launched. Only the
authorized exact Driver.exe and focused existing evidence were read.

## Known limits and coverage

| Capacity | What it limits | Coverage |
|---|---|---|
| 90 | Native registered-emitter objects | Measured; run6736 had1113 attempts, active-before peak30 and no refusal |
| 90 | Separate deferred cleanup records | Sampled active peak4 in6736 |
| 50 | Concurrent bank-operation jobs | Measured peak2/50; not a50-resident-bank ceiling |
| 20 | Middleware pool descriptors | Measured7/20; not a20-bank ceiling |
| 255 | Middleware manager from InitSettings+4 | **Newly verified count-based null return; occupancy unmeasured** |
| 255 | Middleware manager from InitSettings+8 | **Newly verified ceiling/reuse path; occupancy unmeasured** |
| 193 buckets | Event lookup hash index | Collision-linked buckets, not a193-event capacity |

Loaded banks, bank-operation jobs, registered game objects, playback records,
manager slots and physical voices are different quantities. A car can reference
multiple audio records, while a loaded bank need not occupy one active voice.
The two255 managers must not yet be described as255 vehicle banks or physical
voices; their stripped class roles and relation to the silent cars remain to
be resolved. More general audio slot/voice/decoder/bank-authored limits are not
exhaustively mapped.

## Setting source and allocation

Exact Driver.exe SHA256:
`C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.

`GetDefaultInitSettings` at00C50F00 loads ECX=255 at00C50F0C, then stores it
to settings+4 and+8 at00C50F17/00C50F1A. The game calls these defaults at
0084A620; its audited overrides affect byte budgets and pressure thresholds,
not these two count values. None of the current ASIs modifies this255 constant.

Common initialization maps the values into separate managers:

- Saved settings+4 at01D71868 is passed by00C54BEE..00C54BF7 into00C6B740;
  manager pointer is retained at01D71850. Its configured maximum is manager+10,
  with a pointer array begin+4/end+8 and backing capacity+0C. Initialization
  allocates `count *4` bytes and stores the count in full-width fields.
- Saved settings+8 at01D7186C is passed by00C54B97..00C54BA0 into00C6B160;
  manager pointer is retained at01D7184C. Its ceiling is manager+4. It allocates
  two pointer arrays, begin/end/capacity at+8/+C/+10 and+14/+18/+1C, each
  initialized from the configured count. The ceiling applies to the selected
  list, not automatically a255-total sum across both lists.

This separates logical count ceilings from the default pool that backs their
allocations. Increasing the latter alone leaves the former255 values unchanged.

### Does255 imply an8-bit ceiling?

255 equals2^8-1, but the inspected manager paths do **not** store their global
limits in a byte. Initializers00C6B171/00C6B751 store DWORD counts; pointer-array
allocations compute `count *4` using32-bit registers. Both ceiling comparisons
read DWORD fields. Thus2048 is representable in these paths;255 is a configured
default rather than a demonstrated8-bit storage maximum here. This is not proof
that all downstream IDs, masks, indices and cleanup consumers support2048.

At2048 the three initial pointer arrays would total24576B instead of3060B,
an extra21516B before record/subobject allocations. Pointer storage itself is
small; the unknown is consumer compatibility and whether either ceiling is
actually reached. Recommendation remains a count/refusal/reuse probe at255
first, followed by an audited expansion if it saturates. Do not infer safety
from the numeric value alone or patch only a comparison.

## Verified hard refusal before allocation

At00C6B920, ECX is the manager retained in01D71850. Native code computes:

```text
00C6B923..00C6B92B  count = (manager.end - manager.begin) >> 2
00C6B929            EAX = 0
00C6B92E            compare count with manager+10 (configured maximum)
00C6B931            JAE 00C6B9A1
00C6B933..00C6B93C  otherwise request a6C-byte record through Malloc
00C6B9A1..00C6B9A2  restore EDI and return, leaving EAX=0 on the full branch
```

Thus this manager can return null at255 entries without even requesting more
memory. The failure-only allocator hooks cannot see that case. Its later
pointer insertion also checks allocated backing capacity separately, and frees
the new object if insertion cannot succeed. One focused near-call candidate
to the allocation function was found at00C76742; complete caller classification
and indirect-call coverage remain outstanding.

## Verified ceiling with existing-record reuse

At00C6B420, ECX is the other manager and the third argument selects one of its
two pointer lists. EDI becomes manager+14 when that argument equals1, otherwise
manager+8. It computes selected-list count and compares it with manager+4:

```text
00C6B42F..00C6B436  count = (list.end - list.begin) >> 2
00C6B439            compare count with manager+4 (configured ceiling)
00C6B43C            JAE 00C6B464
00C6B43E..00C6B447  otherwise allocate a3C-byte record
00C6B464..00C6B4B8  fallback: select an existing record using its+20 float,
                    call update/cleanup helpers, remove it and reuse it
```

The fallback also handles allocation/construction failure. It can return null
when it cannot choose an existing candidate. It must be counted as a ceiling/
reuse event separately from outright denial. This path can operate while the
RAM allocation counters remain zero. Its downstream effect on an audible sound
has not been established.

The3C-byte constructor00C8C3A0 and update00C8C3D0 show time/progress-like
fields, including normalized elapsed/duration calculation. This supports a
transition-related interpretation, but no external SDK/class name was assumed.
Near-call candidates into00C6B420 include playback helpers00C74DC6/00C7546A/
00C75720/00C75D36 and music-side00CA16xx..00CA19xx. Their exact relationships
to menu, engine, tire and horn symptoms remain unresolved.

Another count check at00C6B9C6 limits a per-record pointer list to8. It is not
established as a global bank/voice ceiling and should not be expanded on the
basis of this isolated compare. No constant in these routines is automatically
a safe limit-adjuster target without following its users and storage.

## Next measurement and adjustment decision

The user's hypothesis is technically viable. Runtime slot saturation still
needs to be distinguished from unavailable event definitions/bank associations,
which remain candidates from the earlier109 PostEvent zeros. Those zeros are
not automatically the result of these later count gates.

Next bounded work is to finish manager/caller classification and measure:

1. Both configured maxima and pointer-array capacities after initialization.
2. Live and high-water selected-list counts, including both lists of01D7184C.
3. Exact pre-allocation full returns for00C6B920, distinguished from Malloc nulls.
4. Ceiling/reuse entry at00C6B420, distinguished from subsequent null/reuse results.
5. Whether those events coincide with a known working versus silent car.

Candidates for later bridge design are00C6B929 (`XOR EAX,EAX; SAR ECX,2`, five
whole bytes, before the first compare) and00C6B42F (load end/subtract begin,
five bytes). These have not received new hook-interior/control-transfer/ABI
verification and are **not implementation-ready merely because they are named**.
No new slot ASI was built in this turn.

If these managers saturate, increasing their configuration must increase the
paired backing arrays and preserve their full/reuse/removal paths. A plain
memory-budget increase or an isolated compare patch is not an adequate fix.

Evidence: `analysis/audio_slot_audit_2026-11-05/`, including
`verified_slot_inventory.json`, focused native disassemblies and near-call
candidate lists. Seven exact instruction spans, the default255 source and both
full/reuse branches were checked against the exact executable.

Reproducer examples (read-only target, workspace evidence only):

```powershell
python scripts/audio_static_audit.py --output analysis/audio_slot_audit_2026-11-05 disasm C6B420:C6B580 C6B920:C6BA60
python scripts/audio_static_audit.py --output analysis/audio_slot_audit_2026-11-05 calls C6B920 C6B420
```

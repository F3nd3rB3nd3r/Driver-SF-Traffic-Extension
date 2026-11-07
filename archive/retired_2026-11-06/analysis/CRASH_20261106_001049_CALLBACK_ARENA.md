# Challenge traffic-switch crash: native callback arena corruption

## Finding

The supplied dump records a native call through address0 on the previous PENDING512_R1 build. It also exposes an unexpanded64-node callback list whose overflow can reproduce the exact invalid iterator geometry. This is a strong additional capacity-corruption candidate, separate from the completed/pending maps, audio-model registry and deferred cleanup queue.

The user reports starting a Challenge that switches traffic. CHAPTERS_R1 may reduce the release work associated with shrinking a150-model roster back to a small native list, but it does not expand or repair this callback arena. A successful challenge test would not by itself establish this arena's capacity safety under every transition.

## Input and build identity

- `dist/MAIN_150/Driver_20261106_001049.dmp`,481547 bytes.
- SHA256 `962B5DB8CF1C83285C2EEB7C90BDE93A791DED7BAB5BC02AFB8C3CF93FFEAFA1`.
- Dump time2026-11-06 00:10:49 UTC /01:10:49 Budapest; PID21772, exception TID11520, x86 context.
- Modules include DriverNGHook.asi and `DriverSF_V29s_160Capacity_150Request_PENDING512_R1.asi` at68B10000, SizeOfImage415000, timestamp1793907857. Size/timestamp match the staged known PENDING512_R1 artifact. Whole loaded image bytes are not present, so this is metadata correspondence rather than a full loaded-image hash.
- No CHAPTERS module is listed; this dump predates the new chapter-extension build.
- The exact matching log `dist/MAIN_150/DSFV29s_pending512_21772.log` was not present. No other old logs or folders were scanned.

## Immediate failure: established from registers, memory and instructions

Exception C0000005, EIP0, EDX0. The recorded exception parameters are0/0; no separate execute-access subtype is inferred.

At native00859031..00859039:

```text
mov ecx,[edi+8]
mov eax,[ecx]
mov edx,[eax+0Ch]
call edx
```

The dump contains:

| Field | Value |
|---|---|
| Current callback iterator EDI | `014449B8` |
| Callback receiver ECX | `2DA4D190` |
| Receiver first word / EAX | `2F326798` |
| Indirect method slot `[2F326798+0C]` | `00000000` |
| Top stack return | `0085903B` |

Thus CALL EDX at00859039 explains EIP0 and the return on the exception stack. This is not a zero pointer in ASI code.

The handler being polled is ESI30FAB158, with captured vtable00E37118. The supported executable's vtable+24 is00859000, matching the next decoded stack return005A2131 from its virtual call at005A212F. The next native return0059CF21 matches the call to005A20C0 at0059CF1C. Higher stack candidates include the005A8030/005A8300/005A8460 vehicle resource-release/update path. The source005A8062 calls0059DB20 to release a16-bit resource key.

The stack contains key04E0 (1248). This is numerically248+1000, consistent with the documented variant-ID convention; its exact asset/variant role is not established here. Do not treat1248 as a newly added base vehicle or evidence that parked vehicles caused this crash.

EBP1B78D194 is a completed-map header, not a frame pointer. The generic parser's EBP-chain entry with return44 is not a valid caller. Only the bounded stack/instruction evidence above is used.

## Newly identified callback storage

Native constructor00DB40A0 initializes21 global list headers at:

`014427EC + index * 0314`

For each header:

| Offset | Native meaning |
|---:|---|
| +00 | Fresh/recycled node cursor/head |
| +04 | Recycled count |
| +08 | Active count |
| +0C | First inline node |
| +30C | Sentinel (two links through+310) |

`(30C-0C)/0C = 64` inline12-byte nodes. Constructor uses20 as an initial loop counter and loops through zero, initializing21 groups.

Insertion005567D0 writes the pointer payload at node+8, advances a fresh cursor by12, increments active count and links the node to the sentinel. It has **no inline-end comparison**. Four direct-call candidates exist at0055690F,005A399A,00858FC8,008CCF4C. The confirmed local registration path at00858FC8 passes the global header selected by handler+560. Other callers/owners still require classification before changing the shared insertion routine.

Polling00859000 uses the same0314 stride and fixed sentinel. It unlinks completed callbacks and recycles their nodes normally. A future extension must preserve those operations and the native sentinel, rather than breaking the poll loop or skipping invalid callbacks.

This dump's handler+560 is10. Therefore:

- Pool10 header014446B4.
- First valid inline node014446C0.
- Sentinel014449C0.
- Captured iterator014449B8 is sentinel-8 and is misaligned by4 relative to the12-byte node sequence.
- Reading iterator+8 therefore reads the sentinel's previous-link field **as a callback receiver**.

This is structural list corruption, not merely a callback returning failure. The bad receiver's+8 contains2F325B18, also the captured handler+55C value; the dump establishes these bytes, not the first write or complete lifetime of that receiver.

## Conditional native-operation replay

An independent Python replay follows the decoded constructor and insertion operations in fixture-owned dictionary memory. It initializes all21 groups, then performs150 fresh inserts into adjacent pool9. No live process, target memory or game code is executed.

| Pool9 insertion | Result |
|---:|---|
|64 | Last valid inline node. |
|65 | Node aliases pool9 sentinel; payload overwrites pool10's cursor/header at014446B4. |
|130 | Node starts014449B8; payload overwrites pool10 sentinel.previous at014449C0. |
|131 | Node starts014449C4; its previous-link write overwrites pool10 sentinel.next with014449B8. |
|150 | Pool10 still begins traversal at014449B8, exactly the dump's EDI. |

For the comparison, replay insertion130's payload is explicitly set to the captured receiver2DA4D190. The replay then reads that value through the same malformed iterator+8. The **iterator address and overlap mechanism follow from native geometry**; the chosen payload is a conditional comparison, not independently recovered insertion history.

The global callback arena itself is omitted from this small dump. Actual pool9/10 active/free/cursor values, whether recycling occurred, and the first overflowing write were not captured. Therefore the replay strongly implicates adjacent callback-arena overflow but does not prove that precisely150 consecutive fresh inserts occurred in this process. Other writers could also corrupt the list.

## Relation to current work and next step

- The chapter patch is absent from this dump and cannot have caused this recorded failure.
- Its common150 roster may avoid some challenge-switch unloading, but callback storage remains64 inline nodes in CHAPTERS_R1 as well. Do not declare this crash fixed solely from the changed preload policy.
- The current source has no expansion for this005567D0/global arena path. Its64 is separate from pending512 and the already extended completed map.
- Next useful static work is a bounded callback-arena writer/removal/reset/constructor audit: classify all shared-helper callers, every global-header user, recycling semantics and overlap with current patches. Then a scoped storage extension plus occupancy/guard logging can be designed from that evidence.
- A new broad probe or full process dump is not a prerequisite for that static audit. If the current chapter test reproduces the crash, preserve its new log and dump so both builds can be compared.
- Parked-car visual fallback is not tied to this failure by the dump.

Report/evidence: `analysis/crash_20261106_001049/dump_summary.json`, `callback_proof.json`, selected memory and bounded native disassemblies. Reproducer:

`python scripts/analyze_crash_001049.py dist/MAIN_150/Driver_20261106_001049.dmp --output analysis/crash_20261106_001049`

Only the supplied dump, exact matching-log existence, known staged baseline/map, current relevant source and supported Driver.exe were read. No live attachment, debugger launch, game execution, ASI/game modification, build, dependency installation or web search was performed for this dump analysis.

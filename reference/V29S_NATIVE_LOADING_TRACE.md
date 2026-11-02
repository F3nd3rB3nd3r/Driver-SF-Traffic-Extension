# V29s — native streaming gate trace (2 November 2026)

Target exact retail PC executable SHA256: `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.

## Validated from V29r test comparison

- V29r 45-request/160-slot control **visibly reached gameplay** (user confirmed city and cars visible). Map reached52; hashLeft2216, collectionCount0. At around 27–29 seconds, the existing release dispatcher counter rose to8 and spatial residency query counter rose to26.
- V29r 150-request/160-slot case **remained on loading screen**. Map150, hashLeft0, hashExtra~24,428, EF extras105, collectionCount14836, releaseCalls0, residentQueries0.
- Both variants have only one caller for `00BAE790`: return site `0069926C`, around100 calls/sec, and function calls all return. It is probably normal per-update work, not the loading blocker itself.
- The existing `0059F3A0` hook has direct callers at `0066DC62`, `007C8B5B` that prepare position data; do not interpret its calls as proof vehicle readiness. `0059CD80` is a general dispatcher, not a loading-complete marker. Neither counter causally proves why loading does not complete.

## Exact Driver.exe native trace findings

`005A7490` reads `DWORD [ESI+0xC8]` at `005A74D6` and iterates staged IDs (`WORD [ESI+EBP*4]` at `005A7526`). On the request path, `005A75B6` calls `0059DB40`; it also reconciles old requests through `005A76F1 -> 0059DB20 -> 0059CD80`. The 0xC8 read is the staged list length, not necessarily current resident count. The extension of stock 50-ID buffer and all consumers must remain enabled.

`0059DB40` checks `BYTE [ECX+0x74DB9]`:

```asm
0059DB40 cmp byte ptr [ecx+74DB9],00
0059DB47 jne 0059DB4E
0059DB49 jmp 0059D740
0059DB4E ret 10
```

`0059D740` is a resource-dispatch body. It chooses a per-kind record with stride 0x734C, checks ID membership through `005A0A80` at `0059D76B`, and only on successful membership reaches `0059D778`. Downstream it uses per-kind resource flags and virtual methods; branch state or failure semantics are not fully classified.

`0059BAD0` is a resource control's **handler transfer/cleanup**, not game READY. At `0059BAF4` it pushes the 16-bit ID from `[ESI+0x28]`; at `0059BAF6` it calls `005A1FD0` to associate the handler with the completed-ID map; later `0059BB01` clears control+0x14 and resets control flags/ID. It may be called through vtable, hence no direct `E8` caller to its entry was found.

## V29s diagnostic build

Two matched builds share 160 graphics/admission slots, 45+256 ED/EF sidecars, and all previous capacity instrumentation. Main requests150; control requests45. Added 5 observation-only, byte-checked detours:

| Target | What V29s records | Exact overwritten bytes |
| --- | --- | --- |
| `005A7490` | staged request reconciliation calls, roster length | `A1 28 72 FE 00` |
| `0059DB40` | request call, kind, ID, manager `+74DB9` gate blocked | `80 B9 B9 4D 07 00 00` |
| `0059D740` | calls entering dispatch body, ID | `83 EC 14 8B 44 24 18` |
| `0059D778` | passed per-kind membership test, ID | `8B 4C 24 28 57 51` |
| `0059BAD0` | handler-transfer/control-cleanup entry, ID/state/handler-present | `56 8B F1 80 7E 4C 00` |

No V29q step hook or V29r caller hooks are present. Probes use preserved x86 general registers and EFLAGS, fixed counters and per-ID arrays; logging runs only in the existing 5-second monitor. No return values, branch targets, resource states or game-owned data are intentionally changed. Detours do patch executable code, though. If a signature fails, the optional probes roll back and the baseline V29p experiment continues. **Compile and offline checks are not runtime validation.**

Log entries: `DSFV29s_START hooks=5`, `DSFV29s_STATUS`, `DSFV29s_KIND`, and `DSFV29s_ID`. Field `blocked` means the byte gate was nonzero at `0059DB40`; `dispatch` may include calls entering `0059D740` from other sources. `pass` means `005A0A80` returned nonzero; `transfer` counts `0059BAD0` entry, not READY/drawable. IDs above511 counted in `idOverflow` without per-ID breakdown. `rosterLast` may return to zero when the staged requests are consumed; compare peaks and time series, not only final value.

## Test order and interpretation

1. Install **only** the V29s 150-request `.asi` with DriverNGHook off; reproduce Chapter 7 stall for at least60-90 seconds; collect full `%TEMP%\DSFV29f_worldmem_<pid>.log`.
2. Repeat with **only** the V29s 45-request/160-slot control and confirm gameplay enters, collecting full log.
3. `py tools\analyze_v29s_run.py path\45.log path\150.log` (passing the files in that order).
4. If V29s shows `blocked` or membership-failure asymmetry for the 150 run, trace corresponding per-kind records and gate writer. If both runs dispatch and transfer all observed IDs comparably, focus on global hash arena exhaustion (`hashLeft=0`, ~24K overflow allocations), subsequent streaming callbacks, and world-entry state, *not* the unrelated `00BAE790` maintenance loop. Do not force gates or claim map entries are drawable.

VS 2026 `DriverSF_V29s_NATIVE_TRACE.slnx`, Release/Win32, toolset v145. MSVC: `tools\Build-V29s-Trace.cmd`. Offline clang-cl: `python tools/build_v29s_cross.py` and `--requests 45`. Verification `python verification/verify_v29s_native.py --exe "path\to\Driver.exe"` (use --requests 45 for control).

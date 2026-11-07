# Audio192 result and exit crash — run260, 2026-11-05

The run strongly supports keeping the model-audio registry at192. All150 distinct profiles registered, and972 measured vehicle-audio setups succeeded without missing profiles, fallback use or setup failures. The exit dump identifies another resource-cleanup list whose index is corrupt at the fault. Its unchecked32768-entry boundary can overwrite its own count; replaying that mechanism reproduces the fault address exactly. The first corrupting write is not captured, so overflow is the leading explanation rather than a fully observed sequence.

## Inputs and scope

- `dist/MAIN_150/DSFV29s_audio_model192_260.log`:4373980B, SHA256 `D47FE5F70297DEE7D96B5751A3672D66AFD9F1ADD0CA247669EA15576E5BB90A`.
- `dist/MAIN_150/Driver_20261105_172557.dmp`:568081B, SHA256 `1DBB6D2520D9925C6EC79A9D39B79E34412A6A229895EDE02B8E2D82442CBF9C`; processID260 matches the log.
- Exact `Driver.exe` SHA256 `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`, base00400000.
- User reports proper sound on every tested car. Log coverage841.563s, approximately14m02s from probe startup. Longer testing remains useful.

Read-only analysis of the supplied files, targeted source and exact executable. No ASI behavior change, build, installation, game execution, debugger launch, dependency installation or web search.

## Audio result

All inherited baseline and slot checks pass. Eight-hook startup passes; one registry attaches safely to external storage, state1/capacity192/generation1. Peak count150,150 attempts and150 stores across150 distinct model keys. There are105 `wouldStockFull` attempts. Each profile registers once in this run, so these represent105 distinct registrations beyond the original45-entry gate.

All972 lookups hit; all972 bank checks and setups succeed. Missing profiles, fallback hit/miss, bank-not-ready and setup failures are zero. No full192 refusals, attach refusals, invalid observations, instance/key overflow or guard damage. All150 removals are observed during exit. Final `current=1` is a snapshot before the last native decrement, as documented; it does not establish a leaked entry.

Remaining normal event lookup failures140 do not equate to silent vehicles. There are9139 event lookup hits, no playing-record creation failures,16-event refusals, unmatched completion/removal, emitter refusals or Malloc/Malign failures. Downstream2048 lists remain below their ceilings, and shared-default pressure samples remain zero. The log does not prove every car/category audibly played or150 simultaneous drawable vehicles. Audible recovery plus the profile/setup census strongly supports the45-profile registry as the cause of the remaining fixed subset of silent cars.

## Exit fault: confirmed facts

Exception `C0000005`, thread14408, `00BAED45: mov DWORD PTR [eax],esi`, writing pointer18893184 to invalid destination63ADD284. Fault bytes in the dump match stock. Resource ownerEDI=01732BF0.

Native00BAECA0 handles resource removal and deferred destruction. Immediate caller is verified from the actual faulting stack layout: return00BBA791, call00BBA78C in bulk handle cleanup00BBA760. Saved loop registers show1024 handles remaining in that batch. Later stack candidates fit streaming/graphics teardown, but are not a complete symbol-based unwind. EBP is a general register here, so an EBP-chain unwind is unavailable.

```text
00BAED34  eax = [owner + 17E090]                 // count
00BAED3A  eax = owner + 15E090 + 4 * eax        // pointer slot
00BAED45  [eax] = esi                          // fault
00BAED4B  ++[owner + 17E090]
```

The array occupies15E090..17E08F: exactly32768 DWORD pointers. Count17E090 immediately follows. This append has no capacity check. Producers00BAEA60 and00BAEBE0 append to the same array with the same unchecked pattern at00BAEA75 and00BAEC2A.

Drain00BAE6B0 walks the list, calls destruction/free operations, then clears the count. Instruction00BAE6E4 reads the count indirectly through `[arrayBase+20000]`, not directly through owner17E090. Relocating storage requires updating that reader too. Constructor00BAE930 initializes the native count. The existing resource-collection131072 and indexed-reference262144 relocations do not expand this separate list.

## Why overflow is the leading explanation

The count itself is absent from the small dump. The captured destination and preceding LEA nevertheless prove an out-of-range scaled count: possible DWORD values18893181,58893181,98893181 orD8893181. Even the smallest is411644289, far beyond32768.

At index32768 the pointer store lands exactly on count018B0C80. A hypothetical previous pointer18893180, four bytes before the current18893184, overwrites that count; the native increment makes it18893181. The following append computes:

```text
(01732BF0 + 0015E090 + 4 * 18893181) modulo2^32 = 63ADD284
```

This exactly matches the fault destination. It corroborates count self-overwrite in a cleanup burst; it is not a captured prior store. Another earlier corruption could also have changed the count. The dump omits the queue/count, so claiming a directly observed32769th append would be incorrect.

The fault is outside the audio hooks, and all150 profile removals were observed without guard damage. This identifies the immediate failing subsystem; it does not exclude every possible indirect interaction with modified resource lifetimes.

## Next implementation

Keep audio192 and150 requests/160 slots/ED+EF/indexed262144. Expand this cleanup list's storage with explicit count/high-water/overflow measurement. Audit all three producers, the drain's indirect count access, and resets together. Preserve destruction/free order. Clamping/resetting the count or skipping the faulting store would lose pending work. An unreviewed drain call inside removal risks reentrancy/lifetime faults.

No new ASI was built. A focused cleanup-list test is the next implementation task.

## Reproduction

`python scripts/analyze_audio_model192_exit_dump.py --log dist/MAIN_150/DSFV29s_audio_model192_260.log --dump dist/MAIN_150/Driver_20261105_172557.dmp --output analysis/audio_model192_run260_exit_2026-11-05`

`python scripts/trace_model192_exit_crash.py`

Evidence: `analysis/audio_model192_run260_exit_2026-11-05/log_summary.json`, `dump_summary.json`, `crash_trace.json`, bounded native disassembly. Python standard-library minidump parsing; no target execution or process attachment. The trace asserts eight cleanup spans, the exact fault span, immediate caller and executable identity. Unverified stack candidates are labeled separately.

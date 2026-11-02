# V29r — caller-attribution diagnostic (2 November 2026)

## Observed basis

V29p with 150 requested and 160 configured slots stalled with or without DriverNGHook at
completed-map=150, sampled collectionCount=14836; 105 EF extra inserts and no guard trips.
V29q PID20688 produced 8,645 entry+return pairs by ~92s and approximately
109 million chained next-node steps. Thus the sampled routine at `00BAE790`
completed calls and was invoked repeatedly; one cyclic traversal is not supported.
High-frequency step detouring at `00BAE81D` (~2m/sec) likely perturbed runtime.

## V29r change

Retains V29p's 150 requests, 160 admission/graphics slots, EF extensions and other
telemetry. Instead of V29q's three hooks, adds only two optional hooks:
- `00BAE790`: capture original caller return address from `[ESP]` before prologue;
  preserve general registers, flags and game stack; aggregate into 64 fixed caller slots.
- `00BAE921`: count common function-epilogue returns.

No hook at `00BAE81D`, no logging, allocation, or game-owned data writes inside
hook execution. Entry code calls small fixed-table diagnostic recorder; counts use
atomic increment and CAS. Sampled status & top-8 callers are printed every five
seconds by the existing monitoring thread. Full calling paths are NOT proven
by the return address alone; inspect the native code surrounding top addresses.

Log lines:
- `DSFV29r_START hooks=2 ... stepHook=0` confirms probes installed.
- `DSFV29r_STATUS tick=... enter=... return=... map=... collectionCount=...
  hashLeft=... efExtra=... overflow=...` shows progression.
- `DSFV29r_CALLER tick=... caller=XXXXXXXX total=... delta=...`
  ranks callers by new calls during each 5s interval.

The 64-slot table is fixed-size; monitor `overflow`. It never tries to walk an
arbitrary game-owned pointer chain. Calls can arrive on multiple threads;
`entry-return` can be temporarily nonzero between paired increments. A return
address names the call site *after* a call, not the address of the CALL opcode.

## Files and verification

Main ASI: `build/v29r_callers/DriverSF_V29r_160Capacity_150Request_CALLERS.asi`
Optional request-count control (separate run only):
`build/v29r_callers_45control/DriverSF_V29r_160Capacity_45Request_CALLERS.asi`.
Its 160-slot capacity is the same as main run; it is NOT a byte-for-byte V29o
replica. Test main V29r first.

- VS 2026 solution `DriverSF_V29r_CALLERS.slnx` (Release / Win32).
- `tools/Build-V29r-Callers.cmd` for MSVC x86.
- `python tools/build_v29r_cross.py` for clang-cl; optional `--requests 45`.
- `python verification/verify_v29r_callers.py --exe <matching Driver.exe>`
- `python tools/analyze_v29r_run.py <raw worldmem log>`

Target game SHA256 `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
Native signatures: `00BAE790=83 EC 10 33 C0`; `00BAE921=5E 83 C4 10 C3`.
V29r is offline built and verified; NOT yet validated in-game. Do not install
multiple traffic ASIs simultaneously or run prolonged debugger pauses.

## Interpretation

If one `caller=` address dominates stalled intervals while the map remains 150,
inspect its surrounding disassembly and caller's wait/loading decision points.
If the same caller dominates both 45-success and 150-stall cases, it may reflect
normal frame maintenance instead; move to READY/streaming gates. Do not change
residency or hash list semantics from repeated-call counts alone.

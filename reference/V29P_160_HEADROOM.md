# V29p 160-slot headroom trial after two loaded 45-ID controls

Date: 2 November 2026. Exact Driver.exe SHA-256: `c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.

## New runtime evidence

The user reported that the matched V29o 45-request/EF-fix ASI loaded into gameplay twice: PID22116 with DriverNGHook disabled and PID29368 with it enabled. Full TEMP logs were copied unchanged:

| PID | DriverNGHook | Evidence SHA-256 | Final tick | Final completed map | EF external inserts | Final raw pending |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| 22116 | disabled | `4c6842c06818d181749bd5502a5cc825c30123d622de13220af0cafd116f4789` | 59484 | 52 | 7 | 0 |
| 29368 | enabled | `b9e3b0369f327f55e6a6b14b8a1bfebba592fe6258cddf2ab85cdd439b2add6c` | 59703 | 52 | 7 | 0 |

Paths: `evidence/raw/V29o_PID22116_45_loaded_worldmem.log` and `evidence/raw/V29o_PID29368_45_loaded_worldmem.log`. Both have `c7pre=1`, `reqHigh=45`, `boundary=0`, `efGuard=0`, and `total=drained=137` EF events without drops. The 45 build inherited a stale `DSFV29n_START request=150` *banner string*; the actual per-run `reqHigh=45`, compiled/exported configured count45, emitted 45-specific request message in x32dbg, and known artifact SHA identify the request. Both logs show City Region 18/18 `rawAbove3`, Sky `rawMax=20`, eight release calls, and later resident queries. Those pool counters are not visual proof; user visual report is recorded separately if supplied.

The stalled V29n 150-request PID23484 reached `reqHigh=150`, completed map150, EF active150 with105 external inserts, raw pending14836 by tick33109 and still at tick509906. It had zero release calls and zero resident queries in the captured interval. Its maps were structurally sound in paused memory. Thus the 45 and 150 outcomes differ with requested count under otherwise matched V29n/V29o code, and also with/without DriverNGHook in the two successful controls. The loaded 45 runs have seven completed-map entries above the 45 requested IDs, while 150 hits the configured 150 graphics/admission ceiling. **Inference:** the 150 request may consume headroom needed by other resources. The seven-entry gap does not identify their resource types, prove that all seven are always needed, or prove the capacity ceiling is the sole loading blocker. The raw `pending` field samples address `0x0188A6B8`; its semantic meaning is not established.

## Staged V29p test artifact

`build/v29p_160_capacity/DriverSF_V29p_160Capacity_150Request_EF.asi`  
SHA-256: `afbc2bb29edcfdaf7327f687dbb388153a811e0f7457d3f73d52bab3d78ce2c9`.

This retains the 150-ID Chapter 7 request and EF/ED sidecars, but raises pre-init graphics descriptors and package-zero kind0 admission from150 to160. It scales pool9 budgets A `0x8C00 -> 0x9400`, B `0x44C00 -> 0x49400` so per-descriptor budget stays at least the stock ratio. Expected handler initialization is164 total (160+2+2). The 256-entry handler sidecars and 301-node ED/EF map capacity remain unchanged. Guard checks now use the configured admission/graphics target. This is an experimental headroom test, not a validated fix.

Build: VS2026 v145 Win32 direct `tools/Build-V29p-160-Capacity.cmd` succeeded with prior C4018 and LNK4216 warnings. `python verification/verify_v29p_160_capacity.py` passed exact EXE hash, six native hook signatures, PE32 x86, required imports/EF exports, compiled target160, budgets, and request150. Existing V29o verifier and map16 verifier passed; `git diff --check` passed. No agent installation or in-game V29p test has occurred.

Next controlled run: install only the staged V29p traffic ASI in the isolated game, keep the same Chapter 7 save and DriverNGHook disabled first, and capture the complete TEMP log and x32dbg exceptions. Verify early `gfx=160`, `admission=160`, `handlers=164`, `kind0=160`, `c7pre=1`, `reqHigh=150`, `efGuard=0`. Check whether completed map exceeds150 and whether raw pending falls instead of remaining14836; time loading and inspect city/sky in gameplay if it completes. If V29p fails its early guards, stop rather than interpreting later load behavior. If it still stalls with map160, investigate the actual pending/READY path before increasing more capacity. If it loads, replicate without and with DriverNGHook, and inspect drawable model residency before claiming the 150-ID objective.

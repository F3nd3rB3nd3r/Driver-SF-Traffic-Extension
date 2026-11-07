# Indexed-capacity startup correction — R2

The user requested correction of the startup checks after run 24376 rejected
the ASI's own handler detour. The correction is built and staged only.

## Resulting behavior

`src/V29sStartupChecks.inc` separates stable PE/image identity checks from the
original handler signature. `ApplyPreinitGraphicsExpansion()` still requires
both, and records successful stock validation before writing configuration or
installing baseline hooks. Existing stock-only verifier callers retain their
stock-byte requirement.

At indexed installation, `VerifyIndexedStartup()` requires that saved preflight
success, revalidates stable executable identity, and verifies the exact owned
handler detour: installed flag, known bridge target, original-function pointer
equal to the owned trampoline, original five bytes in that trampoline, and its
return jump to the native continuation. Unknown or altered hooks are refused.
It does not expect stock bytes at an entry already detoured by this ASI.

The separate 31-site pending redirect check now has reason `pendingRedirect`
and status -9. The preflight/identity/owned-hook failure remains -1, with reason
`preflightIdentityOrOwnedHandlerHook`. All 13 indexed instruction fingerprints,
native cold-state checks, coordinated write/rollback logic and capacity values
are unchanged. No audio/traffic limits or vehicle request counts changed.

The startup header adds `startupCheckRevision=2`, `stockPreflight` and
`ownedHookVerified`. These establish the validation stage reached even if a
later guard refuses installation. Valid expansion still requires `status=1`,
13 patches, nonzero external base, then observed external-head status 2.

## Validation

`python scripts/test_startup_checks.py` builds and runs a standalone optimized
Win32 executable using the production C++ checks with synthetic address-backed
arrays. All **29 checks passed**. It reproduces the old stock-verifier rejection
after installing an owned-hook fixture, accepts that fixture through the new
late check, and rejects missing preflight; wrong branch target/opcode; altered
trampoline bytes/return jump; mismatched original pointer/installed flag;
unreadable addresses; changed PE identity/constructor; wrong base/PE offset.
It also checks the relevant DllMain source order and preflight capture location.

The initial test harness tried to reserve fixed virtual addresses, which were
unavailable in the test host inside and outside the sandbox. The final harness
uses translated synthetic memory, avoiding dependence on host mappings. This
is a harness setup adjustment, not a change to the game guards.

The active single-project solution built Release|Win32 successfully, with the
existing signedness/unused-function/exported-entry warnings.
`python scripts/verify_indexed_capacity.py` passed: quiet-baseline flags,
all 13 compiled patch rows, native executable identity, constructor slot-loop
interpretation at original and expanded counts/three bases, full free-chain
exhaustion and recycling, 165 address checks, operand restoration, all three
measurement bridges and no candidate near branches into stolen interiors.
The compiled indexed installer was also inspected for its preflight flag,
stable identity call and exact handler/trampoline checks before pending/table
validation. Evidence: `analysis/indexed_capacity_r2_2026-11-05/`.

The harness does not execute full DllMain or native game startup, and OS patch
failure paths were not fault-injected. Successful live installation, visual/audio
results and performance remain untested.

## Artifacts

- Project: `DriverSF_V29s_INDEXED_262144.vcxproj`, unchanged test flags and active
  Release|Win32 solution selection; target/output now explicitly R2.
- ASI: `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_INDEXED_262144_R2.asi`.
- Size: 92160 bytes.
- SHA256: `55EDA80BF5972CF0333DE3BF631AC6D145F529BC92026BDAA2789328D4398A46`.
- Procedure: `dist/MAIN_150/INDEXED_262144_R2_TEST_2026-11-05.md`.
- Log remains `%TEMP%\DSFV29s_indexed262144_<PID>.log`; distinguish revision 2
  using its header. Exactly one traffic ASI enabled; replace the older test.

The failed original indexed-test binary is retained unchanged with SHA256
`02F8A327B4DA8E3C7D5670C9CB7E3B801F33D144AC36FDBE0A40E00FB83B1DFD`.
No game installation/plugin folder inspection, deployment, game run or process
attachment occurred. Preserve 150 requests / 160 vehicle slots and ED/EF.

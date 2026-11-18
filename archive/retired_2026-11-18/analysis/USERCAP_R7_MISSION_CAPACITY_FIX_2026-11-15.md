# R7: widen the mission-admission capacity query

The R6 PID36080 dump showed280 READY traffic vehicles but a rejected, separate
mission request for catalog-valid model123. Native admission needed281 slots;
the package getter005A0AD0 reported the metadata byte255 despite290 allocated
slots. R7 corrects this getter to return configured full-width `ModelSlots`
for the recognized vehicle package/subtype0. No roster or vehicle ID is
compiled into the correction.

## Implementation

`src/V29sWideCapacityQuery.inc`, included in the production translation unit,
provides a fastcall wrapper matching the native thiscall getter's stack ABI.
It uses the same `g_pkg0` package recognition as the widened initializer.
Unrecognized packages and nonzero subtypes call the native trampoline.
The metadata remains byte-sized; subtype1/2 keep their original values.

The existing `g_sites` installer validates all18 getter bytes before any site
is changed. Its new detour steals one complete, non-relative six-byte MOV;
the existing trampoline executes that MOV and returns to the original suffix.
Existing rollback restores the getter too. No separate startup phase is added.
The manager-state gate0059C5C0 and mission admission005A6FD0 remain native,
including catalog checks, existing references/pending operations, strict
`required < capacity` comparison, queuing, and the separate loaded predicate.

`CAPACITY_QUERY_START` and sampled five-second `CAPACITY_QUERY_STATUS` report
hook status, configured model slots, override/fallback counters, last vehicle
package and last metadata byte. The hook increments counters without logging
on the query path. It does not claim a vehicle became READY or drawable.
The log analyzer validates these records for R7; missing/mismatched startup
records are rejected. R6 and older logs retain their existing requirements.

## Verification

Release/Win32 build succeeded with the existing unused-helper/exported-entry
warnings. The compiled wrapper loads `g_limits.models` as a DWORD, checks
package identity and subtype0, retains native fallback, and returns with ret4
on both paths. Its patch descriptor and complete signature are pinned to the
built PE and the supported Driver.exe hash. Imports remain KERNEL32/PSAPI.

Ten focused native fixture processes pass at18/150/245/246/255/256/280/700/1000
requests plus signature refusal. They execute copied retail catalog, manager
capacity gateway, mission admission, list insertion and loaded-predicate code.
Actual patched native handler initialization also verifies full-width counts;
the unrelated resource factory is fixture supplied. Synthetic lists/catalogs
model the captured state. The game entry point and asynchronous loader are
never executed.

The280 case reproduces the unpatched255 rejection, initializes290 vehicle
slots, then successfully queues mission123 with the correction. It stays
unloaded until real loading occurs. Tests retain duplicate handling, unknown
ID refusal, native manager-state gating, exact insufficient-capacity boundary,
pending-operation budget, unrelated packages/subtypes, package recognition
changes across12 synthetic generations, ABI stack/nonvolatile registers and
byte-exact hook rollback. A mismatch in the getter suffix refuses installation
before any site is mutated.

All36 inherited fixture outcomes also pass:19 wide/storage/startup,6 traffic
queue and11 handler/scheduler cases, including expected guarded failures and
the intentional historical fourth-queue fault reproduction. All362 audited
scheduler operands are still checked. Total46 expected offline outcomes.
Cold combined startup passes at150/700/1000 and customized settings.
These checks do not establish700/1000-car gameplay support.

Evidence: `analysis/mission_capacity_r7_2026-11-15/` (scoped backup, native and
compiled disassembly, fixtures, source hashes and build verification), plus
the new `wide_limits_r7`, `traffic_queues_r7`, and `stream_handlers_r7` folders.
Inherited static scheduler input copies have a source/hash manifest. Prior
R6 evidence and ASI/HASH180K INI remain unchanged.

## Candidate and live follow-up

Staged `dist/USERCAP_R7/DriverSF_Unlimiter_USERCAP_R7.asi`:306176 bytes,
SHA256 `248d4c3b1a1fdc7ec0fb9ad38cb90ec6292b51a3412b3e59c00c6ba3b075079f`.
The included INI has identical active values to R6/HASH180K; only its comment
header is updated. Keep the same280 AI and save for the first loading retry.
See `dist/USERCAP_R7/R7_TEST.md`.

Live Story entry, rendering/audio, activities and lifecycle remain pending.
No game/debugger launch, deployment, installed INI/asset/save change, commit
or push was performed. Existing uncommitted work was preserved.

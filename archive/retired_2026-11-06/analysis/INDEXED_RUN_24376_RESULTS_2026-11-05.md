# Indexed capacity run 24376: expansion refused by its own startup check

## Result

Supplied log: `dist/MAIN_150/DSFV29s_indexed262144_24376.log`.
This is **not a successful 262144-slot test**. Its first line states:

```text
DSFV29s_INDEXED_START status=-1 reason=identityOrPendingRedirect patches=0 expectedPatches=13 externalBase=00000000 nativeSlots=131072 testSlots=262144 pendingRedirectPatches=31 baselineStatus=1 startupPolicy=untouchedColdOnly activeMigration=0 hotHandleHooks=0
```

The next header has `hooks=3` but `capacityChanges=0`. All 198 status
summaries retain `indexedStatus=-1`, native base `018B0C84` and
`indexedSlots=131072`. No external indexed backing was installed. The normal
150-request / 160-vehicle-slot baseline still reached startup status 2.

## Confirmed integration error

`IndexedInstall()` calls `VerifyExecutable()` before its 13-operand and cold-state
checks. That shared verifier expects the original five bytes
`A1 28 72 FE 00` at handler initializer `005A1420`.

However, DllMain calls `InstallHandlerHook()` before `IndexedInstall()`. A
successful `InstallHandlerHook()` writes an `E9` jump at `005A1420` and sets
`DSFV27B_HookInstalled=1`. Only after this success does execution reach the
indexed installation block. Consequently the stock-byte comparison fails
deterministically on the ASI's own handler detour. Short-circuit evaluation
returns `identityOrPendingRedirect` before checking the pending redirect.
This error does **not** establish that Driver.exe is unsupported or the pending
redirect failed. It also does not establish whether the cold-state guard would
pass: that guard was never reached.

Relevant source locations (line numbers at analysis time):

- `src/DriverSF_V29b_NativeFastPath.cpp:289`: original handler signature.
- `src/DriverSF_V29b_NativeFastPath.cpp:2286`: handler entry becomes `E9`.
- `src/DriverSF_V29b_NativeFastPath.cpp:2963`: handler hook installed first.
- `src/DriverSF_V29b_NativeFastPath.cpp:3056`: indexed install attempted later.
- `src/V29sIndexedCapacity.inc:87`: stock verifier reused after those changes.

The earlier offline tests validated compiled patch records, constructor-slot
arithmetic and measurement bridges. They did not exercise DllMain's ordered
installation against its evolving executable bytes. Their passing result was
insufficient to detect this integration error.

## What the run still measures

The existing diagnostic successfully measured another **native indexed table**
run. Parsed evidence: `analysis/indexed_capacity_test_2026-11-05/run_24376_summary.json`.

- Final printed tick: 197813 ms; 984 initialized 200 ms samples.
- First printed empty-head status: tick 58250 ms. This is a one-second printed
  status, not the exact first empty sample or allocation failure time.
- 699 empty-head samples, two sampled transitions to empty; longest sampled
  empty interval 100000 ms. Slots briefly became available again.
- Pending references peaked at 16009 sampled and ended at 1771.
- Audio-bank queue: 153 observed inserts, peak 2/50, zero boundary attempts.
- Other three pools: no observed insertions, sampled peak zero; coverage limits
  from the earlier diagnostic still apply.
- No invalid-head summaries or file errors reported.

These results continue to show pressure on the native indexed table. They do
not measure the expanded table, establish visible vehicle readiness, or reveal
whether audio recovered. The user has not supplied detailed visual/audio/FPS
observations for this run.

## Required correction before another capacity run

Validate stock executable bytes before installing our hooks, then use explicit
phase-aware checks at the indexed-install stage, including verifying the exact
expected existing handler detour rather than expecting stock bytes there.
Preserve executable identity checks, all 13 full-instruction signatures, the
31-site pending redirect checks and untouched-cold-only guard. Add a focused
startup-order regression check with the handler hook already installed, plus
negative checks for an unknown detour or mismatched identity. Do not bypass the
guards generally, move an active table, or repeat this known-refusing binary.

Only log/source analysis and documentation updates were performed this turn.
No code/ASI modification, build, installation, game run or live process inspection.

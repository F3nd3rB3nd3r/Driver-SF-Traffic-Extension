# V29s capacity measurement test — 2026-11-05

## Artifact and status

`DriverSF_V29s_160Capacity_150Request_CAPACITY_TRACE.asi`

SHA256: `0C8DD7FA4C04EF7E3F6B53B545BE10D3FAC418E4BB43CEF40869CC608358D5BC`

Built and staged, **not installed or run**. The installed traffic ASI was not inspected or changed during this task; the last handoff identifies quiet 45 as active. The active solution now selects the capacity project alone. Older projects and binaries remain available.

This is the quiet ordered-150 baseline plus `DSF_V29S_CAPACITY_TRACE=1`. It preserves 150 requests, 160 base slots, ED/EF sidecars and all existing behavior-changing expansions. It does not enlarge the newly investigated 50-node queues or indexed-handle table. Old handle/hash/draw/audio trace hooks are not enabled.

## Run

1. Close DriverSF before swapping ASIs. Disable the existing traffic ASI, then install this file alone in the usual game plugins folder. Keep exactly one traffic ASI active. This staging task did not perform that swap.
2. Run the same save/route used with quiet 150, without x32dbg. Allow loading and spend a few minutes driving and switching vehicles normally.
3. Note loading/standing/moving FPS, audio categories that fail, and approximate times when a body is invisible and then appears. Also note any substantial roster/save/mission transition. These observations are necessary to correlate the log with symptoms.
4. After exiting, preserve `%TEMP%\DSFV29s_capacity_<PID>.log` in this directory, together with any matching crash dump. This variant uses its own capacity log; no new `DSFV29f_worldmem` log is expected from it.

## First validation

Find `DSFV29s_CAP_START hooks=3 expectedHooks=3`. Status should subsequently show `startupStatus=2`, `graphicsSlots=160`, `requestLast=150` at the expanded chapter request, and `chapterHits` nonzero. A later different native request may change `requestLast`.

If hooks is `4294967295` (the unsigned encoding of -1), signatures or a required baseline hook failed: do not interpret absent queue events as evidence of spare capacity. `fileStatus` should remain 0. No runtime installation or performance result exists yet.

## Measurements

- `CAP_QUEUE`: four queues, each with native capacity 50: audio banks, retiring IDs, retained vehicles and queued vehicle operations. `peakAfterInsertProjected` uses the active count immediately before the native insertion plus one. It is a projected count, not proof that an invalid insertion completed. `peakActiveSampled` and current fields are sampled every 200 ms and printed every five seconds.
- `CAP_BOUNDARY`: the first suspect insertion per queue, logged **before the native write**. Records owner, header, cursor, recycled count and active count. It means either a fresh allocation cursor is outside/misaligned relative to its native pool, or the pre-insertion active count is already at least 50. Examine the recorded fields; this is not automatically a successful expansion, prevented crash or diagnosed root cause.
- `CAP_STATUS`: once per second, native indexed free head and pending-reference count. `initializedByObservedHead=1` requires both an observed valid non-null head and the initialized hash cursor. Only then do `emptySamples` and sampled empty intervals accumulate. If no valid head was ever observed, zero head cannot be classified as full. `headValid=0` indicates a non-null pointer outside the expected inline table.

Queue insertion hooks catch short bursts on their covered paths. The shared 20-byte allocator has an assembly caller filter for `005A68A7`, `005A7E3D`, `005A7E6D`, followed by an owner/header check for the live traffic spooler. The retirement hook covers native first-merge insertions; an unidentified alternative writer is outside that coverage. Sampled current counts provide a secondary observation, but absence of a boundary line is not a universal proof against every possible writer.

Native handle exhaustion is sampled, not counted at every allocation attempt. `emptySamples` is not a rejected-allocation count; `pendingPeakSampled` is not total graphics residency. No field establishes READY or drawable vehicles.

## Logging and behavior

Normal insertion hooks do no file I/O, allocation, VirtualQuery or list walks. They update fixed diagnostic counters. Each queue can emit at most one synchronous boundary line (four total) so evidence can survive a following crash. Other output comes from the monitor. Hook bridges preserve flags, integer registers and x87/SSE state. The shared allocator skips unrelated callers before saving registers/FP state.

The test deliberately continues the original native instructions, including any original overflow. It does not prevent a crash. Runtime overhead has not been benchmarked or verified; report a material FPS regression before attributing symptoms to a limit.

## Offline validation

- Release/Win32 build passed; no game was launched.
- Exact native hook signatures matched the SHA256-verified executable.
- Compiled bridges were disassembled and checked for argument offsets, FXSAVE/FXRSTOR, flags/register restoration and the correct trampoline jumps, including linker thunks.
- Whole-text candidate near-branch scanning found no near CALL/JMP/Jcc entering the stolen instruction interiors. This is a byte-level check, not a replacement for runtime validation.
- Project definitions equal the quiet-150 project definitions plus the single capacity-trace flag. The active solution contains only the new project.
- Build/staged hashes match. Evidence is in `analysis/capacity_trace_2026-11-05/verification.json` and the bridge disassemblies. Run `python scripts/verify_capacity_trace.py` to repeat the offline checks.

The audio lifecycle follow-up confirmed constructor `008498E0`, sole observed insertion `0084BFDD -> 00849870`, normal state-4 retirement `0084A18B..0084A1A9`, and cancellation `0084BFF0 -> 008497D0`. Cancellation removes the queue entry before bank unload and its callback clears bank ID/state. Examined shutdown code waits on the bank-object lifecycle and destroys other bank lists; it supplies no separately identified growth/reset of this 50-node pending queue. Exact field-reference searches found its cursor/count initialization only in the constructor, with subsequent mutations in insertion and the two recycling paths.

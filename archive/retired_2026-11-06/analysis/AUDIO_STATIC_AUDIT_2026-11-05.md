# Audio after save loading: static audit of Driver.exe

## Result

The next useful test is an audio-specific measurement on the working R2 visual
baseline. This audit identifies two previously unmeasured native capacities,
fixed audio byte budgets, and a concrete error in the legacy callback trace.
It does **not** establish which limit, if any, fails during the user's runs.

1. The native sound-emitter object pool admits **90 live objects**. Its full
   branch returns a null object. This is separate from the measured 50-node
   bank-operation queue and from middleware voice counts.
2. The audio memory manager is initialized for **20 pool descriptors**, with
   separately configured byte budgets. Twenty pools is not twenty banks.
3. The game's bank callback marks state 4 regardless of actual load result.
   The old diagnostic also sampled the callback's **pool ID**, not AKRESULT.
   Accepted submission + callback + nonzero bank ID therefore did not prove a
   successful bank load.
4. The exported monitoring/error callback APIs are stubs returning 4 in this
   executable. Enabling `SetLocalOutput` cannot recover middleware errors here.
5. Real allocator return points, pool occupancy fields and a shared numeric
   `PostEvent` route are available for a bounded probe.

Keep R2's 262144 indexed-reference slots, 150 requests / 160 vehicle slots,
ED/EF, pending/hash redirects and quiet monitoring. No audio limit was changed.

## Scope and evidence

Exact authorized executable:
`E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256
`C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.
The executable was read, never loaded or run. No other game folder/file was
needed; no external download, web search, debugger, build or installation.
The working ASI, source behavior and active solution were not changed.

The runtime premise remains the user's three R2 logs: successful expansion,
zero sampled indexed exhaustion, cleared pending work, cars appearing properly,
but nearly all audio disappears after save loading. Initial menu audio works;
a few Shift effects survive. See `INDEXED_R2_THREE_RUN_RESULTS_2026-11-05.md`.

New evidence is in `analysis/audio_static_2026-11-05/`:

- `wwise_exports.json`: 157 non-forwarded Wwise exports in the exact executable.
- `undecorated_exports.txt`: selected exact ABI declarations decoded with the
  already installed Microsoft `undname`, including the four-argument bank callback.
- Focused native function disassemblies and explicitly targeted reference/call
  candidate lists. Candidates were followed to relevant instruction boundaries.
- `verified_inventory.json`: verified budgets, layout and seven candidate hook
  signatures. `probe_branch_candidates.json`: no candidate near CALL/JMP/Jcc
  into those stolen spans; indirect/short-branch and compiled bridge validation
  remain implementation work.

Some disassembly ranges start/end in neighboring instructions or include jump
tables; those fragments/data are not used to establish functions or behavior.

## 1. Legacy bank results were not actual completion results

The exact exported overload at `00C51ED0`, called by the game's `0084BEB0`, is:

```cpp
AKRESULT __cdecl AK::SoundEngine::LoadBank(
    void const* data, unsigned long size,
    void (__cdecl* callback)(unsigned long bankID, AKRESULT result,
                            long poolID, void* cookie),
    void* cookie, unsigned long& bankID);
```

Thus the native callback stack is:

| Address at callback entry | Value |
|---|---|
| `ESP+04` | Bank ID |
| `ESP+08` | Actual AKRESULT |
| `ESP+0C` | Pool ID |
| `ESP+10` | Cookie: game's bank-job pointer |

The game callback `0084BE70` loads the cookie into EAX, then unconditionally
writes `[job+14]=4` at `0084BE74`. It never tests the AKRESULT. State 4 is
completion bookkeeping, not a success predicate.

In `src/V29sAudioBankTrace.inc`, `DiagAudioCallbackBridge` uses `[esp+48]`
after `pushfd` + `pushad` (36 bytes). That is original `ESP+0C`: **pool ID**.
Correct AKRESULT is `[savedFrame+44]`, original `ESP+08`. The old `cbRaw`,
`cbResult1` and `cbResultOther` fields cannot be reinterpreted as load results.
This is a measurement bug, not evidence that the passive hook suppressed audio.
Those older five audio hooks are not enabled in R2.

There is another independent ambiguity: `00C51ED0` derives a bank ID via
`00C60E50` (BKHD offset `+0C` when input is sufficiently large) and stores it to
the output at `00C51F05` **before** enqueuing through `00C51B20 -> 00C62230`.
A nonzero job bank ID can therefore exist before asynchronous parsing finishes.

The immediate result 1 observed by older submission probes means queue
acceptance on the examined path. `00C62230` registers the callback, queues a
36-byte bank command through `00C62100`, signals the worker, and returns its
queue/registration result. Actual parsing can still fail later.

**Consequence:** 153 accepted submissions/callbacks and state-4/nonzero-ID
lookups did not exclude audio-bank loading failures. The new test must record
the second callback argument separately from bank ID, pool ID and cookie.

## 2. Fixed byte budgets and a 20-pool limit

Game initialization passes the following values into the exported middleware
APIs. `A` is the sound manager at `[Engine+50]`.

| Setting/pool | Native source | Configured value | Where ID/state is retained |
|---|---|---:|---|
| Maximum pool descriptors | `0084A4BC -> 00C99D10` | **20** | max global `01D72388` |
| `FilePackagesPool` | `0084A5D5` | `88000` = 544 KiB | `A+0A2C` |
| `PrepareEventPool` | `0084A63F` | `4B000` = 300 KiB | `A+0A30`; passed as InitSettings `+18` |
| `StaticBanksPool` | `0084A683` | `42B800` = 4,372,480 B ≈ 4.17 MiB | `A+0A34` |
| Shared default pool request | `0084A64D` | `599999` = 5,872,025 B ≈ 5.6 MiB | pool ID `00FDD3B8` |
| Platform-side pool request | `0084A646` | `300000` = 3 MiB | allocated via `00C54EE0`, ID `00FDD3C0` |
| Command buffer default | `00C50F29` | `40000` = 256 KiB | InitSettings `+14`; a different structure from the 50-job queue |

The wide strings at `00E36CC8`, `00E36CA4`, `00E36C84` establish the three
explicit pool labels. The platform-side field is copied at `00C54F30` and used
by pool allocation `00C54EE0`; its detailed consumer role is not inferred solely
from a modern SDK field layout. Other default InitSettings fields are not
relabeled as voice limits without proving their consumers.

`00C522A0` allocates the shared default pool from saved InitSettings `+0C`
(`01D71870`). `00C54EE0` uses platform settings `+1C` (`01D718FC`) for the
other pool. These are requested sizes. Creation rounds to block granularity
and can subtract allocator metadata, so usable totals can be smaller.

`00C99D10` forwards the configured descriptor count to `00C999B0`, which
allocates `count * 4C` records. `CreatePool` at `00C99D90` rejects when active
count reaches max, then also scans for a free descriptor. It can fail for
invalid attributes or backing allocation as well; `FFFFFFFF` alone does not
identify descriptor saturation.

The bank parser has a conditional path `00C64711 -> 00C60F10` that can create
a pool for bank media when the stored pool is `-1`. The DATA path also has a
view/no-copy branch, so **one bank does not necessarily consume one pool**.
The 20-pool setting is not evidence that the 21st vehicle bank must fail.

The game's vehicle-bank input can be referenced in place or copied through
`00848210`, which uses a separate aligned CRT allocator. Archive bank bytes
cannot simply be summed and compared with `StaticBanksPool` to prove overflow.

### Safe measurement fields

| Native global/field | Meaning supported by code |
|---|---|
| `01D72390` | Pointer to pool record table |
| `01D72388` | Maximum descriptors |
| `01D7238C` | Active pool count |
| `record+00` | Nonzero initialized/active pool record marker |
| `record+1C` | Pool attributes |
| `record+00/+04` | Block count / block size established at creation |
| `record+18` | Free block count on the fixed-block path |
| `record+44` | Malloc/Malign byte accounting; not all allocation modes |
| `record+48` | Usable pool total |

Each record is `4C` bytes. `Malloc`/`Malign` add allocation size plus local
overhead to `+44` on success; `Free/Falign` subtract on release.
For attributes with bit `08` set, `GetBlock` instead removes a free block and
decrements `record+18`; it does not update `+44`. Track block count/size and
free blocks for these pools. A zero `+44` alone does not establish that a
fixed-block pool is unused.
`GetPoolMemoryUsed` at `00C99B60` returns `+48` in output word 0 and `+44` in
word 1 under the pool lock. Validate IDs and initialized records before use;
several native APIs do not defensively reject every negative ID.

`GetPoolStats` at `00C99B10` only fills total plus zeroed detail fields in this
build. Do not interpret its zeroed used/peak/allocation fields as measured zero
usage. The useful byte counters are independently present at `+44/+48`.

No live count/usage/allocation failure has yet been measured. Fragmentation can
also cause failure before used bytes reach total; global process memory is not
a substitute for observing the failed pool and requested block.

## 3. Separate 90-object emitter pool

This is a game-side registered sound-object pool, not the bank-operation queue
and not the middleware's physical voice count.

| Field relative to A | Meaning |
|---|---|
| `+0E98` | Fresh/recycled allocation cursor |
| `+0E9C` | Recycled node count |
| `+0EA0` | Active object count |
| `+0EA4 .. +92D4` | 90 nodes, each `178` bytes |
| `+92D4/+92D8` | Active doubly linked list sentinel |

Node payload is `170` bytes after eight link bytes. Constructor `008498E0`
initializes the pool at `008499B1..008499D7`. Allocation entry `0084AA20`
compares `[A+EA0]` with `5A` (90) at `0084AA29`; `JAE 0084AAB8` writes a null
result at `0084AABF` and returns without adding an object.

Below the limit it calls `0084AFE0`, then `0084C2F0`, which registers the object
through `AK::SoundEngine::RegisterGameObj` (`00C51970`). Cleanup `0084C180`
cancels callbacks and unregisters the game object. Update/removal in
`0084A1E0..0084A24B` uses `0084C320` as an object reference-count predicate,
unlinks finished objects and decrements `A+EA0` while recycling their nodes.
This establishes ordinary retention/release paths, not a proven leak.

Another `90 * 12` structure at `A+0A4C`, nodes `A+0A58 .. A+0E90`, tracks
deferred object cleanup. It is also absent from the current capacity log.
Neither pool's live peak or admission refusal is known.

Several game wrappers can post events directly using the default game object,
bypassing this emitter allocation. This allows an emitter-path failure and
some surviving effects to coexist in principle. **The specific surviving Shift
effects and missing menu/car paths have not been bound to these routes.** Do
not call the symptom proof that the 90-object limit was reached.

Growing the compare alone is unsafe: physical object storage borders other
sound-manager structures. If runtime admission failure is confirmed, a full
relocation/lifecycle audit would be required before expanding this pool.

## 4. Shared event admission and silent failure paths

Export `00C54DA0` is the numeric `AK::SoundEngine::PostEvent` overload. Its
return is a playing ID; zero denotes failure in the examined code, nonzero is
admission/queueing and does not prove audible output.

Observed game callers include manager wrappers `00848390`, `008483C0`,
`008491B0`, `008491F0`, `00849230`, the object route near `0084CCC4`, and
other callers around `0072Fxxx`. Exact menu/engine event IDs still need runtime
correlation. String overloads are exported but were not direct-call targets
in the bounded call inventory; indirect calls are not excluded.

The common helper `00C53FB0` looks up the event definition at
`00C53FFF -> 00C53F50`, using the index at `[01D71838]+668`.
At `00C54012` a missing definition returns zero via `00C54062`.
If present, it allocates a playing ID and calls `00C5F0B0`; that helper allocates
a `38`-byte playing record from pool `00FDD3B8` at `00C5F0CA`. A null allocation
returns 2, causing the outer helper's cleanup and zero return at `00C54052`.
Only after admission does it queue the event command through `00C5CCC0`.

Consequently the shared default pool participates in both audio object/bank
metadata and event-admission work. Exhaustion is a concrete mechanism capable
of affecting multiple categories, but it has not been observed in the user's run.

The event command writer `00C5CBD0` handles a `34` return from its buffer
allocator by signaling/waiting for consumption and retrying. This is separate
from the game-side bank queue and the immediate playing-record allocation.
Its initialization `00C59B40` reads InitSettings `+14` at `01D71878` to size
the command storage.
Do not classify all nonzero `PostEvent` IDs as completed voices.

## 5. Monitoring stubs and alternative diagnostics

Exports `AK::Monitor::PostCode`, both `PostString` overloads and `SetLocalOutput`
all map to `00C52550`: `MOV EAX,4; RET`. No callback is installed or error
message collected by calling that function. `GetPoolName` and several other
monitoring APIs are also simplified/stubbed; the explicit game labels and IDs
above are more useful than relying on runtime names.

Actual `Malloc` (`00C99A40`) and `Malign` (`00C99AB0`) return a null pointer on
nonzero-size allocation failure. Their shared-style epilogues at `00C99AA5`
and `00C99B04` offer failure-only logging opportunities. A fast success path
must avoid expensive register/FX saves or file writes for every allocation.
Initialization/bank code locally uses return `34` on several null-allocation
paths; retain exact numeric results rather than importing enum meanings from
an unrelated middleware version.

## Recommended next measurement

The static map is sufficient to implement a narrow audio diagnostic. See
`analysis/AUDIO_PROBE_PLAN_2026-11-05.md` for fields, signatures, coverage and
decision rules. First measure actual callback AKRESULT, emitter refusals and
pool/allocator pressure while preserving R2. Include selected numeric event
failure counts/callers and low-rate audio initialization/volume snapshots.

Do not expand the 50-entry queue, the 20-pool limit, the 90-emitter limit or byte
budgets until a corresponding failed stage is recorded. No new ASI was built
or installed during this research.

## Reproduce

```powershell
python scripts/audio_static_audit.py exports
python scripts/audio_static_audit.py disasm 84A400:84A780 84AA20:84AC00 C51ED0:C51F20 C53FB0:C54090 C99D90:C99F70
python scripts/verify_audio_static.py
```

The helper also supports explicit `calls`, `refs` and filtered `strings` queries.
It checks the exact executable SHA256 and writes evidence only in this workspace.

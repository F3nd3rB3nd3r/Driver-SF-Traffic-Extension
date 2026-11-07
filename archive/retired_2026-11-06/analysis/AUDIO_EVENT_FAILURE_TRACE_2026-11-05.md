# Audio event failure trace — 2026-11-05

`Driver.exe` is enough to resolve the immediate failure control flow and identify two failed event names. No debugger is needed for this static stage. The existing allocation probe, combined with this trace, strongly indicates absent event definitions for the logged ordinary zero returns. However, it does not explain why each silent car lacks audio, nor measure a newly recognized16-entry game-side limit that can refuse before the probed API is called. A focused runtime reason/object probe is the next useful measurement; x32dbg is optional, not a prerequisite.

Exact authorized executable SHA256: `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`. All addresses below are preferred-image VAs. Evidence: `analysis/audio_event_failure_trace_2026-11-05/`. Reproducer/26 verified instruction spans: `python scripts/trace_audio_event_failures.py`.

## 1. Exact numeric PostEvent zero paths

Numeric API `C54DA0` calls common helper `C53FB0` at `C54DFE`. Its normal exit returns that helper's playing ID unchanged. The helper follows:

1. `C53FF9..C54006`: look up the event ID in index `[01D71838]+668`, calling `C53F50`.
2. `C5400B..C54012`: if no event-definition pointer is returned, branch directly to `C54062`, returning0. No playback record or voice is created on this branch.
3. If the definition exists, obtain an ID using the counter `[01D7189C]` and call playing-record creation `C5F0B0` at `C5404D`.
4. `C5F0C7..C5F0CA`: request exactly56B from shared default pool via `C99A40`. If null, creation returns2 at `C5F1A5`; the caller releases the definition and returns0. Otherwise creation returns1.
5. After successful creation, queue the command at `C5407C` and return the generated playing ID at `C54081`. The inspected helper does not validate the game object's registration or create its eventual voices before this return.

The ordinary helper therefore does not have a separate object-registration,255-slot,90%-pressure or physical-voice refusal branch that explains an immediate zero here. Those later mechanisms can still silence an admitted event. A generated ID could technically be0 on32-bit counter wrap; repeated failures in these runs are not plausibly explained by billions of ID increments. That counter is not directly measured.

The numeric API also has an earlier optional external-source conversion failure at `C54DD3`. It is separately probed: postEarlyZero0 in both supplied runs.

### Applying this to the logs

The competing ordinary allocation-failure branch is covered by the existing Malloc probe. `C99A40`'s only earlier unprobed return is a zero-size request at `C99A49`; the playing-record request is56B. All positive-size allocation returns go through `C99AA5`, where the installed probe records null EDI results. Both16680 and30860 show Malloc failures0 and positive normal event-zero counts.

Thus **missing event definitions at lookup time are strongly indicated for these logged zero results**, subject to the validated probe's runtime coverage and the counter-wrap exception. Direct lookup-result capture has not yet been performed, so this is an inference by elimination, not a measured branch count. It does not prove the correct bank was ever requested, that an expected definition remains resident, or which car requested it.

`C53F50` uses193 hash buckets, compares the full32-bit event ID at node+0C, and follows node+08 links until match/null.193 is a bucket count, not a193-event ceiling. No255 early stop or fixed maximum chain length appears in this lookup.

## 2. Largest failure ID resolves to none

Exported narrow-string `GetIDFromString` at `C52670` lowercases ASCII A–Z, then calls `C525B0`. The latter starts at811C9DC5, multiplies by01000193 before XORing each byte, with32-bit arithmetic. Verified native instruction bytes give:

| Input | Hash |30860 zero results |
|---|---|---:|
| `none` /`None` /`NONE` | 2CA33BDB | 95 |
| `Menu_Move` /`Menu_move` | 72DCBB24 | 1 |
| Numeric0 | 00000000 | 1 |

Executable strings match these names: `none` atDEA150, `Menu_move` atE1DC14 and `Menu_Move` atE1F500, among others. State-setting code atC513C1 and exported numeric SetStateC52BC7 also handles2CA33BDB specially by translating it to0, corroborating its sentinel-like use in another API. This special handling does not apply to PostEvent.

This corrects the earlier interpretation of the three repeated hashes as equally promising missing-car sound leads: **95/123 failures use the hash of none**. They may be unset/optional event references being posted. They do not establish95 failed real engine/tire/horn starts. Hash collisions or runtime construction are possible; the log did not capture original strings, so it does not prove each caller literally supplied the text `none` or that all such requests are harmless. A missing assignment to a mandatory event could still matter.

The remaining two repeated IDs38C61846(12) andDFBD7040(11) have no matching complete embedded printable event name in the inspected executable sections. Both are posted on the same sound object/gameObject `1A802CA4` in30860. The95 none-hash failures span13 sound objects. None of these object addresses has yet been mapped to Zonda,370Z,Bentley or a particular sound category. Menu_Move's one failure is at10235ms, before the first150-request sample; it does not prove continuing menu-event failure after loading.

All123 details are retained in30860:95 none-hash,1 Menu_Move,1 numeric0,26 other failures (23 are the two repeated unknown hashes;3 are other startup hashes). The longer16680 log retains only its first128 of3545 failures;101 of those128 are the none hash. Later individual IDs cannot be reconstructed from that bounded detail buffer.

Names absent from the executable may live in authored audio/bank or vehicle-assignment data. Static executable inspection alone cannot reconstruct their expected vehicle associations. Capturing input strings/original game callers in a narrow probe can help without a broad external-file search.

## 3. Separate16-playing-event limit, invisible to existing PostEvent counters

The common game object wrapper starts at `84CC90`. At `84CC98` it checks DWORD `[ESI+16C] >=16`; branch `84CC9F ->84CD08` returns0 **before** calling PostEvent. The current audio probe sees only calls that reached middleware PostEvent, so these early refusals are absent from its1512 attempts/123 zero results.

Object layout is fixed:

- Event entries start at object+2C, stride20B.
- Capacity16, so `2C +16*20 =16C`.
- DWORD count is at object+16C and ends at170, the native sound-object payload size. Pool node stride178 includes the8-byte link prefix.
- On nonzero playing ID, the wrapper records event ID/playing ID/callback/cookie fields and increments+16C at84CCF8.
- It installs callback84C9B0. After validating manager/object state, callback handling84C920 finds the playing record. Callback type1 takes the removal path through84C992 ->84CE10. That erase helper compacts20-byte records and decrements header+140, equivalent to object+16C, at84CE5F.
-84C320 uses this same count to determine whether the sound object still has active entries. Copy helper84AE70 also copies this count and the occupied fixed-size records.

This is separate from90 sound objects, the middleware global255/2048 lists, and the per-record8/255 attached-user limits. It counts outstanding event instances on a sound object, not exactly16 audible voices,16 sound categories or16 vehicle models. A car's audio may span multiple sound objects, and none of the supplied logs proves this list reaches16 or that callbacks fail to remove records.

If it fills, attempted sounds can disappear without a Malloc failure or an observed PostEvent zero. It is a concrete measurement gap worth covering. Increasing only16 would write beyond the fixed array into the count/adjacent storage; an actual expansion would require a separate layout/lifetime audit. No increase is justified yet.

## 4. Concrete next runtime measurement; debugger optional

Preserve existing capacities and graphics settings. A bounded reason/object probe can distinguish:

| Candidate native span | Measurement | Inputs before replaying stolen instructions |
|---|---|---|
| C5400B,7B `3B C6 89 44 24 20 5E` | Event lookup pointer present/missing, event ID/object | EAX=definition/null; event `[ESP+34]`, gameObject `[ESP+38]` |
| C54057,6B `8B 4C 24 1C 8B 11` | Selected playing-record creation failure | EAX=result; event `[ESP+30]`, gameObject `[ESP+34]` |
|84CC98,7B `83 BE 6C 01 00 00 10` | Every wrapper request and pre-PostEvent16-entry refusal | ESI=sound object, count+16C; event `[ESP+24]`, original game caller `[ESP+20]` |

Optional wrapper result span84CCC9,5B `83 C4 1C 3B C3` can pair nonzero playing IDs with the wrapper request. A subsequent callback/removal observation is warranted if the16-entry count rises or refuses. Input-string wrappers84CD90/84CDE0 are available for mapping names; their direct near-call inventory is empty, so indirect use or other numeric callers remain possible. The existing16 known direct callers of84CC90 and the larger84CDC0 numeric-wrapper caller inventory are saved with the evidence.

All listed spans were byte-verified and contain no relative control-flow instruction. Full incoming-branch, bridge/stack/FPU-state, startup and rollback checks remain implementation work before installing any new hook. Do not treat these candidate signatures as a tested ASI. Keep hot observations bounded and avoid walking event indices from the monitor thread or calling middleware from a hook.

The useful user comparison is the same input/actions on one working and one silent car, with event/object/caller and lookup/refusal outcomes. If a silent car's real events have valid definitions and nonzero playing IDs without the16-entry gate firing, investigation moves downstream to action dispatch, switches/parameters, voice/stream construction and mixer behavior.

x32dbg could inspect these same branch outcomes and object records, but is not required to obtain them. No debugger was launched/attached, no game was run, and no ASI or game behavior was changed during this trace. Only the authorized executable, supplied log and focused existing files were read; no other game folders or external data were searched.

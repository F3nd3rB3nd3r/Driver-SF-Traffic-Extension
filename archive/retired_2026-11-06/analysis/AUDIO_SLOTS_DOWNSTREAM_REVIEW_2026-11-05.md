# Audio slot downstream review — 2026-11-05

The reviewed global list paths support increasing their configured capacities beyond 255: allocation sizes, configured maxima, active counts, stored record pointers and traversal bounds are 32-bit. No global byte-sized index, fixed 255-iteration traversal or 255-bit mask was found in these paths. This is a bounded static finding, not proof that every audio consumer is compatible or that these lists are saturated during missing car audio.

Exact input: the authorized `E:\Games\Driver San Francisco Save Bug Test\Driver.exe`, SHA256 `C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`. Evidence is in `audio_slots_downstream_2026-11-05/`; the earlier initialization/gate inventory is in `audio_slot_audit_2026-11-05/`.

## Global storage and consumers

| Manager | Storage | Reviewed downstream behavior |
|---|---|---|
| `[01D7184C]`, InitSettings+8 | DWORD max at +4; two pointer arrays with headers +8 and +14 | `C6B160` allocates each array as max*4. `C6B420` derives count from end-begin and compares against the DWORD maximum at `C6B439`; at the ceiling it can reuse an existing record. `C6B2B0` updates/removes using pointer/end bounds; `C6B1E0` destroys using those bounds. |
| `[01D71850]`, InitSettings+4 | begin +4, end +8, DWORD capacity +C and maximum +10 | `C6B740` allocates max*4. `C6B920` compares the DWORD count to +10 at `C6B92E`, returning null before allocation on full. `C6B790` updates through end pointers, `C6B830` removes by pointer search and swaps the last entry, and `C6B7C0` destroys the list. |

At `C76742`, the second manager's allocator returns a record pointer stored directly in playback-side object field +140 at `C76747`; it is not reduced to an 8-bit global index. Generic append `C6B3C0` uses DWORD vector fields and growth helper `CA8970`; generic removal `C6B8D0` searches to the end pointer.

Thus there are three global arrays across two managers, rather than one universal audio list. A configured maximum of 2048 fits the inspected initialization and consumer arithmetic. A future test should change the initialization settings before native allocation, so storage and limits agree; changing only a comparison would not establish that. The common default-setting instruction supplies both 255 values, so a patch there changes both managers unless the settings are handled separately.

## Independent limits inside individual records

| Owner | Limit and exact check | Meaning supported by inspected code |
|---|---|---|
| Record managed through `[01D71850]` | Eight attached users: `C6B9C6 CMP EDX,8`; `C6B9C9 JAE C6BA15`, returning 2. User vector +24/+28/+2C, byte count +31. | Attached object pointers share the record. `C8CDC0` computes three interpolated coordinate components and writes the same components through each attached object's +38 pointer, at offsets +18/+1C/+20 (`C8CE60..C8CE76`). This supports a shared position/path role; exact class names remain inferred. It is not established as eight sound categories or eight voices per vehicle. |
| Record managed through `[01D7184C]` | 255 attached users: `C6B589 CMP BYTE [EDI+39],FF`; branch at `C6B592` returns result 1B on full. Increment at `C6B5A3`; removal decrements at `C6B295`. | A true byte-counted per-record user constraint, separate from the global list count. Record update `C8C3D0` computes time progress, consistent with a transition/fade role. |

The eight-user refusal is checked before the duplicate search. At caller `C767A1`, result 2 clears the playback-side +140 record pointer (`C767AB`); that alone does not demonstrate that audio playback is stopped. These findings do not map the limit to a vehicle or establish a mute cause.

Record path setup `C8CAE0` also uses 16-bit fields for authored path counts/indices (+14/+16), distinct from the manager's global slot count. These should not be widened merely to expand global capacity.

## Interpretation and next experiment

More distinct resident vehicles could increase global record demand. A vehicle's engine, tires, horn, damage sounds and camera-dependent audio cannot be counted one-for-one against the eight entries from current evidence: multiple playback objects can share state, and categories may use different records.

The separate eight- and 255-user limits do not by themselves prevent 2048 global records. They also would not be raised by changing the global defaults. No existing probe measures global list saturation, eight-user refusals, or per-record 255-user refusals, so none is proved responsible for the remaining silent vehicles. A focused measurement of these counts/refusals/reuse events would distinguish them before changing their behavior.

Scope: reviewed central initialization, insertion, update, removal/destruction, record attachment and the identified playback-side allocator caller; not every indirect caller or every audio subsystem. No new ASI, build, install, game run, debugger or web search. Existing 150 requests /160 graphics slots, ED/EF, indexed262144 and prior audio test artifacts remain unchanged.

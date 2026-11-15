# TrafficEditor: support for adding vehicle IDs and larger rosters

Requested after publishing the unlimiter changes. The R3/current package/source/
tests/reports were pushed to origin/main as48c70ca2e6cfebd9edddbb6597e7cb031eda0731.
The reviewed87-file commit excludes dumps, raw logs, failed R2 packages and
generated local evidence. Local HEAD and origin/main match. The R3 build verifier
passes against its existing19-process results and exact staged ASI hash.

TrafficEditor source reviewed read-only at:
`E:/_DSF Mods/ai repos/DriverSF_TrafficEditor_Source`.
No root/nested AGENTS.md was found in the scoped source inventory. No editor
source, project, asset or save changes, build, GUI/game/debugger launch or
installation occurred. This is a source/format audit, not implemented expansion.

## Confirmed current capabilities

The editor is an in-place value patcher. It can replace a16-bit vehicle ID in an
existing spool slot, modify a Float32 weight, and transfer weights between IDs
that already exist in the same table. It cannot insert new probability rows,
create a larger spool group, or save a rebuilt/grown resource.

| Area | Evidence in source |
|---|---|
|Fixed input|Core/TrafficDocument.cs13..21: exact0x433000 length, fixed offsets,191 vehicles,208 tables,58 settings and0x5F8 table stride|
|Fixed weight parsing|TrafficDocument.Load computes WeightData+table*stride and requires metadata.first=table*191/count191|
|No new weight IDs|Models/TrafficModels.cs82: WeightRow.VehicleId is get-only; MainWindow.xaml412 makes the ID column read-only|
|No UI insertion/deletion|MainWindow.xaml193..194 disables both DataGrid row operations|
|Existing-ID transfer only|MainWindow.xaml.cs308 explicitly says both IDs must already exist and rows are not inserted|
|Value-only writer|TrafficDocument.PreparePatch clones original bytes and writes only existing spool IDs, weights and total sums|
|No structural save tracking|HasChanges counts modified values; fixed Slot/FileOffset/Original fields and UI subscriptions do not model add/remove changes|
|Same fixed parser on save|SaveAs reloads the temporary output through Load, so a correctly grown file would currently be rejected too|

Merely enabling DataGrid's add-row setting or changing VehicleCount to1000 is
insufficient. Every data offset, count, writer and validation assumption must
agree. Replacing a spool ID with a new mod ID does not add that ID to probability
weights; a new probability ID cannot currently be entered through this editor.
If a vehicle mod only replaces resources under IDs already present in these
191 rows, existing weight editing may suffice. New numeric IDs absent from those
rows require structural support. Actual Forza assets/catalog were not inspected.

## Additional parser omission in the current supported AI file

Read-only comparison used the already-authorized Save Bug Test media AI file:
4,403,200 bytes, SHA256 `434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13`.
Its root is at file0x1080, and declared arrays are:

| Array | Declared count | File offset |
|---|---:|---|
|Traffic settings|59|0x280A00|
|Spool IDs|1369|0x281080|
|Road/category groups|319|0x248160|
|Setting routes|18821|0x281B40|
|Weight metadata|208|0x2DDA80|
|Weight rows|39728|0x2DE440|

The editor begins its metadata window at0x280A18 and reads58 records. Its leading
index word is actually the trailing selector of the previous native record;
the following hash/start/count fields address native settings1..58 correctly.
The parser consequently omits native setting0, hashE3517AD3, whose groups contain
17+17+17=51 IDs. This explains the advertised1318 IDs/77 nonempty groups versus
actual1369 IDs/80 nonempty frequency groups. The setting is populated, not an
empty padding record. Its human-readable name is not established in this audit.
Existing value patches address their intended fields; the omission does not by
itself establish corruption of saved files. A new serializer must preserve and
expose this first setting instead of dropping it.

All208 weight tables currently have191 distinct IDs (range1..303, the same ID
sequence in every table), matching the editor's legacy weight layout. Independent
checks of its fixed spans/uniqueness/Float32 totals pass on this file. The old
verification/verify_resources.py reaches its original-weight-specific124->208
scratch example and fails because it expects the destination to have0 weight;
this AI has modified weights. That failed fixture assumption is not evidence of
an invalid chunk. The input was never written.

## Recommended implementation boundary

1. Replace fixed offsets/counts/stride with validated resource metadata. Include
   every declared setting and variable per-table first/count values. Preserve
   static name associations as optional labels, not the authority for what exists.
2. Model structural edits explicitly: add/import IDs, remove entries, detect
   duplicates/invalid IDs, preserve existing IDs and weights by default, and track
   collection changes for saving. Provide bulk application to selected tables so
   users do not edit1000 rows across208 tables manually.
3. Implement a resource serializer. Rebuild or relocate changed table payloads;
   update table first/count/total and root array pointer/count fields. Audit and
   update CHNK/AIRT sizes, alignment and serialization/fixup data for changed
   locations. Preserve unrelated resource content. The exact container/fixup
   rewrite is the next unproved format dependency; no safe append-only recipe
   was established here.
4. Independently reparse outputs, require no-edit byte-identical round trips,
   exercise existing and expanded row counts including1000, and check that
   unrelated blocks remain intact. Then live-test with actual available assets.

For use with R3's AssetWeights mode, the first expansion milestone can focus on
probability rows: R3 derives the native-required plus routed positive-weight ID
union itself. It is not necessary to enlarge every native spool group to1000
just to request those weighted models. Preserve native mission-required IDs.

If native spool expansion is also offered, its first/count fields are16-bit.
Blindly concatenating80 disjoint1000-ID groups exceeds a16-bit start-index range;
reuse/deduplication and explicit format checks would be needed. Probability
metadata has32-bit first/count fields, while routing table selectors are bytes.
Adding more vehicles to existing tables does not require more probability tables.

An R3 compatibility preview should compute the exact required per-setting union
from native IDs plus all routed positive-weight rows, using the same semantics
as scripts/audit_generic_traffic_asset.py. Report that count and needed INI
headroom, rather than predicting universal MiB-per-car demand. The editor does
not install/register car models or audio, and structural1000-row files would
still need the appropriate ASI configuration and live resource validation.

Evidence summary: `analysis/traffic_editor_extension_audit_2026-11-15/`.

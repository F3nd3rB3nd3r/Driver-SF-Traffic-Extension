# TrafficEditor 0.2: existing probability tables can grow

User authorized implementation in the sibling TrafficEditor source. This
supersedes the feasibility-only status in TRAFFIC_EDITOR_EXTENSION_AUDIT_2026-11-15.md.
The Unlimiter source, ASI/INI, game executable, installed assets and saves are unchanged.

## Implementation

The editor now adds missing numeric vehicle IDs or ranges to selected/all existing
probability tables, assigns the chosen initial weight, removes records, and keeps
existing IDs/weights when adding duplicates. Dirty tracking includes structural
edits and new rows. Save/reopen uses native variable counts/pointers; it is no
longer restricted to 191 rows or 0x433000 bytes. Native spool groups remain
replacement-only: R3 AssetWeights can derive the added model requests from weights.

The parser includes the omitted native setting0 and resolves labels by hash.
The supplied file has59 settings/80 nonempty frequency groups/1369 native IDs.
Profile labels retain the historical213 observations across135 of208 tables;
the catalog does not control file shape. There is a static R3 capacity report
using the native-plus-routed-positive union, not a MiB/car estimate.

Structural saves append a rebuilt row array inside AIEX and update root160/164,
per-table first/count/total, CHNK ancestor sizes and sibling offsets. Later edits
resize the appended tail. Opaque payloads, descriptor attributes and names are
preserved. Name length is descriptor byte9, version byte8, alignment exponent
byte10; names follow child payloads. An initial serializer draft accumulated
padding between those labels and siblings; a regression check caught this and
the final serializer replaces only the old alignment contribution. Ten reopened
incremental edits now equal one bulk edit byte-for-byte.

The native loader0040ADD0 calls0040A7C0;0040A89A adds the AIEX base to the
existing root+164 pointer. Thus relocated weight rows use an existing relocation
field; rows themselves contain IDs/floats, not new pointers. AIRF is an opaque
route-finding payload, not a fixup table. Scoped native output is retained in
analysis/traffic_editor_extension_impl_2026-11-15/.

## Verification and delivery

- Release WPF/.NET10 build and framework-dependent publish succeed.
- CoreChecks:57 checks pass, including original/value-only/structural round trips,
  selected/all-table growth,208x1000 rows, ID65535, removals, repeated growth/shrink,
  aliases, dirty state, protected source, invalid IDs/weights/totals and bad files.
- Independent Python validates bounds/selectors/totals and compares opaque payloads,
  all chunk labels and descriptor attributes. Catalog/theme/event checks pass.
- Expanded208x1000 fixture is6,066,176 bytes, SHA256
  `0570f90c612dc840b7cc1ba57de02cf524e0d34f608fe99cd12e1804e00c0a17`.
  R3 audit and editor capacity report agree on maximum959 requested IDs: original
  enabled/native IDs plus809 new positive IDs; zero-weight rows are excluded.
- Authorized source asset remains4,403,200 bytes, SHA256
  `434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13`.
-14 changed/new editor source/docs/test files and7 runtime-package files are
  listed in deployment_manifest.json. Original tracked source bytes are preserved
  in original_source.zip and verified against original_source_manifest.json.
  The apply script preflights hashes and refuses unexpected concurrent files.
- Workspace packages:dist/TrafficEditor_v0.2/. Sister deployment target:
  `DriverSF_TrafficEditor_Source/publish/expandable-tables-v0.2/`.

No game, debugger or WPF GUI interaction was run. Expanded-file game behavior
still needs user testing with matching installed models and Unlimiter settings;
1000-row serialization is not1000-car runtime validation. Runtime package needs
the .NET10 Windows Desktop Runtime(x64), already installed on this machine.

No Git commit or push is part of this editor implementation request.

# TrafficEditor 0.3: bulk weights and vehicleStats ID import

The user asked to assign probability weight1 to all vehicle IDs declared in
vehiclestats.lua across19 selected Chapter7-related tables. Existing v0.2 only
bulk-added absent rows and deliberately preserved existing weights, so it could
not perform that operation. v0.3 adds it without changing the existing add-only
behaviour.

## Delivered behaviour

- **Import vehiclestats.lua** opens a user-chosen file and fills the existing ID
  box with direct numeric keys of the literal `vehicleStats` table. `[62]` is an
  ID; numeric Model/Manufacturer fields, nested keys, comments and strings are not.
  Lua is never executed. Decimal/hex keys and duplicate-key deduplication work;
  terminal `return vehicleStats` is allowed. Computed/invalid keys, malformed
  input and additional table references/mutations are refused rather than guessed.
- **Set weights** assigns the entered weight to listed existing rows across the
  selected tables. The default checked option also inserts missing IDs. It can
  be unchecked for existing-only updates. Weight0 disables records without
  removing them. Unlisted IDs and unselected tables retain their values.
- The operation preflights all target Float32 totals before any mutation, reports
  updated/added/skipped counts, emits one document change notification, and shows
  the maximum routed Unlimiter roster after the action.
- **Add missing IDs** remains add-only. Imports do not mutate probability tables
  until an action is applied. All-tables scope stays explicit; Ctrl/Shift table
  selection is preserved. The capacity button now has a version-neutral label.

## Specific source and independent example

The explicitly named file was located by filename within the previously supplied
Lua tree: `E:/_DSF Mods/LuaSF_new/Resources/ScriptsLuaScripts/vehiclestats/vehiclestats.lua`.
It contains146 unique direct keys, range62..303, including non-selectable records.
This is the extracted vanilla source used as a fixture, not proof that it is the
user's intended modified vehicleStats file; the picker lets them choose that.

A scratch copy of the previously supplied
`SS Test/media/san_franciscoAI_vanilla_and_forza_cars.fchunk` supplied exactly19
expanded tables. Setting all146 imported IDs to1 updates2697 existing weights,
adds0 rows and preserves every unrelated row/table. The result remains4,741,120
bytes and saves/reopens successfully. Independent byte comparison finds5561
changed bytes, all in the selected weight fields or their total metadata.
Opaque payloads, all chunk labels and descriptor attributes are preserved.

Scratch output SHA256:
`ef0d582f8523942462312f55650d129e155689e5e7911c40bfb00b96a07ab514`.
Independent Unlimiter routing audit agrees with the editor that the largest
required union becomes280 IDs: existing Forza weights were not zeroed. This
exceeds the current160-request configuration. The user must use the capacity
report for their chosen inventory and configure compatible request/audio-model/
retained-reference budgets before installing newly enabled rosters. No actual
game AI or INI was edited.

## Verification and preservation

-89 core checks pass, including all57 existing serializer checks and the actual
  Lua/19-table fixture, bulk scope, missing-row modes, idempotency, zero weight,
  import exclusions and preflight atomicity for a later-target overflow.
-Release WPF/.NET10 build and framework-dependent publish succeed with0 errors
  or warnings. Existing catalog/theme/XAML event checks pass. No GUI interaction
  test or game launch occurred.
-Current editor source, including its uncommitted v0.2 work, was preserved as25
  files in `analysis/traffic_editor_bulk_2026-11-15/source_before_bulk.zip`, verified
  against source_before_manifest.json. Deployment preflights old/new hashes and
  refuses unexpected existing files. All15 source/runtime file hashes were
  verified after copying8 changed/new source/docs/test files and7 package files.
-Sibling source is updated. Ready build:
  `DriverSF_TrafficEditor_Source/publish/bulk-weights-v0.3/DriverSFTrafficEditor.exe`.
  v0.2 remains available in its separate publish directory.
-Workspace ZIPs/source update/README/validation are in `dist/TrafficEditor_v0.3/`.
  The runtime package needs the installed .NET10 Windows Desktop Runtime(x64).
  No game data, Lua source, ASI, INI, save or Unlimiter production source changed.
  No Git commit/push was requested or performed.

Evidence, source backup, deployment manifest, core test output and independent
checks are in `analysis/traffic_editor_bulk_2026-11-15/`.

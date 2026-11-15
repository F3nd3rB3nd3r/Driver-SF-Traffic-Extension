# USER CAP R4: retirement/work-list storage during traffic replacement

The user authorized implementation after the supplied SS Test dump and Driver.log
identified an unexpanded retirement list. This supersedes R3 for testing that
specific failure; it does not supersede R3's recorded default150 gameplay results.

## Crash evidence

Dump: `E:/Games/Driver San Francisco SS Test/Driver_20261115_161352.dmp`, PID31908.
Driver.log reports loading Race Takedown Activity2, activity vehicle212, traffic
change Chapter7 -> Chapter3. ExceptionC0000005 at0083B17D reads invalidF7261220
while allocating a retained-vehicle node. The owner is1B82E1A8 and its work-list
header atowner+5A4=1B82E74C is corrupted.

The captured memory contains62 linked12-byte retirement nodes in slots52..113
beyond the native50 slots, with backlinks/next links at exact12-byte intervals.
All captured IDs match predicted removals for the previously supplied AI:152
Chapter7 IDs ->18 Chapter3 IDs, intersection6,146 removals and12 additions.
That prediction assumes that supplied AI was active; the small dump does not
include the AI root or the whole retirement header. The requested Unlimiter
31908 log was not supplied before implementation, so its configuration and first
boundary events remain unverified. The dump/native-byte overwrite mechanism is
independently reproduced by the new fixture.

SS Test Driver.exe SHA256 is the supported
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
The dump includes Unlimiter R3, DriverNGHook and dsfforza modules. Native callers
005A7BE8 ->005A7E30 ->0083B170 corroborate the work-list allocator failure.

## Production change

`V29sTrafficQueuesCore.inc` validates and extends three native pools:

| Queue | Owner header | Native first/end | Stride |
|---|---|---|---:|
|retireIds|338|344 /59C|12|
|retainedVehicles|5A4|5B0 /998|20|
|vehicleOperations|9A0|9AC /D94|20|

Each native pool holds50 nodes. Total capacity is the existing
`WideWorkNodes() = max(512, ModelSlots + ReplayRetainedReferences)`; the new
configuration check verifies its20-byte geometry. No request/model default,
INI key or probability list changed.

The helper checks active/free chains, duplicate/unissued nodes, sentinel and
backlinks, fresh/recycled cursor provenance, counts and external guards before
native allocation. It redirects only a fresh inline-end cursor with no recycled
nodes. It never overwrites the native sentinel or drops recycled nodes. A cold
native reset of a used list creates a fresh owner/kind generation; old storage
remains alive. Directories have no historical eight-owner/four-arena ceiling.

CapacityRetireEntry/CapacityTrafficEntry integrate the helper within existing
preserving detours. Legacy overlapping LISTA/LISTB macros remain disabled.
Capacity observations recognize valid external cursors; they no longer treat
every >50 traffic insertion as an unprevented stock-pool overflow. New queue
startup/status/fatal records give actual capacities and integrity counts.
The installer refuses startup if these mandatory hooks are unavailable.

The active one-project solution now builds R4 Release/Win32 to a separate build
directory. R3 distribution bytes and its INI are preserved. The package uses the
existing user INI, with no replacement INI that might reset a152-capable setup.
Logs are named `DSFV29s_usercap_r4_<pid>.log`. The generic log analyzer recognizes
the new three-queue startup/status records while retaining its R1/R3 checks.

## Verification

`scripts/verify_traffic_queues.py`: six isolated processes pass. The fixture maps
the supported executable into its own process; it never calls the game entry
point. It demonstrates the old51st-entry overwrite, then calls the real native
005A7D50 merge through the actual retirement detour for146/18/146/0/152/0/146
cycles, preserving adjacent headers. It executes the native20-byte appenders
after the production guard helper; their hardcoded retail caller filter is
checked separately in the compiled production bridges because the fixture maps
the image at20000000 instead of400000.

All queues are filled/recycled/refilled to capacity, reset at the same address
with new backing, and checked against invalid cursor/guard/alias inputs. The
full-capacity fatal path preserves a neighbouring canary; hook preflight rejects
a changed signature before partial installation. More than eight owners pass.
Fixture request settings150/160/500/1000 derive512/512/1010/2010 queue nodes.

`scripts/verify_wide_limits.py`: the inherited19 process scenarios pass, including
full cold installation at150/700/1000 and customized settings, native layouts,
requests, audio directories, parked paths, completed maps and expected refusals.
`verify_wide_limits_build.py` verifies the new production PE and140 retained
baseline files. `verify_traffic_queues_build.py` pins all fixture dependencies and
traces production bridge calls through linker thunks to the guards, including
FX/flags/register preservation and unchanged trampoline/caller destinations.

Release build succeeds with27 inherited unused-diagnostic/exported-entry warnings
and0 errors. Final ASI is282112 bytes, SHA256
`24cac4fdb996f8aa9f75b492508d0ca8df0f3bcde2f2085d1b3fab70ff2e2aed`.

Evidence: `analysis/traffic_queues_r4_2026-11-15/` and
`analysis/wide_limits_r4_2026-11-15/`. The11-file pre-edit snapshot is ZIP/hash
verified and includes the pre-existing uncommitted handoff changes.

## Remaining verification

No game/debugger launch, game deployment, editor change, game/AI/model/audio/INI
or save write, Git commit or push occurred. The user must replace R3 with R4 as
the only Unlimiter, retain the Forza mod/DriverNGHook and their existing INI, and
repeat Chapter7 -> Race Takedown Activity2 -> Story/menu/re-entry. The supplied
AI requiresRequestCapacity>=152 (160 previously suggested). Other pool budgets,
Forza asset validity, actual rendering/audio/READY residency and general gameplay
limits are not established by these offline queue tests.

# REGMAP_REUSE_R2: reproduced startup refusal and correction

The user reports that R1 prevents startup while the game starts without it.
The two latest inspected TEMP logs, PID11908 and36608, both contain
baselineStatus=-16, downstream baselineStartupRefused markers and a final
CALLBACK_FATAL. There are no runtime samples or REGMAP_START. In DllMain,
-16 is set when DiagSRegMapInstall returns false and the initial patches are
rolled back; the downstream callback guard then intentionally exits. This is
a startup refusal, not gameplay vehicle-definition map exhaustion.

Copies of those logs are in `regmap_reuse_r2_2026-11-14/`. Baseline and callback
analyzers were applied to run11908. The callback analyzer initially assumed
periodic samples existed; it now explicitly handles fatal-only startup logs
without inventing duration/audio/runtime observations. Its normal runtime
analysis path remains unchanged. The exact R1 failing preflight predicate was
not logged live, but its deterministic protection-region defect is reproduced
below using the production installer.

## Root cause and test gap

Constructor signature0086BFA0..0086C010 (113 bytes) crosses the86C000 page
boundary. The installer temporarily changes only page86B000 to PAGE_EXECUTE_
READWRITE. The adjacent page86C000 stays PAGE_EXECUTE_READ, so VirtualQuery
reports separate regions. Production Readable intentionally accepts only a
span fully inside one readable region. R1 called it for the entire constructor
span during precommit, incorrectly rejecting these otherwise readable bytes
after changing its own page protection. It restores protection and returns
false without applying the hooks.

R1's fixture used a simplified Readable stub and initially RWX fixture memory,
so it did not model this split. The test gap was in that fixture; offline
success did not establish live startup success.

Before changing production layout code, the fixture was changed to the exact
production Readable implementation using real VirtualQuery and RX code pages.
R1 then failed its expected-success installer assertion, saved as
`r1_startup_reproduced.log`. The pre-fix layout and reproduction fixture are
saved beside it. Reproduction sources included production dependencies; the
saved files and old R1 artifact hash document the old mechanism, not an
independent standalone fixture package.

## Correction

RegMapReadableSpan checks bounded chunks ending at4KiB page boundaries with
the existing Readable predicate. RegMapMatches and RegMapEqual use this
coverage check before comparing **every expected signature byte**. This
accepts adjacent readable RX/RWX regions while still rejecting guard/no-access/
uncommitted memory, overflow and modified bytes. The shared production
Readable function is unchanged; map allocation, generation, node and queue
logic are unchanged. No protection or resource gate was disabled.

REGMAP_INSTALL_FAIL now records the specific installer stage/site before a
refusal. Successful startup includes startupCheckRevision=2. The log prefix
remains DSFV29s_regmap_reuse_. The parser recognizes installer refusals as
failures even if startup/status records are absent.

## Verification and artifact

The strengthened fixture demonstrates that the old full-span Readable call
fails across the real split while R2's full signature check passes. It rejects
an inaccessible second page, guarded second page and corrupted second-page
bytes. It installs/restores/reinstalls the actual two-site patch, checks both
pages return to RX, and retains allocation/protection/flush/rollback tests.
The verifier requires the fixture Readable body to exactly match production.

All eight-owner,100 same-address reconstruct, native linked-map, recycling,
bounds, diagnostic and ABI tests pass. The active solution was built once,
Release/Win32. Compiled hooks/tables/markers and the complete inherited
verification chain pass. Parser tests pass, including fatal-only startup
handling. Build warnings are the existing C4018/C4505/LNK4216; no new RegMap
warning,0 errors. R1 and REPLAY_PAUSE artifacts remain unchanged/hash checked.

Staged artifact:
`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.asi`.
253440 bytes; SHA256
`8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa`.
Evidence: `regmap_reuse_r2_2026-11-14/verification.json`, `fixture.json`,
`parser.json`, `package.json`, build/fixture logs and compiled disassembly.
Guide: `dist/MAIN_150/REGMAP_REUSE_R2_TEST_2026-11-14.md`.

No game run, debugger session, installation or asset/recording edit was made.
Live startup is the immediate next check; repeated Story/Replay/menu/control/
normal-exit validation follows. R1's startup regression is reproduced/fixed
offline; final game startup and lifecycle stability remain unverified live.

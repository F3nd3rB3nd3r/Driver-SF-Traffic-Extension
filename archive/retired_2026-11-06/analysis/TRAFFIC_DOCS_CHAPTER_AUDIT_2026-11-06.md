# Traffic documentation audit for vanilla chapter independence

## Result

The supplied docs already explain the main setting/copy path and most hashes. The current installation also already has the common 150-ID picker roster across all 125 named Single Player table indices. Chapter independence primarily needs the working preload substitution generalized at the correct named-setting boundary, with native setting/frequency behavior and lifecycle preserved. There is no evidence here requiring another capacity increase or another general hash-discovery campaign first.

This is a documentation and static asset audit, not a new ASI or a cross-chapter gameplay result. The current PENDING512_R1 baseline and its 150 requests / 160 slots / ED+EF / audio / indexed / cleanup / pending fixes remain unchanged.

## Scope and evidence grades

- Indexed all **53 top-level `.md` files** in `E:\_DSF Mods\ai repos\DriverSF_TrafficResearch_VS\docs`, preserving file hashes, sizes, headings and line numbers in `traffic_docs_chapter_audit_2026-11-06/docs_inventory.json`.
- Read the core routing, hash, provenance, roster and loading documents, and relevant sections/updates of the long historical handoff and experiment histories. This does not mean every historical debugger command or evidence claim was independently revalidated.
- Listed only the authorized game's `Install` and `media` directories. Directly decoded three specifically named AI assets in `media`. The Install listing was orientation only; no Install payload was analyzed.
- Did not open the research project's `data`, `build`, `native`, `tools`, captures, ZIPs, Lua source trees, or other paths referenced by the docs. Did not open `docs/traffic.zip`. The Forza audit document was classified as deferred; its referenced archive was not accessed.
- Did not read/disassemble Driver.exe anew, attach to a process, start the game, install anything, web-search or build an ASI. Exact code addresses below are document-supported landmarks for the previously identified executable, requiring signature verification before a future patch.
- Existing current-workspace START_HERE/NEXT_ACTION and the validated pending512 handoff supply the present baseline. Historical external claims about old failures are not silently converted into current defects.

Use three evidence labels: **asset-checked today**, **document-supported historical/code finding**, and **not yet demonstrated**. No raw runtime registry was recomputed because those external files are outside this review's scope.

## Asset-checked today

| Asset in authorized media directory | SHA-256 | Finding |
|---|---|---|
| `san_franciscoAI_original.fchunk` | `DA5B1CA3E4E51B6C8D937ED0EB2CBEB2A54048C9ACA5A3E5A26F8704B1F0B936` | Matches the original used by the early structural reports. |
| `san_franciscoAI.fchunk` | `434E1A516FB88A91FDAA02E2C35AC44F460C804FF312C04232A5BED011D8DB13` | Current installed edited asset. |
| `san_franciscoAI_uniform_150_candidate.fchunk` | Same as installed | Byte-for-byte identical to the installed file. The old claim that this candidate is not installed is superseded. |

Each file has 4,403,200 bytes. The independent decoder follows root-relative pointers from file root `1080`, checks their spans, validates all table row counts/IDs, finite nonnegative weights, and stored totals, and rebuilds the active route matrix.

- **59 native setting records**, 28 bytes each, at `280A00`. The old 58-record export boundary was four bytes early and omitted slot 0.
- **54 named hashes** reproduce the exact seeded CRC32, including meaningful trailing spaces. These are the 53 names in `SETTING_HASHES.md` plus `Chapter 2 God` (`5956218A`) documented elsewhere. The five remaining original names/modes remain unresolved.
- **319 road groups**, **18,821 route records**, **208 probability tables**, **191 fixed ID rows per table**.
- **80 active setting/frequency profiles**, producing **280 distinct static triples** across **149 routed table indices**. These are static routing results, not new observed picker events.
- **45 named non-Online settings / 51 active frequency profiles** use **125 table indices**. Every one of those tables currently contains the same 150 positive IDs, each at weight 1, total 150.
- All native lists for those 51 profiles contain **12–22 IDs**, and every one of those IDs is inside the common 150-ID roster. Thus their union with that roster remains 150; this examined data does not require a larger request list for chapter coverage.
- Settings, native lists, group records and the entire route matrix are unchanged from the original. There are **37,699 changed bytes**, all confined to weight fields and totals for precisely those 125 tables. No other bytes differ. This also preserves the original road/placement data; weight-vector descriptions are the version-sensitive part.
- The **17 named Online table indices are unchanged** and disjoint from those 125. However **19 edited indices are also referenced by unnamed hashes**: the Chapter7-like hash `9CB17769` and challenge-like hash `CC9F35B5` are affected. Do not describe the edit as proven exclusive to Single Player across every native hash.
- Original membership consistency (positive picker IDs contained in the native request) holds for all 80 profiles. Installed data violates that consistency in **55 profiles: all 51 named SP profiles plus four unnamed profiles**. This is an asset mismatch, not 55 independently observed runtime failures. The ASI currently compensates for the Chapter7 normal-load case.

`setting_profiles.csv` contains all 80 profiles, exact names, hashes, frequencies, native ID lists, table indices and mismatch counts. `settings_reference.md` is a readable 59-setting reference. Full decoded original/installed evidence and summary are alongside them.

### Main setting reference

Counts below describe native lists in the file before ASI substitution; every listed profile currently has the common 150-ID positive picker union.

| Exact setting | Hash | Frequency / native requested IDs |
|---|---|---|
| Chapter 1 | `AE88E83B` | 0 / 18 |
| Chapter 2 | `3781B981` | 0 / 18 |
| Chapter 3 | `40868917` | 0 / 18; 1 / 17 |
| Chapter 4 | `DEE21CB4` | 0 / 18; 1 / 18; 2 / 17 |
| Chapter 5 | `A9E52C22` | 0 / 18 |
| Chapter 6 | `30EC7D98` | 0 / 18 |
| Chapter 7 | `47EB4D0E` | 0 / 18 |
| Chapter 8 | `D754509F` | 0 / 17 |
| Free Drive | `9BD98676` | 0 / 14 |
| Exposition | `EF19694A` | 0 / 18 |
| Epilogue | `409E0517` | 0 / 15 |

An entry named Chapter 8 does not by itself identify which installed mode/package requests it. Free roaming does not necessarily request the literal Free Drive setting. Follow actual setting/frequency events rather than chapter folder names.

## Reusable loading and routing findings

`NATIVE_150_PATCH_MAP.md:30–34`, `SPOOLING_CAPACITY.md:49–65`, `STATIC_TRAFFIC_ROUTING.md:57–64` and `ID216_REJECTION_TRACE.md` provide a concrete code-backed chain:

1. Named setter `0076FDF0` looks up the setting hash, writes active hash/frequency, reads that profile's native ID list, stages it in its stock 40-DWORD local area, and calls shared request writer `005A79F0` at **`0076FE95`**.
2. Two other direct writer calls are documented at **`0066E5F3`** (Lua table request) and **`0074BA58`** (another native path). All three obtain the same civilian destination from root+`214`.
3. The shared loader processes the requested set and reconciles/releases IDs not retained. Repeated batches do not establish a persistent union. Skipping release was historically tested and did not yield a reliable working expansion.

This refines the next task: verify those exact instruction boundaries against the supported executable before changing code, then distinguish the named-setter entry from other writer callers. A five-byte call at `0076FE95` implies return address `0076FE9A`; this is a candidate discriminator, not a new runtime observation or complete indirect-call audit.

The existing Chapter7 strategy substitutes the larger list **after** the named setter staged its small native list. Generalizing that approach can leave the stock 40-ID local untouched: the present native profiles fit within it. Earlier blanket statements that the 40-ID frame must be enlarged apply to making the setter itself copy a 150-ID asset list. They are not a prerequisite for post-staging substitution. Do not change the file's native counts to 150 as a shortcut.

`UNPACKED_SCRIPT_TRACE.md` supplies important integration constraints:

- Common `requestVehicleSettings` / `isChapterTrafficSpooled` performs change detection using **name and integer frequency**, waits for the spooler and traffic-loaded state, and handles mission pots separately.
- A mission can request another chapter's setting. Documented examples include Chapter3 content requesting Chapter4/frequency1 and Chapter4 content using Chapter4/frequency2. Chapter folder/current progression chapter cannot replace actual setting identity.
- InterestingVehicleManager regions, traffic events, mission pots and civilian weighted traffic are separate paths. A setting switch alone does not reload all chapter-specific managers/regions. Traffic-event rates are not the integer setting frequency.
- Dares inheriting the active civilian setting are a supported hypothesis, not a universal runtime rule.

These source statements have documented bytecode provenance but the external Lua files were not reopened in this audit. None justifies replacing the game loading flow or every generic request.

## Outdated claims and qualifications

| Claim encountered | Current treatment |
|---|---|
| 58 native settings / exported record index is the native selector | Superseded even within the research docs. Asset-checked result is 59 at the corrected boundary. |
| The hash list's 53 names is the complete native inventory | It is the original observation snapshot. Current documented/asset-checked names are 54 of 59, with five original-name gaps. |
| 135/208 tables, 73 remaining, or the 220/139 and 227/145 unions describe current research coverage | Dated snapshots. The later Chapter1/table50 report states **234 runtime triples / 149 tables**, leaving **46 static-only triples**. The 213-row master remains separately frozen. These runtime counts were not recomputed here. |
| Chapter1/table50, 148, 169, 171, or alias tables 144/204–207 still need first discovery | Later supplied docs report those captures. Do not repeat them as missing table-index discoveries. |
| Later plugin CSV candidates can simply be added to the verified count | `PLUGIN_CSV_AUDIT_20261028.md` reports Chapter2/tables38/51 candidates with incomplete context. They are not an authorized master promotion or independently recomputed latest total. |
| More driving with known settings should uncover all remaining 59 table slots | All 149 indices reachable through the decoded active route matrix already have documented picker coverage. The other 59 slots have no active route here; resolving them is not needed for known-chapter support. |
| Current Chapter1/Chapter5 positive rosters are stock 13–18 IDs, active named SP union is 96, or ID123/212/216 weight statements apply unchanged | Valid only for the identified original asset where stated. Installed weights are the common 150 across all 125 edited tables. Chapter1 civilian-vs-interesting ID disjointness is likewise an original-asset conclusion, not a current extended-roster property. |
| The equal-weight candidate is uninstalled/untested | Its bytes are now installed, and the current user testing is later. Historical playthroughs still do not prove all 150 drawable at once or every mission safe. |
| Current software is blocked at 35/40/45/50/64 or needs the old V4/V12/V20 prototype next | Those are useful stock geometries/history. Current V29s has coordinated extensions and a tested normal-load substitution. Follow the current project's PENDING512_R1 baseline. |
| EF map is still unexpanded and needs its first probe | Superseded by later Nov2 updates in AI_HANDOFF and the current ED+EF baseline. Keep the 45-node geometry/ownership findings, not the old deployment instructions. |
| The old memory snapshots showing released=0 exclude cleanup pressure | They are momentary old-run samples. Latest current baseline recorded a 52,176 cleanup peak, beyond native 32,768, and validated its expansion. Do not conflate the separate pending-handler map with resource-reference arrays. |
| A completed-map count, handler count, READY-looking state, picker result or allocator slot count proves drawable residency | Still unsupported. Keep these layers separate; the original 150 simultaneously READY/drawable goal is not formally established. |
| All old callback/rendering/Pacer faults are now explained by the latest pending or cleanup fix | Unsupported retrospective attribution. The faults belong to different builds/runs and first writers were not always captured. Current user-reported issues passed recent testing, while old mission-preview identity questions lack a current reproducer. Include mission model identity in later validation rather than declaring every old cause resolved. |
| AI_HANDOFF is ordered newest first / NEXT_EXPERIMENT is the current task | AI_HANDOFF starts with Nov1 but appends Nov2 work around lines2227–2332; its latest V29p staging is itself older than the present baseline. NEXT_EXPERIMENT retains many historical instructions. Read date/build/evidence, not just title or first paragraph. |

## Per-document disposition

**Reuse** means relevant evidence remains useful, not that every statement is newly verified. **Historical** means preserve the experiment record, not execute its next-step commands. Large historical files were reviewed by relevant sections rather than by replaying all evidence references.

| Document | Disposition for current task |
|---|---|
| `150_CAPACITY_STATUS.md` | Historical stock bounds; its current-status/next-prototype claims are superseded. |
| `AI_CHUNK_INSPECTION.md` | Reuse corrected layout and original provenance; replace installed weight/count assumptions. |
| `AI_HANDOFF.md` | Historical chronology plus useful corrections; latest entries are also at the bottom. Not the current project's handoff. |
| `ARCHITECTURE.md` | Reuse distinct setting/picker/spooling layers, CRC convention and build-specific anchors. |
| `BROWSER_HANDOVER_2026-10-29.md` | Historical V20/48 capacity and browser packaging; not current instructions. |
| `CHAPTER1_INTERESTING_VEHICLES.md` | Reuse separate manager path; original-weight disjointness does not apply to current uniform tables. |
| `CHAPTER5_TABLE_ROLES.md` | Reuse road/table routing footprints; positive counts and weights are original-specific. |
| `CHAPTER7_INTERESTING_VEHICLES.md` | Reuse region-vs-setting distinction and qualified source attribution; archived zero-weight statements need asset identity. |
| `CHAPTER7_NATIVE_PRELOAD_TRIAL_2026-11-01.md` | Reuse post-staging normal-load design and fingerprints; trial status/build/test sequence is superseded. |
| `CRASH_20261025_TRAFFIC_CAPTURE.md` | Historical old manual-switch crash; not a current blocker or proof of present fault cause. |
| `CURRENT_150_RESIDENT_HANDOFF.md` | Historical pilot/pinning investigation; residency distinctions remain useful. |
| `DATA_PROVENANCE.md` | Reuse evidence grading, cumulative-snapshot caution and asset/source distinctions. |
| `DSF_ENHANCED_ARCHIVE_AUDIT_2026-11-01.md` | Historical comparison and overlapping-hook caveat; no external archive access or integration needed. |
| `EF_RESOURCE_MAP_EXE_AUDIT_2026-11-02.md` | Reuse exact native layout; unexpanded-EF/current-next-probe status superseded. |
| `EXPERIMENTS.md` | Historical replacement mod and mission-sensitive IDs; not current 150-roster policy. |
| `EXTRA_RESOURCE_PILOT.md` | Historical shelved pilot; no reason to restart its deployment sequence. |
| `FINDINGS.md` | Useful overview, but coverage totals, unresolved prerequisites and installed-weight implications are stale. |
| `FORZA_ARCHIVE_AUDIT.md` | Deferred by user scope. Only document relevance considered; no archive followed. |
| `GRAPHICS_LIMIT_CAUSAL_TEST.md` | Preserve old class-pool causal evidence and distinct-pool meanings; old 48-capacity next task superseded. |
| `ID216_CHAPTER7_EXPERIMENT.md` | Historical wrong request-anchor lesson; do not infer raw-request membership from map co-presence. |
| `ID216_REJECTION_TRACE.md` | Reuse request-copy caller and precise ASI-guard-vs-native-rejection distinction. |
| `ID216_REPEAT_CAPTURE.md` | Historical same-model before/after and installed-vs-original weights; old probe status superseded by its own updates. |
| `LUASF_NEW_ARCHIVE_TRAFFIC_AUDIT.md` | Reuse documented source provenance and negative name search; runtime union predates Chapter1/table50 follow-up. |
| `MEMORY_LIMITS_NEXT_STEPS_2026-11-01.md` | Historical sampled counters; do not treat old low samples or old V29b feature inventory as current. |
| `MODE_TABLE_OVERLAP.md` | Reuse named-family classification and unnamed-sharing caveat. Original list/weight membership no longer holds in edited asset. |
| `NATIVE_150_PATCH_MAP.md` | Important reusable source of the three writer callers and reconciliation; old unimplemented architecture/status is superseded. |
| `NEXT_EXPERIMENT.md` | Historical accumulated experiment instructions, not today's plan. |
| `PLUGIN_CSV_AUDIT_20261028.md` | Candidate/provenance qualification only; no external CSV/log/cleanup commands followed. |
| `RECONCILED_COVERAGE.md` | Snapshot history; top233/149 is older than later234/149 report. Master remains distinct. |
| `RESIDENT_ARRAY_45_REDIRECT.md` | Reuse 45-pointer geometry and lifecycle lesson; old sidecar size/test status superseded. |
| `RESIDENT_NODE_45_REDIRECT.md` | Reuse 45-node geometry and ED/EF variant distinction; historical build instructions superseded. |
| `ROAD_LOCATION_MAPPING.md` | Reuse structural road/group interpretation; unrelated to expanding model residency. Current non-weight bytes unchanged. |
| `ROAD_PLACEMENT_PARAMETERS.md` | Reuse placement-vs-table distinction. Do not label unclassified floats as verified density/speed. No placement edit needed. |
| `RUNTIME_CAPTURE_20261025_PID10716_HASH11DF09F4.md` | Historical manual numeric-routing validation; alias is not original name, and initial coverage totals are dated. |
| `RUNTIME_CAPTURE_20261025_PID22224_FIVE_SETTINGS.md` | Historical six added triples; superseded coverage by same-PID Chapter1/table50 follow-up. |
| `RUNTIME_CAPTURE_20261025_PID24752.md` | Preserve same-session observations and no-double-counting; early missing targets were captured later. |
| `RUNTIME_CAPTURE_20261026_CHAPTER1_TABLE50.md` | Latest explicit documented reviewed coverage aggregate234/149/46; no new verification of its external raw files. |
| `SETTING_HASHES.md` | Reuse exact names/CRC; 53-name observed snapshot is not full59-setting native inventory. |
| `SPAWN_MODEL_TRACE.md` | Reuse picker-ID vs assigned-model fallback evidence; old V20 condition is not a present failure measurement. |
| `SPOOLING_150_FEASIBILITY.md` | Reuse stock layouts; 96-positive/catalog and no-expansion-yet conclusions are original/historical. |
| `SPOOLING_CAPACITY.md` | Reuse per-request-vs-residency distinction, three callers and reconciliation. Post-staging strategy avoids widening stock setter frame. |
| `STATIC_TRAFFIC_ROUTING.md` | Central reusable corrected layout/formula; old missing-capture status cells are historical. |
| `TRAFFIC_CAPTURE_PLAN.md` | Historical discovery plan with stale targets/totals; not needed for chapter independence. |
| `TRAFFIC_ELIGIBILITY_VS_RESIDENCY.md` | Central reusable explanation of eligible-but-unavailable models and current asset identity; current broad behavior is later than old V20 samples. |
| `UNIFORM_TRAFFIC_WEIGHT_PLAN.md` | Reuse row-width/total rules and settled roster; candidate-not-installed and unimplemented capacities are outdated. |
| `UNPACKED_SCRIPT_TRACE.md` | Central reusable loading, cache, frequency, mission-pot and cross-chapter override analysis; external sources not reopened. |
| `UNRESOLVED_HASHES.md` | Five original names still unknown; pretest/static-only paragraphs must be superseded by later live updates in same file. |
| `UNRESOLVED_HASH_CONTEXT_AND_SP_COVERAGE.md` | Reuse mode uncertainty and dare inheritance qualification; similar tables do not prove producer/mode. |
| `V12_POOL_ONLY_COMPARISON.md` | Historical causal comparison, not another ASI installation task. |
| `V15_OFFLINE_HOOK_AUDIT.md` | Historical native pool-overflow/ABI findings; no present stopped process to resume. |
| `V29B_CALLBACK_AUDIT_2026-11-01.md` | Historical callback/teardown failure evidence; first-writer gaps remain historical gaps, not current regressions. |
| `VEHICLE_ENTITY_LIFECYCLE.md` | Reuse instance/model/config-object distinction; no fixed entity count or model-capacity formula established. |
| `VEHICLE_IDS.md` | Reuse191-row universe vs model catalog; original all-zero/eligibility statements need original SHA. Four special roster entries remain part of the previously chosen150. |

## Concrete next implementation target

The docs reduce the required research; they do not make an unguarded hash bypass appropriate.

1. Verify the documented named-setter call site/signature and current bridge entry ABI. Use caller identity, exact supported asset profile `(hash, frequency, full native list)`, and the existing startup/admission guards. An active hash by itself cannot distinguish all three request sources.
2. Generalize post-staging substitution to the supported named Single Player profiles while preserving the game's requested hash, frequency, road routing, mission pots, unload/reset calls and normal cleanup. Keep150/160 and all validated fixes. The asset audit supplies all51 profile fingerprints and confirms their required IDs fit the roster.
3. Define handling for the four unnamed profiles whose shared tables are already uniform150. Leave original names/modes unresolved; neither blindly classify them as SP nor silently claim they are unaffected. A data-based policy could recognize their exact uniform roster without inferring mode, but this needs an explicit implementation decision and validation.
4. Audit repeated same/different-profile changes, request ownership, stale-context cleanup and finite owner/arena tables. Preserve native lifetime rules rather than resurrecting the historical release-skip/pinning experiments.
5. Verify decision logic for the full profile matrix and negative cases (other writer callers, empty/reset requests, bad source/hash/frequency, unsupported assets). Then test real early/middle/late chapters, setting/frequency changes, mission vehicle identity/preview behavior, reload/menu/exit and audio/visibility. Include an actual chapter load, since console setting changes do not reload all chapter managers.

An INI, larger counts, speculative mode names, a new broad probe, and another weight rewrite are not prerequisites. New runtime instrumentation is conditional on unresolved caller/owner semantics after focused verification.

## Reproducer

`python scripts/audit_traffic_chapter_assets.py --docs "E:\_DSF Mods\ai repos\DriverSF_TrafficResearch_VS\docs" --media "E:\Games\Driver San Francisco Save Bug Test\media" --output analysis/traffic_docs_chapter_audit_2026-11-06`

Outputs are in the current writable workspace only. The analyzer does not import or execute scripts from the research repository and does not write to the documentation source or game assets.

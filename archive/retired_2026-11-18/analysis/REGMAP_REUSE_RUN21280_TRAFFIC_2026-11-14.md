# REGMAP_REUSE_R2 run21280: reported Mini-only civilian traffic

Superseded by `REGMAP_REUSE_DUMP21280_FORZA_MISMATCH_2026-11-14.md` on the
active-installation and validation-cause questions: dump is SS Test/Forza;
loaded rows match its different AI file and lead to native missing-ID fallback.
The Save Bug Test disk check below was not the active installation's asset.

User reports all civilian traffic became Mini Cooper S on the first Story
load with R2, with the same files that previously had proper vehicle picking.
The latest regmap log in TEMP was copied once into
`regmap_reuse_run21280_2026-11-14/run21280_snapshot.log`. The source continued
growing afterward; this is a partial-session snapshot, not a final exit log.
No process memory was read, debugger/game launched or ASI/source built/changed.

## Evidence

All startup markers succeed, including REGMAP_START startupCheckRevision2.
This confirms R2 passed startup in this captured run; it does not certify
correct civilian selection or overall stability.

Chapter1/frequency0 at32875ms and Chapter7/frequency0 at36484ms both report
assetMismatch. The two accepted requests contain18 IDs, not150. Both rosters
are complete native18 rosters and reached native readiness; neither had tracked
body-draw coverage in this snapshot. There is no expanded150 request.

Last periodic sample207594ms (~3m27.6s): one map owner, one construction,
41 new-key insertion checks,13 recycled uses,0 external redirects,0
reconstructions/reattachments,0 faults/allocation failures. Thus the dynamic
external map backing and reset paths have not been exercised in this snapshot.
The constructor observer and insertion guard have executed. Absence of map
faults is not proof that the new code cannot affect behavior elsewhere.

Parked cache has18 candidates,362 picker calls/239 selections/123 empty
outcomes,7 unique selections,0 invalid/guard failures. This is a different
picker from moving civilian traffic; it does not disprove the reported road
traffic symptom. Audio recorded198 successful setups/0 failures/misses/full.
Model/profile key15 accounts for most setup observations; these counters do
not independently identify the visually observed vehicle or its model ID.
No fatal appears in the captured prefix. Both required analyzers ran; their
structural checks pass, but the user's vehicle-selection result is a failure.

## Disk-versus-loaded-data distinction

Installed media/san_franciscoAI.fchunk is4403200 bytes with SHA256
`434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13`, exactly
the profile header's expected adjusted asset hash. Using the existing read-only
asset decoder, Chapter1/7 frequency0 request IDs, route-selected tables,
table totals and all191 row IDs/weight bits were checked against the profile
fingerprint. They match. Selected tables each contain150 positive weights.
Evidence: `disk_ai_check.json` beside the captured log.

The assetMismatch reason is emitted by ChapterDataMatches on **loaded memory**,
not a disk hash comparison. It can mean an unreadable span, unexpected counts/
pointers, setting/list mismatch, routes or probability rows/totals. The log
does not separate these predicates. The broad single-region Readable checks
remain a possible issue, but there is no captured AI memory/protection evidence
yet to prove that explanation. R2's new page-aware check applies only to map
code-signature verification, not this unchanged chapter checker.

The uniform150 disk weights plus an effective18 request are a material
selection/residency mismatch. This is a lead, not proof that every pick fell
back to a particular model or that it is the sole cause. Do not disable the
asset guard, edit probabilities, blame changed files or increase limits from
this evidence alone. Do not identify Mini as a first/default entry without
tracing the native selection and resolution path.

## Next required evidence

Capture a full dump while the Mini-only Story state is present, retaining the
matching log. Inspect loaded AI root/pointers/counts/readability and the two
settings' actual routes/weights, registry map/native definitions and the
selection/fallback state. This can distinguish a validation failure from
changed loaded data or selection state without another probe ASI first.
Use a distinct dump filename, e.g. Driver_mini_only_21280.dmp, so earlier
replay dumps are retained. No further build or patch proposed as established
until the failure predicate/selection mechanism is supported.

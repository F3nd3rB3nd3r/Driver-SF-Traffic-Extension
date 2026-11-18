# Run21280 dump: Forza selection IDs versus vanilla preloading

The supplied Driver.DMP matches PID21280 and shows a concrete selection/
definition mismatch consistent with the user's Mini-only traffic. It does
not show definition-map capacity exhaustion. No source, project, ASI, asset
or recording was changed, and no game/debugger was launched for this analysis.

## Identity and scope correction

Full Task Manager dump:1065510238 bytes, PID21280, timestamp1794613155,
flags0x421826,51 threads, no exception stream. SHA256:
`ef9b0d8bf0a41c74a3a67285a00f4915bb7038688e9645e0955cdf3ec10f0a1d`.

Its module paths identify **E:\Games\Driver San Francisco SS Test**, not the
Save Bug Test installation whose AI file was checked in the earlier log-only
analysis. The user clarified that SS Test is the Forza-car installation and
offered to return to Save Bug Test. Following that clarification, only SS
Test's Driver.exe and media/san_franciscoAI.fchunk were additionally read.
No plugin configuration/source files or other folders were followed from
dump paths. Other module presence alone is not attribution evidence.

SS Test Driver.exe matches the supported executable SHA256
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
Its AI file is4403200 bytes, SHA256
`957d7eb62d2007c1c79d61fd741d6cf7974da1296631cc3983bda26c25ecf55e`.
The earlier checked Save Bug Test file is also4403200 bytes but has the
expected vanilla-test fingerprint
`434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13`.
Thus that earlier disk check did not identify the active Forza asset. This
does not imply the user changed files within their SS Test installation.

## Exact loaded-data validation failure

Dump AI root is2BBE0100. All six count fields match the expected structure,
and all six arrays pass the **original single-region production Readable**
test using captured MINIDUMP_MEMORY_INFO. This is not the page-protection
failure fixed in R2's code-signature installer.

For Chapter1/frequency0 and Chapter7/frequency0, the native18-ID request
lists, setting selector, group routes and selected table metadata match the
known profiles. The first differing check in ChapterDataMatches is the
first row's weight in the first selected table: model1 has weight1.0 in
this loaded asset, while the vanilla fingerprint requires0. Subsequent
row IDs/weights differ substantially as well. Example table31 has190 row-ID
differences and72 weight-bit differences from the vanilla fingerprint.

All7 Chapter1 tables and16 Chapter7 tables have150 positive rows and totals
150.0. Their captured row bytes match the corresponding SS Test file bytes
exactly. They do not match Save Bug Test's rows. This proves assetMismatch
is a real fingerprint/content difference in the captured state, rather than
an unreadable AI span or an assumed corruption introduced by the map hook.

The existing chapter guard therefore preserves the native18 requests. It
does not preload the150 IDs selected by this different Forza probability set.
Disabling the guard alone would still request the wrong hardcoded vanilla
150 roster and is not a justified compatibility fix.

## Why the same car can be returned for every traffic pick

The native definition registry is at owner1BA13D60/map1BA13FB4. It has27
active nodes and1 recycled node, all within the original45-node inline area.
The registered keys are:

```text
15,62,123,129,136,138,158,170,173,178,185,196,199,201,203,205,
206,249,265,269,271,280,282,286,287,291,298
```

**None** of the150 positive IDs in **any** of the23 selected Forza tables is
in this captured registered-key set. This is stronger than simply observing
18 requested vehicles: no currently weighted choice can directly resolve
through the captured definition registry.

The unchanged native virtual lookup at0086B9E0 searches for the requested
key. If absent, it walks the sorted list for an eligible record whose
reference field is not positive, returning that record's value. The first
eligible record in this dump is key15 at1BA14200, value281D2A20; its native
definition at32477140 also has definition ID15. Reference field is0.
This fallback therefore returns ID15 for the missing weighted IDs in this
captured state, consistent with the Mini Cooper S reported by the user and
the repeated audio profile/key15 observations. The display name was not
independently decoded from localization data.

The registry owner's vtable entry for this method still points to0086B9E0.
The full lookup fallback, equal-key search0086BDC0, removal0086BE90 and
node allocator008CCAA0 bytes match the supported Driver.exe. This is a
native fallback path, not an ASI loop explicitly choosing the Mini.

## Map correction status and log correlation

Private map bookkeeping agrees with native state: one owner/generation1,
28 issued inline nodes,27 active+1 recycled,0 external issued/attached,
both29DE51AC guards intact. Counters:49 insertion checks, no reconstructions,
redirects, reattachments, faults or allocation failures. Thus this workload
does not exercise the expanded backing or the lifecycle correction's reset
path. It cannot certify or disprove the original long-session exit fix.

A later read-only copy of the same growing TEMP log was also analyzed with
both required analyzers. This snapshot is11077365 bytes, SHA256
`dac7d4d08b7d8d27fd9411f6a1360d2eac14616a057bc6baa878a3fe28c69381`,
last tick1011109ms. It still has only the same two18-ID requests, one map
owner/no expansion/reset/fault and no fatal. This is not a claimed final
normal-exit log; the source session may continue. Structural checks passing
does not override the user's incorrect vehicle-selection outcome.

## Recommended next step

Return to **Save Bug Test** with the expected vanilla-test AI file and only
R2 as the traffic ASI for the outstanding vanilla startup/lifecycle test.
Confirm Chapter events expand to150, variety/audio/rendering, then repeat
Story/Replay/menu/control transitions and normal exit.

Forza compatibility is separate work: selection IDs, requested/spooled IDs
and definition/audio/resource IDs must agree for that installation's roster.
The present chapter profiles and canonical150 list remain vanilla-specific.
Do not just raise memory limits, bypass fingerprints, change probabilities
or force key15 away. No additional pool limit is implicated by this dump.

The dump is a snapshot, not a trace of every past pick. It supports the
current missing-ID/fallback mechanism; it does not establish when this Forza
asset was installed, which earlier ASI was used successfully or exclude every
possible interaction with other modules. A controlled vanilla R2 comparison
remains required.

Reproducible analysis/evidence:
`regmap_reuse_dump21280_2026-11-14/analyze_dump.py`, `dump_reader.py`,
`dump_analysis.json`, `baseline.json`, `callback.json` and the log snapshot.
The reader uses read-only mmap, captured memory regions and recovered WOW64
contexts; it does not attach to or modify the running process. Driver.DMP is
now PID21280; do not run older PID8944-specific analyses against it.

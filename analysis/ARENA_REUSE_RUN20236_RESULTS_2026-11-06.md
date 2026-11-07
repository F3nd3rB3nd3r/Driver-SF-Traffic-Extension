# ARENA_REUSE_R1 run20236: Challenge/menu/reload verification passes

The supplied log and user report support keeping ARENA_REUSE_R1 as the current working baseline for the tested Challenge-transition and menu/reload workload. The run crosses the former ninth-setting failure point, continues through33 expanded requests and completes two unloads without a reported guard failure. No new patch or capacity increase is indicated by this result.

User reports approximately eight Challenges via mixed free-drive/garage entry, main-menu return and Story Mode reload, then four further Challenges and a normal main-menu exit. Individual Challenge names and exact audible/visual/FPS observations were not supplied. Traffic-setting events are not one-to-one Challenge counts.

## Input and reproduction

-`dist/MAIN_150/DSFV29s_arena_reuse_20236.log`,13353001 bytes,56346 lines.
-SHA256 `558D273E70CD8B08172AA729BBD2DEED5F5F0C462D1A91F34B7E4F6A4AE56603`.
-Last recorded tick1608578ms:26m48.578s. Final newline is complete.
-Only this supplied log, current telemetry definitions and the existing local setting-name catalog were read. No old raw-log scan, build, ASI modification, game run or debugger.

```powershell
python scripts/analyze_callback_log.py dist/MAIN_150/DSFV29s_arena_reuse_20236.log --output analysis/arena_reuse_run20236_2026-11-06/summary.json
```

Evidence: that summary plus `lifetime_verification.json` in the same directory. All recorded arena/callback and inherited structural checks pass. The additional lifetime checks confirm monotonic arena counters,33x150 scratch checks,per-owner arena accounting,pending generation census and balanced empty completions.

## Arena repair was exercised

| Measurement | Result |
|---|---:|
| Expanded traffic-setting requests |33|
| Reconcile/spool checks |33|
| Scratch checks |4950 =33x150|
| External scratch arenas |0 throughout|
| Scratch active peak before insertion |149, expected for150 appends|
| Persistent spool arenas / distinct traffic owners |2 /2|
| Spool redirects / observed issued peak |2 /150|
| Same-owner constructor-reset reattachments |0|
| Arena invalid / guard / allocation errors |0 /0 /0|
| All fatal records, including legacy exits |0|

Startup reports the intended `ownerReuse`,no fixed owner limit,`expandedFrame150`,zero external scratch and13 frame patches. The first traffic owner1B2544B8 receives one arena and performs24 setting requests without consuming more arenas. After the menu/reload transition, owner2F620048 receives a second arena and performs nine further requests. This is the intended persistent-owner accounting.

The recorded `spoolReuses=0` does not mean reuse failed: it counts reattachment after a native header reset at the **same address**. This run retained each owner's backing through ordinary native updates/recycling and used a different address after reload. Same-address reset reattachment remains fixture-tested.

The ninth event occurs at149547ms, Challenge traffic/f0, and is followed by continued monitoring and24 further setting requests. The final setting event at1594344ms is also followed by14.234s of log output and the final unload. This differs from the three earlier failed logs, which ended on their ninth admission. Together with the native fixture's old ninth-use exhaustion reproduction, this strongly supports the scratch-policy correction as the relevant repair. The earlier runs' actual process exit codes were not captured.

## Setting and lifetime coverage

All33 requests were admitted as150 with no source/asset/profile refusals. Observed traffic profiles:

| Setting | Frequency | Events |
|---|---:|---:|
|Chapter1|0|2|
|Chapter7|0|16|
|Chapter6|0|1|
|Challenge traffic|0 /1 /2|2 /1 /4|
|Exposition|0|7|

These are setting names; they do not prove loading actual Chapter1/6 saves or all chapter-manager variants. User confirmation supplies successful Challenge/garage/free-drive navigation. No claim of universal all-chapter coverage or150 simultaneously drawable vehicles follows from the request counts.

Both menu/unload lifetimes exercised pending512:

| Generation | Map | Created | Peak nodes | Walks begin=end | Empty completion |
|---:|---|---:|---:|---:|---:|
|1|1AFE607C|1396281ms|154|34|1396812ms|
|2|2FDDAECC|1604281ms|154|32|1604625ms|

308 insert attempts=stores,2 redirects beyond native64,66 walks begin=end,final current/depth0 and154 recycled nodes. No invalid/full/allocation/guard error. This directly covers menu teardown that was absent from the earlier short Challenge-crash logs.

Cleanup processed all662563 appends (141 replacements+662422 releases),with155056 drain starts=ends. Peak52157 crosses the original32768 boundary twice; final current/depth0,zero overflow/invalid/guard/drain mismatch. Indexed262144 status/head checks remain healthy,150 requests/160 graphics slots are retained and fileStatus remains0.

The log ends4s after the second pending-empty completion. It does not capture the final OS exit code; the user reports a normal exit. The telemetry is consistent with that report.

## Callback and audio interpretation

Callbacks:1006 stores=1006 native removals,982 recycled inserts,30584 polls,final current/depth0 for every observed group. Maximum active/issued peak is9 in group19. No redirect or64-boundary crossing occurred. Thus callback integrity passes, but the extra callback512 capacity was **not exercised** by this live run. Do not attribute this success to a callback capacity increase or raise it further from these counts.

Audio registry:300 stores and300 remove-hook observations across the two150-model loads;2310 profile hits and bank-ready checks,zero missing/full/disabled/guard error.1265 setup successes and1045 native setup failures remain recorded. Their exact reasons and audible impact are still unverified; structural success does not establish full audio playback. Shared64MiB pool peak logged usage5474624 bytes,with zero threshold-pressure samples.

The last audio registry `current=1` is the last **pre-removal** observed count: `AudioModelEntry(kind=2)` records the native count before its decrement. With300 removal observations and no later registry observation,it is not proof that one profile leaked. Do not interpret this cached gauge as a post-unload live count.

## Decision

Keep ARENA_REUSE_R1 as the working baseline for extended vanilla traffic and this tested transition/menu/reload sequence. The repeated silent-exit regression is resolved in the supplied run. No further ASI or arbitrary limit change is needed for this verification. Remaining separate work includes decoding audio setup failures,parked-car fallback,and broader actual-chapter/longer gameplay validation if prioritized by the user.

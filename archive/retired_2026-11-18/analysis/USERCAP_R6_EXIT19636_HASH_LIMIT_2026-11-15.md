# R6 PID19636: explicit exit at the configured hash arena limit

The supplied `dist/USERCAP_R6/DSFV29s_usercap_r6_19636.log` ends with:

```text
DSFV29s_LEGACY_FATAL reason=hashExternalExhausted exitCode=00000029 tick=99953 owner=01732BF0 cursor=0185A6B0
```

`LegacyExit` logs the reason and calls `ExitProcess(code)`. This accounts for
the user's Alt+F4-like disappearance without an access-violation dump. The
generic analyzer correctly reports `no_fatal=false` and an overall failed
session; other telemetry checks passing do not make this a gameplay pass.

## Progress and the failure boundary

The accepted configuration is requests280, model slots290, audio models384,
global audio4096, shared audio128MiB, reader2MiB, retained512, hash extra45000.
R6 stream revision2 installs successfully, H802, four queues, eight hooks and
362 layout operands. It survives two manager/scheduler generations, with one
destruction each, then requests the asset-derived280-ID roster at88593ms.

Last stream sample at95609ms records286 active handlers/802, queue0 projected
peak262, queue2 peak174 and fourth queue peak37; guards/failures remain0.
This exercises the previously insufficient256 dispatch capacity and the fourth
queue fixed after R5. It does not prove the whole R6 lifecycle or full residency.

The last sampled readiness checkpoint is226/280. Audio registration reaches
244 successful stores later in the log; registration is not a READY/drawable
certificate. The session exits during loading, before all280 are observed
READY. There is no demonstrated audio-pool or model-registry exhaustion.

`V29AcquireFreshHashNode` explains the precise reason. The native arena has
45000 nodes, stride24, from owner+20000 through owner+127AC0. The supplied
cursor0185A6B0 equals owner01732BF0+127AC0 exactly. Once that fresh-node cursor
is exhausted, the external arena issues nodes up to `g_limits.hashExtra`.
Its next reservation at/above the configured45000 extra entries takes the
logged `hashExternalExhausted` branch. Invalid cursor/owner/storage failures
have distinct reasons and are not what this log reports.

The final sampled `hashExtraLifetimeIssued` metric is27405; the fatal branch
is later and records exhaustion after those samples. That older sampled
value cannot override the exact exhaustion check. Counts represent lifetime
fresh-node issuance, not a proven count of90000 simultaneously live vehicles
or resources. The native recycled-node path remains intact.

## INI-only next test

The existing R6 ASI already reads `HashExtraEntries` at startup. Inspection of
the shipped R6 machine code confirms both allocation bytes and the exhaustion
comparison use the same runtime field `g_limits+44h` (preferred VA10045054):
allocation multiplies it by24; the helper compares the external ordinal to it.
No new ASI or native layout extension is needed for this boundary.

Staged `dist/USERCAP_R6/HASH180K/DriverSF_Unlimiter.ini` explicitly sets
`HashExtraEntries=180000`. Every other effective setting matches the original
R6 package/current test. Native45000 remains unchanged, giving225000 total
fresh-node backing positions. External backing changes from1080000 bytes
(about1.03MiB) to4320000 bytes (about4.12MiB), an additional3240000 bytes
(about3.09MiB), excluding allocation granularity/other storage.

180000 is controlled headroom, not a measured minimum or guaranteed280-model
budget. Static geometry and the existing signed byte bound accept it. Retry
the same280 AI with the same R6 ASI, restart the process, and confirm the new
startup log reports hashExtraEntries180000. If another limit is reached,
identify it from the next log rather than raising all settings together.

No game files, ASI, source or existing packaged configuration were changed;
the alternative INI is staged separately. No build, game/debugger launch,
commit or push occurred. Evidence is in
`analysis/usercap_r6_exit19636_2026-11-15/`, including analyzer output,
compiled hash allocation/exhaustion disassembly and configuration manifest.

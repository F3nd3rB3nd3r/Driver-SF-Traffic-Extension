# USER CAP R3: fix for the R2 Story-load crashes

Both supplied dumps were analyzed offline, without a debugger/game launch.
R2 is superseded by USER CAP R3. The exact R2 artifacts, supplied dumps and
R1 fallback remain preserved. No game-folder, asset or save changes occurred.

## Captured failure

| Dump | PID | Exception | Native instruction | Completed map |
|---|---:|---|---|---|
|Driver_20261115_134431.dmp|34196|C0000005 read00000000|005A209F MOV EAX,[EAX]|1B4D909C|
|Driver_20261115_134503.dmp|31312|C0000005 read00000000|005A209F MOV EAX,[EAX]|1B4ED6D4|

Both exception contexts have EAX=0 and ESI=298 (012A). Each completed-map header
has active=0, recycled=0 and cursor=header+12: an empty map's first native slot.
Call-site005A209A calls007D7270, which is detoured to MapInsertBridge; the native
caller immediately dereferences the returned output-pointer address at005A209F.
The exception-context stack also contains0059BAFB/005A41CD/005A4E0B candidates.
These are identified code-address candidates, not a fully unwound symbolic stack.
The thread-list context is inside dump capture; the exception stream carries the
actual fault registers and was used for this diagnosis.

Only DriverNGHook and the R2 traffic ASI appear as ASI modules. The module image
size/timestamp agree with staged R2. These small minidumps do not capture the
ASI's g_mapExtra/g_pkg0 globals, so the exact live external-buffer address and
UnsafeBoundary counter cannot be read. There is no matching R2 log in the two
supplied workspace log directories. This limitation is retained rather than
inventing a live pointer value.

## Root cause and reproduction

R2 changed g_mapExtra from static DLL storage to VirtualAlloc storage. The
completed-map guard still used:

```
free == 0 && cursor >= externalBegin && cursor >= externalEnd
```

The second comparison subsumes the first; this tests address order rather than
whether the cursor belongs to the external arena. When the external arena lies
below the package, a valid native inline cursor passes this exhausted check.
The hook sets UnsafeBoundary=1 and returns0. The native caller expects a nonnull
output address and dereferences it, producing the captured fault. The check does
not examine the incoming resource ID, so298 is not evidence of a bad model asset.

A deterministic isolated native test mapped package35000000 and external storage
10000000. Old R2 rejected insertion0 with UnsafeBoundary=1 while the native map
was empty, exactly the header state in both dumps. The test checks the null result
before dereferencing it, so it exits with a test failure instead of recreating the
access violation. `r2_reproduction.exe`/JSON retain this old-code reproduction.
The exact external-pointer value in either live minidump remains unavailable;
the reproduced source defect matches the shared crash path and state.

The earlier R2 tests covered initializer setup and pending/callback/definition
insertion, but did not exercise completed-map insertion with external storage
below the native package. That missing regression case allowed this bug through.

## Fix and scope

The WIDE_LIMITS completed-map branch now accepts aligned fresh cursors belonging
to either the native64-node inline range or the actual configured external range.
It redirects only at the native boundary. An exhausted, misaligned or foreign
fresh cursor logs and exits explicitly before a native write, instead of returning
an unusable null result to this caller. Recycled-node behavior is unchanged.
The historical non-wide branch is preserved.

This changes pointer classification and failure handling. INI numeric values,
request/traffic selection, model IDs, capacities and asset contents are unchanged.
R3 changes the binary/output/log prefix to keep the failed R2 artifact identifiable.

## Verification and package

Release/Win32 active solution builds with0 errors. Inherited optional unused
probe/exported-DllMain warnings remain. All19 isolated process cases pass,
including the prior16-case matrix and three completed-map refusal cases.
The nine18..1000 request tests now each additionally run native completed-map
insertion with the external allocation below and above the native package, fill
all64+configured-extra slots, recycle/reinsert nodes and reconstruct an empty map
at the same address. First resource ID298 is included. End/unaligned/foreign
cursor cases stop with29F3 and intact tail canaries. Existing full cold installers,
wide native ordinals, replay/parked/audio/request fixtures and the R1 log analyzer
regression still pass. The R2 package hashes and original140-file fallback
snapshot are independently rechecked by the verifier.

Candidate: `dist/USERCAP_R3/DriverSF_Unlimiter_USERCAP_R3.asi`
276480 bytes; SHA256 `8532a9783361e27dd0bad3509cd0da7028a8846abcb78089e6ace1d67474919c`.
INI/guide/package manifest are alongside it. Default INI comments are updated;
its parsed values are identical to R2. The package has not been installed.

Evidence: `analysis/usercap_r2_story_crashes_2026-11-15/` and
`analysis/wide_limits_r3_2026-11-15/`. Read-only mmap parsing preserves dump bytes;
Microsoft dumpbin supplies native/compiled disassembly. No external files beyond
the authorized supported executable and supplied workspace evidence were needed.

Next: replace the enabled R2 traffic ASI with R3, keep the same150 settings and
AI file, and retry Story loading. Copy the new usercap_r3 log into the workspace.
If Story loads, repeat the broader lifecycle regression. Offline reproduction
and passing tests do not yet prove the live crash has been resolved; larger
rosters remain live-untested.

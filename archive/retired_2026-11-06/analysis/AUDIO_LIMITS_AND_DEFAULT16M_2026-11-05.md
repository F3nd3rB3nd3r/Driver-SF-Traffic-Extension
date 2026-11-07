# Audio limits: verified 90% pool-pressure gate and controlled 16 MiB adjustment

The user requested identification and adjustment of audio limits. New static
analysis identifies a concrete mechanism consistent with run4188: **playback can
be stopped or refused when the shared default pool exceeds 90% usage, before a
Malloc failure occurs**. The supplied log exceeds that threshold during loading
and remains above it. This materially changes the earlier interpretation of
zero allocation failures. Individual gate calls/kick decisions were not logged,
so this is a strong causal candidate, not yet a confirmed audible fix.

The user also clarified that the roughly two surviving Shift effects can fail
later in the same state. Those are audible effects, not queue occupancy. Clearing
or lost reload was their hypothesis; R1 did not log unloads. The pressure path
found here acts on playback objects and does not itself prove bank unloading.

## Exact mechanism

Exact authorized `Driver.exe` SHA256 remains
`C79987B0548F7130337C3F295C8FE269E120CEDDAD55C1159E6F5CBC6AA26616`.
Only this executable and focused existing workspace evidence were needed. No
other game folder, external SDK, web search, plugin inspection or debugger was
used. New disassembly/reference evidence: `audio_limits_2026-11-05/`.

1. `0084A62E` loads float `0.8999999761581421`, bits `3F666666`, from `00DD6CD4`.
   Stores at `0084A654/0084A659` copy it into platform settings+20 and common
   InitSettings+10. The common saved threshold is `01D71874`; platform threshold
   is `01D71900`. This differs from the middleware's default threshold1.0.
2. `0084A64D` overrides common InitSettings+0C with `00599999` (5872025 requested
   bytes, about 5.6 MiB). Saved request `01D71870` feeds `00C522A0 -> 00C99D90`,
   which creates the shared default pool retained in `00FDD3B8`.
3. `00C66910` calls `00C66530` with that pool and threshold. If not pressured it
   checks the platform pool `00FDD3C0` with the corresponding platform threshold.
4. `00C66530` calls `GetPoolMemoryUsed` at `00C99B60`, which returns descriptor
   usable bytes+48 and used bytes+44. It returns true for **used/usable strictly
   greater than threshold**. A threshold>=1 disables this particular check.
   The pressure value0.9 is exactly `0xE66666 / 2^24`; no decimal rounding is
   needed to classify recorded integer usage.
5. When either pool is pressured, `00C6694F..00C669EF` scans existing playback
   objects, skips already marked objects and selects an eligible priority
   candidate. It calls `00C75B20(1)` on a selected object, or returns false when
   an eligible candidate cannot be selected under its priority conditions.
   `00C75B20` marks flags at object+E6 and calls `00C75550` with an immediate
   transition structure. That routine drives stop/release paths, including
   `00C74C20` and virtual playback cleanup. Exact source class names are stripped.
6. The gate is called at `00C66BB5` on an ordinary playback construction route
   and `00CA0266` on a music-related construction route. False AL branches to
   failure at `00C66BBF` and `00CA0270`, respectively, **before** their later
   playback construction/allocation. A previously admitted PostEvent can
   therefore have a nonzero playing ID without producing a new audible voice.

This explains why bank callbacks can succeed, initialization flags remain1,
and Malloc/Malign failure counters stay0 while playback is selectively stopped
or denied. Expanding the actual allocation gives this native policy room to
operate; disabling the policy is unnecessary for the first comparison.

## Correlation with the supplied run

Run4188 reports usable default bytes 5868820. Its pool5 snapshots show:

| Tick ms | Used bytes | Exact native pressure predicate |
|---:|---:|---|
| 25235 | 1018484 | false |
| 30266 | 1749596 | false |
| 35313 | 3142676 | false |
| 40344 | 5325936 | true |
| 45375 | 5348684 | true |
| 110860 | 5353468 | true |

All15 five-second snapshots from40344 onward are above threshold, out of 22
pool snapshots total. The high-water is 5361532 bytes (91.356%). The last sample
before crossing is35313, so these rows bound the observed crossing rather than
provide its exact time. No user-timestamped audible-loss marker was captured.
Platform pool peak was only10.019%; the default pool is the measured candidate.

StaticBanksPool is closer to full (97.035%) but is **not one of the two pool IDs
tested by this gate**. Its high occupancy alone is not the same mechanism. It
has no recorded failure, so this comparison changes only the shared default.

## Recognized limits and treatment

| Setting | Verified native value/location | Treatment |
|---|---|---|
| Shared default pool request | 5872025B, `0084A64D` | **16777216B (16MiB)** in new test |
| Shared/platform pressure thresholds | 90%, `0084A62E/654/659` | Preserved; default exceeded in old log |
| StaticBanksPool request | 4372480B, `0084A683` | Preserved;97% usage, no observed rejection |
| Platform request | 3145728B, `0084A646` | Preserved; sampled peak10% |
| FilePackagesPool request | 557056B, `0084A5D5` | Preserved; sampled peak86.923%, no observed failure |
| PrepareEventPool request | 307200B, `0084A63F` | Preserved; no measured pressure |
| Command storage request | 262144B, `00C50F29 -> 00C59B40` | Preserved; writer retries on full storage via signal/wait, not yet measured at that branch |
| Pool descriptors | 20, `0084A4BC` | Preserved; observed7/20 |
| Native emitter objects | 90x178 bytes, `0084AA29` | Preserved;443 attempts, no refusals; inline storage cannot grow by compare alone |
| Deferred cleanup objects | 90x12 bytes, `A+A4C` | Preserved; R1 sampled active peak1 |
| Bank-operation queue | 50x12 bytes, `00849870` | Preserved; observed peak2/50, zero boundary attempts |
| Two common middleware manager settings | 255 at InitSettings+4/+8, `00C50F0C/17/1A` | Preserved; consumers `00C6B740/00C6B160` allocate pointer arrays from default pool; exact roles/runtime pressure not fully classified |
| Event-lookup hash buckets | 193, `00C53F50` | Not a193-event capacity: bucket collisions follow linked chains; do not increase a divisor as a loaded-bank limit |

The two255 settings must not be called physical voice limits without completing
their consumer classification. This is a focused inventory, not proof that every
possible audio, file, decoder, voice or bank-authored limit has been found.

Explicit asynchronous unload calls exist in bank grouping/resource cleanup:
`0084BA02`, `0084BB0D`, `0084BF11`, `0084C028`, `0084E58F`, `0084E70B` call
numeric UnloadBank `00C51FD0`. The resource callback `0084BE80` clears bank ID
and state. These operations can invalidate event definitions, but static presence
does not prove an unexpected unload occurred in4188. No direct ClearBanks call
was found in the focused near-call scan; indirect calls remain possible.

## Controlled adjustment built

`dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_AUDIO_DEFAULT16M_R1.asi`  
Release / Win32;108544 bytes  
SHA256 `4106331601430E38A3BCB888DE0F5FC310B47CE9597EFFFE128742EC786D3535`

The single active solution project is `DriverSF_V29s_AUDIO_DEFAULT16M.vcxproj`.
Only the four-byte allocation-size operand at `0084A650` changes, within the
verified seven-byte `MOV [EBP+40],imm32` instruction. It restores the 16 MiB
request already used by the embedded middleware defaults/fallback. Native pool
creation sizes its own backing allocation, free lists and descriptors; no inline
object-array relocation is required. The extra requested backing is10905191B
(about 10.4 MiB), not an arbitrary10x increase.

R2 indexed262144, requests150, graphics160, ED/EF, pending/hash expansions and
all six existing audio diagnostic hooks remain. No new playback hot hook is
added. Existing R2 and audio-probe R1 ASIs are preserved with their prior hashes.
The disk Driver.exe is unchanged. This ASI is **staged only**, not installed.

`src/V29sAudioDefaultPoolPatch.inc` requires the prior successful stock/owned
R2 checks, stable identity, audio-not-initialized state, exact stock instruction,
exact threshold source/stores and unchanged threshold constant. It rechecks
cold/signature state before writing. Ordinary protection/cache failure attempts
restoration; incomplete rollback has a distinct -2 status. It does not resize an
already initialized audio pool. The old probe's six-hook guard still applies.

Logging uses `%TEMP%\DSFV29s_audio_default16m_<PID>.log`. A distinct
`AUDIO_LIMIT_START` records patch success and requested sizes. New low-rate
`AUDIO_PRESSURE` rows validate the live default pool ID and descriptor, then
log configured request, usable/used bytes, threshold and sampled pressure.
The comparison uses exact integer products equivalent to the native0.9 float,
without calling middleware or changing the pressure policy. It is a sampled
predicate, not a count of exact native gate decisions or kicked voices.

## Verification and limits

Evidence: `audio_default16m_r1_2026-11-05/`.

- `pool_tests.json`:20 production installer scenarios using fake game memory,
  including cold/identity/phase/signature refusal, late mutation, sole-operand
  writes, cache/protection rollback and explicitly failed rollback.
- The exact108-byte `00C66530` predicate was relocated into an isolated Win32
  test with all absolute data and its pool-stat call rebound to test objects.
  Fifteen checks include unsigned/boundary cases and run4188 data, and agree
  with the production integer snapshot helper. The original pressure function
  returns true for recorded old-pool usage and false for that usage with a 16 MiB
  usable-pool fixture. The game and ASI are never loaded or launched by this test.
- `startup_checks.json`:29 production R2 startup checks passed again.
- `audio_verification.json`, `indexed_verification.json`, `verification.json`:
  newly compiled six audio bridges and three capacity bridges, exact signatures,
  near/short incoming-branch checks, quiet baseline flags,13 indexed operands,
  constructor/free-chain models and165 address checks passed. The10-invocation
  shared-bridge harness result is reused with its matching unchanged source hash.
- `run4188_pressure_evidence.json`: all22 historical pool5 rows classified with
  the exact predicate, including15 above threshold.
- `default_pool_verification.json`: sole-operand model, compiled patch function,
  settings/gate fingerprints, startup order and prior artifact preservation.

Compilation had no errors; inherited signedness/unused-diagnostic/exported-entry
warnings remain. Log: `build/audio_default16m_r1_build.log`.

Real playback restoration, runtime patch success and performance remain untested.
The native gate is established and recorded usage satisfies its condition; exact
runtime stop/admission decisions and causation still need the user comparison.
The existing167 event-lookup/admission zeros may be a separate availability
problem. No promise is made that every audio defect shares one cause.

Next: follow `dist/MAIN_150/AUDIO_DEFAULT16M_R1_TEST_2026-11-05.md`, return the
complete new log and sound/body/FPS observations, and compare before further
capacity changes.

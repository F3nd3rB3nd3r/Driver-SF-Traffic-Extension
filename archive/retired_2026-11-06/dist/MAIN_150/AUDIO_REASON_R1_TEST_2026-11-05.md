# Audio event reason / sound object probe

Use `DriverSF_V29s_160Capacity_150Request_AUDIO_REASON_R1.asi` as the only enabled traffic ASI. Replace the previous traffic ASI while the game is closed and start a fresh process. The assistant staged this artifact here; it was not installed or run.

This is diagnostic only relative to the preceding2048 test. The16-event per-sound-object array stays16. Previous shared default64MiB, global audio2048 settings, original static/platform budgets,150 requests/160 graphics slots, ED/EF and indexed262144 remain. It adds no further capacity changes.

## Test

1. Check initial menu effects, then load the same save.
2. Try a known working car and a silent car, such as Crown Victoria/Scirocco/C10/Cadillac versus Zonda/370Z/Bentley. In each, accelerate, sound the horn and make the tires screech. Change camera if that changes the missing audio.
3. Note the order and approximate time from launch when you enter each car, along with which sounds work. This helps correlate sound-object addresses and callers; an address alone is not a car identity.
4. After the final actions, wait about5 seconds for aggregate rows and quit normally. A few minutes is sufficient for the first comparison. Note rough FPS and car visibility too.
5. Copy `%TEMP%\DSFV29s_audio_reason_<PID>.log` into this `dist/MAIN_150` folder and supply the observations.

## Required setup checks

- `DSFV29s_AUDIO_REASON_START revision=1 status=1 reason=armedBeforeAudioInit hooks=10 expectedHooks=10 changedLimits=0 objectEventLimit=16`.
- Inherited AUDIO_SLOTS_START status1/patches1/hooks9/testMax2048, AUDIO_START status1/hooks6, AUDIO_LIMIT_START status1/patches1/testRequestedBytes67108864.
- Inherited indexed startup status1/13 patches/31 pending redirects; later graphics/indexed startup statuses2. Initialized A0/A1/B headers should still show configured/allocated2048 and shared default live67108864B.
- New reason status rows should show invalid0, ownerOverflow0 and eventOverflow0. An overflow means aggregate coverage is bounded and incomplete, not a native game limit. A startup failure makes this probe run unsuitable for the intended comparison; return the log before changing anything.

## What the new rows establish

`AUDIO_REASON_STATUS` prints cumulative lookup hits/misses, selected playing-record creation failures, wrapper requests/returns/accepted/zero, pre-PostEvent full16 refusals, delivered/validated callbacks, completion callbacks, removes and maximum directly observed count. These are independent observations, not an assertion that every sound becomes audible. The16-entry gate can refuse before the old PostEvent probe sees a request.

`AUDIO_REASON_OWNER` aggregates by sound-object address: requests, none requests, refusals, accepted/zero posts, count peaks, completion/unmatched/removal counts and registration count. Repeated registrations indicate address reuse; the row aggregates across those lifetimes and is not a unique vehicle or an exact lifetime balance. `lastCount` is the latest native-path observation. `acceptedPeakProjected` is pre-insertion count+1 after a nonzero post, whose native insertion follows; `peakObserved` is read directly from the object.

`AUDIO_REASON_EVENT` aggregates lookup/post/refusal outcomes per32-bit event ID with first/last object and original wrapper caller.512 nonzero IDs plus a separate zero-ID record are available. Only first/last object/caller contexts are retained in this aggregate; it is not a complete event/object matrix.

`AUDIO_REASON_DETAIL` is bounded separately by kind, so none-hash failures cannot exhaust the other-lookup-miss detail budget:

| Kind | Bound | a /owner /c /d |
|---|---:|---|
| noneLookupMissing |64 | event /gameObject /helper return address /0 |
| lookupMissing |256 | event /gameObject /helper return address /0 |
| createFailed |64 | event /gameObject /creation result /0 |
| objectFull |64 | event /sound object /original game caller /pre-request count |
| wrapperPostZero |128 | event /sound object /0 /pre-insertion count |
| callbackUnmatched |64 | playing ID /sound object /callback type /current count |
| inputName |128 | hash if complete /sound object /caller /name pointer |

For inputName, `nameHex` contains up to63 bytes from the original narrow-string event wrapper input; flags1 means terminated/complete,2 unreadable,4 null,0 truncated. A hash is only supplied for complete names or null. Numeric callers do not supply strings through these wrappers, so zero name calls is possible and does not mean hooks failed.

The monitor drains detail records and prints status roughly once per second, with changed owner/event aggregates roughly every5 seconds. Only bounded scalar tables/buffers are read by the monitor. Native-path helpers preserve GP/flags/x87/SSE state and do no file I/O or middleware calls. Fields may advance independently while being logged. Detailed records stop at their bounds while cumulative counters/aggregates continue; table overflow is reported.

Release/Win32,130048 bytes. SHA256 `FCD56D9AD3FAA845ECFF75A86E4FD75288A8342B8B13453F44C11480B4661715`. Offline tests passed; live installation/audio/FPS remain untested.

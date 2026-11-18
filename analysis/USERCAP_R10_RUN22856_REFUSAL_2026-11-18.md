# R10 run22856:280-model AI exceeds active150-capacity INI

The supplied log contains an explicit guarded process exit rather than an
unclassified failure of the new output policy. At line126:

`GENERIC_FATAL reason=rosterExceedsConfiguredCapacity required=280 configuredRequests=150 partialListPublished=0`

Line10 confirms a valid, found INI (`valid=1 reason=configured`), with
AssetWeights, LogLevel1,150 requests,160 model slots,192 audio profiles,
2048 global audio entries,64MiB shared audio,1MiB Replay reader,256 retained
references and45000 extra hash nodes. These match the documented baseline150
values, not the intended Headroom320 configuration. The log does not record
the full INI path; it declares `location=asiDirectory`.

The initial18-model phase reaches native READY. Later, the active AI requires
280 entries, which cannot fit the configured150. The existing guard refuses
publication of a partial list and exits. There is no reason to remove this
guard, increase capacities silently, alter AI probabilities or change the ASI.

Resolution: copy `config/profiles/Headroom320.ini` beside the actual enabled
R10 ASI as `DriverSF_Unlimiter.ini`, replacing the150 profile, then restart.
The complete profile restores320 requests/330 derived model slots,448 audio
profiles,4096 global entries,128MiB shared audio,4MiB reader,768 retained refs
and180000 hash extras with summary LogLevel1. Changing only RequestCapacity
would leave incompatible smaller audio/reference values. The profile's source
was checked and still contains the intended values.

Startup/logging hooks report installed, and summary mode retained the essential
fatal line. This does not establish a successful R10 Story/activity/Replay
smoke test; that remains pending after correcting the installed configuration.
No dump or new ASI is required to explain this exit.

Input: `config/dsfv29s_usercap_r10_22856.log`,31323 bytes, SHA256
`56b2df4d41e2530da2d244419b7008db2eef3c6710dd51a6795307072a73d482`.
Analysis/handoff only: no code/ASI/INI/build/game/install/asset/save/commit/push
changes were made. The installed INI was not opened or replaced by this review.

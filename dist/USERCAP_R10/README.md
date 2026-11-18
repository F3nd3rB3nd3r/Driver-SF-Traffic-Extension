# DriverSF Unlimiter R10

R10 retains the existing streaming/lifetime fixes and adds reason-specific
audio diagnostics plus configurable log output. The ASI here is the same
327168-byte binary used in the successful R10 Story and Replay smoke tests;
it has not been rebuilt for publication. See `manifest.json` for file hashes.

Install the ASI and one configuration:

1. Enable exactly one traffic-extension ASI in the game's plugins directory.
   Move/disable the previous traffic-extension ASI before enabling this one.
2. Put `DriverSF_Unlimiter_USERCAP_R10.asi` and `DriverSF_Unlimiter.ini`
   together in that directory. An ASI loader is required separately.
3. Choose the INI for the actual traffic roster. The main INI is the
   150-capacity baseline. For the existing 280-model AI, use
   `profiles/Expanded280.ini` or `profiles/Headroom320.ini`, renamed to
   `DriverSF_Unlimiter.ini`. Headroom320_Full lists all advanced keys explicitly.
   Do not replace a working 280/320 configuration with the baseline 150 file.
4. Restart the game after configuration changes. Matching vehicle assets and
   the intended `media/san_franciscoAI.fchunk` are supplied separately.

The [configuration guide](../../CONFIGURATION.md) explains every supported
limit, automatic sizing, traffic weights and the worked 500-model example.
`profiles/Experimental500.ini` is an untested starting point, not verified
500-model support. Capacities do not add vehicles, change weights or increase
the on-road population. Missing assets/incorrect IDs are not repaired by memory
settings. An insufficient request budget is deliberately refused.

`[Diagnostics] LogLevel=1` is the default summary mode. Use 0 for minimal
output or 2 for a focused full trace; resource checks remain active at every
level. Logs are `%TEMP%/DSFV29s_usercap_r10_<PID>.log`. These new INIs require
R10: older versions reject the Diagnostics section.

Validation covers the observed 280-model workload: R9's broader activity,
Lua traffic-switching and Story/menu/Replay tests, followed by R10 Story and
Replay summary-mode smoke tests with 320-capacity backing. It does not establish
all-chapter mission coverage or actual 500/700/1000-model gameplay. READY counts
alone do not certify simultaneous drawable residency. See the linked results:

- [R9 combined activity/lifecycle test](../../analysis/USERCAP_R9_RUN26476_RESULTS_2026-11-18.md)
- [R9 Headroom320 test](../../analysis/USERCAP_R9_RUN7044_RESULTS_2026-11-18.md)
- [R10 Story smoke](../../analysis/USERCAP_R10_RUN20268_RESULTS_2026-11-18.md)
- [R10 Replay smoke](../../analysis/USERCAP_R10_RUN23108_REPLAY_2026-11-18.md)

This directory contains the repository candidate files. Full gameplay logs,
memory dumps, vehicle assets and local build intermediates are not included.

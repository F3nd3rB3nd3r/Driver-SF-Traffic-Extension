# Immediate next action: V29s matched runtime comparison

**Status:** No V29s in-game results have been supplied yet. V29r 45 success and 150 stall are already established; do not rerun V29q/V29r without new reason.

1. Run `dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_TRACE.asi` alone (no other traffic capacity ASI, initially without DriverNGHook) on the same Chapter 7 save; preserve `%TEMP%\DSFV29f_worldmem_<PID>.log` after the loading stall (e.g. 60–90s). Do not deliberately pause x32dbg.
2. Separate process with `dist/CONTROL_45/DriverSF_V29s_160Capacity_45Request_TRACE.asi` alone; confirm visible city/cars and preserve its log.
3. `py tools\analyze_v29s_run.py "C:\path\to\45.log" "C:\path\to\150.log"` (45 first, 150 second). If user attaches logs, analyze them directly; no need to request analyzer output as well.
4. Compare `DSFV29s_STATUS`, `DSFV29s_KIND`, `DSFV29s_ID` for `gate`, `blocked`, `dispatch`, `memberPass`, `transfers`, `rosterLast`, each ID and map. Seek earliest meaningful divergence in processing; no assumption `transfer==READY`.
5. If `DSFV29s_START hooks=-1` (or missing), stop; validate detour installation and executable hash rather than interpreting missing counters.

**After comparing results:** supply a concise proven finding, uncertainty, and one sharply scoped next diagnostic or patch only if justified. Do not automatically begin an experiment, build an ASI, search every file, or run all tests.

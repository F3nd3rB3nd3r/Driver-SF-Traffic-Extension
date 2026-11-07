# Current test: PARKED150_R1

Use `DriverSF_V29s_160Capacity_150Request_PARKED150_R1.asi` with
`PARKED150_R1_TEST_2026-11-06.md`. Native fixture and inherited checks passed;
live parked visual validation is pending. Keep the adjusted AI file, enable
exactly one traffic ASI, and reproduce the toggle/delayed refresh.
Log prefix: `DSFV29s_parked150_`. Package staged; no agent installation.

## Latest live-validated parent: NOTIFY150_R1

Run29344 passed the user's extended Challenge/menu/Story/chase sequence with
correct audio/rendering, roughly100 FPS and normal exit. Parked fallback still
occurred; PARKED150_R1 targets that separate defect. Root report:
`analysis/NOTIFY150_RUN29344_RESULTS_2026-11-06.md`.
The earlier run7480 separately exercised the direct150 notification correction.

Older BASELINE150_R1/ARENA_REUSE_R1 lack that notification correction and remain
unsuitable for the direct150 helper. All older staged ASIs remain unchanged.

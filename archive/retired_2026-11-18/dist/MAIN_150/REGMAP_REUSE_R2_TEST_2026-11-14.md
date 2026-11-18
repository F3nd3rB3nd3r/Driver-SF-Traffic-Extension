# REGMAP_REUSE_R2 startup correction

R1 has a reproduced startup-check bug. Use
`DriverSF_V29s_160Capacity_150Request_REGMAP_REUSE_R2.asi` instead, with R1 and
other traffic ASIs disabled. This is staged here; the agent did not install
it or launch the game.

Release/Win32;253440 bytes. SHA256:
`8d180487ca129e0bc88f880265427858cab9e48e9730bbba112c077d38b5bdaa`.

First check that the game reaches its main menu. Keep the existing assets,
recordings and settings. Copy `%TEMP%\DSFV29s_regmap_reuse_<PID>.log` into
MAIN_150 after the run, including if startup fails. The prefix is unchanged;
R2's successful REGMAP_START includes `startupCheckRevision=2`. A failure
before that line should now have REGMAP_INSTALL_FAIL with its installer stage.

If startup succeeds, load Story, check audio/rendering/parked variety, then
test Story -> Replay -> Story, playback controls, menu/Story reload and normal
exit. Repeat several transitions to exercise new and reused map addresses.
REGMAP_STATUS owners>4 and reconstructs/reattaches>0 identify the lifecycle
paths exercised; loads do not directly equal distinct addresses.

R2 preserves R1's dynamic owner storage, constructor generations,512 external
map nodes, all replay corrections and every existing capacity. The change
allows signature checks across readable pages with different protection;
modified bytes, guard/no-access pages and actual map corruption remain rejected.

Native tests now use the exact production Readable body and real VirtualQuery/
VirtualProtect with read-only executable pages. R1's refusal was reproduced;
R2 passes that case plus the lifecycle, failure/rollback and inherited checks.
Actual game startup and longer lifecycle stability still need live validation.

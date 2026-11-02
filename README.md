# DriverSF V29s — lean VS workspace

Open `START_HERE.md` first. This is a clean, focused snapshot from the larger DriverSF traffic research repository. No raw logs, old binaries, generated build directories, third-party unpacked files, or obsolete solution folders are bundled.

- For the Visual Studio AI, paste the text in `PROMPT_FOR_VISUAL_STUDIO.md` once.
- For the only active VS solution, open `DriverSF_V29s_NATIVE_TRACE.slnx` (**Release | Win32**, v145).
- The 150/160 main and 45/160 control compiled test ASIs are in separate `dist/` folders. They must NOT be installed together.
- A fresh Git repository contains this handoff snapshot as an initial commit; it is deliberately **not** the original dirty historical Git working tree. The previous full V29s archive preserves that history and uncommitted files.
- Python analysis needs no external packages. VS 2026 with x86 C++ build tools is required to rebuild natively.
- `reference/` is optional background material. The VS AI must not read it wholesale at startup.

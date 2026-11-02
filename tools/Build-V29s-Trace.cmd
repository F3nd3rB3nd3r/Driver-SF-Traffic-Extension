@echo off
setlocal
cd /d "%~dp0.."
if not defined VSCMD_VER (
    echo ERROR: Open an x86 Native Tools Command Prompt for VS 2026 first.
    echo Or open DriverSF_V29s_NATIVE_TRACE.slnx in VS and build Release / Win32.
    exit /b 1
)
if not exist "build\v29s_native_trace" mkdir "build\v29s_native_trace"
cl /nologo /c /O2 /Oi- /GS- /Zl /EHs- /GR- /W4 /std:c++17 /DDSF_CHAPTER7_PRELOAD_COUNT=150 /DDSF_GRAPHICS_ADMISSION_TARGET=160 /DDSF_V29S_NATIVE_TRACE=1 /Fo"build\v29s_native_trace\DriverSF_V29s.obj" "src\DriverSF_V29b_NativeFastPath.cpp"
if errorlevel 1 exit /b 1
link /nologo /dll /machine:x86 /entry:DllMain@12 /nodefaultlib /out:"build\v29s_native_trace\DriverSF_V29s_160Capacity_150Request_TRACE.asi" "build\v29s_native_trace\DriverSF_V29s.obj" kernel32.lib psapi.lib
if errorlevel 1 exit /b 1
py verification\verify_v29s_native.py
if errorlevel 1 exit /b 1
exit /b 0

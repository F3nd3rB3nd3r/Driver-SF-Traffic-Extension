#!/usr/bin/env python3
"""Reproducible offline Win32 PE32 cross-build: clang-cl + lld-link.

Supplement to the native VS 2026 Win32 build. No downloads or Windows SDK.
  Optional --requests 45 builds a 45-request control with unchanged 160 slots.
Uses COFF short import objects as documented in the existing build.py.
"""
from pathlib import Path
import os
import struct
import subprocess
import argparse

ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('--requests',type=int,default=150,choices=(45,150))
opts=ap.parse_args()
root = Path(__file__).resolve().parents[1]
output = root / ('build/v29s_native_trace' if opts.requests==150 else 'build/v29s_native_trace_45control')
output.mkdir(parents=True, exist_ok=True)
clang = os.environ.get('CLANG_CL', '/usr/local/swift/usr/bin/clang-cl')
link = os.environ.get('LLD_LINK', '/usr/local/swift/usr/bin/lld-link')
api_names = {
 'GetModuleHandleW':4, 'VirtualProtect':16, 'VirtualQuery':12,
 'OutputDebugStringA':4, 'IsDebuggerPresent':0, 'Sleep':4, 'CreateThread':24,
 'CloseHandle':4, 'DisableThreadLibraryCalls':4, 'VirtualAlloc':16,
 'VirtualFree':12, 'FlushInstructionCache':12, 'GetCurrentProcess':0,
 'ExitProcess':4, 'InterlockedIncrement':4, 'InterlockedExchange':8, 'InterlockedCompareExchange':12,
 'GetTempPathA':8, 'CreateFileA':28, 'WriteFile':20,
 'GetCurrentProcessId':0,'GetCurrentThreadId':0,'GetTickCount':0,
}
imports = []
for name, bytes_ in api_names.items():
    string = f'_{name}@{bytes_}\0KERNEL32.dll\0'.encode('ascii')
    blob = struct.pack('<HHHHIIHH', 0, 0xffff, 0, 0x14c, 0, len(string), 0, 3 << 2) + string
    path = output / f'{name}.obj'
    path.write_bytes(blob)
    imports.append(str(path))
name = '_GetProcessMemoryInfo@12\0PSAPI.dll\0'.encode('ascii')
path = output / 'GetProcessMemoryInfo.obj'
path.write_bytes(struct.pack('<HHHHIIHH',0,0xffff,0,0x14c,0,len(name),0,3 << 2)+name)
imports.append(str(path))
import_lib = output / 'v29s_win32_imports.lib'
subprocess.run([os.environ.get('LLVM_AR', '/usr/local/swift/usr/bin/llvm-ar'), 'rcs', str(import_lib), *imports], check=True)
obj = output / 'DriverSF_V29s.obj'
asi = output / f'DriverSF_V29s_160Capacity_{opts.requests}Request_TRACE.asi'
subprocess.run([clang,'--target=i686-pc-windows-msvc','/nologo','/c','/O2','/Oi-','/GS-','/Zl',
 '/EHs-','/GR-','/W4','/std:c++17',f'/DDSF_CHAPTER7_PRELOAD_COUNT={opts.requests}',
 '/DDSF_GRAPHICS_ADMISSION_TARGET=160','/DDSF_V29S_NATIVE_TRACE=1',f'/Fo{obj}',
 str(root / 'src/DriverSF_V29b_NativeFastPath.cpp')], check=True)
subprocess.run([link,'/nologo','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib',
 '/subsystem:windows',f'/out:{asi}',str(obj),str(import_lib)], check=True)
print(asi)

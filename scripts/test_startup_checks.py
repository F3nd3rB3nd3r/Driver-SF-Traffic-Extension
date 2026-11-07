"""Build/run an isolated Win32 fake-image test of the production C++ checks.

Does not load an ASI, read/execute Driver.exe, inspect plugins or attach a process.
Requires the already installed MSVC toolchain/Windows SDK; installs nothing.
"""
import hashlib
import json
import re
import subprocess
import winreg
from pathlib import Path
from static_streaming_audit import DUMPBIN

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/"analysis/indexed_capacity_r2_2026-11-05"
BUILD=ROOT/"build/startup_checks_r2"


def main(output=OUT,build=BUILD):
    OUT,BUILD=output,build
    OUT.mkdir(parents=True,exist_ok=True)
    BUILD.mkdir(parents=True,exist_ok=True)
    main_source=(ROOT/"src/DriverSF_V29b_NativeFastPath.cpp").read_text()
    test_source=(ROOT/"tests/StartupChecksTest.cpp").read_text()
    for constant in ("kPreferredBase","kGraphicsCtor","kHandlerInit","kTimestamp","kSizeOfImage"):
        def value(source):
            return int(re.search(r"\b"+constant+r"\s*=\s*(0x[0-9A-Fa-f]+)u",source)[1],16)
        assert value(main_source)==value(test_source),constant
    dllmain=main_source[main_source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dllmain.index("ApplyPreinitGraphicsExpansion();")<dllmain.index("InstallHandlerHook()")<dllmain.index("IndexedInstall();")
    preinit=main_source[main_source.index("void ApplyPreinitGraphicsExpansion()") : main_source.index("void ApplyPreinitGraphicsExpansion()")+1300]
    assert preinit.index("if (!VerifyExecutable())")<preinit.index("g_indexedStockVerified=true;")
    indexed=(ROOT/"src/V29sIndexedCapacity.inc").read_text().split("static bool IndexedInstall() {",1)[1]
    assert "VerifyExecutable()" not in indexed
    assert indexed.index("VerifyIndexedStartup(g_indexedStockVerified")<indexed.index("IndexedPendingRedirectIntact()")<indexed.index("IndexedInstructionsMatch(false,0u)")<indexed.index("IndexedCold()")
    # Discover only the installed SDK import library used by this test.
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r"SOFTWARE\Microsoft\Windows Kits\Installed Roots",0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
        sdk=Path(winreg.QueryValueEx(key,"KitsRoot10")[0])
    candidates=[v/"um/x86/kernel32.lib" for v in (sdk/"Lib").iterdir() if re.fullmatch(r"\d+(\.\d+)+",v.name)]
    library=max((v for v in candidates if v.is_file()),key=lambda v:tuple(map(int,v.parents[2].name.split('.'))))
    compiler=DUMPBIN.with_name("cl.exe")
    program=BUILD/"StartupChecksTest.exe"
    command=[str(compiler),"/nologo","/O2","/GS-","/GR-","/Zl","/EHs-c-",
        "/Fo"+str(BUILD/"StartupChecksTest.obj"),str(ROOT/"tests/StartupChecksTest.cpp"),
        "/link","/NODEFAULTLIB","/ENTRY:StartupGuardTestMain@0","/SUBSYSTEM:CONSOLE",
        "/MACHINE:X86","/BASE:0x30000000","/FIXED","/DYNAMICBASE:NO","/OUT:"+str(program),str(library)]
    compile_result=subprocess.run(command,cwd=BUILD,capture_output=True,text=True)
    (OUT/"startup_test_build.log").write_text(compile_result.stdout+compile_result.stderr,encoding="utf-8")
    if compile_result.returncode:
        print(compile_result.stdout+compile_result.stderr)
        raise RuntimeError("Startup test compilation failed")
    run=subprocess.run([str(program)],capture_output=True,text=True,timeout=20)
    assert run.returncode==0,f"Startup test failed at check {run.returncode}: {run.stdout} {run.stderr}"
    assert "startup guard checks passed: 29" in run.stdout,repr(run.stdout)
    summary=dict(checks=29,result="passed",architecture="Win32",configuration="optimized",
        production_checks_sha256=hashlib.sha256((ROOT/"src/V29sStartupChecks.inc").read_bytes()).hexdigest(),
        startup_order_verified=True,old_self_rejection_reproduced=True,owned_hook_accepted=True,
        negative_cases="Missing stock preflight; unknown handler target/opcode; trampoline bytes/return/owner; absent hook; unreadable ranges; modified PE identity/graphics constructor; wrong base/PE offset",
        limitations="Production C++ startup checks executed against fake memory in a standalone test. Does not exercise full DllMain, OS patch failure paths or the game.")
    (OUT/"startup_checks.json").write_text(json.dumps(summary,indent=2),encoding="utf-8")
    print(run.stdout.strip())
    print(json.dumps(summary,indent=2))


if __name__=="__main__": main()

"""Offline validation of the capacity ASI; never loads or executes the ASI."""
import hashlib
import json
import re
import struct
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

from static_streaming_audit import DUMPBIN, PE
from workspace_paths import historical_path

ROOT = Path(__file__).resolve().parents[1]
NAME = "DriverSF_V29s_160Capacity_150Request_CAPACITY_TRACE"
BUILD = ROOT / "build" / "v29s_capacity_150"
OUTPUT = ROOT / "analysis" / "capacity_trace_2026-11-05"


def definitions(filename):
    tree = ET.parse(historical_path(filename))
    node = tree.find(".//{*}PreprocessorDefinitions")
    return set(node.text.split(";"))


def main(project="DriverSF_V29s_CAPACITY_150.vcxproj", name=NAME, build=BUILD,
         output=OUTPUT, extra_flags=None):
    # Reuse the bridge checks for the isolated indexed-table variant as well.
    NAME, BUILD, OUTPUT = name, build, output
    OUTPUT.mkdir(parents=True, exist_ok=True)
    binary = BUILD / (NAME + ".asi")
    pe = PE(binary)
    native = PE()
    assert struct.unpack_from("<H", pe.data, pe.pe + 4)[0] == 0x14C, "Expected x86"
    assert pe.characteristics & 0x2000, "Expected DLL/ASI"
    quiet = definitions("DriverSF_V29s_ORDERED_150_PERF.vcxproj")
    diagnostic = definitions(project)
    expected_flags = extra_flags or {"DSF_V29S_CAPACITY_TRACE=1"}
    assert diagnostic - quiet == expected_flags
    assert not quiet - diagnostic, "Quiet baseline flags were removed"
    solution = ET.parse(ROOT / "DriverSF_V29s_NATIVE_TRACE.slnx")
    projects = solution.findall("Project")
    assert len(projects) == 1 and projects[0].get("Path") == project

    symbols = {}
    for line in (BUILD / (NAME + ".map")).read_text().splitlines():
        match = re.match(r"\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+(\S+)\s+([0-9A-Fa-f]{8})", line)
        if match:
            symbols[match[1]] = int(match[2], 16)
    tramp = next(value for key, value in symbols.items() if key.startswith("?g_capacityTramps@"))
    def resolve_linker_thunk(address):
        for _ in range(4):
            try:
                prefix = pe.read(address, 5)
            except ValueError:
                return None  # A scanned E8 can be an immediate, not a call.
            if prefix[0] != 0xE9:
                return address
            address += 5 + struct.unpack_from("<i", prefix, 1)[0]
        raise AssertionError("Unexpected jump-thunk chain")
    bridge_results = []
    for name, size, index in (("Audio", 0x40, 0), ("Traffic", 0x60, 1), ("Retire", 0x40, 2)):
        start = symbols[f"_Capacity{name}Bridge"]
        code = pe.read(start, size)
        assert code.startswith(bytes.fromhex("9c 60")) if name != "Traffic" else code.startswith(b"\x9c")
        assert bytes.fromhex("0f ae 04 24") in code, "Missing FXSAVE"
        assert bytes.fromhex("0f ae 0c 24") in code, "Missing FXRSTOR"
        assert bytes.fromhex("83 e4 f0") in code, "Missing aligned FP area"
        assert bytes.fromhex("ff 75 18") in code, "Wrong saved ECX argument"
        if name != "Retire":
            assert bytes.fromhex("ff 75 24") in code, "Wrong saved return address"
        assert b"\x61\x9d\xff\x25" + struct.pack("<I", tramp + 4 * index) in code, "Wrong state restore/tail jump"
        target = symbols[f"_Capacity{name}Entry"]
        calls = []
        for i in range(len(code) - 4):
            if code[i] == 0xE8:
                destination = start + i + 5 + struct.unpack_from("<i", code, i + 1)[0]
                if resolve_linker_thunk(destination) == target:
                    calls.append(start + i)
        assert len(calls) == 1, "Bridge does not call the expected helper exactly once"
        if name == "Traffic":
            for caller in (0x5A68A7, 0x5A7E3D, 0x5A7E6D):
                assert struct.pack("<I", caller) in code, "Traffic caller filter missing"
        disassembly = subprocess.run([str(DUMPBIN), "/nologo", "/nopdb", "/disasm:bytes",
            f"/range:0x{start:X},0x{start + size - 1:X}", str(binary)], capture_output=True, text=True, check=True)
        (OUTPUT / f"{name.lower()}_bridge.asm.txt").write_text(disassembly.stdout, encoding="utf-8")
        bridge_results.append(dict(name=name, address=hex(start), helper=hex(target), fx_state_preserved=True))

    spans = {0x849870: bytes.fromhex("83 b9 e0 92 00 00 00"),
             0x83B170: bytes.fromhex("83 79 04 00 8b 01"),
             0x5A7DD1: bytes.fromhex("83 79 04 00 8b 01")}
    for address, signature in spans.items():
        assert native.read(address, len(signature)) == signature
    # Candidate near CALL/JMP/Jcc destinations into stolen instruction interiors.
    # Whole-section byte scanning can produce false matches; any hit needs manual review.
    incoming = []
    text = next(s for s in native.sections if s["name"] == ".text")
    block = native.data[text["offset"]:text["offset"] + text["size"]]
    for i in range(len(block) - 6):
        if block[i] in (0xE8, 0xE9):
            destination = text["va"] + i + 5 + struct.unpack_from("<i", block, i + 1)[0]
        elif block[i] == 0x0F and 0x80 <= block[i + 1] <= 0x8F:
            destination = text["va"] + i + 6 + struct.unpack_from("<i", block, i + 2)[0]
        else:
            continue
        for address, signature in spans.items():
            if address < destination < address + len(signature):
                incoming.append(dict(source=hex(text["va"] + i), target=hex(destination)))
    assert not incoming, f"Review candidate incoming branches: {incoming}"
    summary = dict(binary=str(binary), sha256=hashlib.sha256(pe.data).hexdigest(),
        architecture="Win32", configuration="Release", baseline_flags_preserved=True,
        additional_flags=sorted(expected_flags), bridges=bridge_results,
        native_signature_sites=[hex(v) for v in spans], near_branch_interior_candidates=incoming,
        limitations="Offline checks only; runtime hook installation, behavior and performance untested")
    (OUTPUT / "verification.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()

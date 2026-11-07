"""Offline indexed-reference relocation inventory. Never patches or runs Driver.exe."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path

from static_streaming_audit import DUMPBIN, EXE, PE

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "analysis" / "indexed_handles_static_2026-11-05"
OWNER = 0x01732BF0
SLOT_OFFSET = 0x17E094
NATIVE_COUNT = 0x20000

# VA, immediate/displacement offset, native value, role. Research inventory only.
SITES = [
    (0xBAE86E, 2, 0x17E094, "slot-base subtraction for index encoding"),
    (0xBAE8B3, 3, 0x17E094, "insert: neighboring slot first link"),
    (0xBAE968, 2, 0x17E094, "constructor: first slot address"),
    (0xBAEB5D, 3, 0x17E094, "unlink: slot first link"),
    (0xBAEB64, 3, 0x17E094, "unlink: current slot address"),
    (0xBAEB80, 3, 0x17E094, "unlink: predecessor slot address"),
    (0xBAEBA3, 3, 0x17E094, "unlink: successor slot address"),
    (0xBAED0F, 3, 0x17E094, "resource removal: slot address"),
    (0xBAEC7F, 3, 0x17E098, "resource replacement: next reference"),
    (0xBAED08, 3, 0x17E098, "resource removal: next reference"),
    (0xBAE9A3, 2, 0x27E08C, "constructor: final free-link terminator"),
    (0xBAE984, 1, 0x1FFFF, "constructor: link count minus one"),
    (0xBAE8A9, 1, 0x20000, "neighbor index upper bound"),
]


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    pe = PE()
    sha = hashlib.sha256(pe.data).hexdigest()
    assert sha == "c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616"
    text = next(s for s in pe.sections if s["name"] == ".text")
    def code_refs(value):
        return [hex(v) for item in pe.refs(value, False)
                if text["va"] <= (v := int(item, 16)) < text["va"] + text["size"]]
    inventory = []
    for site, offset, value, role in SITES:
        assert struct.unpack("<I", pe.read(site + offset, 4))[0] == value, hex(site)
        inventory.append(dict(site=hex(site), operand=hex(site + offset), original=hex(value),
                              role=role, instruction_prefix=pe.read(site, offset + 4).hex(" ")))
    literal_refs = {hex(v): code_refs(v) for v in
                    (0x17E094, 0x17E098, 0x27E08C, 0x27E094)}
    expected_counts = {"0x17e094": 8, "0x17e098": 2, "0x27e08c": 1, "0x27e094": 7}
    assert {key: len(value) for key, value in literal_refs.items()} == expected_counts
    ranges = [(0xBAE770, 0xBAE930), (0xBAE930, 0xBAEA10), (0xBAEA80, 0xBAEBD9),
              (0xBAEBE0, 0xBAEF10), (0xBB42E0, 0xBB440E), (0xBABAA0, 0xBABB03),
              (0x6BDCA0, 0x6BDCEE), (0xBAA290, 0xBAA3E0), (0xBA97A0, 0xBA97D6),
              (0xBC5380, 0xBC5400), (0xDC3F50, 0xDC3F66), (0xDC7F30, 0xDC7F40),
              (0xBAB930, 0xBAB9E0), (0xBA9FD0, 0xBAA096), (0xBC3790, 0xBC3820)]
    for lo, hi in ranges:
        result = subprocess.run([str(DUMPBIN), "/nologo", "/nopdb", "/disasm:bytes",
                                 f"/range:0x{lo:X},0x{hi:X}", str(EXE)],
                                capture_output=True, text=True, check=True)
        (OUT / f"{lo:08X}-{hi:08X}.asm.txt").write_text(result.stdout, encoding="utf-8")

    # Stream disassembly to avoid leaving a huge whole-executable listing. Keep
    # only potential absolute addresses into the native slot array and metadata.
    absolute_lines = []
    relative_lines = []
    parsed_lines = 0
    process = subprocess.Popen([str(DUMPBIN), "/nologo", "/nopdb", "/disasm:bytes", "/section:.text", str(EXE)],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    for line in process.stdout:
        match = re.match(r"\s+([0-9A-Fa-f]{8}):", line)
        if not match:
            continue
        parsed_lines += 1
        # Bracketed absolute operands and immediate hex constants are retained
        # for manual review. Numeric text alone does not prove a memory access.
        for token in re.findall(r"\b([0-9A-Fa-f]{1,8})h\b", line):
            value = int(token, 16)
            if OWNER + SLOT_OFFSET <= value < OWNER + 0x27E098:
                absolute_lines.append(line.rstrip())
            if SLOT_OFFSET - 0x100 <= value < 0x27E198:
                relative_lines.append(line.rstrip())
    errors = process.stderr.read()
    assert process.wait() == 0, errors
    (OUT / "absolute_slot_address_candidates.txt").write_text("\n".join(absolute_lines) + "\n", encoding="utf-8")
    (OUT / "relative_slot_offset_candidates.txt").write_text("\n".join(relative_lines) + "\n", encoding="utf-8")

    # Validate relocation/index arithmetic in memory only, including wraparound
    # owner-relative displacements, the first index above the native bound and
    # the final external slot. This is not an executable patch or runtime test.
    proposed_count = 0x40000
    arithmetic = []
    for external_base in (0x21000000, 0x00400000, 0x90000000):
        displacement = (external_base - OWNER) & 0xFFFFFFFF
        for index in (0, 1, 0x1FFFF, 0x20000, proposed_count - 1):
            address = (OWNER + displacement + index * 8) & 0xFFFFFFFF
            recovered_bytes = (address - OWNER - displacement) & 0xFFFFFFFF
            assert recovered_bytes < 0x80000000
            recovered_index = recovered_bytes >> 3
            assert recovered_index == index
            arithmetic.append(dict(base=hex(external_base), index=index, address=hex(address)))
    summary = dict(executable=str(EXE), sha256=sha, owner=hex(OWNER), native_slots=NATIVE_COUNT,
        slot_base=hex(OWNER + SLOT_OFFSET), free_head=hex(OWNER + 0x27E094),
        graphics_manager=hex(0x019B0CD0), metadata_to_graphics_gap=0x019B0CD0 - (OWNER + 0x27E094),
        candidate_operand_changes=inventory, unchanged_free_head_literal_references=literal_refs["0x27e094"],
        code_literal_inventory=literal_refs, streamed_instruction_lines=parsed_lines,
        absolute_slot_address_candidates=absolute_lines,
        relative_slot_offset_candidates=relative_lines,
        arithmetic_checks=arithmetic, illustrative_external_slots=proposed_count,
        illustrative_external_bytes=proposed_count * 8,
        limitations="Validated literal inventory and selected functions; indirect/computed aliases and ASI loader timing still require care. No patch or runtime test.")
    (OUT / "relocation_inventory.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps({key: summary[key] for key in ("sha256", "native_slots", "slot_base", "free_head",
        "metadata_to_graphics_gap", "streamed_instruction_lines", "absolute_slot_address_candidates")}, indent=2))
    print("Relative offset candidates:", len(relative_lines), "(see JSON/text evidence)")
    print("Verified candidate operand changes:", len(inventory), "; arithmetic cases:", len(arithmetic))


if __name__ == "__main__":
    main()

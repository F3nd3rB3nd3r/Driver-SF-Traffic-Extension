"""Offline checks: never loads an ASI, executes Driver.exe or attaches a process."""
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path

from static_streaming_audit import PE, DUMPBIN
import verify_capacity_trace

ROOT = Path(__file__).resolve().parents[1]
NAME = "DriverSF_V29s_160Capacity_150Request_INDEXED_262144_R2"
BUILD = ROOT / "build/v29s_indexed_262144_r2"
OUT = ROOT / "analysis/indexed_capacity_r2_2026-11-05"
R, COUNT = 0x1732BF0, 262144
MASK = 0xFFFFFFFF


def u32(data, at=0):
    return struct.unpack_from("<I", data, at)[0]


def constructor_check(code, base, count):
    """Interpret the actual x86 slot-loop instructions, with guarded fake RAM.

    Starts after unrelated hash/cleanup initialization. Deliberately rejects
    unknown opcodes. No native machine code is executed.
    """
    def read(pc, n):
        return bytes(code[pc - 0xBAE000:pc - 0xBAE000 + n])
    assert read(0xBAE968, 2) == b"\x8d\xbe"
    edi = (R + u32(read(0xBAE96A, 4))) & MASK
    assert edi == base
    memory, writes = {}, set()
    head_address = R + 0x27E094

    def store(address, value):
        assert address == head_address or (base <= address < base + count * 8 and (address-base) % 8 == 0), hex(address)
        memory[address] = value & MASK
        writes.add(address)

    pc, eax, ecx, edx, zero, steps = 0xBAE982, 0, 0, 0, False, 0
    while True:
        steps += 1
        assert steps < count * 6 + 30
        instruction = read(pc, 8)
        if instruction[:2] == b"\x8b\xc7": eax, pc = edi, pc+2
        elif instruction[0] == 0xBA: edx, pc = u32(instruction, 1), pc+5
        elif instruction[:7] == bytes.fromhex("8d a4 24 00 00 00 00"): pc += 7
        elif instruction[:3] == b"\x83\xea\x01":
            edx = (edx-1) & MASK
            zero, pc = edx == 0, pc+3
        elif instruction[:3] == b"\x8d\x48\x08": ecx, pc = (eax+8) & MASK, pc+3
        elif instruction[:2] == b"\x89\x08": store(eax, ecx); pc += 2
        elif instruction[:2] == b"\x8b\xc1": eax, pc = ecx, pc+2
        elif instruction[0] == 0x75:
            displacement = struct.unpack("b", instruction[1:2])[0]
            pc += 2 + (displacement if not zero else 0)
        elif instruction[:2] == b"\x89\xbe":
            store((R+u32(instruction, 2)) & MASK, edi); pc += 6
        elif instruction[0] in (0x5F, 0x5E): pc += 1  # stack restores do not affect remaining slot writes
        elif instruction[:2] == b"\x89\x96":
            store((R+u32(instruction, 2)) & MASK, edx); pc += 6
        elif instruction[:2] == b"\x8b\xc6": eax, pc = R, pc+2
        elif instruction[0] == 0xC3: break
        else: raise AssertionError(f"Unsupported opcode at {pc:08X}: {instruction.hex()}")
    assert len(writes) == count + 1
    assert memory[head_address] == base
    # Exhaust the constructed list: no duplicate/out-of-range slot, exact count.
    head = memory[head_address]
    for index in range(count):
        assert head == base + index * 8
        head = memory[head]
    assert head == 0
    # Simulate native push/pop recycling at the new boundary and final slot.
    for index in (0, 131071, count-1):
        slot = base + index*8
        memory[slot], head = head, slot
        popped, head = head, memory[head]
        assert popped == slot and head == 0
    return dict(base=hex(base), slots=count, constructor_steps=steps,
                writes=count+1, exact_free_chain=True, exhaustion_and_recycling=True)


def main(project="DriverSF_V29s_INDEXED_262144.vcxproj",name=NAME,build=BUILD,output=OUT,extra_flags=None):
    NAME,BUILD,OUT=name,build,output
    OUT.mkdir(parents=True, exist_ok=True)
    native, binary = PE(), PE(BUILD / (NAME + ".asi"))
    regression=json.loads((OUT/"startup_checks.json").read_text())
    assert regression["result"]=="passed" and regression["checks"]==29
    assert regression["production_checks_sha256"]==hashlib.sha256((ROOT/"src/V29sStartupChecks.inc").read_bytes()).hexdigest()
    assert b"startupCheckRevision=2" in binary.data
    assert hashlib.sha256(native.data).hexdigest() == "c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616"
    inventory = json.loads((ROOT / "analysis/indexed_handles_static_2026-11-05/relocation_inventory.json").read_text())
    symbols = {}
    for line in (BUILD / (NAME + ".map")).read_text().splitlines():
        match = re.match(r"\s+\w{4}:\w{8}\s+(\S+)\s+([0-9a-fA-F]{8})", line)
        if match: symbols[match[1]] = int(match[2], 16)
    def symbol(prefix):
        return next(value for key, value in symbols.items() if key.startswith("?"+prefix+"@"))
    # Check the actual compiled C++ inventory, including struct packing.
    table = binary.read(symbol("g_indexedPatches"), 13*16)
    patches = []
    for i, expected in enumerate(inventory["candidate_operand_changes"]):
        row = table[i*16:(i+1)*16]
        site, offset, length = struct.unpack_from("<IBB", row)
        signature, kind = row[6:6+length], row[13]
        assert site == int(expected["site"],16) and site+offset == int(expected["operand"],16)
        assert signature == bytes.fromhex(expected["instruction_prefix"]) == native.read(site,length)
        expected_kind = 3 if site==0xBAE984 else 4 if site==0xBAE8A9 else 2 if site==0xBAE9A3 else 1 if u32(signature,offset)==0x17E098 else 0
        assert kind == expected_kind and offset+4 == length
        assert site & ~0xFFF == 0xBAE000 and (site+length-1) & ~0xFFF == 0xBAE000
        patches.append((site,offset,length,signature,kind))
    original = bytearray(native.read(0xBAE000,0x1000))
    cases = [constructor_check(original, R+0x17E094, 131072)]
    addressing_checks = 0
    for base in (0x21000000,0x00400000,0x90000000):
        page = original.copy()
        displacement = (base-R) & MASK
        allowed = set()
        for site,offset,length,signature,kind in patches:
            value = (displacement,displacement+4,displacement+8*(COUNT-1),COUNT-1,COUNT)[kind] & MASK
            at = site-0xBAE000+offset
            page[at:at+4] = struct.pack("<I", value)
            allowed.update(range(at,at+4))
            if kind < 3:
                for index in (0,1,131071,131072,COUNT-1):
                    # Final-slot displacement has no index register.
                    index_term = 0 if kind==2 or site in (0xBAE86E,0xBAE968) else index*8
                    address = (R+value+index_term) & MASK
                    expected_address = base+4+index*8 if kind==1 else base+8*(COUNT-1) if kind==2 else base+index_term
                    assert address == expected_address
                    addressing_checks += 1
            if site==0xBAE86E:
                for index in (0,1,131071,131072,COUNT-1):
                    encoded = (((base+index*8-R-value) & MASK)//8) | 0x80000000
                    assert encoded == 0x80000000 | index
        assert all(page[j]==original[j] for j in range(4096) if j not in allowed)
        cases.append(constructor_check(page,base,COUNT))
        # Restore exactly the original operands; unrelated pending patches survive.
        for site,offset,length,signature,kind in patches:
            at=site-0xBAE000+offset
            page[at:at+4]=signature[offset:offset+4]
        assert page == original
    for field in ("IndexedCold","IndexedInstall","IndexedInstructionsMatch","IndexedRefuse","VerifyExecutableIdentity"):
        start=symbol(field)
        following=min(v for v in symbols.values() if v>start)
        result=subprocess.run([str(DUMPBIN),"/nologo","/nopdb","/disasm:bytes",
            f"/range:0x{start:X},0x{following-1:X}",str(BUILD/(NAME+".asi"))],capture_output=True,text=True,check=True)
        (OUT/(field+".asm.txt")).write_text(result.stdout,encoding="utf-8")
    verify_capacity_trace.main(project=project,name=NAME,
        build=BUILD,output=OUT,extra_flags=extra_flags or {"DSF_V29S_CAPACITY_TRACE=1","DSF_V29S_INDEXED_262144=1"})
    summary=dict(sha256=hashlib.sha256(binary.data).hexdigest(),bytes=len(binary.data),
        startup_check_revision=2,startup_regression_checks=regression["checks"],
        native_identity_verified=True,compiled_patch_rows=13,code_pages_changed=1,
        constructor_cases=cases,addressing_checks=addressing_checks,
        unrelated_code_preserved=True,operand_restore_model=True,
        limitations="Offline only. Constructor slot loop interpreted in fake RAM; OS failure paths reviewed, not fault-injected. Native startup, runtime installation, visuals/audio and performance require the user's game test.")
    (OUT/"indexed_verification.json").write_text(json.dumps(summary,indent=2),encoding="utf-8")
    print(json.dumps(summary,indent=2))


if __name__=="__main__": main()

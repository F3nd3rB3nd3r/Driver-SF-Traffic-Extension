"""Read-only PE inspection using the installed Microsoft disassembler.

No process attachment, execution of the target, or target writes are performed.
"""
import argparse
import hashlib
import json
import struct
import subprocess
from pathlib import Path

GAME = Path(r"E:\Games\Driver San Francisco Save Bug Test")
EXE = GAME / "Driver.exe"
DUMPBIN = Path(r"E:\Visual Studio\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x86\dumpbin.exe")
OUT = Path(__file__).resolve().parents[1] / "analysis" / "streaming_static_2026-11-04"


class PE:
    def __init__(self, path=EXE):
        self.data = Path(path).read_bytes()
        self.pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        self.count, = struct.unpack_from("<H", self.data, self.pe + 6)
        self.characteristics, = struct.unpack_from("<H", self.data, self.pe + 22)
        optional_size, = struct.unpack_from("<H", self.data, self.pe + 20)
        self.base, = struct.unpack_from("<I", self.data, self.pe + 24 + 28)
        section_start = self.pe + 24 + optional_size
        self.sections = []
        for i in range(self.count):
            at = section_start + i * 40
            name = self.data[at:at + 8].rstrip(b"\0").decode("ascii")
            virtual_size, rva, size, offset = struct.unpack_from("<IIII", self.data, at + 8)
            flags, = struct.unpack_from("<I", self.data, at + 36)
            self.sections.append(dict(name=name, va=self.base + rva, virtual_size=virtual_size,
                                      size=size, offset=offset, flags=flags))

    def read(self, va, size):
        for section in self.sections:
            relative = va - section["va"]
            if 0 <= relative and relative + size <= section["size"]:
                at = section["offset"] + relative
                return self.data[at:at + size]
        raise ValueError(f"Unbacked address {va:08X}")

    def refs(self, target, calls):
        results = []
        for section in self.sections:
            if calls and not section["flags"] & 0x20000000:
                continue
            block = self.data[section["offset"]:section["offset"] + section["size"]]
            if calls:
                for i in range(len(block) - 4):
                    if block[i] == 0xE8 and ((section["va"] + i + 5 + struct.unpack_from("<i", block, i + 1)[0]) & 0xFFFFFFFF) == target:
                        results.append(f"{section['va'] + i:08X}")
            else:
                needle = struct.pack("<I", target)
                at = block.find(needle)
                while at >= 0:
                    results.append(f"{section['va'] + at:08X}")
                    at = block.find(needle, at + 1)
        return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=["identity", "disasm", "calls", "refs", "read", "packages"])
    parser.add_argument("values", nargs="*")
    args = parser.parse_args()
    pe = PE()
    if args.command == "identity":
        identity = dict(path=str(EXE), sha256=hashlib.sha256(pe.data).hexdigest(),
                        base=hex(pe.base), large_address_aware=bool(pe.characteristics & 0x20),
                        sections=pe.sections)
        OUT.mkdir(parents=True, exist_ok=True)
        (OUT / "executable_identity.json").write_text(json.dumps(identity, indent=2), encoding="utf-8")
        print(json.dumps(identity, indent=2))
    elif args.command == "disasm":
        OUT.mkdir(parents=True, exist_ok=True)
        for pair in args.values:
            lo, hi = (int(v, 16) for v in pair.split(":"))
            result = subprocess.run([str(DUMPBIN), "/nologo", "/nopdb", "/disasm:bytes",
                                     f"/range:0x{lo:X},0x{hi:X}", str(EXE)],
                                    capture_output=True, text=True, check=True)
            destination = OUT / f"{lo:08X}-{hi:08X}.asm.txt"
            destination.write_text(result.stdout, encoding="utf-8")
            print(f"{destination}: {len(result.stdout.splitlines())} lines")
    elif args.command in ("calls", "refs"):
        for value in args.values:
            print(value, "candidate byte references; validate instruction boundaries:",
                  " ".join(pe.refs(int(value, 16), args.command == "calls")))
    elif args.command == "packages":
        OUT.mkdir(parents=True, exist_ok=True)
        package_path = GAME / "Install" / "dngvehicles.sp"
        with package_path.open("rb") as stream:
            def entries(at):
                stream.seek(at)
                header = stream.read(16)
                if len(header) != 16 or header[:4] != b"CHNK":
                    return []
                _, total, count, version = struct.unpack("<4sIII", header)
                if count > 100000 or version & 0x80000000:
                    raise ValueError(f"Unexpected chunk at {at:X}: {header.hex()}")
                table = stream.read(16 * count)
                result = []
                for i in range(count):
                    tag, relative, ver, name_size, alignment, flag, size = struct.unpack_from("<4sIBBBBI", table, 16 * i)
                    if relative + size > total:
                        raise ValueError("Chunk entry exceeds parent")
                    result.append(dict(tag=tag.decode("ascii"), at=at + relative, size=size,
                                       version=ver, name_size=name_size, alignment=alignment, flag=flag))
                return result
            root = entries(0)
            summary = dict(path=str(package_path), size=package_path.stat().st_size, root_count=len(root),
                           root_tags={}, ssic=[], vehicle_nonr=[], drsp_children=[], banks=[])
            summary["largest_vehc"] = max((v["size"] for v in root if v["tag"] == "VEHC"), default=0)
            for entry in root:
                summary["root_tags"][entry["tag"]] = summary["root_tags"].get(entry["tag"], 0) + 1

            def walk(parent, prefix, output):
                for entry in entries(parent):
                    path = prefix + "/" + entry["tag"]
                    output.append(dict(path=path, **entry))
                    if entry["tag"] in ("NONR", "VSRT", "MDXN", "DRSP"):
                        children = entries(entry["at"])
                        if entry["tag"] == "DRSP":
                            summary["drsp_children"].append(len(children))
                        walk(entry["at"], path, output)

            for i, entry in enumerate(root):
                if entry["tag"] not in ("SSIC", "VEHC"):
                    continue
                nested = []
                walk(entry["at"], entry["tag"], nested)
                if entry["tag"] == "SSIC":
                    summary["ssic"] = nested
                    for item in nested:
                        if item["tag"] == "SSLP":
                            stream.seek(item["at"])
                            payload = stream.read(item["size"])
                            records = [struct.unpack_from("<HHH", payload, at) for at in range(0x1C, len(payload), 6)]
                            summary["sslp"] = dict(header=payload[:0x1C].hex(" "),
                                count=struct.unpack_from("<H", payload, 2)[0],
                                nonr_sizes=struct.unpack_from("<III", payload, 8), slots=list(payload[0x18:0x1C]),
                                records=records)
                else:
                    for item in nested:
                        if item["path"] == "VEHC/NONR":
                            summary["vehicle_nonr"].append(dict(root_index=i, size=item["size"]))
                        if item["tag"] == "SBAK":
                            summary["banks"].append(dict(root_index=i, size=item["size"]))
            (OUT / "package_metadata.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
            print(json.dumps({key: value for key, value in summary.items() if key not in ("ssic", "sslp", "vehicle_nonr", "drsp_children", "banks")}, indent=2))
            print("SSIC metadata entries:", len(summary["ssic"]))
            print("SSLP:", {key: value for key, value in summary.get("sslp", {}).items() if key != "records"})
            print("NONR max:", max(summary["vehicle_nonr"], key=lambda v: v["size"], default=None))
            print("DRSP child max:", max(summary["drsp_children"], default=0))
            print("SBAK count/total/max:", len(summary["banks"]), sum(v["size"] for v in summary["banks"]), max((v["size"] for v in summary["banks"]), default=0))
    elif args.command == "read":
        va, size = (int(v, 16) for v in args.values)
        print(pe.read(va, size).hex(" "))


if __name__ == "__main__":
    main()

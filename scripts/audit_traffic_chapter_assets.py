"""Read only the explicitly supplied docs and three named media AI assets.

Reconstruct asset routing independently of the research repository's scripts,
CSVs, captures, archives, or source trees. Writes evidence only under --output.
"""
import argparse
import collections
import csv
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import zlib

ORIGINAL_SHA = "da5b1ca3e4e51b6c8d937ed0eb2cbeb2a54048c9aca5a3e5a26f8704b1f0b936"
INSTALLED_SHA = "434e1a516fb88a91fdaa02e2c35ac44f460c804ff312c04232a5bed011d8db13"
ROOT = 0x1080
UNKNOWN = {0xC071E02A, 0x9CB17769, 0x11DF09F4, 0x40CF70C9, 0xCC9F35B5}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def decode(data, names):
    def u32(at):
        return struct.unpack_from("<I", data, at)[0]

    def pointer(field, count, stride):
        result = ROOT + u32(ROOT + field)
        assert result >= ROOT and result + count * stride <= len(data)
        return result

    counts = {field: u32(ROOT + field) for field in (0xE8, 0x118, 0x120, 0x128, 0x158, 0x160)}
    assert counts[0xE8] == 319 and counts[0x118] == 59
    assert counts[0x128] == 319 * 59 and counts[0x158] == 208
    assert counts[0x160] == 208 * 191
    groups = pointer(0xEC, 319, 12)
    settings = pointer(0x11C, 59, 28)
    lists = pointer(0x124, counts[0x120], 2)
    routes = pointer(0x12C, 319 * 59, 20)
    metadata = pointer(0x15C, 208, 12)
    weights = pointer(0x164, 208 * 191, 8)
    assert settings == 0x280A00 and lists == 0x281080
    assert metadata == 0x2DDA80 and weights == 0x2DE440
    tables = []
    row_ids = None
    for index in range(208):
        start, count, total = struct.unpack_from("<IIf", data, metadata + 12 * index)
        assert count == 191 and start == index * 191
        rows = [struct.unpack_from("<If", data, weights + (start + n) * 8) for n in range(count)]
        assert all(math.isfinite(weight) and weight >= 0 for _, weight in rows)
        ids = [model for model, _ in rows]
        if row_ids is None:
            row_ids = ids
            assert len(set(ids)) == 191
        assert ids == row_ids
        assert abs(sum(weight for _, weight in rows) - total) < 0.0001
        tables.append({"table": index, "total": total,
                       "positive_ids": [model for model, weight in rows if weight > 0],
                       "positive_weights": [weight for _, weight in rows if weight > 0]})
    records = []
    triples = set()
    membership_mismatches = []
    for slot in range(59):
        at = settings + slot * 28
        key = u32(at)
        starts = struct.unpack_from("<5H", data, at + 4)
        sizes = struct.unpack_from("<5H", data, at + 14)
        selector = data[at + 24]
        assert selector < 59
        record = {"slot": slot, "hash": f"{key:08X}", "name": names.get(key),
                  "selector": selector, "offset": f"0x{at:X}", "frequencies": []}
        for frequency, (start, count) in enumerate(zip(starts, sizes)):
            assert start + count <= counts[0x120]
            if not count:
                continue
            ids = list(struct.unpack_from(f"<{count}H", data, lists + 2 * start))
            table_counts = collections.Counter()
            for group in range(319):
                base = struct.unpack_from("<H", data, groups + group * 12 + 8)[0]
                route_index = base + selector
                assert route_index < counts[0x128]
                index = data[routes + route_index * 20 + 13 + frequency]
                assert index < 208
                table_counts[index] += 1
            selected = sorted(table_counts)
            positive = sorted({model for index in selected for model in tables[index]["positive_ids"]})
            missing = sorted(set(positive) - set(ids))
            if missing:
                membership_mismatches.append({"hash": record["hash"], "frequency": frequency, "missing_ids": missing})
            for index in selected:
                triples.add((key, frequency, index))
            record["frequencies"].append({"frequency": frequency, "request_count": count,
                "request_ids": ids, "tables": selected,
                "group_counts": dict(sorted(table_counts.items())), "positive_ids": positive,
                "positive_count": len(positive), "positive_ids_missing_from_native_request": missing})
        records.append(record)
    assert len({r["hash"] for r in records}) == 59
    assert {int(r["hash"], 16) for r in records if r["name"] is None} == UNKNOWN
    assert len(triples) == 280 and len({t[2] for t in triples}) == 149
    named_sp = {t[2] for t in triples if t[0] in names and not names[t[0]].startswith("Online Traffic")}
    named_online = {t[2] for t in triples if t[0] in names and names[t[0]].startswith("Online Traffic")}
    unnamed = {t[2] for t in triples if t[0] not in names}
    return {"counts": {hex(k): v for k, v in counts.items()}, "records": records, "tables": tables,
            "triples": [{"hash": f"{h:08X}", "frequency": f, "table": t} for h, f, t in sorted(triples)],
            "named_sp_tables": sorted(named_sp), "named_online_tables": sorted(named_online),
            "unnamed_tables": sorted(unnamed), "membership_mismatches": membership_mismatches,
            "regions": {"settings": (settings, 59 * 28), "lists": (lists, counts[0x120] * 2),
                        "groups": (groups, 319 * 12), "routes": (routes, 319 * 59 * 20)}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--docs", type=Path, required=True)
    parser.add_argument("--media", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    inventory = []
    for path in sorted(args.docs.glob("*.md")):
        raw = path.read_bytes()
        lines = raw.decode("utf-8-sig").splitlines()
        inventory.append({"name": path.name, "bytes": len(raw), "sha256": sha(raw), "lines": len(lines),
                          "headings": [{"line": n, "text": line} for n, line in enumerate(lines, 1)
                                       if re.match(r"^#{1,6} ", line)]})
    names = {}
    for line in (args.docs / "SETTING_HASHES.md").read_text(encoding="utf-8-sig").splitlines():
        match = re.match(r"\| `0x([0-9A-Fa-f]{8})` \| `([^`]+)` \|", line)
        if match:
            key, name = int(match[1], 16), match[2]
            assert zlib.crc32(name.encode("utf-8"), 0xFFFFFFFF) == key
            names[key] = name
    assert len(names) == 53
    names[0x5956218A] = "Chapter 2 God"
    assert zlib.crc32(b"Chapter 2 God", 0xFFFFFFFF) == 0x5956218A
    assets = {}
    raw_assets = {}
    for label, filename, expected in (
        ("original", "san_franciscoAI_original.fchunk", ORIGINAL_SHA),
        ("installed", "san_franciscoAI.fchunk", INSTALLED_SHA),
        ("candidate", "san_franciscoAI_uniform_150_candidate.fchunk", INSTALLED_SHA),
    ):
        path = args.media / filename
        raw = path.read_bytes()
        assert sha(raw) == expected, f"Unsupported asset identity: {path}"
        raw_assets[label] = raw
        assets[label] = {"path": str(path), "bytes": len(raw), "sha256": sha(raw)}
    original = decode(raw_assets["original"], names)
    installed = decode(raw_assets["installed"], names)
    changed_tables = [i for i in range(208) if original["tables"][i] != installed["tables"][i]]
    assert len(changed_tables) == 125
    assert changed_tables == installed["named_sp_tables"]
    assert not (set(changed_tables) & set(installed["named_online_tables"]))
    roster = installed["tables"][changed_tables[0]]["positive_ids"]
    assert len(roster) == 150
    assert all(installed["tables"][i]["positive_ids"] == roster and
               installed["tables"][i]["positive_weights"] == [1.0] * 150 and
               installed["tables"][i]["total"] == 150.0 for i in changed_tables)
    unchanged = {}
    for region, (start, size) in original["regions"].items():
        assert installed["regions"][region] == (start, size)
        unchanged[region] = raw_assets["original"][start:start + size] == raw_assets["installed"][start:start + size]
        assert unchanged[region]
    allowed = bytearray(len(raw_assets["original"]))
    for i in changed_tables:
        allowed[0x2DDA80 + i * 12 + 8:0x2DDA80 + i * 12 + 12] = b"\1" * 4
        start = 0x2DE440 + i * 191 * 8
        for row in range(191):
            allowed[start + row * 8 + 4:start + row * 8 + 8] = b"\1" * 4
    different = [i for i, (a, b) in enumerate(zip(raw_assets["original"], raw_assets["installed"])) if a != b]
    assert len(raw_assets["original"]) == len(raw_assets["installed"])
    assert all(allowed[i] for i in different), "Changed bytes outside documented weights/totals"
    for record in installed["records"]:
        for freq in record["frequencies"]:
            freq["native_request_ids_outside_150_roster"] = sorted(set(freq["request_ids"]) - set(roster))
    sp_profiles = [(r, f) for r in installed["records"] if r["name"] and not r["name"].startswith("Online Traffic")
                   for f in r["frequencies"]]
    summary = {"scope": f"All {len(inventory)} top-level markdown docs indexed; three exact named media assets parsed; no external references followed",
               "docs_count": len(inventory), "named_hashes_crc_checked": len(names), "assets": assets,
               "candidate_equals_installed": raw_assets["candidate"] == raw_assets["installed"],
               "unchanged_regions": unchanged, "changed_byte_count": len(different), "changed_tables": changed_tables,
               "uniform_roster": roster, "uniform_roster_count": len(roster),
               "changed_unnamed_shared_tables": sorted(set(changed_tables) & set(installed["unnamed_tables"])),
               "original_membership_mismatch_count": len(original["membership_mismatches"]),
               "installed_membership_mismatch_count": len(installed["membership_mismatches"]),
               "named_sp_setting_count": len({r["hash"] for r, _ in sp_profiles}),
               "named_sp_profile_count": len(sp_profiles),
               "named_sp_native_count_range": [min(f["request_count"] for _, f in sp_profiles),
                                                max(f["request_count"] for _, f in sp_profiles)],
               "named_sp_native_ids_all_inside_150": all(not f["native_request_ids_outside_150_roster"] for _, f in sp_profiles),
               "named_sp_tables": len(installed["named_sp_tables"]), "named_online_tables": len(installed["named_online_tables"]),
               "static_triples": len(installed["triples"]), "routed_tables": len({r["table"] for r in installed["triples"]}),
               "no_active_route_tables": sorted(set(range(208)) - {r["table"] for r in installed["triples"]}),
               "records": installed["records"]}
    args.output.mkdir(parents=True, exist_ok=True)
    for filename, value in (("docs_inventory.json", inventory), ("original.json", original),
                            ("installed.json", installed), ("summary.json", summary)):
        (args.output / filename).write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")
    fields = ["slot", "hash", "name", "family", "frequency", "native_request_count", "native_request_ids",
              "table_indices", "positive_id_count", "positive_ids_missing_from_native_request_count",
              "native_request_ids_outside_150_roster"]
    with (args.output / "setting_profiles.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for record in summary["records"]:
            family = "unresolved_name_mode" if record["name"] is None else (
                "named_online" if record["name"].startswith("Online Traffic") else "named_single_player")
            for freq in record["frequencies"]:
                writer.writerow({"slot": record["slot"], "hash": record["hash"], "name": record["name"] or "",
                    "family": family, "frequency": freq["frequency"], "native_request_count": freq["request_count"],
                    "native_request_ids": " ".join(map(str, freq["request_ids"])),
                    "table_indices": " ".join(map(str, freq["tables"])), "positive_id_count": freq["positive_count"],
                    "positive_ids_missing_from_native_request_count": len(freq["positive_ids_missing_from_native_request"]),
                    "native_request_ids_outside_150_roster": " ".join(map(str, freq["native_request_ids_outside_150_roster"]))})
    reference = ["# Current traffic setting reference", "",
                 "Derived from the three identity-checked media assets and names in the supplied docs.",
                 "54 names CRC-checked with exact whitespace; five original names/modes remain unresolved.",
                 "All 59 native settings and 80 active frequency profiles are included in setting_profiles.csv.",
                 "Counts are static requested IDs, not READY/drawable counts. Current weighted SP roster has 150 IDs.", "",
                 "| Slot | Hash | Exact name | Frequency: native request / weighted ID union |",
                 "|---:|---|---|---|"]
    for record in sorted(summary["records"], key=lambda r: r["slot"]):
        label = "unresolved" if record["name"] is None else "`" + record["name"] + "`"
        if record["name"] and record["name"].endswith(" "):
            label += " (trailing space)"
        profiles = "; ".join(f"{f['frequency']}: {f['request_count']} / {f['positive_count']}" for f in record["frequencies"])
        reference.append(f"| {record['slot']} | `{record['hash']}` | {label} | {profiles} |")
    (args.output / "settings_reference.md").write_text("\n".join(reference) + "\n", encoding="utf-8")
    compact = {key: value for key, value in summary.items() if key not in ("records", "uniform_roster", "no_active_route_tables")}
    print(json.dumps(compact, indent=2))
    print("Chapter/native requests:")
    for record in summary["records"]:
        if record["name"] and (record["name"].startswith("Chapter ") or record["name"] in ("Exposition", "Epilogue", "Free Drive")):
            print(record["hash"], repr(record["name"]), [(f["frequency"], f["request_count"], f["positive_count"],
                  f["native_request_ids_outside_150_roster"]) for f in record["frequencies"]])


if __name__ == "__main__":
    main()

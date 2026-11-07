"""Summarize one supplied V29s capacity log without reading other logs."""
import argparse
import hashlib
import itertools
import json
import re
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    raw = args.log.read_bytes()
    rows = []
    for number, line in enumerate(raw.decode("utf-8-sig").splitlines(), 1):
        if not line.strip():
            continue
        values = dict(re.findall(r"(\w+)=([^\s]+)", line))
        rows.append(dict(line=number, kind=line.split()[0], **values))
    statuses = [r for r in rows if r["kind"] == "DSFV29s_CAP_STATUS"]
    queues = [r for r in rows if r["kind"] == "DSFV29s_CAP_QUEUE"]
    boundaries = [r for r in rows if r["kind"] == "DSFV29s_CAP_BOUNDARY"]
    starts = [r for r in rows if r["kind"] == "DSFV29s_CAP_START"]
    assert statuses and len(starts) == 1, "Expected one complete session header"
    last = statuses[-1]
    initialized = [r for r in statuses if r.get("initializedByObservedHead") == "1"]
    empty = [r for r in initialized if int(r["indexedFreeHead"], 16) == 0]
    first_empty = empty[0] if empty else None
    after = [r for r in initialized if first_empty and int(r["tick"]) >= int(first_empty["tick"])]
    stats = {}
    for name in sorted({r["queue"] for r in queues}):
        selected = [r for r in queues if r["queue"] == name]
        valid = [r for r in selected if r["valid"] == "1"]
        stats[name] = dict(valid_summaries=len(valid),
            inserts=max(int(r["inserts"]) for r in selected),
            projected_peak=max(int(r["peakAfterInsertProjected"]) for r in selected),
            sampled_peak=max(int(r["peakActiveSampled"]) for r in selected),
            boundary_attempts=max(int(r["boundaryAttempts"]) for r in selected),
            invalid_counts=max(int(r["invalidCounts"]) for r in selected),
            owner_changes=max(int(r["ownerChanges"]) for r in selected),
            final=selected[-1])
    plateaus = []
    for pending, group in itertools.groupby(initialized, key=lambda r: int(r["pending"])):
        run = list(group)
        plateaus.append(dict(pending=pending, first_tick=int(run[0]["tick"]),
            last_tick=int(run[-1]["tick"]), summaries=len(run),
            duration_ms=int(run[-1]["tick"]) - int(run[0]["tick"])))
    timeline = []
    previous = None
    for r in statuses:
        state = (r["startupStatus"], r["requestLast"], r["chapterHits"],
                 r["initializedByObservedHead"], int(r["indexedFreeHead"], 16) == 0)
        if state != previous:
            timeline.append(r)
            previous = state
    summary = dict(log=str(args.log), sha256=hashlib.sha256(raw).hexdigest(),
        start=starts[0], lines=len(rows), status_summaries=len(statuses),
        last_tick=int(last["tick"]), final_status=last, queues=stats,
        boundary_lines=boundaries,
        bad_head_summaries=[r for r in initialized if r["headValid"] != "1"],
        file_status_values=sorted({int(r["fileStatus"]) for r in statuses}),
        first_empty_status=first_empty,
        nonempty_statuses_after_first_empty=sum(int(r["indexedFreeHead"], 16) != 0 for r in after),
        pending_max_sampled=max(int(r["pendingPeakSampled"]) for r in statuses),
        pending_max_printed=max(int(r["pending"]) for r in statuses),
        longest_positive_pending_plateaus=sorted([p for p in plateaus if p["pending"]],
            key=lambda p: p["duration_ms"], reverse=True)[:4],
        transition_timeline=timeline)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print("session:", json.dumps(summary["start"]))
    print("status summaries / last tick:", len(statuses), summary["last_tick"])
    print("queues:", json.dumps({name: {k: v for k, v in values.items() if k != "final"}
                                for name, values in stats.items()}))
    print("first empty:", json.dumps(first_empty))
    print("final:", json.dumps(last))
    print("positive pending plateaus:", json.dumps(summary["longest_positive_pending_plateaus"]))
    print("transition timeline:", json.dumps(timeline))
    print("bad heads / file statuses / boundary lines / nonempty later:",
          len(summary["bad_head_summaries"]), summary["file_status_values"], len(boundaries),
          summary["nonempty_statuses_after_first_empty"])


if __name__ == "__main__":
    main()

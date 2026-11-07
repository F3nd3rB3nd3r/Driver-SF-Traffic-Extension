"""Compare explicitly supplied indexed-test logs; never scans log directories."""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("logs",type=Path,nargs="+")
    parser.add_argument("--output",type=Path,required=True)
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    comparison=[]
    for log in args.logs:
        output=args.output/(log.stem+"_summary.json")
        subprocess.run([sys.executable,str(ROOT/"scripts/analyze_capacity_log.py"),str(log),
            "--output",str(output)],capture_output=True,text=True,check=True)
        summary=json.loads(output.read_text())
        rows=[]
        for number,line in enumerate(log.read_text(encoding="utf-8-sig").splitlines(),1):
            if not line.strip():continue
            rows.append(dict(line=number,kind=line.split()[0],**dict(re.findall(r"(\w+)=([^\s]+)",line))))
        starts=[r for r in rows if r["kind"]=="DSFV29s_INDEXED_START"]
        assert len(starts)==1,"Expected exactly one indexed session"
        start=starts[0]
        statuses=[r for r in rows if r["kind"]=="DSFV29s_CAP_STATUS"]
        observed=[r for r in statuses if r["initializedByObservedHead"]=="1"]
        selected=[r for r in observed if int(r["requestLast"])==150 and int(r["chapterHits"])>0]
        assert selected,"Expanded chapter request never observed"
        faults=[r for r in rows if "FATAL" in r["kind"] or "BOUNDARY" in r["kind"]]
        checks={"revision_2":start.get("startupCheckRevision")=="2",
            "installed_13":start["status"]=="1" and start["patches"]=="13",
            "stock_and_owned_hook":start.get("stockPreflight")=="1" and start.get("ownedHookVerified")=="1",
            "pending_redirect_31":start["pendingRedirectPatches"]=="31",
            "capacity_change_enabled":summary["start"]["capacityChanges"]=="1",
            "all_three_hooks":all(r["hooks"]=="3" for r in statuses),
            "external_table_observed":bool(observed) and all(r["indexedStatus"]=="2" and r["indexedSlots"]=="262144" and r["indexedBase"]==start["externalBase"] for r in observed),
            "head_range_valid":all(r["headValid"]=="1" for r in statuses),
            "150_requests_160_slots":all(r["startupStatus"]=="2" and r["graphicsSlots"]=="160" for r in selected),
            "no_file_error":summary["file_status_values"]==[0],"no_boundary_or_fatal":not faults}
        assert all(checks.values()),checks
        nonzero=[r for r in observed if int(r["indexedFreeHead"],16)]
        for r in nonzero:
            address=int(r["indexedFreeHead"],16);base=int(r["indexedBase"],16)
            assert address>=base and (address-base)%8==0 and (address-base)//8==int(r["indexedHeadIndex"])<262144
        head_max=max(int(r["indexedHeadIndex"]) for r in nonzero)
        beyond_old=[r for r in nonzero if int(r["indexedHeadIndex"])>=131072]
        last=summary["final_status"]
        audio=summary["queues"]["audioBanks"]
        record=dict(log=str(log),sha256=summary["sha256"],checks=checks,
            startup=start,last_tick_ms=summary["last_tick"],initialized_samples=int(last["samples"]),
            empty_samples=int(last["emptySamples"]),empty_transitions=int(last["emptyTransitionsSampled"]),
            pending_peak_sampled=summary["pending_max_sampled"],pending_final=int(last["pending"]),
            final_indexed_status=int(last["indexedStatus"]),
            first_printed_150_tick_ms=int(selected[0]["tick"]),
            printed_pending_after_150_max=max(int(r["pending"]) for r in selected),
            printed_zero_pending_after_150=sum(r["pending"]=="0" for r in selected),
            printed_statuses_after_150=len(selected),max_printed_free_head_index=head_max,
            first_printed_head_above_native_range=beyond_old[0] if beyond_old else None,
            audio_inserts=audio["inserts"],audio_projected_peak=audio["projected_peak"],
            audio_sampled_peak=audio["sampled_peak"],audio_boundary_attempts=audio["boundary_attempts"],
            audio_final_active=int(audio["final"]["active"]),other_queues={name:{k:v for k,v in q.items() if k!="final"}
                for name,q in summary["queues"].items() if name!="audioBanks"})
        comparison.append(record)
        print(json.dumps({k:v for k,v in record.items() if k not in ("checks","startup","other_queues","first_printed_head_above_native_range")},indent=2))
    result=dict(runs=comparison,
        total_initialized_samples=sum(r["initialized_samples"] for r in comparison),
        total_empty_samples=sum(r["empty_samples"] for r in comparison),
        limitations="200 ms samples may miss transient exhaustion. Free-head index is a list position, not occupancy or total active references. Queue acceptance/retirement does not prove successful bank loading, event playback or drawable vehicles.")
    (args.output/"comparison.json").write_text(json.dumps(result,indent=2),encoding="utf-8")


if __name__=="__main__":main()

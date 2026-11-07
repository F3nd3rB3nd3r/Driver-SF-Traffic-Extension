"""Offline proof for the two explicitly supplied BASELINE150 minidumps.

No game, debugger, native fixture or ASI build is executed. Validate native
instruction spans, derive the callback stack layout, and compare captured data.
"""
import argparse,hashlib,json,struct
from pathlib import Path
from analyze_audio_model192_exit_dump import analyze_dump
from analyze_baseline150_log import analyze
from compare_audio_slots_logs import rows
from static_streaming_audit import PE

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pair',action='append',nargs=2,metavar=('LOG','DUMP'),required=True)
    parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    native=PE();assert hashlib.sha256(native.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    spans={
        0x5A77E1:'8b8e680e000085c9740d8b4424108b118b52045056ffd2',
        0x74B660:'83790400740a8b49048b018b4004ffe0c20800',
        0x754C75:'81ecd0000000',
        0x754CA4:'8bbc24e4000000',
        0x754CBB:'8b8c24e00000008d04bd0000000050518d54241c52e84b3b4d0083c40c',
        0x754D03:'5e81c4d0000000c20800',
    }
    for address,hexbytes in spans.items():assert native.read(address,len(bytes.fromhex(hexbytes)))==bytes.fromhex(hexbytes),hex(address)
    assert struct.unpack('<I',native.read(0xE2A70C,4))[0]==0x74B660
    assert struct.unpack('<I',native.read(0xE2AE1C,4))[0]==0x754C70
    results=[]
    for log_arg,dump_arg in args.pair:
        log,dump=Path(log_arg),Path(dump_arg);records=rows(log);coverage=analyze(log);info=analyze_dump(dump);data=dump.read_bytes()
        def read(address,n):
            for m in info['memory_ranges']:
                start=int(m['start'],16)
                if start<=address and address+n<=start+m['size']:
                    off=m['rva']+address-start;return data[off:off+n]
        def word(address):
            b=read(address,4);return struct.unpack('<I',b)[0] if b else None
        e=info['exception'];regs={k:int(v,16) for k,v in e['registers'].items()};owner=regs['esi']
        # RET8 advances ESP by12. Payload starts200 bytes below the saved return.
        callback_return_slot=regs['esp']-12
        payload=callback_return_slot-(0xD0-8)
        count=word(owner+0xC8)
        assert count==150 and int(e['address'],16)==regs['eip']==150
        assert word(callback_return_slot)==count
        assert word(owner+0xD4)==150 and word(owner+0xD0)==0
        matches=[]
        for offset in range(0,count*4,4):
            a,b=word(owner+offset),word(payload+offset)
            if a is not None and b is not None and a==b:matches.append(offset)
        assert all(offset in matches for offset in range(0xB0,0x258,4))
        direct=[p for p in coverage['phases'] if p['caller']=='0066E5F8']
        assert len(direct)==1 and direct[0]['count']==150 and direct[0]['exact_roster_recorded']
        assert not direct[0]['all_ready_in_recorded_sweep']
        # The previously broken parser must retain the new startup payload key.
        startup=next(r for r in records if r['type']=='DSFV29s_BASELINE_START')
        assert startup['field_type']=='0' and startup['status']=='1'
        results.append(dict(log=str(log),log_sha256=hashlib.sha256(log.read_bytes()).hexdigest(),dump=str(dump),dump_sha256=info['sha256'],
            pid=info['process_id'],exception=e['code'],eip=hex(regs['eip']),esp=hex(regs['esp']),owner=hex(owner),
            requested_count=count,persistent_active=word(owner+0xD4),persistent_free=word(owner+0xD0),
            callback_return_slot=hex(callback_return_slot),payload_start=hex(payload),
            native_payload_capacity_ids=50,copy_bytes=600,overflow_bytes=400,
            corrupted_return_value=word(callback_return_slot),first_overflow_source='owner+0xC8 request count',
            matching_owner_to_stack_words=len(matches),contiguous_match_offsets=['0xB0','0x258'],
            callback_pointer_captured=word(owner+0xE68) is not None,
            direct_request_ready_sweeps=max(int(r.get('checkpoints',0)) for r in records if r['type']=='DSFV29s_BASELINE_PHASE' and r.get('seq')==str(direct[0]['seq'])),
            named_events=[r for r in records if r['type']=='DSFV29s_CHAPTER_EVENT' and r.get('caller')=='0076FE9A']))
    result=dict(result='passed',method='static native-byte validation and captured-memory arithmetic; no native execution',
        native_spans={f'{k:08X}':v for k,v in spans.items()},pairs=results,
        conclusion='The150-count copy into the50-ID callback payload overwrites its return address with owner+0xC8=150, matching both dumps.',
        required_fix=['Expand notification payload frame and all affected stack argument/epilogue offsets.',
            'Obtain IDs51..150 from the request sidecar; owner inline storage contains only50 IDs.',
            'Audit the recording/replay writer and reader before extending the packet.',
            'Replay the complete callback with production bytes in a bounded fixture before a new live build.'],
        limitations=['Small minidumps do not capture owner+E68; its live pointer/vtable is inferred through native registration and the exact copy footprint.',
            'This proves this overflow mechanism, not the absence of additional downstream limits.',
            'No new ASI or behavior fix was built by this analysis.'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('native_spans','pairs')},indent=2))
if __name__=='__main__':main()

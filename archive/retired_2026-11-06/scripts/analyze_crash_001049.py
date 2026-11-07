"""Read supplied minidump and verify the callback-arena corruption geometry.

No process/game execution or mutation. The replay uses a Python memory map,
following the decoded native constructor/insertion instructions.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from analyze_audio_model192_exit_dump import analyze_dump
from analyze_driver_hang_dump import Dump
from static_streaming_audit import PE, DUMPBIN, EXE


def replay(bad_receiver):
    origin, stride, sentinel_offset = 0x14427EC, 0x314, 0x30C
    memory = {}
    def read(at):return memory.get(at, 0)
    def write(at, value):memory[at] = value & 0xFFFFFFFF
    for slot in range(21):
        base = origin + slot * stride
        end = base + sentinel_offset
        for at, value in ((base,base+12),(base+4,0),(base+8,0),(end,end),(end+4,end)):
            write(at,value)
    base, end = origin+9*stride, origin+9*stride+sentinel_offset
    next_base, next_end = origin+10*stride, origin+10*stride+sentinel_offset
    milestones = []
    for ordinal in range(1,151):
        node = read(base)
        payload = bad_receiver if ordinal==130 else 0x50000000+ordinal*0x100
        # 005567D0: payload write; recycled or fresh cursor; active increment;
        # append to the retained in-object sentinel. No end/capacity comparison.
        write(node+8,payload)
        if read(base+4):
            following = read(node)
            write(base+4,read(base+4)-1)
            write(base,following)
        else:
            write(base,read(base)+12)
        write(base+8,read(base+8)+1)
        previous = read(end)
        write(node,previous)
        write(node+4,end)
        write(read(end)+4,node)
        write(end,node)
        if ordinal in (64,65,130,131,150):
            milestones.append(dict(ordinal=ordinal,node=hex(node),payload_address=hex(node+8),
                next_header_cursor=hex(read(next_base)),next_active=hex(read(next_base+8)),
                next_sentinel_previous=hex(read(next_end)),next_sentinel_next=hex(read(next_end+4))))
    iterator = read(next_end+4)
    return dict(pool9=hex(base),pool10=hex(next_base),pool10_sentinel=hex(next_end),milestones=milestones,
                replay_iterator=hex(iterator),replay_receiver=hex(read(iterator+8)),
                conditional=True,assumptions='150 unrecycled native inserts into pool9; payload130 set to the captured receiver to compare the exact addresses. This is not a captured insertion history.')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('dump',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    summary=analyze_dump(args.dump);d=Dump(args.dump);pe=PE()
    assert hashlib.sha256(pe.data).hexdigest()=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    ex=summary['exception'];r={k:int(v,16) for k,v in ex['registers'].items()}
    assert ex['code']=='0xc0000005' and r['eip']==0 and r['edx']==0
    assert d.u32(r['esp'])==0x85903B and pe.read(0x859039,2)==bytes.fromhex('ffd2')
    assert pe.read(0x859031,10)==bytes.fromhex('8b4f088b018b500cffd2')
    assert d.u32(r['ecx'])==r['eax'] and d.u32(r['eax']+12)==0
    assert pe.read(0xDB40A0,10)==bytes.fromhex('ba14000000b8f0274401')
    assert pe.read(0xDB40CF,5)==bytes.fromhex('0514030000')
    assert pe.read(0x5567FF,3)==bytes.fromhex('83010c')
    assert pe.read(0x556805,6)==bytes.fromhex('8b910c030000')
    assert d.u32(r['esp']+20)==0x5A2131
    handler=r['esi'];vtable=d.u32(handler)
    assert vtable==0xE37118 and struct.unpack('<I',pe.read(vtable+0x24,4))[0]==0x859000
    slot=d.u32(handler+0x560);assert slot==10
    base=0x14427EC+slot*0x314;end=base+0x30C;first=base+12
    assert r['edi']==end-8 and d.u32(r['esp']+16)==end
    result=replay(r['ecx'])
    assert int(result['replay_iterator'],16)==r['edi']
    assert int(result['replay_receiver'],16)==r['ecx']
    # Supported native boundaries, including the complete constructor and insert.
    spans=[(0xDB40A0,0xDB40DC),(0x5567D0,0x556821),(0x859000,0x859111),
           (0x5A20C0,0x5A21CE),(0x5A8030,0x5A807E),(0x5A8300,0x5A83EC),(0x5A8460,0x5A8482)]
    for start,stop in spans:
        text=subprocess.run([str(DUMPBIN),'/nologo','/nopdb','/disasm:bytes',f'/range:0x{start:X},0x{stop:X}',str(EXE)],capture_output=True,text=True,check=True).stdout
        (args.output/f'{start:08X}.verified.asm.txt').write_text(text,encoding='utf-8')
    baseline=PE(Path(__file__).resolve().parents[1]/'dist/MAIN_150/DriverSF_V29s_160Capacity_150Request_PENDING512_R1.asi')
    loaded=next(m for m in summary['modules'] if m['name'].endswith('DriverSF_V29s_160Capacity_150Request_PENDING512_R1.asi'))
    baseline_match=loaded['timestamp']==struct.unpack_from('<I',baseline.data,baseline.pe+8)[0] and loaded['size']==struct.unpack_from('<I',baseline.data,baseline.pe+24+56)[0]
    proof=dict(result='passed',process_id=summary['process_id'],dump_sha256=summary['sha256'],
        native_exe_sha256=hashlib.sha256(pe.data).hexdigest(),
        exception=ex,baseline_module=loaded,baseline_metadata_matches_staged=baseline_match,
        chapter_extension_loaded=any('CHAPTERS' in m['name'] for m in summary['modules']),
        callback=dict(handler=hex(handler),handler_vtable=hex(vtable),slot=slot,header=hex(base),sentinel=hex(end),
            iterator=hex(r['edi']),iterator_alignment_remainder=(r['edi']-first)%12,
            receiver_slot_address=hex(r['edi']+8),receiver=hex(r['ecx']),receiver_first_word=hex(r['eax']),
            method_slot_address=hex(r['eax']+12),method_slot_value=0,
            handler_receiver_field=hex(d.u32(handler+0x55C)),receiver_payload_word=hex(d.u32(r['ecx']+8)),
            global_callback_header_captured=d.read(base,12) is not None,
            inline_capacity=64,node_bytes=12,global_groups=21),
        conditional_replay=result,
        released_key=dict(stack_address='0019FC14',raw=hex(d.u32(0x19FC14)),decimal=d.u32(0x19FC14),
            note='1248 is numerically248+1000, consistent with a documented variant convention; exact model/variant semantics not proved here.'),
        user_context='Starting a Challenge that switches traffic, per user report.',
        limitations=['Global callback arena omitted from minidump; no active/free/issued count or first overwriting store.',
                     'Replay demonstrates a native overflow mechanism matching the iterator geometry, not the actual full history.',
                     'Frame-pointer chain is invalid here because EBP is a container pointer; only bounded decoded stack frames are used.',
                     'No proof parked-car fallback shares this cause; CHAPTERS changes roster selection, not callback storage.'])
    (args.output/'callback_proof.json').write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:proof[k] for k in ('result','process_id','baseline_metadata_matches_staged','chapter_extension_loaded','callback','conditional_replay')},indent=2))
    d.close()


if __name__=='__main__':main()

"""Verify the frozen streaming loop and its list topology from Driver.dmp."""
import hashlib
import json
from pathlib import Path
import re
import struct
from analyze_driver_hang_dump import Dump
from static_streaming_audit import PE
from audio_static_audit import EXPECTED

ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'analysis/driver_hang_dump_2026-11-05'
BUILD=ROOT/'build/v29s_cleanup262144_r1';NAME='DriverSF_V29s_160Capacity_150Request_CLEANUP262144_R1'


def main():
    summary=json.loads((OUT/'dump_summary.json').read_text());d=Dump(ROOT/summary['dump']);native=PE()
    try:
        assert hashlib.sha256(native.data).hexdigest()==EXPECTED
        thread=next(t for t in summary['threads'] if (t.get('registers') or {}).get('eip')=='0x5a237c')
        assert thread['context_source']=='live_amd64_context_in_cs23'
        entry=native.base+struct.unpack_from('<I',native.data,native.pe+24+16)[0]
        assert int(thread['info']['start_address'],16)==entry
        r={k:int(v,16) for k,v in thread['registers'].items()};s=r['esi'];sent=s+0x6998
        assert r['edx']==sent
        expected=bytes.fromhex('663b690872078b49043bca75f3')
        assert d.read(0x5A2373,len(expected))==native.read(0x5A2373,len(expected))==expected
        def node(at):
            b=d.read(at,16)
            if b is None:return None
            prev,nxt,key,value=struct.unpack('<4I',b)
            return dict(address=f'{at:08X}',prev=f'{prev:08X}',next=f'{nxt:08X}',key=key&0xFFFF,
                        key_raw=f'{key:08X}',value=f'{value:08X}')
        def walk(start,sentinel,bound=4096):
            seen={};nodes=[];at=start
            for i in range(bound):
                if at==sentinel:return dict(termination='sentinel',nodes=nodes)
                if at in seen:return dict(termination='cycle',repeat=f'{at:08X}',cycle_start=seen[at],
                    cycle_length=len(nodes)-seen[at],nodes=nodes)
                seen[at]=len(nodes);record=node(at)
                if record is None:return dict(termination='unreadable',at=hex(at),nodes=nodes)
                nodes.append(record);at=int(record['next'],16)
            return dict(termination='bound',nodes=nodes)
        cycle=walk(r['ecx'],sent)
        assert cycle['termination']=='cycle' and cycle['cycle_start']==0 and cycle['cycle_length']==12
        assert all(n['key']<=r['ebp'] and int(n['address'],16)!=sent for n in cycle['nodes'])
        # At this PC the next loop reads key, follows +4, compares sentinel.
        # No writes or calls in the repeating instruction block can repair it.
        source=PE(BUILD/(NAME+'.asi'))
        module=next(m for m in d.modules if m['name'].lower().endswith(NAME.lower()+'.asi'))
        stamp=struct.unpack_from('<I',source.data,source.pe+8)[0]
        size=struct.unpack_from('<I',source.data,source.pe+24+56)[0]
        assert stamp==module['timestamp'] and size==module['size']
        symbols={}
        for line in (BUILD/(NAME+'.map')).read_text().splitlines():
            m=re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})',line)
            if m:symbols[m[1]]=int(m[2],16)-source.base+module['base']
        def symbol(prefix):return next(v for k,v in symbols.items() if k.startswith(prefix))
        census={}
        for prefix in ('?g_pkg0@','?g_sIncludeArenasUsed@','_DSFV29s_IncludeRedirects','_DSFV29s_IncludeFault',
            '_DSFV29s_IncludeHigh','_DSFV27B_MapExternal','_DSFV27B_MapMaxActive','?g_cleanupBase@','?g_cleanupPeak@',
            '?g_cleanupDrained@','?g_cleanupDepth@','?g_cleanupDrainMismatch@'):
            at=symbol(prefix);census[prefix]=dict(address=f'{at:08X}',value=d.u32(at))
        assert census['?g_pkg0@']['value']==s
        fields={f'{off:04X}':f'{d.u32(s+off):08X}' for off in
            (0x10,0x5B44,0x5B48,0x5B4C,0x5F50,0x5F54,0x5F5C,0x5F60,0x5F64,0x6368,0x636C,
             0x6374,0x6378,0x637C,0x6680,0x6684,0x668C,0x6690,0x6694,0x6998,0x699C,0x69A4,0x69A8,0x69AC)}
        maps={}
        for name,header,begin,end,stride in (
            ('completed',0x5B44,0x5B50,0x5F50,16),('pending',0x5F5C,0x5F68,0x6368,16),
            ('intermediate',0x6374,0x6380,0x6680,12),('include',0x668C,0x6698,0x6998,12)):
            cursor,free,active=(d.u32(s+header+i*4) for i in range(3))
            maps[name]=dict(header=f'{s+header:08X}',cursor=f'{cursor:08X}',free=free,active=active,
                inline_begin=f'{s+begin:08X}',sentinel=f'{s+end:08X}',stride=stride,inline_capacity=(end-begin)//stride,
                cursor_distance_from_begin=cursor-s-begin,cursor_distance_in_nodes=(cursor-s-begin)/stride,
                list=walk(d.u32(s+end+4),s+end))
        pending_base=s+0x5F68
        overlapping=node(pending_base+114*16)
        assert pending_base+114*16+4==s+0x668C
        assert overlapping['next']==fields['668C'] and overlapping['key_raw']==fields['6690'] and overlapping['value']==fields['6694']
        pending_spans={
            0x5A1A98:'81c15c5f0000e89d0c0000',
            0x5A1EF0:'8d8d5c5f0000e845080000',
            0x5A21BE:'8d8e5c5f0000e877050000',
            0x5A2313:'8bcf66896c2428e8514f2300',
            0x5A277B:'8bcee8ee4a2300',
            0x7D72AC:'830110ff4108',
            0x5A1BEA:'8d8e685f0000898e5c5f0000',
            0x5A1BDC:'8d866863000089866c6300008900',
        }
        for at,signature in pending_spans.items():
            assert native.read(at,len(bytes.fromhex(signature)))==bytes.fromhex(signature),hex(at)
        script=(ROOT/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
        bridge=script[script.index('UPtr __fastcall MapInsertBridge('):script.index('// Selector node sidecar,')]
        assert 'map==g_pkg0+0x5B44u' in bridge and '0x5F5C' not in bridge and '0x5f5c' not in bridge
        cleanup_base=census['?g_cleanupBase@']['value']
        guard_bad=sum(d.u32(cleanup_base+off)!=0x51473236 for off in (-16,-12,-8,-4,1048576,1048580,1048584,1048588))
        assert guard_bad==0 and d.u32(0x1732BF0+0x17E090)==0 and census['?g_cleanupDepth@']['value']==0
        # Verify the immediate stack return using the native prologue: SUB14,
        # PUSH ESI/EDI/EBX/EBP -> total36 bytes below the return address.
        ret=d.u32(r['esp']+36);assert ret==0x5A24CD and native.read(ret-5,5)==bytes.fromhex('e863fdffff')
        parent_ret=d.u32(r['esp']+48);assert parent_ret==0x59D5FA and native.read(parent_ret-5,5)==bytes.fromhex('e8764e0000')
        result=dict(result='passed',dump_pid=summary['pid'],dump_sha256=summary['sha256'],thread=thread['id'],
            executable_entry=f'{entry:08X}',thread_count=len(summary['threads']),
            eip=f'{r["eip"]:08X}',section=f'{s:08X}',section_state=d.u32(s+0x10),sentinel=f'{sent:08X}',
            target_key=r['ebp'],loop_cycle=cycle,include_list=walk(d.u32(sent+4),sent),
            verified_stack_returns=[f'{ret:08X}',f'{parent_ret:08X}'],fields=fields,maps=maps,census=census,
            pending_node114_aliasing_include_header=overlapping,
            verified_pending_native_spans={f'{a:08X}':b for a,b in pending_spans.items()},
            pending_map_expanded_by_current_MapInsertBridge=False,
            pending_node64_aliases_original_sentinel=pending_base+64*16==s+0x6368,
            pending_active_plus_recycled=maps['pending']['active']+maps['pending']['free'],
            cleanup_snapshot=dict(count=d.u32(0x1732BF0+0x17E090),guard_bad=guard_bad,
                peak=census['?g_cleanupPeak@']['value'],drained=census['?g_cleanupDrained@']['value'],
                depth=census['?g_cleanupDepth@']['value'],mismatch=census['?g_cleanupDrainMismatch@']['value']),
            map_pending_offset_references=native.refs(0x5F5C,False),
            map_pending_begin_references=native.refs(0x5F68,False),
            map_pending_end_references=native.refs(0x6368,False),
            limitations=['Cycle and current loop failure are directly captured; first corrupting store is not.',
                'Node114 overlap is a structural alias/corruption signature, not historical writer attribution.',
                'Module timestamp/image-size match identifies layout but is not a byte-for-byte loaded-module hash.'])
        (OUT/'streaming_cycle.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
        print(json.dumps({k:result[k] for k in ('result','dump_pid','thread','eip','section','section_state','sentinel','target_key',
            'verified_stack_returns','fields','census','pending_node114_aliasing_include_header',
            'map_pending_offset_references','map_pending_begin_references','map_pending_end_references')},indent=2))
    finally:d.close()


if __name__=='__main__':main()

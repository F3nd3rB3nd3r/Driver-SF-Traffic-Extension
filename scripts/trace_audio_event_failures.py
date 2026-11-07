"""Bounded static evidence and supplied-log correlation; never executes the game."""
from collections import Counter
import hashlib
import json
import re
from pathlib import Path
from static_streaming_audit import PE
from audio_static_audit import EXPECTED

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_event_failure_trace_2026-11-05'
LOG=ROOT/'dist/MAIN_150/DSFV29s_audio_slots2048_30860.log'
SPANS={
    0xC54006:('e845ffffff','event index lookup call'),
    0xC5400B:('3bc6894424205e','lookup result observation before null branch'),
    0xC54012:('744e','null event definition -> zero return'),
    0xC5404D:('e85eb00000','playing-record creation'),
    0xC54052:('83f8017411','creation result !=1 -> cleanup/zero'),
    0xC54057:('8b4c241c8b11','selected creation-failure observation'),
    0xC54062:('33c083c42cc3','common failure zero return'),
    0xC5F0C7:('6a3850','fixed56-byte request and pool argument'),
    0xC5F0CA:('e871a90300','Malloc for playing record'),
    0xC5F0D8:('0f84be000000','null allocation -> creation failure'),
    0xC5F1A5:('b802000000','only ordinary creation failure result2'),
    0xC99AA5:('5e5b8bc75f','existing Malloc probe spans nonzero-size return'),
    0x84CC98:('83be6c01000010','per-object playing-entry limit16'),
    0x84CC9F:('7367','full -> wrapper zero without PostEvent'),
    0x84CCC4:('e8d7804000','wrapper numeric PostEvent call'),
    0x84CCC9:('83c41c3bc3','wrapper result observation'),
    0x84CCDA:('8d0c898d4c8e2c','20-byte event records starting at object+2C'),
    0x84CCF8:('ff866c010000','increment count after nonzero post'),
    0x84CD0A:('33c0','wrapper shared zero result'),
    0x84C988:('750d','only callback type1 takes record-removal call'),
    0x84C992:('e879040000','callback invokes erase helper with object+2C header'),
    0x84CE5F:('ff8940010000','erase decrements header+140 = object+16C'),
    0xC525C2:('b8c59d1c81','native FNV seed'),
    0xC525D3:('69c0930100014133c6','native FNV multiply before XOR'),
    0xC526B6:('3c417c093c5a7f0504208804','ASCII uppercase normalization'),
    0xC52BC7:('3ddb3ba32c0f44c1','SetState handles hash none specially'),
}

def event_hash(value):
    result=0x811C9DC5
    for c in value.lower().encode('ascii'):result=((result*0x1000193)&0xFFFFFFFF)^c
    return result

def main():
    OUT.mkdir(exist_ok=True)
    pe=PE();assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    spans=[]
    for address,(signature,role) in SPANS.items():
        assert pe.read(address,len(bytes.fromhex(signature)))==bytes.fromhex(signature),(hex(address),role)
        spans.append(dict(address=f'{address:08X}',signature=signature,role=role))
    wanted={0x2CA33BDB,0x38C61846,0xDFBD7040,0xA6073B55,0x2E385822,0x12E6FA03,0x72DCBB24}
    matches=[]
    for section in pe.sections:
        if section['flags']&0x20000000:continue
        data=pe.data[section['offset']:section['offset']+section['size']]
        for encoding,pattern in [('ascii',rb'[\x20-\x7e]{3,}\x00'),('utf16',rb'(?:[\x20-\x7e]\x00){3,}\x00\x00')]:
            for match in re.finditer(pattern,data):
                name=match[0][:-1 if encoding=='ascii' else -2].decode('ascii' if encoding=='ascii' else 'utf-16le')
                value=event_hash(name)
                if value in wanted:matches.append(dict(address=f'{section["va"]+match.start():08X}',encoding=encoding,string=name,hash=f'{value:08X}'))
    assert event_hash('none')==0x2CA33BDB and event_hash('Menu_Move')==0x72DCBB24
    details=[];last_status=None
    for number,line in enumerate(LOG.read_text().splitlines(),1):
        parts=line.split()
        if not parts:continue
        row=dict(piece.split('=',1) for piece in parts[1:] if '=' in piece)
        if parts[0]=='DSFV29s_AUDIO_STATUS':last_status=row
        if parts[0]=='DSFV29s_AUDIO_DETAIL' and row.get('kind')=='postZero':details.append(dict(line=number,**row))
    assert last_status and len(details)==int(last_status['postZero'])==123
    counts=Counter(r['event'] for r in details)
    classification=dict(total=123,none_hash=counts['2CA33BDB'],menu_move_hash=counts['72DCBB24'],
        literal_zero_event=counts['00000000'],other_unresolved_event_hashes=123-counts['2CA33BDB']-counts['72DCBB24']-counts['00000000'],
        malloc_failures=int(last_status['mallocFailed']),malign_failures=int(last_status['malignFailed']),
        early_post_zero=int(last_status['postEarlyZero']),events=[dict(event=e,count=n,
            distinct_game_objects=len({r['gameObject'] for r in details if r['event']==e}),
            game_objects=sorted({r['gameObject'] for r in details if r['event']==e}),
            callers=sorted({r['caller'] for r in details if r['event']==e})) for e,n in counts.most_common()])
    result=dict(executable_sha256=EXPECTED,verified_instruction_spans=spans,
        per_object_layout=dict(entries_offset=0x2C,entry_bytes=20,capacity=16,count_offset=0x16C,payload_bytes=0x170,
            equation='0x2C + 16*20 == 0x16C; count DWORD ends at0x170',existing_probes_measure_full_refusal=False),
        log_sha256=hashlib.sha256(LOG.read_bytes()).hexdigest(),classification=classification,
        runtime_inference='Missing event definitions strongly indicated for ordinary zero returns: only competing ordinary branch needs a56-byte Malloc failure, absent from existing measurements. Direct branch outcome still unmeasured.',
        counter_wrap_exception='A generated playing ID could be zero on32-bit wrap; no wrap instrumentation, implausible explanation for repeated failures in this run.',
        limitations='Native signatures verified; no ASI build, debugger or game execution. Hash matches do not prove which string supplied a runtime ID or its car association.')
    (OUT/'verified_failure_trace.json').write_text(json.dumps(result,indent=2))
    (OUT/'event_name_hash_matches.json').write_text(json.dumps(dict(native_hash='ASCII lowercase then FNV-1',
        matches=matches,unmatched_targets=[f'{h:08X}' for h in sorted(wanted-{int(r['hash'],16) for r in matches})]),indent=2))
    print(json.dumps(dict(verified_spans=len(spans),classification=classification,
        resolved_names={'2CA33BDB':'none','72DCBB24':'Menu_Move'},per_object_full_refusal_unmeasured=True),indent=2))

if __name__=='__main__':main()

"""Verify bounded native evidence for the shared45-entry model-audio registry."""
import hashlib
import json
import struct
from pathlib import Path
from static_streaming_audit import PE
from audio_static_audit import EXPECTED

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'analysis/audio_vehicle_shared_static_2026-11-05'
SPANS={
    0xD39A20:('68a00000006800705500e8c10ab3ff','serviceA0 factory registration'),
    0x557000:('68f0020000','factory allocates2F0 bytes'),
    0x557013:('c7002438de00','provider vtable'),
    0x557019:('8d4810894804','inline node cursor starts at object+10'),
    0x55701F:('8d88e00200008988e40200008909','sentinel at2E0, linked to itself'),
    0x557031:('89500889500c','recycled/active counters zero'),
    0x556E30:('83790c2d7313','DWORD active>=45 silently skips registration'),
    0x556E3B:('83c104e81d010000','map header object+4 passed to find/insert'),
    0x556E43:('8b4c24048908','store supplied profile pointer in keyed node'),
    0x556E49:('c20800','full branch returns without failure result'),
    0x556E04:('8981ec020000','set common/default data pointer'),
    0x556E10:('8b81ec020000c3','get common/default data pointer'),
    0x556EA0:('8d7704','model lookup uses same map'),
    0x556EA6:('e865000000','lookup calls keyed map finder'),
    0x556EC7:('8b400c','lookup returns node value pointer'),
    0x556ED0:('33c083c408c20400','missing model lookup returns null'),
    0x556F27:('394a087d07','map key is DWORD signed comparison'),
    0x556F2C:('8b5204','map traversal follows next pointer'),
    0x7D72AC:('830110','insert advances inline cursor by16'),
    0x7D72AF:('ff4108','insert increments header+8 = object+C'),
    0x55388F:('8b90ac050000895610','shared setup takes DWORD vehicle model key from+5AC'),
    0x5538AF:('8bb880020000','model-data provider at engine+280'),
    0x5538B5:('8b078b5010','provider virtual slot10 model lookup'),
    0x5538BD:('ffd28986bc000000','save model profile at controller+BC'),
    0x5538C5:('8b078b50148bcfffd2','provider virtual slot14 common data lookup'),
    0x5538DD:('c7461001000000','missing profile changes key to fallback1'),
    0x553913:('e868bef3ff84c00f84b5010000','matching bank-job readiness failure exits setup'),
    0x553920:('83bebc000000000f84a8010000','missing model profile exits setup'),
    0x55392D:('83bec0000000000f849b010000','missing common data exits setup'),
    0x553AD5:('32c05ec20800','shared setup returns false'),
    0x48F798:('83790403','bank-list search filters type3'),
    0x48F79E:('397108','bank-list search matches full model key'),
    0x48F7B5:('83781404750f83780c00','bank job completed state4 and nonzero native bankID required'),
    0x54A6D1:('83f8057cca','separate heavy controller pool scans5 slots'),
    0x55522C:('83f80772ef','separate scene controller acquisition scans7 slots'),
}

def main():
    pe=PE();assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    OUT.mkdir(exist_ok=True);verified=[]
    for at,(signature,role) in SPANS.items():
        assert pe.read(at,len(bytes.fromhex(signature)))==bytes.fromhex(signature),(f'{at:08X}',role)
        verified.append(dict(address=f'{at:08X}',bytes=signature,role=role))
    u32=lambda at:struct.unpack('<I',pe.read(at,4))[0]
    assert u32(0xDE3828)==0x556E30 and u32(0xDE3834)==0x556E90 and u32(0xDE3838)==0x556E10
    assert 0x10+45*16==0x2E0 and 0x2E0+16==0x2F0
    assert 0x556E34+2+pe.read(0x556E35,1)[0]==0x556E49
    # Decode only the actual six-byte native capacity gate: it takes an unsigned
    # DWORD count, and has the same before/at/after boundary regardless of RAM.
    cap=pe.read(0x556E33,1)[0]
    cases=[dict(active=n,capacity=cap,skips_registration=(n>=cap)) for n in (0,44,45,46,150)]
    assert cases[1]['skips_registration'] is False and cases[2]['skips_registration'] is True
    result=dict(result='passed',executable_sha256=EXPECTED,verified_instruction_spans=verified,
        provider=dict(service_id=160,factory='00557000',vtable='00DE3824',engine_offset='280',
            capacity=45,object_bytes=752,header_offset=4,active_offset=12,node_offset=16,node_stride=16,
            sentinel_offset=736,common_data_offset=748,key_bits=32,registration='00556E30',lookup='00556E90'),
        native_gate_cases=cases,
        shared_setup='00553860',other_unmeasured_fixed_pools=dict(heavy_controller_slots=5,scene_controller_slots=7),
        runtime_registration_count_measured=False,runtime_refusals_measured=False,
        limitations='Static identities/layout/control flow verified. Does not prove live table is full, which models are retained, or whether fallback1 succeeds. No ASI change/build/game/debugger.')
    (OUT/'verified_model_audio_registry.json').write_text(json.dumps(result,indent=2))
    print(json.dumps(dict(result='passed',verified_spans=len(verified),model_audio_capacity=45,inline_nodes=45,gate_cases=cases,
        runtime_saturation_measured=False),indent=2))

if __name__=='__main__':main()

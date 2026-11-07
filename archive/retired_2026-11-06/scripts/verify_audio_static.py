"""Verify the bounded audio audit inventory against Driver.exe; never execute it."""
import hashlib
import json
import struct
from pathlib import Path
from audio_static_audit import OUT, EXPECTED, exports
from static_streaming_audit import PE


def main():
    pe=PE();assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    ex=exports(pe);assert len(ex)==157
    def exported(prefix,address):
        matches=[r for r in ex if r["name"].startswith(prefix)]
        assert matches and all(int(r["address"],16)==address and not r["forwarded"] for r in matches)
    exported("?PostEvent@SoundEngine@AK@@YAKKI",0xC54DA0)
    exported("?LoadBank@SoundEngine@AK@@YA?AW4AKRESULT@@PBXKP6",0xC51ED0)
    exported("?Malloc@MemoryMgr@AK",0xC99A40)
    exported("?Malign@MemoryMgr@AK",0xC99AB0)
    for prefix in ("?PostCode@Monitor@AK","?PostString@Monitor@AK","?SetLocalOutput@Monitor@AK"):
        exported(prefix,0xC52550)
    assert pe.read(0xC52550,6)==bytes.fromhex("b8 04 00 00 00 c3")
    assert pe.read(0x6754A0,3)==bytes.fromhex("33 c0 c3")
    assert pe.read(0xC99C0F,3)==bytes.fromhex("ff 49 18")
    budgets=[(0x84A4BC,3,20,"poolDescriptorLimit"),
             (0x84A5D5,1,0x88000,"FilePackagesPoolBytes"),
             (0x84A63F,1,0x4B000,"PrepareEventPoolBytes"),
             (0x84A683,1,0x42B800,"StaticBanksPoolBytes"),
             (0x84A64D,3,0x599999,"DefaultPoolRequestedBytes"),
             (0x84A646,3,0x300000,"PlatformPoolRequestedBytes"),
             (0xC50F29,3,0x40000,"DefaultCommandQueueBytes")]
    values=[]
    for site,offset,expected,name in budgets:
        actual=struct.unpack("<I",pe.read(site+offset,4))[0];assert actual==expected
        values.append(dict(name=name,instruction=f"{site:08X}",value=actual))
    assert (0x92D4-0xEA4)//0x178==90 and (0x92D4-0xEA4)%0x178==0
    assert (0xE90-0xA58)//12==90
    assert pe.read(0x84AA29,7)==bytes.fromhex("83 be a0 0e 00 00 5a")
    assert pe.read(0x84AABF,6)==bytes.fromhex("c7 00 00 00 00 00")
    candidates=[(0x84BE74,"c7 40 14 04 00 00 00","bank callback, before unconditional completion"),
        (0x84AA29,"83 be a0 0e 00 00 5a","emitter admission compare, owner ESI"),
        (0xC54DA0,"83 ec 10 53 57","numeric PostEvent entry"),
        (0xC54E17,"8b c6 5e 5f 5b 83 c4 10","normal numeric PostEvent return, playing ID ESI"),
        (0xC54DD3,"5f 5b 83 c4 10","external-source allocation failure return"),
        (0xC99AA5,"5e 5b 8b c7 5f","nonzero-size Malloc return, pointer EDI"),
        (0xC99B04,"5e 5b 8b c7 5f","nonzero-size Malign return, pointer EDI")]
    signatures=[]
    for site,signature,role in candidates:
        stock=bytes.fromhex(signature);assert pe.read(site,len(stock))==stock
        signatures.append(dict(site=f"{site:08X}",signature=signature,role=role))
    # Independent ABI fact from the exact executable's exported decorated name.
    signature=next(r["name"] for r in ex if int(r["address"],16)==0xC51ED0)
    assert "P6AXKW43@JPAX@Z" in signature  # bank ID, AKRESULT, long pool ID, cookie
    frame={40:0x12345678,44:0x34,48:7,52:0x22222222} # after pushfd + pushad
    assert frame[44]==0x34 and frame[48]==7 and frame[44]!=frame[48]
    old=(Path(__file__).resolve().parents[1]/"src/V29sAudioBankTrace.inc").read_text()
    assert "mov edx,dword ptr [esp+48]" in old
    result=dict(executable_sha256=EXPECTED,verified_exports=157,monitor_is_return_4_stub=True,
        budgets=values,native_emitters=90,emitter_node_stride=0x178,emitter_payload=0x170,
        pool_record_stride=0x4C,pool_table_pointer="01D72390",max_pools="01D72388",active_pools="01D7238C",
        used_offset="44",usable_total_offset="48",
        allocation_modes={"malloc_malign":"byte accounting at +44",
            "attributes_bit_08":"fixed-block mode: count +00, size +04, free count +18; GetBlock does not update +44"},
        bank_callback_arguments={
            "original_esp_plus_4":"bank ID","original_esp_plus_8":"AKRESULT",
            "original_esp_plus_C":"pool ID","original_esp_plus_10":"cookie"},
        old_callback_trace_captured_pool_id=True,candidate_signatures=signatures,
        limitations="Static signatures and layout only. No live saturation, allocation failure or playback result established. Candidate hooks still require branch/bridge/integration verification before building a probe.")
    OUT.mkdir(exist_ok=True)
    (OUT/"verified_inventory.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
    print(json.dumps(result,indent=2))


if __name__=="__main__":main()

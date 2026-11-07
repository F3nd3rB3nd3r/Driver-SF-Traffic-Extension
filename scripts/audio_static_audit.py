"""Bounded read-only audio research on the already authorized exact Driver.exe."""
import argparse
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from static_streaming_audit import PE, DUMPBIN, EXE

OUT=Path(__file__).resolve().parents[1]/"analysis/audio_static_2026-11-05"
EXPECTED="c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616"


def exports(pe):
    export_rva,export_size=struct.unpack_from("<II",pe.data,pe.pe+24+96)
    fields=struct.unpack("<IIHHIIIIIII",pe.read(pe.base+export_rva,40))
    ordinal_base,count,names,functions,name_table,ordinals=fields[5:]
    def string(address):
        value=bytearray()
        for i in range(1024):
            byte=pe.read(address+i,1)[0]
            if not byte:return value.decode("ascii")
            value.append(byte)
        raise ValueError("Unterminated export")
    result=[]
    for i in range(names):
        name_rva=struct.unpack("<I",pe.read(pe.base+name_table+i*4,4))[0]
        index=struct.unpack("<H",pe.read(pe.base+ordinals+i*2,2))[0]
        function_rva=struct.unpack("<I",pe.read(pe.base+functions+index*4,4))[0]
        name=string(pe.base+name_rva)
        if "@AK@@" in name or "@MemoryMgr@AK" in name or "@Monitor@AK" in name:
            result.append(dict(name=name,address=f"{pe.base+function_rva:08X}",ordinal=index+ordinal_base,
                forwarded=export_rva<=function_rva<export_rva+export_size))
    return result


def main(output_default=OUT):
    parser=argparse.ArgumentParser()
    parser.add_argument("--output",type=Path,default=output_default)
    parser.add_argument("command",choices=["exports","disasm","refs","calls","strings"])
    parser.add_argument("values",nargs="*")
    args=parser.parse_args();pe=PE();OUT=args.output
    assert hashlib.sha256(pe.data).hexdigest()==EXPECTED
    OUT.mkdir(exist_ok=True)
    if args.command=="exports":
        result=exports(pe)
        (OUT/"wwise_exports.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
        pattern=re.compile(args.values[0] if args.values else ".")
        for r in result:
            if pattern.search(r["name"]):print(r["address"],r["name"])
        print("Wwise exports",len(result))
    elif args.command=="disasm":
        for pair in args.values:
            lo,hi=(int(v,16) for v in pair.split(":"))
            result=subprocess.run([str(DUMPBIN),"/nologo","/nopdb","/disasm:bytes",
                f"/range:0x{lo:X},0x{hi:X}",str(EXE)],capture_output=True,text=True,check=True)
            p=OUT/f"{lo:08X}-{hi:08X}.asm.txt";p.write_text(result.stdout,encoding="utf-8")
            print(p.name,len(result.stdout.splitlines()),"lines")
    elif args.command in ("refs","calls"):
        result={v:pe.refs(int(v,16),args.command=="calls") for v in args.values}
        (OUT/(args.command+"_"+"_".join(args.values)+".json")).write_text(json.dumps(result,indent=2),encoding="utf-8")
        print(json.dumps(result,indent=2))
    elif args.command=="strings":
        pattern=re.compile(args.values[0],re.I);result=[]
        for section in pe.sections:
            if section["flags"]&0x20000000:continue
            block=pe.data[section["offset"]:section["offset"]+section["size"]]
            for enc,regex in [("ascii",rb"[\x20-\x7e]{4,}"),("utf16",rb"(?:[\x20-\x7e]\x00){4,}")]:
                for m in re.finditer(regex,block):
                    value=m[0].decode("ascii" if enc=="ascii" else "utf-16le")
                    if pattern.search(value):result.append(dict(address=f"{section['va']+m.start():08X}",encoding=enc,text=value))
        (OUT/"selected_strings.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
        print(json.dumps(result,indent=2))


if __name__=="__main__":main()

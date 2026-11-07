#!/usr/bin/env python3
"""Offline PE32/ABI/5 native hook signature checks for V29s (not runtime verification)."""
from pathlib import Path
from argparse import ArgumentParser
import hashlib,struct

class PE:
    def __init__(self,p):
        self.data=p.read_bytes(); d=self.data
        assert d[:2]==b'MZ', 'MZ header missing'
        self.pe=struct.unpack_from('<I',d,0x3c)[0]
        assert d[self.pe:self.pe+4]==b'PE\0\0'
        assert struct.unpack_from('<H',d,self.pe+4)[0]==0x14c,'not x86'
        self.opt=self.pe+24
        assert struct.unpack_from('<H',d,self.opt)[0]==0x10b,'not PE32'
        self.base=struct.unpack_from('<I',d,self.opt+28)[0]
        n=struct.unpack_from('<H',d,self.pe+6)[0]
        off=self.opt+struct.unpack_from('<H',d,self.pe+20)[0]
        self.secs=[]
        for i in range(n):
            a=off+40*i;vs,rv,sz,rp=struct.unpack_from('<IIII',d,a+8)
            self.secs.append((rv,max(vs,sz),rp))
    def offset(self,rva):
        for rv,sz,rp in self.secs:
            if rv<=rva<rv+sz:return rp+rva-rv
        raise ValueError(f'unmapped RVA {rva:x}')
    def bytes(self,va,n):
        off=self.offset(va-self.base)
        return self.data[off:off+n]
    def exports(self):
        d=self.data;drva=struct.unpack_from('<I',d,self.opt+96)[0]
        exp=self.offset(drva)
        names=struct.unpack_from('<I',d,exp+24)[0]
        funcs,namesr,ords=struct.unpack_from('<III',d,exp+28)
        result={}
        for i in range(names):
            nr=struct.unpack_from('<I',d,self.offset(namesr)+i*4)[0]
            at=self.offset(nr);name=d[at:d.index(b'\0',at)].decode('ascii')
            ordidx=struct.unpack_from('<H',d,self.offset(ords)+i*2)[0]
            result[name]=struct.unpack_from('<I',d,self.offset(funcs)+ordidx*4)[0]
        return result

root=Path(__file__).resolve().parents[1]
p=ArgumentParser(description=__doc__)
p.add_argument('--requests',type=int,choices=(45,150),default=150)
p.add_argument('--exe',type=Path,help='Exact audited Driver.exe for SHA/signature verification')
p.add_argument('--asi',type=Path)
a=p.parse_args()
target=a.asi or (root/'build'/('v29s_native_trace' if a.requests==150 else 'v29s_native_trace_45control')/f'DriverSF_V29s_160Capacity_{a.requests}Request_TRACE.asi')
asi=PE(target)
assert struct.unpack_from('<H',asi.data,asi.pe+22)[0]&0x2000, 'not DLL'
exports=asi.exports()
for name,value in {'DSF_Chapter7PreloadConfiguredCount':a.requests,'DSFV29p_ConfigTarget':160,
                   'DSFV29p_ConfigBudgetA':0x9400,'DSFV29p_ConfigBudgetB':0x49400}.items():
    assert name in exports,name
    current=struct.unpack_from('<I',asi.data,asi.offset(exports[name]))[0]
    assert current==value,(name,current,value)
required={'DSFV29s_Hooks','DSFV29s_Reconcile','DSFV29s_GateCalls','DSFV29s_GateBlocked',
          'DSFV29s_Dispatched','DSFV29s_MembershipPass','DSFV29s_TransferCalls','DSFV29s_TransferWithHandler'}
assert required<=exports.keys(),required-exports.keys()
assert 'DSFV29q_ChainSteps' not in exports
assert 'DSFV29r_Enter' not in exports
assert b'KERNEL32.dll' in asi.data and b'PSAPI.dll' in asi.data
sigs={
    0x005A7490:'A1 28 72 FE 00',
    0x0059DB40:'80 B9 B9 4D 07 00 00',
    0x0059D740:'83 EC 14 8B 44 24 18',
    0x0059D778:'8B 4C 24 28 57 51',
    0x0059BAD0:'56 8B F1 80 7E 4C 00',
}
if a.exe:
    sha=hashlib.sha256(a.exe.read_bytes()).hexdigest()
    assert sha=='c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616',sha
    game=PE(a.exe)
    for va,raw in sigs.items():
        sig=bytes.fromhex(raw)
        assert game.bytes(va,len(sig))==sig,f'Signature mismatch at {va:08X}'
src=(root/'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
probe=(root/'src/V29sNativeLoadingTrace.inc').read_text()
assert src.count('DSF_V29S_NATIVE_TRACE')==4
assert 'DiagSInstall()?5:-1' in src
assert 'DiagSPrintStatus(loops)' in src
for va in sigs:
    assert f'0x{va:08X}u' in probe
assert probe.count('DiagFInstallDetour(g_sHooks[')==5
assert 'pushfd' in probe and 'pushad' in probe and 'popfd' in probe and 'popad' in probe
print('PASS: exact Driver.exe SHA and five native signatures' if a.exe else 'SKIP: game binary validation (provide --exe for exact checks)')
print(f'PASS: PE32 x86 DLL / {a.requests} requests / 160 slots / 5-hook V29s exports, no V29q/V29r hooks')
print('PASS: source contains five guarded detours and monitor logger; runtime untested')
print('ASI SHA256:',hashlib.sha256(asi.data).hexdigest())

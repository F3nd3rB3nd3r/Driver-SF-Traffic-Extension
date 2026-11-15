"""Focused policy checks. Default is static; --fixture-only compiles a Win32 fixture.

Never builds/installs an ASI or runs the game. Use the fixture option only after
explicit build authorization. It executes isolated production C++ with stubs.
"""
import argparse
import hashlib
import json
import re
import struct
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'analysis/traffic_policy_r1_2026-11-14'
BUILD = ROOT / 'build/traffic_policy_fixture'
NAME = 'DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1'
ASI = ROOT / 'build/v29s_traffic_policy_r1' / (NAME + '.asi')


def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def static():
    baseline = json.loads((ROOT / 'analysis/unlimiter_static_2026-11-14/source_inventory.json').read_text())
    changed = {'DriverSF_V29s_ARENA_REUSE.vcxproj', 'src/DriverSF_V29b_NativeFastPath.cpp',
        'src/V29sChapterCore.inc', 'src/V29sChapterRuntime.inc', 'src/V29sNativeLoadingTrace.inc'}
    preserved = []
    for row in baseline['inputs']:
        if row['path'] in changed:
            continue
        assert hashlib.sha256((ROOT / row['path']).read_bytes()).hexdigest() == row['sha256'], row['path']
        preserved.append(row['path'])
    project = ET.parse(ROOT / 'DriverSF_V29s_ARENA_REUSE.vcxproj')
    ns = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
    flags = project.find('.//m:PreprocessorDefinitions', ns).text.split(';')
    flags = [f for f in flags if f.startswith('DSF')]
    assert len(flags) == len(set(flags))
    assert set(flags) == set(baseline['active_definitions']) | {'DSF_V29S_TRAFFIC_POLICY=1'}
    assert project.find('.//m:TargetName', ns).text == 'DriverSF_V29s_160Capacity_150Request_TRAFFIC_POLICY_R1'
    assert project.find('.//m:OutDir', ns).text == '$(ProjectDir)build\\v29s_traffic_policy_r1\\'
    for item in project.findall('.//m:ClInclude', ns):
        assert (ROOT / item.attrib['Include'].replace('\\', '/')).is_file(), item.attrib
    source = (ROOT / 'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    core = (ROOT / 'src/V29sChapterCore.inc').read_text()
    runtime = (ROOT / 'src/V29sChapterRuntime.inc').read_text()
    probe = function(source, 'static void RecordRequestProbe(')
    assert 'kChapter7' not in probe and 'slot->ids[index]' in probe
    assert 'g_requestProbeExpected[i]' in (ROOT / 'src/V29sNativeLoadingTrace.inc').read_text()
    decide = function(core, 'static __declspec(noinline) ChapterReason ChapterDecide(')
    assert decide.index('if(g_chapterStatus!=1)') < decide.index('return ChapterNative;') < decide.index('ChapterDataMatches(')
    assert 'VerifyExecutable() && ChapterSignature()' in core
    assert 'count=150;' in core and 'assetCheckedOnRequest=0' in runtime
    assert '#if defined(DSF_V29S_TRAFFIC_POLICY) && !defined(DSF_V29S_CHAPTERS)' in source
    dll = source[source.index('extern "C" __declspec(dllexport) BOOL __stdcall DllMain'):]
    assert dll.index('TrafficLoadPolicy(module);') < dll.index('CapacityOpen();') < dll.index('ChapterPreflight()') < dll.index('ApplyPreinitGraphicsExpansion();')
    assert (ROOT / 'dist/MAIN_150/DriverSF_Traffic.ini').read_text().count('RosterMode=Native') == 1
    inputs = ['DriverSF_V29s_ARENA_REUSE.vcxproj', 'src/DriverSF_V29b_NativeFastPath.cpp',
        'src/V29sChapterCore.inc', 'src/V29sChapterRuntime.inc', 'src/V29sCapacityTrace.inc',
        'src/V29sNativeLoadingTrace.inc', 'src/V29sTrafficPolicyCore.inc', 'src/V29sTrafficPolicyConfig.inc',
        'tests/TrafficPolicyTest.cpp', 'scripts/analyze_traffic_policy_log.py',
        'scripts/test_traffic_policy_logs.py', 'scripts/verify_traffic_policy.py', 'dist/MAIN_150/DriverSF_Traffic.ini']
    result = dict(result='passed', scope='Static source/project/inherited-byte checks only',
        preserved_inputs=preserved, added_build_flag='DSF_V29S_TRAFFIC_POLICY=1',
        requests=150, graphics=160, production_compiled=False, fixture_executed=False,
        staged_asi_unchanged=True,
        source_sha256={name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in inputs})
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'static_checks.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    return result


def fixture():
    # Imports/compiler lookup deferred: --static-only cannot launch a compiler
    # or inspect game assets via inherited verification entry points.
    import subprocess
    import winreg
    from static_streaming_audit import DUMPBIN
    BUILD.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    bridge = function(source, 'extern "C" void __fastcall RequestWriteBridge(')
    (BUILD / 'TrafficRequestBody.inc').write_text(bridge.replace('_ReturnAddress()', 'TrafficFixtureCaller()') + '\n')
    probe = function(source, 'static void RecordRequestProbe(')
    (BUILD / 'TrafficRequestProbe.inc').write_text(probe + '\n')
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\Microsoft\Windows Kits\Installed Roots',
            0, winreg.KEY_READ | winreg.KEY_WOW64_32KEY) as key:
        sdk = Path(winreg.QueryValueEx(key, 'KitsRoot10')[0])
    libs = [p / 'um/x86/kernel32.lib' for p in (sdk / 'Lib').iterdir() if re.fullmatch(r'\d+(\.\d+)+', p.name)]
    lib = max((p for p in libs if p.is_file()), key=lambda p: tuple(map(int, p.parents[2].name.split('.'))))
    exe = BUILD / 'TrafficPolicyTest.exe'
    command = [str(DUMPBIN.with_name('cl.exe')), '/nologo', '/O2', '/Oi-', '/GS-', '/GR-', '/Zl', '/EHs-c-',
        '/I' + str(BUILD), '/Fo' + str(BUILD / 'TrafficPolicyTest.obj'), str(ROOT / 'tests/TrafficPolicyTest.cpp'),
        '/link', '/NODEFAULTLIB', '/ENTRY:TrafficPolicyTestMain@0', '/SUBSYSTEM:CONSOLE', '/MACHINE:X86',
        '/OUT:' + str(exe), str(lib)]
    build = subprocess.run(command, cwd=BUILD, capture_output=True, text=True)
    (OUT / 'fixture_build.log').write_text(build.stdout + build.stderr)
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
    (OUT / 'fixture_run.log').write_text(run.stdout + run.stderr)
    assert run.returncode == 0 and run.stdout.strip() == 'traffic policy checks passed', (run.returncode, run.stdout, run.stderr)
    result = dict(result='passed', architecture='Win32',
        source_bridge_sha256=hashlib.sha256(bridge.encode()).hexdigest(),
        source_sha256={name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in (
            'src/DriverSF_V29b_NativeFastPath.cpp', 'src/V29sChapterCore.inc',
            'src/V29sTrafficPolicyCore.inc', 'src/V29sTrafficPolicyConfig.inc', 'tests/TrafficPolicyTest.cpp')},
        checks=['INI parse, BOM, whitespace, comments, invalid/duplicate mode, missing/path/read errors',
            '80 native profiles preserved without AI access', 'noncanonical150, inline prefix, sidecar and actual-ID diagnostic',
            'oversize, missing source and wide-ID rejections retain previous list', 'zero/negative clear semantics',
            '55 preset profiles expand;25 retain their list', 'preset asset/admission/caller guards',
            'executable/setter preflight', 'idle native processing still invoked'],
        limitations='Fake native services, synthetic asset layout and substituted return-address identity. No compiled ASI or live-game certification.')
    (OUT / 'fixture_checks.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    return result


def verify_build():
    """Inspect this candidate; never invoke old configure/build/package chains."""
    import subprocess
    from static_streaming_audit import PE, DUMPBIN
    from test_traffic_policy_logs import run as parser_checks
    checked = static()
    tested = json.loads((OUT / 'fixture_checks.json').read_text())
    assert tested['result'] == 'passed'
    for name, digest in tested['source_sha256'].items():
        assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest, ('fixture stale', name)
    pe = PE(ASI)
    old_path = ROOT / 'build/v29s_replay_read1m_r1/DriverSF_V29s_160Capacity_150Request_REPLAY_READ1M_R1.asi'
    old = PE(old_path)
    assert hashlib.sha256(old.data).hexdigest() == 'fa34ef9c10015bbebd065af8a5e0be8f322688f0c0c395e6aa44a10bdec894db'
    native = PE()
    assert hashlib.sha256(native.data).hexdigest() == 'c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616'
    assert struct.unpack_from('<H', pe.data, pe.pe + 4)[0] == 0x14c
    assert struct.unpack_from('<H', pe.data, pe.pe + 24)[0] == 0x10b and pe.characteristics & 0x2000
    def symbols(path):
        result = {}
        for line in path.read_text().splitlines():
            match = re.match(r'\s+\w{4}:\w{8}\s+(\S+)\s+([0-9A-Fa-f]{8})', line)
            if match:
                result[match[1]] = int(match[2], 16)
        return result
    current_symbols, old_symbols = symbols(ASI.with_suffix('.map')), symbols(old_path.with_suffix('.map'))
    def symbol(table, prefix):
        return next(value for key, value in table.items() if key.startswith(prefix))
    def aliases(table, address):
        return {key for key, value in table.items() if value == address}
    def same_target(before, after):
        def resolve(image, address):
            for _ in range(6):
                data = image.read(address, 5)
                if data[0] != 0xE9:
                    return address
                address += 5 + struct.unpack_from('<i', data, 1)[0]
            raise AssertionError('thunk chain')
        if not aliases(old_symbols, before):
            before = resolve(old, before)
        if not aliases(current_symbols, after):
            after = resolve(pe, after)
        assert aliases(old_symbols, before) & aliases(current_symbols, after), (hex(before), hex(after))
    def span(prefix, size):
        assert old.read(symbol(old_symbols, prefix), size) == pe.read(symbol(current_symbols, prefix), size), prefix
    def function_code(prefix):
        at = symbol(current_symbols, prefix)
        end = min(v for v in current_symbols.values() if v > at)
        return at, end, pe.read(at, end - at)
    def disasm(prefix, label):
        at, end, code = function_code(prefix)
        output = subprocess.run([str(DUMPBIN), '/nologo', '/nopdb', '/disasm:bytes',
            f'/range:0x{at:X},0x{end-1:X}', str(ASI)], capture_output=True, text=True, check=True).stdout
        (OUT / (label + '.asm.txt')).write_text(output)
        return output, code
    imports = subprocess.run([str(DUMPBIN), '/nologo', '/imports', str(ASI)],
        capture_output=True, text=True, check=True).stdout
    (OUT / 'imports.txt').write_text(imports)
    dlls = {n.lower() for n in re.findall(r'^\s+(\S+\.dll)\s*$', imports, re.M | re.I)}
    assert dlls == {'kernel32.dll', 'psapi.dll'}, dlls
    for name in ('GetModuleFileNameW', 'CreateFileW', 'ReadFile', 'GetLastError'):
        assert re.search(r'\b' + name + r'\b', imports), name
    # New default is Native, not an implicit preset on missing configuration.
    policy = symbol(current_symbols, '?g_trafficPolicy@')
    assert struct.unpack('<I', pe.read(policy, 4))[0] == 0
    request_asm, request_code = disasm('@RequestWriteBridge@16', 'request_bridge')
    assert re.search(r'\bret\s+8\b', request_asm, re.I) and b'\xC2\x08\x00' in request_code
    disasm('?ChapterDecide@', 'chapter_decision')
    disasm('?TrafficParse@', 'ini_parser')
    disasm('_DllMain@12', 'dll_entry')
    # Fixed operands, sizes, profiles and native bytes must match the tested parent.
    geometry = [('?g_poolPatches@', 18*24), ('?g_requestFramePatches@', 13*24),
        ('?g_indexedPatches@', 13*16), ('?kNotifyOperands@', 5*12),
        ('?kParkOperands@', 9*12), ('?kParkCalls@', 6*8), ('?kParkRoots@', 2*4),
        ('?kCleanupOperands@', 2*16), ('?kChapterProfiles@', 80*96),
        ('?kChapterSetterBytes@', 198), ('?kChapterRowIds@', 191*4), ('?kChapterRowWeights@', 191*4)]
    for prefix, size in geometry:
        span(prefix, size)
    for prefix, value in [('_DSFV29p_ConfigTarget', 160), ('_DSF_Chapter7PreloadConfiguredCount', 150)]:
        assert struct.unpack('<I', pe.read(symbol(current_symbols, prefix), 4))[0] == value
    # Inline-signature hook tables: same native sites/signatures and same targets.
    table_counts = {}
    for prefix, count, stride in [('?kAudioModelSites@',8,24), ('?kAudioSlotsSites@',9,24),
            ('?kPendingSites@',2,20), ('?kCallbackSites@',3,20), ('?kCleanupSites@',6,24)]:
        before, after = symbol(old_symbols, prefix), symbol(current_symbols, prefix)
        for i in range(count):
            a, b = old.read(before+i*stride, stride), pe.read(after+i*stride, stride)
            assert a[:-4] == b[:-4], (prefix, i)
            at, length = struct.unpack_from('<2I', b)
            assert b[8:8+length] == native.read(at, length), (prefix, hex(at))
            same_target(struct.unpack_from('<I', a, stride-4)[0], struct.unpack_from('<I', b, stride-4)[0])
        table_counts[prefix] = count
    # Pointer-signature hook tables retain their full native dependency bodies.
    for prefix, count, stride in [('?kRFPatches@',8,20), ('?kSCPatches@',3,20),
            ('?kReplayPatches@',7,24), ('?g_regMapPatches@',2,32)]:
        before, after = symbol(old_symbols, prefix), symbol(current_symbols, prefix)
        for i in range(count):
            a, b = old.read(before+i*stride, stride), pe.read(after+i*stride, stride)
            assert a[:12] == b[:12], (prefix, i)
            at, length, total, stock, target = struct.unpack_from('<5I', b)
            old_stock, old_target = struct.unpack_from('<2I', a, 12)
            assert pe.read(stock, total) == old.read(old_stock, total) == native.read(at, total)
            same_target(old_target, target)
            if prefix == '?kReplayPatches@':
                assert a[20:] == b[20:]
        table_counts[prefix] = count
    main = (ROOT / 'src/DriverSF_V29b_NativeFastPath.cpp').read_text()
    entries = main[main.index('static PatchSite g_sites[]'):].split('\n};', 1)[0]
    count = len(re.findall(r'^\s*\{0x', entries, re.M))
    assert count == 31
    before, after = symbol(old_symbols, '?g_sites@'), symbol(current_symbols, '?g_sites@')
    for i in range(count):
        a, b = old.read(before+i*40, 40), pe.read(after+i*40, 40)
        at, stock, length, target, resume, trampoline = struct.unpack_from('<6I', b)
        old_at, old_stock, old_length, old_target, old_resume, old_trampoline = struct.unpack_from('<6I', a)
        assert (at, length) == (old_at, old_length) and a[24:] == b[24:]
        assert pe.read(stock, length) == old.read(old_stock, length) == native.read(at, length)
        for previous, current in ((old_target,target), (old_resume,resume), (old_trampoline,trampoline)):
            if previous or current:
                same_target(previous,current)
    table_counts['?g_sites@'] = count
    # Retained naked bridge ABI: GP/flags/FP state, saved stack and continuation.
    for prefix in ('_RFBridgePoll', '_RFBridgeCommit', '_RFBridgeWrap', '_RFBridgeSpace', '_RFBridgeIssue', '_RFBridgeEnd'):
        _, _, code = function_code(prefix)
        for pattern in ('9c60','8bdc','83e4f0','0fae0424','fc','0fae0c24','8be3619d'):
            assert bytes.fromhex(pattern) in code, (prefix, pattern)
    for prefix in ('_SCBridgeSamples', '_SCBridgeVehicle', '_SCBridgePlayer'):
        _, _, code = function_code(prefix)
        for pattern in ('9c50','60','8bec','83e4f0','0fae0424','fc','0fae0c24','8be561','9d8d642404c3'):
            assert bytes.fromhex(pattern) in code, (prefix, pattern)
    assert not any(k.startswith('_RHBridge') or 'RHScope' in k for k in current_symbols)
    for marker in (b'DSFV29s_TRAFFIC_POLICY_START revision=1 mode=', b'DSFV29s_traffic_policy_',
            b'DriverSF_Traffic.ini', b'nativeRoster', b'Preset150', b'capacityChanges=0 liveReload=0',
            b'testCapacity=192', b'fixedOwnerLimit=0 inlineNodes=45 externalNodes=512'):
        assert marker in pe.data, marker
    result = dict(result='passed', name=NAME, bytes=len(pe.data), sha256=hashlib.sha256(pe.data).hexdigest(),
        configuration='Release', platform='Win32', imports=sorted(dlls), policy_default='Native',
        request_abi='fastcall/RET8 retained', requests=150, graphics=160,
        geometry_tables=len(geometry), retained_hook_tables=table_counts,
        source_sha256=checked['source_sha256'], fixture=tested, parser=parser_checks(), installed=False, game_run=False,
        limitations=['Compiled table/signature/target and bridge checks plus unchanged core hashes; not a rerun of every historical verifier.',
            'Fixture native services and return-address identity are stubbed; user gameplay/teardown testing remains required.'])
    (OUT / 'verification.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    return result


def package():
    checked = json.loads((OUT / 'verification.json').read_text())
    assert checked['result'] == 'passed'
    for name, digest in checked['source_sha256'].items():
        assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest, ('verification stale', name)
    data = ASI.read_bytes()
    assert hashlib.sha256(data).hexdigest() == checked['sha256']
    dest = ROOT / 'dist/MAIN_150' / ASI.name
    assert not dest.exists() or dest.read_bytes() == data, 'refuse overwrite'
    dest.write_bytes(data)
    assert dest.read_bytes() == data
    result = dict(result='passed', path=str(dest.relative_to(ROOT)), bytes=len(data),
        sha256=checked['sha256'], installed=False, game_run=False)
    (OUT / 'package.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--static-only', action='store_true')
    group.add_argument('--fixture-only', action='store_true')
    group.add_argument('--verify-build', action='store_true')
    group.add_argument('--package', action='store_true')
    args = parser.parse_args()
    if args.package:
        result = package()
    elif args.verify_build:
        result = verify_build()
    elif args.fixture_only:
        static()
        result = fixture()
    else:
        result = static()
    print(json.dumps(result, indent=2))

"""Read only the supplied model192 log and Windows minidump; no debugger/game execution."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

from analyze_audio_probe_log import analyze
from compare_audio_slots_logs import rows, slot_summary


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def analyze_log(path):
    records = rows(path)
    def of(kind):
        return [r for r in records if r['type'] == 'DSFV29s_AUDIO_MODEL_' + kind]
    starts, status, registry, keys = map(of, ('START', 'STATUS', 'REGISTRY', 'KEY'))
    latest_registry = {r['owner']: r for r in registry}
    latest_keys = {r['model']: r for r in keys}
    checks = {
        'startup': len(starts) == 1 and all(starts[0].get(k) == v for k, v in {
            'status': '1', 'hooks': '8', 'expectedHooks': '8', 'stockCapacity': '45',
            'testCapacity': '192', 'nativeLayoutPreserved': '1', 'emptyOnly': '1', 'activeMigration': '0'}.items()),
        'no_invalid_or_overflow': bool(status) and all(r.get(k) == '0' for r in status for k in
            ('attachRefused', 'invalid', 'instanceOverflow', 'keyOverflow')),
        'attempted_registries_expanded': bool(registry) and all(
            r['state'] == '1' and r['capacity'] == '192' for r in registry if int(r['attempts'])),
        'guards_healthy': bool(registry) and all(r['guardBad'] == '0' for r in registry),
        'no_full_refusals': bool(registry) and all(r['full'] == '0' for r in registry),
    }
    return dict(log=str(path), sha256=digest(path), checks=checks, valid=all(checks.values()),
        duration_seconds=int(status[-1]['tick']) / 1000, first_start=starts,
        final_status=status[-1], registries=list(latest_registry.values()),
        distinct_model_keys=len(latest_keys),
        models_stored=sorted(int(k) for k, r in latest_keys.items() if int(r['stores'])),
        models_beyond_stock_gate=sorted(int(k) for k, r in latest_keys.items() if int(r['wouldStockFull'])),
        models_with_setup=sorted(int(k) for k, r in latest_keys.items() if int(r['setup'])),
        model_failures=[r for r in latest_keys.values() if any(int(r[k]) for k in
            ('full', 'misses', 'fallbackMiss', 'bankNotReady', 'setupFail'))],
        model_totals={k: sum(int(r[k]) for r in latest_keys.values()) for k in
            ('attempts', 'stores', 'full', 'wouldStockFull', 'removes', 'hits', 'misses',
             'fallbackHit', 'fallbackMiss', 'setup', 'disabled', 'bankReady', 'bankNotReady', 'setupOk', 'setupFail')},
        baseline=analyze(path), slots=slot_summary(records),
        reason_last=next((r for r in reversed(records) if r['type'] == 'DSFV29s_AUDIO_REASON_STATUS'), None),
        registry_transitions=[r for i, r in enumerate(registry) if i == 0 or
            any(r[k] != registry[i-1][k] for k in ('current', 'removes', 'state', 'guardBad'))][-20:])


def analyze_dump(path):
    data = path.read_bytes()
    def unpack(fmt, off):
        return struct.unpack_from('<' + fmt, data, off)
    signature, version, count, directory, checksum, timestamp, flags = unpack('6IQ', 0)
    assert signature == 0x504D444D, 'Not a Windows minidump'
    assert directory + count * 12 <= len(data)
    streams = {kind: (size, rva) for kind, size, rva in
               (unpack('3I', directory + i * 12) for i in range(count))}
    def string(rva):
        size, = unpack('I', rva)
        return data[rva+4:rva+4+size].decode('utf-16le', errors='replace')
    modules = []
    if 4 in streams:
        _, off = streams[4]
        n, = unpack('I', off)
        for i in range(n):
            entry = off + 4 + i * 108
            base, size, check, stamp, name = unpack('Q4I', entry)
            modules.append(dict(base=base, end=base+size, size=size, timestamp=stamp,
                                name=string(name), checksum=check))
    def module(address):
        m = next((m for m in modules if m['base'] <= address < m['end']), None)
        return None if m is None else dict(name=m['name'], base=hex(m['base']), rva=hex(address-m['base']))
    memory = []
    if 5 in streams:
        _, off = streams[5]
        n, = unpack('I', off)
        for i in range(n):
            start, size, rva = unpack('Q2I', off+4+i*16)
            memory.append((start, size, rva))
    if 9 in streams:
        _, off = streams[9]
        n, rva = unpack('2Q', off)
        for i in range(n):
            start, size = unpack('2Q', off+16+i*16)
            memory.append((start, size, rva))
            rva += size
    threads = []
    if 3 in streams:
        _, off = streams[3]
        n, = unpack('I', off)
        for i in range(n):
            entry = off + 4 + i * 48
            tid, suspend, priority_class, priority, teb, stack, size, rva, ctxsize, ctxrva = unpack('4I2Q4I', entry)
            threads.append(dict(id=tid, teb=hex(teb), stack=hex(stack), stack_size=size,
                                context_rva=ctxrva, context_size=ctxsize))
            memory.append((stack, size, rva))
    def read(address, size):
        for start, length, rva in memory:
            if start <= address and address + size <= start + length:
                return data[rva+address-start:rva+address-start+size]
        return None
    def word(address):
        b = read(address, 4)
        return None if b is None or len(b) != 4 else struct.unpack('<I', b)[0]
    def context(off, size):
        if size < 204:
            return None
        names = ['edi', 'esi', 'ebx', 'edx', 'ecx', 'eax', 'ebp', 'eip', 'cs', 'eflags', 'esp', 'ss']
        return dict(zip(names, unpack('12I', off+156)))
    exception = None
    if 6 in streams:
        _, off = streams[6]
        tid, = unpack('I', off)
        code, eflags, record, address, nparams, unused = unpack('2I2Q2I', off+8)
        params = list(unpack('15Q', off+40))[:nparams]
        size, rva = unpack('2I', off+160)
        regs = context(rva, size)
        stack_words = []
        if regs:
            for delta in range(0, 512, 4):
                a = regs['esp'] + delta
                value = word(a)
                if value is None:
                    break
                stack_words.append(dict(address=hex(a), value=hex(value), module=module(value)))
        frames = []
        fp = regs['ebp'] if regs else 0
        for _ in range(64):
            nxt, ret = word(fp), word(fp+4)
            if nxt is None or ret is None:
                break
            frames.append(dict(ebp=hex(fp), next=hex(nxt), return_address=hex(ret), module=module(ret)))
            if nxt <= fp or nxt-fp > 0x100000:
                break
            fp = nxt
        exception = dict(thread_id=tid, code=hex(code), flags=eflags, address=hex(address),
            module=module(address), parameters=[hex(p) for p in params],
            registers={k: hex(v) for k, v in (regs or {}).items()},
            code_bytes=(read(address-16, 64) or b'').hex(' '),
            ebp_chain=frames, stack_words=stack_words)
    pid = None
    if 15 in streams:
        _, off = streams[15]
        size, infoflags = unpack('2I', off)
        if infoflags & 1:
            pid, = unpack('I', off+8)
    system = None
    if 7 in streams:
        _, off = streams[7]
        arch, level, revision, processors, product, major, minor, build, platform, csdrva = unpack('3H2B5I', off)
        system = dict(architecture=arch, processors=processors, major=major, minor=minor, build=build)
    return dict(dump=str(path), bytes=len(data), sha256=digest(path), process_id=pid,
        timestamp=timestamp, flags=hex(flags), system=system,
        streams={str(k): dict(size=s, rva=r) for k, (s, r) in streams.items()},
        exception=exception, threads=threads,
        memory_ranges=[dict(start=hex(a), size=s, rva=r) for a, s, r in memory],
        modules=[dict(m, base=hex(m['base']), end=hex(m['end'])) for m in modules],
        limitations=['EBP chain is a frame-pointer heuristic, not a symbol-based unwind.',
                    'Stack words in module address ranges are candidates, not proven caller frames.',
                    'Only memory included in the supplied minidump is available.'])


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--log', type=Path, required=True)
    p.add_argument('--dump', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    log, dump = analyze_log(a.log), analyze_dump(a.dump)
    for name, result in [('log_summary', log), ('dump_summary', dump)]:
        (a.output / (name+'.json')).write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(dict(audio_checks=log['checks'], duration=log['duration_seconds'],
        final=log['final_status'], registry=log['registries'], model_totals=log['model_totals'],
        baseline_valid=log['baseline']['valid_probe'], slots_valid=log['slots']['valid'],
        dump_pid=dump['process_id'], dump_exception={k: dump['exception'][k] for k in
            ('thread_id', 'code', 'address', 'module', 'parameters', 'registers', 'ebp_chain')},
        loaded_asi=[m for m in dump['modules'] if m['name'].lower().endswith('.asi')]), indent=2))


if __name__ == '__main__':
    main()

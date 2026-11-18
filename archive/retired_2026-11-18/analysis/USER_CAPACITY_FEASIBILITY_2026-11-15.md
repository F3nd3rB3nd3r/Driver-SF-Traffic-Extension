# Feasibility of user-controlled capacities from available material

User asks whether the four proposed steps (tested150 default, coordinated
runtime storage, removal of testing-only ceilings, structural/allocation checks)
can be done using available information and static audit. This turn performs
a feasibility review, not the wide-capacity implementation or a new ASI build.

## Answer

The source, supported Driver.exe and existing evidence are sufficient to
continue the static migration work without requiring a new live probe first.
The complete wider-capacity path is **not yet established**: native indirect
provider/index consumers remain untraced, and fixed coupled storage remains
in the active code. Existing information supports an implementation approach;
it is not proof that all hidden/native limits have already been found.

Static tracing can establish formats, arithmetic, allocation geometry and
coherent bound changes. Building and offline fixtures are still needed to
verify new executable code. User live regression validates actual game behavior
afterward; a500/1000 configuration must remain explicitly untested until such
a run exists. No existing150 pass transfers automatically to changed native
storage code.

## Concrete findings in this review

- Active project is GENERIC_R1 with requests150 and graphics/admission160.
  Current150 ASI and its default INI remain the verified fallback/default.
- V29sArenaReuseCore.inc still declares256-node persistent spool arenas,
  a13-DWORD seen-node mask,150 active/recycled/request bounds, and150-node
  scratch validation with fixed ESP offsets. Removing GenericValidate's150
  ceiling would leave these real bounds and fatal paths unchanged.
- V29sBaseline150.inc still embeds150-ID/model arrays and stack scratch arrays.
  Diagnostics must either grow coherently or explicitly bound their own
  observation without reading/writing beyond storage. A dynamic request list
  alone is not sufficient.
- Graphics counts/budgets and model descriptor source values remain fixed;
  handler/global/per-kind/selector/map/ED/EF arrays and their relocation sites
  must consume the chosen model capacity while preserving current lifetime
  fixes and unrelated native object layouts.
- Callback512, pending512, registry512, model-key diagnostics512 and other
  fixed queues/maps remain distinct capacities. Some can stay documented
  structural bounds until migrated, but a build cannot claim that every
  user-adjustable capacity has no testing-only ceiling while silently keeping
  incompatible coupled storage behind it.
- Request scratch is currently a patched1824-byte native frame; formula
  12*N+24 yields6024 at500. The native prologue performs SUB ESP without a
  probing helper. Wider scratch requires stack probing or a reviewed external
  scratch design, with every displaced offset and ABI preserved.
- Notification copying is already runtime-sized but its native prologue is
  still a stack allocation; sufficiently large configured frames also require
  a page-probing audit. Runtime arithmetic alone does not make it unbounded.

## Native check repeated against the exact available executable

Supported Driver.exe SHA256:
`c79987b0548f7130337c3f295c8fe269e120ceddad55c1159e6f5cbc6aa26616`.
Read-only Microsoft disassembly retained in
`analysis/user_capacity_feasibility_2026-11-15/`.

Initializer005A1420 uses byte metadata for the per-kind zero gate005A1469
and loop bound005A14EA, with DWORD counters and ordinals. This supports a
package0/kind0-specific wide-bound implementation, rather than attempting to
write500 into an8-bit asset field. Other kinds/metadata consumers must retain
their original meanings.

Factory0059F9C0 forwards ordinal arguments through indirect provider calls
at0059FA85 and0059FAA0. The forwarding sites use full DWORD operands, but
that does **not** prove the target methods and downstream consumers preserve
all ordinal bits. Resolving and tracing those methods remains the next native
dependency. Do not claim the byte count is the only native obstacle on this
evidence.

## Four-step implementation status

| Step | Present status | Required work |
|---|---|---|
|150-car default|Already staged and live tested|Preserve exact fallback bytes/configuration; regression-test the migration at the same settings|
|Coherent runtime storage|Partially implemented in GENERIC_R1|Finish model/graphics/provider bounds, spool/native scratch, remaining coupled sidecars and diagnostics|
|Remove testing-only ceilings|Not performed in this review|Remove each only after its complete consumer/allocation chain is migrated; label larger configurations untested|
|Structural/allocation checks|Existing checks cover the current milestone|Extend checked arithmetic, ID/count/index constraints, coherent dependencies and allocation failure handling for the new path|

Offline acceptance should cover150 unchanged, counts255/256,500/1000,
page-crossing scratch sizes, arithmetic overflow, allocation failure and
constructor/recycle/teardown behavior. These are test inputs, not proposed
new policy ceilings. No new dependencies or live game are needed to begin
those static/fixture preparations.

Literal absence of all upper bounds is not a valid claim for a32-bit binary
with fixed-width identifiers and file formats. The feasible contract is
user-controlled capacities limited by actual representation/arithmetic/
allocation constraints, with a tested150 default and larger values untested.
The current review does not widen capacities, change live request counts,
compile/deploy an ASI, launch a game/debugger or modify assets/saves.

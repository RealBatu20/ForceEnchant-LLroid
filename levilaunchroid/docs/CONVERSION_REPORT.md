# ForceEnchant — Java/Windows → LeviLaunchroid conversion report

## 1. Project classification

- **Input project type:** native **C++ Windows mod / DLL** for Minecraft Bedrock
  (Windows 10/11 edition, `Minecraft.Windows.exe`). It is *not* a Java mod,
  plugin, or datapack — there is no `fabric.mod.json`, `mods.toml`, `plugin.yml`,
  bytecode, or Gradle/Maven build. Evidence: `ForceEnchant.sln` (Visual Studio),
  `ForceEnchant.vcxproj`, `dllmain.cpp` with `DllMain`, MinHook + libhat.
- **Loaded via:** a debug injector (README references `DebugInjector`).
- **Target platform of original:** x86-64 Windows.

Because the source is already native C++, this is a **native-to-native platform
port** (Windows x86-64 → Android arm64-v8a), not a Java decompilation task. No
CFR / bytecode analysis was required.

## 2. Target platform (port)

- Minecraft Bedrock on **Android**, ABI **arm64-v8a**
- Loader: **LeviLaunchroid** (preloader), output `libforceenchant.so`
- Hook target: `libminecraftpe.so`

## 3. Recommended conversion path

Map each Windows building block to its LeviLaunchroid equivalent:

| Windows (original) | LeviLaunchroid (port) |
|---|---|
| `DllMain` + injected thread (`dllmain.cpp`) | `PL_REGISTER_MOD` + `load()/enable()/disable()/unload()` lifecycle |
| MinHook (`MH_CreateHook`/`MH_EnableHook`) | `pl::hook::hook` / `pl::hook::unhook` (Gloss-backed) |
| libhat pattern scan vs `Minecraft.Windows.exe` | `pl::signature::resolveSignature(sig, "libminecraftpe.so")` |
| Hardcoded x86-64 patterns in headers | External, version-agnostic `config/config.json` (symbol **or** arm64 pattern) |
| `__declspec(property)` field accessors | Portable `fieldAt<T>()` template (documentation only; unused at runtime) |

## 4. Compliance status

**Compliant** as a creative/sandbox quality-of-life tool. The two behaviours
relax local enchanting validation (max level + non-vanilla combinations). No
X-ray/ESP, no combat automation, no packet abuse, no anti-cheat evasion, no
remote access, no memory manipulation beyond two reversible function hooks.
Intended for single-player / owned worlds; misuse on servers you don't control
may breach the EULA/server rules (stated in README).

## 5. Estimated conversion feasibility

**High** for the code/architecture (done — fully implemented, builds against the
NDK + LeviLaunchroid SDK). The **only** remaining work is data, not code:
generating two arm64 signatures for the user's Minecraft version (see
`SIGNATURES.md`), which inherently requires the target binary + IDA/Ghidra.

---

## 6. Behaviour reconstruction (FACT / INFERENCE / HYPOTHESIS)

### Feature A — unlimited `/enchant` level
- **FACT:** `CommandHook::ValidRange::handle` (`CommandHook.cpp`) returns `true`
  unconditionally and never calls the original. It hooks `Command::validRange`.
- **FACT:** the author's note ties it to the `commands.enchant.invalidLevel`
  string → `EnchantCommand::execute` → first range guard.
- **INFERENCE:** `validRange` is a `bool(int,int,int[,…])` comparator; forcing
  `true` makes the command skip the "level out of range" rejection.
- **Port:** `ValidRangeHook.cpp` — detour returns `true`. No original call,
  so no ABI risk on the return path.

### Feature B — any enchant on any item
- **FACT:** `ItemEnchantsHook::canEnchant::handle` (`ItemEnchants.cpp`) re-calls
  the original with the final boolean argument forced to `true`
  (`oFunc(_this, result, en, true)`), where the argument is `allowNonVanilla`.
- **INFERENCE:** that boolean lifts the vanilla item/enchant compatibility
  restriction.
- **HYPOTHESIS (needs IDA on Android):** the exact owning class
  (`ItemEnchants::canEnchant` per Windows note vs `EnchantUtils::canEnchant` per
  LeviLamina), the argument count, and whether the return is by-value (arm64 x8
  `sret`) or by-pointer (x0). The port assumes opaque pointer args + pointer
  return and isolates this assumption to one file for easy correction.

---

## 7. Hook analysis

| Hook | File | Strategy | Calls original? | Notes |
|---|---|---|---|---|
| `Command::validRange` | `ValidRangeHook.cpp` | replace return with `true` | No | Lowest risk; return-only override |
| `EnchantUtils::canEnchant` | `CanEnchantHook.cpp` | forward args, force `allowNonVanilla=true` | Yes | ABI/prototype is HYPOTHESIS — verify in IDA |

All hooks install via `pl::hook::hook(target, detour, &original, PriorityNormal)`
and are removed on `disable()`. Resolution failure is non-fatal and logged.

## 8. Signature analysis

No verified arm64 signatures are shipped — they cannot be produced without the
target `libminecraftpe.so` and IDA/Ghidra, and fabricating them would violate
the project's accuracy rules. Instead:

- Signatures live in `config/config.json` (symbol name **or** arm64 byte pattern).
- Empty by default ⇒ hook is skipped with a clear log line; the mod still loads.
- `SIGNATURES.md` documents the exact IDA Pro MCP / Ghidra workflow to derive
  each one, plus validation criteria.
- The original **x86-64** patterns are preserved as reverse-engineering *leads*
  only, explicitly marked invalid for arm64.

## 9. VTable / Rendering / NBT analysis

**Not applicable.** ForceEnchant installs two function hooks and touches no
vtables, no rendering (bgfx/RenderDragon/ImGui), and no NBT/world-data
serialization. No UI is added. These sections are intentionally empty rather
than padded.

## 10. Feature parity

| Feature | Parity | Notes |
|---|---|---|
| Unlimited `/enchant` level | **Full** (pending signature) | Direct behavioural equivalent |
| Any enchant on any item | **Full** (pending signature + ABI confirm) | Confirm `canEnchant` prototype in IDA |

Net: 100% feature parity is achievable; activation depends on user-supplied
signatures for the running Minecraft version.

## 11. Final recommendation

1. Build `libforceenchant.so` with the NDK + LeviLaunchroid SDK (`./build.sh`).
2. Open your device's `libminecraftpe.so` in IDA Pro (MCP) / Ghidra and generate
   the two signatures per `SIGNATURES.md`; confirm the `canEnchant` prototype.
3. Put them in `config/config.json`, import via LeviLaunchroid, and verify the
   logged hook results plus in-game behaviour on a world you own.

> Honesty note: this environment had **no Android NDK and no `libminecraftpe.so`**,
> so the `.so` was **not** compiled here and no runtime/device validation was
> performed. The project is complete and build-ready; the signature + on-device
> verification steps remain for the user, as they require the target binary.

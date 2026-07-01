# ForceEnchant — Levi Launchroid (Android) port

Native Android port of the Windows **ForceEnchant** mod for Minecraft Bedrock.
It removes the enchantment level limit so that `/enchant` accepts levels above
the vanilla cap, exactly like the original Windows DLL — but compiled as
`libforceenchant.so` for the [Levi Launchroid](https://levilaunchroid.levimc.org)
preloader on `arm64-v8a`.

> The original Windows project lives in [`../ForceEnchant`](../ForceEnchant).
> This directory is the standalone Android port; nothing in the Windows project
> was modified.

---

## 1. Project classification

| Field | Value |
|---|---|
| Source project type | Windows DLL (C++) injected into `Minecraft.Windows.exe` |
| Source tooling | MinHook (function hooks) + libhat (byte-pattern signature scan) |
| Target platform | Minecraft Bedrock Edition, **Android**, `arm64-v8a` |
| Target loader | Levi Launchroid (`preload-native` mod) |
| Target binary | `libminecraftpe.so` |
| Output | `libforceenchant.so` (+ `ForceEnchant.levipack`) |
| Compliance | Single-client gameplay/QoL change (enchant cap removal); reversible function hooks only; no packet abuse, no anti-cheat evasion, no remote access |
| Feasibility | High — two small, well-defined function hooks |

## 2. Recommended conversion path

The Windows build hooks two engine functions. The same two functions exist in
the Android engine; only the hooking mechanism and signature format differ.

| Windows | Android (this port) |
|---|---|
| `MinHook` (`MH_CreateHook`) | `pl::hook::hook()` (Gloss-backed) |
| `libhat` byte patterns (x86-64) | `pl::signature::resolveSignature()` by mangled symbol name (or AArch64 pattern) |
| `__fastcall` + explicit `__int64 _this` | AAPCS64 standard call; `this` is the first argument |
| `DllMain` + worker thread | `enable()` / `disable()` lifecycle methods |

## 3. Behaviour mapping (FACT, from upstream source)

1. **`ItemEnchants::canEnchant(...)`** — upstream calls the original with
   `allowNonVanilla = true`. This port does the same (`hookCanEnchant`).
2. **`Command…::validRange(int, int, int, …)`** — upstream returns `true`
   unconditionally to bypass the `/enchant` level range check
   (`commands.enchant.invalidLevel`). This port does the same
   (`hookValidRange`).

See `src/hooks/EnchantHooks.cpp`.

## 4. Signature analysis — IMPORTANT (verification required)

The Windows byte patterns in the original source are **x86-64** and cannot work
on ARM64. This port resolves the two targets **by mangled symbol name** in
[`src/Signatures.h`](src/Signatures.h). Those names are an **INFERENCE /
HYPOTHESIS** derived from class/method names — they are **not** verified against
a specific `libminecraftpe.so` here (no device binary / IDA on the build host).

**Before relying on it in-game you must verify the two symbols for your exact
Minecraft Bedrock version** (instructions are inline in `src/Signatures.h`):

- Open the target `libminecraftpe.so` (arm64-v8a) in IDA Pro / Ghidra.
- Confirm `ItemEnchants::canEnchant` and the enchant-command `validRange`
  helper, then paste the exact mangled name **or** an AArch64 byte pattern
  (with `?` wildcards) into `src/Signatures.h`.

The mod fails **safe**: if a signature does not resolve, that hook is skipped
with a warning in the Levi Launchroid log instead of crashing the game.

## 5. Hook analysis

| Hook | Target | Detour effect | Safety |
|---|---|---|---|
| canEnchant | `ItemEnchants::canEnchant` | forces `allowNonVanilla = true`, then calls original | original is always called; no state corruption |
| validRange | enchant-command range check | returns `true` | scoped to the `/enchant` level argument path; see caveat below |

**Caveat (INFERENCE):** if the resolved `validRange` is a shared command helper
rather than enchant-specific, forcing `true` could relax range checks for other
commands too. This mirrors the upstream Windows behaviour. If you want it
strictly enchant-only, hook `EnchantCommand::execute` instead and clamp there.

## 6. Build

### Locally
```bash
export ANDROID_NDK_HOME=/path/to/android-ndk   # r25c+
cd levilaunchroid
./build.sh                 # -> build/libforceenchant.so + dist/ForceEnchant.levipack
```
`build.sh` fetches the Levi Launchroid SDK headers
(`pl/cpp/*.hpp` from `LiteLDev/preloader-android`) into
`third_party/levilaunchroid-sdk/` automatically. The preloader provides the
`pl::*` symbols at load time, so the `.so` is linked with undefined symbols
allowed and needs no import library.

### CI / Release
GitHub Actions (`.github/workflows/build-llroid.yml`) builds on every push and
uploads `libforceenchant.so` + `.levipack` as artifacts. Tagging publishes a
release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

## 7. Install

1. Build (or download the release artifact) to get `ForceEnchant.levipack`.
2. Import it via the Levi Launchroid app, or drop `libforceenchant.so` +
   `manifest.json` into the loader's mods directory.
3. Launch Minecraft. Check the Levi Launchroid log for
   `[ForceEnchant] Enabled (enchant limit removed)`.

## 8. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `dlopen failed: cannot locate symbol _ZN2pl9signature16resolveSignature...` | Mod not linked against `libpreloader.so` (no `DT_NEEDED`) | Fixed by the build here — CMake links the SDK's `preloader` target so the pl:: symbols resolve at load |
| Log: `Could not resolve … signature` | Symbol names in `Signatures.h` don't match your MC version | Re-verify in IDA, update `Signatures.h` (Section 4) |
| `Enabled but NO hooks installed` | Both signatures failed | Same as above |
| `/enchant` still rejects high levels | `validRange` hook not installed, or version mismatch | Verify the `validRange` signature |
| Build error: SDK headers not found | SDK fetch failed / offline | Clone `LiteLDev/preloader-android` and pass `-DLL_SDK_DIR=<dir with pl/cpp>` |

## 9. Final recommendation

The code is complete and CI-buildable. The **only** runtime-blocking item is
signature verification against your specific `libminecraftpe.so` (Section 4) —
this requires the target binary and was not performed on the build host. After
confirming the two symbols in `src/Signatures.h`, the produced
`libforceenchant.so` reproduces the upstream ForceEnchant behaviour on Android.

# Generating signatures for ForceEnchant (Android / arm64-v8a)

ForceEnchant needs to locate two functions inside `libminecraftpe.so`. Because
their addresses change with every Minecraft version, you generate the locating
signatures yourself, in IDA Pro (with the IDA Pro MCP) or Ghidra, against the
exact `libminecraftpe.so` your device runs, then paste them into
`config/config.json`.

A signature can be **either** form accepted by
`pl::signature::resolveSignature`:

- a **symbol name** — preferred when the binary still exports/retains the symbol
  (Bedrock Android builds historically keep many mangled C++ symbols). Most
  robust across minor updates.
- an **arm64-v8a byte pattern** with wildcards (`?`/`??` = whole byte,
  `A?` = low nibble, `?F` = high nibble) — use when the symbol is stripped.

> The original Windows mod stored **x86-64** patterns (see
> `../ForceEnchant/.../*.h`). Those are for `Minecraft.Windows.exe` and **do not
> work on arm64** — they are only reverse-engineering *leads*, listed below.

---

## 1. `Command::validRange`  → config key `hooks.validRange.signature`

**Goal:** the bounds check `/enchant` uses to reject an out-of-range level.

**How to find it**

1. In IDA/Ghidra, search the string list for `commands.enchant.invalidLevel`
   (Windows lead string; on Bedrock the key is often
   `commands.enchant.invalidLevel` / `…invalidLevel`).
2. Follow its cross-reference (xref) into `EnchantCommand::execute`.
3. The first range-guard call in that function — comparing the parsed level
   against a min/max and producing that error — is `Command::validRange`
   (a small templated/`int` comparator returning `bool`).
4. Either:
   - copy its **mangled symbol** (look for `Command` + `validRange` in the
     Functions/Names window), **or**
   - select the first ~16–32 stable bytes of the function prologue and export a
     masked pattern (wildcard any relative offsets / address-bearing operands).

**Validate**

- Only one match in `libminecraftpe.so`.
- It is a leaf-ish `bool(int, int, int [,…])` comparator, not the whole command.
- After installing, an enchant level beyond the vanilla cap is accepted in game.

---

## 2. `EnchantUtils::canEnchant`  → config key `hooks.canEnchant.signature`

**Goal:** the eligibility function with an `allowNonVanilla` boolean argument
that gates whether a given enchantment may be applied to a given item. The
Windows author named it `ItemEnchants::canEnchant`; LeviLamina references call
the equivalent `EnchantUtils::canEnchant`. Confirm the real owning class in your
binary — do not assume.

**How to find it**

1. Start from enchant-application code: xref `EnchantCommand::execute` (from
   step 1) and the anvil/enchanting-table apply paths, or search the Names
   window for `canEnchant`.
2. Identify the function that takes an enchantment-instance argument **and** a
   trailing `bool` (the `allowNonVanilla` / "ignore compatibility" flag).
3. Confirm in the decompiler that the `bool` argument, when true, skips the
   vanilla item/enchant compatibility checks.
4. Copy its **mangled symbol** or export a masked prologue pattern as above.

**Confirm the ABI before trusting the hook** (`src/hook/CanEnchantHook.cpp`):

- Check the decompiled prototype: how many pointer arguments precede the
  `bool`, and whether the function returns a value **by pointer** (x0) or
  **by value** (arm64 indirect-result register **x8**). The detour assumes
  opaque pointer args (x0..x3) and a pointer return (x0). If IDA shows an x8
  `sret` return or a different argument count, update the `Fn` typedef and
  `detour` in that one file accordingly.

**Validate**

- Only one match in `libminecraftpe.so`.
- After installing, an enchantment can be applied to an item that vanilla would
  normally reject (e.g. via `/enchant`), in a world you own.

---

## Putting it in config

```jsonc
{
  "hooks": {
    "validRange": { "enabled": true, "signature": "<symbol or arm64 pattern>" },
    "canEnchant": { "enabled": true, "signature": "<symbol or arm64 pattern>" }
  }
}
```

Edit on the device next to `libforceenchant.so` (`./config/config.json`), then
relaunch. The mod logs each hook result (`installed` / `skipped` / `FAILED to
resolve` / `installation FAILED`) — check the LeviLaunchroid log to confirm.

## Original Windows leads (x86-64 — NOT valid on Android)

| Function | Windows x86-64 pattern (lead only) |
|---|---|
| `Command::validRange` | `55 56 57 48 81 EC C0 00 00 00 48 8D AC 24 80 00 00 00 48 C7 45 38 FE FF FF FF 4C 89 CE 89` |
| `ItemEnchants::canEnchant` | `41 57 41 56 41 55 41 54 56 57 53 48 83 EC 20 4C 89 C7 48 89 D6 40` |

Use these only to understand the original targets; regenerate fresh arm64
signatures as described above.

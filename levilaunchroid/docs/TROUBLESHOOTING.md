# Troubleshooting

Check the LeviLaunchroid log first — ForceEnchant logs one line per hook:

```
[hook] Command::validRange: installed
[hook] EnchantUtils::canEnchant: skipped (no signature configured — see docs/SIGNATURES.md)
```

| Log line | Meaning | Fix |
|---|---|---|
| `skipped (no signature configured …)` | The `signature` field is empty | Add a symbol/pattern in `config/config.json` (see `SIGNATURES.md`) |
| `disabled in config` | `"enabled": false` for that hook | Set it to `true` |
| `FAILED to resolve signature …` | Signature didn't match this `libminecraftpe.so` | Regenerate it for your exact game version; check for typos / extra spaces |
| `resolved but hook installation FAILED` | Address found but `pl::hook::hook` rejected it | Wrong address (matched mid-function), or already hooked by another mod |
| `installed` | Hook is active | — |

## Build issues

- **`Android NDK not found`** — set `ANDROID_NDK_HOME` (or `ANDROID_NDK_ROOT`)
  to an NDK r26+ install.
- **`pl/cpp/Hook.hpp: No such file`** — set `PL_SDK_DIR` to your LeviLaunchroid
  preloader SDK (the folder containing `include/pl/...`).
- **`libpreloader not found` warning** — expected if you don't ship the import
  lib; the build links with `--allow-shlib-undefined` and the preloader resolves
  the `pl::` symbols at runtime. Only a problem if the device log shows unresolved
  symbols on load.
- **nlohmann/json fetch fails** — CI/host needs network for `FetchContent`. For
  offline builds, vendor `json.hpp` and point the include at it.

## Runtime issues

- **Mod loads but nothing changes in game** — almost always a signature problem;
  re-read the hook log lines above.
- **Game crashes when applying an enchant** — most likely the `canEnchant`
  prototype/ABI is wrong for your build. Re-check the decompiled signature in
  IDA: argument count and by-value (x8 `sret`) vs by-pointer return, then adjust
  the `Fn` typedef + `detour` in `src/hook/CanEnchantHook.cpp`. As a stopgap, set
  `hooks.canEnchant.enabled = false` to keep only the level-limit feature.
- **Works in single-player, rejected/flagged on a server** — expected: servers
  validate enchants independently and may not honor or may penalize forced
  enchants. Use only where permitted.

## Updating for a new Minecraft version

Native addresses shift between versions. When you update Minecraft:
1. Regenerate both signatures against the new `libminecraftpe.so`.
2. Update `config/config.json` (no recompile needed).
3. Optionally bump `minecraft_versions` in `manifest.json`.

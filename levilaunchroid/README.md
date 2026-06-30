# ForceEnchant — LeviLaunchroid (Android) port

Bedrock-native reimplementation of the Windows **ForceEnchant** mod for
[LeviLaunchroid](https://levilaunchroid.levimc.org), built as
`libforceenchant.so` for `arm64-v8a`.

## What it does

Two behaviours, ported 1:1 from the original Windows mod
(`../ForceEnchant/`):

| Feature | Original Windows hook | Bedrock-native port |
|---|---|---|
| `/enchant` accepts any level (no bounds check) | `Command::validRange` forced to return `true` | `src/hook/ValidRangeHook.cpp` |
| Any enchantment can go on any item | `ItemEnchants::canEnchant` called with `allowNonVanilla = true` | `src/hook/CanEnchantHook.cpp` |

Both are quality-of-life / creative-sandbox features for **single-player and
worlds you own**. See *Compliance* below.

## Requirements

- Android device running Minecraft Bedrock under LeviLaunchroid
- To build: Android NDK r26+, CMake ≥ 3.22, Ninja, and the LeviLaunchroid
  preloader SDK (headers + `libpreloader.so`)

## ⚠️ You must supply signatures for your game version

This repository ships with **empty signatures**. Native functions move between
Minecraft versions, so the addresses must be resolved against *your* exact
`libminecraftpe.so`. Until you do this, the mod loads but stays inactive and
logs which hooks are unconfigured.

Fill in `config/config.json` (no recompile needed) following
[`docs/SIGNATURES.md`](docs/SIGNATURES.md). Each signature may be a **symbol
name** or an **arm64-v8a byte pattern** — both are accepted by
`pl::signature::resolveSignature`.

## Build

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk
export PL_SDK_DIR=/path/to/levilaunchroid-sdk   # headers + libpreloader.so
./build.sh
# -> build/libforceenchant.so
```

CI builds automatically (`.github/workflows/build.yml`) and publishes a
`.levipack` on tag push:

```bash
git tag v1.0.0
git push origin v1.0.0
```

## Install (LeviLaunchroid)

Package layout (also produced by CI):

```
forceenchant/
├── manifest.json
├── config/config.json   <-- put your signatures here
└── libforceenchant.so
```

Import the folder/`.levipack` through the LeviLaunchroid launcher, then edit
`config/config.json` for your Minecraft version.

## Project layout

```
levilaunchroid/
├── CMakeLists.txt            # arm64-v8a shared lib, C++20, fetches nlohmann/json
├── build.sh                  # NDK build wrapper
├── manifest.json             # preload-native mod manifest
├── config/config.json        # runtime signature config (empty by default)
├── src/
│   ├── mod/ForceEnchantMod.* # mod lifecycle + PL_REGISTER_MOD entry
│   ├── config/Config.*       # tolerant config.json loader (dladdr-relative)
│   ├── hook/                  # HookManager + ValidRange + CanEnchant hooks
│   └── sdk/EnchantTypes.hpp   # opaque enchant structs (documentation only)
└── docs/                      # conversion report, signature guide, troubleshooting
```

## Compliance

ForceEnchant is a **creative/sandbox quality-of-life** tool. Use it only in
single-player or on worlds/servers you own or where the owner permits it. Using
it to gain an unfair advantage on servers you do not control may violate the
Minecraft EULA and server rules. It performs **no** anti-cheat evasion, packet
abuse, or remote access — it only relaxes two local enchanting checks.

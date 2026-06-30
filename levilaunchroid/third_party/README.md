# third_party

## levilaunchroid-sdk/  (you provide)

`CMakeLists.txt` expects the LeviLaunchroid **preloader SDK** here by default
(override with `-DPL_SDK_DIR=...`). It must provide the public headers used by
this mod:

```
levilaunchroid-sdk/
├── include/
│   └── pl/cpp/
│       ├── Hook.hpp
│       ├── Signature.hpp
│       ├── Mod.hpp
│       ├── Config.hpp
│       └── mod/RegisterHelper.hpp
└── lib/arm64-v8a/libpreloader.so   (optional import lib)
```

Obtain it from the LeviLaunchroid / preloader-android distribution that matches
the launcher you target, or add it as a git submodule:

```bash
git submodule add <preloader-android-sdk-url> levilaunchroid-sdk
```

If `libpreloader.so` is absent, the build links with
`--allow-shlib-undefined` and the loader resolves the `pl::` symbols at runtime.

## nlohmann/json

Fetched automatically by CMake (`FetchContent`, tag `v3.11.3`) — nothing to do.
For offline builds, vendor `json.hpp` here and adjust the include path.

# =============================================================================
#  ForceEnchant - Levi Launchroid native mod (Android arm64-v8a)
#  Output: libforceenchant.so
# =============================================================================
cmake_minimum_required(VERSION 3.22)

project(forceenchant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
#  Levi Launchroid SDK (LiteLDev/preloader-android).
#
#  The pl:: API is split: pl::hook::hook / NativeMod / Logger are header-only,
#  but pl::signature::resolveSignature and the C entry points (pl_hook,
#  pl_resolve_signature) are exported from libpreloader.so. A mod MUST link
#  against that library, otherwise dlopen fails at load with
#  "cannot locate symbol _ZN2pl9signature16resolveSignature...".
#
#  We build the SDK's `preloader` target here so that:
#    * headers resolve via its PUBLIC include dir (its `src`),
#    * pl:: symbols resolve at link time, and
#    * libforceenchant.so records DT_NEEDED libpreloader.so, which the launcher
#      provides in the mod's linker namespace at runtime.
#
#  Override the source with -DPRELOADER_ANDROID_DIR=<path> for offline builds.
# -----------------------------------------------------------------------------
include(FetchContent)

if(DEFINED PRELOADER_ANDROID_DIR AND NOT PRELOADER_ANDROID_DIR STREQUAL "")
    FetchContent_Declare(preloader_android SOURCE_DIR "${PRELOADER_ANDROID_DIR}")
else()
    FetchContent_Declare(
        preloader_android
        GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
        GIT_TAG main
    )
endif()
FetchContent_MakeAvailable(preloader_android)

add_library(forceenchant SHARED
    src/Mod.cpp
    src/hooks/EnchantHooks.cpp
)

target_include_directories(forceenchant PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# `preloader` brings its PUBLIC include dir (pl/**) and the exported pl:: symbols
# (DT_NEEDED libpreloader.so). `log` is for our own __android_log_print calls.
target_link_libraries(forceenchant PRIVATE preloader log)

target_compile_options(forceenchant PRIVATE
    -O2
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
)

target_link_options(forceenchant PRIVATE
    -Wl,--gc-sections
)

# Strip release builds.
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
    target_link_options(forceenchant PRIVATE -Wl,--strip-all)
endif()

# Produce exactly libforceenchant.so.
set_target_properties(forceenchant PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "forceenchant"
)

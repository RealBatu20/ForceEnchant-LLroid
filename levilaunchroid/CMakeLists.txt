# =============================================================================
#  ForceEnchant - Levi Launchroid native mod (Android arm64-v8a)
#  Output: libforceenchant.so
# =============================================================================
cmake_minimum_required(VERSION 3.21)

project(forceenchant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
#  Levi Launchroid SDK headers (pl/cpp/*.hpp from LiteLDev/preloader-android).
#
#  Provide the include directory via -DLL_SDK_DIR=/path/to/sdk so that
#  ${LL_SDK_DIR}/pl/cpp/Mod.hpp resolves. The CI workflow and build.sh fetch
#  this automatically. The preloader supplies the pl::* symbols at load time,
#  so the mod links with undefined symbols allowed (no import library needed).
# -----------------------------------------------------------------------------
if(NOT DEFINED LL_SDK_DIR OR LL_SDK_DIR STREQUAL "")
    set(LL_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/levilaunchroid-sdk")
endif()

if(NOT EXISTS "${LL_SDK_DIR}/pl/cpp/Mod.hpp")
    message(FATAL_ERROR
        "Levi Launchroid SDK headers not found at '${LL_SDK_DIR}'.\n"
        "Pass -DLL_SDK_DIR=<dir containing pl/cpp/*.hpp> or run build.sh, "
        "which fetches the SDK automatically.")
endif()

add_library(forceenchant SHARED
    src/Mod.cpp
    src/hooks/EnchantHooks.cpp
)

target_include_directories(forceenchant PRIVATE
    "${LL_SDK_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Android logging facility (__android_log_print).
target_link_libraries(forceenchant PRIVATE log)

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
    # The preloader resolves pl::* symbols at load time.
    -Wl,--allow-shlib-undefined
    -Wl,-z,undefs
)

# Strip release builds.
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
    target_link_options(forceenchant PRIVATE -Wl,--strip-all)
endif()

# Produce exactly libforceenchant.so (no extra prefix/suffix juggling).
set_target_properties(forceenchant PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "forceenchant"
)

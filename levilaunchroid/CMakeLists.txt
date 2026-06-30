cmake_minimum_required(VERSION 3.22)

project(forceenchant LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Output: lib<name>.so  (CMake adds the "lib" prefix automatically for arm64-v8a)
# Target binary to hook at runtime: libminecraftpe.so (Minecraft Bedrock / Android)
# Toolchain: Android NDK, ABI arm64-v8a (configure via build.sh / -DCMAKE_TOOLCHAIN_FILE)
# -----------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MOD_NAME forceenchant)

# --- LeviLaunchroid preloader SDK -------------------------------------------
# The SDK provides the pl/cpp/*.hpp headers and the import library to link
# against (libpreloader.so). Point PL_SDK_DIR at your local SDK / submodule.
#   -DPL_SDK_DIR=/path/to/levilaunchroid-sdk
set(PL_SDK_DIR "${CMAKE_SOURCE_DIR}/third_party/levilaunchroid-sdk"
    CACHE PATH "Path to the LeviLaunchroid preloader SDK (headers + libpreloader.so)")

# --- nlohmann/json (config.json parsing) -------------------------------------
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

# --- Mod library -------------------------------------------------------------
add_library(${MOD_NAME} SHARED
    src/mod/ForceEnchantMod.cpp
    src/config/Config.cpp
    src/hook/HookManager.cpp
    src/hook/ValidRangeHook.cpp
    src/hook/CanEnchantHook.cpp
)

target_include_directories(${MOD_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${PL_SDK_DIR}/include
)

target_link_libraries(${MOD_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    ${CMAKE_DL_LIBS}   # dladdr / dlfcn
)

# Link the preloader import library if present in the SDK. When building inside
# the official preloader-android template the symbols are provided by the loader
# at runtime, so allow undefined symbols rather than hard-failing the link.
find_library(PL_PRELOADER_LIB
    NAMES preloader pl
    PATHS "${PL_SDK_DIR}/lib/${ANDROID_ABI}" "${PL_SDK_DIR}/lib"
    NO_DEFAULT_PATH
)
if(PL_PRELOADER_LIB)
    target_link_libraries(${MOD_NAME} PRIVATE ${PL_PRELOADER_LIB})
else()
    message(WARNING
        "libpreloader not found under ${PL_SDK_DIR}; linking with "
        "--allow-shlib-undefined (symbols resolved by the loader at runtime).")
    target_link_options(${MOD_NAME} PRIVATE "LINKER:--allow-shlib-undefined")
endif()

# --- Size / visibility / optimisation ---------------------------------------
target_compile_options(${MOD_NAME} PRIVATE
    -O2
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
)

target_link_options(${MOD_NAME} PRIVATE
    "LINKER:--gc-sections"
)

# Strip only for non-debug builds.
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_options(${MOD_NAME} PRIVATE "LINKER:--strip-all")
endif()

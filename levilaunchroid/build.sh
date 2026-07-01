#!/usr/bin/env bash
# =============================================================================
#  ForceEnchant - local build script (Android arm64-v8a)
#
#  Requirements:
#    * Android NDK r28+ (set ANDROID_NDK_HOME or ANDROID_NDK_ROOT)
#    * cmake 3.22+, ninja, git, zip, network access (CMake fetches the SDK)
#
#  Usage:
#    ./build.sh                 # release build of libforceenchant.so
#    ABI=armeabi-v7a ./build.sh # 32-bit build
#
#  Output: build/libforceenchant.so  and  dist/ForceEnchant.levipack
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

ABI="${ABI:-arm64-v8a}"
API="${API:-28}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

NDK="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"
if [[ -z "$NDK" || ! -d "$NDK" ]]; then
  echo "ERROR: Set ANDROID_NDK_HOME (or ANDROID_NDK_ROOT) to your Android NDK (r28+)." >&2
  exit 1
fi

# --- Configure + build -------------------------------------------------------
# The Levi Launchroid SDK (LiteLDev/preloader-android) is fetched automatically
# by CMake (FetchContent). For an offline build, clone it and pass
#   -DPRELOADER_ANDROID_DIR=/path/to/preloader-android
CMAKE_EXTRA=()
if [[ -n "${PRELOADER_ANDROID_DIR:-}" ]]; then
  CMAKE_EXTRA+=("-DPRELOADER_ANDROID_DIR=${PRELOADER_ANDROID_DIR}")
fi

cmake -G Ninja -B build \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM="android-$API" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  "${CMAKE_EXTRA[@]}"

cmake --build build --config "$BUILD_TYPE"

# --- Package a .levipack -----------------------------------------------------
echo ">> Packaging .levipack..."
rm -rf dist/pkg && mkdir -p dist/pkg
cp build/libforceenchant.so dist/pkg/
cp manifest.json dist/pkg/
( cd dist/pkg && zip -r -X ../ForceEnchant.levipack . >/dev/null )

echo ""
echo "Build complete:"
echo "  $SCRIPT_DIR/build/libforceenchant.so"
echo "  $SCRIPT_DIR/dist/ForceEnchant.levipack"

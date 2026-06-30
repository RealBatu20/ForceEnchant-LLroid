#!/usr/bin/env bash
# =============================================================================
#  ForceEnchant - local build script (Android arm64-v8a)
#
#  Requirements:
#    * Android NDK r25c+ (set ANDROID_NDK_HOME or ANDROID_NDK_ROOT)
#    * cmake 3.21+, ninja, git
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
  echo "ERROR: Set ANDROID_NDK_HOME (or ANDROID_NDK_ROOT) to your Android NDK." >&2
  exit 1
fi

# --- Fetch the Levi Launchroid SDK headers (pl/cpp/*.hpp) --------------------
SDK_DIR="$SCRIPT_DIR/third_party/levilaunchroid-sdk"
if [[ ! -f "$SDK_DIR/pl/cpp/Mod.hpp" ]]; then
  echo ">> Fetching Levi Launchroid SDK headers..."
  rm -rf "$SCRIPT_DIR/third_party/_preloader"
  git clone --depth 1 https://github.com/LiteLDev/preloader-android.git \
      "$SCRIPT_DIR/third_party/_preloader"
  # Locate the directory that contains pl/cpp/Mod.hpp and expose it as the SDK.
  HDR="$(find "$SCRIPT_DIR/third_party/_preloader" -type f -path '*/pl/cpp/Mod.hpp' | head -n1 || true)"
  if [[ -z "$HDR" ]]; then
    echo "ERROR: Could not find pl/cpp/Mod.hpp in preloader-android." >&2
    echo "       Point LL_SDK_DIR at the SDK include root manually." >&2
    exit 1
  fi
  # SDK root = the dir two levels above pl/cpp (i.e. the parent of 'pl').
  PL_ROOT="$(dirname "$(dirname "$(dirname "$HDR")")")"
  mkdir -p "$SDK_DIR"
  cp -r "$PL_ROOT/pl" "$SDK_DIR/"
fi

# --- Configure + build -------------------------------------------------------
cmake -G Ninja -B build \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM="android-$API" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DLL_SDK_DIR="$SDK_DIR"

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

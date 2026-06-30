#!/usr/bin/env bash
# Build ForceEnchant for LeviLaunchroid (Android / arm64-v8a).
#
# Requirements:
#   * Android NDK r26+ (set ANDROID_NDK_HOME or ANDROID_NDK_ROOT)
#   * cmake >= 3.22, ninja
#   * LeviLaunchroid preloader SDK (headers + libpreloader.so); pass its path
#     with PL_SDK_DIR=... (defaults to ./third_party/levilaunchroid-sdk)
#
# Usage:
#   ./build.sh                       # release build, arm64-v8a
#   ABI=arm64-v8a ./build.sh         # explicit ABI
#   PL_SDK_DIR=/path/to/sdk ./build.sh
#
# Output: build/libforceenchant.so
set -euo pipefail

NDK="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"
if [[ -z "${NDK}" || ! -d "${NDK}" ]]; then
    echo "ERROR: Android NDK not found. Set ANDROID_NDK_HOME or ANDROID_NDK_ROOT." >&2
    exit 1
fi

ABI="${ABI:-arm64-v8a}"
API="${API:-21}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
PL_SDK_DIR="${PL_SDK_DIR:-$(pwd)/third_party/levilaunchroid-sdk}"
BUILD_DIR="${BUILD_DIR:-build}"

echo ">> NDK:         ${NDK}"
echo ">> ABI:         ${ABI}"
echo ">> API level:   ${API}"
echo ">> Build type:  ${BUILD_TYPE}"
echo ">> PL_SDK_DIR:  ${PL_SDK_DIR}"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${NDK}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="${ABI}" \
    -DANDROID_PLATFORM="android-${API}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DPL_SDK_DIR="${PL_SDK_DIR}"

cmake --build "${BUILD_DIR}" --parallel

echo ">> Done: ${BUILD_DIR}/libforceenchant.so"

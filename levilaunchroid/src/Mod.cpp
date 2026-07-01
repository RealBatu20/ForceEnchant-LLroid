// SPDX-License-Identifier: see repository LICENSE
//
// ForceEnchant - Levi Launchroid native mod entry point.
//
// Lifecycle mapping from the upstream Windows DLL:
//   DllMain(DLL_PROCESS_ATTACH) + init thread  ->  enable()
//   (MinHook init + HookManager::initHooks)        installs the enchant hooks.
//   DLL_PROCESS_DETACH                         ->  disable() / unload()

#include "Mod.h"
#include "hooks/EnchantHooks.h"

// pl/Logger.h uses std::vformat as a non-dependent name; include <format>
// first and build with an NDK whose libc++ provides it (r28+, see CI).
#include <format>

#include <pl/cpp/Mod.hpp>
#include <pl/cpp/mod/RegisterHelper.hpp>

// Our own log lines go through Android's facility, independent of the SDK
// logger's formatting path.
#include <android/log.h>

#define FE_LOG_TAG "ForceEnchant"

namespace fe {

ForceEnchantMod& ForceEnchantMod::getInstance() {
    static ForceEnchantMod instance;
    return instance;
}

bool ForceEnchantMod::load() {
    __android_log_print(ANDROID_LOG_INFO, FE_LOG_TAG, "Loaded");
    return true;
}

bool ForceEnchantMod::enable() {
    enabled_ = hooks::installEnchantPatch();
    if (enabled_) {
        __android_log_print(ANDROID_LOG_INFO, FE_LOG_TAG,
                            "Enabled (enchant limit removed)");
    } else {
        __android_log_print(ANDROID_LOG_WARN, FE_LOG_TAG,
                            "Enabled but patch NOT applied - update the pattern "
                            "in src/Signatures.h for your Minecraft Bedrock version.");
    }
    return true;
}

bool ForceEnchantMod::disable() {
    // The patch is a single-instruction NOP; it is left in place for the
    // process lifetime (the game reloads the library fresh on next launch).
    enabled_ = false;
    __android_log_print(ANDROID_LOG_INFO, FE_LOG_TAG, "Disabled");
    return true;
}

bool ForceEnchantMod::unload() {
    return true;
}

} // namespace fe

// Register the mod with the Levi Launchroid preloader.
PL_REGISTER_MOD(fe::ForceEnchantMod, fe::ForceEnchantMod::getInstance());

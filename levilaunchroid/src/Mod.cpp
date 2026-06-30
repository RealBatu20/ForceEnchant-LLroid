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

#include <pl/cpp/Mod.hpp>
#include <pl/cpp/mod/RegisterHelper.hpp>

// The SDK logger (pl/Logger.h) relies on std::vformat, which libc++ in the
// targeted NDK does not provide; log via Android's facility instead.
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
    int installed = hooks::installEnchantHooks();
    if (installed > 0) {
        __android_log_print(ANDROID_LOG_INFO, FE_LOG_TAG,
                            "Enabled (enchant limit removed)");
    } else {
        __android_log_print(ANDROID_LOG_WARN, FE_LOG_TAG,
                            "Enabled but NO hooks installed - verify signatures "
                            "in src/Signatures.h for your Minecraft Bedrock version.");
    }
    enabled_ = installed > 0;
    return true;
}

bool ForceEnchantMod::disable() {
    hooks::removeEnchantHooks();
    enabled_ = false;
    __android_log_print(ANDROID_LOG_INFO, FE_LOG_TAG, "Disabled");
    return true;
}

bool ForceEnchantMod::unload() {
    hooks::removeEnchantHooks();
    return true;
}

} // namespace fe

// Register the mod with the Levi Launchroid preloader.
PL_REGISTER_MOD(fe::ForceEnchantMod, fe::ForceEnchantMod::getInstance());

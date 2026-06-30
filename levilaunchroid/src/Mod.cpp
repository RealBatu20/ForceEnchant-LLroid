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

namespace fe {

ForceEnchantMod& ForceEnchantMod::getInstance() {
    static ForceEnchantMod instance;
    return instance;
}

bool ForceEnchantMod::load() {
    if (auto* mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("[ForceEnchant] Loaded");
    }
    return true;
}

bool ForceEnchantMod::enable() {
    int installed = hooks::installEnchantHooks();
    if (auto* mod = pl::mod::NativeMod::current()) {
        if (installed > 0) {
            mod->getLogger().info("[ForceEnchant] Enabled (enchant limit removed)");
        } else {
            mod->getLogger().warn("[ForceEnchant] Enabled but NO hooks installed - "
                                  "verify signatures in src/Signatures.h for your "
                                  "Minecraft Bedrock version.");
        }
    }
    enabled_ = installed > 0;
    return true;
}

bool ForceEnchantMod::disable() {
    hooks::removeEnchantHooks();
    enabled_ = false;
    if (auto* mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("[ForceEnchant] Disabled");
    }
    return true;
}

bool ForceEnchantMod::unload() {
    hooks::removeEnchantHooks();
    return true;
}

} // namespace fe

// Register the mod with the Levi Launchroid preloader.
PL_REGISTER_MOD(fe::ForceEnchantMod, fe::ForceEnchantMod::getInstance());

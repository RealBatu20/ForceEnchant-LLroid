// ForceEnchantMod.cpp
#include "mod/ForceEnchantMod.hpp"

#include <string>
#include <vector>

#include <pl/cpp/Mod.hpp>
#include <pl/cpp/mod/RegisterHelper.hpp>

#include "config/Config.hpp"
#include "hook/HookManager.hpp"

namespace fe {

ForceEnchantMod& ForceEnchantMod::getInstance() {
    static ForceEnchantMod instance;
    return instance;
}

bool ForceEnchantMod::load() {
    auto& self = getSelf();
    auto& logger = self.getLogger();

    logger.info("Loading {}", self.getName());

    const fe::config::Config cfg = fe::config::Config::load();

    const std::vector<std::string> report = fe::hook::installAll(cfg);
    bool anyInstalled = false;
    for (const std::string& line : report) {
        logger.info("[hook] {}", line);
        if (line.find("installed") != std::string::npos &&
            line.find("FAILED") == std::string::npos) {
            anyInstalled = true;
        }
    }
    hooksInstalled_ = anyInstalled;

    if (!anyInstalled) {
        logger.warn("No hooks were installed. ForceEnchant is inactive until valid "
                    "signatures for this Minecraft version are set in config.json "
                    "(see docs/SIGNATURES.md).");
    }

    // Returning true keeps the mod loaded so its status is visible in the launcher
    // even when signatures still need to be configured for the current version.
    return true;
}

bool ForceEnchantMod::enable() {
    return true;
}

bool ForceEnchantMod::disable() {
    if (hooksInstalled_) {
        fe::hook::removeAll();
        hooksInstalled_ = false;
        getSelf().getLogger().info("ForceEnchant hooks removed.");
    }
    return true;
}

bool ForceEnchantMod::unload() {
    return true;
}

} // namespace fe

// Register the mod with the LeviLaunchroid preloader.
PL_REGISTER_MOD(fe::ForceEnchantMod, fe::ForceEnchantMod::getInstance());

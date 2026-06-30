// ForceEnchantMod.hpp
#pragma once

namespace fe {

// LeviLaunchroid native mod implementing ForceEnchant.
// Lifecycle (per LeviLaunchroid mod API): load -> enable -> disable -> unload.
class ForceEnchantMod {
public:
    static ForceEnchantMod& getInstance();

    bool load();    // mod loaded: read config, install hooks
    bool enable();  // game about to start
    bool disable(); // game closing: remove hooks
    bool unload();  // final cleanup

private:
    bool hooksInstalled_ = false;
};

} // namespace fe

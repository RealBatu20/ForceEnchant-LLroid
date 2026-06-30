// SPDX-License-Identifier: see repository LICENSE
//
// ForceEnchant - Levi Launchroid native mod entry point.
#pragma once

namespace fe {

class ForceEnchantMod {
public:
    static ForceEnchantMod& getInstance();

    bool load();    // mod loaded into the process
    bool enable();  // before the game starts -> install hooks here
    bool disable(); // during game shutdown -> remove hooks
    bool unload();  // final cleanup

private:
    bool enabled_ = false;
};

} // namespace fe

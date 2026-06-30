// SPDX-License-Identifier: see repository LICENSE
//
// ForceEnchant hook installation for Levi Launchroid (Android arm64-v8a).
#pragma once

namespace fe::hooks {

// Resolves the target signatures and installs the enchant-related hooks.
// Returns the number of hooks successfully installed (0..2). Hooks whose
// signature fails to resolve are skipped (with a warning) rather than fatal.
int installEnchantHooks();

// Removes any hooks that were installed by installEnchantHooks().
void removeEnchantHooks();

} // namespace fe::hooks

// SPDX-License-Identifier: see repository LICENSE
//
// ForceEnchant patch installation for Levi Launchroid (Android arm64-v8a).
#pragma once

namespace fe::hooks {

// Locates the enchant-result routing branch in EnchantCommand::execute and
// neutralizes it (NOP) so above-vanilla enchantments are applied instead of
// rejected. Returns true if the patch was applied, false if the signature did
// not resolve (in which case nothing is modified).
bool installEnchantPatch();

} // namespace fe::hooks

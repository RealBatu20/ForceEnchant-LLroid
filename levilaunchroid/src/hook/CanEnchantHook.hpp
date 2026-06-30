// CanEnchantHook.hpp
// -----------------------------------------------------------------------------
// Port of the original Windows `ItemEnchantsHook::canEnchant`.
//
// Original behaviour (Force/Hooks/ItemEnchants/ItemEnchants.cpp):
//   Hooks the enchant-eligibility function and re-calls the original with its
//   `allowNonVanilla` flag forced to `true`, so any enchantment may be applied to
//   any item (the "force" in ForceEnchant) instead of being limited to vanilla
//   item/enchant combinations.
//
//   Windows prototype the author used:
//     EnchantResult* canEnchant(_this, EnchantResult* result,
//                               EnchantmentInstance* en, bool allowNonVanilla)
//   and the detour returned: original(_this, result, en, /*allowNonVanilla=*/true)
//
// Verification status on Android (HYPOTHESIS — confirm in IDA before trusting):
//   * The exact owning class/name (Windows comment says ItemEnchants::canEnchant;
//     LeviLamina references call the equivalent EnchantUtils::canEnchant) and the
//     argument layout must be confirmed against your arm64-v8a libminecraftpe.so.
//   * On arm64 a by-value struct return uses the indirect-result register x8; the
//     detour below treats all object arguments AND the return as opaque pointers
//     (x0..x3 / x0), which matches a function that returns a pointer. If IDA shows
//     a by-value (x8 sret) return, adjust the prototype here — this is the single
//     place to change.
//   The ONLY semantic change is forcing the trailing boolean argument to true.
// -----------------------------------------------------------------------------
#pragma once

#include <cstdint>

namespace fe::hook::can_enchant {

bool install(std::uintptr_t target);
void remove();

} // namespace fe::hook::can_enchant

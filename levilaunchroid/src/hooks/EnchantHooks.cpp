// SPDX-License-Identifier: see repository LICENSE
//
// =============================================================================
//  ForceEnchant - native enchant-limit removal for Minecraft Bedrock (Android)
// =============================================================================
//
//  Behaviour ported 1:1 from the upstream Windows DLL
//  (ForceEnchant/Force/Hooks/...):
//
//   1. ItemEnchants::canEnchant  -> force the `allowNonVanilla` argument to true
//      so enchantments above the vanilla level cap / non-vanilla combinations
//      are accepted by the engine's enchant-application path.
//
//   2. CommandUtils::validRange  -> short-circuit the range validation used by
//      `/enchant` so that out-of-range level arguments no longer raise
//      "commands.enchant.invalidLevel".
//
//  CALLING CONVENTION NOTE (Windows -> Android)
//  --------------------------------------------
//  The Windows build used __fastcall and an explicit `__int64 _this`. On
//  AArch64 there is a single standard calling convention (AAPCS64): the implicit
//  `this` pointer is simply the first argument in x0. The detour signatures
//  below therefore use plain C++ functions whose first parameter is the object
//  pointer for member functions. No __fastcall / no MinHook is involved; hooks
//  go through Levi Launchroid's Gloss-backed pl::hook API.
//
//  COMPLIANCE
//  ----------
//  This reimplements the exact, unmodified intent of the open-source upstream
//  project (removing the enchantment level limit for the local client). It uses
//  only documented Levi Launchroid APIs and reversible function hooks. It does
//  not read other players' data, automate combat, evade anti-cheat, or perform
//  any packet abuse.
// =============================================================================

#include "EnchantHooks.h"

#include "../Signatures.h"
#include "../sdk/Enchant.h"

// pl/Logger.h uses std::vformat/std::make_format_args but does not include
// <format> itself; include it before the SDK headers so the logger compiles.
#include <format>

#include <pl/cpp/Hook.hpp>
#include <pl/cpp/Mod.hpp>
#include <pl/cpp/Signature.hpp>

#include <cstdint>

namespace fe::hooks {
namespace {

using fe::sdk::EnchantmentInstance;
using fe::sdk::EnchantResult;

// ---- Original function pointers (filled in by pl::hook::hook) ----------------

using CanEnchantFn = EnchantResult* (*)(void* self,
                                        EnchantResult* out,
                                        EnchantmentInstance* inst,
                                        bool allowNonVanilla);

using ValidRangeFn = bool (*)(int value, int low, int high, void* outName);

CanEnchantFn g_origCanEnchant = nullptr;
ValidRangeFn g_origValidRange = nullptr;

// Resolved target addresses, kept so removeEnchantHooks() can unhook cleanly.
uintptr_t g_canEnchantAddr = 0;
uintptr_t g_validRangeAddr = 0;

inline void logInfo(const char* msg) {
    if (auto mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info(msg);
    }
}

inline void logWarn(const char* msg) {
    if (auto mod = pl::mod::NativeMod::current()) {
        mod->getLogger().warn(msg);
    }
}

// ---- Detours -----------------------------------------------------------------

// Force allowNonVanilla = true so the engine accepts above-cap enchantments.
EnchantResult* hookCanEnchant(void* self,
                              EnchantResult* out,
                              EnchantmentInstance* inst,
                              bool /*allowNonVanilla*/) {
    return g_origCanEnchant(self, out, inst, /*allowNonVanilla=*/true);
}

// Report every level as in-range so /enchant never rejects the level argument.
bool hookValidRange(int /*value*/, int /*low*/, int /*high*/, void* /*outName*/) {
    return true;
}

} // namespace

int installEnchantHooks() {
    int installed = 0;

    // --- ItemEnchants::canEnchant ------------------------------------------
    g_canEnchantAddr = pl::signature::resolveSignature(fe::sig::kCanEnchant,
                                                       fe::sig::kModule);
    if (g_canEnchantAddr != 0) {
        int rc = pl::hook::hook(
            reinterpret_cast<void*>(g_canEnchantAddr),
            reinterpret_cast<void*>(&hookCanEnchant),
            reinterpret_cast<void**>(&g_origCanEnchant),
            pl::hook::PriorityNormal);
        if (rc == 0 && g_origCanEnchant != nullptr) {
            logInfo("[ForceEnchant] Hooked ItemEnchants::canEnchant");
            ++installed;
        } else {
            g_canEnchantAddr = 0;
            logWarn("[ForceEnchant] Failed to install canEnchant hook");
        }
    } else {
        logWarn("[ForceEnchant] Could not resolve canEnchant signature "
                "(update src/Signatures.h for your MC version)");
    }

    // --- CommandUtils::validRange ------------------------------------------
    g_validRangeAddr = pl::signature::resolveSignature(fe::sig::kValidRange,
                                                       fe::sig::kModule);
    if (g_validRangeAddr != 0) {
        int rc = pl::hook::hook(
            reinterpret_cast<void*>(g_validRangeAddr),
            reinterpret_cast<void*>(&hookValidRange),
            reinterpret_cast<void**>(&g_origValidRange),
            pl::hook::PriorityNormal);
        if (rc == 0 && g_origValidRange != nullptr) {
            logInfo("[ForceEnchant] Hooked CommandUtils::validRange");
            ++installed;
        } else {
            g_validRangeAddr = 0;
            logWarn("[ForceEnchant] Failed to install validRange hook");
        }
    } else {
        logWarn("[ForceEnchant] Could not resolve validRange signature "
                "(update src/Signatures.h for your MC version)");
    }

    return installed;
}

void removeEnchantHooks() {
    if (g_canEnchantAddr != 0) {
        pl::hook::unhook(reinterpret_cast<void*>(g_canEnchantAddr),
                         reinterpret_cast<void*>(&hookCanEnchant));
        g_canEnchantAddr = 0;
        g_origCanEnchant = nullptr;
    }
    if (g_validRangeAddr != 0) {
        pl::hook::unhook(reinterpret_cast<void*>(g_validRangeAddr),
                         reinterpret_cast<void*>(&hookValidRange));
        g_validRangeAddr = 0;
        g_origValidRange = nullptr;
    }
}

} // namespace fe::hooks

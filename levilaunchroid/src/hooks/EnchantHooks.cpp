// SPDX-License-Identifier: see repository LICENSE
//
// =============================================================================
//  ForceEnchant - native enchant-limit removal for Minecraft Bedrock (Android)
// =============================================================================
//
//  In this Minecraft Bedrock build, ItemEnchants::canEnchant and the enchant
//  level-range validation are inlined into EnchantCommand::execute (verified by
//  disassembly of the target libminecraftpe.so). There is no standalone
//  function to hook, so instead of GlossHook we apply a single, reversible
//  in-memory patch: NOP the branch that routes a "cannot enchant / over-limit"
//  result to the reject path, so the enchantment falls through to the apply
//  path. See src/Signatures.h for the byte pattern and full analysis.
//
//  Safety: if the signature does not resolve, nothing is written. The patch
//  touches exactly 4 bytes (one instruction) and restores page protection.
//
//  COMPLIANCE: reimplements the intent of the open-source upstream ForceEnchant
//  (remove the enchant level limit for the local client). Reversible, single
//  instruction, no packet abuse, no anti-cheat evasion, no remote access.
// =============================================================================

#include "EnchantHooks.h"

#include "../Signatures.h"

#include <pl/cpp/Signature.hpp>

#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

namespace fe::hooks {
namespace {

constexpr const char* kTag = "ForceEnchant";

// Make [addr, addr+len) writable, copy `bytes`, flush icache, restore R+X.
bool patchBytes(uintptr_t addr, const uint8_t* bytes, size_t len) {
    const size_t pageSize = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    const uintptr_t pageStart = addr & ~(pageSize - 1);
    // The patch may straddle a page boundary; cover two pages to be safe.
    const size_t span = (addr + len) - pageStart;

    if (mprotect(reinterpret_cast<void*>(pageStart), span,
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }

    std::memcpy(reinterpret_cast<void*>(addr), bytes, len);
    __builtin___clear_cache(reinterpret_cast<char*>(addr),
                            reinterpret_cast<char*>(addr + len));

    // Restore to executable + readable (drop write).
    mprotect(reinterpret_cast<void*>(pageStart), span,
             PROT_READ | PROT_EXEC);
    return true;
}

} // namespace

bool installEnchantPatch() {
    const uintptr_t base =
        pl::signature::resolveSignature(fe::sig::kEnchantRoutePattern,
                                        fe::sig::kModule);
    if (base == 0) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Could not resolve enchant-route pattern "
                            "(update src/Signatures.h for your MC version)");
        return false;
    }

    const uintptr_t tbnzAddr = base + fe::sig::kTbnzOffset;
    if (!patchBytes(tbnzAddr, fe::sig::kNop, sizeof(fe::sig::kNop))) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Failed to patch enchant-route branch at %p",
                            reinterpret_cast<void*>(tbnzAddr));
        return false;
    }

    __android_log_print(ANDROID_LOG_INFO, kTag,
                        "Patched enchant-route branch at %p (limit removed)",
                        reinterpret_cast<void*>(tbnzAddr));
    return true;
}

} // namespace fe::hooks

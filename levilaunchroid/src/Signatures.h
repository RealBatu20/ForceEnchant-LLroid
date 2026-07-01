// SPDX-License-Identifier: see repository LICENSE
//
// =============================================================================
//  ForceEnchant - target signature for libminecraftpe.so (Android arm64-v8a)
// =============================================================================
//
//  These values were derived by DIRECT ANALYSIS of a real arm64-v8a
//  libminecraftpe.so (disassembly with capstone/pyelftools), not guessed.
//
//  FINDING (FACT): In this Minecraft Bedrock build, ItemEnchants::canEnchant and
//  the level-range validation are INLINED into EnchantCommand::execute; there
//  are no standalone exported functions to hook. The command calls a check
//  routine and then routes each per-enchantment result through a switch:
//
//      ldurb w9, [x29, #-0xc0]        ; w9 = enchant-result type
//      ...
//      tbnz  w9, #0, <cantEnchant>    ; bit0 set => reject ("can't enchant")
//      <fall-through>                 ; => accept: apply the enchantment
//
//  EXPERIMENTAL PATCH: NOP the `tbnz` so the reject branch is never taken and
//  every enchantment (including above-vanilla levels) falls through to the
//  apply path. This is the arm64 equivalent of the Windows mod forcing
//  allowNonVanilla = true.
//
//  The signature is a unique 20-byte pattern anchored on the instructions
//  immediately preceding and including that `tbnz`. If a future game update
//  shifts these bytes, re-derive the pattern from the new binary. The patch is
//  crash-safe: if the pattern does not resolve, no memory is modified and a
//  warning is logged.
// =============================================================================
#pragma once

#include <cstddef>
#include <cstdint>

namespace fe::sig {

// Target module name (Android arm64-v8a Minecraft Bedrock binary).
inline constexpr const char* kModule = "libminecraftpe.so";

// Unique anchor pattern ending with the routing `tbnz w9, #0, <cantEnchant>`.
// Layout of the 20 bytes (5 instructions):
//   +0x00: stp/str ...            (A9 03 54 38)
//   +0x04: add x8, sp, #0x98      (E8 63 02 91)
//   +0x08: str x8, [sp, #0xf0]    (E8 7B 00 F9)
//   +0x0C: strb wzr, [sp, #0xf8]  (FF E3 03 39)
//   +0x10: tbnz w9, #0, ...       (09 61 00 37)  <-- instruction to NOP
inline constexpr const char* kEnchantRoutePattern =
    "A9 03 54 38 E8 63 02 91 E8 7B 00 F9 FF E3 03 39 09 61 00 37";

// Byte offset from the pattern start to the `tbnz` instruction to neutralize.
inline constexpr size_t kTbnzOffset = 0x10;

// AArch64 NOP (0xD503201F), little-endian byte order.
inline constexpr uint8_t kNop[4] = {0x1F, 0x20, 0x03, 0xD5};

} // namespace fe::sig

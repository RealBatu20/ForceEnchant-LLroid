// SPDX-License-Identifier: see repository LICENSE
//
// Minimal Bedrock SDK type stubs used by ForceEnchant.
//
// These mirror the layout the original Windows build relied on
// (see ../../../ForceEnchant/Force/SDK/EnchantResult.h). Field offsets are the
// SAME engine-level layout used by Minecraft Bedrock and are independent of the
// Windows/Android split, BUT they must still be confirmed against the target
// libminecraftpe.so for the exact game version you ship for (see Signatures.h).
//
// Classification of the offsets below:
//   FACT      - taken verbatim from the upstream Windows ForceEnchant SDK.
//   INFERENCE - assumed identical on the Android ARM64 build (same engine
//               struct, compiler-independent member layout).
//   HYPOTHESIS - must be re-verified in IDA for your exact MC version.
#pragma once

#include <cstdint>

namespace fe::sdk {

// EnchantmentInstance: { int type; int level; }
class EnchantmentInstance {
public:
    int enchantType; // +0x0
    int level;       // +0x4

    [[nodiscard]] int getType() const { return enchantType; }
    [[nodiscard]] int getLevel() const { return level; }
};

// EnchantResult layout (FACT from upstream Windows SDK):
//   +0x0  int enchantResultType
//   +0x8  int enchantId
//   +0x10 int level
class EnchantResult {
public:
    int  enchantResultType; // +0x0
    int  _pad0;             // +0x4 (alignment for the +0x8 int)
    int  enchantId;         // +0x8
    int  _pad1;             // +0xC
    int  level;             // +0x10
};

} // namespace fe::sdk

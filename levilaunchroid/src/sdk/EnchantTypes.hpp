// EnchantTypes.hpp
// -----------------------------------------------------------------------------
// Portable (Windows-free) reconstruction of the small Minecraft Bedrock enchant
// data structures referenced by the original Windows ForceEnchant mod.
//
// IMPORTANT — verification status:
//   The field OFFSETS below are INFERENCE, carried over from the original
//   Windows x86-64 build (Force/SDK/EnchantResult.h). They are NOT confirmed for
//   the Android arm64-v8a `libminecraftpe.so` and MUST be re-verified in IDA Pro
//   / Ghidra before any code is allowed to *read* these fields.
//
//   The ForceEnchant hooks in this project intentionally treat these objects as
//   OPAQUE pointers (see src/hook/*). They never dereference the fields, so the
//   correctness of the mod does NOT depend on these offsets. The struct is kept
//   only as documentation and as a starting point for future features.
// -----------------------------------------------------------------------------
#pragma once

#include <cstdint>

namespace fe::sdk {

// Generic, ABI-neutral member accessor (replaces the Windows __declspec(property)
// based BUILD_ACCESS macro from the original project). Reads/writes a value of
// type T at a byte `Offset` from the object base.
template <typename T>
inline T& fieldAt(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}

// Mirrors Windows `EnchantMentInstance` (a single enchantment + level).
// Offsets are INFERENCE (Windows layout) — re-verify on Android before use.
struct EnchantmentInstance {
    static constexpr std::size_t kEnchantTypeOffset = 0x0;
    static constexpr std::size_t kLevelOffset       = 0x4;

    int enchantType() { return fieldAt<int>(this, kEnchantTypeOffset); }
    int level()       { return fieldAt<int>(this, kLevelOffset); }
};

// Mirrors Windows `EnchantResult`.
// Offsets are INFERENCE (Windows layout) — re-verify on Android before use.
struct EnchantResult {
    static constexpr std::size_t kResultTypeOffset = 0x0;
    static constexpr std::size_t kEnchantIdOffset  = 0x8;
    static constexpr std::size_t kLevelOffset      = 0x10;

    int resultType() { return fieldAt<int>(this, kResultTypeOffset); }
    int enchantId()  { return fieldAt<int>(this, kEnchantIdOffset); }
    int level()      { return fieldAt<int>(this, kLevelOffset); }
};

} // namespace fe::sdk

// Config.hpp
// -----------------------------------------------------------------------------
// Runtime configuration for ForceEnchant (LeviLaunchroid / Android).
//
// Why config-driven signatures?
//   Bedrock's `libminecraftpe.so` changes between game versions, so any byte
//   pattern or symbol used to locate a function is version-specific. Keeping the
//   signatures in an external `config.json` lets a user update the mod for a new
//   Minecraft version by editing one JSON file — no recompile required.
//
//   Out of the box the signature fields are EMPTY. An empty signature is treated
//   as "not configured": the corresponding hook is skipped and a clear message is
//   logged. The mod still loads cleanly. See docs/SIGNATURES.md for how to
//   generate the correct values in IDA Pro / Ghidra for your exact game version.
// -----------------------------------------------------------------------------
#pragma once

#include <string>

namespace fe::config {

// One configurable hook target.
struct HookSetting {
    bool        enabled   = true;
    // A LeviLaunchroid signature string. Accepted by pl::signature::resolveSignature
    // as EITHER a symbol name (e.g. "_ZN12EnchantUtils10canEnchant...") OR an
    // arm64-v8a byte pattern (e.g. "FF 83 01 D1 ?? ?? ?? ??"). Empty = not set.
    std::string signature = "";
};

struct Config {
    HookSetting validRange;   // Command::validRange  -> bypass /enchant level bounds
    HookSetting canEnchant;   // EnchantUtils::canEnchant -> allow non-vanilla combos

    // Loads config.json located next to this mod's .so (./config/config.json,
    // falling back to ./config.json). Missing or malformed files are tolerated:
    // the returned Config simply keeps the compiled defaults (empty signatures).
    // JSON line comments ("// ...") are permitted.
    static Config load();
};

} // namespace fe::config

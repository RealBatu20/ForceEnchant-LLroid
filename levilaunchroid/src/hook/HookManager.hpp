// HookManager.hpp
// -----------------------------------------------------------------------------
// Installs the ForceEnchant hooks against the running `libminecraftpe.so`.
//
// Each hook is resolved through pl::signature::resolveSignature(), which accepts
// either a symbol name or an arm64-v8a byte pattern. Resolution failures and
// disabled/unconfigured hooks are reported (never fatal): the mod keeps loading.
// -----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

#include "config/Config.hpp"

namespace fe::hook {

constexpr const char* kTargetModule = "libminecraftpe.so";

// Installs every enabled+configured hook described by `cfg`.
// Returns one human-readable status line per hook for the caller to log.
std::vector<std::string> installAll(const fe::config::Config& cfg);

// Removes any hooks that were installed (used on disable()/unload()).
void removeAll();

} // namespace fe::hook

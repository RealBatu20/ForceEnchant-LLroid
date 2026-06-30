// HookManager.cpp
#include "hook/HookManager.hpp"

#include <cstdint>

#include <pl/cpp/Signature.hpp>

#include "hook/CanEnchantHook.hpp"
#include "hook/ValidRangeHook.hpp"

namespace fe::hook {

namespace {

using InstallFn = bool (*)(std::uintptr_t);

// Resolves one configured hook and installs it, producing a status line.
std::string installOne(const char* label,
                       const fe::config::HookSetting& setting,
                       InstallFn installer) {
    std::string head = std::string(label) + ": ";

    if (!setting.enabled) {
        return head + "disabled in config";
    }
    if (setting.signature.empty()) {
        return head + "skipped (no signature configured — see docs/SIGNATURES.md)";
    }

    const std::uintptr_t addr =
        pl::signature::resolveSignature(setting.signature, kTargetModule);
    if (addr == 0) {
        return head + "FAILED to resolve signature in " + kTargetModule +
               " (signature out of date for this game version?)";
    }
    if (!installer(addr)) {
        return head + "resolved but hook installation FAILED";
    }
    return head + "installed";
}

} // namespace

std::vector<std::string> installAll(const fe::config::Config& cfg) {
    std::vector<std::string> report;
    report.push_back(installOne("Command::validRange", cfg.validRange,
                                &fe::hook::valid_range::install));
    report.push_back(installOne("EnchantUtils::canEnchant", cfg.canEnchant,
                                &fe::hook::can_enchant::install));
    return report;
}

void removeAll() {
    fe::hook::valid_range::remove();
    fe::hook::can_enchant::remove();
}

} // namespace fe::hook

// Config.cpp
#include "config/Config.hpp"

#include <dlfcn.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

#include <nlohmann/json.hpp>

namespace fe::config {

namespace {

// Resolve the directory that this shared object was loaded from, so config files
// can be located relative to the .so regardless of the process working dir.
std::optional<std::filesystem::path> selfDir() {
    Dl_info info{};
    // Any symbol address inside this module works; use selfDir itself.
    if (dladdr(reinterpret_cast<const void*>(&selfDir), &info) == 0 || info.dli_fname == nullptr) {
        return std::nullopt;
    }
    std::error_code ec;
    std::filesystem::path p = std::filesystem::path(info.dli_fname);
    std::filesystem::path dir = p.parent_path();
    if (dir.empty()) return std::nullopt;
    return dir;
}

// First existing candidate path for config.json.
std::optional<std::filesystem::path> findConfigFile() {
    auto dir = selfDir();
    if (!dir) return std::nullopt;
    const std::filesystem::path candidates[] = {
        *dir / "config" / "config.json",
        *dir / "config.json",
    };
    std::error_code ec;
    for (const auto& c : candidates) {
        if (std::filesystem::exists(c, ec)) return c;
    }
    return std::nullopt;
}

void readHook(const nlohmann::json& parent, const char* key, HookSetting& out) {
    if (!parent.contains(key) || !parent.at(key).is_object()) return;
    const auto& node = parent.at(key);
    if (node.contains("enabled") && node.at("enabled").is_boolean()) {
        out.enabled = node.at("enabled").get<bool>();
    }
    if (node.contains("signature") && node.at("signature").is_string()) {
        out.signature = node.at("signature").get<std::string>();
    }
}

} // namespace

Config Config::load() {
    Config cfg; // compiled defaults (signatures empty -> hooks skipped + logged)

    auto path = findConfigFile();
    if (!path) return cfg;

    std::ifstream in(*path, std::ios::binary);
    if (!in) return cfg;

    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();

    // Tolerant parse: allow comments, never throw out of load().
    nlohmann::json root =
        nlohmann::json::parse(text, /*cb=*/nullptr, /*allow_exceptions=*/false,
                              /*ignore_comments=*/true);
    if (root.is_discarded() || !root.is_object()) return cfg;

    const nlohmann::json* hooks = &root;
    if (root.contains("hooks") && root.at("hooks").is_object()) {
        hooks = &root.at("hooks");
    }

    readHook(*hooks, "validRange", cfg.validRange);
    readHook(*hooks, "canEnchant", cfg.canEnchant);

    return cfg;
}

} // namespace fe::config

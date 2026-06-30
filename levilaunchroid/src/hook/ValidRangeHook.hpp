// ValidRangeHook.hpp
// -----------------------------------------------------------------------------
// Port of the original Windows `CommandHook::ValidRange`.
//
// Original behaviour (Force/Hooks/CommandHook/CommandHook.cpp):
//   Hooks `Command::validRange(int input, int low, int high, ...)` — the bounds
//   check `/enchant` uses to reject "commands.enchant.invalidLevel" — and makes
//   it unconditionally return true, so any level passed to the enchant command is
//   accepted instead of being clamped/rejected.
//
// Reverse-engineering lead (same as the original author's note):
//   In IDA/Ghidra, search the string "commands.enchant.invalidLevel" and follow
//   its xref into EnchantCommand::execute. The first range guard called there is
//   Command::validRange. Generate the signature from that function.
// -----------------------------------------------------------------------------
#pragma once

#include <string>

namespace fe::hook::valid_range {

// Installs the hook at the resolved address `target` (already non-zero).
// Returns true on success.
bool install(std::uintptr_t target);
void remove();

} // namespace fe::hook::valid_range

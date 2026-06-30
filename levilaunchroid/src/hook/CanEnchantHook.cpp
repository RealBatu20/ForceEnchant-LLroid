// CanEnchantHook.cpp
#include "hook/CanEnchantHook.hpp"

#include <pl/cpp/Hook.hpp>

namespace fe::hook::can_enchant {

namespace {

// Opaque-pointer forwarding prototype (see header for the ABI rationale).
//   a0/a1/a2 : object pointers (this / result / EnchantmentInstance), forwarded
//   allowNonVanilla : the gate we override to true
//   return   : pointer result, forwarded
using Fn = void* (*)(void* a0, void* a1, void* a2, bool allowNonVanilla);

Fn g_original = nullptr;
void* g_target = nullptr;

void* detour(void* a0, void* a1, void* a2, bool /*allowNonVanilla*/) {
    // Re-enter the original with the non-vanilla restriction lifted.
    return g_original(a0, a1, a2, /*allowNonVanilla=*/true);
}

} // namespace

bool install(std::uintptr_t target) {
    g_target = reinterpret_cast<void*>(target);
    const int rc = pl::hook::hook(g_target,
                                  reinterpret_cast<void*>(&detour),
                                  reinterpret_cast<void**>(&g_original),
                                  pl::hook::PriorityNormal);
    return rc == 0;
}

void remove() {
    if (g_target != nullptr) {
        pl::hook::unhook(g_target, reinterpret_cast<void*>(&detour));
        g_target = nullptr;
        g_original = nullptr;
    }
}

} // namespace fe::hook::can_enchant

// ValidRangeHook.cpp
#include "hook/ValidRangeHook.hpp"

#include <cstdint>

#include <pl/cpp/Hook.hpp>

namespace fe::hook::valid_range {

namespace {

// Prototype carried over from the Windows build:
//   bool Command::validRange(int input, int low, int high, <out/message>)
//
// FACT (from original source): the detour must return `true` unconditionally and
// must NOT call the original — the original would re-apply the range limit.
//
// ABI note (arm64-v8a): the integer/pointer arguments arrive in x0..x3, so this
// flat prototype is ABI-correct for forwarding/ignoring them. Because the detour
// never touches the trailing argument, its exact type is irrelevant; void* keeps
// it ABI-neutral. Confirm the argument count in IDA if you extend this hook.
using Fn = bool (*)(int input, int low, int high, void* out);

Fn g_original = nullptr;
void* g_target = nullptr;

bool detour(int /*input*/, int /*low*/, int /*high*/, void* /*out*/) {
    return true; // every level is "in range"
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

} // namespace fe::hook::valid_range

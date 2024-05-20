#pragma once

namespace hctx {

constexpr auto kStackSize = 1024 * 1024;

constexpr void* to_stack(void* allocated_mem) {
  return static_cast<char*>(allocated_mem) + kStackSize;
}

constexpr void* to_allocation(void* stack) {
  return static_cast<char*>(stack) - kStackSize;
}

}  // namespace hctx
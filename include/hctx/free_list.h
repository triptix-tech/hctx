#pragma once

#include <atomic>

namespace hctx {

struct free_list {
  inline void* take() {
    while (true) {
      auto ptr = next_.load();
      if (ptr == nullptr) {
        return nullptr;
      }
      if (next_.compare_exchange_weak(ptr, ptr->next_)) {
        return ptr;
      }
    }
  }

  inline void push(void* p) {
    auto const mem_ptr = reinterpret_cast<free_list*>(p);
    while (true) {
      auto next = next_.load();
      if (next_.compare_exchange_weak(next, mem_ptr)) {
        mem_ptr->next_ = next;
        break;
      }
    }
  }

  std::atomic<free_list*> next_{nullptr};
};

}  // namespace hctx
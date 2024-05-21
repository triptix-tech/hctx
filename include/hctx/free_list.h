#pragma once

#include <atomic>

#include "cista/containers/vector.h"

#include "hctx/stack.h"

namespace hctx {

struct free_list {
  explicit free_list(unsigned const n) { base_.resize(n * kStackSize); }

  ~free_list() {
    void* x = nullptr;
    while ((x = take()) != nullptr) {
      //      fprintf(stderr, "FREE %p\n", x);
      //      fflush(stderr);
      if (!base_.contains(static_cast<std::uint8_t const*>(x))) {
        std::free(x);
      }
    }
  }

  inline void* take() {
    auto const l = std::scoped_lock{m_};

    if (pos_ < base_.size()) {
      auto const ptr = &base_[pos_];
      pos_ += kStackSize;
      return ptr;
    }

    if (next_ == nullptr) {
      //      fprintf(stderr, "POP NULL\n");
      //      fflush(stderr);
      return nullptr;
    }
    auto const ptr = next_;
    next_ = next_->next_;
    //    fprintf(stderr, "POP %p\n", static_cast<void*>(ptr));
    //    fflush(stderr);
    return ptr;
  }

  inline void push(void* p) {
    //    fprintf(stderr, "PUSH %p\n", p);
    //    fflush(stderr);
    auto const l = std::scoped_lock{m_};
    auto const mem_ptr = reinterpret_cast<node*>(p);
    mem_ptr->next_ = next_;
    next_ = mem_ptr;
  }

  struct node {
    node* next_{nullptr};
  };

  std::mutex m_;
  node* next_{nullptr};

  cista::raw::vector<std::uint8_t> base_;
  std::uint32_t pos_{0U};
};

}  // namespace hctx
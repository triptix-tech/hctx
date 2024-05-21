#pragma once

#include <memory>

namespace hctx {

struct op;

struct cv {
  cv();

  template <typename Predicate>
  void wait(Predicate&& pred) {
    while (!std::forward<Predicate>(pred)()) {
      wait();
    }
  }

  static std::weak_ptr<op> get_caller();
  void wait();
  void notify();

  std::weak_ptr<op> caller_;
};

}  // namespace hctx
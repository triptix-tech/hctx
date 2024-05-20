#pragma once

#include <memory>

#include "hctx/op.h"
#include "hctx/scheduler.h"

namespace hctx {

struct condition_variable {
  condition_variable() : caller_{get_caller()} {}

  static std::weak_ptr<op> get_caller() {
    if (this_op == nullptr) {
      return std::weak_ptr<op>();
    } else {
      return this_op->shared_from_this();
    }
  }

  template <typename Predicate>
  void wait(Predicate&& pred) {
    while (!std::forward<Predicate>(pred)()) {
      wait();
    }
  }

  void wait() {
    auto caller_lock = caller_.lock();
    assert(caller_lock);
    caller_lock->suspend(/* finished = */ false);
  }

  void notify() {
    auto caller_lock = caller_.lock();
    if (caller_lock) {
      caller_lock->sched_.enqueue_high_prio(caller_lock);
    }
  }

  std::weak_ptr<op> caller_;
};

}  // namespace hctx
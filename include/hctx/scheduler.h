#pragma once

#include <atomic>
#include <memory>

#include "blockingconcurrentqueue.h"

#include "hctx/free_list.h"
#include "hctx/future.h"
#include "hctx/stack.h"

namespace hctx {

struct op;

using op_ptr = std::shared_ptr<op>;

struct scheduler {
  scheduler(free_list& stacks) : stacks_{stacks} {}

  scheduler(scheduler const&) = delete;
  scheduler& operator=(scheduler const&) = delete;

  template <typename Fn>
  future_ptr<std::result_of_t<Fn>> post_high_prio(Fn&& f,
                                                  void* user_data = nullptr);

  template <typename Fn>
  future_ptr<std::result_of_t<Fn>> post_low_prio(Fn&& f,
                                                 void* user_data = nullptr);

  void enqueue_high_prio(op_ptr o) { high_prio_q_.enqueue(std::move(o)); }
  void enqueue_low_prio(op_ptr o) { low_prio_q_.enqueue(std::move(o)); }

  void* get_stack() {
    auto const stack = stacks_.take();
    if (stack != nullptr) {
      return stack;
    } else {
      auto mem = std::malloc(kStackSize);
      if (mem == nullptr) {
        throw std::bad_alloc{};
      }
      return to_stack(mem);
    }
  }

  free_list& stacks_;
  moodycamel::ConcurrentQueue<op_ptr> high_prio_q_, low_prio_q_;
};

}  // namespace hctx
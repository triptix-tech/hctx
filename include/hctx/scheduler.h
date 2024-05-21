#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "blockingconcurrentqueue.h"

#include "hctx/free_list.h"
#include "hctx/future.h"
#include "hctx/stack.h"

namespace hctx {

struct op;

using op_ptr = std::shared_ptr<op>;

struct scheduler {
  scheduler(free_list& stacks) : stack_mem_{stacks} {}
  ~scheduler() = default;
  scheduler(scheduler const&) = delete;
  scheduler& operator=(scheduler const&) = delete;
  scheduler(scheduler&&) = delete;
  scheduler& operator=(scheduler&&) = delete;

  template <typename Fn>
  future_ptr<std::invoke_result_t<Fn>> post_high_prio(
      Fn&& f, void* user_data = nullptr);

  template <typename Fn>
  future_ptr<std::invoke_result_t<Fn>> post_low_prio(Fn&& f,
                                                     void* user_data = nullptr);

  void enqueue_high_prio(op_ptr o) { high_prio_q_.enqueue(std::move(o)); }
  void enqueue_low_prio(op_ptr o) { low_prio_q_.enqueue(std::move(o)); }

  void* get_stack() {
    auto const free_list_mem = stack_mem_.take();
    if (free_list_mem != nullptr) {
      return to_stack(free_list_mem);
    } else {
      auto mem = std::malloc(kStackSize);
      if (mem == nullptr) {
        throw std::bad_alloc{};
      }
      return to_stack(mem);
    }
  }

  void run() {
    auto op = hctx::op_ptr{};
    while (low_prio_q_.try_dequeue(op)) {
      op->resume();
      while (high_prio_q_.try_dequeue(op)) {
        op->resume();
      }
    }
  }

  void run_parallel(
      unsigned const concurrency = std::thread::hardware_concurrency()) {
    auto workers = std::vector<std::thread>(concurrency);
    for (auto& w : workers) {
      w = std::thread{[this]() { run(); }};
    }
    for (auto& w : workers) {
      w.join();
    }
  }

  free_list& stack_mem_;
  moodycamel::ConcurrentQueue<op_ptr> high_prio_q_, low_prio_q_;
};

}  // namespace hctx
#pragma once

#include "hctx/op.h"

namespace hctx {

template <typename Fn>
future_ptr<std::invoke_result_t<Fn>> scheduler::post_high_prio(
    Fn&& f, void* user_data) {
  using ret_t = std::invoke_result_t<Fn>;
  using fut_t = future<ret_t>;
  auto fut = std::make_shared<fut_t>();
  auto o = std::make_shared<op>(*this, user_data, get_stack(),
                                [fx = std::forward<Fn>(f), fut]() {
                                  std::exception_ptr ex;
                                  try {
                                    if constexpr (std::is_same_v<void, ret_t>) {
                                      fx();
                                      fut->set();
                                    } else {
                                      fut->set(fx());
                                    }
                                    return;
                                  } catch (...) {
                                    ex = std::current_exception();
                                  }
                                  fut->set(ex);
                                });
  high_prio_q_.enqueue(std::move(o));
  return fut;
}

template <typename Fn>
future_ptr<std::invoke_result_t<Fn>> scheduler::post_low_prio(Fn&& f,
                                                              void* user_data) {
  using ret_t = std::invoke_result_t<Fn>;
  using fut_t = future<ret_t>;
  auto fut = std::make_shared<fut_t>();
  auto o = std::make_shared<op>(*this, user_data, get_stack(),
                                [fx = std::forward<Fn>(f), fut]() {
                                  std::exception_ptr ex;
                                  try {
                                    if constexpr (std::is_same_v<void, ret_t>) {
                                      fx();
                                      fut->set();
                                    } else {
                                      fut->set(fx());
                                    }
                                    return;
                                  } catch (...) {
                                    ex = std::current_exception();
                                  }
                                  fut->set(ex);
                                });
  low_prio_q_.enqueue(std::move(o));
  return fut;
}

}  // namespace hctx
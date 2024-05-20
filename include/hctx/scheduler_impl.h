#pragma once

#include "hctx/op.h"

namespace hctx {

template <typename Fn>
future_ptr<std::result_of_t<Fn>> scheduler::post_high_prio(Fn&& f,
                                                           void* user_data) {
  using fut_t = future<std::result_of_t<Fn>>;
  auto fut = std::make_shared<fut_t>();
  auto o = std::make_shared<op>(*this, user_data, get_stack(),
                                [fx = std::forward<Fn>(f)]() {
                                  std::exception_ptr ex;
                                  try {
                                    fut->set(fx());
                                    return;
                                  } catch (...) {
                                    ex = std::current_exception();
                                  }
                                  fut->set(ex);
                                });

  return fut;
}

}  // namespace hctx
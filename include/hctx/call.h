#pragma once

#include <type_traits>

#include "hctx/op.h"
#include "hctx/scheduler.h"

namespace hctx {

template <typename Fn>
auto call(Fn&& fn) -> future_ptr<std::invoke_result_t<Fn>> {
  assert(this_op != nullptr);
  return this_op->sched_.post_high_prio(std::forward<Fn>(fn));
}

}  // namespace hctx
#include "hctx/cv.h"

#include "hctx/op.h"
#include "hctx/scheduler.h"

namespace hctx {

cv::cv() : caller_{get_caller()} {}

std::weak_ptr<op> cv::get_caller() {
  if (this_op == nullptr) {
    return std::weak_ptr<op>();
  } else {
    return this_op->shared_from_this();
  }
}

void cv::wait() {
  auto caller_lock = caller_.lock();
  assert(caller_lock.get() != nullptr);
  caller_lock->suspend(/* finished = */ false);
}

void cv::notify() {
  auto caller_lock = caller_.lock();
  if (caller_lock) {
    caller_lock->sched_.enqueue_high_prio(caller_lock);
  }
}

}  // namespace hctx
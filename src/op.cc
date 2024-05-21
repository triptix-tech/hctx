#include "hctx/op.h"

#include <iostream>

#include "hctx/scheduler.h"

namespace hctx {

CTX_ATTRIBUTE_TLS op* this_op = nullptr;

op::~op() {
  if (stack_ != nullptr) {
    sched_.stack_mem_.push(to_allocation(stack_));
    stack_ = nullptr;
  }
}

void op::resume() {
  // =============
  // STATE CHANGE
  // -------------
  while (true) {
    auto s = state_.load();
    switch (s) {
      case kActive:
        if (state_.compare_exchange_weak(s, kActiveReschedule)) {
          // Already running -> reschedule.
          return;
        } else {
          // State changed between load and now, try again.
          continue;
        }

      case kInactive:
        if (state_.compare_exchange_weak(s, kActive)) {
          // Inactive and we are the ones who set it to active.
          // -> let's run it!
          goto run;
        } else {
          // State changed between load and now, try again.
          continue;
        }

      case kFinished:
        // Finished - nothing to do.
        [[fallthrough]];

      case kActiveReschedule:
        // Active and already rescheduling - nothing to do.
        return;
    }
  }

  // ==========
  // EXECUTION
  // ----------

run:
  this_op = this;
  enter_op_start_switch();
  auto const t = bc::jump_fcontext(op_ctx_, this);
  exit_op_finish_switch();

  op_ctx_ = t.fctx;

  auto const finished = t.data == nullptr;
  if (finished) {
    // Finished = not interested in rescheduling.
    state_.store(kFinished);
    return;
  }

  // ============
  // STATE CHANGE
  // ------------
  while (true) {
    auto s = state_.load();
    switch (s) {
      case kActiveReschedule:
        if (state_.compare_exchange_weak(s, kInactive)) {
          // We're the ones who set it from kActiveReschedule to kInactive.
          // -> go equeue!
          sched_.enqueue_high_prio(this->shared_from_this());
          return;
        } else {
          // State changed. Check again.
          continue;
        }

      case kActive:
        if (state_.compare_exchange_weak(s, kInactive)) {
          // We're the ones who set it from kActive to kInactive. Done.
          return;
        } else {
          // State changed. Check again.
          continue;
        }

      case kInactive: [[fallthrough]];
      case kFinished: assert(false); return;
    }
  }
}

void op::suspend(bool const finished) {
  auto const self = this->shared_from_this();
  exit_op_start_switch();
  auto const t =
      bc::jump_fcontext(main_ctx_, reinterpret_cast<void*>(finished ? 0 : 1));
  enter_op_finish_switch();
  main_ctx_ = t.fctx;
}

void op::start() {
  (*fn_)();
  suspend(true);
}

void execute(bc::transfer_t const t) {
  auto const o = reinterpret_cast<op*>(t.data);
  o->enter_op_finish_switch();
  o->main_ctx_ = t.fctx;
  o->start();
}

#ifdef CTX_ENABLE_ASAN
void op::enter_op_start_switch() {
  __sanitizer_start_switch_fiber(&fake_stack_, to_allocation(stack_),
                                 kStackSize);
}

void op::enter_op_finish_switch() {
  __sanitizer_finish_switch_fiber(fake_stack_,
                                  &bottom_old_,  // NOLINT
                                  &size_old_);
}

void op::exit_op_start_switch() {
  __sanitizer_start_switch_fiber(&fake_stack_, bottom_old_, size_old_);
}

void op::exit_op_finish_switch() {
  __sanitizer_finish_switch_fiber(fake_stack_, nullptr, nullptr);
}
#else
void op::enter_op_start_switch() {}
void op::enter_op_finish_switch() {}
void op::exit_op_start_switch() {}
void op::exit_op_finish_switch() {}
#endif

}  // namespace hctx
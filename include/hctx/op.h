#pragma once

#include <cassert>
#include <atomic>
#include <memory>

#include "boost/context/detail/fcontext.hpp"

#include "hctx/fn.h"
#include "hctx/stack.h"
#include "hctx/thread_local.h"

#ifdef CTX_ENABLE_ASAN
extern "C" {
void __sanitizer_start_switch_fiber(void** fake_stack_save,
                                    const void* bottom,
                                    size_t size);
void __sanitizer_finish_switch_fiber(void* fake_stack_save,
                                     const void** bottom_old,
                                     size_t* size_old);
}
#endif

namespace hctx {

namespace bc {
using boost::context::detail::fcontext_t;
using boost::context::detail::jump_fcontext;
using boost::context::detail::make_fcontext;
using boost::context::detail::transfer_t;
}  // namespace bc

void execute(bc::transfer_t);

struct op;
struct scheduler;

extern CTX_ATTRIBUTE_TLS op* this_op;

struct op : public std::enable_shared_from_this<op> {
  enum state : std::uint8_t {
    kInactive,
    kActive,
    kActiveReschedule,
    kFinished
  };

  template <typename Fn>
  op(scheduler& sched, void* user_data, void* stack, Fn&& fn)
      :
#ifdef CTX_ENABLE_ASAN
        fake_stack_(nullptr),
        bottom_old_(nullptr),
        size_old_(0),
#endif
        user_data_{user_data},
        stack_{stack},
        op_ctx_{bc::make_fcontext(stack, kStackSize, execute)},
        sched_{sched},
        fn_{make_fn(std::forward<Fn>(fn))},
        state_{kInactive} {
  }

  ~op();

  void enter_op_start_switch();
  void enter_op_finish_switch();
  void exit_op_start_switch();
  void exit_op_finish_switch();

  void resume();

  void suspend(bool const finished);

  void start();

#ifdef CTX_ENABLE_ASAN
  void* fake_stack_;
  void const* bottom_old_;
  size_t size_old_;
#endif

  void* user_data_;
  void* stack_;
  bc::fcontext_t op_ctx_;
  bc::fcontext_t main_ctx_;

  scheduler& sched_;
  std::unique_ptr<fn> fn_;

  std::atomic_uint8_t state_;
  std::atomic_flag no_reschedule_{true};
};

}  // namespace hctx

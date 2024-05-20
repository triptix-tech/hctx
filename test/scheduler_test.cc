#include "gtest/gtest.h"

#include "hctx/scheduler.h"
#include "hctx/scheduler_impl.h"

TEST(hctx, scheduler) {
  auto f = hctx::free_list{};
  auto s = hctx::scheduler{f};

  auto x = 0U;
  s.post_high_prio([&]() { x = 1U; });

  auto op = hctx::op_ptr{};
  while (s.high_prio_q_.try_dequeue(op)) {
    op->resume();
  }

  EXPECT_EQ(1U, x);
}
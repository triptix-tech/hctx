#include "gtest/gtest.h"

#include "hctx/call.h"
#include "hctx/scheduler.h"
#include "hctx/scheduler_impl.h"

TEST(hctx, scheduler) {
  auto f = hctx::free_list{100U};
  auto s = hctx::scheduler{f};

  auto x = 0U;
  s.post_low_prio([&]() {
    x = 1U;

    auto const a = hctx::call([&]() { return 7; });
    auto const b = hctx::call([&]() { return 11; });

    hctx::call([&]() {
      x = 2U;
      hctx::call([&]() { x = 3U; })->val();
    })->val();

    EXPECT_EQ(18, a->val() + b->val());
  });

  s.run();

  std::cerr << "FIN\n";

  EXPECT_EQ(3U, x);
}
#include "gtest/gtest.h"

#include "hctx/call.h"
#include "hctx/scheduler_impl.h"

unsigned iterfib(unsigned const n) {
  if (n == 0) {
    return 0;
  }
  if (n == 1) {
    return 1;
  }

  auto i = 0U;
  auto j = 1U;

  for (auto c = 0U; c < n - 1U; ++c) {
    auto tmp = j;
    j = i + j;
    i = tmp;
  }
  return j;
}

unsigned recfib_sync(unsigned const i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }
  return recfib_sync(i - 1) + recfib_sync(i - 2);
}

unsigned recfib_async(unsigned const i) {
  if (i < 20) {
    return recfib_sync(i);
  }

  auto res_1 = hctx::call([i]() { return recfib_async(i - 1); });
  auto res_2 = hctx::call([i]() { return recfib_async(i - 2); });
  return res_1->val() + res_2->val();
}

TEST(hctx, fib) {
  constexpr auto kCount = 40;

  auto expected = std::vector<unsigned>{};
  for (auto i = 0u; i < kCount; ++i) {
    expected.push_back(iterfib(i));
  }

  auto f = hctx::free_list{1024U};
  auto s = hctx::scheduler{f};

  auto count = std::atomic_uint32_t{0U};
  for (auto i = 0U; i < kCount; ++i) {
    s.post_low_prio([i, &expected, &count]() {
      auto const actual = hctx::call([i]() { return recfib_async(i); })->val();
      EXPECT_EQ(expected[i], actual);
      ++count;
    });
  }

  s.run_parallel();

  EXPECT_EQ(kCount, count);
}
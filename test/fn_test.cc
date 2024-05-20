#include "gtest/gtest.h"

#include "hctx/fn.h"

TEST(dir, file_contents) {
  auto x = 0U;
  auto const f = hctx::make_fn([&]() { x = 1U; });
  (*f)();
  EXPECT_EQ(1, x);
}
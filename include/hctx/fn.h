#pragma once

#include <memory>
#include <utility>

namespace hctx {

struct fn {
  virtual ~fn();
  virtual void operator()() = 0;
};

template <typename Fn>
struct fn_impl final : public fn {
  fn_impl(Fn&& f) : fn_{std::forward<Fn>(f)} {}
  ~fn_impl() override = default;

  void operator()() override { fn_(); }

  std::decay_t<Fn> fn_;
};

template <typename Fn>
std::unique_ptr<fn> make_fn(Fn&& f) {
  return std::unique_ptr<fn>{new fn_impl<Fn>{std::forward<Fn>(f)}};
}

}  // namespace hctx
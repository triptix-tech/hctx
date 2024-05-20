#pragma once

#include <exception>
#include <memory>
#include <type_traits>
#include <variant>

#include "hctx/condition_variable.h"

namespace hctx {

template <typename T>
struct future {
  T& val() {
    if (!result_available_) {
      cv_.wait([&]() { return result_available_.load(); });
    }
    if (std::holds_alternative<std::exception_ptr>(result_)) {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    }
    return result_;
  }

  void set(T&& result) {
    result_ = std::move(result);
    result_available_.store(true);
    cv_.notify();
  }

  void set(std::exception_ptr e) {
    result_ = e;
    result_available_.store(true);
    cv_.notify();
  }

  std::variant<T, std::exception_ptr> result_;
  condition_variable cv_;
  std::atomic_bool result_available_;
};

template <typename T>
using future_ptr = std::shared_ptr<future<T>>;

}  // namespace hctx
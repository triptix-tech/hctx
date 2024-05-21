#pragma once

#include <exception>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

#include "hctx/cv.h"

namespace hctx {

template <typename T, typename Enable = void>
struct future {};

template <typename T>
struct future<T, std::enable_if_t<!std::is_same_v<T, void>>> {
  T& val() {
    if (!result_available_) {
      cv_.wait([&]() { return result_available_.load(); });
    }
    if (exception_) {
      std::rethrow_exception(exception_);
    }
    return result_;
  }

  void set(T&& result) {
    result_ = std::move(result);
    result_available_.store(true);
    cv_.notify();
  }

  void set(std::exception_ptr e) {
    exception_ = e;
    result_available_.store(true);
    cv_.notify();
  }

  T result_;
  std::exception_ptr exception_;
  cv cv_;
  std::atomic_bool result_available_{false};
};

template <typename T>
struct future<T, std::enable_if_t<std::is_same_v<T, void>>> {
  void val() {
    if (!result_available_) {
      cv_.wait([&]() { return result_available_.load(); });
    }
    if (exception_) {
      std::rethrow_exception(exception_);
    }
  }

  void set() {
    result_available_.store(true);
    cv_.notify();
  }

  void set(std::exception_ptr e) {
    exception_ = e;
    result_available_.store(true);
    cv_.notify();
  }

  std::exception_ptr exception_;
  cv cv_;
  std::atomic_bool result_available_{false};
};

template <typename T>
using future_ptr = std::shared_ptr<future<T>>;

template <typename T>
void await_all(std::vector<future_ptr<T>> const& futures) {
  auto exception = std::exception_ptr{};
  for (auto const& fut : futures) {
    try {
      fut->val();
    } catch (...) {
      if (!exception) {
        exception = std::current_exception();
      }
    }
  }
  if (exception) {
    std::rethrow_exception(exception);
  }
}

}  // namespace hctx
#pragma once

#include <boost/scope_exit.hpp>

#include <atomic>
#include <concepts>
#include <cstddef>

namespace vanilla::exchange {

class request_throttler {
  const size_t concurrency_{0};
  std::atomic<size_t> active_requests_count_{0};

public:
  explicit request_throttler(size_t concurrency) : concurrency_(concurrency) {}

  template <std::invocable Fh, std::invocable Fb>
  void operator()(Fh &&handler, Fb &&bouncer) noexcept(
      noexcept(handler()) && noexcept(bouncer())) {
    BOOST_SCOPE_EXIT_ALL(this) {
      active_requests_count_.fetch_add(-1, std::memory_order_acq_rel);
    };

    // if using the last available theread then throttle the request
    if (active_requests_count_.fetch_add(1, std::memory_order_acq_rel) ==
        concurrency_ - 1) {
      bouncer();
    } else {
      handler();
    }
  }
};

} // namespace vanilla::exchange
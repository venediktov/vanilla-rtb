#pragma once
#include "rtb/core/tagged_tuple.hpp"
#include <tuple>
#include <type_traits>
#include <utility>

namespace vanilla {
template <size_t N, typename... Fs>
  requires(N < sizeof...(Fs))
struct nth_function {
  using func_type = std::tuple_element_t<N, std::tuple<Fs...>>;

  template <typename... Args>
  using return_type = std::invoke_result<func_type, Args...>;
};

template <typename... Fs>
  requires(sizeof...(Fs) > 0)
struct terminal_function {
  using func_type = std::tuple_element_t<sizeof...(Fs) - 1, std::tuple<Fs...>>;

  template <typename... Args>
  using return_type = std::invoke_result_t<func_type, Args...>;
};
} // namespace vanilla

/*
 * File:   concepts.hpp
 * Author: VanillaRTB
 * Copyright (c) 2023-2033 Venediktes Gruppe, LLC
 *
 * Created on March 8, 2023, 5:27 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once
#ifndef VANILLA_COMMON_CONCEPTS_HPP
#define VANILLA_COMMON_CONCEPTS_HPP

#include <concepts>
#include <string_view>
#include <type_traits>

/**
************************
example of use
************************
template <string_like_concept... Sn>
requires(not char_array_concept<Sn, sizeof(Sn)> and ...)
constexpr auto string_concat(Sn const&... s)
**/

namespace vanilla::common {

template <class T>
concept string_like_concept = std::convertible_to<T, std::string_view>;

template <class T, std::size_t N>
concept char_array_concept = std::is_same_v<char[N], T>;

// BidRequest
template <typename T>
concept is_bid_request = requires(T value) {
    value.imp;
    typename T::request_type;
};

// BidResponse
template <typename T>
concept is_bid_response = requires(T value) {
    value.bidid;
    typename T::data_type;
};

template <typename T, typename... Args>
concept custom_bid_processor = requires(T processor, Args... args) {
    sizeof...(args) == 4;
    { processor.operator()(args...) } -> std::same_as<void>;
};

template <typename T>
concept loadable_cache_v = requires(T& cache) {
    { cache.load() } -> std::same_as<void>;
};

} // namespace vanilla::common

#endif /* VANILLA_COMMON_CONCEPTS_HPP */

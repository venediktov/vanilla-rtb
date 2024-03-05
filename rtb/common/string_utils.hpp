
/*
 * File:   string_utils.hpp
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
#ifndef VANILLA_COMMON_STRING_UTILS_HPP
#define VANILLA_COMMON_STRING_UTILS_HPP

#include "concepts.hpp"
#include <algorithm>
#include <string>

namespace vanilla {
namespace common {

template <string_like_concept... Sn>
    requires(not char_array_concept<Sn, sizeof(Sn)> and ...)
constexpr auto string_concat(Sn const&... s) {
    using value_type = typename std::common_type_t<Sn...>::value_type;
    std::basic_string<value_type> result;
    result.resize((std::size(s) + ...));
    std::size_t pos = 0;
    ((std::copy(s.begin(), s.end(), result.begin() + pos), pos += std::size(s)),...);
    return result;
}

} // namespace common
} // namespace vanilla

#endif /* VANILLA_COMMON_STRING_UTILS_HPP */

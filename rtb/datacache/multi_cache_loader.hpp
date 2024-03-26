/*
 * File:   multi_cache_loader.hpp
 * Author: Vladimir Lysyy
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on December 18, 2017, 1:42 PM
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
 */

#pragma once

#include "common/concepts.hpp"

namespace vanilla {

template <common::loadable_cache_v Cache> struct CacheLoader : private Cache {
    using Cache::Cache;
    using Cache::load;
    using Cache::retrieve;

    Cache& get() noexcept { return *this; }
};

template <common::loadable_cache_v... Caches> struct MultiCacheLoader : private CacheLoader<Caches>... {
    template <typename Config> explicit MultiCacheLoader(Config const& config) : CacheLoader<Caches>(config)... {}

    void load() { (CacheLoader<Caches>::load(), ...); }

    using CacheLoader<Caches>::retrieve...;

    template <common::loadable_cache_v Cache>
        requires(std::same_as<Cache, Caches> || ...)
    Cache& get() noexcept {
        return CacheLoader<Cache>::get();
    }
};

} // namespace vanilla
/*
 * File:   generic_bidder_cache_loader.hpp
 * Author: Vladimir Venediktov
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

#ifndef VANILLA_RTB_GENERIC_BIDDER_CACHE_LOADER_HPP
#define VANILLA_RTB_GENERIC_BIDDER_CACHE_LOADER_HPP

#include "rtb/core/core.hpp"
#include <type_traits>
#include <tuple>
#include <utility>

namespace vanilla {

    template<typename ...Entities>
    struct GenericBidderCacheLoader;

    template<typename Entity, typename ...Entities>
    struct GenericBidderCacheLoader<Entity, Entities ...> : GenericBidderCacheLoader<Entities ...> {
        using GenericBidderCacheLoader<Entities...>::retrieve;
        using GenericBidderCacheLoader<Entities...>::get;

        template<typename T, typename... Keys>
        using retrieve_type = decltype(std::declval<Entity>().retrieve(std::declval<T&>(), std::declval<Keys>()...)) ;

        template<typename Config>
        GenericBidderCacheLoader(const Config &config): GenericBidderCacheLoader<Entities...>(config), entity(config)
        {}
        void load() noexcept(false) {
           entity.load();
           GenericBidderCacheLoader<Entities...>::load();
        }

        template<typename T, typename... Keys>
        bool retrieve(T & t, Keys&& ... keys) {
            return  this->retrieve(t, std::make_tuple(std::forward<Keys>(keys)...),
                                      std::make_index_sequence<sizeof...(keys)>()
            );
        }

        template<typename T, typename Tuple, std::size_t... Idx>
        decltype(std::declval<Entity>().retrieve(std::declval<T&>(),std::get<Idx>(std::declval<Tuple>())...), bool())
        retrieve(T & t, Tuple&& tuple, std::index_sequence<Idx...>, Entity* = 0) {
            return  entity.template retrieve(t, std::get<Idx>(std::forward<Tuple>(tuple))...);
        }

        template<typename EntityT>
        Entity& get(typename std::enable_if<std::is_same<Entity, typename std::decay<EntityT>::type>::value>::type* = 0) {
            return entity;
        }

    private:
        Entity entity;
    };

    template<typename Entity>
    struct GenericBidderCacheLoader<Entity> {

        template<typename Config>
        GenericBidderCacheLoader(const Config &config): entity(config)
        {}
        void load() noexcept(false) {
            entity.load();
        }

        template<typename T, typename... Keys>
        bool retrieve(T & t, Keys&&... keys) {
            return  this->retrieve(t, std::make_tuple(std::forward<Keys>(keys)...),
                                      std::make_index_sequence<sizeof...(keys)>()
            );
        }

        template<typename T, typename Tuple, std::size_t... Idx>
        decltype(std::declval<Entity>().retrieve(std::declval<T&>(),std::get<Idx>(std::declval<Tuple>())...), bool())
        retrieve(T & t, Tuple&& tuple, std::index_sequence<Idx...>, Entity* = 0) {
            return  entity.template retrieve(t, std::get<Idx>(std::forward<Tuple>(tuple))...);
        }

        template<typename EntityT>
        Entity& get(typename std::enable_if<std::is_same<Entity, typename std::decay<EntityT>::type>::value>::type* = 0) {
            return entity;
        }

    private:
        Entity entity;
    };
}


#endif //VANILLA_RTB_GENERIC_BIDDER_CACHE_LOADER_HPP

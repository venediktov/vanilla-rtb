
/*  
 * File:   decision_tree.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on March 22, 2017, 5:27 PM
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

#ifndef VANILLA_COMMON_DECISION_TREE_HPP
#define VANILLA_COMMON_DECISION_TREE_HPP

#include <functional>
#include <array>

/**
 
auto action_1 = [](){ std::cout << "executing action_1\n"; return false; };
auto action_2 = [](){ std::cout << "executing action_2\n"; return false; };
auto action_3 = [](){ std::cout << "executing action_3\n"; return true;  };


decision_tree_manager<3> tm = decision_tree<3>
{{
    { 0 , { action_1, -1 , 1}  }, //true->EXIT , false->index=1
    { 1 , { action_2, -1 , 2}  }, //true->EXIT , false->index=2
    { 2 , { action_3, -1 , -1} }  //EXIT
}} ;

tm.execute();

 **/

namespace vanilla { namespace common { 

struct decision_action {
    using function_type  = std::function<bool()>;
    struct node_S {
        function_type f;
        int next_true;
        int next_false;
    };
    using node_type = node_S;
    enum CODES { EXIT=-1 , CONTINUE=0 };
    template<bool T> int get() const ;
    int operator()() const;

    uint32_t row_num;
    node_type node;
};

template<>
inline int decision_action::get<true>() const {
    return node.next_true;
}

template<>
inline int decision_action::get<false>() const {
    return node.next_false;
}

inline int decision_action::operator()() const {
    if (node.f()) {        
        return get<true>();
    } else {
        return get<false>();
    }
}


template<std::size_t N>
using decision_tree = std::array<decision_action,N>;

template<std::size_t N>
struct decision_tree_manager {
    decision_tree_manager(decision_tree<N> && tree) : tree{std::move(tree)} 
    {}
    decision_tree_manager(const decision_tree<N> & tree) : tree{tree} 
    {}
    
    void execute() {
        next(tree.at(0));
    }
    decision_action next(const decision_action &action_now) {
        int next_node_idx = action_now(); //executes and produces action_next
        if ( next_node_idx == decision_action::CONTINUE ) {
            next(tree.at(action_now.row_num+1));
        }
        if ( next_node_idx != decision_action::EXIT ) {
            next(tree.at(next_node_idx));
        }
    }
private:
    decision_tree<N> tree;
};

}}

#endif /* VANILLA_COMMON_DECISION_TREE_HPP */


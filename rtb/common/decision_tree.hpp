
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

namespace vanilla {
    namespace common {
        template <typename PARAMS>
        struct decision_action {
            using function_type = std::function<bool(const PARAMS &params)>;

            struct node_S {
                function_type f;
                int next_true;
                int next_false;
            };
            
            using node_type = node_S;

            enum CODES {
                EXIT = -1, CONTINUE = 0
            };
            int get(bool res) const {
                return res ? node.next_true : node.next_false;
            }

            inline int operator()(const PARAMS &params) const {
                return get(node.f(params));
            }

            uint32_t row_num{0};
            node_type node;
        };


        template<typename PARAMS, std::size_t N>
        using decision_tree = std::array<decision_action<PARAMS>, N>;

        template<typename PARAMS, std::size_t N>
        struct decision_tree_manager {
            using params_decision_action = decision_action<PARAMS>;
            using decision_tree_type = decision_tree<PARAMS, N>;
            decision_tree_manager(decision_tree_type && tree) : tree{std::move(tree)}
            {
            }
            decision_tree_manager(const decision_tree_type & tree) : tree{tree}
            {
            }
            void execute(const PARAMS &params) const noexcept(false) {
                next(params, tree.at(0));
            }

            void next(const PARAMS &params, const params_decision_action &action_now) const noexcept(false) {
                int next_node_idx = action_now(params); //executes and produces action_next
                if (next_node_idx == params_decision_action::CONTINUE) {
                    next(params, tree.at(action_now.row_num + 1));
                }
                else if (next_node_idx != params_decision_action::EXIT) {
                    next(params, tree.at(next_node_idx));
                }
            }
        private:
            decision_tree_type tree;
        };

    }
}

#endif /* VANILLA_COMMON_DECISION_TREE_HPP */


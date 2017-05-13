/* 
 * File:   decision_exchange.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 9 мая 2017 г., 17:12
 */

#ifndef DECISION_EXCHANGE_HPP
#define DECISION_EXCHANGE_HPP

#include <rtb/common/decision_tree.hpp>

namespace vanilla {
    namespace decision_exchange {
        
        template <typename Params>
        struct default_traits {
            enum CODES {USER_PROFILE, AUCTION_ASYNC, COUNT};
            using decision_params = Params;
            using decision_action = vanilla::common::decision_action<decision_params>;
            using decision_manager = vanilla::common::decision_tree_manager<decision_params, COUNT>;
        };
        
        template <typename Traits>
        class decision_exchange {
            using decision_tree_type = typename Traits::decision_manager::decision_tree_type;
            using decision_params = typename Traits::decision_params;
            using decision_manager = typename Traits::decision_manager;
        public:
            decision_exchange(const decision_tree_type &decision_tree):
                decision_tree{decision_tree}, manager{this->decision_tree}
            {}
            void exchange(const decision_params &params) {
                //decision_manager manager(decision_tree);
                manager.execute(params);
            }
        private:
            decision_tree_type decision_tree;
            decision_manager manager;
            
        };
    }
}

#endif /* DECISION_EXCHANGE_HPP */


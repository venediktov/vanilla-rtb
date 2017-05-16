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
  
        enum class DEFAULT_CODES {AUCTION_ASYNC, COUNT};
        
        template <typename Codes = DEFAULT_CODES, typename ...Args>
        class decision_exchange {
            static constexpr int tree_depth{static_cast<int>(Codes::COUNT)};
        public:
            using decision_manager = vanilla::common::decision_tree_manager<tree_depth,Args...>;
            using decision_tree_type = typename decision_manager::decision_tree_type;
            using decision_action = vanilla::common::decision_action<Args...>;
            
            template <typename T>
            decision_exchange(T &&decision_tree):
                decision_tree{decision_tree}, manager{this->decision_tree}
            {}  
            template<typename ...TArgs>
            void exchange(TArgs && ...args) {
                //decision_manager manager(decision_tree);
                manager.execute(std::forward<TArgs>(args)...);
            }
        private:
            decision_tree_type decision_tree;
            decision_manager manager;
            
        };
    }
}

#endif /* DECISION_EXCHANGE_HPP */


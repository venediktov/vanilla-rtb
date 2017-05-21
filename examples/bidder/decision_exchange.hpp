/* 
 * File:   decision_router.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 9 мая 2017 г., 17:12
 */

#ifndef DECISION_ROUTER_HPP
#define DECISION_ROUTER_HPP

#include <rtb/common/decision_tree.hpp>

namespace vanilla {
    namespace decision {
        template <unsigned int SIZE, typename ...Args>
        class router {
            static constexpr int tree_depth{static_cast<int>(SIZE)};
            using decision_manager = vanilla::common::decision_tree_manager<tree_depth,Args...>;
        public:           
            using decision_tree_type = typename decision_manager::decision_tree_type;
            using decision_action = vanilla::common::decision_action<Args...>;
            
            template <typename T>
            router(T &&decision_tree):
                decision_tree{decision_tree}, manager{this->decision_tree}
            {}  
            template<typename ...TArgs>
            void execute(TArgs && ...args) {
                manager.execute(std::forward<TArgs>(args)...);
            }
        private:
            decision_tree_type decision_tree;
            decision_manager manager;
            
        };
    }
}

#endif /* DECISION_ROUTER_HPP */


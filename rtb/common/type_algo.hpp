
#pragma once
#include "rtb/core/tagged_tuple.hpp"
#include <type_traits>
#include <utility>

namespace vanilla {

    template<std::size_t n, typename ...Funcs>
    struct nth_function {
        using func_type = vanilla::nth_element<n, Funcs...>;
        template<typename ...Args>
        using return_type = typename std::result_of<func_type(Args...)>;
    };


    template<typename ...Func>
    struct terminal_function ;

    template<typename Func, typename ...Funcs>
    struct terminal_function<Func,Funcs...> {
        template<typename ...Args>
        using current_return_type =  typename std::result_of<Func(Args...)>::type;
        template<typename Arg, typename ...Args>
        struct func_ {
            using type = typename terminal_function<Funcs...>::template func_<current_return_type<Arg,Args...>,Args...>::type ;
        };
        template<typename ...Args>
        using return_type = typename std::result_of<typename func_<Args...>::type>::type;
    };

    template<typename Func>
    struct terminal_function<Func> {
        template<typename ...Args>
        struct func_ {
            using type = Func(Args...);
        };
        template<typename ...Args>
        using return_type = typename std::result_of<typename func_<Args...>::type>::type;
    };

}
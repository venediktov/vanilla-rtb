#pragma once

/*
   Copyright 2017 Vladimir Lysyy (mrbald@github)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <boost/interprocess/containers/string.hpp>
#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    template<typename CharT, typename CharTraits>
    using basic_string_view = boost::basic_string_ref<CharT,CharTraits>;
}
#else
#include <boost/utility/string_view.hpp>
#endif

#include <string>

namespace ufw {

    /**
     * Functor for containers transparent across `std::string`,
     * `boost::container::string`, `boost::string_view`, and `char const*`.
     */
    template<typename Alloc, typename Op>
    struct any_str_op {
        template <typename X, typename Y>
        bool operator () (X&& l, Y&& r) const { return op_(view(l), view(r)); }

    private:
        Op op_;

        using char_t = char;
        using char_traits_t = std::char_traits<char_t>;
        using shm_str_t = typename boost::container::basic_string<char_t, char_traits_t, Alloc>;
        using std_str_t = std::basic_string<char_t, char_traits_t, std::allocator<char_t>>;
        using str_view_t = boost::basic_string_view<char_t, char_traits_t>;

        static str_view_t view(char_t const* x) { return {x}; }
        static str_view_t view(shm_str_t const& x) { return {x.data(), x.size()}; }
        static str_view_t view(std_str_t const& x) { return {x}; }
        static str_view_t view(str_view_t x) { return x; }
    };

    template <typename Alloc>
    using any_str_less = any_str_op<Alloc, std::less<void>>;

} // namespace ufw

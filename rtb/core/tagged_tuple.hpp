#ifndef __VANILLA_TAGGED_TUPPLE_HPP__
#define __VANILLA_TAGGED_TUPPLE_HPP__


#include <tuple>
#include <iostream>

//http://stackoverflow.com/questions/13065166/c11-tagged-tuple
//http://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
namespace vanilla {

template<typename... Ts> struct typelist {
  template<typename T> using prepend = typelist<T, Ts...>;
};

template<typename T, typename... Ts> struct index;
template<typename T, typename... Ts> struct index<T, T, Ts...>:
  std::integral_constant<int, 0> {};
template<typename T, typename U, typename... Ts> struct index<T, U, Ts...>:
  std::integral_constant<int, index<T, Ts...>::value + 1> {};

//NTH_ELEMENT_IMPL
template<int n, typename... Ts> struct nth_element_impl;
template<typename T, typename... Ts> 
struct nth_element_impl<0, T, Ts...> { using type = T; };
template<int n, typename T, typename... Ts> struct nth_element_impl<n, T, Ts...> {
  using type = typename nth_element_impl<n - 1, Ts...>::type; 
};
//NTH_ELEMENT
template<int n, typename... Ts> 
using nth_element = typename nth_element_impl<n, Ts...>::type;

//EXTRACT_IMPL
template<int n, int m, typename... Ts> struct extract_impl;

template<int n, int m, typename T, typename... Ts>
struct extract_impl<n, m, T, Ts...> : extract_impl<n, m - 1, Ts...> {};

template<int n, typename T, typename... Ts>
struct extract_impl<n, 0, T, Ts...> { using types = typename extract_impl<n, n - 1, Ts...>::types::template prepend<T>; };

template<int n, int m> 
struct extract_impl<n, m> { using types = typelist<>; };

//EXTRACT
template<int n, int m, typename... Ts> 
using extract = typename extract_impl<n, m, Ts...>::types;

//TAGGED_TUPLE_IMPL
template<typename S, typename T> struct tagged_tuple_impl;
template<typename... Ss, typename... Ts>
struct tagged_tuple_impl<typelist<Ss...>, typelist<Ts...>>: public std::tuple<Ts...> 
{
  template<typename... Args> 
  tagged_tuple_impl(Args &&...args): std::tuple<Ts...>(std::forward<Args>(args)...) {}
  template<typename S> nth_element<index<S, Ss...>::value, Ts...> get() 
  {
    return std::get<index<S, Ss...>::value>(*this); 
  }
};

//TAGGED_TUPLE
template<typename... Ts> 
struct tagged_tuple: tagged_tuple_impl<extract<2, 0, Ts...>, extract<2, 1, Ts...>> 
{
  using tagged_tuple_base = tagged_tuple_impl<extract<2, 0, Ts...>, extract<2, 1, Ts...>> ;
  template<typename... Args> 
  tagged_tuple(Args &&...args): tagged_tuple_base(std::forward<Args>(args)...) {}
};

} //namespace

#endif /*  __VANILLA_TAGGED_TUPPLE_HPP__ */

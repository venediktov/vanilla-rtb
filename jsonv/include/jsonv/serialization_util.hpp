/** \file jsonv/serialization_util.hpp
 *  Helper types and functions for serialization. These are usually not needed unless you are writing your own
 *  \c extractor or \c serializer.
 *  
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_SERIALIZATION_UTIL_HPP_INCLUDED__
#define __JSONV_SERIALIZATION_UTIL_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/functional.hpp>
#include <jsonv/serialization.hpp>

#include <initializer_list>
#include <functional>
#include <map>
#include <type_traits>

namespace jsonv
{

/** \addtogroup Serialization
 *  \{
**/

template <typename T>
class extractor_construction :
        public extractor
{
public:
    virtual const std::type_info& get_type() const override
    {
        return typeid(T);
    }
    
    virtual void extract(const extraction_context& context,
                         const value&              from,
                         void*                     into
                        ) const override
    {
        return extract_impl<T>(context, from, into);
    }
    
protected:
    template <typename U>
    auto extract_impl(const extraction_context& context,
                      const value&              from,
                      void*                     into
                     ) const
            -> decltype(U(from, context), void())
    {
        new(into) U(from, context);
    }
    
    template <typename U, typename = void>
    auto extract_impl(const extraction_context&,
                      const value&              from,
                      void*                     into
                     ) const
            -> decltype(U(from), void())
    {
        new(into) U(from);
    }
};

template <typename T>
class extractor_for :
        public extractor
{
public:
    virtual const std::type_info& get_type() const override
    {
        return typeid(T);
    }
    
    virtual void extract(const extraction_context& context,
                         const value&              from,
                         void*                     into
                        ) const override
    {
        new(into) T(create(context, from));
    }
    
protected:
    virtual T create(const extraction_context& context, const value& from) const = 0;
};

template <typename T, typename FExtract>
class function_extractor :
        public extractor_for<T>
{
public:
    template <typename FUExtract>
    explicit function_extractor(FUExtract&& func) :
            _func(std::forward<FUExtract>(func))
    { }
    
protected:
    virtual T create(const extraction_context& context, const value& from) const override
    {
        return create_impl(_func, context, from);
    }
    
private:
    template <typename FUExtract>
    static auto create_impl(const FUExtract& func, const extraction_context& context, const value& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }
    
    template <typename FUExtract, typename = void>
    static auto create_impl(const FUExtract& func, const extraction_context&, const value& from)
            -> decltype(func(from))
    {
        return func(from);
    }
    
private:
    FExtract _func;
};

template <typename FExtract>
auto make_extractor(FExtract func)
    -> function_extractor<decltype(func(std::declval<const extraction_context&>(), std::declval<const value&>())),
                          FExtract
                         >
{
    return function_extractor<decltype(func(std::declval<const extraction_context&>(), std::declval<const value&>())),
                              FExtract
                             >
            (std::move(func));
}

template <typename FExtract, typename = void>
auto make_extractor(FExtract func)
    -> function_extractor<decltype(func(std::declval<const value&>())),
                          FExtract
                         >
{
    return function_extractor<decltype(func(std::declval<const value&>())), FExtract>
            (std::move(func));
}

template <typename T>
class serializer_for :
        public serializer
{
public:
    virtual const std::type_info& get_type() const override
    {
        return typeid(T);
    }
    
    virtual value to_json(const serialization_context& context,
                          const void*                  from
                         ) const override
    {
        return to_json(context, *static_cast<const T*>(from));
    }
    
protected:
    virtual value to_json(const serialization_context& context,
                          const T&                     from
                         ) const = 0;
};

template <typename T, typename FToJson>
class function_serializer :
        public serializer_for<T>
{
public:
    template <typename FUToJson>
    explicit function_serializer(FUToJson&& to_json_) :
            _to_json(std::forward<FUToJson>(to_json_))
    { }
    
protected:
    
    virtual value to_json(const serialization_context& context, const T& from) const override
    {
        return to_json_impl(_to_json, context, from);
    }
    
private:
    template <typename FUToJson>
    static auto to_json_impl(const FUToJson& func, const serialization_context& context, const T& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }
    
    template <typename FUToJson, typename = void>
    static auto to_json_impl(const FUToJson& func, const serialization_context&, const T& from)
            -> decltype(func(from))
    {
        return func(from);
    }
    
private:
    FToJson _to_json;
};

template <typename T, typename FToJson>
function_serializer<T, FToJson> make_serializer(FToJson to_json_)
{
    return function_serializer<T, FToJson>(std::move(to_json_));
}

template <typename T>
class adapter_for :
        public adapter
{
public:
    virtual const std::type_info& get_type() const override
    {
        return typeid(T);
    }
    
    virtual void extract(const extraction_context& context,
                         const value&              from,
                         void*                     into
                        ) const override
    {
        new(into) T(create(context, from));
    }
    
    virtual value to_json(const serialization_context& context,
                          const void*                  from
                         ) const override
    {
        return to_json(context, *static_cast<const T*>(from));
    }
    
protected:
    virtual T create(const extraction_context& context, const value& from) const = 0;
    
    virtual value to_json(const serialization_context& context, const T& from) const = 0;
};

template <typename T, typename FExtract, typename FToJson>
class function_adapter :
        public adapter_for<T>
{
public:
    template <typename FUExtract, typename FUToJson>
    explicit function_adapter(FUExtract&& extract_, FUToJson&& to_json_) :
            _extract(std::forward<FUExtract>(extract_)),
            _to_json(std::forward<FUToJson>(to_json_))
    { }
    
protected:
    virtual T create(const extraction_context& context, const value& from) const override
    {
        return create_impl(_extract, context, from);
    }
    
    virtual value to_json(const serialization_context& context, const T& from) const override
    {
        return to_json_impl(_to_json, context, from);
    }
    
private:
    template <typename FUExtract>
    static auto create_impl(const FUExtract& func, const extraction_context& context, const value& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }
    
    template <typename FUExtract, typename = void>
    static auto create_impl(const FUExtract& func, const extraction_context&, const value& from)
            -> decltype(func(from))
    {
        return func(from);
    }
    
    template <typename FUToJson>
    static auto to_json_impl(const FUToJson& func, const serialization_context& context, const T& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }
    
    template <typename FUToJson, typename = void>
    static auto to_json_impl(const FUToJson& func, const serialization_context&, const T& from)
            -> decltype(func(from))
    {
        return func(from);
    }
    
private:
    FExtract _extract;
    FToJson  _to_json;
};

template <typename FExtract, typename FToJson>
auto make_adapter(FExtract extract, FToJson to_json_)
    -> function_adapter<decltype(extract(std::declval<const extraction_context&>(), std::declval<const value&>())),
                        FExtract,
                        FToJson
                       >
{
    return function_adapter<decltype(extract(std::declval<const extraction_context&>(), std::declval<const value&>())),
                            FExtract,
                            FToJson
                           >
            (std::move(extract), std::move(to_json_));
}

template <typename FExtract, typename FToJson, typename = void>
auto make_adapter(FExtract extract, FToJson to_json_)
    -> function_adapter<decltype(extract(std::declval<const value&>())),
                        FExtract,
                        FToJson
                       >
{
    return function_adapter<decltype(extract(std::declval<const value&>())),
                            FExtract,
                            FToJson
                           >
            (std::move(extract), std::move(to_json_));
}

/** An adapter for container types. This is for convenience of creating an \c adapter for things like \c std::vector,
 *  \c std::set and such.
 *  
 *  \tparam TContainer is the container to create and encode. It must have a member type \c value_type, support
 *                     iteration and an \c insert operation.
**/
template <typename TContainer>
class container_adapter :
        public adapter_for<TContainer>
{
    using element_type = typename TContainer::value_type;
    
protected:
    virtual TContainer create(const extraction_context& context, const value& from) const override
    {
        using std::end;
        
        TContainer out;
        from.as_array(); // get nice error if input is not an array
        for (value::size_type idx = 0U; idx < from.size(); ++idx)
            out.insert(end(out), context.extract_sub<element_type>(from, idx));
        return out;
    }
    
    virtual value to_json(const serialization_context& context, const TContainer& from) const override
    {
        value out = array();
        for (const element_type& x : from)
            out.push_back(context.to_json(x));
        return out;
    }
};

/** An adapter for enumeration types. The most common use of this is to map \c enum values in C++ to string values in a
 *  JSON representation (and vice versa).
 *  
 *  \tparam TEnum The type to map. This is not restricted to C++ enumerations (types defined with the \c enum keyword),
 *                but any type you wish to restrict to a subset of values.
 *  \tparam FEnumComp <tt>bool (*)(TEnum, TEnum)</tt> -- a strict ordering for \c TEnum values.
 *  \tparam FValueComp <tt>bool (*)(value, value)</tt> -- a strict ordering for \c value objects. By default, this is a
 *                     case-sensitive comparison, but this can be replaced with anything you desire (for example, use
 *                     \c value_less_icase to ignore case in extracting from JSON).
 * 
 *  \see enum_adapter_icase
**/
template <typename TEnum,
          typename FEnumComp  = std::less<TEnum>,
          typename FValueComp = std::less<value>
         >
class enum_adapter :
        public adapter_for<TEnum>
{
public:
    /** Create an adapter with mapping values from the range <tt>[first, last)</tt>.
     *  
     *  \tparam TForwardIterator An iterator yielding the type <tt>std::pair<jsonv::value, TEnum>></tt>
    **/
    template <typename TForwardIterator>
    explicit enum_adapter(std::string enum_name, TForwardIterator first, TForwardIterator last) :
            _enum_name(std::move(enum_name))
    {
        for (auto iter = first; iter != last; ++iter)
        {
            _val_to_cpp.insert({ iter->second, iter->first });
            _cpp_to_val.insert(*iter);
        }
    }
    
    /** Create an adapter with the specified \a mapping values.
     *  
     *  \param enum_name A user-friendly name for this enumeration to be used in error messages.
     *  \param mapping A list of C++ types and values to use in \c to_json and \c extract. It is okay to have a C++
     *                 value with more than one JSON representation. In this case, the \e first JSON representation will
     *                 be used in \c to_json, but \e all JSON representations will be interpreted as the C++ value. It
     *                 is also okay to have the same JSON representation for multiple C++ values. In this case, the
     *                 \e first JSON representation provided for that value will be used in \c extract.
     *  
     *  \example
     *  \code
     *  enum_adapter<ring>("ring",
     *                     {
     *                       { ring::fire,  "fire"    },
     *                       { ring::wind,  "wind"    },
     *                       { ring::earth, "earth"   },
     *                       { ring::water, "water"   },
     *                       { ring::heart, "heart"   }, // "heart" is preferred for to_json
     *                       { ring::heart, "useless" }, // "useless" is interpreted as ring::heart in extract
     *                     }
     *                    );
     *  \endcode
    **/
    explicit enum_adapter(std::string enum_name, std::initializer_list<std::pair<TEnum, value>> mapping) :
            enum_adapter(std::move(enum_name), mapping.begin(), mapping.end())
    { }
    
protected:
    virtual TEnum create(const extraction_context& context, const value& from) const override
    {
        using std::end;
        
        auto iter = _val_to_cpp.find(from);
        if (iter != end(_val_to_cpp))
            return iter->second;
        else
            throw extraction_error(context,
                                   std::string("Invalid value for ") + _enum_name + ": " + to_string(from)
                                  );
    }
    
    virtual value to_json(const serialization_context&, const TEnum& from) const override
    {
        using std::end;
        
        auto iter = _cpp_to_val.find(from);
        if (iter != end(_cpp_to_val))
            return iter->second;
        else
            return null;
    }
    
private:
    std::string                        _enum_name;
    std::map<value, TEnum, FValueComp> _val_to_cpp;
    std::map<TEnum, value, FEnumComp>  _cpp_to_val;
};

/** An adapter for enumeration types which ignores the case when extracting from JSON.
 *  
 *  \see enum_adapter
**/
template <typename TEnum, typename FEnumComp = std::less<TEnum>>
using enum_adapter_icase = enum_adapter<TEnum, FEnumComp, value_less_icase>;

/** An adapter which can create polymorphic types. This allows you to parse JSON directly into a type heirarchy without
 *  some middle layer.
 *  
 *  @code
 *  [
 *    {
 *      "type": "worker",
 *      "name": "Adam"
 *    },
 *    {
 *      "type": "manager",
 *      "name": "Bob",
 *      "head": "Development"
 *    }
 *  ]
 *  @endcode
 *  
 *  \tparam TPointer Some pointer-like type (likely \c unique_ptr or \c shared_ptr) you wish to extract values into. It
 *                   must support \c operator*, an explicit conversion to \c bool, construction with a pointer to a
 *                   subtype of what it contains and default construction.
**/
template <typename TPointer>
class polymorphic_adapter :
        public adapter_for<TPointer>
{
public:
    using match_predicate = std::function<bool (const extraction_context&, const value&)>;
    
public:
    polymorphic_adapter() = default;
    
    /** Add a subtype which can be transformed into \c TPointer which will be called if the discriminator \a pred is
     *  matched.
     *  
     *  \see add_subtype_keyed
    **/
    template <typename T>
    void add_subtype(match_predicate pred)
    {
        _subtype_ctors.emplace_back(std::move(pred),
                                    [] (const extraction_context& context, const value& value)
                                    {
                                        return TPointer(new T(context.extract<T>(value)));
                                    }
                                   );
    }
    
    /** Add a subtype which can be transformed into \c TPointer which will be called if given a JSON \c value with
     *  \c kind::object which has a member with \a key and the provided \a expected_value.
     *  
     *  \see add_subtype
    **/
    template <typename T>
    void add_subtype_keyed(std::string key, value expected_value)
    {
        match_predicate op = [key, expected_value] (const extraction_context&, const value& value)
                             {
                                 if (!value.is_object())
                                     return false;
                                 auto iter = value.find(key);
                                 return iter != value.end_object()
                                     && iter->second == expected_value;
                             };
        return add_subtype<T>(op);
    }
    
    /** When extracting a C++ value, should \c kind::null in JSON automatically become a default-constructed \c TPointer
     *  (which is usually the \c null representation)?
    **/
    void check_null_input(bool on)
    {
        _check_null_input = on;
    }
    
    bool check_null_input() const
    {
        return _check_null_input;
    }
    
    /** When converting with \c to_json, should a \c null input translate into a \c kind::null? **/
    void check_null_output(bool on)
    {
        _check_null_output = on;
    }
    
    bool check_null_output() const
    {
        return _check_null_output;
    }
    
protected:
    virtual TPointer create(const extraction_context& context, const value& from) const override
    {
        using std::begin;
        using std::end;
        
        if (_check_null_input && from.is_null())
            return TPointer();
        
        auto iter = std::find_if(begin(_subtype_ctors), end(_subtype_ctors),
                                 [&] (const std::pair<match_predicate, create_function>& pair)
                                 {
                                     return pair.first(context, from);
                                 }
                                );
        if (iter != end(_subtype_ctors))
            return iter->second(context, from);
        else
            throw extraction_error(context,
                                   std::string("No discriminators matched JSON value: ") + to_string(from)
                                  );
    }
    
    virtual value to_json(const serialization_context& context, const TPointer& from) const override
    {
        if (_check_null_output && !from)
            return null;
        else
            return context.to_json(typeid(*from), static_cast<const void*>(&*from));
    }
    
private:
    using create_function = std::function<TPointer (const extraction_context&, const value&)>;
    
private:
    std::vector<std::pair<match_predicate, create_function>> _subtype_ctors;
    bool                                                     _check_null_input  = false;
    bool                                                     _check_null_output = false;
};

/** \} **/

}

#endif/*__JSONV_SERIALIZATION_UTIL_HPP_INCLUDED__*/

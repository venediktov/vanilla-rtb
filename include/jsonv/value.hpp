/** \file jsonv/value.hpp
 *  
 *  Copyright (c) 2012-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_VALUE_HPP_INCLUDED__
#define __JSONV_VALUE_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/string_view.hpp>
#include <jsonv/detail/basic_view.hpp>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace jsonv
{

class path;
class value;

namespace detail
{

class object_impl;
class array_impl;
class string_impl;

union value_storage
{
    object_impl* object;
    array_impl*  array;
    string_impl* string;
    int64_t      integer;
    double       decimal;
    bool         boolean;
    
    constexpr value_storage() :
            object(nullptr)
    { }
};

}

/** Describes the \e kind of data a \c value holds. See \c value for more information.
 *  
 *  \see http://json.org/
**/
enum class kind : unsigned char
{
    null,
    object,
    array,
    string,
    integer,
    decimal,
    boolean
};

/** Print out the name of the \c kind. **/
JSONV_PUBLIC std::ostream& operator<<(std::ostream&, const kind&);

/** Get the name of the \c kind. **/
JSONV_PUBLIC std::string to_string(const kind&);

/** Get a string representation of the given \c value. **/
JSONV_PUBLIC std::string to_string(const value&);

/** Thrown from various \c value methods when attempting to perform an operation which is not valid for the \c kind of
 *  value.
**/
class JSONV_PUBLIC kind_error :
        public std::logic_error
{
public:
    explicit kind_error(const std::string& description);
    
    virtual ~kind_error() noexcept;
};

/** Represents a single JSON value, which can be any one of a potential \c kind, each behaving slightly differently.
 *  Instances will vary their behavior based on their kind -- functions will throw a \c kind_error if the operation does
 *  not apply to the value's kind. For example, it does not make sense to call \c find on an \c integer.
 *  
 *   - \c kind::null
 *     You cannot do anything with this...it is just null.
 *   - \c kind::boolean
 *     These values can be \c true or \c false.
 *   - \c kind::integer
 *     A numeric value which can be added, subtracted and all the other things you would expect.
 *   - \c kind::decimal
 *     Floating-point values should be considered "more general" than integers -- you may request an integer value as a
 *     decimal, but you cannot request a decimal as an integer, even when doing so would not require rounding. The
 *     literal \c 20.0 will always have \c kind::decimal.
 *   - \c kind::string
 *     A UTF-8 encoded string which is mostly accessed through the \c std::string class. Some random functions work in
 *     the cases where it makes sense (for example: \c empty and \c size), but in general, string manipulation should be
 *     done after calling \c as_string.
 *   - \c kind::array
 *     An array behaves like a \c std::deque because it is ultimately backed by one. If you feel the documentation is
 *     lacking, read this: http://en.cppreference.com/w/cpp/container/deque.
 *   - \c kind::object
 *     An object behaves lake a \c std::map because it is ultimately backed by one. If you feel the documentation is
 *     lacking, read this: http://en.cppreference.com/w/cpp/container/map. This library follows the recommendation in
 *     RFC 7159 to not allow for duplicate keys because most other libraries can not deal with it. It would also make
 *     the AST significantly more painful.
 *  
 *  \see http://json.org/
 *  \see http://tools.ietf.org/html/rfc7159
**/
class JSONV_PUBLIC value
{
public:
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    
    /** \addtogroup Array
     *  \{
     *  The base type for iterating over array values.
    **/
    template <typename T, typename TArrayView>
    struct basic_array_iterator :
            public std::iterator<std::random_access_iterator_tag, T>
    {
    public:
        basic_array_iterator() :
                _owner(0),
                _index(0)
        { }
        
        basic_array_iterator(TArrayView* owner, size_type index) :
                _owner(owner),
                _index(index)
        { }
        
        template <typename U, typename UArrayView>
        basic_array_iterator(const basic_array_iterator<U, UArrayView>& source,
                             typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0
                            ) :
                _owner(source._owner),
                _index(source._index)
        { }
        
        basic_array_iterator& operator++()
        {
            ++_index;
            return *this;
        }
        
        basic_array_iterator operator++(int) const
        {
            basic_array_iterator clone = *this;
            ++clone;
            return clone;
        }
        
        basic_array_iterator& operator--()
        {
            --_index;
            return *this;
        }
        
        basic_array_iterator operator--(int) const
        {
            basic_array_iterator clone = *this;
            --clone;
            return clone;
        }
        
        template <typename U, typename UArrayView>
        bool operator==(const basic_array_iterator<U, UArrayView>& other) const
        {
            return _owner == other._owner && _index == other._index;
        }
        
        template <typename U, typename UArrayView>
        bool operator!=(const basic_array_iterator<U, UArrayView>& other) const
        {
            return !operator==(other);
        }
        
        T& operator*() const
        {
            return _owner->operator[](_index);
        }
        
        T* operator->() const
        {
            return &_owner->operator[](_index);
        }
        
        basic_array_iterator& operator+=(size_type n)
        {
            _index += n;
            return *this;
        }
        
        basic_array_iterator operator+(size_type n) const
        {
            basic_array_iterator clone = *this;
            clone += n;
            return clone;
        }
        
        basic_array_iterator& operator-=(size_type n)
        {
            _index -= n;
            return *this;
        }
        
        basic_array_iterator operator-(size_type n) const
        {
            basic_array_iterator clone = *this;
            clone -= n;
            return clone;
        }
        
        difference_type operator-(const basic_array_iterator& other) const
        {
            return difference_type(_index) - difference_type(other._index);
        }
        
        bool operator<(const basic_array_iterator& rhs) const
        {
            return _index < rhs._index;
        }
        
        bool operator<=(const basic_array_iterator& rhs) const
        {
            return _index <= rhs._index;
        }
        
        bool operator>(const basic_array_iterator& rhs) const
        {
            return _index > rhs._index;
        }
        
        bool operator>=(const basic_array_iterator& rhs) const
        {
            return _index >= rhs._index;
        }
        
        T& operator[](size_type n) const
        {
            return _owner->operator[](_index + n);
        }
    private:
        template <typename U, typename UArrayView>
        friend struct basic_array_iterator;
        
        friend class value;
        
    private:
        TArrayView* _owner;
        size_type   _index;
    };
    
    /** The \c array_iterator is applicable when \c kind is \c kind::array. It allows you to use algorithms as if
     *  a \c value was a normal sequence container.
    **/
    typedef basic_array_iterator<value, value>                       array_iterator;
    typedef basic_array_iterator<const value, const value>           const_array_iterator;
    
    /** If \c kind is \c kind::array, an \c array_view allows you to access a value as a sequence container. This is
     *  most useful for range-based for loops.
    **/
    typedef detail::basic_view<array_iterator, const_array_iterator>               array_view;
    typedef detail::basic_view<const_array_iterator>                               const_array_view;
    typedef detail::basic_owning_view<value, array_iterator, const_array_iterator> owning_array_view;
    
    /** \} **/
    
    /** \addtogroup Object
     *  \{
     *  The base iterator type for iterating over object types. It is a bidirectional iterator similar to a
     *  \c std::map<std::string, jsonv::value>.
    **/
    template <typename T, typename TIterator>
    struct basic_object_iterator :
            public std::iterator<std::bidirectional_iterator_tag, T>
    {
    public:
        basic_object_iterator() :
                _impl()
        { }
        
        basic_object_iterator(const basic_object_iterator& source) :
                _impl(source._impl)
        { }
        
        /** This allows assignment from an \c object_iterator to a \c const_object_iterator. **/
        template <typename U, typename UIterator>
        basic_object_iterator(const basic_object_iterator<U, UIterator>& source,
                              typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0
                             ) :
                _impl(source._impl)
        { }
        
        basic_object_iterator& operator=(const basic_object_iterator& source)
        {
            _impl = source._impl;
            return *this;
        }
        
        template <typename U, typename UIterator>
        typename std::enable_if<std::is_convertible<U*, T*>::value, basic_object_iterator&>::type
        operator=(const basic_object_iterator<U, UIterator>& source)
        {
            return operator=(basic_object_iterator(source));
        }
        
        basic_object_iterator& operator++()
        {
            increment();
            return *this;
        }
        
        basic_object_iterator operator++(int) const
        {
            basic_object_iterator clone(*this);
            clone.increment();
            return clone;
        }
        
        basic_object_iterator& operator--()
        {
            decrement();
            return *this;
        }
        
        basic_object_iterator operator--(int) const
        {
            basic_object_iterator clone(*this);
            clone.decrement();
            return clone;
        }
        
        template <typename U, typename UIterator>
        bool operator ==(const basic_object_iterator<U, UIterator>& other) const
        {
            return _impl == other._impl;
        }
        
        template <typename U, typename UIterator>
        bool operator !=(const basic_object_iterator<U, UIterator>& other) const
        {
            return _impl != other._impl;
        }
        
        T& operator *() const
        {
            return current();
        }
        
        T* operator ->() const
        {
            return &current();
        }
        
    private:
        friend class value;
        
        template <typename UIterator>
        explicit basic_object_iterator(const UIterator& iter) :
                _impl(iter)
        { }
        
        void increment()
        {
            ++_impl;
        }
        
        void decrement()
        {
            --_impl;
        }
        
        T& current() const
        {
            return *_impl;
        }
        
    private:
        TIterator _impl;
    };
    
    /** The type of value stored when \c kind is \c kind::object. **/
    typedef std::pair<const std::string, value>                        object_value_type;
    
    /** The \c object_iterator is applicable when \c kind is \c kind::object. It allows you to use algorithms as if
     *  a \c value was a normal associative container.
    **/
    typedef basic_object_iterator<object_value_type,       std::map<std::string, value>::iterator>       object_iterator;
    typedef basic_object_iterator<const object_value_type, std::map<std::string, value>::const_iterator> const_object_iterator;
    
    /** If \c kind is \c kind::object, an \c object_view allows you to access a value as an associative container.
     *  This is most useful for range-based for loops.
    **/
    typedef detail::basic_view<object_iterator, const_object_iterator>               object_view;
    typedef detail::basic_view<const_object_iterator>                                const_object_view;
    typedef detail::basic_owning_view<value, object_iterator, const_object_iterator> owning_object_view;
    
    /** \} **/
    
public:
    /** Default-construct this to null. **/
    constexpr value() :
            _kind(jsonv::kind::null)
    { }
    
    /** The nullptr overload will fail to compile -- use \c null if you want a \c kind::null. **/
    value(std::nullptr_t) = delete;
    
    /** Copy the contents of \a source into a new instance. **/
    value(const value& source);
    
    /** Create a \c kind::string with the given \a value. **/
    value(const std::string& value);
    
    /** Create a \c kind::string with the given \a value.
     *  
     *  \param value The value to create with. This must be null-terminated.
    **/
    value(const char* value);
    
    /** Create a \c kind::string with the given \a value. Keep in mind that it will be converted to and stored as a
     *  UTF-8 encoded string.
    **/
    value(const std::wstring& value);
    
    /** Create a \c kind::string with the given \a value. Keep in mind that it will be converted to and stored as a
     *  UTF-8 encoded string.
     *  
     *  \param value The value to create with. This must be null-terminated.
    **/
    value(const wchar_t* value);
    
    /** Create a \c kind::integer with the given \a value. **/
    value(int64_t value);
    
    /** Create a \c kind::decimal with the given \a value. **/
    value(double value);
    
    /** Create a \c kind::decimal with the given \a value. **/
    value(float value);
    
    /** Create a \c kind::boolean with the given \a value. **/
    value(bool value);
    
    #define JSONV_VALUE_INTEGER_ALTERNATIVE_CTOR_PROTO_GENERATOR(type_)              \
        value(type_ val);
    JSONV_INTEGER_ALTERNATES_LIST(JSONV_VALUE_INTEGER_ALTERNATIVE_CTOR_PROTO_GENERATOR)
    
    /** Destruction will never throw. **/
    ~value() noexcept;
    
    /** Copy-assigns \c source to this.
     *  
     *  If an exception is thrown during the copy, it is propagated out. This instance will remain unchanged.
    **/
    value& operator=(const value& source);
    
    /** Move-construct this instance, leaving \a source as a null value. **/
    value(value&& source) noexcept;
    
    /** Move-assigns \c source to this, leaving \a source as a null value.
     *  
     *  Unlike a copy, this will never throw.
    **/
    value& operator=(value&& source) noexcept;
    
    /** \addtogroup Conversions
     *  These functions are used for accessing specific kinds of values.
     *  \{
    **/
    
    /** Get this value as a string.
     *  
     *  \throws kind_error if this value does not represent a string.
    **/
    const std::string& as_string() const;
    
    /** Tests if this \c kind is \c kind::string. **/
    bool is_string() const;
    
    /** Get this value as a wide string. Keep in mind that this is slower than \c as_string, as the internal storage is
     *  the \c char base \c std::string.
     *  
     *  \throws kind_error if this value does not represent a string.
    **/
    std::wstring as_wstring() const;
    
    /** Get this value as an integer.
     *  
     *  \throws kind_error if this value does not represent an integer.
    **/
    int64_t as_integer() const;
    
    /** Tests if this \c kind is \c kind::integer. **/
    bool is_integer() const;
    
    /** Get this value as a decimal. If the value's underlying kind is actually an integer type, cast the integer to a
     *  double before returning. This ignores the potential loss of precision.
     *  
     *  \throws kind_error if this value does not represent a decimal or integer.
    **/
    double as_decimal() const;
    
    /** Tests if this \c kind is \c kind::integer or \c kind::decimal. **/
    bool is_decimal() const;
    
    /** Get this value as a boolean.
     *  
     *  \throws kind_error if this value does not represent a boolean.
    **/
    bool as_boolean() const;
    
    /** Tests if this \c kind is \c kind::boolean. **/
    bool is_boolean() const;
    
    /** Tests if this \c kind is \c kind::array. **/
    bool is_array() const;
    
    /** Tests if this \c kind is \c kind::object. **/
    bool is_object() const;
    
    /** Tests if this \c kind is \c kind::null. **/
    bool is_null() const;
    
    /** \} **/
    
    /** \addtogroup Shared
     *  These functions are applicable to all kinds of values and have the same fundemental meaning for all kinds.
     *  \{
    **/
    
    /** Resets this value to null. **/
    void clear();
    
    /** Get this value's kind. **/
    inline jsonv::kind kind() const
    {
        return _kind;
    }
    
    /** Get the value specified by the path \a p.
     *  
     *  \throws std::out_of_range if any path along the chain did not exist.
     *  \throws kind_error if the path traversal is not valid for the value (for example: if the path specifies an array
     *                     index when the value is a string).
     *  \throws parse_error if a \c string_view was specified that did not have a valid specification (see
     *                      \c path::create).
    **/
    value&       at_path(const path& p);
    value&       at_path(string_view p);
    value&       at_path(size_type   p);
    const value& at_path(const path& p) const;
    const value& at_path(string_view p) const;
    const value& at_path(size_type   p) const;
    
    /** Similar to \c count, but walks the given path \a p to determine its presence.
     *  
     *  \returns \c 1 if the path finds an element; \c 0 if there is no such path in the tree.
     *  
     *  \throws parse_error if a \c string_view was specified that did not have a valid specification (see
     *                      \c path::create).
    **/
    size_type count_path(const path& p) const;
    size_type count_path(string_view p) const;
    size_type count_path(size_type   p) const;
    
    /** Get or create the value specified by the path \a p. This is the moral equivalent to \c operator[] for paths. If
     *  no value exists at the path, a new one is created as the default (\c null) value. If any path along the way
     *  either does not exist or is \c null, it is created for you, based on the \e implications of the specification
     *  \a p. Unlike \c at_path, which will throw if accessing a non-existent key of an \c object or going past the end
     *  of an \c array, this will simply create that path and fill in the blanks with \c null values.
     *  
     *  \throws kind_error if the path traversal is not valid for the value (for example: if the path specifies an array
     *                     index when the value is a string).
     *  \throws parse_error if a \c string_view was specified that did not have a valid specification (see
     *                      \c path::create).
     *  
     *  \see at_path
    **/
    value& path(const path& p);
    value& path(string_view p);
    value& path(size_type   p);
    
    /** Swap the value this instance represents with \a other. **/
    void swap(value& other) noexcept;
    
    /** Compares two JSON values for equality. Two JSON values are equal if and only if all of the following conditions
     *  apply:
     *  
     *   1. They have the same valid value for \c kind.
     *      - If \c kind is invalid (memory corruption), then two JSON values are \e not equal, even if they have been
     *        corrupted in the same way and even if they share \c this (a corrupt object is not equal to itself).
     *   2. The kind comparison is also equal:
     *      - Two null values are always equivalent.
     *      - string, integer, decimal and boolean follow the classic rules for their type.
     *      - objects are equal if they have the same keys and values corresponding with the same key are also equal.
     *      - arrays are equal if they have the same length and the values at each index are also equal.
     *  
     *  \note
     *  The rules for equality are based on Python \c dict and \c list.
    **/
    bool operator==(const value& other) const;
    
    /** Compares two JSON values for inequality. The rules for inequality are the exact opposite of equality.
    **/
    bool operator!=(const value& other) const;
    
    /** Used to build a strict-ordering of JSON values. When comparing values of the same kind, the ordering should
     *  align with your intuition. When comparing values of different kinds, some arbitrary rules were created based on
     *  how "complicated" the author thought the type to be.
     *  
     *   - null: less than everything but null, which it is equal to.
     *   - boolean: false is less than true.
     *   - integer, decimal: compared by their numeric value. Comparisons between two integers do not cast, but comparison
     *     between an integer and a decimal will coerce to decimal.
     *   - string: compared lexicographically by character code (with basic char strings and non-ASCII encoding, this
     *     might lead to surprising results)
     *   - array: compared lexicographically by elements (recursively following this same technique)
     *   - object: entries in the object are sorted and compared lexicographically, first by key then by value
     *  
     *  \returns -1 if this is less than other by the rules stated above; 0 if this is equal to other; -1 if otherwise.
    **/
    int compare(const value& other) const;
    
    bool operator< (const value& other) const;
    bool operator> (const value& other) const;
    bool operator<=(const value& other) const;
    bool operator>=(const value& other) const;
    
    /** Output this value to a stream. **/
    friend std::ostream& operator<<(std::ostream& stream, const value& val);
    
    /** Get a string representation of the given \c value. **/
    friend std::string to_string(const value&);
    
    /** \} **/
    
    /** \addtogroup Array
     *  These functions are only applicable if the kind of this value is an array.
     *  \{
    **/
    
    /** Get an iterator to the beginning of this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    array_iterator       begin_array();
    const_array_iterator begin_array() const;
    
    /** Get an iterator to the end of this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    array_iterator       end_array();
    const_array_iterator end_array() const;
    
    /** View this instance as an array.
     * 
     *  \throws kind_error if the kind is not an array.
    **/
    array_view        as_array() &;
    const_array_view  as_array() const &;
    owning_array_view as_array() &&;
    
    /** Get the value in this array at the given \a idx. The overloads which accept an \c int are required to resolve
     *  the type ambiguity of the literal \c 0 between a size_type and a char*.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    value& operator[](size_type idx);
    const value& operator[](size_type idx) const;
    inline value&       operator[](int idx)       { return operator[](size_type(idx)); }
    inline const value& operator[](int idx) const { return operator[](size_type(idx)); }
    
    /** Get the value in this array at the given \a idx.
     *  
     *  \throws kind_error if the kind is not an array.
     *  \throws std::out_of_range if the provided \a idx is above \c size.
    **/
    value& at(size_type idx);
    const value& at(size_type idx) const;
    
    /** Push \a item to the back of this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    void push_back(value item);
    
    /** Pop an item off the back of this array.
     *  
     *  \throws kind_error if the kind is not an array.
     *  \throws std::logic_error if the array is empty.
    **/
    void pop_back();
    
    /** Push \a item to the front of this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    void push_front(value item);
    
    /** Pop an item from the front of this array.
     *  
     *  \throws kind_error if the kind is not an array.
     *  \throws std::logic_error if the array is empty.
    **/
    void pop_front();
    
    /** Insert an item into \a position on this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    array_iterator insert(const_array_iterator position, value item);
    
    /** Insert the range defined by [\a first, \a last) at \a position in this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    template <typename TForwardIterator>
    array_iterator insert(const_array_iterator position, TForwardIterator first, TForwardIterator last)
    {
        difference_type orig_offset = std::distance(const_array_iterator(begin_array()), position);
        
        for (difference_type offset = orig_offset ; first != last; ++first, ++offset)
            insert(begin_array() + offset, *first);
        return begin_array() + orig_offset;
    }
    
    /** Assign \a count elements to this array with \a val.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    void assign(size_type count, const value& val);
    
    /** Assign the contents of range [\a first, \a last) to this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    template <typename TForwardIterator>
    void assign(TForwardIterator first, TForwardIterator last)
    {
        resize(std::distance(first, last), value());
        auto iter = begin_array();
        while (first != last)
        {
            *iter = *first;
            ++iter;
            ++first;
        }
    }
    
    /** Assign the given \a items to this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    void assign(std::initializer_list<value> items);
    
    /** Resize the length of this array to \a count items. If the resize creates new elements, fill those newly-created
     *  elements with \a val.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    void resize(size_type count, const value& val = value());
    
    /** Erase the item at this array's \a position.
     * 
     *  \throws kind_error if the kind is not an array.
    **/
    array_iterator erase(const_array_iterator position);
    
    /** Erase the range [\a first, \a last) from this array.
     *  
     *  \throws kind_error if the kind is not an array.
    **/
    array_iterator erase(const_array_iterator first, const_array_iterator last);
    
    /** \}
    **/
    
    /** \addtogroup Object
     *  These functions are only applicable if the kind of this value is an object.
     *  \{
    **/
    
    /** Get an iterator to the first key-value pair in this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_iterator       begin_object();
    const_object_iterator begin_object() const;
    
    /** Get an iterator to the one past the end of this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_iterator       end_object();
    const_object_iterator end_object() const;
    
    /** View this instance as an object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_view        as_object() &;
    const_object_view  as_object() const &;
    owning_object_view as_object() &&;
    
    /** Get the value associated with the given \a key of this object. If the \a key does not exist, it will be created.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    value& operator[](const std::string& key);
    value& operator[](std::string&& key);
    value& operator[](const std::wstring& key);
    
    /** Get the value associated with the given \a key of this object.
     *  
     *  \throws kind_error if the kind is not an object.
     *  \throws std::out_of_range if the \a key is not in this object.
    **/
    value& at(const std::string& key);
    value& at(const std::wstring& key);
    const value& at(const std::string& key) const;
    const value& at(const std::wstring& key) const;
    
    /** Check if the given \a key exists in this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    size_type count(const std::string& key) const;
    size_type count(const std::wstring& key) const;
    
    /** Attempt to locate a key-value pair with the provided \a key in this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_iterator       find(const std::string& key);
    object_iterator       find(const std::wstring& key);
    const_object_iterator find(const std::string& key)  const;
    const_object_iterator find(const std::wstring& key) const;
    
    /** Insert \a pair into this object. If \a hint is provided, this insertion could be optimized.
     *  
     *  \returns A pair whose \c first refers to the newly-inserted element (or the element which shares the key).
     *  \throws kind_error if the kind is not an object.
    **/
    std::pair<object_iterator, bool> insert(std::pair<std::string, value>  pair);
    std::pair<object_iterator, bool> insert(std::pair<std::wstring, value> pair);
    object_iterator insert(const_object_iterator hint, std::pair<std::string, value>  pair);
    object_iterator insert(const_object_iterator hint, std::pair<std::wstring, value> pair);
    
    /** Insert range defined by [\a first, \a last) into this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    template <typename TForwardIterator>
    void insert(TForwardIterator first, TForwardIterator last)
    {
        for ( ; first != last; ++first)
            insert(*first);
    }
    
    /** Insert \a items into this object.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    void insert(std::initializer_list<std::pair<std::string, value>>  items);
    void insert(std::initializer_list<std::pair<std::wstring, value>> items);
    
    /** Erase the item with the given \a key.
     *  
     *  \returns 1 if \a key was erased; 0 if it did not.
     *  \throws kind_error if the kind is not an object.
    **/
    size_type erase(const std::string&  key);
    size_type erase(const std::wstring& key);
    
    /** Erase the item at the given \a position.
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_iterator erase(const_object_iterator position);
    
    /** Erase the range defined by [\a first, \a last).
     *  
     *  \throws kind_error if the kind is not an object.
    **/
    object_iterator erase(const_object_iterator first, const_object_iterator last);
    
    /** \}
    **/
    
    /** \addtogroup Shared
     *  These functions are only applicable if the kind of this value is an array.
     *  \{
    **/
    
    /** Is the underlying structure empty? This has similar meaning for all types it works on and is always equivalent
     *  to asking if the size is 0.
     *  
     *   - object: Are there no keys?
     *   - array: Are there no values?
     *   - string: Is the string 0 length?
     *  
     *  \throws kind_error if the kind is not an object, array or string.
    **/
    bool empty() const;
    
    /** Get the number of items in this value.
     *  
     *   - object: The number of key/value pairs.
     *   - array: The number of values.
     *   - string: The number of code points in the string (including \c \\0 values and counting multi-byte encodings as
     *             more than one value).
     *  
     *  \throws kind_error if the kind is not an object, array or string.
    **/
    size_type size() const;
    
    /** \} **/
    
    /** \addtogroup Algorithm
     *  \{
    **/
    
    /** Run a function over the values of this instance. The behavior of this function is different, depending on the
     *  \c kind. For scalar kinds (\c kind::integer, \c kind::null, etc), \a func is called once with the value. If this
     *  is \c kind::array, \c func is called for every value in the array and the output will be an array with each
     *  element transformed by \a func. If this is \c kind::object, the result will be an object with each key
     *  transformed by \a func.
     *  
     *  \param func The function to apply to the element or elements of this instance.
    **/
    value map(const std::function<value (const value&)>& func) const&;
    
    /** Run a function over the values of this instance. The behavior of this function is different, depending on the
     *  \c kind. For scalar kinds (\c kind::integer, \c kind::null, etc), \a func is called once with the value. If this
     *  is \c kind::array, \c func is called for every value in the array and the output will be an array with each
     *  element transformed by \a func. If this is \c kind::object, the result will be an object with each key
     *  transformed by \a func.
     *  
     *  \param func The function to apply to the element or elements of this instance.
     * 
     *  \note
     *  This version of \c map provides only a basic exception-safety guarantee. If an exception is thrown while
     *  transforming a non-scalar \c kind, there is no rollback action, so \c this is left in a usable, but
     *  \e unpredictable state. If you need a strong exception guarantee, use the constant reference version of \c map.
    **/
    value map(const std::function<value (value)>& func) &&;
    
    /** \} **/
    
private:
    friend JSONV_PUBLIC value array();
    friend JSONV_PUBLIC value object();
    
private:
    detail::value_storage _data;
    jsonv::kind           _kind;
};

/** An instance with \c kind::null. This is intended to be used for convenience and readability (as opposed to using the
 *  default constructor of \c value.
**/
JSONV_PUBLIC extern const value null;

/** A user-defined literal for parsing JSON. Uses the default (non-strict) \c parse_options.
 *  
 *  \code
 *  R"({
 *    "taco": "cat",
 *    "burrito": "dog",
 *    "whatever": [ "goes", "here", 1, 2, 3, 4 ]
 *  })"_json;
 *  \endcode
**/
JSONV_PUBLIC value operator"" _json(const char* str, std::size_t len);

/** Swap the values \a a and \a b. **/
JSONV_PUBLIC void swap(value& a, value& b) noexcept;

/** \addtogroup Creation
 *  Free functions meant for easily creating \c value instances.
 *  \{
**/

/** Create an empty array value. **/
JSONV_PUBLIC value array();

/** Create an array value from the given source. **/
JSONV_PUBLIC value array(std::initializer_list<value> source);

/** Create an array with contents defined by range [\a first, \a last). **/
template <typename TForwardIterator>
value array(TForwardIterator first, TForwardIterator last)
{
    value arr = array();
    arr.assign(first, last);
    return arr;
}

/** Create an empty object. **/
JSONV_PUBLIC value object();

/** Create an object with key-value pairs from the given \a source. **/
JSONV_PUBLIC value object(std::initializer_list<std::pair<std::string, value>>  source);
JSONV_PUBLIC value object(std::initializer_list<std::pair<std::wstring, value>> source);

/** Create an object whose contents are defined by range [\a first, \a last). **/
template <typename TForwardIterator>
value object(TForwardIterator first, TForwardIterator last)
{
    value obj = object();
    obj.insert(first, last);
    return obj;
}

/** \} **/

}

namespace std
{

/** Explicit specialization of \c std::hash for \c jsonv::value types so you can store a \c value in an unordered
 *  container. Hashing results depend on the \c kind for the provided value -- most kinds directly use the hasher for
 *  their kind (hashing a \c jsonv::value for integer \c 5 should have the same hash value as directly hashing the same
 *  integer). For aggregate kinds \c array and \c object, hashing visits every sub-element recursively. This might be
 *  expensive, but is required when storing multiple values with similar layouts in the a set (which is the most common
 *  use case).
**/
template <>
struct JSONV_PUBLIC hash<jsonv::value>
{
    std::size_t operator()(const jsonv::value& val) const noexcept;
};

}

#endif/*__JSONV_VALUE_HPP_INCLUDED__*/

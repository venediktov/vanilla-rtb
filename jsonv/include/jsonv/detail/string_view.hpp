/** \file jsonv/detail/string_view.hpp
 *  The JsonVoorhees implementation of string_view. You should not include this directly; instead, prefer including
 *  \c jsonv/string_view.hpp and manipulating \c JSONV_STRING_REF_TYPE and \c JSONV_STRING_REF_INCLUDE.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_STRING_VIEW_HPP_INCLUDED__
#define __JSONV_DETAIL_STRING_VIEW_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <ostream>
#include <stdexcept>

namespace jsonv
{
namespace detail
{

/** A non-owning reference to an \c std::string, as proposed to the C++ Standard Committee by Jeffrey Yasskin in N3921
 *  (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html). Unlike the one proposed, this is not templated
 *  over the \c char type, as this project only deals with UTF-8 encoded strings.
**/
class string_view
{
public:
    using value_type             = char;
    using pointer                = const value_type*;
    using reference              = const value_type&;
    using const_reference        = const value_type&;
    using iterator               = pointer;
    using const_iterator         = pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    
    static constexpr size_type npos = size_type(~0);
    
public:
    constexpr string_view() noexcept :
            _base(nullptr),
            _length(0)
    { }
    
    string_view(pointer p) noexcept :
            _base(p),
            _length(std::strlen(p))
    { }
    
    constexpr string_view(pointer p, size_type len) noexcept :
            _base(p),
            _length(len)
    { }
    
    template <typename UAllocator>
    string_view(const std::basic_string<value_type, std::char_traits<value_type>, UAllocator>& src)
                noexcept(noexcept(src.data()) && noexcept(src.length())):
            _base(src.data()),
            _length(src.length())
    { }
    
    string_view(const string_view&) noexcept = default;
    string_view& operator=(const string_view&) noexcept = default;
    
    template <typename UAllocator>
    explicit operator std::basic_string<value_type, std::char_traits<value_type>, UAllocator>() const
    {
        return std::basic_string<value_type, std::char_traits<value_type>, UAllocator>(_base, _length);
    }

    #ifdef _MSC_VER
    // MSVC 14 CTP 5 has issues picking up templated cast operators, so provide this one for the sake of convenience
    explicit operator std::string() const
    {
        return std::string(_base, _length);
    }
    #endif
    
    constexpr size_type size()     const noexcept { return _length; }
    constexpr size_type max_size() const noexcept { return _length; }
    constexpr size_type length()   const noexcept { return _length; }
    
    constexpr bool empty() const noexcept { return _length == 0; }
    
    constexpr const_reference operator[](size_type idx) const { return _base[idx]; }
    const_reference at(size_type idx) const
    {
        if (idx < _length)
            return _base[idx];
        else
            throw std::out_of_range("jsonv::string_view::at");
    }
    
    constexpr const_reference front() const { return _base[0]; }
    constexpr const_reference back()  const { return _base[_length - 1]; }
    
    constexpr pointer data() const { return _base; }
    
    constexpr const_iterator begin()  const { return _base; }
    constexpr const_iterator cbegin() const { return _base; }
    constexpr const_iterator end()    const { return _base + _length; }
    constexpr const_iterator cend()   const { return _base + _length; }
    
    const_reverse_iterator rbegin()  const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend()    const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend()   const { return const_reverse_iterator(begin()); }
    
    void clear()
    {
        _base = nullptr;
        _length = 0;
    }
    
    void remove_prefix(size_type n)
    {
        if (n <= _length)
        {
            _base += n;
            _length -= n;
        }
        else
        {
            throw std::range_error("jsonv::string_view::remove_prefix");
        }
    }
    
    void remove_suffix(size_type n)
    {
        if (n <= _length)
        {
            _length -= n;
        }
        else
        {
            throw std::range_error("jsonv::string_view::remove_prefix");
        }
    }
    
    string_view substr(size_type idx, size_type count = npos) const
    {
        count = count == npos ? (_length - idx) : count;
        if (idx + count <= _length)
            return string_view(_base + idx, count);
        else
            throw std::range_error("jsonv::string_view::substr");
    }
    
    bool starts_with(value_type val) const
    {
        return !empty() && front() == val;
    }
    
    bool starts_with(const string_view& sub) const
    {
        return sub.length() <= length() && std::equal(sub.begin(), sub.end(), begin());
    }
    
    bool ends_with(value_type val) const
    {
        return !empty() && back() == val;
    }
    
    bool ends_with(const string_view& sub) const
    {
        return sub.length() <= length() && std::equal(sub.rbegin(), sub.rend(), rbegin());
    }
    
    size_type find(const string_view& sub) const
    {
        auto iter = std::search(begin(), end(), sub.begin(), sub.end());
        return iter == end() ? npos : std::distance(begin(), iter);
    }
    
    size_type find(value_type val) const
    {
        auto iter = std::find(begin(), end(), val);
        return iter == end() ? npos : std::distance(begin(), iter);
    }
    
    size_type rfind(const string_view& sub) const
    {
        auto iter = std::search(rbegin(), rend(), sub.rbegin(), sub.rend());
        return iter == rend() ? npos : std::distance(iter, rend());
    }
    
    size_type rfind(value_type val) const
    {
        auto iter = std::find(rbegin(), rend(), val);
        return iter == rend() ? npos : std::distance(iter, rend());
    }
    
    size_type find_first_of(value_type val) const
    {
        return find(val);
    }
    
    size_type find_first_of(const string_view& chars) const
    {
        auto iter = std::find_first_of(begin(), end(), chars.begin(), chars.end());
        return iter == end() ? npos : std::distance(begin(), iter);
    }
    
    size_type find_first_not_of(value_type val) const
    {
        auto iter = std::find_if(begin(), end(), [val] (value_type x) { return val != x; });
        return iter == end() ? npos : std::distance(begin(), iter);
    }
    
    size_type find_first_not_of(const string_view& chars) const
    {
        auto iter = std::find_if(begin(), end(),
                                 [&chars] (value_type x)
                                 {
                                     return chars.find(x) == npos;
                                 }
                                );
        return iter == end() ? npos : std::distance(begin(), iter);
    }
    
    size_type find_last_of(value_type val) const
    {
        return rfind(val);
    }
    
    size_type find_last_of(const string_view& chars) const
    {
        auto iter = std::find_first_of(rbegin(), rend(), chars.begin(), chars.end());
        return iter == rend() ? npos : std::distance(iter, rend());
    }
    
    size_type find_last_not_of(value_type val) const
    {
        auto iter = std::find_if(rbegin(), rend(), [val] (value_type x) { return val != x; });
        return iter == rend() ? npos : std::distance(iter, rend());
    }
    
    size_type find_last_not_of(const string_view& chars) const
    {
        auto iter = std::find_if(rbegin(), rend(),
                                 [&chars] (value_type x)
                                 {
                                     return chars.find(x) == npos;
                                 }
                                );
        return iter == rend() ? npos : std::distance(iter, rend());
    }
    
    bool operator==(const string_view& other) const
    {
        return _length == other._length
            && (_base == other._base || std::equal(begin(), end(), other.begin()));
    }
    
    bool operator!=(const string_view& other) const
    {
        return !operator==(other);
    }
    
    bool operator<(const string_view& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }
    
    bool operator<=(const string_view& other) const
    {
        return !(other < *this);
    }
    
    bool operator>(const string_view& other) const
    {
        return other < *this;
    }
    
    bool operator>=(const string_view& other) const
    {
        return !(*this < other);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const string_view& self)
    {
        os.write(self.begin(), self.size());
        return os;
    }
    
private:
    pointer   _base;
    size_type _length;
};

template <typename UAllocator>
bool operator==(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) == rhs;
}

template <typename UAllocator>
bool operator!=(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) != rhs;
}

template <typename UAllocator>
bool operator<(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) < rhs;
}

template <typename UAllocator>
bool operator<=(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) <= rhs;
}

template <typename UAllocator>
bool operator>(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) > rhs;
}

template <typename UAllocator>
bool operator>=(const std::basic_string<UAllocator> lhs, const string_view& rhs)
{
    return string_view(lhs) >= rhs;
}

}
}

#endif/*__JSONV_DETAIL_STRING_VIEW_HPP_INCLUDED__*/

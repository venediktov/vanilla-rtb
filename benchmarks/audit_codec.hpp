#pragma once

/*
   Copyright 2017 Vladimir Lysyy (mrbald@github)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <cstddef>
#include <cstring>
#include <cassert>
#include <array>
#include <tuple>
#include <type_traits>

#include <string_view>

#include <tuple>

namespace auditor {

#if __cplusplus == 201402L
namespace details {

/**
 * @arg Op folding operator
 * @arg Args folding arguments
 */
template <typename Op, typename... Args> inline
constexpr auto fold(Op&& op, Args&&... args)
{
    std::common_type_t<Args...> result {};
    (void)std::initializer_list<int>{(result = op(result, std::forward<Args>(args)), 0)...};
    return result;
}
static_assert(fold(std::plus<>{}, 1, 2, 3) == 6, "");

template <typename... Args> inline
constexpr auto foldsum(Args&&... args)
{
    return fold(std::plus<> {}, std::forward<Args>(args)...);
}

} // namespace details
#endif

struct typemap {
    template <typename Arg> inline
    static constexpr uint8_t index_of() {
        return index_of_<Arg>(std::make_index_sequence<std::tuple_size<mappings_t>::value>{});
    }

    template <typename Arg> inline
    static constexpr uint8_t code_of() {
        constexpr std::array<uint8_t, std::tuple_size<mappings_t>::value> type_codes {{'?','B','b','H','h','L','l','Q','q','f','d','S'}};
        return type_codes[index_of_<Arg>(std::make_index_sequence<std::tuple_size<mappings_t>::value>{})];
    }

private:
    using mappings_t = std::tuple<bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, std::string_view>;

    template <typename Arg, size_t... Is> inline
    static constexpr uint8_t index_of_(std::index_sequence<Is...>) {
#if __cplusplus > 201402L
        static_assert((std::is_same<Arg, std::tuple_element_t<Is, mappings_t>>::value + ...) == 1, ""); // unique mapping
        return static_cast<uint8_t>(((std::is_same<Arg, std::tuple_element_t<Is, mappings_t>>::value * (Is + 1)) + ...) - 1);
#else
        return static_cast<uint8_t>(details::foldsum(std::is_same<Arg, std::tuple_element_t<Is, mappings_t>>::value * (Is + 1)...) - 1);
#endif
    }
}; // struct typemap

static_assert(typemap::index_of<uint8_t>() == 1, "");
static_assert(typemap::index_of<int32_t>() == 6, "");

// ------------------------------------------------------------------------------

template<typename Arg> inline
constexpr uint32_t field_size() {
    using naked_t = std::remove_cv_t<std::remove_reference_t<Arg>>;

    constexpr bool is_blob =
            std::is_same<std::string_view, naked_t>::value
            || std::is_array<naked_t>::value
            || std::is_pointer<naked_t>::value;

    return 1 /* type tag */ + (is_blob ? sizeof(uint32_t) : sizeof(Arg));
}

template<typename Arg> inline
constexpr uint32_t blob_size(Arg&&) { return 0u; }

inline
constexpr uint32_t blob_size(std::string_view arg) { return  1 /* Start-of-Text */ + sizeof(uint32_t) + arg.size(); }

inline
constexpr uint32_t blob_size(char const* arg) { return blob_size(std::string_view(arg)); }

template <int N> inline
constexpr uint32_t blob_size(char(&arg)[N]) { return blob_size(std::string_view(arg, N - 1)); }

#if __cplusplus > 201402L
template<typename... Args> inline
constexpr uint32_t record_size() { return (field_size<Args>() + ...); }
#else
template<typename... Args> inline
constexpr uint32_t record_size() { return details::foldsum(field_size<Args>()...); }
#endif
static_assert(record_size<uint8_t, uint16_t, uint32_t>() == 10, "");

template<typename... Args> inline
constexpr uint32_t full_record_size(Args&&... args) {
    constexpr uint32_t fixed_part_size = record_size<Args...>();

    const auto blobs_part_size =
#if __cplusplus > 201402L
        (blob_size(std::forward<Args>(args)) + ...);
#else
        details::foldsum(blob_size(std::forward<Args>(args))...);
#endif

    return 1 /* Start-of-Heading */ + sizeof(fixed_part_size) + fixed_part_size + blobs_part_size;
}

// ------------------------------------------------------------------------------

template<typename Arg> inline
uint32_t encode_field(char* buf, uint32_t& blob_offset, Arg&& arg) {
    using naked_t = std::remove_cv_t<std::remove_reference_t<Arg>>;

    *(buf++) = typemap::code_of<naked_t>();

    memcpy(buf, reinterpret_cast<char const*>(&arg), sizeof(arg));

    return 1 /* type tag */ + sizeof(arg);
}

inline
uint32_t encode_field(char* buf, uint32_t& blob_offset, std::string_view arg) {
    *(buf++) = typemap::code_of<std::string_view>();

    memcpy(buf, reinterpret_cast<char const*>(&blob_offset), sizeof(blob_offset));
    blob_offset += blob_size(arg);

    return 1 /* type tag */ + sizeof(blob_offset);
}

template <int N> inline
uint32_t encode_field(char* buf, uint32_t& blob_offset, char(&arg)[N]) { return encode_field(buf, blob_offset, std::string_view(arg, N - 1)); }

inline
uint32_t encode_field(char* buf, uint32_t& blob_offset, char const* arg) { return encode_field(buf, blob_offset, std::string_view(arg)); }

template<typename Arg> inline
uint32_t encode_blob(char* buf, Arg&& arg) { return 0; }

inline
uint32_t encode_blob(char* blob_buf, std::string_view arg) {
    *blob_buf++ = 0x02; /* Start-of-Text */

    uint32_t const blob_len =  arg.size();
    memcpy(blob_buf, reinterpret_cast<char const*>(&blob_len), sizeof(blob_len));
    blob_buf += sizeof(blob_len);

    memcpy(blob_buf, arg.data(), blob_len);

    return blob_size(arg);
}

template <int N> inline
uint32_t encode_blob(char* buf, char(&arg)[N]) { return encode_blob(buf, std::string_view(arg, N - 1)); }

inline
uint32_t encode_blob(char* buf, char const* arg) { return encode_blob(buf, std::string_view(arg)); }

template<typename... Args> inline
auto encode_record(char* buf, Args&&... args) {
    constexpr uint32_t fixed_part_size = record_size<Args...>();

    *buf++ = 0x01; /* Start-of-Heading */

    memcpy(buf, reinterpret_cast<char const*>(&fixed_part_size), sizeof(fixed_part_size));
    buf += sizeof(fixed_part_size);

    uint32_t rel_blob_offset = 0u;

#if __cplusplus > 201402L
    char* rbeg = buf;
    ((buf += encode_field(buf, rel_blob_offset, std::forward<Args>(args))), ...);
    uint32_t rec_size = buf - rbeg;

    char* bbeg = buf;
    ((buf += encode_blob(buf, std::forward<Args>(args))), ...);
    uint32_t blobs_part_size = buf - bbeg;
    assert(blobs_part_size == (blob_size(std::forward<Args>(args)) + ...));
#else
    char* rbeg = buf;
    (void)std::initializer_list<int>{((buf += encode_field(buf, rel_blob_offset, std::forward<Args>(args))), 0)...};
    uint32_t rec_size = buf - rbeg;
    
    char* bbeg = buf;
    (void)std::initializer_list<int>{((buf += encode_blob(buf, std::forward<Args>(args))), 0)...};
    uint32_t blobs_part_size = buf - bbeg;
#endif
    assert(rel_blob_offset == blobs_part_size);
    assert(rec_size == fixed_part_size);

    return 1 /* Start-of-Heading */ + sizeof(fixed_part_size) + fixed_part_size + blobs_part_size;
}

// ------------------------------------------------------------------------------

template <class Arg>
struct decoder {
    inline
    static Arg decode(char const*& addr) {
        Arg val;
        memcpy(&val, addr, sizeof(val));
        addr += sizeof(Arg);
        return val;
    }

    template<class Fn> inline
    static void decode(char const*& buf, Fn&& fn) {
        fn(decode(buf));
    }
};

template <>
struct decoder<std::string_view> {
    template<class Fn> inline
    static void decode(char const*& buf, char const* blob_buf, Fn&& fn) {
        auto const blob_offset = decoder<uint32_t>::decode(buf);
        blob_buf += blob_offset + 1 /* Start-of-Text */;
        auto const blob_len = decoder<uint32_t>::decode(blob_buf);
        fn(std::string_view(blob_buf, blob_len));
    }
};


template<class Fn> inline
void decode_field(char const*& buf, char const* blob_buf, Fn&& fn) {
    auto const type_code = decoder<uint8_t>::decode(buf);

    switch (type_code) {
        default:
            assert(false);
            abort();
        case typemap::code_of<uint8_t>():
            decoder<uint8_t>::decode(buf, fn); break;
        case typemap::code_of<int8_t>():
            decoder<int8_t>::decode(buf, fn); break;
        case typemap::code_of<uint16_t>():
            decoder<uint16_t>::decode(buf, fn); break;
        case typemap::code_of<int16_t>():
            decoder<int16_t>::decode(buf, fn); break;
        case typemap::code_of<uint32_t>():
            decoder<uint32_t>::decode(buf, fn); break;
        case typemap::code_of<int32_t>():
            decoder<int32_t>::decode(buf, fn); break;
        case typemap::code_of<uint64_t>():
            decoder<uint64_t>::decode(buf, fn); break;
        case typemap::code_of<int64_t>():
            decoder<int64_t>::decode(buf, fn); break;
        case typemap::code_of<float>():
            decoder<float>::decode(buf, fn); break;
        case typemap::code_of<double>():
            decoder<double>::decode(buf, fn); break;
        case typemap::code_of<std::string_view>():
            decoder<std::string_view>::decode(buf, blob_buf, fn); break;
    }
}

} // namespace auditor

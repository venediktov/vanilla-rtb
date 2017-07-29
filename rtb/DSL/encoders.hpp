
/* 
 * File:   jsonv_store.hpp
 * Author: vladimir venediktov
 *
 * Created on April 9, 2017, 7:39 PM
 */

#ifndef JSONV_STORE_HPP
#define JSONV_STORE_HPP

#include "parsers/jsmn.h"
#include "jsonv/all.hpp"
#include <boost/lexical_cast.hpp>

/***** FOR BOOST ANY IMPLEMENTATION *****/
#include <boost/any.hpp>
#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    using string_view = string_ref;
}
#else
#include <boost/utility/string_view.hpp>
#endif
#include <vector>
#include <map>

namespace  encoders {
    
static inline
int encode(const char *js, jsmntok_t *t, size_t count, jsonv::value &value) {
    int i, j;
    if (count == 0) {
        return 0;
    }
    if (t->type == JSMN_PRIMITIVE) {
        auto s = jsonv::string_view(js + t->start, t->end - t->start);
        if (s.find_first_of('.') != jsonv::string_view::npos)
            value = boost::lexical_cast<double>(s.data(), s.size());
        else if (s.at(0) == '-')
            value = boost::lexical_cast<std::int64_t>(s.data(), s.size());
        else if ( s.at(0) == 't' )
            value = true;
        else if ( s.at(0) == 'f' )
            value = false;
        else
            value = static_cast<int64_t>(boost::lexical_cast<uint64_t>(s.data(), s.size()));
        return 1;
    } else if (t->type == JSMN_STRING) {
        value = jsonv::string_view(js + t->start, t->end - t->start);
        return 1;
    } else if (t->type == JSMN_OBJECT) {
        value = jsonv::object();
        j = 0;
        for (i = 0; i < t->size; ++i) {
            jsonv::value k;
            jsonv::value v;
            j += encode(js, t+1+j, count-j, k);
            j += encode(js, t+1+j, count-j, v);
            if ( k.is_null() || v.is_null()) {
                continue;
            }
            value.insert({std::move(k.as_string()),std::move(v)});
        }
        return j + 1;
    } else if (t->type == JSMN_ARRAY) {
        j = 0;
        value = jsonv::array();
        for (i = 0; i < t->size; ++i) {
            jsonv::value v;
            j += encode(js, t+1+j, count-j, v);
            if ( !v.is_null()) {
                value.push_back(std::move(v));
            }
        }
        return j + 1;
    }
    return 0;
}

/********************** FOR BOOST ANY ************************************/
static inline
int encode(const char *js, jsmntok_t *t, size_t count, boost::any &value) {
    int i, j;
    if (count == 0) {
        return 0;
    }
    if (t->type == JSMN_PRIMITIVE) {
        auto s = boost::string_view(js + t->start, t->end - t->start);
        if (s.find_first_of('.') != boost::string_view::npos)
            value = boost::lexical_cast<double>(s.data(), s.size());
        else if (!s.empty() && s[0] == '-')
            value = boost::lexical_cast<std::int64_t>(s.data(), s.size()); //value = vanilla::common::strtol(s); 
        else if ( !s.empty() && s[0] == 't' )
            value = true;
        else if ( !s.empty() && s[0] == 'f' )
            value = false;
        else
            value = static_cast<int64_t>(boost::lexical_cast<uint64_t>(s.data(), s.size()));
        return 1;
    } else if (t->type == JSMN_STRING) {
        value = boost::string_view(js + t->start, t->end - t->start);
        return 1;
    } else if (t->type == JSMN_OBJECT) {
        value = std::map<boost::string_view, boost::any>();
        j = 0;
        for (i = 0; i < t->size; ++i) {
            boost::any k;
            boost::any v;
            j += encode(js, t+1+j, count-j, k);
            j += encode(js, t+1+j, count-j, v);
            if ( k.empty() || v.empty()) {
                continue;
            }
            boost::any_cast<std::map<boost::string_view, boost::any>&>(value).insert({std::move(boost::any_cast<boost::string_view>(k)),std::move(v)});
        }
        return j + 1;
    } else if (t->type == JSMN_ARRAY) {
        j = 0;
        value = std::vector<boost::any>();
        boost::any_cast<std::vector<boost::any>&>(value).reserve(t->size);
        for (i = 0; i < t->size; ++i) {
            boost::any v;
            j += encode(js, t+1+j, count-j, v);
            if ( !v.empty()) {
                boost::any_cast<std::vector<boost::any>&>(value).push_back(std::move(v));
            }
        }
        return j + 1;
    }
    return 0;
}


} //namespace
#endif /* JSONV_STORE_HPP */


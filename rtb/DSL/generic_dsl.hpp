/* 
 * File:   generic_dsl.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on October 7, 2016, 9:08 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#ifndef RTB_DSL_GENERIC_DSL_HPP
#define RTB_DSL_GENERIC_DSL_HPP

#include "encoders.hpp"
#include "dsl_mapper.hpp"

namespace DSL {
    using namespace jsonv;

    template<typename T=std::string , template<class> class Mapper = DSL::dsl_mapper, unsigned int Size=128>
    class GenericDSL : public Mapper<T>  {
            
    public:
        using deserialized_type = typename Mapper<T>::deserialized_type;
        using serialized_type = typename Mapper<T>::serialized_type;
        using parse_error_type = typename Mapper<T>::parse_error_type;

        GenericDSL() {
            request_fmt_  = this->build_request();
            response_fmt_ = this->build_response(); 
        }

        template<typename string_view_type>
        deserialized_type extract_request(const string_view_type & bid_request) {
            jsmn_parser parser;
            jsmntok_t t[Size];
            thread_local jsonv::value encoded;
            encoded.clear();
            jsmn_init(&parser);
            auto r = jsmn_parse(&parser, bid_request.c_str(), bid_request.length(), t, sizeof(t)/sizeof(t[0]));
            if (r < 0) {
                throw std::runtime_error("DSL::jsmn_parse exception");
            }
            encoders::encode(bid_request.c_str(), &t[0], parser.toknext, encoded);
            return extract<deserialized_type>(encoded, request_fmt_);
        }

        jsonv::value create_response(const serialized_type & bid_response) {
            return to_json(bid_response, response_fmt_);
        }

    private:
        formats request_fmt_;
        formats response_fmt_;
    };

} //namespace

#endif

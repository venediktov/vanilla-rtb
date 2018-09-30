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

    template<typename T=std::string , template<class, unsigned int...> class Mapper = DSL::dsl_mapper>
    class GenericDSL : public Mapper<T>  {
        
        using encoded_type =  typename Mapper<T>::encoded_type;
    public:
        using Mapper<T>::Mapper;
        using deserialized_type = typename Mapper<T>::deserialized_type;
        using serialized_type = typename Mapper<T>::serialized_type;
        using parse_error_type = typename Mapper<T>::parse_error_type;


        template<typename string_view_type>
        deserialized_type extract_request(const string_view_type & bid_request) {
            return Mapper<T>::template extract<deserialized_type>(bid_request);
        }

        auto create_response(const serialized_type & bid_response) {
            return Mapper<T>::serialize(bid_response);
        }

    };

} //namespace

#endif

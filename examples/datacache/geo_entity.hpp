/* 
 * File:   geo_entity.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 15 февраля 2017 г., 1:05
 */

#ifndef __IPC_DATA_GEO_ENTITY_HPP__
#define __IPC_DATA_GEO_ENTITY_HPP__

#include "rtb/datacache/base_entity.hpp"
#include <string>
#include <cstdint>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace ipc { namespace data {
    
    template <typename Alloc>
    struct geo_entity : base_entity<Alloc>
    {
        using char_string = typename base_entity<Alloc>::char_string;
        using base_type   = base_entity<Alloc>; 
      
        //for tagging in multi_index_container
        struct geo_id_tag {}; // search on geo_id
        
        geo_entity( const Alloc & a ) :
            base_entity<Alloc>(a),
            geo_id{},
            ad_id{a}
        {}
            
        uint32_t geo_id;
        char_string ad_id;
        
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            geo_id = key.template get<geo_id_tag>();  
            ad_id =  char_string(data.ad_id.data(), data.ad_id.size(), base_type::allocator);
        }
        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return base_type::size(std::forward<Serializable>(data)) +                
                   sizeof(data.geo_id) +                
                   ss.str().size() ;
        }
        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            data.geo_id = geo_id;
            data.ad_id = std::move(std::string(ad_id.data(), ad_id.length()));
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(geo_entity &entry) const {
            entry.geo_id=geo_id;
            entry.ad_id=ad_id;
        }
    };
   
 
 
template<typename Alloc>
using geo_container =
boost::multi_index_container<
    geo_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename geo_entity<Alloc>::geo_id_tag>,
            boost::multi_index::composite_key<
              geo_entity<Alloc>,
              BOOST_MULTI_INDEX_MEMBER(geo_entity<Alloc>,uint32_t,geo_id),
              BOOST_MULTI_INDEX_MEMBER(geo_entity<Alloc>,typename geo_entity<Alloc>::char_string,ad_id)
            >
        >
    >,
    boost::interprocess::allocator<geo_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
  
}}

#endif /* __IPC_DATA_GEO_ENTITY_HPP__ */


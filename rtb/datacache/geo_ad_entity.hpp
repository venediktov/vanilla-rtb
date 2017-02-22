/* 
 * File:   geo_ad.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 15 февраля 2017 г., 1:05
 */

#ifndef __IPC_DATA_GEO_AD_ENTITY_HPP__
#define __IPC_DATA_GEO_AD_ENTITY_HPP__

#include "base_entity.hpp"
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
    struct geo_ad_entity : base_entity<Alloc>
    {
        using char_string = typename base_entity<Alloc>::char_string;
        using base_type   = base_entity<Alloc>; 
      
        //for tagging in multi_index_container
        struct geo_id_tag {}; // search on geo_id
        
        geo_ad_entity( const Alloc & a ) :
            base_entity<Alloc>(a),
            geo_id{}
        {}
            
        uint32_t geo_id;
        
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            base_type::store(std::forward<Serializable>(data)) ;
            geo_id = key.template get<geo_id_tag>();  
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
            base_type::template retrieve(data) ;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(geo_ad_entity &entry) const {
            base_entity<Alloc>::operator()(static_cast<base_type &>(entry));            
            entry.geo_id=geo_id;
        }
    };
   
 
 
template<typename Alloc>
using geo_ad_container =
boost::multi_index_container<
    geo_ad_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename geo_ad_entity<Alloc>::geo_id_tag>,
              BOOST_MULTI_INDEX_MEMBER(geo_ad_entity<Alloc>,uint32_t,geo_id)
        >
    >,
    boost::interprocess::allocator<geo_ad_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
  
}}

#endif /* __IPC_DATA_GEO_AD_ENTITY_HPP__ */


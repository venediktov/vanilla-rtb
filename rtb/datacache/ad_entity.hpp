/* 
 * File:   ad_entity.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 4 февраля 2017 г., 20:39
 */

#ifndef __IPC_DATA_AD_ENTITY_HPP__
#define __IPC_DATA_AD_ENTITY_HPP__

#include "base_entity.hpp" 
#include <string>
#include <sstream>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
  
namespace ipc { namespace data {
    
    template <typename Alloc>
    struct ad_entity : base_entity<Alloc>
    {
        using char_string = typename base_entity<Alloc>::char_string;
        using base_type   = base_entity<Alloc>; 
      
        //for tagging in multi_index_container
        struct ad_id_tag {}; // search on ad_id
        
       
        ad_entity( const Alloc & a ) :
            base_entity<Alloc>(a),
            ad_id(a)
        {} //ctor END
       
        char_string ad_id;
 
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            base_type::store(std::forward<Serializable>(data)) ;
            //Store keys
            const std::string &key_ad_id     = key.template get<ad_id_tag>() ;
            
            ad_id = char_string(key_ad_id.data(), key_ad_id.size(), base_type::allocator);
            
        }
        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return base_type::size(std::forward<Serializable>(data)) +
                   sizeof(data.ad_id) +
                   ss.str().size() ;
        }
        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            base_type::template retrieve(data) ;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(ad_entity &entry) const {
            base_entity<Alloc>::operator()(static_cast<base_type &>(entry));
            entry.ad_id=ad_id;
        }
    };
   
 
 
template<typename Alloc>
using ad_container =
boost::multi_index_container<
    ad_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename ad_entity<Alloc>::ad_id_tag>,
            BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,typename ad_entity<Alloc>::char_string,ad_id)
        >
    >,
    boost::interprocess::allocator<ad_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
    
}}

#endif /* __IPC_DATA_AD_ENTITY_HPP__ */


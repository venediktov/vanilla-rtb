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
        
        struct size_tag {}; // search on width-height
        struct width_tag {}; // search on width
        struct height_tag {}; // search on height
        
       
        ad_entity( const Alloc & a ) :
            base_entity<Alloc>(a)
        {} //ctor END
       
        uint16_t width;
        uint16_t height;
        
        
 
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            base_type::store(std::forward<Serializable>(data)) ;
            //Store keys
            
            width = key.template get<width_tag>();
            height = key.template get<height_tag>();  
        }
        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return base_type::size(std::forward<Serializable>(data)) +                
                   sizeof(data.width) +
                   sizeof(data.height) +                
                   ss.str().size() ;
        }
        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            base_type::template retrieve(data) ;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(ad_entity &entry) const {
            base_entity<Alloc>::operator()(static_cast<base_type &>(entry));            
            entry.width=width;
            entry.height=height;
        }
    };
   
 
 
template<typename Alloc>
using ad_container =
boost::multi_index_container<
    ad_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename ad_entity<Alloc>::size_tag>,
            boost::multi_index::composite_key<
                ad_entity<Alloc>,
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint16_t,width),
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint16_t,height)
            >
        >
    >,
    boost::interprocess::allocator<ad_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
    
}}

#endif /* __IPC_DATA_AD_ENTITY_HPP__ */


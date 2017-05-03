/* 
 * File:   ad_entity.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 4 февраля 2017 г., 20:39
 */

#ifndef __IPC_DATA_AD_ENTITY_HPP__
#define __IPC_DATA_AD_ENTITY_HPP__

#include "rtb/datacache/base_entity.hpp" 
#include <string>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
  
namespace ipc { namespace data {
    
    template <typename Alloc>
    struct ad_entity : base_entity<Alloc> {
        using base_type   = base_entity<Alloc>; 
        using char_string = typename base_type::char_string;
      
        //for tagging in multi_index_container
        
        struct ad_id_tag {}; // search on ad_id
        struct campaign_tag{}; // searcj om campaign
        struct campaign_size_tag {}; // search on campaign-width-height
        struct width_tag {}; // search on width
        struct height_tag {}; // search on height
        
       
        ad_entity( const Alloc & a ) :
            base_entity<Alloc>(a),
            ad_id{},
            campaign_id{},
            width{},
            height{},
            position{},
            max_bid_micros{},
            code(a)
        {} //ctor END
       
        uint64_t ad_id;
        uint32_t campaign_id;
        uint16_t width;
        uint16_t height;
        uint64_t position;
        uint64_t max_bid_micros;
        char_string code;
        
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            campaign_id = key.template get<campaign_tag>();
            width = key.template get<width_tag>();
            height = key.template get<height_tag>();  
            ad_id = key.template get<ad_id_tag>();  
            position = data.position;
            max_bid_micros = data.max_bid_micros;
            code = char_string(data.code.data(), data.code.size(), base_type::allocator);
        }
        
        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            data.campaign_id=campaign_id;
            data.width=width;
            data.height=height;
            data.ad_id = ad_id;
            data.position = position;
            data.max_bid_micros = max_bid_micros;
            data.code = std::string(code.data(), code.size());
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(ad_entity &entry) const {
            entry.campaign_id=campaign_id;
            entry.width=width;
            entry.height=height;
            entry.ad_id = ad_id;
            entry.position = position;
            entry.max_bid_micros = max_bid_micros;
            entry.code = code;

        }
    };
   
 
 
template<typename Alloc>
using ad_container =
boost::multi_index_container<
    ad_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename ad_entity<Alloc>::campaign_size_tag>,
            boost::multi_index::composite_key<
                ad_entity<Alloc>,
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint32_t,campaign_id),
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint16_t,width),
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint16_t,height),
                BOOST_MULTI_INDEX_MEMBER(ad_entity<Alloc>,uint64_t,ad_id)
            >
        >
    >,
    boost::interprocess::allocator<ad_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
    
}}

#endif /* __IPC_DATA_AD_ENTITY_HPP__ */


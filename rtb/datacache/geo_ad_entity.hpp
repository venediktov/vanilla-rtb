/* 
 * File:   geo_ad.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 15 февраля 2017 г., 1:05
 */

#ifndef __IPC_DATA_AD_GEO_ENTITY_HPP__
#define __IPC_DATA_AD_GEO_ENTITY_HPP__

namespace ipc { namespace data {
    
    template <typename Alloc>
    struct geo_ad_entity : base_entity<Alloc>
    {
        using char_string = typename base_entity<Alloc>::char_string;
        using base_type   = base_entity<Alloc>; 
      
        //for tagging in multi_index_container
        
        struct geo_ad_tag {}; // search on geo_id -> ad_id
        struct ad_id_tag {}; // search on ad_id
        struct geo_id_tag {}; // search on geo_id
        
       
        geo_ad_entity( const Alloc & a ) :
            base_entity<Alloc>(a),
                ad_id(a)
        {} //ctor END
        uint32_t geo_id;
        char_string ad_id;
        
        
        
 
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            base_type::store(std::forward<Serializable>(data)) ;
            //Store keys
            const std::string &key_ad_id = key.template get<ad_id_tag>() ;
            
            ad_id = char_string(key_ad_id.data(), key_ad_id.size(), base_type::allocator);
            geo_id = key.template get<geo_id_tag>();  
        }
        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return base_type::size(std::forward<Serializable>(data)) +                
                   sizeof(data.ad_id) +
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
            entry.ad_id=ad_id;
            entry.geo_id=geo_id;
        }
    };
   
 
 
template<typename Alloc>
using geo_ad_container =
boost::multi_index_container<
    geo_ad_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename geo_ad_entity<Alloc>::geo_ad_tag>,
            boost::multi_index::composite_key<
                geo_ad_entity<Alloc>,
                BOOST_MULTI_INDEX_MEMBER(geo_ad_entity<Alloc>,uint32_t,geo_id), 
                BOOST_MULTI_INDEX_MEMBER(geo_ad_entity<Alloc>,typename geo_ad_entity<Alloc>::char_string,ad_id)
                
            >
        >
    >,
    boost::interprocess::allocator<geo_ad_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
  
}}

#endif /* __IPC_DATA_AD_GEO_ENTITY_HPP__ */


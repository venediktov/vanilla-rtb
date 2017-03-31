/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   multibidder_collector.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:08
 */

#ifndef MULTIBIDDER_COLLECTOR_HPP
#define MULTIBIDDER_COLLECTOR_HPP

#include <functional>
#include <vector>
#include "rtb/core/openrtb.hpp"

namespace vanilla {
class multibidder_collector {
    public:
        using response_handler_type = std::function<void(const multibidder_collector*)>;
        using add_handler_type = std::function<void(const multibidder_collector*)>;
        using self_type = multibidder_collector;
        using responses_type = std::vector<openrtb::BidResponse>;
                
        multibidder_collector(int num_bidders) :
            num_bidders{num_bidders}
        {}
        
        self_type &on_response(const response_handler_type &response_handler_) {
            response_handler = response_handler_;
            return *this;
        }    
        self_type &on_add(const add_handler_type & add_handler_) {
            add_handler = add_handler_;
            return *this;
        }    
        
        openrtb::BidResponse response() {
            if(response_handler) {
                response_handler(this);
            }
            
            if(responses.empty()) {
                return openrtb::BidResponse(); 
            }
            // TODO 
            // Add ability to pass custom selection algorithm
            std::sort(responses.begin(), responses.end(), [](const openrtb::BidResponse &first, const openrtb::BidResponse & second) -> bool {
                if (!first.seatbid.size() || !first.seatbid[0].bid.size()) {
                    return false;
                }
                if (!second.seatbid.size() || !second.seatbid[0].bid.size()) {
                    return true;
                }
                return first.seatbid[0].bid[0].price > second.seatbid[0].bid[0].price;
            }); // sort by first imp bid?
            return responses[0]; 
        }

        
        bool done() const {
            return responses.size() == num_bidders;
        }
        self_type & clear() {
            responses.clear();
            return *this;
        }
        
        void add(openrtb::BidResponse && bid) {
            if(add_handler) {
                add_handler(this);
            }
            responses.emplace_back(bid);
        }
        
        const responses_type &get_reponses() const {
            return responses;
        }
        
        int get_num_bidders() const {
            return num_bidders;
        }
    private:
        int num_bidders;
        
        responses_type responses;
        response_handler_type response_handler;
        add_handler_type add_handler;
    };
}
#endif /* MULTIBIDDER_COLLECTOR_HPP */


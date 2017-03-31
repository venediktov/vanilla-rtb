/* 
 * File:   empty_asio_client_adapter.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:49
 */

#ifndef EMPTY_ASIO_CLIENT_ADAPTER_HPP
#define EMPTY_ASIO_CLIENT_ADAPTER_HPP

#include <functional>
#include <string>
#include <boost/asio/io_service.hpp>

namespace vanilla {    
    class empty_asio_client_adapter {
    public:
        using connection_ok_handler_type = std::function<void(boost::asio::io_service&)>;
        using connection_fail_handler_type = std::function<void(const std::string&, boost::asio::io_service&)>;
        using get_handler_type = std::function<void(boost::asio::io_service&, const std::string&)>;
    
        empty_asio_client_adapter(boost::asio::io_service &io):
            io{io}
        {}
        void connect(const std::string &host, uint16_t port, const connection_ok_handler_type &connection_ok_handler, const connection_fail_handler_type &connection_fail_handler) {        
            // Please call specific clients  connect method here,
            // call connection_ok_handler & connection_fail_handler in callback lambda
            connection_ok_handler(this->io);        
        }
        void get(const std::string &key, std::string &data, const get_handler_type &get_handler) {
            // Please call specific clients  get method here,
            // call get_handler  in callback lambda
            get_handler(this->io, "");    
        }
        bool connected() const {
            // Please check weather specific client is connected here,
            return true;
        }
    private:
        boost::asio::io_service &io;
    };
    
}

#endif /* EMPTY_ASIO_CLIENT_ADAPTER_HPP */


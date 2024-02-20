/* 
 * File:   asio_key_value_client.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:45
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

#ifndef ASIO_KEY_VALUE_CLIENT_HPP
#define ASIO_KEY_VALUE_CLIENT_HPP

#include <functional>
#include <boost/asio/io_service.hpp>

namespace vanilla {
    namespace client {
        template <typename Wrapper>
        class asio_key_value_client {
        public:
            using self_type = asio_key_value_client;
            using response_handler_type = std::function<void(const std::string&) >;

            asio_key_value_client() :
                client{io}
            {}

            self_type &response(const response_handler_type &handler) {
                response_handler = handler;
                return *this;
            }

            void request(const std::string &key, std::string &data) {
                client.get(key, data, [](boost::asio::io_service& io, [[maybe_unused]] const std::string & data) {
                    io.stop();
                });

                io.reset();
                io.run();

                if(response_handler) {
                    response_handler(data);
                }
            }

            void connect(const std::string &host, uint16_t port) {
                client.connect(host, port,
                    [](boost::asio::io_service & io) {
                        io.stop();
                    },
                    []([[maybe_unused]] const std::string& err, boost::asio::io_service & io) {
                        io.stop();
                    }
                );
                io.reset();
                io.run();
            }

            bool connected() const {
                return client.connected();
            }

        private:
            boost::asio::io_service io;
            Wrapper client;
            response_handler_type response_handler;
        };
    }
}

#endif /* ASIO_KEY_VALUE_CLIENT_HPP */


/* 
 * File:   asio_key_value_client.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:45
 */

#ifndef ASIO_KEY_VALUE_CLIENT_HPP
#define ASIO_KEY_VALUE_CLIENT_HPP

#include <functional>
#include <boost/asio/io_service.hpp>

namespace vanilla {
    namespace cleint {
        template <typename Wrapper>
        class asio_key_value_client {
        public:
            using self_type = asio_key_value_client;
            using response_handler_type = std::function<void(void) >;

            asio_key_value_client() :
                client{io}
            {
            }

            self_type &response(const response_handler_type &handler) {
                response_handler = handler;
                return *this;
            }

            void request(const std::string &key, std::string &data) {
                client.get(key, data, [](boost::asio::io_service& io, const std::string & data) {
                    io.stop();
                });

                io.reset();
                io.run();

                response_handler();
            }

            void connect(const std::string &host, uint16_t port) {
                client.connect(host, port,
                    [](boost::asio::io_service & io) {
                        io.stop();
                    },
                [](const std::string& err, boost::asio::io_service & io) {
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


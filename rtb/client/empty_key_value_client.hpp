/* 
 * File:   empty_key_value_client.h
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:33
 */

#ifndef EMPTY_KEY_VALUE_CLIENT_H
#define EMPTY_KEY_VALUE_CLIENT_H

#include <functional>
#include <string>

namespace vanilla {
    namespace client {
        class empty_key_value_client {
        public:
            using self_type = empty_key_value_client;
            using response_handler_type = std::function<void(void) >;

            empty_key_value_client() {
            }

            self_type &response(const response_handler_type &handler) {
                response_handler = handler;
                return *this;
            }

            void request(const std::string &key, std::string &data) {
                response_handler();
            }

            void connect(const std::string &host, uint16_t port) {
            }

            bool connected() const {
                return true;
            }
        private:
            response_handler_type response_handler;
        };

    }
}

#endif /* EMPTY_KEY_VALUE_CLIENT_H */


/*
 * File:   shared_io_service.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on February 20, 2017, 11:19 PM
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

#ifndef __VANILLA_MESSAGING_SHARED_IO_SERVICE__
#define __VANILLA_MESSAGING_SHARED_IO_SERVICE__

#include <boost/asio.hpp>

namespace vanilla { namespace messaging {

struct shared_io_service {
    shared_io_service() : io_service_ptr{new boost::asio::io_service}
    {}
    void run() {
        io_service_ptr->run();
    }
    void stop() {
        io_service_ptr->stop();
    }
    operator boost::asio::io_service& () const {
        return *io_service_ptr;
    }
private:
   std::shared_ptr<boost::asio::io_service> io_service_ptr;
};

}}

#endif /* __VANILLA_MESSAGING_SHARED_IO_SERVICE__ */


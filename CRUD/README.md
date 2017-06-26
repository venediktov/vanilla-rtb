# CRUD
Restful web-service written in C++11  based on boost.ASIO and CRUD handlers

### REST handler with regex

```C++
using regex_restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    regex_restful_dispatcher_t regex_handler(".") ; //root is irrelavant for REST only used for web-server
    regex_handler.crud_match(boost::regex("/venue_handler/(\w+)") )
        .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            r << "{}" << http::server::reply::flush("json") ;
            std::cout << "POST group_1_match=[ << match[1] << "], request_data=" << match.data << std::endl;
        });

```

### simple REST handler

```C++
// SIMPLE NO REGEX MATCH FOR POST "/venue_handler/RTB"
    using simple_restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply, std::string, std::string> ;
    simple_restful_dispatcher_t simple_handler(".") ; //root is irrelavant for REST only used for web-server
    simple_handler.crud_match(std::string("/venue_handler/RTB") )
        .post([](http::server::reply & r, const http::crud::crud_match<std::string> & match) {
            r << "{}" << http::server::reply::flush("json") ;
            std::cout << "POST request_data=" << match.data << std::endl;
        });
```


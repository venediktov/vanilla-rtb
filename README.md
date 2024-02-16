# CRUD
High performance Restful web-service library written in C++11  based on boost.ASIO and CRUD handlers

This library supports persistent connections to achieve highest throughput and utilizes optinally regex for enpoints

[![build ](https://travis-ci.org/venediktov/CRUD.svg?branch=master)](https://travis-ci.org/venediktov/CRUD)

### Installing from GitHub
```bash
git clone https://github.com/venediktov/CRUD.git
```

### Building on Linux 
```bash
$ mkdir Release
$ cd Release
$ cmake -DCMAKE_BUILD_TYPE=Release -DCRUD_WITH_EXAMPLES=1 .. -G "Unix Makefiles"
$ make -j $(nproc) install

```

### Building on  Mac OS 
```bash
$ mkdir Release
$ cd Release
$ cmake -DCMAKE_BUILD_TYPE=Release -DCRUD_WITH_EXAMPLES=1 .. -G "Unix Makefiles"
$ make -j $(sysctl -n hw.physicalcpu) install

```



### Running examples 
```bash
$ cd Release/examples
$ ./webserver 0.0.0.0 8080 . & 
$ ./simple_restful_service 0.0.0.0 8081 . & 
$ ./regex_restful_service 0.0.0.0 8082 . & 
$ ./persisted_regex_restful_service 0.0.0.0 8083 . &

```


### Testing output and benchmarks
```bash
curl localhost:8080
curl localhost:8081/venue_handler/RTB
curl localhost:8082/venue_handler/ANY/123
curl localhost:8083/venue_handler/ANY/123
ab -k -n 100000 -c 30 http://localhost:8081/RTB
ab -k -n 100000 -c 30 http://localhost:8082/ANY/123
ab -k -n 100000 -c 30 http://localhost:8083/ANY/123

```

### Stop all background examples
```bash
pkill -9 "persisted|restful|webserver"
```


## Utilizing API

### REST handler with regex

```C++
    using regex_restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply>;
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
    using simple_restful_dispatcher_t = http::crud::crud_dispatcher<http::server::request, http::server::reply, std::string, std::string>;
    simple_restful_dispatcher_t simple_handler(".") ; //root is irrelavant for REST only used for web-server
    simple_handler.crud_match(std::string("/venue_handler/RTB") )
                  .post([](http::server::reply & r, const http::crud::crud_match<std::string> & match) {
                     r << "{}" << http::server::reply::flush("json") ;
                     std::cout << "POST request_data=" << match.data << std::endl;
                   });
```
### All in one go
```C++
    // CREAT/READ/UPDATE/DELETE "/venue_handler/XEMDP/123"
    handler.crud_match(boost::regex("/venue_handler/(\\w+)/(\\d+)") )
           .put([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match)
              std::cout << "CREATE request=" << match[0] << "/" << match[1] << std::endl;
              r = http::server::reply::stock_reply(http::server::reply::no_content);
           })
           .get([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              r << "name: " << match[1] << ", instance number: " << match[2]
                << http::server::reply::flush("text") ;
              std::cout << "READ request=" << match[0] << std::endl;
           })
           .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch>  & match) {
              r << "name: " << match[1] << ", instance number: " << match[2]
                << http::server::reply::flush("text") ;
              std::cout << "UPDATE request=" << match[0] << std::endl;
              std::cout << "UPDATE request_data=" << match.data << std::endl;
           })
           .del([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match)
              std::cout << "DELETE request=" << match[0] << "/" << match[1] << std::endl;
              r = http::server::reply::stock_reply(http::server::reply::no_content);
           }) ;
```

### Setting up connection types

#### For keep-alive persistent connections specifiy second template argument with presistent type for connection
```C++
http::server::server<simple_restful_dispatcher_t, http::server::persistent_connection> server{host,port,handler};
```

#### By default server is using non-persistent connection from the library 
```C++
http::server::server<simple_restful_dispatcher_t> server{host,port,handler};
```

#### Adding CRUD as GitHub submodule to your project
Create a file .gitmodules in the root of your project with contents
```bash
[submodule "CRUD"]
	path = CRUD
	url = https://github.com/venediktov/CRUD.git
```

Or execute following command in your repo
```bash
git submodule add https://github.com/venediktov/CRUD
```

If you add CRUD as submodule you will have to use ```git clone --recursive``` for your repo in order to get us as dependency

You can look at GitHub subtrees instead ```git subtree add --prefix CRUD git@github.com:venediktov/CRUD.git master --squash```

## Support on Beerpay
Hey dude! Help me out for a couple of :beers:!

[![Beerpay](https://beerpay.io/venediktov/CRUD/badge.svg?style=beer-square)](https://beerpay.io/venediktov/CRUD)  [![Beerpay](https://beerpay.io/venediktov/CRUD/make-wish.svg?style=flat-square)](https://beerpay.io/venediktov/CRUD?focus=wish)

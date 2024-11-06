/*
 * File:   crud_dispatcher.hpp
* Author: vvenedict@gmail.com
*
*      dispatcher<response,request> d;
*             // READ or CREAT/UPDATE "/venue_handlers/XEMDP/123"
*             d.crud_match(boost::regex("/stp_handler/(\w+)/(\d+)") )
*                             .get([](response & r, const boost::cmatch & what) {
*                   r << "name: " << what[1] << ", instance number: " << what[2]
 *                     << http::server::reply::flush("text") ;
*                   std::cout << "body=" << what[0] << std::endl;
*                             })
*                             .post([](response & r, const boost::cmatch & what) {
*                   r << "name: " << what[1] << ", instance number: " << what[2]
 *                     << http::server::reply::flush("text") ;
*                   std::cout << "body=" << what[0] << std::endl;
*                                             std::cout << "body=" << what[0] << std::endl;
*                             });
*
*             // READ "/venue_handlers/XEMDP/"
*             d.crud_matcher(boost::regex("/handlers/(\w+)") )
*                             .get([](response & r, const boost::cmatch & what) {
*                                             r << "id: " ;
*                                             r << what[1] << http::server::reply::flush("text") ;
*                             });
*
*             // READ "/venue_handlers"
*             d.crude_matcher(boost::regex("/handlers"))
*                             .get([](response & r, const boost::cmatch & what) {
*                                             r << "received:" << what[0];
*                      r << http::server::reply::flush("text") ;
*                             });
*
*             TESTING LIBRARY WITh CURL :
*             curl http://localhost:8987/venue_handlers
*             curl http://localhost:8987/venue_handlers/XEMDP
*             curl http://localhost:8987/venue_handlers/XEMDP/10
*
 *
 * Created on September 29, 2015, 5:59 PM
*/
 
#ifndef HTTP_CRUD_DISPATCHER_HPP
#define HTTP_CRUD_DISPATCHER_HPP
 
#include "crud_matcher.hpp"
#include <boost/regex.hpp>
#include <memory>
#ifdef __GNUC__
#include <map>
#else
//due to CC 12.4 bug #20814942, Assetion ../lnk/declarations.cc line 1378 std::map<int, shared_ptr<T>>
#include <boost/container/flat_map.hpp>
#endif
 
namespace http::crud {
template <typename Request, typename Response, typename Match = boost::cmatch, typename Expression = boost::regex,
          typename CrudMatcher = crud_matcher<Response, Expression, Match>>
class crud_dispatcher {
        using  crud_matcher_type = CrudMatcher;
        using  crud_matcher_type_p = std::shared_ptr<crud_matcher_type>;
    public :
        crud_dispatcher() : _base_path() {}
        explicit crud_dispatcher(const std::string &base_path) : _base_path(base_path) {}
        crud_matcher_type & crud_match(const Expression &expression) {
            crud_matcher_type_p & p = _crud_matchers[expression] ;
            if(!p) {
                p = std::make_shared<crud_matcher_type>() ;
            }
            return *p;
        }
        template<typename E = Expression>
        typename std::enable_if<!std::is_same<E, std::string>::value, void>::type
        handle_request(const Request& request, Response& response) {
            for ( const auto &matcher : _crud_matchers ) {
                Match what;
                if ( boost::regex_match(request.uri.c_str(), what,  matcher.first ) ) {
                    matcher.second->handle_request(request, response, what) ;
                }
            }
        }
        template<typename E = Expression>
        typename std::enable_if<std::is_same<E, std::string>::value, void>::type
        handle_request(const Request& request, Response& response) {
            for ( const auto &matcher : _crud_matchers ) {
                Match what;
                if ( request.uri.find(matcher.first) != std::string::npos  ) {
                    what = matcher.first;
                    matcher.second->handle_request(request, response, what) ;
                }
            }
        }
    private:
        std::string _base_path;
#ifdef __GNUC__
        std::map<Expression, crud_matcher_type_p> _crud_matchers ;
#else
        boost::container::flat_map<Expression, crud_matcher_type_p> _crud_matchers ;
#endif
    };
   
}

#endif   /* HTTP_CRUD_DISPATCHER_HPP */
 

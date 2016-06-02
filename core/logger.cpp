#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>



void init_framework_logging(const std::string & full_file_name){
    
boost::log::add_file_log(
         boost::log::keywords::file_name = full_file_name + "_%Y%m%d.log",
         boost::log::keywords::open_mode = (std::ios::out | std::ios::app),
         boost::log::keywords::auto_flush = true,
         boost::log::keywords::format =
         (
                 boost::log::expressions::stream
                         << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S.%f")
                         << ": <" << boost::log::trivial::severity
                         << "> " << boost::log::expressions::smessage
         )
 );
 boost::log::add_console_log(
         std::cout,
         boost::log::keywords::format =
         (
                 boost::log::expressions::stream
                         << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S.%f")
                         << ": <" << boost::log::trivial::severity
                         << "> " << boost::log::expressions::smessage
         )
 );

 boost::log::add_common_attributes();

}
    

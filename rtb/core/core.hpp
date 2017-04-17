/* 
 * File:   core.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 17 апреля 2017 г., 23:34
 */

#ifndef CORE_HPP
#define CORE_HPP

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

/*
#ifdef NDEBUG
#include <iosfwd>
struct nullstream_t : public std::ostream {
    nullstream_t() : std::ios(0), std::ostream(0) {}
};

static nullstream_t nullstream;
#define LOG(x) nullstream
#else */
#define LOG(x) BOOST_LOG_TRIVIAL(x)
//#endif

#endif /* CORE_HPP */


/*
 * File:   extractors.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on July 28, 2017, 9:00 PM
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


#pragma once
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rtb/core/openrtb.hpp"
#include "rtb/core/core.hpp"
#include <boost/optional.hpp>
#include <algorithm>

#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    using string_view = string_ref;
}
#else
#include <boost/utility/string_view.hpp>
#endif

template<typename T>
struct extractors {
static T extract( boost::any & value );
static T extract(rapidjson::Value &value);
static T extract(rapidjson::Document &m);
};

template<typename T>
struct extractors<std::vector<T>> {
    static std::vector<T> extract( boost::any & value ) {
        std::vector<T> decoded_vector;
        if ( value.empty() ) {
            return decoded_vector;
        }
        auto &encoded_vector = boost::any_cast<std::vector<boost::any> &>(value);
        decoded_vector.reserve(encoded_vector.size());
        std::transform( std::begin(encoded_vector),
                        std::end(encoded_vector),
                        std::back_inserter(decoded_vector), []( boost::any & a) {
                            return extractors<T>::extract(a);
                      });

        return decoded_vector;
    }
    static std::vector<T> extract(rapidjson::Value &value) {
        std::vector<T> decoded_vector;
        if (value.IsNull() || !value.IsArray()) {
            return decoded_vector;
        }
        auto encoded_vector = value.GetArray();
        decoded_vector.reserve(encoded_vector.Size());
        std::transform(encoded_vector.Begin(),
                       encoded_vector.End(),
                       std::back_inserter(decoded_vector), [](rapidjson::Value &a) {
                    return extractors<T>::extract(a);
                });

        return decoded_vector;
    }
};

//struct declarations parcial specializations
template<typename T>
struct extractors<openrtb::Impression<T>> {
static openrtb::Impression<T> extract( boost::any & value );
static openrtb::Impression <T> extract(rapidjson::Value &value);
};

template<typename T>
struct extractors<boost::optional<openrtb::User<T>>> {
static boost::optional<openrtb::User<T>> extract( boost::any & value );
static boost::optional<openrtb::User<T>> extract( rapidjson::Value & value );
};

template<typename T>
struct extractors<boost::optional<openrtb::Geo<T>>> {
static boost::optional<openrtb::Geo<T>> extract( boost::any & value );
static boost::optional<openrtb::Geo<T>> extract( rapidjson::Value & value );
};

template<typename T>
struct extractors<boost::optional<openrtb::Site<T>>> {
static boost::optional<openrtb::Site<T>> extract( boost::any & value );
static boost::optional<openrtb::Site<T>> extract( rapidjson::Value & value );
};

template<typename T>
struct extractors<boost::optional<openrtb::Banner<T>>> {
static boost::optional<openrtb::Banner<T>> extract( boost::any & value );
static boost::optional<openrtb::Banner<T>> extract( rapidjson::Value & value );
};

template<typename T>
struct extractors<openrtb::BidRequest<T>> {
static openrtb::BidRequest<T> extract( boost::any & value ) {
    openrtb::BidRequest<T> request;
    if ( value.empty() ) {
        return request;
    }
    try {
       auto &m = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
       auto &id     = boost::any_cast<boost::string_view &>(m["id"]);
       request.id   = decltype(request.id)(id.data(), id.size());
       request.imp  = extractors<decltype(request.imp)>::extract(m["imp"]);
       request.user = extractors<decltype(request.user)>::extract(m["user"]);
       request.site = extractors<decltype(request.site)>::extract(m["site"]);
    } catch (const std::exception &e) {
       LOG(error) << "openrtb::BidRequest<T> extract exception " << e.what() ;
    }
    return request;
}

static openrtb::BidRequest<T> extract(rapidjson::Document &m);
};

//extract functions implementation inline keyword ?
template<typename T>
openrtb::Impression<T> 
extractors<openrtb::Impression<T>>::extract( boost::any & value ) {
    openrtb::Impression<T> imp;
    if ( value.empty() ) {
        return imp;
    }
    try {
       auto &m = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
       auto &id          = boost::any_cast<boost::string_view &>(m["id"]);
       imp.id            = decltype(imp.id)(id.data(), id.size());
       imp.banner        = extractors<decltype(imp.banner)>::extract(m["banner"]);
       imp.bidfloor      = boost::any_cast<decltype(imp.bidfloor)>(m["bidfloor"]);
       auto &bidfloorcur = boost::any_cast<boost::string_view &>(m["bidfloorcur"]);
       imp.bidfloorcur   = decltype(imp.bidfloorcur)(bidfloorcur.data(), bidfloorcur.size());
    } catch (const std::exception &e) {
       LOG(error) << "openrtb::Impression<T> extract exception " << e.what() ;
    }
    return imp;
}

template<typename T>
boost::optional<openrtb::User<T>> 
extractors<boost::optional<openrtb::User<T>>>::extract( boost::any & value ) {
    if ( value.empty() ) {
        return boost::optional<openrtb::User<T>>();
    }
    openrtb::User<T> user;
    auto &m  = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
    user.geo = extractors<decltype(user.geo)>::extract(m["geo"]);
    return user;
}

template<typename T>
boost::optional<openrtb::Geo<T>>
extractors<boost::optional<openrtb::Geo<T>>>::extract( boost::any & value ) {
    if ( value.empty() ) {
        return boost::optional<openrtb::Geo<T>>();
    }
    openrtb::Geo<T> geo;
    auto &m         = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
    auto &city      = boost::any_cast<boost::string_view &>(m["city"]);
    auto &country   = boost::any_cast<boost::string_view &>(m["country"]);
    geo.city       = decltype(geo.city)(city.data(), city.size());
    geo.country    = decltype(geo.country)(country.data(), country.size());
    return boost::make_optional(geo);
}


template<typename T>
boost::optional<openrtb::Site<T>> 
extractors<boost::optional<openrtb::Site<T>>>::extract( boost::any & value ) {
    if ( value.empty() ) {
        return boost::optional< openrtb::Site<T>>();
    }
    openrtb::Site<T> site;
    auto &m  = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
    auto &id = boost::any_cast<boost::string_view &>(m["id"]);
    site.id  = decltype(site.id)(id.data(), id.size()); 
    return boost::make_optional(site);
}

template<typename T>
boost::optional<openrtb::Banner<T>>
extractors<boost::optional<openrtb::Banner<T>>>::extract( boost::any & value ) {
    if ( value.empty() ) {
        return boost::optional<openrtb::Banner<T>>();
    }
    openrtb::Banner<T> banner ;
    auto &m = boost::any_cast<std::map<boost::string_view , boost::any> &>(value);
    try {
       //banner.h = boost::any_cast<decltype(banner.h)>(m["h"]);
       //banner.w = boost::any_cast<decltype(banner.w)>(m["w"]);
       banner.h   = boost::any_cast<int64_t>(m["h"]);
       banner.w   = boost::any_cast<int64_t>(m["w"]);
       banner.pos = static_cast<decltype(banner.pos)>(boost::any_cast<int64_t>(m["pos"]));
    } catch (const std::exception &e) {
       LOG(error) << "openrtb::Banner<T> extract exception " << e.what() ;
    }

    return boost::make_optional(banner);
}

////////////////////////// RAPID JSON EXTRACTORS //////////////////////////////////////////

template<typename T>
openrtb::BidRequest <T>
extractors<openrtb::BidRequest<T>>::extract(rapidjson::Document &d) {
openrtb::BidRequest <T> request;
if (d.IsNull() || !d.IsObject()) {
    return request;
}
try {
    for ( auto &m : d.GetObject() ) {
        if ( m.name.GetStringLength()==2 && !std::strcmp(m.name.GetString(), "id") ) {
            request.id   = decltype(request.id)(m.value.GetString(), m.value.GetStringLength());
        }
        else if (  !std::strcmp(m.name.GetString(),"imp") ) {
            request.imp  = extractors<decltype(request.imp)>::extract(m.value);
        }
        else if ( !std::strcmp(m.name.GetString(),"user") ) {
            request.user  = extractors<decltype(request.user)>::extract(m.value);
        }
        else if ( !std::strcmp(m.name.GetString(),"site") ) {
            request.site = extractors<decltype(request.site)>::extract(m.value);
        }
    }
//    auto &id = d["id"];
//    request.id   = decltype(request.id)(id.GetString(), id.GetStringLength());
//    request.imp  = extractors<decltype(request.imp)>::extract(d["imp"]);
//    request.user = extractors<decltype(request.user)>::extract(d["user"]);
//    request.site = extractors<decltype(request.site)>::extract(d["site"]);
} catch (const std::exception &e) {
    LOG(error) << "openrtb::BidRequest<T> extract exception " << e.what();
} catch (...) {
    LOG(error) << "openrtb::BidRequest<T> extract exception ";
}
return request;
}


//extract functions implementation inline keyword ?
template<typename T>
openrtb::Impression <T>
extractors<openrtb::Impression<T>>::extract( rapidjson::Value & value ) {
    openrtb::Impression <T> imp;
    if ( value.IsNull() || !value.IsObject()) {
        return imp;
    }
    try {
        for ( auto &m : value.GetObject()) {
            if ( !std::strcmp(m.name.GetString(),"id") ) {
                imp.id   = decltype(imp.id)(m.value.GetString(), m.value.GetStringLength());
            }
            else if ( !std::strcmp(m.name.GetString(),"banner") ) {
                imp.banner  = extractors<decltype(imp.banner)>::extract(m.value);
            }
            else if ( !std::strcmp(m.name.GetString(),"bidfloor") ) {
                imp.bidfloor  = m.value.GetDouble();
            }
            else if ( !std::strcmp(m.name.GetString(), "bidfloorcur") ) {
                imp.bidfloorcur = decltype(imp.bidfloorcur)(m.value.GetString(), m.value.GetStringLength());
            }
        }
//        auto m  = value.GetObject();
//        auto &id = m["id"];
//        imp.id = decltype(imp.id)(id.GetString(), id.GetStringLength());
//        imp.banner = extractors <decltype(imp.banner)>::extract(m["banner"]);
//        imp.bidfloor = m["bidfloor"].GetDouble();
//        auto &bidfloorcur = m["bidfloorcur"];
//        imp.bidfloorcur = decltype(imp.bidfloorcur)(bidfloorcur.GetString(), bidfloorcur.GetStringLength());
    } catch ( const std::exception &e) {
        LOG(error) << "openrtb::Impression<T> extract exception " << e.what();
    } catch (...) {
        LOG(error) << "openrtb::Impression<T> extract exception ";
    }
    return imp;
}

template<typename T>
boost::optional<openrtb::User <T>>
extractors<boost::optional<openrtb::User < T>>>::extract( rapidjson::Value & value ) {
    if ( value.IsNull() || !value.IsObject()) {
        return boost::optional<openrtb::User<T>>();
    }
    openrtb::User <T> user;
    auto m  = value.GetObject();
    user.geo = extractors<decltype(user.geo)>::extract(m["geo"]);
    return user;
}

template<typename T>
boost::optional<openrtb::Geo<T>>
extractors<boost::optional<openrtb::Geo<T>>>::extract( rapidjson::Value & value ) {
    if ( value.IsNull() || !value.IsObject()) {
        return boost::optional<openrtb::Geo<T>>();
    }
    openrtb::Geo<T> geo;
    auto m    = value.GetObject();
    auto &city = m["city"];
    auto &country = m["country"];
    geo.city = decltype(geo.city)(city.GetString(), city.GetStringLength());
    geo.country = decltype(geo.country)(country.GetString(), country.GetStringLength());
    return boost::make_optional(geo);
}


template<typename T>
boost::optional<openrtb::Site<T>>
extractors<boost::optional<openrtb::Site<T>>>::extract( rapidjson::Value & value ) {
    if ( value.IsNull() || !value.IsObject()) {
        return boost::optional<openrtb::Site <T>>();
    }
    openrtb::Site<T> site;
    auto m    = value.GetObject();
    auto &id = m["id"];
    auto &ref = m["ref"];
    site.id = decltype(site.id)(id.GetString(), id.GetStringLength());
    site.ref = decltype(site.ref)(ref.GetString(), ref.GetStringLength());
    return boost::make_optional(site);
}

template<typename T>
boost::optional<openrtb::Banner<T>>
extractors<boost::optional<openrtb::Banner<T>>>::extract( rapidjson::Value & value ) {
    if ( value.IsNull() || !value.IsObject()) {
        return boost::optional<openrtb::Banner<T>>();
    }
    openrtb::Banner<T> banner;
    try {
        auto m   = value.GetObject();
        banner.h = m["h"].GetInt();
        banner.w = m["w"].GetInt();
        banner.pos = static_cast<decltype(banner.pos)>(m["pos"].GetInt());
    } catch ( const std::exception &e) {
        LOG(error) << "openrtb::Banner<T> extract exception " << e.what();
    } catch (...) {
        LOG(error) << "openrtb::Banner<T> extract exception ";
    }

    return boost::make_optional(banner);
}


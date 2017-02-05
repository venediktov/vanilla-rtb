
#include "core/tagged_tuple.hpp"
#include "datacache/city_country_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include <chrono>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <random>
#include <memory>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/log/trivial.hpp>

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace po = boost::program_options;

struct CityCountry {
    std::string city;
    std::string country;
    std::string record;
    CityCountry(std::string city, std::string country) : city{std::move(city)}, country{std::move(country)} {}
    CityCountry() : city{}, country{} {}
    template<typename T>
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<T> & t)  {
        os <<  *t ;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  CityCountry & value)  {
        os << value.city << "|" << value.country << "|" ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, CityCountry &l) {
       if ( !std::getline(is, l.record) ){
          return is;
       }
       std::vector<std::string> fields;
       boost::split(fields, l.record, boost::is_any_of(","), boost::token_compress_on);
       l.city = fields.at(0); 
       l.country = fields.at(5);
       return is;
   }
};

template<typename Container>
auto random_pick(Container && c) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, c.size()-1);
  return dis(gen);
}

template<typename Keys, typename Cache>
void populate_cache(const std::vector<CityCountry> &cities , Cache && cache) {
    for ( auto &value : cities ) {
       if ( cache.insert(Keys{value.city, value.country}, value) ) {
         LOG(info) << "inserted{" << value.city << "," << value.country << "} OK!" ; 
       } else {
          LOG(error) << "insert {" << value.city << "," << value.country << "} FAILED!" ;
       }
    }
}

template<typename Stream>
struct perf_timer {
    perf_timer(std::shared_ptr<Stream> &osp) : begin{std::chrono::steady_clock::now()}, end{begin}, osp{osp} {}
    ~perf_timer() {
      end = std::chrono::steady_clock::now() ;
      *osp << "elapsed_ms=" 
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
         << "|";
      *osp << "elapsed_mu=" 
         << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() 
         << "|";
      *osp << "elapsed_ns=" 
         << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() 
         << "|"; 
    }
private:
  decltype(std::chrono::steady_clock::now()) begin;
  decltype(std::chrono::steady_clock::now()) end;
  std::shared_ptr<Stream> osp;
};

//Non-Intrusive boost serialization implementation
namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, CityCountry & value, const unsigned int version)
{
    ar & value.city;
    ar & value.country;
    ar & value.record;
}

} // namespace serialization
} // namespace boost


int main(int argc, char**argv) {

  using namespace vanilla;
  using Memory = mpclmi::ipc::Shared;
  using Cache = datacache::entity_cache<Memory, ipc::data::city_country_container> ; 
  using Alloc = typename Cache::char_allocator ;
  using Tag   = typename ipc::data::city_country_entity<Alloc>::unique_city_country_tag ;
  using Keys   = tagged_tuple
  <
     ipc::data::city_country_entity<Alloc>::city_tag,    std::string, 
     ipc::data::city_country_entity<Alloc>::country_tag, std::string
  > ;

  init_framework_logging("/tmp/openrtb_cache_test_log");

  Cache cache("test-vanilla-ipc") ;
  std::string file_name;
  long iter_count;
  po::options_description desc;
        desc.add_options()
            ("help", "produce help message")
            ("populate", "used to populate cache")
            ("iter_count", po::value<long>(&iter_count)->default_value(100), "how many iterations")
            ("city_source", po::value<std::string>(&file_name)->default_value("./world_cities.csv"), "city file")
        ;

  boost::program_options::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch ( const std::exception& ) {
    LOG(error) << desc;
    return 1;
  }

  if (vm.count("help")) {
      LOG(info) << desc;
      return 0;
  }

  std::vector<CityCountry> cities;
  if (vm.count("city_source")) {
     std::ifstream in {file_name};
     if ( !in ) {
        LOG(error) << "could not open file " << file_name << " exiting...";
        return 1;
     }
     std::copy ( std::istream_iterator<CityCountry>(in), std::istream_iterator<CityCountry>(), std::back_inserter(cities)) ;
     if (vm.count("populate")) {
         cache.clear();
         populate_cache<Keys>(cities,cache);
     }

  }

  for ( int i=0; i<iter_count; ++i) {
     auto pick = random_pick(cities);
     auto sp = std::make_shared<std::stringstream>() ;
     std::vector<std::shared_ptr<CityCountry>> retrieved_cached_cities;
     {
       perf_timer<std::stringstream> timer(sp) ;
       if ( !cache.retrieve<Tag>(retrieved_cached_cities, cache.create_ipc_key(cities[pick].city) , cache.create_ipc_key(cities[pick].country) ) ) 
       { 
          return 1; 
       }
     }
     std::for_each(std::begin(retrieved_cached_cities), std::end(retrieved_cached_cities), [](std::shared_ptr<CityCountry> &cp) {
       LOG(debug) << *cp; 
     });
     LOG(info) << sp->str() ;
  }
  
}



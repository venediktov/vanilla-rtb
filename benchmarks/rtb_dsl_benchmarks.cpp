/*
   Copyright 2017 Vladimir Lysyy (mrbald@github)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <benchmark/benchmark.h>

#include <rtb/DSL/generic_dsl.hpp>

namespace {
struct GenericDslBenchmarkFixture: benchmark::Fixture
{
    DSL::GenericDSL<jsonv::string_view> parser;
    std::string const input = R"(

{
  "id" : "9zj61whbdl319sjgz098lpys5cngmtro_short_false_false",
  "imp" : [ {
    "id" : "imp1",
    "native" : {
      "request" : "{\"ver\":\"1\",\"layout\":2,\"adunit\":4,\"plcmtcnt\":1,\"seq\":1}"
    },
    "bidfloor" : 100.0,
    "bidfloorcur" : "USD"
  } ],
  "app" : {
    "id" : "app1",
    "name" : "my-app-name",
    "domain" : "mysite.foo.com",
    "paid" : 1,
    "keywords" : "my,app,key,words"
  },
  "device" : {
    "ua" : "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36",
    "geo" : {
      "city" : "New York"
    },
    "dnt" : 0,
    "lmt" : 1,
    "ip" : "192.168.1.0",
    "ipv6" : "1:2:3:4:5:6:0:0"
  },
  "user" : {
    "id" : "user1",
    "buyeruid" : "buyer1",
    "gender" : "O",
    "keywords" : "user,builder,key,words",
    "geo" : {
      "city" : "New York"
    }
  }
}

)"; // std::string const input

}; // GenericDslBenchmarkFixture

BENCHMARK_DEFINE_F(GenericDslBenchmarkFixture, generic_dsl_extract_request_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(parser.extract_request(input));
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * input.size());
}

BENCHMARK_REGISTER_F(GenericDslBenchmarkFixture, generic_dsl_extract_request_benchmark);

} // local namespace

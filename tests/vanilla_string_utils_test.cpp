#include "rtb/common/string_utils.hpp"
#include <boost/test/unit_test.hpp>
namespace {

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(vanilla_string_utils)

BOOST_AUTO_TEST_CASE(string_utils_test,
                     *utf::description("String Concat test")) {
    BOOST_TEST_MESSAGE("Testing string_util");
    decltype(auto) actual_result = vanilla::common::string_concat(
        std::string_view("first="), std::to_string(123u),
        std::string_view("&second="), std::to_string(456u),
        std::string_view("&third="), std::to_string(789u));
    std::string expected_result{"first=123&second=456&third=789"};
    BOOST_CHECK_EQUAL(expected_result, actual_result); // "soft" assertion
    BOOST_REQUIRE_EQUAL(expected_result, actual_result);
} // BOOST_AUTO_TEST_CASE(string_utils_test)

BOOST_AUTO_TEST_SUITE_END(/* vanilla_string_utils */)

} // namespace
#include <boost/test/unit_test.hpp>
#include <stdexcept>

namespace {

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(vanilla_all)

BOOST_AUTO_TEST_CASE(green_test, *utf::description("good passing test")) {
    BOOST_TEST_MESSAGE("I am a good test, I am passing");
    BOOST_REQUIRE_THROW(throw std::runtime_error{"indentionally thrown"}, std::runtime_error);
    BOOST_REQUIRE_EQUAL(42, 42);
} // BOOST_AUTO_TEST_CASE(green_test)

BOOST_AUTO_TEST_CASE(red_test,
        *utf::disabled()
        *utf::description("requires network")) {
    BOOST_TEST_MESSAGE("I am a bad test, I am failing");
    BOOST_CHECK_EQUAL(42, 43); // "soft" assertion
    BOOST_REQUIRE_EQUAL(43, 44);
} // BOOST_AUTO_TEST_CASE(red_test)

BOOST_AUTO_TEST_SUITE_END(/* vanilla_all */)

} // local namespace

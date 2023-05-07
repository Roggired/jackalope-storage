#include <boost/test/included/unit_test.hpp>
#include "example/example.hpp"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(test_example)

BOOST_AUTO_TEST_CASE(sum_test) {
    int32_t result = sum(6, 4);
    BOOST_CHECK_EQUAL(result, 10);
}

BOOST_AUTO_TEST_SUITE_END()

test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
    framework::master_test_suite().p_name.value = "test_example_unit_test";
    return nullptr;
}

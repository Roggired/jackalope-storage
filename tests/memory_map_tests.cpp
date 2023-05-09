//
// Created by ego on 09.05.23.
//

#include <boost/test/included/unit_test.hpp>
#include "memory/memory.hpp"

using namespace boost::unit_test;
using namespace memory;
using namespace memoryMapKey;


test_suite *init_unit_test_suite(int /*argc*/, char * /*argv*/[]) {
    framework::master_test_suite().p_name.value = "memory_map_unit_test";
    return nullptr;
}


BOOST_AUTO_TEST_SUITE(memory_map_test_suite)

    BOOST_AUTO_TEST_CASE(memory_map_key_test) {
        models::PID pid(10, 20, 0);
        Key key = keyByPid(pid);
        int64_t expected = 0x0000000A00000014;
        BOOST_CHECK_EQUAL(expected, key);

        int32_t fileNumber = fileNumberByKey(key);
        BOOST_CHECK_EQUAL(10, fileNumber);

        int32_t pageNumber = pageNumberByKey(key);
        BOOST_CHECK_EQUAL(20, pageNumber);
    }

BOOST_AUTO_TEST_SUITE_END()
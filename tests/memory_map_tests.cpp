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

    BOOST_AUTO_TEST_CASE(memory_map_create_page_and_get_it_test) {
        FileHeader fileHeader = {
                1,
                FILE_HEADER_PAGE_8_TYPE,
                {0},
                0
        };

        MemoryMapsConfig memoryMapsConfig = {
                128*1024*1024,
                16*1024*1024
        };
        MemoryMap* memoryMap = initLocalMemoryMap(memoryMapsConfig);

        Page& createdPage = memoryMap->createPage8(fileHeader, PAGE_TYPE_SENSE);
        models::PID pid(1, 0, 0);
        Page& fetchedPage = memoryMap->getPageByKey(keyByPid(pid));

        BOOST_CHECK_EQUAL(createdPage.getFileNumber(), fetchedPage.getFileNumber());
        BOOST_CHECK_EQUAL(createdPage.getPageNumber(), fetchedPage.getPageNumber());
        BOOST_CHECK_EQUAL(createdPage.getFreeSpace(), fetchedPage.getFreeSpace());
        BOOST_CHECK_EQUAL(createdPage.getType(), fetchedPage.getType());

        destroyLocalMemoryMap(memoryMap);
    }

    BOOST_AUTO_TEST_CASE(memory_map_create_page_and_unload_test) {
        FileHeader fileHeader = {
                1,
                FILE_HEADER_PAGE_8_TYPE,
                {0},
                0
        };

        MemoryMapsConfig memoryMapsConfig = {
                128*1024*1024,
                16*1024*1024
        };
        MemoryMap* memoryMap = initLocalMemoryMap(memoryMapsConfig);

        Page& createdPage = memoryMap->createPage8(fileHeader, PAGE_TYPE_SENSE);
        models::PID pid(1, 0, 0);
        memoryMap->unloadPage(keyByPid(pid));

        destroyLocalMemoryMap(memoryMap);
    }

    BOOST_AUTO_TEST_CASE(memory_map_get_not_existed_page_test) {
        MemoryMapsConfig memoryMapsConfig = {
                128*1024*1024,
                16*1024*1024
        };
        MemoryMap* memoryMap = initLocalMemoryMap(memoryMapsConfig);
        models::PID pid(1, 0, 0);

        BOOST_CHECK_THROW(memoryMap->getPageByKey(keyByPid(pid)), NoSuchPageException);
    }

    BOOST_AUTO_TEST_CASE(memory_map_create_page_and_memory_limit_test) {
        FileHeader fileHeader = {
                1,
                FILE_HEADER_PAGE_8_TYPE,
                {0},
                0
        };

        MemoryMapsConfig memoryMapsConfig = {
                128*1024*1024,
                1*1024 // less than 1KB
        };
        MemoryMap* memoryMap = initLocalMemoryMap(memoryMapsConfig);

        BOOST_CHECK_THROW(memoryMap->createPage8(fileHeader, PAGE_TYPE_SENSE), MemoryLimitException);
    }

    BOOST_AUTO_TEST_CASE(memory_map_create_page_throws_on_already_existed_page_test) {
        FileHeader fileHeader = {
                1,
                FILE_HEADER_PAGE_8_TYPE,
                {0},
                0
        };

        MemoryMapsConfig memoryMapsConfig = {
                128*1024*1024,
                16*1024*1024
        };
        MemoryMap* memoryMap = initLocalMemoryMap(memoryMapsConfig);
        memoryMap->createPage8(fileHeader, PAGE_TYPE_SENSE);

        BOOST_CHECK_THROW(memoryMap->createPage8(fileHeader, PAGE_TYPE_SENSE), FileHeaderInvalidPagesNumberException);
    }

BOOST_AUTO_TEST_SUITE_END()
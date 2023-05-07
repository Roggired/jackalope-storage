#include <boost/test/included/unit_test.hpp>
#include <cstdint>
#include "memory/memory.hpp"
//#include "model/semnet_models.hpp"

using namespace boost::unit_test;
using namespace memory;


test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
    framework::master_test_suite().p_name.value = "page_unit_test";
    return nullptr;
}

#define TEST_STRUCT_ARRAY_SIZE 128
struct TestStruct {
    int32_t a{0};
    int32_t b{0};
    int32_t array[TEST_STRUCT_ARRAY_SIZE]{};
};
typedef struct TestStruct TestStruct;


BOOST_AUTO_TEST_SUITE(pointer_tests)

BOOST_AUTO_TEST_CASE(pointer_xmin_committed_test) {
    PagePointer pointer = unusedPagePointer();
    pointer.flags = 0x01;
    BOOST_CHECK_EQUAL(true, pointer.isXminCommitted());
}

BOOST_AUTO_TEST_CASE(pointer_xmin_aborted_test) {
    PagePointer pointer = unusedPagePointer();
    pointer.flags = 0x02;
    BOOST_CHECK_EQUAL(true, pointer.isXminAborted());
}

BOOST_AUTO_TEST_CASE(pointer_xmax_committed_test) {
    PagePointer pointer = unusedPagePointer();
    pointer.flags = 0x04;
    BOOST_CHECK_EQUAL(true, pointer.isXmaxCommitted());
}

BOOST_AUTO_TEST_CASE(pointer_xmax_aborted_test) {
    PagePointer pointer = unusedPagePointer();
    pointer.flags = 0x08;
    BOOST_CHECK_EQUAL(true, pointer.isXmaxAborted());
}

BOOST_AUTO_TEST_CASE(init_unused_page_pointer_test) {
    PagePointer pointer = unusedPagePointer();
    BOOST_CHECK_EQUAL(POINTER_STATUS_UNUSED, pointer.status);
}

BOOST_AUTO_TEST_CASE(page_8_size_test) {
    Page8 page8 = Page8(0, 0, PAGE_TYPE_SENSE);
    BOOST_CHECK_EQUAL(8*1024, sizeof(page8));
}

BOOST_AUTO_TEST_CASE(page_pointer_size_test) {
    PagePointer pointer = unusedPagePointer();
    BOOST_CHECK_EQUAL(16, sizeof(pointer));
}

BOOST_AUTO_TEST_CASE(page_simple_get_put_test) {
    TestStruct testStruct;
    testStruct.a = 15;
    testStruct.b = 1278;
    for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; ++i) {
        testStruct.array[i] = (int32_t) i;
    }
    RowHeader rowHeader = {
           sizeof(testStruct),
           21,
           22
    };

    std::cout << sizeof(Page8) << std::endl;

    auto page = new Page8(0, 10, PAGE_TYPE_SENSE);
    PagePut pagePut = page->put({&rowHeader, &testStruct});
    BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);
    BOOST_CHECK_EQUAL(0, pagePut.pid.getFileNumber());
    BOOST_CHECK_EQUAL(10, pagePut.pid.getPageNumber());
    BOOST_CHECK_EQUAL(0, pagePut.pid.getRowPointerNumber());

    PageGet pageGet = page->getByPid(pagePut.pid);
    BOOST_CHECK_EQUAL(PGS_OK, pageGet.status);

    RowHeader* resultRowHeader = pageGet.row.rowHeader;
    auto resultTestStruct = (TestStruct*) pageGet.row.payload;
    BOOST_CHECK_EQUAL(sizeof(TestStruct), resultRowHeader->payloadSize);
    BOOST_CHECK_EQUAL(21, resultRowHeader->extIdFile);
    BOOST_CHECK_EQUAL(22, resultRowHeader->extIdPointer);
    BOOST_CHECK_EQUAL(15, resultTestStruct->a);
    BOOST_CHECK_EQUAL(1278, resultTestStruct->b);
    int32_t* resultArray = resultTestStruct->array;
    for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; ++i) {
        BOOST_CHECK_EQUAL((int32_t) i, resultArray[i]);
    }

    delete page;
}

BOOST_AUTO_TEST_SUITE_END()

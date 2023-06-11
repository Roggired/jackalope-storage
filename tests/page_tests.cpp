#include <boost/test/included/unit_test.hpp>
#include "memory/memory.hpp"

using namespace boost::unit_test;
using namespace memory;


test_suite *init_unit_test_suite(int /*argc*/, char * /*argv*/[]) {
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

TestStruct init_test_struct() {
    TestStruct testStruct;
    testStruct.a = 15;
    testStruct.b = 1278;
    for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; ++i) {
        testStruct.array[i] = (int32_t) i;
    }
    return testStruct;
}

RowHeader init_row_header() {
    return {
            sizeof(TestStruct),
            21,
            22
    };
}

uint16_t placePagePointerAndRowWithTestStructInPageContent(
        void *content,
        uint16_t previousRowPositionOffset,
        uint16_t pointerNumber,
        uint32_t pointerXmin,
        uint32_t pointerXmax,
        bool pointerXminCommitted,
        bool pointerXminAborted,
        bool pointerXmaxCommitted,
        bool pointerXmaxAborted,
        uint16_t gapBetweenRows
) {
    TestStruct testStruct = init_test_struct();
    RowHeader rowHeader = init_row_header();
    auto pagePointer = PagePointer{
            POINTER_STATUS_USED,
            0,
            (uint16_t) (previousRowPositionOffset - sizeof(RowHeader) - sizeof(TestStruct) - gapBetweenRows),
            pointerXmin,
            pointerXmax,
            sizeof(RowHeader) + sizeof(TestStruct),
            pointerNumber
    };
    if (pointerXminCommitted) pagePointer.setXminCommited();
    if (pointerXminAborted) pagePointer.setXminAborted();
    if (pointerXmaxCommitted) pagePointer.setXmaxCommited();
    if (pointerXmaxAborted) pagePointer.setXmaxAborted();
    std::copy(
            (int8_t *) &rowHeader,
            (int8_t *) &rowHeader + sizeof(RowHeader),
            (int8_t *) content + pagePointer.rowPositionOffset
    );
    std::copy(
            (int8_t *) &testStruct,
            (int8_t *) &testStruct + sizeof(TestStruct),
            (int8_t *) content + pagePointer.rowPositionOffset + sizeof(RowHeader)
    );
    std::copy(
            (int8_t *) &pagePointer,
            (int8_t *) &pagePointer + sizeof(PagePointer),
            (int8_t *) content + sizeof(PagePointer) * pointerNumber
    );

    return pagePointer.rowPositionOffset;
}


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

    BOOST_AUTO_TEST_CASE(page_type_test) {
        auto page = new Page(0, 10, PAGE_TYPE_SENSE);
        BOOST_CHECK_EQUAL(true, page->isTypeSense());
        BOOST_CHECK_EQUAL(false, page->isTypeInstance());
        BOOST_CHECK_EQUAL(false, page->isTypeReference());
        BOOST_CHECK_EQUAL(false, page->isTypeWordform());
        delete page;

        page = new Page(0, 10, PAGE_TYPE_INSTANCE);
        BOOST_CHECK_EQUAL(false, page->isTypeSense());
        BOOST_CHECK_EQUAL(true, page->isTypeInstance());
        BOOST_CHECK_EQUAL(false, page->isTypeReference());
        BOOST_CHECK_EQUAL(false, page->isTypeWordform());
        delete page;

        page = new Page(0, 10, PAGE_TYPE_REFERENCE);
        BOOST_CHECK_EQUAL(false, page->isTypeSense());
        BOOST_CHECK_EQUAL(false, page->isTypeInstance());
        BOOST_CHECK_EQUAL(true, page->isTypeReference());
        BOOST_CHECK_EQUAL(false, page->isTypeWordform());
        delete page;

        page = new Page(0, 10, PAGE_TYPE_WORDFORM);
        BOOST_CHECK_EQUAL(false, page->isTypeSense());
        BOOST_CHECK_EQUAL(false, page->isTypeInstance());
        BOOST_CHECK_EQUAL(false, page->isTypeReference());
        BOOST_CHECK_EQUAL(true, page->isTypeWordform());
        delete page;
    }

    BOOST_AUTO_TEST_CASE(page_8_size_test) {
        Page page8 = Page(0, 0, PAGE_TYPE_SENSE);
        BOOST_CHECK_EQUAL(8 * 1024, sizeof(page8));
    }

    BOOST_AUTO_TEST_CASE(page_pointer_size_test) {
        PagePointer pointer = unusedPagePointer();
        BOOST_CHECK_EQUAL(16, sizeof(pointer));
    }

    BOOST_AUTO_TEST_CASE(page_simple_get_put_test) {
        // TestStruct size - 520
        TestStruct testStruct = init_test_struct();
        // Row size - 528
        RowHeader rowHeader = init_row_header();

        auto page = new Page(0, 10, PAGE_TYPE_SENSE);
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);
        BOOST_CHECK_EQUAL(0, pagePut.pid.getFileNumber());
        BOOST_CHECK_EQUAL(10, pagePut.pid.getPageNumber());
        BOOST_CHECK_EQUAL(0, pagePut.pid.getRowPointerNumber());

        page->setCommitted(2, pagePut.pid);

        PageGet pageGet = page->getByPid(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_OK, pageGet.status);

        RowHeader *resultRowHeader = pageGet.row.rowHeader;
        auto resultTestStruct = (TestStruct *) pageGet.row.payload;
        BOOST_CHECK_EQUAL(sizeof(TestStruct), resultRowHeader->payloadSize);
        BOOST_CHECK_EQUAL(21, resultRowHeader->extIdFile);
        BOOST_CHECK_EQUAL(22, resultRowHeader->extIdPointer);
        BOOST_CHECK_EQUAL(15, resultTestStruct->a);
        BOOST_CHECK_EQUAL(1278, resultTestStruct->b);
        int32_t *resultArray = resultTestStruct->array;
        for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; ++i) {
            BOOST_CHECK_EQUAL((int32_t) i, resultArray[i]);
        }

        pageGet = page->getByPid(1, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_NOT_VISIBLE, pageGet.status);

        delete page;
    }

    BOOST_AUTO_TEST_CASE(page_multiple_put_single_get_same_transaction_test) {
        auto page = new Page(0, 10, PAGE_TYPE_SENSE);

        models::PID targetPID(0, 0, 0);
        size_t amountOfRows = 15;
        for (size_t i = 0; i < amountOfRows; i++) {
            TestStruct testStruct = init_test_struct();
            RowHeader rowHeader = init_row_header();
            PagePut pagePut = page->put(1, {&rowHeader, &testStruct});
            BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

            if (i == amountOfRows / 2) {
                targetPID = pagePut.pid;
            }
        }

        PageGet pageGet = page->getByPid(1, targetPID);
        BOOST_CHECK_EQUAL(PGS_OK, pageGet.status);
        RowHeader *resultRowHeader = pageGet.row.rowHeader;
        auto resultTestStruct = (TestStruct *) pageGet.row.payload;
        BOOST_CHECK_EQUAL(sizeof(TestStruct), resultRowHeader->payloadSize);
        BOOST_CHECK_EQUAL(21, resultRowHeader->extIdFile);
        BOOST_CHECK_EQUAL(22, resultRowHeader->extIdPointer);
        BOOST_CHECK_EQUAL(15, resultTestStruct->a);
        BOOST_CHECK_EQUAL(1278, resultTestStruct->b);
        int32_t *resultArray = resultTestStruct->array;
        for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; ++i) {
            BOOST_CHECK_EQUAL((int32_t) i, resultArray[i]);
        }

        delete page;
    }

    BOOST_AUTO_TEST_CASE(page_put_not_enough_space_test) {
        auto page = new Page(0, 10, PAGE_TYPE_SENSE);

        size_t amountOfRows = 15;
        for (size_t i = 0; i < amountOfRows; i++) {
            TestStruct testStruct = init_test_struct();
            RowHeader rowHeader = init_row_header();
            // single claim size == 528 (row) + 16 (pointer) == 544
            PagePut pagePut = page->put(1, {&rowHeader, &testStruct});
            BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);
        }

        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        // single claim size == 528 (row) + 16 (pointer) == 544
        PagePut pagePut = page->put(1, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_NOT_ENOUGH_SPACE, pagePut.status);
    }

    BOOST_AUTO_TEST_CASE(page_get_invalid_pid_test) {
        auto page = new Page(0, 10, PAGE_TYPE_SENSE);
        PageGet pageGet = page->getByPid(1, models::PID(1, 10, 0));
        BOOST_CHECK_EQUAL(PGS_INVALID_PID, pageGet.status);
        pageGet = page->getByPid(1, models::PID(0, 9, 0));
        BOOST_CHECK_EQUAL(PGS_INVALID_PID, pageGet.status);
        pageGet = page->getByPid(1, models::PID(0, 10, 0));
        BOOST_CHECK_EQUAL(PGS_INVALID_PID, pageGet.status);
    }

    BOOST_AUTO_TEST_CASE(page_vacuum_two_pointers_test1) {
        // first pointer USED and is deleted in the previously committed transaction which ID is before eventHorizon
        // second pointer USED and has xmax == 0(a)
        void *content = (void *) (new int8_t[PAGE_8_CONTENT_SIZE]{0});
        uint16_t currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                PAGE_8_CONTENT_SIZE,
                0,
                1,
                2,
                true,
                false,
                true,
                false,
                456
        );
        currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                currentRowsOffset,
                1,
                3,
                0,
                true,
                false,
                false,
                true,
                1045
        );

        auto page = new Page(0, 1, PAGE_TYPE_SENSE, (int8_t *) content);
        auto *ptrToPointerOffset = (uint16_t *) ((int8_t *) page + 8);
        auto *ptrToRowsOffset = ptrToPointerOffset + 1;
        *ptrToPointerOffset = sizeof(PagePointer) * 2;
        *ptrToRowsOffset = currentRowsOffset;

        uint16_t vacuumedPointersNumber = page->vacuum(3);
        BOOST_CHECK_EQUAL(1, vacuumedPointersNumber);

        PageGet pageGetByPointerNumber1 = page->getByPid(3, models::PID(0, 1, 0));
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_UNUSED, pageGetByPointerNumber1.status);

        PageGet pageGetByPointerNumber2 = page->getByPid(3, models::PID(0, 1, 1));
        BOOST_CHECK_EQUAL(PGS_OK, pageGetByPointerNumber2.status);

        RowHeader *resultRowHeader = pageGetByPointerNumber2.row.rowHeader;
        auto *resultTestStruct = (TestStruct *) pageGetByPointerNumber2.row.payload;
        RowHeader expectedRowHeader = init_row_header();
        TestStruct expectedTestStruct = init_test_struct();
        BOOST_CHECK_EQUAL(resultRowHeader->payloadSize, expectedRowHeader.payloadSize);
        BOOST_CHECK_EQUAL(resultRowHeader->extIdPointer, expectedRowHeader.extIdPointer);
        BOOST_CHECK_EQUAL(resultRowHeader->extIdFile, expectedRowHeader.extIdFile);
        BOOST_CHECK_EQUAL(resultTestStruct->a, expectedTestStruct.a);
        BOOST_CHECK_EQUAL(resultTestStruct->b, expectedTestStruct.b);
        for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; i++) {
            BOOST_CHECK_EQUAL(resultTestStruct->array[i], expectedTestStruct.array[i]);
        }
        delete page;

        delete[] (int8_t *) content;
    }

    BOOST_AUTO_TEST_CASE(page_vacuum_two_pointers_test2) {
        // first pointer USED and has xmax == 0(a)
        // second pointer USED and is deleted in the previously committed transaction which ID is before eventHorizon
        void *content = (void *) (new int8_t[PAGE_8_CONTENT_SIZE]{0});
        uint16_t currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                PAGE_8_CONTENT_SIZE,
                0,
                3,
                0,
                true,
                false,
                false,
                true,
                456
        );
        currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                currentRowsOffset,
                1,
                1,
                2,
                true,
                false,
                true,
                false,
                1045
        );

        auto page = new Page(0, 1, PAGE_TYPE_SENSE, (int8_t *) content);
        auto *ptrToPointerOffset = (uint16_t *) ((int8_t *) page + 8);
        auto *ptrToRowsOffset = ptrToPointerOffset + 1;
        *ptrToPointerOffset = sizeof(PagePointer) * 2;
        *ptrToRowsOffset = currentRowsOffset;

        uint16_t vacuumedPointersNumber = page->vacuum(3);
        BOOST_CHECK_EQUAL(1, vacuumedPointersNumber);

        PageGet pageGetByPointerNumber1 = page->getByPid(3, models::PID(0, 1, 0));
        BOOST_CHECK_EQUAL(PGS_OK, pageGetByPointerNumber1.status);
        RowHeader *resultRowHeader = pageGetByPointerNumber1.row.rowHeader;
        auto *resultTestStruct = (TestStruct *) pageGetByPointerNumber1.row.payload;
        RowHeader expectedRowHeader = init_row_header();
        TestStruct expectedTestStruct = init_test_struct();
        BOOST_CHECK_EQUAL(resultRowHeader->payloadSize, expectedRowHeader.payloadSize);
        BOOST_CHECK_EQUAL(resultRowHeader->extIdPointer, expectedRowHeader.extIdPointer);
        BOOST_CHECK_EQUAL(resultRowHeader->extIdFile, expectedRowHeader.extIdFile);
        BOOST_CHECK_EQUAL(resultTestStruct->a, expectedTestStruct.a);
        BOOST_CHECK_EQUAL(resultTestStruct->b, expectedTestStruct.b);
        for (size_t i = 0; i < TEST_STRUCT_ARRAY_SIZE; i++) {
            BOOST_CHECK_EQUAL(resultTestStruct->array[i], expectedTestStruct.array[i]);
        }

        PageGet pageGetByPointerNumber2 = page->getByPid(3, models::PID(0, 1, 1));
        BOOST_CHECK_EQUAL(PGS_INVALID_PID, pageGetByPointerNumber2.status);
        delete page;

        delete[] (int8_t *) content;
    }

    BOOST_AUTO_TEST_CASE(page_vacuum_two_pointers_test3) {
        // first pointer USED and is deleted in the previously committed transaction which ID is before eventHorizon
        // second pointer USED and is deleted in the previously committed transaction which ID is before eventHorizon
        void *content = (void *) (new int8_t[PAGE_8_CONTENT_SIZE]{0});
        uint16_t currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                PAGE_8_CONTENT_SIZE,
                0,
                1,
                2,
                true,
                false,
                true,
                false,
                456
        );
        currentRowsOffset = placePagePointerAndRowWithTestStructInPageContent(
                content,
                currentRowsOffset,
                1,
                1,
                2,
                true,
                false,
                true,
                false,
                1045
        );

        auto page = new Page(0, 1, PAGE_TYPE_SENSE, (int8_t *) content);
        auto *ptrToPointerOffset = (uint16_t *) ((int8_t *) page + 8);
        auto *ptrToRowsOffset = ptrToPointerOffset + 1;
        *ptrToPointerOffset = sizeof(PagePointer) * 2;
        *ptrToRowsOffset = currentRowsOffset;

        uint16_t vacuumedPointersNumber = page->vacuum(3);
        BOOST_CHECK_EQUAL(2, vacuumedPointersNumber);

        PageGet pageGetByPointerNumber1 = page->getByPid(3, models::PID(0, 1, 0));
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_UNUSED, pageGetByPointerNumber1.status);

        PageGet pageGetByPointerNumber2 = page->getByPid(3, models::PID(0, 1, 1));
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_UNUSED, pageGetByPointerNumber2.status);
        delete page;

        delete[] (int8_t *) content;
    }

    BOOST_AUTO_TEST_CASE(page_remove_test) {
        auto *page = new Page(0, 10, PAGE_TYPE_SENSE);
        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

        page->setCommitted(2, pagePut.pid);

        PageRemove pageRemove = page->remove(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_OK, pageRemove.status);

        page->setCommitted(3, pagePut.pid);

        pageRemove = page->remove(4, models::PID(0, 10, 123));
        BOOST_CHECK_EQUAL(PRS_INVALID_PID, pageRemove.status);

        pageRemove = page->remove(4, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_POINTER_IS_NOT_VISIBLE, pageRemove.status);

        pageRemove = page->remove(1, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_POINTER_IS_NOT_VISIBLE, pageRemove.status);
    }

    BOOST_AUTO_TEST_CASE(page_put_remove_get_test_1) {
        // put - committed
        // remove - committed
        // get - not visible
        auto *page = new Page(0, 10, PAGE_TYPE_SENSE);
        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

        page->setCommitted(2, pagePut.pid);

        PageRemove pageRemove = page->remove(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_OK, pageRemove.status);

        page->setCommitted(3, pagePut.pid);

        PageGet pageGet = page->getByPid(1, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_NOT_VISIBLE, pageGet.status);

        pageGet = page->getByPid(4, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_NOT_VISIBLE, pageGet.status);
    }

    BOOST_AUTO_TEST_CASE(page_put_remove_get_test_2) {
        // put - committed
        // remove - aborted
        // get - ok
        auto *page = new Page(0, 10, PAGE_TYPE_SENSE);
        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

        page->setCommitted(2, pagePut.pid);

        PageRemove pageRemove = page->remove(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_OK, pageRemove.status);

        page->setAborted(3, pagePut.pid);

        PageGet pageGet = page->getByPid(4, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_OK, pageGet.status);
    }

    BOOST_AUTO_TEST_CASE(page_put_get_single_transaction_test) {
        auto *page = new Page(0, 10, PAGE_TYPE_SENSE);
        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

        PageGet pageGet = page->getByPid(2, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_OK, pageGet.status);
    }

    BOOST_AUTO_TEST_CASE(page_remove_get_single_transaction_test) {
        auto *page = new Page(0, 10, PAGE_TYPE_SENSE);
        TestStruct testStruct = init_test_struct();
        RowHeader rowHeader = init_row_header();
        PagePut pagePut = page->put(2, {&rowHeader, &testStruct});
        BOOST_CHECK_EQUAL(PPS_OK, pagePut.status);

        page->setCommitted(2, pagePut.pid);

        PageRemove pageRemove = page->remove(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PRS_OK, pageRemove.status);

        PageGet pageGet = page->getByPid(3, pagePut.pid);
        BOOST_CHECK_EQUAL(PGS_POINTER_IS_NOT_VISIBLE, pageGet.status);
    }

BOOST_AUTO_TEST_SUITE_END()

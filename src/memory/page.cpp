//
// Created by roggired on 07.05.23.
//

#include "algorithm"
#include "page.hpp"

using namespace memory;


// PUBLIC SECTION

[[maybe_unused]]
PageGet Page8::getByPid(models::PID pid) {
    if (!isRequestedPidValid(pid)) {
        return emptyPageGet(PGS_INVALID_PID);
    }

    PagePointer* pointer = getPointerByOffset(calcPointerOffsetForPid(pid));
    if (pointer->isStatusUnused()) {
        return emptyPageGet(PGS_POINTER_IS_UNUSED);
    }

    void* undefined = getUndefinedByOffset(pointer->rowPositionOffset);
    void* payload = (void*) ((int8_t*) undefined + sizeof(RowHeader));
    auto rowHeader = (RowHeader*) undefined;

    return {
        PGS_OK,
        {
            rowHeader,
            payload
        }
    };
}

[[maybe_unused]]
PagePut Page8::put(Row row) {
    int16_t rowSize = row.sizeWithHeader();
    if (rowSize > getFreeSpace()) {
        return notEnoughSpacePagePut();
    }

    PagePointer* pointer = getPointerByOffset(pointerOffset);

    pointer->status = POINTER_STATUS_USED;
    rowsOffset = (int16_t) (rowsOffset - rowSize);
    pointer->rowPositionOffset = rowsOffset;
    pointer->size = (int16_t) row.sizeWithHeader();
    pointer->number = (int16_t) (pointerOffset / sizeof(PagePointer));

    auto copyStart = (int8_t*) row.rowHeader;
    int8_t* copyEnds = (int8_t*) row.rowHeader + sizeof(RowHeader);
    int8_t* copyTo = (int8_t*) content + pointer->rowPositionOffset;
    std::copy(copyStart, copyEnds, copyTo);

    copyStart = (int8_t*) row.payload;
    copyEnds = (int8_t*) row.payload + row.rowHeader->payloadSize;
    copyTo += sizeof(RowHeader);
    std::copy(copyStart, copyEnds, copyTo);

    pointerOffset += sizeof(PagePointer);

    return {
        PPS_OK,
        pidForPointer(pointer)
    };
}

// PRIVATE SECTION

models::PID Page8::pidForPointer(PagePointer* pagePointer) const {
    return {fileNumber, pageNumber, pagePointer->number};
}

void* Page8::getUndefinedByOffset(int16_t offset) {
    return (void*) (content + offset);
}

int16_t Page8::calcPointerOffsetForPid(models::PID requestedPid) {
    return (int16_t) (requestedPid.getRowPointerNumber() * sizeof(PagePointer));
}

bool Page8::isRequestedPidValid(models::PID requestedPid) const {
    bool fileCorrect = fileNumber == requestedPid.getFileNumber();
    bool pageCorrect = pageNumber == requestedPid.getPageNumber();

    auto requestedPointerOffset = calcPointerOffsetForPid(requestedPid);
    bool pointerCorrect = requestedPointerOffset >= 0 && requestedPointerOffset < pointerOffset;

    return fileCorrect && pageCorrect && pointerCorrect;
}

PagePointer* Page8::getPointerByOffset(int16_t offset) {
    return (PagePointer*) (void *) (content + offset);
}

PageGet memory::emptyPageGet(PageGetStatus status) {
    Row row = {nullptr,nullptr};
    return { status, row };
}

PagePut memory::notEnoughSpacePagePut() {
    return {
            memory::PPS_NOT_ENOUGH_SPACE,
            models::PID(0, 0, 0)
    };
}
//
// Created by ego on 07.05.23.
//

#include <list>
#include "algorithm"
#include "page.hpp"

using namespace memory;


// ==================================== PUBLIC SECTION =============================================================

[[maybe_unused]]
int16_t Page8::vacuum(uint32_t eventHorizon) {
    std::list<PagePointer*> pointers;
    int16_t vacuumedPointersNumber = 0;

    iterateOverPointers([&pointers](PagePointer *pointer) { pointers.push_back(pointer); } );

    // we need to be sure that pointers is sorted in the descending order by pointer's rowPositionOffset, because
    // we will pack associated rows without using additional memory. If the pointers is not sorted,
    // we can corrupt memory.
    pointers.sort(
            [](PagePointer* a, PagePointer* b) {
                return a->rowPositionOffset > b->rowPositionOffset;
            }
    );

    int16_t localRowsOffset = PAGE_8_CONTENT_SIZE;
    for (auto pointer: pointers) {
        if (!pointer->isStatusUsed()) {
            continue;
        }

        // xmax > 0 means that pointer which is not yet deleted in any transaction should not be vacuumed.
        // xmax < eventHorizon is used to determine pointers which has been deleted in a committed transaction with ID
        // and that ID is less (in the past) than minimum ID of active transactions.
        if (pointer->xmax < eventHorizon && pointer->isXmaxCommitted() && pointer->xmax > 0) {
            vacuumedPointersNumber++;
            pointer->status = POINTER_STATUS_UNUSED;
            continue;
        }

        localRowsOffset = (int16_t) (localRowsOffset - pointer->size);

        if (localRowsOffset > pointer->rowPositionOffset) {
            // seems like pointer.size is many times less than PAGE_8_CONTENT_SIZE
            auto localRow = new int8_t[pointer->size];
            // this copy operations cannot corrupt data, because we iterate over rowPositionOffsets from the content end
            // to content start and each time we move data only 'to the end' of the memory region.
            std::copy(
                    (int8_t *) content + pointer->rowPositionOffset,
                    (int8_t *) content + pointer->rowPositionOffset + pointer->size,
                    localRow
            );
            std::copy(
                    localRow,
                    localRow + pointer->size,
                    (int8_t *) content + localRowsOffset
            );
            delete[] localRow;
        }

        pointer->rowPositionOffset = localRowsOffset;
    }

    rowsOffset = localRowsOffset;

    // iterate over the pointers section from pointerOffset to 0 and find the new pointerOffset based on
    // pointer's statuses. This cycle removes  previously allocated, currently unused pointers after the very last
    // used pointer. Look at picture below:
    // 0 | 16 | 32 | 48 | 64 | 80 | 96 | 112 | 128 | 144 | - byte number in the page content
    // 1 | *  | *  | 1  | 1  |  * | 1  | 0   | 0   | 0   | - pointers section in the page content
    // 1 - used pointer
    // * - unused and not deallocated pointer
    // 0 - unused and deallocated pointer
    auto localPointerOffset = (int16_t) (pointerOffset - sizeof(PagePointer));
    auto* pagePointer = getPointerByOffset(localPointerOffset);
    while (pagePointer->status == POINTER_STATUS_UNUSED && localPointerOffset >= 0) {
        localPointerOffset = (int16_t) (localPointerOffset - sizeof(PagePointer));
        pagePointer = getPointerByOffset(localPointerOffset);
    }

    pointerOffset = (int16_t) (localPointerOffset + sizeof(PagePointer));

    return vacuumedPointersNumber;
}

[[maybe_unused]]
PageGet Page8::getByPid(uint32_t xid, models::PID pid) const {
    if (!isPidValid(pid)) {
        return emptyPageGet(PGS_INVALID_PID);
    }

    int16_t targetPointerOffset = calcPointerOffsetByPid(pid);
    PagePointer *pointer = getPointerByOffset(targetPointerOffset);

    if (!pointer->isStatusUsed()) {
        return emptyPageGet(PGS_POINTER_IS_UNUSED);
    }

    // TODO what about xmin, xmax committed/aborted flags?
    if (pointer->xmin > xid || (pointer->xmax > 0 && pointer->xmax < xid)) {
        return emptyPageGet(PGS_POINTER_IS_NOT_VISIBLE);
    }

    Row row = getRowOnPointerOffset(pointer);

    return {
            PGS_OK,
            row
    };
}

[[maybe_unused]]
PagePut Page8::put(uint32_t xid, Row row) {
    int16_t rowSize = row.sizeWithHeader();
    int16_t freeSpace = getFreeSpace();
    if (rowSize > freeSpace) {
        return notEnoughSpacePagePut();
    }

    rowsOffset = (int16_t) (rowsOffset - rowSize);

    int16_t firstUnusedPointerOffset = findFirstUnusedPointerOffset();
    PagePointer *pointer;
    if (firstUnusedPointerOffset == -1) {
        auto rowAndPointerSize = (int16_t) (rowSize + sizeof(PagePointer));
        if (rowAndPointerSize > freeSpace) {
            return notEnoughSpacePagePut();
        }

        pointer = createPagePointerOnOffset(xid, pointerOffset, row);
        pointerOffset += sizeof(PagePointer);
    } else {
        pointer = createPagePointerOnOffset(xid, firstUnusedPointerOffset, row);
    }

    placeRowOnCurrentOffset(row);

    return {
            PPS_OK,
            createPidByPointer(pointer)
    };
}

[[maybe_unused]]
PageRemove Page8::remove(uint32_t xid, models::PID pid) {
    if (!isPidValid(pid)) {
        return PageRemove{PRS_INVALID_PID};
    }

    int16_t targetPointerOffset = calcPointerOffsetByPid(pid);
    PagePointer *pointer = getPointerByOffset(targetPointerOffset);

    if (!pointer->isStatusUsed()) {
        return PageRemove{PRS_POINTER_IS_UNUSED};
    }

    // TODO what about xmin, xmax committed/aborted flags?
    if (pointer->xmin > xid || (pointer->xmax > 0 && pointer->xmax < xid)) {
        return PageRemove{PRS_POINTER_IS_NOT_VISIBLE};
    }

    pointer->xmax = xid;
    //TODO what about committed/aborted flags&

    return PageRemove{PRS_OK};
}

// ==================================== PRIVATE SECTION =============================================================

void Page8::iterateOverPointers(const std::function<void(PagePointer *)> &consumer) {
    int16_t pointersNumber = calcPointersNumber();

    models::PID currentPid(0, 0, 0);
    int16_t currentPointerOffset;
    PagePointer *currentPointer;
    for (int16_t i = 0; i < pointersNumber; i++) {
        currentPid = models::PID(fileNumber, pageNumber, i);
        currentPointerOffset = calcPointerOffsetByPid(currentPid);
        currentPointer = getPointerByOffset(currentPointerOffset);

        consumer(currentPointer);
    }
}

int16_t Page8::findFirstUnusedPointerOffset() const {
    int16_t currentPointerOffset = 0;
    PagePointer* pointer = getPointerByOffset(currentPointerOffset);
    while (pointer->status != POINTER_STATUS_UNUSED && currentPointerOffset < pointerOffset) {
        currentPointerOffset = (int16_t) (currentPointerOffset + sizeof(PagePointer));
        pointer = getPointerByOffset(currentPointerOffset);
    }

    if (currentPointerOffset >= pointerOffset) return -1;

    return currentPointerOffset;
}

PagePointer *Page8::createPagePointerOnOffset(uint32_t xid, int16_t targetPointerOffset, Row row) {
    PagePointer *pointer = getPointerByOffset(targetPointerOffset);
    pointer->status = POINTER_STATUS_USED;
    pointer->rowPositionOffset = rowsOffset;
    pointer->size = (int16_t) row.sizeWithHeader();
    pointer->number = calcPointersNumber();
    pointer->xmin = xid;
    //TODO what about xmin committed/aborted flags?
    pointer->xmax = 0;
    pointer->setXmaxAborted();
    return pointer;
}

void Page8::placeRowOnCurrentOffset(Row row) {
    auto copyStart = (int8_t *) row.rowHeader;
    int8_t *copyEnds = (int8_t *) row.rowHeader + sizeof(RowHeader);
    int8_t *copyTo = (int8_t *) content + rowsOffset;
    std::copy(copyStart, copyEnds, copyTo);

    copyStart = (int8_t *) row.payload;
    copyEnds = (int8_t *) row.payload + row.rowHeader->payloadSize;
    copyTo += sizeof(RowHeader);
    std::copy(copyStart, copyEnds, copyTo);
}

Row Page8::getRowOnPointerOffset(PagePointer *pointer) const {
    auto undefined = (void *) (content + pointer->rowPositionOffset);
    auto payload = (void *) ((int8_t *) undefined + sizeof(RowHeader));
    auto rowHeader = (RowHeader *) undefined;
    return {
            rowHeader,
            payload
    };
}

int16_t Page8::calcPointersNumber() const {
    return (int16_t) (pointerOffset / sizeof(PagePointer));
}

models::PID Page8::createPidByPointer(PagePointer *pagePointer) const {
    return {fileNumber, pageNumber, pagePointer->number};
}

int16_t Page8::calcPointerOffsetByPid(models::PID requestedPid) {
    return (int16_t) (requestedPid.getRowPointerNumber() * sizeof(PagePointer));
}

bool Page8::isPidValid(models::PID requestedPid) const {
    bool fileCorrect = fileNumber == requestedPid.getFileNumber();
    bool pageCorrect = pageNumber == requestedPid.getPageNumber();

    auto requestedPointerOffset = calcPointerOffsetByPid(requestedPid);
    bool pointerCorrect = requestedPointerOffset >= 0 && requestedPointerOffset < pointerOffset;

    return fileCorrect && pageCorrect && pointerCorrect;
}

PagePointer *Page8::getPointerByOffset(int16_t offset) const {
    return (PagePointer *) (void *) (content + offset);
}

PageGet memory::emptyPageGet(PageGetStatus status) {
    Row row = {nullptr, nullptr};
    return {status, row};
}

PagePut memory::notEnoughSpacePagePut() {
    return {
            memory::PPS_NOT_ENOUGH_SPACE,
            models::PID(0, 0, 0)
    };
}
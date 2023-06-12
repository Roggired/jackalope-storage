//
// Created by ego on 11.06.23.
//

#include "fsi.hpp"

using namespace std;
using namespace freeSpaceIndex;

template<class Archive>
void FreeSpaceIndex::serialize(Archive& boostArchive, const unsigned int version) const {
    boostArchive & nodes;
}

[[maybe_unused]]
[[nodiscard]]
uint16_t FreeSpaceIndex::findAppropriatePageNumber(uint16_t requestedFreeSpace) {
    lock_guard<mutex> locker(nodesMutex);

    auto largestPageIterator = nodes.begin();
    if (largestPageIterator == nodes.end()) {
        throw CantFindAppropriatePageException();
    }

    if (largestPageIterator->freeSpace < requestedFreeSpace) {
        throw CantFindAppropriatePageException();
    }

    return largestPageIterator->pageNumber;
}

[[maybe_unused]]
void FreeSpaceIndex::insertOrUpdateNode(const memory::Page &page) {
    lock_guard<mutex> locker(nodesMutex);

    removeNode(page);

    FSINode newNode = {
        page.getPageNumber(),
        page.getFreeSpace()
    };
    nodes.insert(newNode);
}

[[maybe_unused]]
void FreeSpaceIndex::removeNode(const memory::Page &page) {
    lock_guard<mutex> locker(nodesMutex);

    auto existedNode = nodes.end();
    for (auto iterator = nodes.begin(); iterator != nodes.end(); iterator++) {
        if (iterator->pageNumber == page.getPageNumber()) {
            existedNode = iterator;
            break;
        }
    }

    if (existedNode != nodes.end()) {
        nodes.erase(existedNode);
    }
}
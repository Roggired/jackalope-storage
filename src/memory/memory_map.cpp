//
// Created by ego on 08.05.23.
//

#include "memory_map.hpp"

using namespace memory;
using namespace memoryMapKey;
using namespace std;


// ==================================== PUBLIC SECTION =============================================================

[[maybe_unused]]
Key memoryMapKey::keyByPid(models::PID pid) {
    return createKey(pid.getFileNumber(), pid.getPageNumber());
}

[[maybe_unused]]
Key memoryMapKey::keyByPage8(const Page *page) {
    return createKey(page->getFileNumber(), page->getPageNumber());
}

[[maybe_unused]]
Key memoryMapKey::createKey(uint32_t fileNumber, uint16_t pageNumber) {
    int64_t key = (int64_t) fileNumber << 32;
    key |= (int64_t) pageNumber;
    return key;
}

[[maybe_unused]]
uint32_t memoryMapKey::fileNumberByKey(Key key) {
    return (uint32_t) ((key & MEMORY_MAP_KEY_FILE_NUMBER_MASK) >> 32);
}

[[maybe_unused]]
uint16_t memoryMapKey::pageNumberByKey(Key key) {
    return (uint16_t) (key & MEMORY_MAP_KEY_PAGE_NUMBER_MASK);
}

[[maybe_unused]]
Page& MemoryMap::loadPage(const Page *page8) {
    lock_guard<mutex> locker(pageMapMutex);

    Key key = keyByPage8(page8);

    auto existedPage = pageMap.find(key);
    if (existedPage != pageMap.end()) {
        throw PageExistsException();
    }

    uint32_t currentMemoryUsage = PAGE_8_SIZE * pageMap.size();
    if (currentMemoryUsage + PAGE_8_SIZE > memoryLimit) {
        throw MemoryLimitException();
    }

    auto iterator = pageMap.insert(
            {
                    key,
                    *page8
            }
    ).first;
    return (*iterator).second;
}

[[maybe_unused]]
Page& MemoryMap::createPage(FileHeader fileHeader, int8_t pageType) {
    lock_guard<mutex> locker(pageMapMutex);

    Key key = createKey(fileHeader.fileNumber, fileHeader.pagesNumber);

    auto existedPage = pageMap.find(key);
    if (existedPage != pageMap.end()) {
        throw FileHeaderInvalidPagesNumberException();
    }

    uint32_t currentMemoryUsage = PAGE_8_SIZE * pageMap.size();
    if (currentMemoryUsage + PAGE_8_SIZE > memoryLimit) {
        throw MemoryLimitException();
    }

    auto iterator = pageMap.insert(
            {
                    key,
                    Page(
                        fileNumberByKey(key),
                        pageNumberByKey(key),
                        pageType
                )
            }
    ).first;
    return (*iterator).second;
}

[[maybe_unused]]
void MemoryMap::unloadPage(memoryMapKey::Key key) {
    lock_guard<mutex> locker(pageMapMutex);

    auto existedPage = pageMap.find(key);
    if (existedPage == pageMap.end()) {
        throw NoSuchPageException();
    }

    pageMap.erase(key);
}

[[maybe_unused]]
Page& MemoryMap::getPageByKey(memoryMapKey::Key key) {
    lock_guard<mutex> locker(pageMapMutex);

    auto pageIterator = pageMap.find(key);
    if (pageIterator == pageMap.end()) {
        throw NoSuchPageException();
    }

    return (*pageIterator).second;
}


// ==================================== PRIVATE SECTION =============================================================



//
// Created by roggired on 12.06.23.
//

#include "fsi_container.hpp"

using namespace freeSpaceIndex;

// ################### PUBLIC SECTION ########################################
[[maybe_unused]]
[[nodiscard]]
freeSpaceIndexKey::Key keyByFileHeader(memory::FileHeader fileHeader) {
    return fileHeader.fileNumber;
}

[[maybe_unused]]
[[nodiscard]]
freeSpaceIndexKey::Key keyByPid(models::PID pid) {
    return pid.getFileNumber();
}

[[maybe_unused]]
[[nodiscard]]
freeSpaceIndexKey::Key keyByMemoryMapKey(memory::memoryMapKey::Key key) {
    return memory::memoryMapKey::fileNumberByKey(key);
}

[[maybe_unused]]
[[nodiscard]]
freeSpaceIndexKey::Key keyByPage(const memory::Page& page) {
    return page.getFileNumber();
}

// ################ MemoryMap Interface #####################
[[maybe_unused]]
[[nodiscard]]
memory::Page& FreeSpaceIndexContainer::loadPage(memory::FileHeader fileHeader, const memory::Page* page) {
    auto fsi = createOrLoadFSI(
            freeSpaceIndexKey::keyByPage(*page),
            fileHeader,
            *page
    );
    fsi->insertOrUpdateNode(*page);
    return memoryMap->loadPage(page);
}

[[maybe_unused]]
[[nodiscard]]
memory::Page& FreeSpaceIndexContainer::createPage(memory::FileHeader fileHeader, int8_t pageType) {
    memory::Page& page = memoryMap->createPage(fileHeader, pageType);
    auto fsi = createOrLoadFSI(
            freeSpaceIndexKey::keyByPage(page),
            fileHeader,
            page
    );
    fsi->insertOrUpdateNode(page);
    return page;
}

[[maybe_unused]]
void FreeSpaceIndexContainer::unloadPage(memory::memoryMapKey::Key key) {
    memoryMap->unloadPage(key);
}

[[maybe_unused]]
[[nodiscard]]
memory::Page& FreeSpaceIndexContainer::getPageByKey(memory::memoryMapKey::Key key) {
    return memoryMap->getPageByKey(key);
}
// ##########################################################

[[maybe_unused]]
[[nodiscard]]
memory::Page& FreeSpaceIndexContainer::getPageWithEnoughFreeSpace(uint16_t requestedSize) {
    bool found = false;
    memory::FileHeader targetFileHeader;
    uint16_t targetPageNumber = 0;

    {
        lock_guard<mutex> locker(fsiMapMutex);

        for(auto iterator = fsiMap->begin(); iterator != fsiMap->end(); iterator++) {
            try {
                targetPageNumber = iterator->second.second->findAppropriatePageNumber(requestedSize);
                targetFileHeader = iterator->second.first;
                found = true;
                break;
            } catch (CantFindAppropriatePageException exception) {}
        }
    }

    if (!found) {
        throw NotEnoughFreeSpaceException();
    }

    try {
        return memoryMap->getPageByKey(
                memory::memoryMapKey::createKey(targetFileHeader.fileNumber, targetPageNumber)
        );
    } catch (memory::NoSuchPageException exception) {
        throw PageIsNotLoadedException(targetFileHeader, targetPageNumber);
    }
}
// ############################################################################

// ################## PRIVATE SECTION #########################################
[[nodiscard]]
FreeSpaceIndex* FreeSpaceIndexContainer::getFSIByKey(freeSpaceIndexKey::Key fsiKey) {
    return (*fsiMap)[fsiKey].second;
}

FreeSpaceIndex* FreeSpaceIndexContainer::createOrLoadFSI(
        freeSpaceIndexKey::Key fsiKey,
        memory::FileHeader fileHeader,
        const memory::Page& page
) {
    lock_guard<mutex> locker(fsiMapMutex);

    if (fsiMap->find(fsiKey) == fsiMap->end()) {
        //TODO we need to check if FSI exists on disk and load it if so
        fsiMap->insert(
                std::make_pair(
                        fsiKey,
                        std::make_pair(fileHeader, new FreeSpaceIndex())
                )
        );
    }

    return getFSIByKey(fsiKey);
}
// ############################################################################
//
// Created by roggired on 12.06.23.
//

#ifndef JACKALOPE_STORAGE_FSI_CONTAINER_HPP
#define JACKALOPE_STORAGE_FSI_CONTAINER_HPP

#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <mutex>

#include "free_space_index/fsi.hpp"
#include "memory/memory.hpp"
#include "model/pid.hpp"

using namespace std;

namespace freeSpaceIndex {
    namespace freeSpaceIndexKey {
        typedef uint32_t Key;

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByFileHeader(memory::FileHeader fileHeader);

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByPid(models::PID pid);

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByMemoryMapKey(memory::memoryMapKey::Key key);

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByPage(const memory::Page& page);
    }

    class NotEnoughFreeSpaceException: public exception {};
    class PageIsNotLoadedException: public exception {
    public:
        const memory::FileHeader fileHeader;
        const uint16_t pageNumber;

        PageIsNotLoadedException(
                memory::FileHeader fileHeader,
                uint16_t pageNumber
        ): fileHeader(fileHeader), pageNumber(pageNumber) {}
    };

    typedef unordered_map<freeSpaceIndexKey::Key, pair<memory::FileHeader, FreeSpaceIndex*>> FSIMap;
    class [[maybe_unused]] FreeSpaceIndexContainer {
    private:
        //TODO maybe we should implement memory limit for FSI Container?
        FSIMap* fsiMap;
        mutex fsiMapMutex{};
        memory::MemoryMap* memoryMap;

        [[nodiscard]]
        FreeSpaceIndex* getFSIByKey(freeSpaceIndexKey::Key fsiKey);

        FreeSpaceIndex* createOrLoadFSI(freeSpaceIndexKey::Key fsiKey, memory::FileHeader fileHeader, const memory::Page& page);

    public:
        FreeSpaceIndexContainer(memory::MemoryMap* memoryMap) {
            this->memoryMap = memoryMap;
            this->fsiMap = new FSIMap();
        }

        /**
         * FreeSpaceIndexContainer doesn't deconstruct memory::MemoryMap, because the deconstruction
         * process depends on MemoryMap type (local or shared)
         */
        ~FreeSpaceIndexContainer() {
            for (auto iterator = fsiMap->begin(); iterator != fsiMap->end(); iterator++) {
                delete iterator->second.second;
            }
            delete fsiMap;
        }

        // ################ MemoryMap Interface #####################
        [[maybe_unused]]
        [[nodiscard]]
        memory::Page& loadPage(memory::FileHeader fileHeader, const memory::Page* page);

        [[maybe_unused]]
        [[nodiscard]]
        memory::Page& createPage(memory::FileHeader fileHeader, int8_t pageType);

        [[maybe_unused]]
        void unloadPage(memory::memoryMapKey::Key key);

        [[maybe_unused]]
        [[nodiscard]]
        memory::Page& getPageByKey(memory::memoryMapKey::Key key);
        // ##########################################################

        /**
         * Returns memory::Page with enough free space to insert a row
         * @param requestedSize
         * @throws NotEnoughFreeSpaceException if there is no file with enough free space
         * @throws PageIsNotLoadedException if appropriate page has been found in the index, but target page is not
         * loaded in the underlying memoryMap. Exception object will contain information about associated
         * memory::FileHeader and uint16_t pageNumber for future loading
         * @return
         */
        [[maybe_unused]]
        [[nodiscard]]
        memory::Page& getPageWithEnoughFreeSpace(uint16_t requestedSize);
    };
}
#endif //JACKALOPE_STORAGE_FSI_CONTAINER_HPP

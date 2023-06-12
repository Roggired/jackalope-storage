//
// Created by ego on 08.05.23.
//

#ifndef JACKALOPE_STORAGE_MEMORY_MAP_HPP
#define JACKALOPE_STORAGE_MEMORY_MAP_HPP

#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <mutex>
#include "page.hpp"
#include "file_header.hpp"
#include "model/semnet_models.hpp"

using namespace std;


namespace memory {
    namespace memoryMapKey {
        #define MEMORY_MAP_KEY_FILE_NUMBER_MASK 0xffffffff00000000
        #define MEMORY_MAP_KEY_PAGE_NUMBER_MASK 0x00000000ffffffff
        typedef int64_t Key;

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByPid(models::PID pid);

        [[maybe_unused]]
        [[nodiscard]]
        Key keyByPage8(const Page* page);

        [[maybe_unused]]
        [[nodiscard]]
        Key createKey(uint32_t fileNumber, uint16_t pageNumber);

        [[maybe_unused]]
        [[nodiscard]]
        uint32_t fileNumberByKey(Key key);

        [[maybe_unused]]
        [[nodiscard]]
        uint16_t pageNumberByKey(Key key);
    }

    class NoSuchPageException : public exception {};
    class PageExistsException : public exception {};
    class MemoryLimitException : public exception {};
    class FileHeaderInvalidPagesNumberException : public exception {};

    /**
     * MemoryMap is an abstraction of the available memory space. It stores pages by memory keys
     * (fileNumber + pageNumber). We should provide the memory limit value in the constructor to
     * prevent MemoryMap from uncontrolled growing. It should be thread safe (currently not).
     * MemoryMap stores pages internally in the std::unordered_map, therefore lookup operations are
     * really fast (but, keep in mind the worst case of O(n)).
     *
     * We need only to allocate memory for MemoryMap in the appropriate space (process-local or shared memory) and
     * we will get ready memory modal to work with.
     *
     * @author ego
     */
    class [[maybe_unused]] MemoryMap {
    private:
        unordered_map<memoryMapKey::Key, Page> pageMap{};
        uint32_t memoryLimit;
        mutex pageMapMutex{};

    public:
        [[maybe_unused]]
        explicit MemoryMap(uint32_t memoryLimit): memoryLimit(memoryLimit) {}

        /**
         * Copies given page into the pageMap.
         * <br></br>
         * <br></br>
         * <b>THREAD SAFE</b>
         * @param page8
         * @throw PageExistsException if Key associated with given page is already in use
         * @throw MemoryLimitException if creation violates memoryLimit
         * @return reference to a new page in pageMap
         */
        [[maybe_unused]]
        [[nodiscard]]
        Page& loadPage(const Page* page8);

        /**
         * Creates new page and place it into pageMap. Key is generated with fileName and pagesNumber from FileHeader.
         * Calling code should modify FileHeader by itself.
         * <br></br>
         * <br></br>
         * <b>THREAD SAFE</b>
         * @param fileNumber
         * @throw FileHeaderInvalidPagesNumberException if key(fileNumber, pagesNumber) is already in pageMap
         * @throw MemoryLimitException if creation violates memoryLimit
         * @return reference to a new page in pageMap
         */
        [[maybe_unused]]
        [[nodiscard]]
        Page& createPage(FileHeader fileHeader, int8_t pageType);

        /**
         * Removes page with associated key from pageMap.
         * <br></br>
         * <br></br>
         * <b>THREAD SAFE</b>
         * @param key
         * @throw NoSuchPageException
         */
        [[maybe_unused]]
        void unloadPage(memoryMapKey::Key key);

        /**
         * Returns reference to the page by given key.
         * <br></br>
         * <br></br>
         * <b>THREAD SAFE</b>
         * @param key
         * @throw NoSuchPageException
         * @return
         */
        [[maybe_unused]]
        [[nodiscard]]
        Page& getPageByKey(memoryMapKey::Key key);
    };
}
#endif //JACKALOPE_STORAGE_MEMORY_MAP_HPP

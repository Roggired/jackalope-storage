//
// Created by ego on 11.06.23.
//

#ifndef JACKALOPE_STORAGE_FSI_HPP
#define JACKALOPE_STORAGE_FSI_HPP

#include <cstdint>
#include <cstddef>
#include <set>
#include <mutex>
#include "boost/serialization/set.hpp"
#include "memory/memory.hpp"

using namespace std;

namespace freeSpaceIndex {
    struct FSINode {
        uint16_t pageNumber;
        uint16_t freeSpace;
    };
    typedef struct FSINode FSINode;

    struct FSINodeComparator {
        bool operator()(const FSINode& lhs, const FSINode& rhs) const {
            return lhs.freeSpace > rhs.freeSpace;
        }
    };

    class CantFindAppropriatePageException: public exception{};

    class FreeSpaceIndex {
    private:
        friend class boost::serialization::access;

        set<FSINode, FSINodeComparator> nodes;
        mutex nodesMutex{};
    public:
        FreeSpaceIndex() = default;

        template<class Archive>
        [[maybe_unused]]
        void serialize(Archive& boostArchive, unsigned int version) const;

        /**
         * Returns the largest page's number in which requested amount of space is free.
         * @param requestedFreeSpace
         * @throws CantFindAppropriatePageException if there is no pages in the index at all or if there is no page
         * in which requested amount of space is free.
         * @return page's number
         */
        [[maybe_unused]]
        [[nodiscard]]
        uint16_t findAppropriatePageNumber(uint16_t requestedFreeSpace);

        [[maybe_unused]]
        void insertOrUpdateNode(const memory::Page& page);

        [[maybe_unused]]
        void removeNode(const memory::Page& page);
    };

    struct FreeSpaceIndexConfig {
        uint32_t memoryLimit;
    };
    typedef struct FreeSpaceIndexConfig FreeSpaceIndexConfig;
}

#endif //JACKALOPE_STORAGE_FSI_HPP

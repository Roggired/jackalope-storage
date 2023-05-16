//
// Created by ego on 16.05.23.
//

#ifndef JACKALOPE_STORAGE_MEMORY_MAP_FACTORY_HPP
#define JACKALOPE_STORAGE_MEMORY_MAP_FACTORY_HPP

#include <mutex>
#include "memory_map.hpp"

namespace memory {
    struct MemoryMapsConfig {
        uint32_t sharedMemSize;
        uint32_t localMemSize;
    };
    typedef struct MemoryMapsConfig MemoryMapsConfig;

    [[maybe_unused]]
    [[nodiscard]]
    MemoryMap* initLocalMemoryMap(MemoryMapsConfig config);

    [[maybe_unused]]
    void destroyLocalMemoryMap(MemoryMap* localMemoryMap);

    std::mutex sharedMemoryMapMutex;
    MemoryMap* sharedMemoryMapPtr = nullptr;

    /**
     * not thread safe
     */
    [[maybe_unused]]
    [[nodiscard]]
    MemoryMap* initSharedMemoryMap(MemoryMapsConfig config);

    [[maybe_unused]]
    void detachFromSharedMemoryMap(MemoryMap* sharedMemoryMap);

    [[maybe_unused]]
    [[nodiscard]]
    MemoryMap* getSharedMemoryMap();
}
#endif //JACKALOPE_STORAGE_MEMORY_MAP_FACTORY_HPP
